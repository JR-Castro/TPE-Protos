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
#include "pop3_files.h"
#include "definitions.h"
#include "file_handlers.h"

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
    bool closed; /* Used to indicate to write handler that the connection is being closed */

    struct command_parser *commandParser;

    // TODO: Also add byte-stuffing parser
    struct file_array_item *fileArray;
    unsigned int fileArraySize;
    unsigned int totalMailSize;
    int emailFd;
    bool emailFinished;

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
    /*
     *  Writes the result of a RETR to the client
     *  Interests: WRITE
     *
     *  Transitions:
     *    - POP3_READ               if file finished
     *    - POP3_FILE_WRITE         if file not finished
     *    - POP3_ERROR              if error occurred
     */
    POP3_FILE_WRITE,
    POP3_CLOSE,
    POP3_ERROR,
};

void passiveAccept(struct selector_key *key);

/*
 * Automatically formats response: "-ERR %s\r\n"
 * Should use the MAX_ERROR_LENGTH macro to ensure protocol compliance
 */
void errResponse(struct client_data *data, const char *msg);

/*
 * Automatically formats response: "+OK %s\r\n"
 * Should use the MAX_ONELINE_LENGTH macro to ensure protocol compliance
 */
void okResponse(struct client_data *data, const char *msg);


/*
 *  Writes a response to the client: "%s\r\n"
 */
void normalResponse(struct client_data *data, const char *msg);

#endif //TPE_PROTOS_POP3_H
