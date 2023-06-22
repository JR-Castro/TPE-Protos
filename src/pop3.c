// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "pop3.h"
#include "logger.h"
#include "metrics.h"
#include "netutils.h"

//Patch for MacOS
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0x4000
#endif

ssize_t sendMetrics(int fd, const void *buf, size_t n, int flags) {
    ssize_t count = send(fd, buf, n, flags);
    metricsBytesSent(count);
    return count;
}

static unsigned greetClient(struct selector_key *key) {
    struct client_data *data = key->data;

    size_t limit;       // Max we can read from buffer
    ssize_t count;      // How much we actually read from buffer
    uint8_t *buffer;    // Pointer to read position in buffer
    selector_status status;

    buffer = buffer_read_ptr(&data->outputBuffer, &limit);
    count = sendMetrics(key->fd, buffer, limit, MSG_NOSIGNAL);

    if (count <= 0) goto handle_error;

    buffer_read_adv(&data->outputBuffer, count);

    if (buffer_can_read(&data->outputBuffer)) {
        return POP3_GREETING_WRITE;
    }

    status = selector_set_interest_key(key, OP_READ);
    if (status != SELECTOR_SUCCESS) goto handle_error;

    return POP3_READ;

handle_error:
    status = selector_set_interest_key(key, OP_NOOP);
    return POP3_ERROR;
}

void errResponse(struct client_data *data, const char *msg) {
    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&data->outputBuffer, &limit);
    count = snprintf((char *) buffer, limit, "-ERR %s\r\n", msg);
    buffer_write_adv(&data->outputBuffer, count);
}

void okResponse(struct client_data *data, const char *msg) {
    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&data->outputBuffer, &limit);
    count = snprintf((char *) buffer, limit, "+OK %s\r\n", msg);
    buffer_write_adv(&data->outputBuffer, count);
}

void normalResponse(struct client_data *data, const char *msg) {
    size_t limit;
    uint8_t *buffer;
    ssize_t count;

    buffer = buffer_write_ptr(&data->outputBuffer, &limit);
    count = snprintf((char *) buffer, limit, "%s\r\n", msg);
    buffer_write_adv(&data->outputBuffer, count);
}

static enum pop3_state feedInputToParser(struct selector_key *key, struct client_data *data) {
    enum pop3_state transition;
    while (buffer_can_read(&data->inputBuffer) && buffer_fits(&data->outputBuffer, BUFFER_FREE_SPACE)) {
        enum command_state commandState = parse_byte_command(&data->inputBuffer, data->commandParser);

        if (commandState == CMD_OK) {
            // TODO:
            // EXECUTE COMMAND FOR THIS USER
            // IF COMMAND IS QUIT, GO TO POP3_CLOSE
            // IF COMMAND IS OK OR INVALID, GO TO POP3_WRITE

            transition = executeCommand(key, data->commandParser->command);

            // TODO: We should check if output is too full, and if it's directly jump to write.
            // Once write is complete, continue parsing and if there's still data in input buffer
            // Parse it there and continue with the parse - send thing.
            // Otherwise we're not actually doing pipelining.
            // Ask Martone in case of doubt.

            // Maybe simply execute the first command here and then parse the rest on writeResponse.

            command_parser_reset(data->commandParser);

            return transition;
        }

        if (commandState == CMD_INVALID) {
            errResponse(data, "Invalid command");
            return POP3_WRITE;
        };
    }
    return POP3_READ;
}

static unsigned readUserCommand(struct selector_key *key) {
    struct client_data *data = key->data;

    size_t limit;       // Max we can read from buffer
    ssize_t count;      // How much we actually read from buffer
    uint8_t *buffer;    // Pointer to read position in buffer
    selector_status status;
    enum pop3_state transition;

    buffer = buffer_write_ptr(&data->inputBuffer, &limit);
    count = recv(key->fd, buffer, limit, MSG_NOSIGNAL);

    if (count <= 0 && limit != 0) goto handle_error;

    buffer_write_adv(&data->inputBuffer, count);

    transition = feedInputToParser(key, data);

    switch (transition) {
        case POP3_WRITE:
            status = selector_set_interest_key(key, OP_WRITE);
            if (status != SELECTOR_SUCCESS) goto handle_error;
            return POP3_WRITE;
        case POP3_ERROR:
            goto handle_error;
        default:
            return transition;
    }

handle_error:
    selector_set_interest_key(key, OP_NOOP);

    return POP3_ERROR;
}

static unsigned writeResponse(struct selector_key *key) {
    struct client_data *data = key->data;

    size_t limit;       // Max we can read from buffer
    ssize_t count;      // How much we actually read from buffer
    uint8_t *buffer;    // Pointer to read position in buffer
    selector_status status;
    enum pop3_state transition;

    buffer = buffer_read_ptr(&data->outputBuffer, &limit);
    count = sendMetrics(key->fd, buffer, limit, MSG_NOSIGNAL);

    if (count <= 0 && limit != 0) goto handle_error;

    buffer_read_adv(&data->outputBuffer, count);

    // We sent the response to quit and close the connection
    if (data->closed)
        return POP3_CLOSE;

    if (buffer_can_read(&data->outputBuffer)) {
        return POP3_WRITE;
    }

    transition = feedInputToParser(key, data);

    switch (transition) {
        case POP3_READ:
            status = selector_set_interest_key(key, OP_READ);
            if (status != SELECTOR_SUCCESS) goto handle_error;
            return POP3_READ;
        case POP3_ERROR:
            goto handle_error;
        default:
            return transition;
    }

handle_error:

    selector_set_interest_key(key, OP_NOOP);
    return POP3_ERROR;
}

static unsigned int writeFile(struct selector_key *key) {
    struct client_data *data = key->data;

    size_t limit;               // Max we can read from buffer
    ssize_t count;              // How much we actually read from buffer
    uint8_t *buffer;            // Pointer to read position in buffer
    selector_status status;
    enum pop3_state transition;

    buffer = buffer_read_ptr(&data->outputBuffer, &limit);
    count = sendMetrics(key->fd, buffer, limit, MSG_NOSIGNAL);

    if (count < 0 && limit != 0) goto handle_error;

    buffer_read_adv(&data->outputBuffer, count);

    if (buffer_can_read(&data->outputBuffer)) {
        // There's still more to send.
        return POP3_FILE_WRITE;
    }

    if (data->emailFinished) {
        if (buffer_can_read(&data->inputBuffer)) {
            transition = feedInputToParser(key, data);
            switch (transition) {
                case POP3_READ:
                    status = selector_set_interest_key(key, OP_READ);
                    if (status != SELECTOR_SUCCESS) goto handle_error;
                    return POP3_READ;
                default:
                    return transition;
            }
        }

        // Email finished and no more to send to user, read next command.
        status = selector_set_interest_key(key, OP_READ);
        if (status != SELECTOR_SUCCESS) goto handle_error;
        return POP3_READ;
    }

    // We don't have anything else to write but file didn't finish, so we need to wait for file to read more
    status = selector_set_interest(key->s, data->emailFd, OP_READ);
    if (status != SELECTOR_SUCCESS) goto handle_error;
    status = selector_set_interest_key(key, OP_NOOP);
    if (status != SELECTOR_SUCCESS) goto handle_error;
    return POP3_FILE_WRITE;

handle_error:
    selector_set_interest_key(key, OP_NOOP);
    return POP3_ERROR;
}

static void stopFileWrite(const enum pop3_state state, struct selector_key *key) {
    struct client_data *data = key->data;
    if (data->emailFd != -1) {
        selector_unregister_fd(key->s, data->emailFd);
    }
    data->emailFinished = false;
}

static const struct state_definition client_states[] = {
    {
        .state = POP3_GREETING_WRITE,
        .on_write_ready = greetClient,
    },
    {
        .state = POP3_READ,
        .on_read_ready = readUserCommand,
    },
    {
        .state = POP3_WRITE,
        .on_write_ready = writeResponse,
    },
    {
        .state = POP3_FILE_WRITE,
        .on_write_ready = writeFile,
        .on_departure = stopFileWrite,
    },
    {
        .state = POP3_CLOSE,
    },
    {
        .state = POP3_ERROR,
    }};

static void closeConnection(struct selector_key *key) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) {
        log(INFO, "Closing connection with user \"%s\" from %s", data->user.username, sockaddr_to_human_buffered((struct sockaddr*)&data->addr))
    } else {
        log(INFO, "Closing connection with %s", sockaddr_to_human_buffered((struct sockaddr*)&data->addr))
    }

    if (data->commandParser != NULL) {
        command_parser_destroy(data->commandParser);
    }

    if (key->fd != -1) {
        selector_unregister_fd(key->s, key->fd);
        close(key->fd);
    }

    if (data->emailFd != -1) {
        selector_unregister_fd(key->s, data->emailFd);
    }

    if (data->fileArray != NULL) {
        destroy_file_array(key);
    }

    free(data);

    metricsConnectionClosed();
}

/* Handlers for the selector events, so we can wrap the state machine calls */
static void pop3Read(struct selector_key *key);
static void pop3Write(struct selector_key *key);
static void pop3Block(struct selector_key *key);
static void pop3Close(struct selector_key *key);

static const fd_handler pop3Handlers = {
    .handle_read = pop3Read,
    .handle_write = pop3Write,
    .handle_block = pop3Block,
    .handle_close = pop3Close,
};

void pop3Read(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    const enum pop3_state state = stm_handler_read(stm, key);
    if (state == POP3_ERROR || state == POP3_CLOSE) {
        closeConnection(key);
    }
}

void pop3Write(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    const enum pop3_state state = stm_handler_write(stm, key);
    if (state == POP3_ERROR || state == POP3_CLOSE) {
        closeConnection(key);
    }
}

void pop3Block(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    const enum pop3_state state = stm_handler_block(stm, key);
    if (state == POP3_ERROR || state == POP3_CLOSE) {
        closeConnection(key);
    }
}

void pop3Close(struct selector_key *key) {
    struct state_machine *stm = &((struct client_data *) key->data)->stm;
    stm_handler_close(stm, key);
}


void passiveAccept(struct selector_key *key) {
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    struct client_data *data = NULL;

    int clientSocket = accept(key->fd, (struct sockaddr *) &clientAddr, &clientAddrLen);

    /* TODO: Ask if we should retry accept in these cases:
     * "For reliable operation the application should detect the network errors defined for
     * the protocol after accept() and treat them like EAGAIN by retrying.  In the case of TCP/IP,
     * these are ENETDOWN,  EPROTO,  ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH,
     * EOPNOTSUPP, and ENETUNREACH."
     * - man 2 accept
     */

    if (clientSocket == -1) goto handle_error;

    data = calloc(1, sizeof(struct client_data));

    if (data == NULL) goto handle_error;

    // I don't know if this will be useful
    data->addr = clientAddr;

    buffer_init(&data->inputBuffer, BUFFER_SIZE, data->inputBufferData);
    buffer_init(&data->outputBuffer, BUFFER_SIZE, data->outputBufferData);

    // Put greeting in output buffer

    char *s = "+OK POP3 server ready\r\n";

    // Since the output buffer is empty, we can write directly to it
    for (int i=0; s[i]; i++) {
        buffer_write(&data->outputBuffer, s[i]);
    }

    data->stm.initial = POP3_GREETING_WRITE;
    data->stm.max_state = POP3_ERROR;
    data->stm.states = client_states;
    data->commandParser = command_parser_init();
    data->emailFd = -1;

    stm_init(&data->stm);

    if (selector_fd_set_nio(clientSocket) == -1) goto handle_error;

    selector_status selectorStatus = selector_register(key->s, clientSocket, &pop3Handlers, OP_WRITE, data);
    if (selectorStatus != SELECTOR_SUCCESS) goto handle_error;

    metricsNewConnection();
    log(INFO, "New connection from %s", sockaddr_to_human_buffered((struct sockaddr*)&clientAddr));

    return;

handle_error:

    if (clientSocket != -1) {
        close(clientSocket);
    }

    if (data != NULL) {
        free(data);
    }

    log(ERROR, "Error accepting connection");
}
