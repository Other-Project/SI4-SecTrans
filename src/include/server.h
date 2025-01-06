#ifndef SERVER_H
#define SERVER_H

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <sodium.h>
#include "b64.h"
#include "common.h"
#include "message.h"
#include "client.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port);

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len)
{
    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return 1;

        packet = (PACKET *)b64_decode(buffer, strlen(buffer));
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
        size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % sizeof(packet->content) : sizeof(packet->content);
        memcpy(*decoded + sizeof(packet->content) * packet->header.index, packet->content, content_size);
        packet_received++;
    }
    return 0;
}

int read_bytes_ciphered(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char* nonce, unsigned char* server_private_key, unsigned char* client_public_key)
{
    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return 1;

        TRACE("Received ciphered packet:\n");
        unsigned char *decoded_buffer = b64_decode(buffer, strlen(buffer));

        size_t decrypted_len = sizeof(PACKET);
        unsigned char *decrypted_buffer = malloc(decrypted_len - crypto_box_MACBYTES);

        TRACE("\t%s\n", buffer);

        if (crypto_box_open_easy(decrypted_buffer, decoded_buffer, decrypted_len, nonce, client_public_key, server_private_key) != 0)
        {
            ERROR("Failed to decrypt message\n");
            return 1;
        }

        free(decoded_buffer);

        packet = (PACKET *)decrypted_buffer;
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
        size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % sizeof(packet->content) : sizeof(packet->content);
        memcpy(*decoded + sizeof(packet->content) * packet->header.index, packet->content, content_size);
        packet_received++;
    }
    return 0;
}
MESSAGE *read_message()
{
    if (sodium_init() < 0) {
        return NULL; 
    }

    MESSAGE *msg = NULL;
    size_t len;

    HAND_SHAKE_MESSAGE *shake_msg = NULL;
    size_t shake_len;

    if (read_bytes(HAND_SHAKE, (void **)&shake_msg, &shake_len)) {
        return NULL;
    }

    TRACE("response_port: %s\nclient_key: %s\nnouce: %s\n", b64_encode((unsigned char *)&shake_msg->response_port, sizeof(shake_msg->response_port)), b64_encode(shake_msg->public_key, crypto_box_PUBLICKEYBYTES), b64_encode(shake_msg->nonce, crypto_box_NONCEBYTES));

    unsigned char server_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char server_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(server_public_key, server_private_key);

    HAND_SHAKE_MESSAGE response;
    response.response_port = SERVER_PORT;
    memcpy(response.public_key, server_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(response.nonce, shake_msg->nonce, crypto_box_NONCEBYTES);

    TRACE("Sending server public key: %s\n", b64_encode(server_public_key, crypto_box_PUBLICKEYBYTES));

    if (send_memory_zone(&response, sizeof(response), HAND_SHAKE, shake_msg->response_port)) {
        return NULL;
    }

   if (read_bytes_ciphered(TRANSFERT, (void **)&msg, &len, shake_msg->nonce, server_private_key, shake_msg->public_key)) {
        return NULL;
   }

    TRACE("Message of %zu bytes received\n", len);

    return msg;
}

#endif