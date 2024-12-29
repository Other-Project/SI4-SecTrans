#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "server.h"
#include "common.h"

void stopServer(int _)
{
	if(!stopserver()) printf("Failed to gracefully stop the server\n");
}

int main(int argc, char **argv)
{
	printf("Starting server\n");
	// signal(SIGINT, stopServer);
	startserver(SERVER_PORT);

	MESSAGE *message;
	while ((message = read_full_message()))
	{
		printf("Received [%c]: %s\n", message->action_type, message->content);
		free(message);
	}
	return 0;
}
