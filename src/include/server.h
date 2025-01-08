#ifndef SERVER_H
#define SERVER_H

/* send message (maximum size: 1024 bytes) */
int getmsg(char msg_read[MAX_PACKET_LENGTH]);

int startserver(int port);
int stopserver();

#endif