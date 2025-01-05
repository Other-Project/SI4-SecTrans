#ifndef CLIENT_H
#define CLIENT_H

#include <assert.h>
#include <sodium.h>
#include "b64.h"
#include "common.h"
#include "message.h"
#include "server.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int startserver(int port);
int stopserver();
int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len);

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port)
{
    assert(sizeof(PACKET) <= MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH));

    PACKET packet;
    packet.header.message_type = msg_type;
    packet.header.total_size = len;
    packet.header.index = 0;
    packet.header.count = CEIL_DIV(packet.header.total_size, sizeof(packet.content));

    int hasError = 0;
    for (char *msg_ptr = start; !hasError && packet.header.index < packet.header.count; msg_ptr += sizeof(packet.content), packet.header.index++)
    {
        memcpy(packet.content, msg_ptr, sizeof(packet.content));
        char *buffer = b64_encode((unsigned char *)&packet, sizeof(PACKET));
        LOG("Sending packet [%c] %d/%d\n", packet.header.message_type, packet.header.index + 1, packet.header.count);
        TRACE("\t%s\n", buffer);
        hasError = sndmsg(buffer, port);
        free(buffer);
    }
    return hasError;
}

int send_message(MESSAGE *message, int port)
{
    if (sodium_init() < 0) {
        return -1; 
    }
    
    unsigned char client_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char client_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(client_public_key, client_private_key);

    int client_port = 5001;
    int err = startserver(client_port);
    if (err) return err;

    HAND_SHAKE_MESSAGE shake_msg;

    shake_msg.response_port = client_port;
    memcpy(shake_msg.public_key, client_public_key, crypto_box_PUBLICKEYBYTES);

    err = send_memory_zone(&shake_msg, sizeof(shake_msg), HAND_SHAKE, port);
    if (err) return err;

    HAND_SHAKE_MESSAGE *response = NULL;
    size_t response_size;
    err = read_bytes(HAND_SHAKE, (void **)&response, &response_size);
    if (err) return err;
    if (sizeof(response->public_key) != crypto_box_PUBLICKEYBYTES) {
        ERROR("Invalid server public key size\n");
        free(response);
        return -1;
    }

    size_t message_len = sizeof(*message) + strlen(message->content);
    unsigned char nonce[crypto_box_NONCEBYTES];
    unsigned char *encrypted_message = malloc(message_len + crypto_box_MACBYTES);
    randombytes_buf(nonce, sizeof nonce);
    if (crypto_box_easy(encrypted_message, (unsigned char *)message, message_len, nonce, response->public_key, client_private_key) != 0) {
        free(response);
        free(encrypted_message);
        return -1; 
    }

    ENCRYPTED_MESSAGE *encrypted_msg = (ENCRYPTED_MESSAGE *)malloc(sizeof(ENCRYPTED_MESSAGE) + message_len + crypto_box_MACBYTES);
    encrypted_msg->action_type = message->action_type;
    strcpy(encrypted_msg->filename, message->filename);
    memcpy(encrypted_msg->content, encrypted_message, message_len + crypto_box_MACBYTES);
    memcpy(encrypted_msg->nonce, nonce, crypto_box_NONCEBYTES);

    err = send_memory_zone(encrypted_msg, sizeof(*encrypted_msg) + message_len + crypto_box_MACBYTES, TRANSFERT, port);

    free(response);
    free(encrypted_message);
    free(encrypted_msg);

    if (err) return err;

    err = stopserver();
    return err;
}

#endif