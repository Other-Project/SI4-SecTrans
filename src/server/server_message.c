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

int hex_to_bin(const char *hex, unsigned char *bin) {
    size_t len = strlen(hex);
    if (len % 2 != 0) {
        return -1;
    }

    for (size_t i = 0; i < len / 2; i++) {
        if (sscanf(hex + 2 * i, "%2hhx", &bin[i]) != 1) {
            return -1; 
        }
    }
    return 0;
}

int read_login(unsigned char** authorized_user_list, ENCRYPTION_TOOLS *encryption_tools) {
    LOGIN_MESSAGE* login_message = NULL;
    size_t len;

    int err = read_bytes(LOGIN, (void**)&login_message, &len, encryption_tools->nonce, encryption_tools->private_key, encryption_tools->public_key);
    if (err) 
        return err;
    unsigned char stored_hash_bin[crypto_hash_BYTES];
    if (hex_to_bin((const char*)authorized_user_list[1], stored_hash_bin) != 0) {
        printf("Error: Failed to convert hex to binary.\n");
        return 1;
    }
    int cmp = strcmp((const char*)login_message->username, (const char*)authorized_user_list[0]) != 0 || memcmp(login_message->hashed_password, stored_hash_bin, crypto_hash_BYTES) != 0;
    free(login_message);
    return cmp;
}
