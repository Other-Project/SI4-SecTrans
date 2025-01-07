#ifndef CLIENT_H
#define CLIENT_H

#include <assert.h>
#include <strings.h>
#include "b64.h"
#include "common.h"
#include "message.h"
#include "server.h"

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int send_memory_zone(void *start, size_t len, MESSAGE_TYPE msg_type, int port)
{
    assert(sizeof(PACKET) <= MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH));

    PACKET packet;
    packet.header.message_type = msg_type;
    packet.header.total_size = len;
    packet.header.index = 0;
    packet.header.count = CEIL_DIV(packet.header.total_size, sizeof(packet.content));

    int hasError = 0;
    for (char *msg_ptr = start; !hasError && packet.header.index < packet.header.count; msg_ptr += sizeof(packet.content), packet.header.index++)
    {
        size_t content_len = packet.header.index == packet.header.count - 1 ? len % sizeof(packet.content) : sizeof(packet.content);
        memcpy(packet.content, msg_ptr, content_len);
        bzero(packet.content + content_len, sizeof(packet.content) - content_len);
        char *buffer = b64_encode((unsigned char *)&packet, sizeof(PACKET));
        LOG("Sending packet [%c] %d/%d\n", packet.header.message_type, packet.header.index + 1, packet.header.count);
        TRACE("\t%s\n", buffer);
        hasError = sndmsg(buffer, port);
        free(buffer);
    }
    return hasError;
}

int send_message(MESSAGE *message, int port)
{
    return send_memory_zone(message, sizeof(*message) + strlen(message->content), TRANSFERT, port);
}

#endif