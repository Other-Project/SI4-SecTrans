#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "b64.h"
#include "common.h"
#include "message.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

MESSAGE *read_full_message()
{
    MESSAGE *msg = NULL;

    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return NULL;

        packet = (PACKET *)b64_decode(buffer, strlen(buffer));
        switch (packet->header.message_type)
        {
        case HAND_SHAKE:
            TRACE("Received hand shake packet:\n\t%s\n", packet->content);
            packet_received--;
            continue;
        case TRANSFERT:
            TRACE("Received transfert packet %d/%d\n", packet->header.index + 1, packet->header.count);
            if (!msg)
            {
                packet_count = packet->header.count;
                msg = malloc(packet->header.total_size);
            }
            size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % sizeof(packet->content) : sizeof(packet->content);
            strncpy((char *)msg + sizeof(packet->content) * packet->header.index, packet->content, content_size);
            packet_received++;
            continue;
        default:
            ERROR("Received unknown message type %c\n", packet->header.message_type);
            packet_received--;
            continue;
        }
    }

    TRACE("Full message received:\n\t%s\n", msg->content);
    return msg;
}
