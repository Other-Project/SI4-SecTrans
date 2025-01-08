#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <sodium.h>
#include <assert.h>
#include <string.h>
#include "b64.h"
#include "common.h"
#include "message.h"
#include "client.h"
#include "server.h"

int startserver(int port);
int stopserver();

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len, unsigned char* nonce, unsigned char* server_private_key, unsigned char* client_public_key);

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port, unsigned char* nonce, unsigned char* client_private_key, unsigned char* server_public_key);

#endif