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
    INITIALISE = 'I',
    TRANSFERT = 'T'
} MESSAGE_TYPE;

typedef struct __attribute__((packed))
{
    MESSAGE_TYPE message_type;
    union
    {
        struct
        {
            ACTION_TYPE action_type;
            char filename[MAX_FILENAME_LENGTH];
        }; // For initialisation phase
        char content[MAX_PACKET_LENGTH - 1 - sizeof(MESSAGE_TYPE)]; // For hand_shake and transfert phases
    };
} PACKET;

typedef struct
{
    ACTION_TYPE action_type;
    char filename[MAX_FILENAME_LENGTH];
    char content[];
} MESSAGE;

#endif
