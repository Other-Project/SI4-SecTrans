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
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        if (fseek(file, 0L, SEEK_END) == 0)
        {
            buffer_size = ftell(file);
            if (buffer_size == -1)
            {
                ERROR("Failed to create buffer size when uploading message");
                exit(1);
            }
            content = malloc(sizeof(char) * (buffer_size + 1));
            if (fseek(file, 0L, SEEK_SET) != 0)
            {
                ERROR("Failed to return at the start of file when uploading message");
                exit(1);
            }
            size_t newLen = fread(content, sizeof(char), buffer_size+1, file);
            if (ferror(file) != 0)
            {
                ERROR("Error while reading file");
                exit(1);
            }
        }
        else
        {
            ERROR("Can't go to the end of the file");
            exit(1);
        }
    }
    else
    {
        ERROR("Can't read the file");
        exit(1);
    }
    fclose(file);
    printf("Buffer size: %ld\n", buffer_size);
    size_t encoded_size = 4 * ((buffer_size + 2) / 3);
    printf("Encoded size: %zu\n", encoded_size);
    char *buffer_64 = b64_encode((unsigned char *)content, buffer_size);
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + encoded_size);
    msg->action_type = UPLOAD;
    char *end = stpcpy(msg->filename, filename);
    bzero(end, msg->content - end);
    memcpy(msg->content, buffer_64, encoded_size);
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
        MESSAGE *message = read_message();
        switch (message->action_type)
        {
        case DOWNLOAD:
            LOG("Received download message\n");
            struct stat st = {0};
            if (stat("../src/client/files", &st) == -1)
            {
                mkdir("../src/client/files", 0700);
            }
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "../src/client/files/%s", basename(message->filename));
            FILE *file = fopen(filepath, "wb");
            if (file == NULL)
            {
                ERROR("Can't create file for uploading");
                exit(1);
            }
            size_t decoded_size = (strlen(message->content)) * 3 / 4 + 1;
            char *buffer = (char *)b64_decode(message->content, strlen(message->content));
            size_t buffer_size = strlen(buffer);
            size_t written = fwrite(buffer, sizeof(char), decoded_size, file);
            fclose(file);
            free(buffer);
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
        MESSAGE *message = read_message();
        switch (message->action_type)
        {
        case LIST:
            LOG("Received list message\n");
            printf("Files on server:\n\n%s\n", message->content);
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
