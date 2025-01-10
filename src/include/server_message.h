#ifndef SERVER_MESSAGE_H
#define SERVER_MESSAGE_H

#include "client_utils.h"
#include "server_utils.h"

int read_message(void **msg, ENCRYPTION_TOOLS *encryption_tools);
int do_handshake_server(ENCRYPTION_TOOLS *send_encryption_tools, ENCRYPTION_TOOLS *read_encryption_tools, HAND_SHAKE_MESSAGE **handshake_message);
int read_login(unsigned char **authorized_user_list, ENCRYPTION_TOOLS *encryption_tools);

#endif
