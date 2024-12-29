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

int main(int argc, char **argv)
{
	printf("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);

	while (1)
	{
		MESSAGE *message = read_full_message();
		printf("Received [%c]: %s\n", message->action_type, message->content);
		free(message);
	}
}
