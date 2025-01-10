#include "server_message.h"

int read_message(void **msg, ENCRYPTION_TOOLS *encryption_tools)
{

    size_t len = 0;
    int err = 0;

    if (encryption_tools == NULL)
    {
        err = read_bytes(HAND_SHAKE, msg, &len, NULL, NULL, NULL);
        if (!err)
        {
            char *other_public_key = b64_encode(((HAND_SHAKE_MESSAGE *)*msg)->public_key, crypto_box_PUBLICKEYBYTES);
            char *other_nonce = b64_encode(((HAND_SHAKE_MESSAGE *)*msg)->nonce, crypto_box_NONCEBYTES);
            TRACE("Received public key: %s\n", other_public_key);
            TRACE("Received nonce: %s\n", other_nonce);
            TRACE("Received response port: %d\n", ((HAND_SHAKE_MESSAGE *)*msg)->response_port);
            free(other_public_key);
            free(other_nonce);
        }
    }
    else
    {
        err = read_bytes(TRANSFERT, msg, &len, encryption_tools->nonce, encryption_tools->private_key, encryption_tools->public_key);
    }
    if (!err)
        TRACE("Message of %zu bytes received\n", len);
    return err;
}

int do_handshake_server(ENCRYPTION_TOOLS *send_encryption_tools, ENCRYPTION_TOOLS *read_encryption_tools, HAND_SHAKE_MESSAGE **handshake_message) {
    int err;

    err = read_message((void**)handshake_message, NULL);
    if (err) 
        return err;
	err = send_handshake_message((*handshake_message)->response_port, SERVER_PORT, send_encryption_tools);
    if (err)
        return err;

    memcpy(send_encryption_tools->public_key, (*handshake_message)->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(read_encryption_tools->private_key, send_encryption_tools->private_key, crypto_box_SECRETKEYBYTES);
    memcpy(read_encryption_tools->nonce, (*handshake_message)->nonce, crypto_box_NONCEBYTES);
    memcpy(read_encryption_tools->public_key, (*handshake_message)->public_key, crypto_box_PUBLICKEYBYTES);
    return 0;
}
