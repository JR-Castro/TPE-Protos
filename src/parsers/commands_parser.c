#include <stdio.h>
#include "logger.h"
#include "emalloc.h"
#include "commands_parser.h"

////////////////////////////////////////////////////////////
// STATIC FUNCTIONS
/**
 * @brief feed one byte to the parser and return the next state
 *  CMD_DISPATCH: initial state, parsing command name
 *  CMD_ARGS:     parsing command arguments
 *  CMD_CRLF:     parsing CRLF, end of argument
 *  CMD_OK:       command parsed successfully
 *  CMD_INVALID:  command not recognized, no CRLF was found
 * @param p: ptr to command_parser struct
 * @param c: byte to feed
 * @param index: ptr to the byte in the buffer
 * @return next state after parsing the byte
 */
static enum command_state command_parser_feed(struct command_parser *p, uint8_t c);

static void reset_command_parser(struct command_parser *p) {
    p->bytes = 0;
    p->command->args1 = p->command->args2 = NULL;
}

// For debug only
static void printCommand(struct command_parser *p);

////////////////////////////////////////////////////////////
// FUNCTIONS
struct command_parser * command_parser_init() {
    log(DEBUG, "Initializing command parser...");
    struct command_parser *p = emalloc(sizeof(struct command_parser));
    p->state = p->prev_state = CMD_DISPATCH;
    p->bytes = 0;
    p->command = emalloc(sizeof(struct command));
    p->command->args1 = p->command->args2 = NULL;
    log(DEBUG, "command parser initialization DONE!");
    return p;
}

void command_parser_reset(struct command_parser *p) {
    p->state = p->prev_state = CMD_DISPATCH;
    p->bytes = 0;
    p->command->args1 = p->command->args2 = NULL;
}

static enum command_state command_parser_feed(struct command_parser *p, uint8_t c) {
    p->command->data[p->bytes] = c;
    p->bytes++;
    switch (p->state) {
        case CMD_DISPATCH:
            if (c == ' ') {
                p->prev_state = p->state;
                p->state = CMD_ARGS;
                p->command->data[p->bytes-1] = '\0';
            } else if (c == '\r') {
                p->state = CMD_CRLF;
                p->command->data[p->bytes-1] = '\0';
            }
            break;
        case CMD_CRLF:
            p->state = (c == '\n')
                     ? (p->prev_state == CMD_INVALID) ? CMD_DISPATCH : CMD_OK
                     : CMD_INVALID;
            // reset parser after
            if (p->state == CMD_DISPATCH) {
                reset_command_parser(p);
            } else if (p->state == CMD_OK) {
                p->command->data[p->bytes-2] = '\0';
                p->command->data[p->bytes-1] = '\0';
            }
            p->prev_state = CMD_CRLF;
            break;
        case CMD_ARGS:
            if (c != ' ' && c != '\t' && c != '\r'
                && p->prev_state == CMD_DISPATCH) {
                // first char after one or more spaces
                if (p->command->args1 == NULL) {
                    p->command->args1 = &(p->command->data[p->bytes-1]);
                } else {
                    p->command->args2 = &(p->command->data[p->bytes-1]);
                }
                p->prev_state = CMD_ARGS;
            } else if (c == ' ' && p->prev_state == CMD_ARGS) {
                // space for second argument
                p->prev_state = CMD_DISPATCH;
                p->command->data[p->bytes-1] = '\0';
            } else if (c == '\r') {
                p->prev_state = CMD_ARGS;
                p->state = CMD_CRLF;
            }
            break;
        case CMD_INVALID:
            p->prev_state = CMD_INVALID;
            p->state = (c == '\r') ? CMD_CRLF : CMD_INVALID;
            break;
        case CMD_OK:
            break;
    }

    return p->state;
}

enum command_state parse_byte_command(buffer *inputBuffer, struct command_parser *p) {
    enum command_state state = p->state;
    if (buffer_can_read(inputBuffer)) {
        state = command_parser_feed(p, buffer_read(inputBuffer));
    }

    return state;
}

void command_parser_destroy(struct command_parser *p) {
    log(DEBUG, "Destroying command parser...");
    memset(p->command, 0, sizeof(struct command));
    free(p->command);
    memset(p, 0, sizeof(struct command_parser));
    free(p);
    log(DEBUG, "command parser destruction DONE!");
}

/** FOR DEBUG ONLY */
static void printCommand(struct command_parser *p) {
    printf("=== PARSED DATA ===\n");
    printf("command_name: %s\n", p->command->data);
    printf("arg1: %s\n", p->command->args1);
    printf("arg2: %s\n", p->command->args2);
    printf("=== MEM DUMP ===\n");
    for (int i = 0; i < p->bytes; i++) {
        printf("%c (%p)\n", p->command->data[i], &(p->command->data[i]));
    }
    printf("===================\n");
}