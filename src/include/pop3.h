#ifndef TPE_PROTOS_POP3_H
#define TPE_PROTOS_POP3_H

#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "selector.h"

struct client_data {
    struct sockaddr_storage addr;
    socklen_t addrLen;
};

void passiveAccept(struct selector_key *key);

#endif //TPE_PROTOS_POP3_H
