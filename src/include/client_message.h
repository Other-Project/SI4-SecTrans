#ifndef CLIENT_MESSAGE_H
#define CLIENT_MESSAGE_H

#include "client_utils.h"

int send_message(void *message, int port, ENCRYPTION_TOOLS *encryption_tools);
int send_handshake_message(int port, int response_port, ENCRYPTION_TOOLS *encryption_tools);

#endif
