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
	struct stat st = {0};
	if (stat("../src/server/files", &st) == -1)
	{
		mkdir("../src/server/files", 0700);
	}
	char filepath[1024];
	snprintf(filepath, sizeof(filepath), "../src/server/files/%s", basename(message->filename));
	FILE *file = fopen(filepath, "wb");
	if (file == NULL)
	{
		ERROR("Can't create file for uploading");
		exit(1);
	}
	size_t decoded_size = (strlen(message->content)) * 3 / 4 + 1;
	char *buffer = (char *)b64_decode(message->content, MAX_DECODED_SIZE);
	size_t buffer_size = strlen(buffer);
	size_t written = fwrite(buffer, sizeof(char), (buffer_size < decoded_size && (decoded_size - buffer_size) < 4) ? (buffer_size) : (decoded_size), file);
	fclose(file);
	free(buffer);
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
