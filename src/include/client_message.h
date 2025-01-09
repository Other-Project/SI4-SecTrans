#ifndef CLIENT_MESSAGE_H
#define CLIENT_MESSAGE_H

#include "client_utils.h"

int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools);
int send_handshake_message(int port, int response_port, ENCRYPTION_TOOLS *encryption_tools);
int do_handshake_client(int port, int response_port, ENCRYPTION_TOOLS *send_encryption_tools, ENCRYPTION_TOOLS *read_encryption_tools, HAND_SHAKE_MESSAGE *handshake_message);

#endif
