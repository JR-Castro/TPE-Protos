#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "selector.h"

#define SERVICE "pop3"
#define DEFAULT_PORT "1100"
#define PORTNUM 1100
#define MAX_CON 3
#define SELECTOR_SIZE 1024


static int setupSocket();

int main(const int argc, const char ** argv) {

    int ret = 0;

    close(STDIN_FILENO);

    int serverSocket = setupSocket();

    const struct selector_init init = {
            .signal = SIGALRM,
            .select_timeout = {
                    .tv_sec = 10,
                    .tv_nsec = 0,
            },
    };

    fd_selector selector = NULL;

    if ((ret = selector_init(&init)) != SELECTOR_SUCCESS) goto finally;

    selector = selector_new(SELECTOR_SIZE);
    if (selector == NULL) goto finally;

    // Make handlers

    // Register fd's

    // Loop

finally:
    if (serverSocket != -1) close(serverSocket);
    if (selector != NULL) selector_destroy(selector);
    selector_close();
    return ret;
}

static int setupSocket() {

    struct sockaddr_in6 serveraddr;

    int newSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    if (newSocket < 0) goto handle_error;

    if (setsockopt(newSocket,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &(int) {1},
                   sizeof(int))){
        // TODO: Log
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin6_port = htons(PORTNUM);
    // This allows connections from any IPv4 or IPv6 client that specifies the correct port.
    serveraddr.sin6_family = AF_INET6;
    serveraddr.sin6_addr = in6addr_any;

    if (bind(newSocket,
             (struct sockaddr *)&serveraddr,
            sizeof(serveraddr))) goto handle_error;

    if (listen(newSocket, MAX_CON)) goto handle_error;

    return newSocket;

    handle_error:
    // TODO: Log errors

    close(newSocket);

    return -1;
}
