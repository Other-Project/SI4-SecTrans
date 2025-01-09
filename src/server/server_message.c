#include "server_message.h"

/*int read_message(MESSAGE *msg)
{
    if (sodium_init() < 0)
        return -1;

    size_t len;

    HAND_SHAKE_MESSAGE *shake_msg = NULL;
    size_t shake_len;

    int err = (read_bytes(HAND_SHAKE, (void **)&shake_msg, &shake_len, NULL, NULL, NULL));
    if (err)
        return err;

    char *encoded_key = b64_encode(shake_msg->public_key, crypto_box_PUBLICKEYBYTES);
    char *encoded_nonce = b64_encode(shake_msg->nonce, crypto_box_NONCEBYTES);
    TRACE("response_port: %d\nclient_key: %s\nnouce: %s\n", shake_msg->response_port, encoded_key, encoded_nonce);
    free(encoded_key);
    free(encoded_nonce);

    unsigned char server_public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char server_private_key[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(server_public_key, server_private_key);

    unsigned char server_nonce[crypto_box_NONCEBYTES];
    randombytes_buf(server_nonce, sizeof(server_nonce));

    HAND_SHAKE_MESSAGE response;
    response.response_port = SERVER_PORT;
    memcpy(response.public_key, server_public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(response.nonce, server_nonce, crypto_box_NONCEBYTES);

    char *encoded_server_key = b64_encode(server_public_key, crypto_box_PUBLICKEYBYTES);
    TRACE("Sending server public key: %s\n", encoded_server_key);
    free(encoded_server_key);

    err = send_memory_zone(&response, sizeof(response), HAND_SHAKE, shake_msg->response_port, NULL, NULL, NULL);
    if (err)
        return err;

    err = read_bytes(TRANSFERT, (void **)&msg, &len, shake_msg->nonce, server_private_key, shake_msg->public_key);
    if (!err)    
        TRACE("Message of %zu bytes received\n", len);

    free(shake_msg);

    return err;
}*/

int read_message(void *msg, ENCRYPTION_TOOLS *encryption_tools){

    size_t len;

    if (encryption_tools == NULL) {
        int err = read_bytes(HAND_SHAKE, (void **)msg, &len, NULL, NULL, NULL);
        if (!err)
            TRACE("Message of %zu bytes received\n", len);
        return err;
    }
    return read_bytes(TRANSFERT, (void **)msg, &len, encryption_tools->nonce, encryption_tools->public_key, encryption_tools->private_key);
}

    