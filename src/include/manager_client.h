#ifndef TPE_PROTOS_MANAGER_CLIENT_H
#define TPE_PROTOS_MANAGER_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include "manager_protocol.h"

struct manager_command {
    char * name;
    char * description;
    char * msg;
    size_t params_num;
    bool (*request_builder)(struct manager_request *req, uint8_t *payload);
};

#endif // TPE_PROTOS_MANAGER_CLIENT_H
