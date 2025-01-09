#include "client_utils.h"

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char *nonce, unsigned char *server_private_key, unsigned char *client_public_key)
{
    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return 1;

        unsigned char *decrypted_buffer = NULL;
        unsigned char *decoded_buffer = b64_decode(buffer, strlen(buffer));

        if (nonce != NULL)
        {

            if (server_private_key == NULL || client_public_key == NULL)
            {
                ERROR("Server private key or client public key is NULL\n");
                return 1;
            }

            TRACE("Received ciphered packet:\n");

            size_t decrypted_len = sizeof(PACKET);
            decrypted_buffer = (unsigned char *)malloc(decrypted_len - crypto_box_MACBYTES);

            TRACE("\t%s\n", buffer);

            if (crypto_box_open_easy(decrypted_buffer, decoded_buffer, decrypted_len, nonce, client_public_key, server_private_key) != 0)
            {
                ERROR("Failed to decrypt message\n");
                return 1;
            }
            free(decoded_buffer);
        }

        packet = (PACKET *)decrypted_buffer ? (PACKET *)decrypted_buffer : (PACKET *)decoded_buffer;

        if (packet->header.message_type != expected_msg_type)
        {
            ERROR("Expected %c message type, received %c\n", expected_msg_type, packet->header.message_type);
            continue;
        }

        TRACE("Received packet [%c] %d/%d\n", packet->header.message_type, packet->header.index + 1, packet->header.count);
        if (!*decoded)
        {
            packet_count = packet->header.count;
            *decoded = malloc(packet->header.total_size);
            *decoded_len = packet->header.total_size;
        }

        size_t usefull_packet_content_size = nonce ? sizeof(packet->content) - crypto_box_MACBYTES : sizeof(packet->content);
        size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % usefull_packet_content_size : usefull_packet_content_size;
        void *dest = *decoded + usefull_packet_content_size * packet->header.index;
        if (dest + content_size <= *decoded + packet->header.total_size)
            memcpy(dest, packet->content, content_size);
        else
        {
            ERROR("Wrong total size\n");
            return 1;
        }
        TRACE("Copying %zu bytes to decoded buffer\n", content_size);
        packet_received++;
    }
    return 0;
}

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port, unsigned char *nonce, unsigned char *client_private_key, unsigned char *server_public_key)
{
    assert(sizeof(PACKET) <= MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH));

    PACKET packet;

    size_t usefull_packet_content_size = nonce ? sizeof(packet.content) - crypto_box_MACBYTES : sizeof(packet.content);

    packet.header.message_type = msg_type;
    packet.header.index = 0;
    packet.header.count = CEIL_DIV(len, usefull_packet_content_size);
    packet.header.total_size = len;

    int hasError = 0;
    for (char *msg_ptr = start; !hasError && packet.header.index < packet.header.count; msg_ptr += usefull_packet_content_size, packet.header.index++)
    {
        size_t content_len = packet.header.index == packet.header.count - 1 ? len % usefull_packet_content_size : usefull_packet_content_size;
        memcpy(packet.content, msg_ptr, content_len);
        bzero(packet.content + content_len, usefull_packet_content_size - content_len);

        unsigned char *encrypted_buffer = nonce != NULL ? malloc(sizeof(PACKET)) : (unsigned char *)&packet;

        if (nonce != NULL && crypto_box_easy(encrypted_buffer, (unsigned char *)&packet, sizeof(PACKET) - crypto_box_MACBYTES, nonce, server_public_key, client_private_key) != 0)
        {
            ERROR("Failed to encrypt message\n");
            free(encrypted_buffer);
            return -1;
        }

        char *encoded_buffer = b64_encode(encrypted_buffer, sizeof(PACKET));
        if (encoded_buffer == NULL)
        {
            ERROR("Failed to encode Base64 message\n");
            free(encrypted_buffer);
            return -1;
        }

        LOG("Sending packet [%c] %d/%d\n", packet.header.message_type, packet.header.index + 1, packet.header.count);

        TRACE("\t%s\n", encoded_buffer);

        hasError = sndmsg(encoded_buffer, port);
        if (nonce)
            free(encrypted_buffer);
        free(encoded_buffer);
    }
    return hasError;
}

int read_message(void **msg, ENCRYPTION_TOOLS *encryption_tools){

    size_t len;
    int err;

    if (encryption_tools == NULL) {
        err = read_bytes(HAND_SHAKE, msg, &len, NULL, NULL, NULL);
        if (!err) {
            char *other_public_key = b64_encode(((HAND_SHAKE_MESSAGE*)*msg)->public_key, crypto_box_PUBLICKEYBYTES);
            char *other_nonce = b64_encode(((HAND_SHAKE_MESSAGE*)*msg)->nonce, crypto_box_NONCEBYTES);
            TRACE("Received public key: %s\n", other_public_key);
            TRACE("Received nonce: %s\n", other_nonce);
            TRACE("Received response port: %d\n", ((HAND_SHAKE_MESSAGE*)*msg)->response_port);
            free(other_public_key);
            free(other_nonce);
        }
    }
    else {
        err = read_bytes(TRANSFERT, msg, &len, encryption_tools->nonce, encryption_tools->private_key, encryption_tools->public_key);
    }
    if (!err) 
        TRACE("Message of %zu bytes received\n", len);
    return err;
}

int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools){
    if (encryption_tools == NULL) {
        HAND_SHAKE_MESSAGE *msg = (HAND_SHAKE_MESSAGE *)message;
        TRACE("Sending handshake message\n");
        TRACE("response_port: %d\n", msg->response_port);
        char *encoded_key = b64_encode(msg->public_key, crypto_box_PUBLICKEYBYTES);
        char *encoded_nonce = b64_encode(msg->nonce, crypto_box_NONCEBYTES);
        TRACE("public_key: %s\nnouce: %s\n", encoded_key, encoded_nonce);
        free(encoded_key);
        free(encoded_nonce);
        return send_memory_zone(msg, sizeof(*msg), HAND_SHAKE, port, NULL, NULL, NULL);
    }
    MESSAGE *msg = (MESSAGE *)message;
    TRACE("Sending message\n");
    return send_memory_zone(msg, sizeof(*msg) + strlen(msg->content) + 1, TRANSFERT, port, encryption_tools->nonce, encryption_tools->private_key, encryption_tools->public_key);
}

int send_handshake_message(int port, int response_port, ENCRYPTION_TOOLS *encryption_tools){
    HAND_SHAKE_MESSAGE msg;
    msg.response_port = response_port;
    memcpy(msg.public_key, encryption_tools->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(msg.nonce, encryption_tools->nonce, crypto_box_NONCEBYTES);
    return send_message(&msg, port, NULL);
}
