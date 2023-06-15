#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/pop3.h"
#include "../include/pop3_files.h"
#include "../include/args.h"
#include "../include/emalloc.h"

extern struct pop3_args pop3_args;

int fill_file_array(struct selector_key *key) {
    struct client_data *data = key->data;
    struct file_array_item *file_array_item;
    struct dirent *de;
    struct stat st;

    char directory[MAX_PATH_LENGTH];
    char *path = pop3_args.pop3_directory;

    struct file_array_item *fileArray = calloc(1, sizeof(struct file_array_item) * MAX_EMAILS);

    if (fileArray == NULL)
        return -1;

    unsigned int length = strlen(path);

    if (snprintf(directory, MAX_PATH_LENGTH, "%s%s%s", path,
                 path[length-1] == '/' ? "" : "/",
                 data->user.username) > MAX_PATH_LENGTH)
        return -1;

    DIR *dr = opendir(directory);
    int i = 0;
    unsigned int totalSize = 0;

    if (dr == NULL) {
        return -1;
    }

    while ((de = readdir(dr)) != NULL && i < MAX_EMAILS) {
        if (de->d_type == DT_REG) {
            char filepath[MAX_PATH_LENGTH];
            snprintf(filepath, MAX_PATH_LENGTH, "%s/%s", directory, de->d_name);

            fileArray[i].filename = malloc(strlen(de->d_name) + 1);
            strcpy(fileArray[i].filename, de->d_name);

            stat(filepath, &st);
            totalSize += st.st_size;

            fileArray[i].num = i;
            fileArray[i].deleted = false;
            i++;

        }
    }

    data->fileArray = fileArray;
    data->fileArraySize = i;
    data->totalMailSize = totalSize;

    closedir(dr);
    return 0;
}

void destroy_file_array(struct selector_key *key) {
    struct client_data *data = key->data;
    struct file_array_item *file_array_item;

    for (int i = 0; i < data->fileArraySize; i++) {
        file_array_item = &data->fileArray[i];
        free(file_array_item->filename);
    }

    free(data->fileArray);
}

