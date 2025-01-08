#ifndef FILE_H
#define FILE_H

#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void retrieve_downloadable_filenames(char **files)
{
    int filesLength = 10;
    int fileCount = 0;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("../src/server/files")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (ent->d_name[0] != '.' && strcmp(ent->d_name, "..") != 0)
            {
                fileCount++;
                while (strlen(*files) + strlen(ent->d_name) + 1 > filesLength)
                {
                    filesLength *= 2;
                    char *tmp = (char *)realloc(*files, filesLength);
                    if (tmp != NULL)
                    {
                        *files = tmp;
                    }
                    else
                    {
                        free(files);
                        FATAL("Failed to reallocate memory\n");
                    }
                }
                strcat(*files, ent->d_name);
                strcat(*files, "\n");
            }
        }
        closedir(dir);
        if (fileCount == 0)
        {
            strcpy(*files, "No file downloadable\n");
        }
    }
    else
    {
        strcpy(*files, "No file downloadable\n");
    }
}

long get_file_content(char **content, char *filename)
{
    long buffer_size;
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
            *content = malloc(sizeof(char) * (buffer_size + 1));
            if (fseek(file, 0L, SEEK_SET) != 0)
            {
                ERROR("Failed to return at the start of file when uploading message");
                exit(1);
            }
            size_t newLen = fread(*content, sizeof(char), buffer_size, file);
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
        ERROR("File not found. The file %s does not seem to exist", filename);
        exit(1);
    }
    fclose(file);
    return buffer_size;
}

void create_file_from_message(MESSAGE *message, char *path_to_download)
{
    struct stat st = {0};
    if (stat(path_to_download, &st) == -1)
    {
        mkdir(path_to_download, 0700);
    }
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/%s", path_to_download, basename(message->filename));
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

#endif