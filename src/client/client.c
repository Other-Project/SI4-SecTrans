#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <signal.h>
#include "file.h"
#include "server_message.h"
#include "client_message.h"

#define CHECK_ARGS(nb, msg) \
    if (argc < nb)          \
        FATAL("%s\n", msg);

void stopServer(int _)
{
    if (stopserver())
        FATAL("Failed to gracefully stop the server\n");
    exit(0);
}

MESSAGE *wait_for_response(ACTION_TYPE expected_type, ENCRYPTION_TOOLS *read_encryption_tools)
{
    MESSAGE *message = NULL;
    if (read_message((void**)&message, read_encryption_tools))
        FATAL("Couldn't read server response\n");
    
    if (message->action_type == expected_type)
        return message;
    else
    {
        ERROR("Received unexpected action type %c\n", message->action_type);
        free(message);
        return wait_for_response(expected_type, read_encryption_tools);
    }
}

MESSAGE *upload_message(char filename[])
{
    char *content = NULL;
    long buffer_size;
    if ((buffer_size = get_file_content(&content, filename)) == -1)
        FATAL("Failed to obtain file content\n");
    MESSAGE *msg = create_message_from_file(content, buffer_size, filename, UPLOAD);
    free(content);
    return msg;
}

void download_message(char filename[], ENCRYPTION_TOOLS *read_encryption_tools, ENCRYPTION_TOOLS *send_encryption_tools)
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + 5);
    msg->action_type = DOWNLOAD;
    strncpy(msg->filename, filename, sizeof(msg->filename));
    strcpy(msg->content, "5001");
    LOG("Sending message\n");
    signal(SIGINT, stopServer);
    if (send_message(msg, SERVER_PORT, send_encryption_tools))
        FATAL("Error sending message\n");
    free(msg);
    MESSAGE *response = wait_for_response(DOWNLOAD, read_encryption_tools);
    LOG("Received download message\n");
    if (strcmp(response->content, "File not found") == 0)
        ERROR("Failed to download file\nPlease check the filename\n");
    else if (create_file_from_message(response, DIRECTORY_CLIENT) == -1)
        ERROR("Failed to create file from message\n");
    free(response);
    stopServer(0);
}

void list_message(ENCRYPTION_TOOLS *read_encryption_tools, ENCRYPTION_TOOLS *send_encryption_tools)
{
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + 5);
    msg->action_type = LIST;
	bzero(msg->filename, sizeof(msg->filename));
    strcpy(msg->content, "5001");
    LOG("Sending message\n");
    if (send_message(msg, SERVER_PORT, send_encryption_tools))
        FATAL("Error sending message\n");
    free(msg);
    signal(SIGINT, stopServer);
    MESSAGE *response = wait_for_response(LIST, read_encryption_tools);
    printf("Files on server:\n\n%s", response->content);
    free(response);
    stopServer(0);
}

int main(int argc, char *argv[])
{
    CHECK_ARGS(2, "Missing action argument");

    startserver(CLIENT_PORT);
    ENCRYPTION_TOOLS read_encryption_tools;
    ENCRYPTION_TOOLS send_encryption_tools;

    if (sodium_init() < 0)
        FATAL("Failed to initialize sodium\n");

    HAND_SHAKE_MESSAGE *handshake_message = NULL;
    
    generate_encryption_tools(&send_encryption_tools);

    if (do_handshake_client(SERVER_PORT, CLIENT_PORT, &send_encryption_tools, &read_encryption_tools, handshake_message))
        FATAL("Failed to do handshake\n");

    if (!strcmp(argv[1], "-up"))
    {
        CHECK_ARGS(3, "The filename must be provided");
        MESSAGE *message;
        message = upload_message(argv[2]);
        LOG("Sending message\n");
        if (send_message(message, SERVER_PORT, &send_encryption_tools))
            FATAL("Error sending message\n");
        free(message);
    }
    else if (!strcmp(argv[1], "-down"))
    {
        CHECK_ARGS(3, "The filename must be provided");
        download_message(argv[2], &read_encryption_tools, &send_encryption_tools);
    }
    else if (!strcmp(argv[1], "-list"))
    {
        list_message(&read_encryption_tools, &send_encryption_tools);
    }
    else
        FATAL("Unknown action\n");
    return 0;
}
