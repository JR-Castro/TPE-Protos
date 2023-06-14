// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>

#include "pop3.h"
#include "logger.h"

//Patch for MacOS
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0x4000
#endif

static unsigned greetClient(struct selector_key *key) {
    struct client_data *data = key->data;

    size_t limit;       // Max we can read from buffer
    ssize_t count;      // How much we actually read from buffer
    uint8_t *buffer;    // Pointer to read position in buffer
    selector_status status;

    buffer = buffer_read_ptr(&data->outputBuffer, &limit);
    count = send(key->fd, buffer, limit, MSG_NOSIGNAL);

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

static void setupCommandParser(const unsigned state, struct selector_key *key) {
    struct client_data *data = key->data;

    data->commandParser = command_parser_init();
}

static void destroyCommandParser(const unsigned state, struct selector_key *key) {
    struct client_data *data = key->data;

    command_parser_destroy(data->commandParser);
    data->commandParser = NULL;
};

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

    while (buffer_can_read(&data->inputBuffer)) {
        enum command_state commandState = parse_byte_command(&data->inputBuffer, data->commandParser);

        if (commandState == CMD_OK) {
            // TODO:
            // EXECUTE COMMAND FOR THIS USER
            // IF COMMAND IS QUIT, GO TO POP3_CLOSE
            // IF COMMAND IS OK OR INVALID, GO TO POP3_WRITE

            transition = executeCommand(key, data->commandParser->command);

            command_parser_reset(data->commandParser);

            if (transition == POP3_CLOSE) return transition;
            if (transition == POP3_ERROR) goto handle_error;
        }

        if (commandState == CMD_INVALID) {
            errResponse(data, "Invalid command");

            status = selector_set_interest_key(key, OP_WRITE);
            if (status != SELECTOR_SUCCESS) goto handle_error;
            return POP3_WRITE;
        };
    }

    if (transition == POP3_WRITE) {
        status = selector_set_interest_key(key, OP_WRITE);
        if (status != SELECTOR_SUCCESS) goto handle_error;
        return POP3_WRITE;
    }

    return POP3_READ;

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

    buffer = buffer_read_ptr(&data->outputBuffer, &limit);
    count = send(key->fd, buffer, limit, MSG_NOSIGNAL);

    if (count <= 0 && limit != 0) goto handle_error;

    buffer_read_adv(&data->outputBuffer, count);

    if (buffer_can_read(&data->outputBuffer)) {
        return POP3_WRITE;
    }

    status = selector_set_interest_key(key, OP_READ);

    if (status != SELECTOR_SUCCESS) goto handle_error;

    return data->closed ? POP3_CLOSE : POP3_READ;

handle_error:

    status = selector_set_interest_key(key, OP_NOOP);
    return POP3_ERROR;
}

static const struct state_definition client_states[] = {
    {
        .state = POP3_GREETING_WRITE,
        .on_write_ready = greetClient,
    },
    {
        .state = POP3_READ,
        .on_arrival = setupCommandParser,         // Setup parser for auth requests?
        .on_departure = destroyCommandParser,       // Free parser for auth requests?
        .on_read_ready = readUserCommand,
    },
    {
        .state = POP3_WRITE,
        .on_write_ready = writeResponse,
    },
    {
        .state = POP3_CLOSE,
    },
    {
        .state = POP3_ERROR,
    }};

// TODO: Free everything
static void closeConnection(struct selector_key *key) {
    struct client_data *data = key->data;

    log(INFO, "Closing connection with fd %d", key->fd);

    if (data->commandParser != NULL) {
        command_parser_destroy(data->commandParser);
    }

    if (key->fd != -1) {
        selector_unregister_fd(key->s, key->fd);
        close(key->fd);
    }

    if (data->fileArray != NULL) {
        destroy_file_array(key);
    }

    free(data);
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

    // TODO: Client data and state

    data = calloc(1, sizeof(struct client_data));

    if (data == NULL) goto handle_error;

    // I don't know if this will be useful
    data->addr = clientAddr;
    data->addrLen = clientAddrLen;

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

    stm_init(&data->stm);

    // TODO: Client handlers

    if (selector_fd_set_nio(clientSocket) == -1) goto handle_error;

    selector_status selectorStatus = selector_register(key->s, clientSocket, &pop3Handlers, OP_WRITE, data);

    if (selectorStatus != SELECTOR_SUCCESS) goto handle_error;

    return;

handle_error:

    if (clientSocket != -1) {
        close(clientSocket);
    }

    if (data != NULL) {
        free(data);
    }

    // TODO: Log error
}
