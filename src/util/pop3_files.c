#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "pop3.h"
#include "pop3_files.h"
#include "args.h"
#include "emalloc.h"

extern struct pop3_args pop3_args;

/*
 *  Appends the filename at the end of the path:
 *  - 'path' -> 'path/filename'
 *  - 'path/' -> 'path/filename'
 *  Returns -1 if the resulting path is too long, 0 otherwise
 */
static int get_file_path(char path[MAX_PATH_LENGTH], char *filename) {
    unsigned int length = strlen(pop3_args.pop3_directory);

    if (snprintf(path, MAX_PATH_LENGTH, "%s%s%s", pop3_args.pop3_directory,
                 pop3_args.pop3_directory[length-1] == '/' ? "" : "/",
                 filename) > MAX_PATH_LENGTH)
        return -1;
    return 0;
}

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

    if (get_file_path(directory, data->user.username) < 0)
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

            if (get_file_path(filepath, de->d_name) < 0)
                continue;

            fileArray[i].filename = malloc(strlen(de->d_name) + 1);
            strcpy(fileArray[i].filename, de->d_name);

            stat(filepath, &st);
            totalSize += st.st_size;

            fileArray[i].num = i;
            fileArray[i].deleted = false;
            fileArray[i].size = st.st_size;
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

void sync_to_maildrop(struct selector_key *key) {
    struct client_data *data = key->data;
    struct file_array_item *fa = data->fileArray;
    char directory[MAX_PATH_LENGTH];

    if (get_file_path(directory, data->user.username) < 0)
        return;

    int ret = 0;

    for (int i = 0; i < data->fileArraySize; ++i) {
        if (fa[i].deleted) {
            char filepath[MAX_PATH_LENGTH];

            if (get_file_path(filepath, fa[i].filename) < 0)
                continue;

            ret += unlink(filepath);
        }
    }

    if (ret) {
        errResponse(data, "some deleted messages not removed");
    } else {
        okResponse(data, "");
    }
}
