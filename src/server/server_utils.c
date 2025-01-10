#include "server_utils.h"

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char *nonce, unsigned char *server_private_key, unsigned char *client_public_key)
{
    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return 1;

        unsigned char *decrypted_buffer = NULL;
        size_t decrypted_len;
        unsigned char *decoded_buffer = b64_decode_ex(buffer, strlen(buffer), &decrypted_len);

        if (nonce != NULL)
        {

            if (server_private_key == NULL || client_public_key == NULL)
            {
                ERROR("Server private key or client public key is NULL\n");
                return 1;
            }

            TRACE("Received ciphered packet:\n");

            decrypted_buffer = (unsigned char *)malloc(sizeof(PACKET));
            bzero(decrypted_buffer, sizeof(PACKET));

            TRACE("\t%s\n", buffer);

            if (crypto_box_open_easy(decrypted_buffer, decoded_buffer, decrypted_len, nonce, client_public_key, server_private_key) != 0)
            {
                ERROR("Failed to decrypt message\n");
                return 1;
            }
            decrypted_len = sizeof(PACKET);
            free(decoded_buffer);
        }

        packet = (PACKET *)decrypted_buffer ? (PACKET *)decrypted_buffer : (PACKET *)decoded_buffer;
        if(decrypted_len != sizeof(PACKET)) 
        {
            ERROR("Packet of invalid size received expected %zu got %zu\n", sizeof(PACKET), decrypted_len);
            continue;
        }

        if (packet->header.message_type != expected_msg_type)
        {
            ERROR("Expected %c message type, received %c\n", expected_msg_type, packet->header.message_type);
            continue;
        }

        TRACE("Received packet [%c] %d/%d\n", packet->header.message_type, packet->header.index + 1, packet->header.count);
        if (!*decoded)
        {
            if (packet->header.total_size < sizeof(HAND_SHAKE_MESSAGE) || packet->header.count < 1 || packet->header.index >= packet->header.count)
            {
                ERROR("Corrupted packet\n");
                continue;
            }
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