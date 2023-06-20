#include <sys/socket.h>
#include <string.h>
#include <errno.h>

#include "metrics.h"
#include "users.h"
#include "management_handler.h"
#include "manager_protocol.h"
#include "definitions.h"
#include "logger.h"
#include "args.h"

extern struct pop3_args pop3_args;

typedef void (*manager_cmd_handler)(struct manager_request *request, struct manager_response *response);

static void setResponseHeader(struct manager_request *request, struct manager_response *response);

static bool validateVersion(struct manager_request *request);

static bool validateToken(struct manager_request *request);

static bool validateType(struct manager_request *request);

static bool validateCommand(struct manager_request *request);

static bool validateArguments(struct manager_request *request);

static void get_users_handler(struct manager_request *request, struct manager_response *response);

static void get_historics_handler(struct manager_request *request, struct manager_response *response);

static void get_concurrents_handler(struct manager_request *request, struct manager_response *response);

static void get_sent_handler(struct manager_request *request, struct manager_response *response);

static void get_page_size_handler(struct manager_request *request, struct manager_response *response);

static void set_stop_handler(struct manager_request *request, struct manager_response *response);

static void set_add_user_handler(struct manager_request *request, struct manager_response *response);

static void set_remove_user_handler(struct manager_request *request, struct manager_response *response);

static void set_page_size_handler(struct manager_request *request, struct manager_response *response);

manager_cmd_handler get_handlers[] = {
        get_users_handler,
        get_historics_handler,
        get_concurrents_handler,
        get_sent_handler,
        get_page_size_handler,
};

manager_cmd_handler set_handlers[] = {
        set_stop_handler,
        set_add_user_handler,
        set_remove_user_handler,
        set_page_size_handler,
};

struct manager_state {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

    struct manager_request request;
    struct manager_response response;

    size_t response_len;
    uint8_t page_size;

    uint8_t buffer_read[UDP_BUFFER_SIZE], buffer_write[UDP_BUFFER_SIZE];
};

struct manager_state managerState;

void manager_receive(struct selector_key *key) {
    managerState.client_addr_len = sizeof(managerState.client_addr);
    managerState.page_size = managerState.page_size == 0 ? MAX_PAGE_SIZE : managerState.page_size;
    memset(managerState.buffer_read, 0, UDP_BUFFER_SIZE);
    memset(managerState.buffer_write, 0, UDP_BUFFER_SIZE);

    ssize_t read = recvfrom(key->fd, managerState.buffer_read, UDP_BUFFER_SIZE, 0,
                            (struct sockaddr *) &managerState.client_addr, &managerState.client_addr_len);

    if (read <= 0) {
        log(ERROR, "Error receiving message from manager: %s", strerror(errno))
    }

    if (manager_packet_to_request(managerState.buffer_read, &managerState.request) < 0) {
        log(ERROR, "Error parsing manager request")
        return;
    }

    setResponseHeader(&managerState.request, &managerState.response);

    if (managerState.response.status == SC_OK) {
        if (managerState.request.type == TYPE_GET) {
            get_handlers[managerState.request.cmd](&managerState.request, &managerState.response);
        } else if (managerState.request.type == TYPE_SET) {
            set_handlers[managerState.request.cmd](&managerState.request, &managerState.response);
        }
    }

    if (manager_response_to_packet(&managerState.response, managerState.buffer_write, &managerState.response_len) < 0) {
        log(ERROR, "Error parsing manager response")
        return;
    }

    if (sendto(key->fd, managerState.buffer_write, managerState.response_len, 0,
               (struct sockaddr *) &managerState.client_addr, managerState.client_addr_len) < 0) {
        log(ERROR, "Error sending manager message to client: %s", strerror(errno))
    }
}

static void setResponseHeader(struct manager_request *request, struct manager_response *response) {
    response->status = SC_OK;
    if (!validateVersion(request)) {
        response->status = SC_INVALID_VERSION;
    } else if (!validateToken(request)) {
        response->status = SC_INVALID_TOKEN;
    } else if (!validateType(request)) {
        response->status = SC_INVALID_TYPE;
    } else if (!validateCommand(request)) {
        response->status = SC_INVALID_CMD;
    } else if (!validateArguments(request)) {
        response->status = SC_INVALID_ARG;
    }
    response->version = MANAGER_VERSION_1;
    response->id = request->id;
    response->type = request->type;
    response->cmd = request->cmd;
}

static bool check_set_uint8(struct manager_request *request) {
    switch (request->cmd) {
        case SET_PAGE_SIZE:
            return MIN_PAGE_SIZE <= request->data.uint8_data && request->data.uint8_data <= MAX_PAGE_SIZE;
    }
    return true;
}

const char *strnchr(const char *str, int c, size_t n) {
    const char *end = str + n;
    while (str < end && *str != '\0' && *str != c) {
        str++;
    }
    return (str == end || *str != c) ? NULL : str;
}

static bool check_set_add_user(char *s) {
    // Find the position of the colon in the string
    // Since MAX_USERNAME accounts for '\0', it already takes care of the ':' character
    const char *colon = strnchr(s, ':', MAX_USERNAME + MAX_PASSWORD);
    if (colon == NULL) {
        // No colon found, invalid format
        return false;
    }

    // Calculate the lengths of the user and password strings
    size_t userLen = colon - s;
    size_t passwordLen = strnlen(colon + 1, MAX_PASSWORD);  // Skip the colon

    // Check the lengths against the maximum limits
    if (userLen >= MAX_USERNAME || passwordLen >= MAX_PASSWORD) {
        return false;
    }

    return true;
}

static bool check_set_string(struct manager_request *request) {
    switch (request->cmd) {
        case ADD_USER:
            return check_set_add_user(request->data.string);
        case DEL_USER:
            if (strlen(request->data.string) == 0 || strnlen(request->data.string, MAX_USERNAME) >= MAX_USERNAME) {
                return false;
            }
    }
    return true;
}

static bool validateVersion(struct manager_request *request) {
    return request->version == MANAGER_VERSION_1;
}

static bool validateToken(struct manager_request *request) {
    return request->token == pop3_args.manager_token;
}

static bool validateType(struct manager_request *request) {
    return request->type == TYPE_SET || request->type == TYPE_GET;
}

static bool validateCommand(struct manager_request *request) {
    switch (request->type) {
        case TYPE_SET:
            return request->cmd < SET_CMD_QTY;
        case TYPE_GET:
            return request->cmd < GET_CMD_QTY;
    }
}

static bool validateArguments(struct manager_request *request) {
    bool ret = true;
    switch (get_req_data_type(request->cmd, request->cmd)) {
        case UINT8_DATA:
            ret = check_set_uint8(request);
            break;
        case STRING_DATA:
            ret = check_set_string(request);
            break;
        default:
            break;
    }
    return ret;
}

static void get_users_handler(struct manager_request *request, struct manager_response *response) {
    size_t offset = (request->data.uint8_data - 1) * managerState.page_size;
    if (offset >= user_count()) {
        response->data.string[0] = '\0';
        return;
    }
    size_t string_offset = 0;
    for (int i = 0; i < managerState.page_size; ) {
        strcpy(response->data.string + string_offset, user_get_username(offset+i));
        string_offset += strlen(user_get_username(offset+i));
        response->data.string[string_offset++] = '\n';
        i++;
    }
}

static void get_historics_handler(struct manager_request *request, struct manager_response *response) {
    struct metrics metrics;
    getMetrics(&metrics);
    response->data.uint64_data = metrics.totalConnections;
}

static void get_concurrents_handler(struct manager_request *request, struct manager_response *response) {
    struct metrics metrics;
    getMetrics(&metrics);
    response->data.uint64_data = metrics.currentConnections;
}

static void get_sent_handler(struct manager_request *request, struct manager_response *response) {
    struct metrics metrics;
    getMetrics(&metrics);
    response->data.uint64_data = metrics.bytesSent;
}

static void get_page_size_handler(struct manager_request *request, struct manager_response *response) {
    response->data.uint8_data = managerState.page_size;
}

extern bool terminate;

static void set_stop_handler(struct manager_request *request, struct manager_response *response) {
    terminate = true;
    response->status = SC_OK;
}

static void set_add_user_handler(struct manager_request *request, struct manager_response *response) {
    if (user_count() >= MAX_USERS) {
        response->status = SC_USERS_FULL;
        return;
    }
    if (user_add_basic(request->data.string)) {
        response->status = SC_OK;
    } else {
        response->status = SC_USER_ALREADY_EXISTS;
    }
}

static void set_remove_user_handler(struct manager_request *request, struct manager_response *response) {
    if (user_remove(request->data.string)) {
        response->status = SC_OK;
    } else {
        response->status = SC_USER_NOT_FOUND;
    }
}

static void set_page_size_handler(struct manager_request *request, struct manager_response *response) {
    managerState.page_size = request->data.uint8_data;
}
