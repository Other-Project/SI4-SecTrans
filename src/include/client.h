#include <assert.h>
#include "common.h"
#include "message.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int safe_send_message(ACTION_TYPE action_type, char *msg, int port)
{
    assert(sizeof(PACKET) <= MAX_PACKET_LENGTH);

    char buffer[sizeof(PACKET)];
    PACKET packet;
    packet.action_type = action_type;
    int hasError = 0;
    for (char *msg_ptr = msg;; msg_ptr += sizeof(packet.content))
    {
        char *buffer_last_written = stpncpy(packet.content, msg_ptr, sizeof(packet.content));
        memcpy(buffer, &packet, sizeof(PACKET));
        hasError = sndmsg(buffer, port);
        if (hasError || buffer_last_written != &packet.content[sizeof(packet.content)])
            break;
    }
    return hasError;
}
