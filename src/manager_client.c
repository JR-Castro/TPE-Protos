#include "manager_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <bits/types/struct_timeval.h>
#include <errno.h>

#include "netutils.h"
#include "logger.h"
#include "manager_protocol.h"
#include "definitions.h"
#include "emalloc.h"

#define TIMEOUT_SEC  5
#define INPUT_SIZE 100

/*****************************************************************
* STATIC PROTOTYPES
***/
static uint32_t get_auth_token(void);
static int parse_input(char **command, char **arg);
static char * get_error_message(int64_t status_code);
static int build_request(struct manager_request *req, const char* command, char* param);
static void process_response(struct manager_request req, struct manager_response rsp, char *msg);
static void set_request_header(struct manager_request *req, uint8_t type, uint8_t cmd);
static void print_help(void);
static int udpClientSocket(const char *host, const char *service, struct addrinfo *finalAddr);
// SET COMMAND REQUESTS
static bool set_add_user_request(struct manager_request *req, uint8_t *payload);
static bool set_del_user_request(struct manager_request *req, uint8_t *payload);
static bool set_terminate_request(struct manager_request *req, uint8_t *payload);
static bool set_page_size_request(struct manager_request *req, uint8_t *payload);
// GET COMMAND REQUESTS
static bool get_list_request(struct manager_request *req, uint8_t *payload);
static bool get_page_size_request(struct manager_request *req, uint8_t *payload);
static bool get_historic_request(struct manager_request *req, uint8_t *payload);
static bool get_concurrent_request(struct manager_request *req, uint8_t *payload);
static bool get_bytes_transf_request(struct manager_request *req, uint8_t *payload);
/*****************************************************************
* VARIABLES
***/
static struct manager_command mgmt_commands[] = {
        {"help", "Print usage/help info\n", NULL, 0, NULL},

        {"list", "Return the requested page of the user's directory\tUsage:\t list <page>\n",
              "User", 1,   get_list_request},

        {"get-page-size", "Return number of user per page (max 200)\nUsage:\t get-page-size\n",
                "User per page", 0, get_page_size_request},

        {"connections", "Return the number of concurrent connections in the server\nUsage:\t connections\n",
         "Concurrent connections now", 0, get_concurrent_request},

        {"bytes", "Return the number of bytes sent by the server\nUsage:\t bytes\n",
         "Bytes sent", 0, get_bytes_transf_request},

        {"historic", "Return the number of requests served by the server\nUsage:\t historic\n",
         "Requests served", 0, get_historic_request},

        {"add-user", "Add a new user to the server\nUsage:\t add-user <username>\n",
         "User", 1, set_add_user_request},

        {"del-user", "Delete a user from the server\nUsage:\t del-user <username>\n",
         "User", 1, set_del_user_request},

        {"set-page-size", "Set the number of users per page (max 200)\nUsage:\t set-page-size <page_size>\n",
         "User per page", 1, set_page_size_request},

        {"stop", "Stop the server\nUsage:\t stop\n",
         NULL, 0, set_terminate_request},
};

static struct manager_request request;
static struct manager_response response;
uint16_t trace_id = 1;
uint32_t token;

/*****************************************************************
* MAIN FUNCTION
***/
int main(int argc, const char *argv[]) {
    int sockfd = -1;
    int port;
    int af = AF_INET;
    char *command, *param;
    uint8_t buffin[BUFFER_SIZE], buffout[BUFFER_SIZE];
    if (argc != 3) {
        log(ERROR, "Usage:\t %s <server_addr> <server_port>", argv[0])
        goto failure;
    }
    token = get_auth_token();

    struct addrinfo serverAddr;
    memset(&serverAddr, 0, sizeof(struct addrinfo));

    /*if ((port = htons((int)strtol(argv[2], NULL, 10))) <= 0) {
        log(ERROR, "Invalid port");
        goto failure;
    }

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr.s_addr) > 0) {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = port;
        af = AF_INET;
    } else if (inet_pton(AF_INET6, argv[1], &serv_addr6.sin6_addr) > 0) {
        serv_addr6.sin6_family = AF_INET6;
        serv_addr6.sin6_port = port;
        af = AF_INET6;
    } else {
        log(ERROR, "Invalid address")
        goto failure;
    }*/

    if ((sockfd = udpClientSocket(argv[1], argv[2], &serverAddr)) < 0) {
        log(ERROR, "Unable to create socket\n")
        goto failure;
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        log(ERROR, "Failed manager client setsockopt")
        goto failure;
    }

    while (true) {
        command = param = NULL;
        if (parse_input(&command, &param) < 0) { continue; }

        if (strcmp(command, "help") == 0) {
            print_help();
            continue;
        }

        int command_idx = build_request(&request, command, param);
        if (command_idx < 0) {
            printf("Invalid command.\n");
            continue;
        }

        socklen_t socklen;
        size_t request_size = 0;
        memset(buffin, 0, BUFFER_SIZE);
        memset(buffout, 0, BUFFER_SIZE);
        struct sockaddr_storage fromAddr;
        socklen_t fromAddrLen = sizeof(fromAddr);

        if (manager_request_to_packet(&request, buffout, &request_size) < 0) {
            log(ERROR, "converting request to raw packet")
            continue;
        }

        sendto(sockfd, buffout, request_size, MSG_CONFIRM,
               serverAddr.ai_addr, serverAddr.ai_addrlen);

        if (recvfrom(sockfd, (char *)buffin,
                     BUFFER_SIZE, MSG_WAITALL,
                     (struct sockaddr *)&fromAddr, &fromAddrLen) < 0) {
            printf("Destination unreachable.\n"); // Timeout
            continue;
        }

        if (manager_packet_to_response(buffin, &response) < 0) {
            log(ERROR, "converting raw packet to response")
            continue;
        }

        // response handler
        process_response(request, response, mgmt_commands[command_idx].msg);

    }

failure:
    log(ERROR, "Client disconnected due to an error")
    close(sockfd);
    exit(EXIT_FAILURE);
}

/*****************************************************************
* STATIC FUNCTIONS
***/
static uint32_t get_auth_token(void) {
    const char *env_token = getenv(TOKEN);
    if (env_token == NULL || strlen(env_token) != TOKEN_BYTES) {
        log(ERROR, "Missing or invalid token environment variable")
        return 0;
    }

    return strtoul(env_token, NULL, 10);
}

static int parse_input(char **command, char **arg) {
    char input[INPUT_SIZE] = {0};
    if (fgets(input, INPUT_SIZE, stdin) == NULL || *input == 0) {
        printf("Empty or Invalid command\n");
        return -1;
    }

    // Remove newline character from input
    input[strcspn(input, "\r\n")] = '\0';

    // Split command and argument
    char *space = strchr(input, ' ');
    if (space != NULL) {
        *space = '\0';
        *arg = malloc(INPUT_SIZE);
        strncpy(*arg, space + 1, INPUT_SIZE - (space - input));
    } else {
        *arg = NULL;
    }

    *command = malloc(INPUT_SIZE);
    strncpy(*command, input, INPUT_SIZE);

    return 0;
}

static int build_request(struct manager_request *req, const char* command, char* param) {
    int idx = -1;
    size_t cmds_len = N(mgmt_commands);
    for (int i = 0; i < cmds_len; i++) {
        if (strcmp(command, mgmt_commands[i].name) == 0
        && (mgmt_commands[i].params_num != 0 && param != NULL ||
            mgmt_commands[i].params_num == 0 && param == NULL  )) {
            mgmt_commands[i].request_builder(req, (uint8_t*)param);
            idx = i;
            break;
        }
    }
    return idx;
}

static void process_response(struct manager_request req, struct manager_response rsp, char *msg) {
    int8_t sc = (req.id != rsp.id) ? -1 : rsp.status;
    if (sc != SC_OK) {
        printf("[Error] %s.\n", get_error_message(sc));
        return;
    }

    switch (get_res_data_type(rsp.type, rsp.cmd)) {
        case EMPTY_DATA:
            printf("done\n");
            break;
        case STRING_DATA:
            printf("%s:\n%s\n", msg, rsp.data.string);
            break;
        case UINT64_DATA:
            printf("%s: %lu\n", msg, rsp.data.uint64_data);
            break;
        case UINT16_DATA:
            printf("%s: %d\n", msg, rsp.data.uint16_data);
            break;
        case UINT8_DATA:
            printf("%s: %d\n", msg, rsp.data.uint8_data);
            break;
        default:
            printf("%s\n", msg);
            break;
    }
}

static char * get_error_message(int64_t status_code) {
    switch (status_code) {
        case -1:
            return "Request-Response ID mismatch";
        case SC_INVALID_VERSION:
            return "Invalid version";
        case SC_INVALID_TOKEN:
            return "Invalid token";
        case SC_INVALID_TYPE:
            return "Invalid type";
        case SC_INVALID_CMD:
            return "Invalid command";
        case SC_INVALID_ARG:
            return "Invalid argument";
        case SC_USERS_FULL:
            return "Can't add more users. Not enough space";
        case SC_USER_ALREADY_EXISTS:
            return "User already exists";
        case SC_USER_NOT_FOUND:
            return "No such user";
        case SC_SERVER_ERROR:
            return "Server error";
        default:
            break;
    }
    return "Something went wrong";
}

static int udpClientSocket(const char *host, const char *service, struct addrinfo *finalAddr) {
    struct addrinfo addrCriteria;
    struct addrinfo *servAddr;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
    if (rtnVal != 0) {
        log(ERROR, "getaddrinfo() failed: %s", gai_strerror(rtnVal))
        return -1;
    }

    int sock = -1;

    for (struct addrinfo *addr = servAddr; addr != NULL && sock == -1; addr = addr->ai_next) {
        errno = 0;
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (sock < 0) {
            // TODO: Use sockaddr_to_human_buffered
            log(DEBUG, "Can't create socket on one address")
            continue;
        }

        if (connect(sock, addr->ai_addr, addr->ai_addrlen) < 0) {
            log(DEBUG, "Can't connect to one address")
            close(sock);
            sock = -1;
        } else {
            memcpy(finalAddr, addr, sizeof(struct addrinfo));
            finalAddr->ai_next = NULL;
        }
    }
    freeaddrinfo(servAddr);
    return sock;
}

// REQUEST HEADER BUILDER
static void set_request_header(struct manager_request *req, uint8_t type, uint8_t cmd) {
    req->id = trace_id++;
    req->version = MANAGER_VERSION_1;
    req->token = token;
    req->type = type;
    req->cmd = cmd;
}

// SET COMMAND REQUESTS
static bool set_add_user_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_SET, ADD_USER);
    if (*payload == PWD_DELIMETER) { return false; }

    char *creds = strchr((char*)payload, PWD_DELIMETER);
    if (creds == NULL        || strlen(creds) > (MAX_USERNAME + MAX_PASSWORD)
        || *(creds++) == '\0' || strlen(creds) > MAX_PASSWORD) {
        return false;
    }
    strcpy((char*)req->data.string, (char*)payload);
    return true;
}

static bool set_del_user_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_SET, DEL_USER);
    if (payload == NULL || strlen((char*)payload) > MAX_USERNAME)
        return false;
    strcpy((char*)req->data.string, (char*)payload);
    return true;
}

static bool set_terminate_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_SET, STOP_SERVER);
    return true;
}

static bool set_page_size_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_SET, SET_PAGE_SIZE);
    uint8_t page_size = strtol((char*)payload, NULL, 10);
    if (page_size >= MIN_PAGE_SIZE && page_size <= MAX_PAGE_SIZE) {
        req->data.uint8_data = page_size;
        return true;
    }
    return false;
}

// GET COMMAND REQUESTS
static bool get_list_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_GET, GET_USERS);
    uint8_t size = strtol((char*)payload, NULL, 10);
    if (size <= 0) { return false; }
    req->data.uint8_data = size;
    return true;
}

static bool get_page_size_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_GET, GET_PAGE_SIZE);
    return true;
}

static bool get_historic_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_GET, GET_HISTORIC);
    return true;
}

static bool get_concurrent_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_GET, GET_CONCURRENT);
    return true;
}

static bool get_bytes_transf_request(struct manager_request *req, uint8_t *payload) {
    set_request_header(req, TYPE_GET, GET_SENT);
    return true;
}

static void print_help(void) {
    printf("====================================== HELP ================================================\n");
    for (int i = 0; i < N(mgmt_commands); i++) {
        printf("> %-*s\t| %s", 15, mgmt_commands[i].name, mgmt_commands[i].description);
        printf("============================================================================================\n");
    }
}