#ifndef SERVER_H
#define SERVER_H

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "b64.h"
#include "message.h"
#include "client.h"

int startserver(int port);
int stopserver();

/* read message sent by client */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

int read_bytes(MESSAGE_TYPE expected_msg_type, void **decoded, size_t *decoded_len)
{
    char buffer[MAX_PACKET_LENGTH];
    PACKET *packet = NULL;
    for (int packet_received = 0, packet_count = 1; packet_received < packet_count; free(packet))
    {
        if (getmsg(buffer))
            return 1;

        packet = (PACKET *)b64_decode(buffer, strlen(buffer));
        if (packet->header.message_type != expected_msg_type)
        {
            ERROR("Expected %c message type, received %c\n", expected_msg_type, packet->header.message_type);
            continue;
        }

        TRACE("Received packet [%c] %d/%d\n", packet->header.message_type, packet->header.index + 1, packet->header.count);
        if (!*decoded)
        {
            packet_count = packet->header.count;
            *decoded = malloc(packet->header.total_size);
            *decoded_len = packet->header.total_size;
        }
        size_t content_size = packet->header.index == packet_count - 1 ? packet->header.total_size % sizeof(packet->content) : sizeof(packet->content);
        memcpy(*decoded + sizeof(packet->content) * packet->header.index, packet->content, content_size);
        packet_received++;
    }
    return 0;
}

MESSAGE *read_message()
{
    MESSAGE *msg = NULL;
    size_t len;
    if (read_bytes(TRANSFERT, (void **)&msg, &len))
        return NULL;
    TRACE("Message of %zu bytes received\n", len);
    return msg;
}


#endif