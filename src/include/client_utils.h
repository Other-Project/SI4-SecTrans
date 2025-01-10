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
#include "encryption.h"

int startserver(int port);
int stopserver();

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port, unsigned char *nonce, unsigned char *client_private_key, unsigned char *server_public_key);
int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools);
int send_handshake_message(int port, int response_port, ENCRYPTION_TOOLS *encryption_tools);
int send_login_message(int port, LOGIN_MESSAGE* message, ENCRYPTION_TOOLS *encryption_tools);

#endif
