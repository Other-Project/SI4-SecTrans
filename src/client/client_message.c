#include "client_message.h"

int do_handshake_client(int port, int response_port, ENCRYPTION_TOOLS *send_encryption_tools, ENCRYPTION_TOOLS *read_encryption_tools, HAND_SHAKE_MESSAGE *handshake_message){
    int err;
    
    err = send_handshake_message(SERVER_PORT, CLIENT_PORT, send_encryption_tools);
    if (err) 
        return err;
    err = read_message((void**)&handshake_message, NULL);
    if (err) 
        return err;

    memcpy(send_encryption_tools->public_key, handshake_message->public_key, crypto_box_PUBLICKEYBYTES);
    memcpy(read_encryption_tools->private_key, send_encryption_tools->private_key, crypto_box_SECRETKEYBYTES);
    memcpy(read_encryption_tools->nonce, handshake_message->nonce, crypto_box_NONCEBYTES);
    memcpy(read_encryption_tools->public_key, handshake_message->public_key, crypto_box_PUBLICKEYBYTES);
    free(handshake_message);
    return 0;
}

int login(char *username, unsigned char *hashed_password, ENCRYPTION_TOOLS *send_encryption_tools){
    LOGIN_MESSAGE message;
    memcpy(message.username, username, strlen(username));
    memcpy(message.hashed_password, hashed_password, crypto_hash_BYTES);
    int err = send_login_message(SERVER_PORT, &message, send_encryption_tools);
    return err;
}
