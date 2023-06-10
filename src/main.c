// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "selector.h"
#include "./include/logger.h"
#include "pop3.h"

#define SERVICE "pop3"
#define DEFAULT_PORT "1100"
#define DEFAULT_PORT_NUM 1100
#define MAX_CON 3
#define SELECTOR_SIZE 1024

// skipcq: CXX-W2009
static bool terminate = false;

static int setupSocket(void);

static void sigterm_handler(const int signal) {
    log(INFO, "Signal %d", signal);
    terminate = true;
}

int main(const int argc, const char **argv) {

    int ret = 0;

    close(STDIN_FILENO);
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    int serverSocket = setupSocket();

    const struct selector_init init = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec = 10,
                    .tv_nsec = 0,
            },
    };

    fd_selector selector = NULL;
    selector_status selectStatus = SELECTOR_SUCCESS;

    if ((ret = selector_init(&init)) != SELECTOR_SUCCESS) {
        goto finally;
    }

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) {
        goto finally;
    }

    // Make handlers

    const fd_handler passiveHandler = {
            .handle_read = passiveAccept,
            .handle_write = NULL,
            .handle_close = NULL,
            .handle_block = NULL,
    };

    const char *err_msg = NULL;

    // Register fd's

    selectStatus = selector_register(selector, serverSocket, &passiveHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS) {
        goto finally;
    }

    // Loop

    while (!terminate) {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS) { 
            goto finally;
        }
    }

    ret = 0;

    finally:
    if (selectStatus != SELECTOR_SUCCESS) {
        log(ERROR, "%s: %s", (err_msg == NULL) ? "" : err_msg,
            selectStatus == SELECTOR_IO
            ? strerror(errno)
            : selector_error(selectStatus));
        ret = 2;
    } else if (err_msg != NULL) {
        log(ERROR, "%s", err_msg)
        ret = 1;
    }
    if (selector != NULL) {
        selector_destroy(selector);
    }
    log(INFO, "%s", "Closing the selector");
    selector_close();

    if (serverSocket != -1) { 
        log(INFO, "%s", "Closing the server socket");
        close(serverSocket);
    }
    log(INFO, "%s", "Closing main");
    return ret;
}

static int setupSocket(void) {

    struct sockaddr_in6 serveraddr;

    const char *err_msg = NULL;

    int newSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket < 0) {
        err_msg = "Socket failed";
        goto handle_error;
    }

    if (setsockopt(newSocket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &(int) {1},
                   sizeof(int))) {
        // TODO: Log
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_port = htons(DEFAULT_PORT_NUM);
    serveraddr.sin6_family = AF_INET6; // This allows connections from any IPv4 or IPv6 client.
    serveraddr.sin6_addr = in6addr_any;

    if (bind(newSocket,
             (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)))
        goto handle_error;

    if (listen(newSocket, MAX_CON)) {
        goto handle_error;
    }

    if (selector_fd_set_nio(newSocket) == -1) {
        err_msg = "Getting socket flags";
        goto handle_error;
    }

    return newSocket;

    handle_error:
    // TODO: Log errors
    if (newSocket >= 0) {
        log(INFO, "%s", "Closing socket");
        close(newSocket);
    }
    

    close(newSocket);

    return -1;
}
