#include <stdio.h>
#include <stdlib.h>

#ifndef COMMON_H
#define COMMON_H

#define SERVER_PORT 5000
#define CLIENT_PORT 5001
#define MAX_PACKET_LENGTH 1024
#define MAX_FILENAME_LENGTH 256
#define DIRECTORY_SERVER "files/"
#define DIRECTORY_CLIENT "downloaded_files/"

#ifdef NDEBUG
#define TRACE(...)
#else
#define TRACE(...) printf(__VA_ARGS__)
#endif
#define LOG(...) printf(__VA_ARGS__)
#define ERROR(...) fprintf(stderr, __VA_ARGS__)
#define FATAL(...)                    \
	{                                 \
		fprintf(stderr, __VA_ARGS__); \
		exit(1);                      \
	}

#define CEIL_DIV(x, y) ((x) / (y) + ((x) % (y) != 0))

#define MAX_SIZE_BEFORE_B64(size_after) ((size_after) / 4 * 3 - 4)

#endif
