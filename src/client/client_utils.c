#include "client_utils.h"

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char* nonce, unsigned char* server_private_key, unsigned char* client_public_key)
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

        packet = (PACKET *)decrypted_buffer ? (PACKET *)decrypted_buffer : (PACKET *) decoded_buffer;

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
        size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % usefull_packet_content_size: usefull_packet_content_size;
        TRACE("Copying %zu bytes to decoded buffer\n", content_size);
        memcpy(*decoded + usefull_packet_content_size * packet->header.index, packet->content, content_size);
        packet_received++;
    }
    return 0;
}

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port, unsigned char* nonce, unsigned char* client_private_key, unsigned char* server_public_key)
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
        memcpy(packet.content, msg_ptr, usefull_packet_content_size);

        unsigned char *encrypted_buffer = nonce != NULL ? malloc(sizeof(PACKET)) : (unsigned char *)&packet;

        if (nonce != NULL && crypto_box_easy(encrypted_buffer, (unsigned char *)&packet, sizeof(PACKET) - crypto_box_MACBYTES, nonce, server_public_key, client_private_key) != 0)
        {
            ERROR("Failed to encrypt message\n");
            free(encrypted_buffer);
            return -1;
        }

        char *encoded_buffer = b64_encode(encrypted_buffer, sizeof(PACKET));
        if (encoded_buffer == NULL) {
            ERROR("Failed to encode Base64 message\n");
            free(encrypted_buffer);
            return -1;
        }

        LOG("Sending packet [%c] %d/%d\n", packet.header.message_type, packet.header.index + 1, packet.header.count);   

        TRACE("\t%s\n", encoded_buffer);

        hasError = sndmsg(encoded_buffer, port);
        if (nonce) free(encrypted_buffer);
        free(encoded_buffer);
    }
    return hasError;
}