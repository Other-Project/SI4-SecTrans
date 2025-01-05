#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "client.h"
#include "common.h"

#define CHECK_ARGS(nb, msg) \
    if (argc < nb)          \
        FATAL("%s\n", msg);

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
            size_t newLen = fread(content, sizeof(char), buffer_size, file);
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
    size_t encoded_size = 4 * ((buffer_size + 1) / 3);
    char *buffer_64 = b64_encode((unsigned char *)content, encoded_size + 1);
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + encoded_size + 1);
    msg->action_type = UPLOAD;
    strcpy(msg->filename, filename);
    memcpy(msg->content, buffer_64, encoded_size);
    free(content);
    free(buffer_64);
    return msg;
}

MESSAGE *download_message(char filename[])
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE));
    msg->action_type = DOWNLOAD;
    strcpy(msg->filename, filename);
    return msg;
}

MESSAGE *list_message()
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE));
    msg->action_type = LIST;
    msg->filename[0] = '\0';
    return msg;
}

int main(int argc, char *argv[])
{
    CHECK_ARGS(2, "Missing action argument");

    MESSAGE *message;
    if (!strcmp(argv[1], "-up"))
    {
        CHECK_ARGS(3, "The filename must be provided");
        message = upload_message(argv[2]);
    }
    else if (!strcmp(argv[1], "-down"))
    {
        CHECK_ARGS(3, "The filename must be provided");
        message = download_message(argv[2]);
    }
    else if (!strcmp(argv[1], "-list"))
        message = list_message();
    else
        FATAL("Unknown action\n");

    LOG("Sending message\n");
    if (send_message(message, SERVER_PORT))
        FATAL("Error sending message\n");
    free(message);
    return 0;
}
