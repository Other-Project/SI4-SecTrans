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

        TRACE("Raw message data: %s\n", buffer);

        packet = (PACKET *)b64_decode(buffer, strlen(buffer));

        if (packet->header.message_type != expected_msg_type)
        {
            ERROR("Expected %c message type, received %c\n", expected_msg_type, packet->header.message_type);
            continue;
        }

        TRACE("Received packet [%c] %d/%d\n", packet->header.message_type, packet->header.index + 1, packet->header.count);
        {
            packet_count = packet->header.count;
            *decoded = malloc(packet->header.total_size);
            if (!*decoded) {
                ERROR("Failed to allocate memory for decoded message\n");
                free(packet);
                return 1;
            }
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
    MESSAGE *msg = NULL;
    size_t len;

    HAND_SHAKE_MESSAGE shake_msg;

    if (read_bytes(HAND_SHAKE, (void **)&shake_msg, &len)) {
        return NULL;
    }
    TRACE("Handshake message received with client public key\n");

    unsigned char server_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char server_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(server_public_key, server_private_key);

    if (send_memory_zone(server_public_key, crypto_box_PUBLICKEYBYTES, HAND_SHAKE, shake_msg.response_port) != 0) {
        return NULL;
    }
    TRACE("Server public key sent\n");

    if (read_bytes(TRANSFERT, (void **)&msg, &len)) {
        return NULL;
    }
    TRACE("Message of %zu bytes received\n", len);

    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char *encrypted_message = (unsigned char *)msg;
    unsigned char *decrypted_message = malloc(len - crypto_box_MACBYTES);
    if (crypto_box_open_easy(decrypted_message, encrypted_message, len, nonce, shake_msg.public_key, server_private_key) != 0) {
        free(decrypted_message);
        return NULL;
    }
    TRACE("Message decrypted\n");

    return (MESSAGE *)decrypted_message;
}

#endif