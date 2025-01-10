#include <strings.h>
#include "client_utils.h"

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port, unsigned char *nonce, unsigned char *client_private_key, unsigned char *server_public_key)
{
    assert(sizeof(PACKET) <= MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH));

    PACKET packet;

    size_t usefull_packet_content_size = sizeof(packet.content);

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

        size_t encrypted_size = sizeof(PACKET) + crypto_box_MACBYTES;
        unsigned char *encrypted_buffer = nonce != NULL ? malloc(encrypted_size) : (unsigned char *)&packet;

        if (nonce != NULL && crypto_box_easy(encrypted_buffer, (unsigned char *)&packet, encrypted_size, nonce, server_public_key, client_private_key) != 0)
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

int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools)
{
    if (encryption_tools == NULL)
    {
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

int send_handshake_message(int port, int response_port, ENCRYPTION_TOOLS *encryption_tools)
{
    HAND_SHAKE_MESSAGE msg;
    msg.response_port = response_port;
    memcpy(msg.public_key, encryption_tools->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(msg.nonce, encryption_tools->nonce, crypto_box_NONCEBYTES);
    return send_message(&msg, port, NULL);
}
