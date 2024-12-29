#include <malloc.h>
#include "common.h"
#include "message.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

MESSAGE *read_full_message()
{
    PACKET packet;
    int content_size = sizeof(packet.content);
    char *content = malloc(content_size);
    char *last_char = content;
    while (!getmsg((char *)&packet))
    {
        last_char = stpncpy(last_char, packet.content, sizeof(packet.content));
        if (last_char - content < content_size)
        {
            MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + content_size);
            msg->action_type = packet.action_type;
            strcpy(msg->content, content);
            return msg;
        }
        else {
            int i = last_char - content;
            content_size += sizeof(packet.content);
            content = realloc(content, content_size);
            last_char = content + i;
        }
    }

    return NULL;
}
