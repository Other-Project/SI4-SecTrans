#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "server.h"
#include "common.h"

void stopServer(int _)
{
	int error = stopserver();
	if (error)
		printf("Failed to gracefully stop the server\n");
	exit(error);
}

void handle_upload_message(MESSAGE *message)
{
	printf("Received upload request: %s\n", message->content);
}

void handle_download_message(MESSAGE *message)
{
	printf("Received download request for file %s\n", message->content);
}

void handle_list_message(MESSAGE *message)
{
	printf("Received list request\n");
}

int main(int argc, char **argv)
{
	printf("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);

	while (1)
	{
		MESSAGE *message = read_full_message();
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
			printf("Received unknown action type %c\n", message->action_type);
			break;
		}
		free(message);
	}
}
