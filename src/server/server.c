#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "server.h"
#include "common.h"
#include "file.h"

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

void handle_list_message(MESSAGE *messageReceived)
{
	LOG("Received list request\n");
	char *files = retrieve_downloadable_filenames(DIRECTORY_SERVER);
	MESSAGE *message = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(files) + 1);
	message->action_type = LIST;
	bzero(message->filename, sizeof(message->filename));
	strcpy(message->content, files);
	if (send_message(message, atoi(messageReceived->content)))
		ERROR("Error sending message\n");
	free(files);
	free(message);
}

int main(int argc, char **argv)
{
	LOG("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);
	while (1)
	{
		MESSAGE *message = NULL;
		if (read_message(&message))
		{
			ERROR("Couldn't read message\n");
			continue;
		}
		switch (message->action_type)
		{
		case UPLOAD:
			handle_upload_message(message);
			break;
		case DOWNLOAD:
			if (send_message(handle_download_message(message), atoi(message->content)))
				ERROR("Error sending message\n");
			break;
		case LIST:
			handle_list_message(message);
			break;
		default:
			ERROR("Received unknown action type %c\n", message->action_type);
			break;
		}
		free(message);
	}
}
