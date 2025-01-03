#include "common.h"

#ifndef MESSAGE_H
#define MESSAGE_H

typedef enum __attribute__((packed))
{
    UPLOAD = 'U',
    DOWNLOAD = 'D',
    LIST = 'L'
} ACTION_TYPE;

typedef enum __attribute__((packed))
{
    HAND_SHAKE = 'H',
    TRANSFERT = 'T'
} MESSAGE_TYPE;

typedef struct __attribute__((packed))
{
    MESSAGE_TYPE message_type;
    unsigned char index;
    unsigned char count;
    size_t total_size;
} PACKET_HEADER;

typedef struct __attribute__((packed))
{
    PACKET_HEADER header;
    char content[MAX_SIZE_BEFORE_B64(MAX_PACKET_LENGTH - 1) - sizeof(PACKET_HEADER)];
} PACKET;

typedef struct
{
    ACTION_TYPE action_type;
    char filename[MAX_FILENAME_LENGTH];
    char content[];
} MESSAGE;

#endif
