#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include "client.h"
#include "common.h"

#define CHECK_ARGS(nb, msg) \
    if (argc < nb)          \
        FATAL("%s\n", msg);

void stopServer(int _)
{
    if (stopserver())
        FATAL("Failed to gracefully stop the server\n");
    exit(0);
}

MESSAGE *upload_message(char filename[])
{
    char *content = NULL;
    long buffer_size;
    if ((buffer_size = get_file_content(&content, filename)) == -1)
        ERROR("Failed to obtain file content\n");
    char *buffer_64 = b64_encode((unsigned char *)content, buffer_size);
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(buffer_64));
    msg->action_type = UPLOAD;
    char *end = stpcpy(msg->filename, filename);
    bzero(end, msg->content - end);
    strcpy(msg->content, buffer_64);
    free(content);
    free(buffer_64);
    return msg;
}

void download_message(char filename[])
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + 5);
    msg->action_type = DOWNLOAD;
    strcpy(msg->filename, filename);
    strcpy(msg->content, "5001");
    LOG("Sending message\n");
    startserver(CLIENT_PORT);
    if (send_message(msg, SERVER_PORT))
        FATAL("Error sending message\n");
    free(msg);
    while (1)
    {
        MESSAGE *message;
        if (read_message(&message))
            FATAL("Couldn't read server response\n");
        switch (message->action_type)
        {
        case DOWNLOAD:
            LOG("Received download message\n");
            if (create_file_from_message(message, DIRECTORY_CLIENT) == -1)
                ERROR("Failed to create file from message\n");
            stopServer(0);
            return;
        default:
            ERROR("Received unknown action type %c\n", message->action_type);
            break;
        }
        free(message);
    }
}

void list_message()
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + 5);
    msg->action_type = LIST;
    msg->filename[0] = '\0';
    strcpy(msg->content, "5001");
    LOG("Sending message\n");
    if (send_message(msg, SERVER_PORT))
        FATAL("Error sending message\n");
    free(msg);
    startserver(CLIENT_PORT);
    while (1)
    {
        MESSAGE *message = NULL;
        if (read_message(&message))
            FATAL("Couldn't read server response\n");
        switch (message->action_type)
        {
        case LIST:
            LOG("Received list message\n");
            printf("Files on server:\n\n%s", message->content);
            free(message);
            stopServer(0);
            return;
        default:
            ERROR("Received unknown action type %c\n", message->action_type);
            break;
        }
        free(message);
    }
}

int main(int argc, char *argv[])
{
    CHECK_ARGS(2, "Missing action argument");

    if (!strcmp(argv[1], "-up"))
    {
        MESSAGE *message;
        CHECK_ARGS(3, "The filename must be provided");
        message = upload_message(argv[2]);
        LOG("Sending message\n");
        if (send_message(message, SERVER_PORT))
            FATAL("Error sending message\n");
        free(message);
    }
    else if (!strcmp(argv[1], "-down"))
    {
        CHECK_ARGS(3, "The filename must be provided");
        download_message(argv[2]);
    }
    else if (!strcmp(argv[1], "-list"))
    {
        list_message();
    }
    else
        FATAL("Unknown action\n");
    return 0;
}
