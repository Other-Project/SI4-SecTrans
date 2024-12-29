#include "common.h"

#ifndef MESSAGE_H
#define MESSAGE_H

typedef enum __attribute__((packed))
{
    UPLOAD = 'U',
    DOWNLOAD = 'D',
    LIST = 'L'
} ACTION_TYPE;

typedef struct __attribute__((packed))
{
    ACTION_TYPE action_type;
    char content[MAX_PACKET_LENGTH - sizeof(ACTION_TYPE)];
} PACKET;

typedef struct
{
    ACTION_TYPE action_type;
    char content[];
} MESSAGE;

#endif
