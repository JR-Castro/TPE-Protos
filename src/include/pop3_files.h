#ifndef TPE_PROTOS_POP3_FILES_H
#define TPE_PROTOS_POP3_FILES_H

#include "buffer.h"
#include "definitions.h"

struct file {
    int fd;
    struct buffer readBuffer;


    uint8_t readBufferData[FILE_BUFFER_SIZE];
};

struct file_array_item {
    char *filename;
    unsigned int num;
    unsigned int size;
    bool deleted;
};

/*
 *  Populates the file array and file array size fields in the client data associated
 *  with this key. Returns -1 if an error occurred, 0 otherwise
 */
int fill_file_array(struct selector_key *key);

/*
 *  Frees the memory allocated for the file array
 */
void destroy_file_array(struct selector_key *key);

void sync_to_maildrop(struct selector_key *key);

/**
 *  @brief Appends the filename at the end of the user maildir path:
 *  - 'path'  -> 'path/username/filename'
 *  - 'path/' -> 'path/username/filename'
 *  @param path The resulting path
 *  @param username The username
 *  @param filename The filename
 *  @return -1 if the resulting path is too long, 0 otherwise
 */
int get_file_path_user(char path[MAX_PATH_LENGTH], const char *username, const char *filename);

/**
 * @brief  Check if file is marked as deleted in file array
 * @return bool true if the file is deleted, false otherwise
 */
bool is_file_deleted(const struct selector_key *key, const char *filename, int *index);

#endif //TPE_PROTOS_POP3_FILES_H
