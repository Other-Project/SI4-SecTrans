#include <assert.h>
#include "common.h"
#include "message.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int safe_send_message(MESSAGE *message, int port)
{
    assert(sizeof(PACKET) <= MAX_PACKET_LENGTH);

    char buffer[sizeof(PACKET)];
    PACKET packet;
    packet.action_type = message->action_type;

    int hasError = 0;
    char *content_end = &packet.content[sizeof(packet.content)];
    char *content_curr_end = content_end;
    for (char *msg_ptr = message->content; !hasError && content_curr_end == content_end; msg_ptr += sizeof(packet.content))
    {
        content_curr_end = stpncpy(packet.content, msg_ptr, sizeof(packet.content));
        memcpy(buffer, &packet, sizeof(PACKET));
        hasError = sndmsg(buffer, port);
    }
    return hasError;
}
