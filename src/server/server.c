#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
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
	size_t decoded_size;
	char *buffer = (char *)b64_decode_ex(message->content, strlen(message->content), &decoded_size);
	size_t buffer_size = strlen(buffer);
	size_t written = fwrite(buffer, sizeof(char), decoded_size, file);
	fclose(file);
	free(buffer);
}

void handle_download_message(MESSAGE *message)
{
	LOG("Received download request for file: %s\n", message->filename);
	char *content = NULL;
	long buffer_size;
	char filename[1024];
	snprintf(filename, sizeof(filename), "../src/server/files/%s", message->filename);
	printf("Downloading file %s\n", filename);
	FILE *file = fopen(filename, "r");
	if (file != NULL)
	{
		if (fseek(file, 0L, SEEK_END) == 0)
		{
			buffer_size = ftell(file);
			if (buffer_size == -1)
			{
				ERROR("Failed to create buffer size when uploading message");
				exit(1);
			}
			content = malloc(sizeof(char) * (buffer_size + 1));
			if (fseek(file, 0L, SEEK_SET) != 0)
			{
				ERROR("Failed to return at the start of file when uploading message");
				exit(1);
			}
			size_t newLen = fread(content, sizeof(char), buffer_size, file);
			if (ferror(file) != 0)
			{
				ERROR("Error while reading file");
				exit(1);
			}
		}
		else
		{
			ERROR("Can't go to the end of the file");
			exit(1);
		}
	}
	else
	{
		ERROR("File not found\nThe given file %s\n does not seem to exist", message->filename);
		exit(1);
	}
	fclose(file);
	char *buffer_64 = b64_encode((unsigned char *)content, buffer_size);
	MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(buffer_64));
	msg->action_type = DOWNLOAD;
	strcpy(msg->filename, filename);
	strcpy(msg->content, buffer_64);
	free(content);
	free(buffer_64);
	if (send_message(msg, atoi(message->content)))
		FATAL("Error sending message\n");
}

void handle_list_message(MESSAGE *messageReceived)
{
	LOG("Received list request\n");
	int filesLength = 10;
	char *files = (char *)malloc(filesLength);
	files[0] = '\0';
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir("../src/server/files")) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (ent->d_name[0] != '.' && strcmp(ent->d_name, "..") != 0)
			{
				while (strlen(files) + strlen(ent->d_name) + 1 > filesLength)
				{
					filesLength *= 2;
					char *tmp = (char *)realloc(files, filesLength);
					if (tmp != NULL)
					{
						files = tmp;
					}
					else
					{
						free(files);
						FATAL("Failed to reallocate memory\n");
					}
				}
				strcat(files, ent->d_name);
				strcat(files, "\n");
			}
		}
		closedir(dir);
	}
	else
	{
		ERROR("Can't open directory\n");
	}
	MESSAGE *message = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(files));
	message->action_type = LIST;
	strcpy(message->content, files);
	if (send_message(message, atoi(messageReceived->content)))
		FATAL("Error sending message\n");
	free(files);
	free(message);
}

int main(int argc, char **argv)
{
	LOG("Starting server\n");
	signal(SIGINT, stopServer);
	startserver(SERVER_PORT);
	while (1)
	{
		MESSAGE *message = NULL;
		if (read_message(&message))
		{
			ERROR("Couldn't read message\n");
			continue;
		}
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
