// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "selector.h"
#include "pop3.h"
#include "args.h"
#include "logger.h"
#include "management_handler.h"
#include "metrics.h"

#define MAX_CON 3
#define SELECTOR_SIZE 1024

extern struct pop3_args pop3_args;

int serverSocket = -1;
int managementSocket = -1;

// skipcq: CXX-W2009
static bool terminate = false;

static int setupSocket(char *addr, int port);
static int setupManagementSocket(char *addr, int port);

static void sigterm_handler(const int signal) {
    log(INFO, "Signal %d received. Shutting down...", signal);
    terminate = true;
}

int main(const int argc, char **argv) {
    parse_args(argc, argv, &pop3_args);

    close(STDIN_FILENO);

    int ret = -1;

    metrics_init();

    const struct selector_init init = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec = 10,
                    .tv_nsec = 0,
            },
    };

    fd_selector selector = NULL;
    selector_status selectStatus = selector_init(&init);
    if (selectStatus != SELECTOR_SUCCESS) goto finally;

    serverSocket = setupSocket((pop3_args.pop3_addr == NULL) ? "::" : pop3_args.pop3_addr,pop3_args.pop3_port);
    if (serverSocket == -1) goto finally;

    managementSocket = setupManagementSocket((pop3_args.mng_addr == NULL) ? "::1" : pop3_args.mng_addr,pop3_args.mng_port);
    if (managementSocket == -1) goto finally;

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    if ((serverSocket != -1 && selector_fd_set_nio(serverSocket) == -1) ||
        (managementSocket != -1 && selector_fd_set_nio(managementSocket) == -1)) {
        goto finally;
    }

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) goto finally;

    // Make handlers

    const fd_handler passiveHandler = {
            .handle_read = passiveAccept,
            .handle_write = NULL,
            .handle_close = NULL,
            .handle_block = NULL,
    };

    const fd_handler managerHandler = {.handle_read = manager_receive, .handle_write = NULL, .handle_close = NULL, .handle_block = NULL};

    // Register fd's
    selectStatus = selector_register(selector, serverSocket, &passiveHandler, OP_READ, NULL);
    if(selectStatus != SELECTOR_SUCCESS) goto finally;

    selectStatus = selector_register(selector, managementSocket, &managerHandler, OP_READ, NULL);
    if(selectStatus != SELECTOR_SUCCESS) goto finally;

    // Loop

    while (!terminate) {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS) goto finally;
    }

    ret = 0;

finally:
    if (selectStatus != SELECTOR_SUCCESS) {
        log(ERROR, "Error in selector: %s", selector_error(selectStatus));
        if (selectStatus == SELECTOR_IO) {
            log(ERROR, "More info: %s", strerror(errno));
        }
    }
    if (selector != NULL) selector_destroy(selector);
    selector_close();

    if(serverSocket >= 0) close(serverSocket);
    if(managementSocket >= 0) close(managementSocket);

    return ret;
}


static int setupSocket(char *addr, int port) {

    struct sockaddr_in6 serveraddr;
    int newSocket = -1;

    if(port < 0){
        log(ERROR, "Invalid port");
        return -1;
    }

    //chequeo de addr

    newSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket < 0) {
        log(ERROR, "Error creating socket: %s", strerror(errno));
        goto handle_error;
    }

    if (setsockopt(newSocket,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &(int) {1},
                        sizeof(int)) < 0) {
        log(INFO, "Error setting socket options: %s", strerror(errno));
    }


    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_port = htons(port);
    serveraddr.sin6_family = AF_INET6; // This allows connections from any IPv4 or IPv6 client.
    serveraddr.sin6_addr = in6addr_any;

    if (inet_pton(AF_INET6, addr, &serveraddr.sin6_addr) < 0) {
        log(ERROR, "Error parsing address: %s", strerror(errno));
        goto handle_error;
    }

    if (bind(newSocket,
             (struct sockaddr *) &serveraddr,
             sizeof(serveraddr))) {
        log(ERROR, "Error binding socket: %s", strerror(errno));
        goto handle_error;
    }

    if (listen(newSocket, MAX_CON)) {
        log(ERROR, "Error listening on socket: %s", strerror(errno));
        goto handle_error;
    }

    if (selector_fd_set_nio(newSocket) == -1) {
        log(ERROR, "Error setting socket to non-blocking: %s", strerror(errno));
        goto handle_error;
    }

    log(INFO, "Listening on %s:%d", addr, port)

    return newSocket;

    handle_error:

    if (newSocket != -1) close(newSocket);

    return -1;
}

static int setupManagementSocket(char *addr, int port) {
    struct sockaddr_in6 server_addr;
    int newSocket = -1;

    if(port < 0){
        log(ERROR, "Invalid port");
        return -1;
    }

    newSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (newSocket < 0) {
        log(ERROR, "Error creating management socket: %s", strerror(errno));
        goto handle_error;
    }

    if (setsockopt(newSocket,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &(int) {1},
                        sizeof(int)) < 0) {
        log(INFO, "Error setting management socket options: %s", strerror(errno));
    }

    if (setsockopt(newSocket,
                   IPPROTO_IPV6,
                   IPV6_V6ONLY,
                   &(int) {0},
                   sizeof(int)) < 0) {
        log(INFO, "Error setting management socket options: %s", strerror(errno));
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    server_addr.sin6_addr = in6addr_any;

    if (inet_pton(AF_INET6, addr, &server_addr.sin6_addr) <= 0) {
        log(ERROR, "Error parsing management address: %s", strerror(errno));
        goto handle_error;
    }

    if (bind(newSocket,
             (struct sockaddr *) &server_addr,
             sizeof(server_addr))) {
        log(ERROR, "Error binding management socket: %s", strerror(errno));
        goto handle_error;
    }

    log(INFO, "Listening on %s:%d for management", addr, port);

    return newSocket;
    handle_error:
    if (newSocket != -1) close(newSocket);
    return -1;
}

