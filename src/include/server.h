#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "message.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

MESSAGE *read_full_message()
{
    PACKET packet;
    MESSAGE *msg = NULL;

    int packet_received = 0;
    while (!getmsg((char *)&packet))
    {
        switch (packet.header.message_type)
        {
        case HAND_SHAKE:
            TRACE("Received hand shake packet:\n\t%s\n", (char *)&packet);
            continue;
        case TRANSFERT:
            TRACE("Received transfert packet\n");
            if (!msg)
                msg = (MESSAGE *)malloc(packet.header.total_size);

            strncpy((char *)&msg + sizeof(packet.content) * packet.header.index, packet.content, sizeof(packet.content));
            if (++packet_received == packet.header.count)
                break;
            continue;
        default:
            ERROR("Received unknown message type %c\n", packet.header.message_type);
            continue;
        }
        TRACE("Full message received\n");
        return msg;
    }

    return NULL;
}
