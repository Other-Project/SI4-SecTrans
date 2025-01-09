#include "client_message.h"

/*int send_message(MESSAGE *message, int port, ENCRYPTION_TOOLS *encryption_tools)
{
    if (sodium_init() < 0)
        return -1;

    unsigned char client_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char client_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(client_public_key, client_private_key);

    unsigned char nonce[crypto_box_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    int err = startserver(CLIENT_PORT);
    if (err)
        return err;

    HAND_SHAKE_MESSAGE shake_msg;
    shake_msg.response_port = CLIENT_PORT;
    memcpy(shake_msg.public_key, client_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(shake_msg.nonce, nonce, crypto_box_NONCEBYTES);

    char *encoded_key = b64_encode(shake_msg.public_key, crypto_box_PUBLICKEYBYTES);
    char *encoded_nonce = b64_encode(shake_msg.nonce, crypto_box_NONCEBYTES);
    TRACE("client_key: %s\nnouce: %s\n", encoded_key, encoded_nonce);
    free(encoded_key);
    free(encoded_nonce);

    err = send_memory_zone(&shake_msg, sizeof(shake_msg), HAND_SHAKE, port, NULL, NULL, NULL);
    if (err)
        return err;

    HAND_SHAKE_MESSAGE *response = NULL;
    size_t response_size;

    err = read_bytes(HAND_SHAKE, (void **)&response, &response_size, NULL, NULL, NULL);
    if (err)
        return err;

    char *encoded_response_key = b64_encode(response->public_key, crypto_box_PUBLICKEYBYTES);
    char *encoded_response_nonce = b64_encode(response->nonce, crypto_box_NONCEBYTES);
    TRACE("Received server public key: %s\n", encoded_response_key);
    TRACE("Received server nonce: %s\n", encoded_response_nonce);
    free(encoded_response_key);
    free(encoded_response_nonce);

    send_memory_zone(message, sizeof(*message) + strlen(message->content) + 1, TRANSFERT, port, nonce, client_private_key, response->public_key);

    free(response);
    err = stopserver();
    return err;
}*/

int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools){
    if (encryption_tools == NULL) {
        return send_memory_zone((HAND_SHAKE_MESSAGE *)message, sizeof(*message), HAND_SHAKE, port, NULL, NULL, NULL);
    }
    MESSAGE *msg = (MESSAGE *)message;
    return send_memory_zone(msg, sizeof(*msg) + strlen(msg->content) + 1, TRANSFERT, port, encryption_tools->nonce, encryption_tools->private_key, encryption_tools->public_key);
}

int send_handshake_message(int port, ENCRYPTION_TOOLS *encryption_tools){
    HAND_SHAKE_MESSAGE msg;
    msg.response_port = CLIENT_PORT;
    memcpy(msg.public_key, encryption_tools->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(msg.nonce, encryption_tools->nonce, crypto_box_NONCEBYTES);
    return send_message(&msg, port, NULL);
}
