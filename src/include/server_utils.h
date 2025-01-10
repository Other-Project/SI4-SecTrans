#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include "b64.h"
#include "common.h"
#include "message.h"
#include "client.h"
#include "server.h"
#include "encryption.h"

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char *nonce, unsigned char *server_private_key, unsigned char *client_public_key);

#endif