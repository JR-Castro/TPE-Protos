#ifndef TPE_PROTOS_POP3_H
#define TPE_PROTOS_POP3_H

#include <stdint.h>

#include "selector.h"
#include "buffer.h"
#include "stm.h"

// TODO: Define a proper buffer size
#define BUFFER_SIZE 4096

struct client_data {
    struct sockaddr_storage addr;
    socklen_t addrLen;

    struct buffer inputBuffer;
    uint8_t inputBufferData[BUFFER_SIZE];
    struct buffer outputBuffer;
    uint8_t outputBufferData[BUFFER_SIZE];

    struct state_machine stm;
};

enum pop3_state {
    POP3_GREETING_READ = 0,
    POP3_GREETING_WRITE,
    POP3_AUTH_READ,
    POP3_AUTH_WRITE,
    POP3_TRANSACTION_READ,
    POP3_TRANSACTION_WRITE,
    POP3_UPDATE,
    POP3_CLOSE
};

void passiveAccept(struct selector_key *key);

#endif //TPE_PROTOS_POP3_H
