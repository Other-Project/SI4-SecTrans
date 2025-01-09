#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "file.h"
#include "server_message.h"
#include "client_message.h"

void stopServer(int _)
{
	if (stopserver())
		FATAL("Failed to gracefully stop the server\n");
	exit(0);
}

void handle_upload_message(MESSAGE *message)
{
	LOG("Received upload request for file: %s\n", basename(message->filename));
	if (create_file_from_message(message, DIRECTORY_SERVER) == -1)
		ERROR("Failed to create file from message\n");
}

MESSAGE *handle_download_message(MESSAGE *message)
{
	LOG("Received download request for file: %s\n", basename(message->filename));
	char *content = NULL;
	char filename[1024];
	snprintf(filename, sizeof(filename), "%s%s", DIRECTORY_SERVER, basename(message->filename));
	long buffer_size = get_file_content(&content, filename);
	if (buffer_size < 0)
	{
		MESSAGE *messageNotFound = malloc(sizeof(MESSAGE) + sizeof("File not found"));
		messageNotFound->action_type = DOWNLOAD;
		bzero(messageNotFound->filename, sizeof(messageNotFound->filename));
		strcpy(messageNotFound->content, "File not found");
		ERROR("Failed to obtain file content\n");
		return messageNotFound;
	}

	MESSAGE *msg = create_message_from_file(content, buffer_size, filename, DOWNLOAD);
	free(content);
	return msg;
}

MESSAGE *handle_list_message(MESSAGE *messageReceived, ENCRYPTION_TOOLS *send_encryption_tools, int response_port)
{
	LOG("Received list request\n");
	char *files = retrieve_downloadable_filenames(DIRECTORY_SERVER);
	MESSAGE *message = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(files) + 1);
	message->action_type = LIST;
	bzero(message->filename, sizeof(message->filename));
	strcpy(message->content, files);
	free(files);
	return message;
}

int main(int argc, char **argv)
{
	LOG("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);

	ENCRYPTION_TOOLS read_encryption_tools;
    ENCRYPTION_TOOLS send_encryption_tools;

    if (sodium_init() < 0)
        FATAL("Failed to initialize sodium\n");

    HAND_SHAKE_MESSAGE *handshake_message = NULL;
    
    generate_encryption_tools(&send_encryption_tools);
	unsigned char *public_server_key = malloc(crypto_box_PUBLICKEYBYTES);
	memcpy(public_server_key, send_encryption_tools.public_key, crypto_box_PUBLICKEYBYTES);

	while (1)
	{
		memcpy(send_encryption_tools.public_key, public_server_key, crypto_box_PUBLICKEYBYTES);
		if(do_handshake_server(&send_encryption_tools, &read_encryption_tools, &handshake_message))
			FATAL("Failed to do handshake\n");

		MESSAGE *message = NULL;
		if (read_message((void**)&message, &read_encryption_tools))
		{
			ERROR("Couldn't read message\n");
			continue;
		}

		MESSAGE *response_message = NULL;
		switch (message->action_type)
		{
		case UPLOAD:
			handle_upload_message(message);
			break;
		case DOWNLOAD:
			response_message = handle_download_message(message);
			break;
		case LIST:
			response_message = handle_list_message(message, &send_encryption_tools, handshake_message->response_port);
			break;
		default:
			ERROR("Received unknown action type %c\n", message->action_type);
			break;
		}
		if (response_message && send_message(response_message, handshake_message->response_port, &send_encryption_tools))
				ERROR("Error sending message\n");
		free(response_message);
		free(message);
	}
}
