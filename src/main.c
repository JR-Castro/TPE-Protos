// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>

#include "include/selector.h"
#include "include/pop3.h"
#include "include/args.h"
#include "include/netutils.h"

#define SERVICE "pop3"
#define DEFAULT_PORT "1100"
#define DEFAULT_PORT_NUM 1100
#define MAX_CON 3
#define SELECTOR_SIZE 1024

extern struct pop3_args pop3_args;

int ipv4_server = -1;
int ipv6_server = -1;
int ipv4_admin = -1;
int ipv6_admin = -1;

// skipcq: CXX-W2009
static bool terminate = false;

static int setupSocket(char *addr, int port, unsigned protocol, bool is_ipv6);

static void sigterm_handler(const int signal) {
    // TODO: Log
    terminate = true;
}

int main(const int argc, char **argv) {

    int ret = 0;

    parse_args(argc, argv, &pop3_args);

    close(STDIN_FILENO);


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

    if (pop3_args.pop3_addr == NULL) {
        ipv6_server = setupSocket("::", pop3_args.pop3_port, IPPROTO_TCP, true);
        ipv4_server = setupSocket("0.0.0.0", pop3_args.pop3_port, IPPROTO_TCP, false);

    } else if (is_ipv6(pop3_args.pop3_addr)) {
        ipv6_server = setupSocket(pop3_args.pop3_addr, pop3_args.pop3_port, IPPROTO_TCP, true);

    } else {
        ipv4_server = setupSocket(pop3_args.pop3_addr, pop3_args.pop3_port, IPPROTO_TCP, false);
    }

    if (ipv4_server == -1 && ipv6_server == -1)
        goto finally;

    // TODO log

    if (pop3_args.mng_addr == NULL) {
        ipv4_admin = setupSocket("127.0.0.1", pop3_args.mng_port, IPPROTO_UDP, false);
        ipv6_admin = setupSocket("::1", pop3_args.mng_port, IPPROTO_UDP, true);

    } else if (is_ipv6(pop3_args.mng_addr)) {
        ipv6_admin = setupSocket(pop3_args.mng_addr, pop3_args.mng_port, IPPROTO_UDP, true);

    } else {
        ipv4_admin = setupSocket(pop3_args.mng_addr, pop3_args.mng_port, IPPROTO_UDP, false);
    }

    if (ipv4_admin == -1 && ipv6_admin == -1)
        goto finally;

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);


    if ((ipv4_server != -1 && selector_fd_set_nio(ipv4_server) == -1) ||
        (ipv6_server != -1 && selector_fd_set_nio(ipv6_server) == -1) ||
        (ipv4_admin != -1 && selector_fd_set_nio(ipv4_admin) == -1) ||
        (ipv6_admin != -1 && selector_fd_set_nio(ipv6_admin) == -1)) {

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

    if (ipv4_server != -1) {
        selectStatus = selector_register(selector, ipv4_server, &passiveHandler, OP_READ, NULL);
        if(selectStatus != SELECTOR_SUCCESS) {
            goto finally;
        }
    }
    if (ipv6_server != -1) {
        selectStatus = selector_register(selector, ipv6_server, &passiveHandler, OP_READ, NULL);
        if(selectStatus != SELECTOR_SUCCESS) {
            goto finally;
        }
    }

    if (ipv4_admin != -1) {
        selectStatus = selector_register(selector, ipv4_admin, &passiveHandler, OP_READ, NULL);
        if(selectStatus != SELECTOR_SUCCESS) {
            goto finally;
        }
    }

    if (ipv6_admin != -1) {
        selectStatus = selector_register(selector, ipv6_admin, &passiveHandler, OP_READ, NULL);
        if(selectStatus != SELECTOR_SUCCESS) {
            goto finally;
        }
    }


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

    if(ipv4_server >= 0) {
        close(ipv4_server);
    }
    if(ipv6_server >= 0) {
        close(ipv6_server);
    }
    if(ipv4_admin >= 0) {
        close(ipv4_admin);
    }
    if(ipv6_admin >= 0) {
        close(ipv6_admin);
    }

    return ret;
}


static int setupSocket(char *addr, int port, unsigned protocol, bool is_ipv6) {

    struct sockaddr_in6 serveraddr;

    if(port < 0){
        //TODO log -> invalid port
        return -1;
    }

    //chequeo de addr

    int domain, type;

    if(is_ipv6){
        domain = AF_INET6;

    } else {
        domain = AF_INET;
    }

    if(protocol == IPPROTO_TCP){
        type = SOCK_STREAM;
    } else {
        type = SOCK_DGRAM;
    }

    int newSocket = socket(domain, type, protocol);

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

