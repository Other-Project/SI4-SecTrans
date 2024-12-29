#include <malloc.h>
#include <assert.h>
#include "common.h"
#include "message.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

MESSAGE *read_full_message()
{
    PACKET packet;
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE));
    int content_size = sizeof(packet.content);
    char *content = malloc(content_size);
    
    char *last_char = content;
    while (!getmsg((char *)&packet))
    {
        switch (packet.message_type) // For now, the order of arrival of the packets is important
        {
        case HAND_SHAKE:
            TRACE("Received hand shake packet:\n\t%s\n", (char *)&packet);
            continue;
        case INITIALISE:
            TRACE("Received initialisation packet: %c %s\n", packet.action_type, packet.filename);
            msg->action_type = packet.action_type;
            strcpy(msg->filename, packet.filename);
            continue;
        case TRANSFERT:
            TRACE("Received transfert packet\n");
            last_char = stpncpy(last_char, packet.content, sizeof(packet.content));
            if (last_char - content < content_size)
                break;
            else
            {
                int i = last_char - content;
                content_size += sizeof(packet.content);
                content = realloc(content, content_size);
                last_char = content + i;
                TRACE("Realloc content\n");
            }
            continue;

        default:
            ERROR("Received unknown message type %c\n", packet.message_type);
            continue;
        }

        msg = (MESSAGE *)realloc(msg, sizeof(*msg) + content_size);
        strcpy(msg->content, content);
        TRACE("Full message received\n");
        return msg;
    }

    return NULL;
}
