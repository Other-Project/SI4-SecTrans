#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
	LOG("Received upload request for file: %s\n", message->filename);
	create_file_from_message(message, "../src/server/files/");
}

void handle_download_message(MESSAGE *message)
{
	LOG("Received download request for file: %s\n", message->filename);
	char *content = NULL;
	char filename[1024];
	snprintf(filename, sizeof(filename), "../src/server/files/%s", message->filename);
	long buffer_size = get_file_content(&content, filename);
	char *buffer_64 = b64_encode((unsigned char *)content, buffer_size);
	MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(buffer_64));
	msg->action_type = DOWNLOAD;
	strcpy(msg->filename, filename);
	strcpy(msg->content, buffer_64);
	free(content);
	free(buffer_64);
	if (send_message(msg, atoi(message->content)))
		FATAL("Error sending message\n");
}

void handle_list_message(MESSAGE *messageReceived)
{
	LOG("Received list request\n");
	char *files = (char *)malloc(10);
	files[0] = '\0';
	retrieve_downloadable_filenames(&files);
	MESSAGE *message = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(files));
	message->action_type = LIST;
	strcpy(message->content, files);
	if (send_message(message, atoi(messageReceived->content)))
		FATAL("Error sending message\n");
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
			handle_download_message(message);
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
