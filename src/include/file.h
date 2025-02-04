#ifndef FILE_H
#define FILE_H

#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include "message.h"
#include "b64.h"
#include <sys/stat.h>
#include <unistd.h>
#include "common.h"

char *retrieve_downloadable_filenames(char *directory)
{
    int filesLength = 1024;
    int fileCount = 0;
    char *files = malloc(filesLength);
    files[0] = '\0';
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(directory)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_name[0] != '.')
            {
                fileCount++;
                while (strlen(files) + strlen(ent->d_name) + 1 > filesLength)
                {
                    filesLength *= 2;
                    char *tmp = (char *)realloc(files, filesLength);
                    if (tmp != NULL)
                        files = tmp;
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
        if (fileCount == 0)
            strcpy(files, "No file downloadable\n");
    }
    else
        strcpy(files, "No file downloadable\n");
    return files;
}

long get_file_content(char **content, char *filename)
{
    long file_size;
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        ERROR("File not found. The file %s does not seem to exist\n", filename);
        return -1;
    }
    if (fseek(file, 0L, SEEK_END) != 0)
    {
        ERROR("Can't go to the end of the file");
        return -1;
    }

    file_size = ftell(file);
    if (file_size == -1)
    {
        ERROR("Failed to obtain file size");
        return -1;
    }
    *content = malloc(sizeof(char) * (file_size + 1));
    if (fseek(file, 0L, SEEK_SET) != 0)
    {
        ERROR("Failed to return at the start of file when uploading message");
        return -1;
    }
    size_t newLen = fread(*content, sizeof(char), file_size, file);
    if (ferror(file) != 0)
    {
        ERROR("Error while reading file");
        return -1;
    }

    fclose(file);
    return file_size;
}

int create_file_from_message(MESSAGE *message, char *path_to_download)
{
    struct stat st = {0};
    if (stat(path_to_download, &st) == -1)
        mkdir(path_to_download, 0700);

    char *filename = basename(message->filename);
    size_t path_len = strlen(path_to_download) + strlen(filename) + 1;
    char *filepath = malloc(path_len);
    snprintf(filepath, path_len, "%s%s", path_to_download, filename);
    FILE *file = fopen(filepath, "wb");
    if (file == NULL)
    {
        ERROR("Can't write file\n");
        return -1;
    }
    free(filepath);
    size_t decoded_size;
    char *buffer = (char *)b64_decode_ex(message->content, strlen(message->content), &decoded_size);
    size_t written = fwrite(buffer, sizeof(char), decoded_size, file);
    fclose(file);
    free(buffer);
    return 0;
}

MESSAGE *create_message_from_file(void *content, long content_size, char *filename, ACTION_TYPE action_type)
{
    char *buffer_64 = b64_encode((unsigned char *)content, content_size);
    MESSAGE *msg = (MESSAGE *)malloc(sizeof(MESSAGE) + strlen(buffer_64) + 1);
    msg->action_type = action_type;
    char *end = stpncpy(msg->filename, filename, sizeof(msg->filename));
    bzero(end, msg->content - end);
    strcpy(msg->content, buffer_64);
    free(buffer_64);
    return msg;
}

#endif
