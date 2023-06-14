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


#endif //TPE_PROTOS_POP3_FILES_H
