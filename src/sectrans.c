#include <stdio.h>
#include "client.h"
#include "server.h"
#include <signal.h>
#include <string.h>

static volatile int keepRunning = 1;

void stopServer(int _) {
    stopserver();
}

int main(int argc, char** argv)
{
	if(argc < 2) {
		printf("Missing argument\n");
		return 1;
	}

	if(!strcmp(argv[1], "server")){
		printf("Starting server\n");
   	//signal(SIGINT, stopServer);
		startserver(1024);

		char buffer[1024];
		while(!getmsg(buffer)){
			printf("%s\n", buffer);
		}
	}
	else if(!strcmp(argv[1], "client")){
		printf("Starting client\n");
		sndmsg("Salut !", 1024);
	}
   return 0;
}
