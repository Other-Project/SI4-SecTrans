#include "server_message.h"
#include "client_message.h"

int do_handshake_server(int port, ENCRYPTION_TOOLS *send_encryption_tools, ENCRYPTION_TOOLS *read_encryption_tools, HAND_SHAKE_MESSAGE *handshake_message) {
    int err;

    err = read_message((void**)&handshake_message, NULL);
    if (err) 
        return err;
	err = send_handshake_message(handshake_message->response_port, SERVER_PORT, send_encryption_tools);
    if (err)
        return err;

    memcpy(send_encryption_tools->public_key, handshake_message->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(read_encryption_tools->private_key, send_encryption_tools->private_key, crypto_box_SECRETKEYBYTES);
    memcpy(read_encryption_tools->nonce, handshake_message->nonce, crypto_box_NONCEBYTES);
    memcpy(read_encryption_tools->public_key, handshake_message->public_key, crypto_box_PUBLICKEYBYTES);
    return 0;
}
