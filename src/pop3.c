// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <sys/socket.h>
#include "pop3.h"

// TODO: Handlers to parse and execute actions
static const struct state_definition client_states[] = {
    {
        .state = POP3_GREETING_READ,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_GREETING_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_AUTH_READ,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_AUTH_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_TRANSACTION_READ,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_TRANSACTION_WRITE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_UPDATE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    },
    {
        .state = POP3_CLOSE,
        .on_arrival = NULL,
        .on_departure = NULL,
        .on_read_ready = NULL,
        .on_write_ready = NULL,
        .on_block_ready = NULL,
    }};

static void closeConnection(struct selector_key *key) {
    struct client_data *data = key->data;

    // TODO: Log disconnection

    selector_unregister_fd(key->s, key->fd);
    close(key->fd);

    free(data);
}

/* Handlers for the selector events, so we can wrap the state machine calls */
static void pop3Read(struct selector_key *key);
static void pop3Write(struct selector_key *key);
static void pop3Block(struct selector_key *key);
static void pop3Close(struct selector_key *key);
static fd_handler pop3Handlers = {
    .handle_read = pop3Read,
    .handle_write = pop3Write,
    .handle_block = pop3Block,
    .handle_close = pop3Close,
};

void pop3Read(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    stm_handler_read(stm, key);
}

void pop3Write(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    stm_handler_write(stm, key);
}

void pop3Block(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    stm_handler_block(stm, key);
}

void pop3Close(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    stm_handler_close(stm, key);
    closeConnection(key);
}


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

    // I don't know if this will be useful
    data->addr = clientAddr;
    data->addrLen = clientAddrLen;

    buffer_init(&data->inputBuffer, BUFFER_SIZE, data->inputBufferData);
    buffer_init(&data->outputBuffer, BUFFER_SIZE, data->outputBufferData);

    data->stm.initial = POP3_GREETING_READ;
    data->stm.max_state = POP3_CLOSE;
    data->stm.states = client_states;

    stm_init(&data->stm);

    // TODO: Client handlers

    selector_status selectorStatus = selector_register(key->s, clientSocket, &pop3Handlers, OP_READ | OP_WRITE, data);

    if (selectorStatus != SELECTOR_SUCCESS) {
        // TODO: Log error
        close(clientSocket);
        free(data);
        return;
    }
}
