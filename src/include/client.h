#ifndef CLIENT_H
#define CLIENT_H

/* send message (maximum size: 1024 bytes) */
int sndmsg(char msg[MAX_PACKET_LENGTH], int port);

int startserver(int port);
int stopserver();

#endif