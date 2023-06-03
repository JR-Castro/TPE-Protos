#include <stdlib.h>
#include "pop3.h"
#include "netutils.h"

void passiveAccept(struct selector_key *key) {
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(key->fd, (struct sockaddr *) &clientAddr, &clientAddrLen);

    /* TODO: Ask if we should retry accept in these cases:
     * "For reliable operation the application should detect the network errors defined for
     * the protocol after accept() and treat them like EAGAIN by retrying.  In the case of TCP/IP,
     * these are ENETDOWN,  EPROTO,  ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH,
     * EOPNOTSUPP, and ENETUNREACH."
     * - man 2 accept
     */

    if (clientSocket == -1) {
        // TODO: Log error
        return;
    }

    // TODO: Client data and state

    struct client_data *data = calloc(1, sizeof(struct client_data));

    if (data == NULL) {
        // TODO: Log error
        close(clientSocket);
        return;
    }

    // I don't know if these will be useful
    data->addr = clientAddr;
    data->addrLen = clientAddrLen;

    // TODO: Client handlers

    selector_status selectorStatus = selector_register(key->s, clientSocket, NULL, OP_READ | OP_WRITE, data);

    if (selectorStatus != SELECTOR_SUCCESS) {
        // TODO: Log error
        close(clientSocket);
        free(data);
        return;
    }
}
