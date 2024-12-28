#include "common.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_MSG_LENGTH], int port);

int safe_send_message(char *msg, int port)
{
    char buffer[MAX_MSG_LENGTH];
    int status = 0;
    for (char *msg_ptr = msg;; msg_ptr += MAX_MSG_LENGTH)
    {
        char *buffer_last_written = stpncpy(buffer, msg_ptr, MAX_MSG_LENGTH);
        if ((status = sndmsg(buffer, port)) || buffer_last_written != &buffer[MAX_MSG_LENGTH])
            break;
    }
    return status;
}
