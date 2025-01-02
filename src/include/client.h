#include <assert.h>
#include "b64.h"
#include "common.h"
#include "message.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int safe_send_message(MESSAGE *message, int port)
{
    assert(sizeof(PACKET) <= MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH));

    PACKET packet;
    packet.header.message_type = TRANSFERT;
    packet.header.total_size = sizeof(*message) + strlen(message->content);
    packet.header.index = 0;
    packet.header.count = CEIL_DIV(packet.header.total_size, sizeof(packet.content));

    int hasError = 0;
    for (char *msg_ptr = (char *)message; !hasError && packet.header.index < packet.header.count; msg_ptr += sizeof(packet.content), packet.header.index++)
    {
        memcpy(packet.content, msg_ptr, sizeof(packet.content));
        char *buffer = b64_encode((unsigned char *)&packet, sizeof(PACKET));
        LOG("Sending transfert packet %d/%d\n", packet.header.index + 1, packet.header.count);
        TRACE("\t%s\n", buffer);
        hasError = sndmsg(buffer, port);
        free(buffer);
    }
    return hasError;
}
