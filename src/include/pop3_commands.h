#ifndef TPE_PROTOS_POP3_COMMANDS_H
#define TPE_PROTOS_POP3_COMMANDS_H

#include <stdint.h>

#include "selector.h"
#include "commands_parser.h"

/*
 *  Executes a command and sends the response to the client
 *  Returns:    0   if command was executed successfully
 *              -1  if an error occurred
 *              -2  if command was not recognized
 */
int executeCommand(struct selector_key *key, struct command *command);

#endif //TPE_PROTOS_POP3_COMMANDS_H
