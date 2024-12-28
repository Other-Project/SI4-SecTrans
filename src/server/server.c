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

	char buffer[MAX_MSG_LENGTH];
	while (!getmsg(buffer))
	{
		printf("Received: %s\n", buffer);
	}
	return 0;
}
