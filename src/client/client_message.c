#include "client_message.h"

int send_message(MESSAGE *message, int port)
{
    if (sodium_init() < 0) {
        return -1; 
    }
    
    unsigned char client_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char client_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(client_public_key, client_private_key);

    unsigned char nonce[crypto_box_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    int client_port = 5001;
    int err = startserver(client_port);
    if (err) return err;

    HAND_SHAKE_MESSAGE shake_msg;
    shake_msg.response_port = client_port;
    memcpy(shake_msg.public_key, client_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(shake_msg.nonce, nonce, crypto_box_NONCEBYTES);

    TRACE("client_key: %s\nnouce: %s\n", b64_encode(shake_msg.public_key, crypto_box_PUBLICKEYBYTES), b64_encode(shake_msg.nonce, crypto_box_NONCEBYTES));

    err = send_memory_zone(&shake_msg, sizeof(shake_msg), HAND_SHAKE, port, NULL, NULL, NULL);
    if (err) return err;

    HAND_SHAKE_MESSAGE *response = NULL;
    size_t response_size;

    err = read_bytes(HAND_SHAKE, (void **)&response, &response_size, NULL, NULL, NULL);
    if (err) return err;

    TRACE("Received server public key: %s\n", b64_encode(response->public_key, crypto_box_PUBLICKEYBYTES));

    send_memory_zone(message, sizeof(*message) + strlen(message->content), TRANSFERT, port, nonce, client_private_key, response->public_key);

    err = stopserver();
    return err;
}