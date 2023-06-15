#ifndef TPE_PROTOS_POP3_COMMANDS_H
#define TPE_PROTOS_POP3_COMMANDS_H

#include <stdint.h>

#include "selector.h"
#include "commands_parser.h"
#include "pop3.h"

/*
 *  Executes a command and sends the response to the client
 *  Returns:    pop3 state to transition to.
 */
enum pop3_state executeCommand(struct selector_key *key, struct command *command);

#endif //TPE_PROTOS_POP3_COMMANDS_H
