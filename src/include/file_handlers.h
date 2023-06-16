#ifndef TPE_PROTOS_FILE_HANDLERS_H
#define TPE_PROTOS_FILE_HANDLERS_H

#include <stdint.h>

#include "buffer.h"
#include "selector.h"
#include "definitions.h"
#include "pop3.h"

struct file_data {
    struct buffer readBuffer;
    // TODO: Here goes file parser

    int clientFd;
    struct client_data *clientData;
    uint8_t readBufferData[FILE_BUFFER_SIZE];
};

const fd_handler* get_file_handlers();

#endif //TPE_PROTOS_FILE_HANDLERS_H