#ifndef TPE_PROTOS_POP3_H
#define TPE_PROTOS_POP3_H

#include <stdint.h>
#include <sys/socket.h>

#include "selector.h"
#include "buffer.h"
#include "stm.h"
#include "users.h"
#include "commands_parser.h"
#include "pop3_commands.h"

// TODO: Define a proper buffer size
#define BUFFER_SIZE 4096
// POP3 defines a max of 255 chars, including "-ERR " and "\r\n"
// 255 - 4 + 1 (\r\n) = 252
#define MAX_ERROR_LENGTH 252

struct client_data {
    struct sockaddr_storage addr;
    socklen_t addrLen;

    struct buffer inputBuffer;
    uint8_t inputBufferData[BUFFER_SIZE];
    struct buffer outputBuffer;
    uint8_t outputBufferData[BUFFER_SIZE];

    struct state_machine stm;

    struct user user;
    bool isLoggedIn;
    bool closed;

    struct command_parser *commandParser;

    char error[MAX_ERROR_LENGTH];
};

enum pop3_state {
    /*
     * Sends initial message to client indicating pop3 server is ready
     * Interests: WRITE
     *
     * Transitions:
     *      - POP3_GREETING_WRITE   if message was not completely sent
     *      - POP3_READ             if message was sent
     *      - POP3_ERROR            if error occurred
     */
    POP3_GREETING_WRITE = 0,
    /*
     * Reads user command from client and executes them if they're valid
     * Interests: READ
     *
     * Transitions:
     *      - POP3_READ             if parsing doesn't reach a conclusion
     *      - POP3_WRITE            if parsing of command was successful
     *      - POP3_CLOSE            if command is quit connection
     *      - POP3_ERROR            if error occurred
     */
    POP3_READ,
    /*
     * Sends commands results to client
     * Interests: WRITE
     *
     * Transitions:
     *      -POP3_READ              if all bytes in output buffer are sent
     *      -POP3_WRITE             if there are still bytes to send
     *      -POP3_ERROR             if error occurred
     */
    POP3_WRITE,
    POP3_CLOSE,
    POP3_ERROR,
};

void passiveAccept(struct selector_key *key);

void errResponse(struct client_data *data, const char *msg);

void okResponse(struct client_data *data, const char *msg);

#endif //TPE_PROTOS_POP3_H
