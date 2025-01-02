#include <assert.h>
#include "binn/binn.h"
#include "common.h"
#include "message.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int safe_send_message(MESSAGE *message, int port)
{
    assert(sizeof(PACKET) < MAX_PACKET_LENGTH);

    PACKET packet;
    packet.header.message_type = TRANSFERT;
    packet.header.total_size = sizeof(*message) + strlen(message->content);
    packet.header.index = 0;
    packet.header.count = CEIL_DIV(packet.header.total_size, sizeof(packet.content));

    char buffer[MAX_PACKET_LENGTH];
    buffer[sizeof(PACKET)] = '\0';
    int hasError = 0;
    for (char *msg_ptr = (char *)&message; !hasError && packet.header.index < packet.header.count; msg_ptr += sizeof(packet.content), packet.header.index++)
    {
        strncpy(packet.content, msg_ptr, sizeof(packet.content));
        memcpy(buffer, &packet, sizeof(PACKET));
        TRACE("Sending transfert packet:\n\t%s\n", buffer);
        hasError = sndmsg(buffer, port);
    }
    return hasError;
}
