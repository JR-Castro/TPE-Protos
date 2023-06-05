// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "include/selector.h"
#include "pop3.h"
#include "include/args.h"

#define SERVICE "pop3"
#define DEFAULT_PORT "1100"
#define DEFAULT_PORT_NUM 1100
#define MAX_CON 3
#define SELECTOR_SIZE 1024

extern struct pop3_args pop3_args;

// skipcq: CXX-W2009
static bool terminate = false;

static int setupSocket(void);

static void sigterm_handler(const int signal) {
    // TODO: Log
    terminate = true;
}

int main(const int argc, char **argv) {

    int ret = 0;

//    parse_args(argc, argv, &pop3_args);

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

    if ((ret = selector_init(&init)) != SELECTOR_SUCCESS) goto finally;

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) goto finally;

    // Make handlers

    const fd_handler passiveHandler = {
            .handle_read = passiveAccept,
            .handle_write = NULL,
            .handle_close = NULL,
            .handle_block = NULL,
    };

    // Register fd's

    selectStatus = selector_register(selector, serverSocket, &passiveHandler, OP_READ, NULL);
    if (selectStatus != SELECTOR_SUCCESS) goto finally;

    // Loop

    while (!terminate) {
        selectStatus = selector_select(selector);
        if (selectStatus != SELECTOR_SUCCESS) goto finally;
    }

    ret = 0;

    finally:
    if (selectStatus != SELECTOR_SUCCESS) ret = 2;
    if (selector != NULL) selector_destroy(selector);
    selector_close();

    if (serverSocket != -1) close(serverSocket);
    return ret;
}

static int setupSocket(void) {

    struct sockaddr_in6 serveraddr;

    int newSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket < 0) goto handle_error;

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

    if (listen(newSocket, MAX_CON)) goto handle_error;

    if (selector_fd_set_nio(newSocket) == -1) goto handle_error;

    return newSocket;

    handle_error:
    // TODO: Log errors

    close(newSocket);

    return -1;


}

