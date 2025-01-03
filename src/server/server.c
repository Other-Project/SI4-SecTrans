#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include "server.h"
#include "common.h"

void stopServer(int _)
{
	if (stopserver())
		FATAL("Failed to gracefully stop the server\n");
	exit(0);
}

void handle_upload_message(MESSAGE *message)
{
	LOG("Received upload request for file: %s\n", message->filename);
	TRACE("\t%s\n", message->content);
	char filepath[1024];
	sprintf(filepath, "../src/server/files/%s", basename(message->filename));
	FILE *file = fopen(filepath, "w");
	if (file == NULL)
	{
		ERROR("Can't create file for uploading");
		exit(1);
	}
	size_t written = fwrite(message->content, sizeof(char), strlen(message->content), file);
	fclose(file);
}

void handle_download_message(MESSAGE *message)
{
	LOG("Received download request for file: %s\n", message->filename);
}

void handle_list_message(MESSAGE *message)
{
	LOG("Received list request\n");
}

int main(int argc, char **argv)
{
	LOG("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);

	while (1)
	{
		MESSAGE *message = read_message();
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
