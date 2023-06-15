// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "include/selector.h"
#include "include/pop3.h"
#include "include/args.h"
#include "include/logger.h"

#define MAX_CON 3
#define SELECTOR_SIZE 1024

extern struct pop3_args pop3_args;

int serverSocket = -1;
int managementSocket = -1;

// skipcq: CXX-W2009
static bool terminate = false;

static int setupSocket(char *addr, int port);

static void sigterm_handler(const int signal) {
    log(INFO, "Signal %d received. Shutting down...", signal);
    terminate = true;
}

int main(const int argc, char **argv) {
    parse_args(argc, argv, &pop3_args);

    close(STDIN_FILENO);

    int ret = -1;


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

    managementSocket = setupSocket((pop3_args.mng_addr == NULL) ? "::" : pop3_args.mng_addr,pop3_args.mng_port);
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

    // Register fd's
    selectStatus = selector_register(selector, serverSocket, &passiveHandler, OP_READ, NULL);
    if(selectStatus != SELECTOR_SUCCESS) goto finally;

    selectStatus = selector_register(selector, managementSocket, &passiveHandler, OP_READ, NULL);
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
    int newSocket;

    if(port < 0){
        //TODO log -> invalid port
        log(ERROR, "Invalid port");
        return -1;
    }

    //chequeo de addr

    newSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket < 0) goto handle_error;

    if (setsockopt(newSocket,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        &(int) {1},
                        sizeof(int)) < 0) {
        goto handle_error;
    }


    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_port = htons(port);
    serveraddr.sin6_family = AF_INET6; // This allows connections from any IPv4 or IPv6 client.
    serveraddr.sin6_addr = in6addr_any;

    if (inet_pton(AF_INET6, addr, &serveraddr.sin6_addr) < 0) goto handle_error;

    if (bind(newSocket,
             (struct sockaddr *) &serveraddr,
             sizeof(serveraddr)))
        goto handle_error;

    if (listen(newSocket, MAX_CON)) goto handle_error;

    if (selector_fd_set_nio(newSocket) == -1) goto handle_error;

    return newSocket;

    handle_error:
    log(ERROR, "Error setting up socket, %s", strerror(errno));

    close(newSocket);

    return -1;
}

