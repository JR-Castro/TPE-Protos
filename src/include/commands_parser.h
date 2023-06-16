#ifndef COMMANDS_PARSER_H
#define COMMANDS_PARSER_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "macros.h"
#include "buffer.h"

#define CMD_MAX_LENGTH 255

// TODO: Change to char *?
struct command {
    uint8_t data[CMD_MAX_LENGTH];
    uint8_t * args1;
    uint8_t * args2;
};

enum command_state {
    CMD_DISPATCH,
    CMD_ARGS,
    CMD_CRLF,
    CMD_OK,
    CMD_INVALID,
};

struct command_parser {
    enum command_state state;
    enum command_state prev_state;
    struct command * command;
    int bytes;
};

////////////////////////////////////////////////////////////
// PROTOTYPES
/**
 * @brief Initialize commands parser
 * @return ptr to command_parser struct
 */
struct command_parser * command_parser_init();

/**
 * @brief Free memory for commands parser
 */
void command_parser_destroy(struct command_parser *p);

// TODO: Instead of explicitly resetting it, make it so the parser restarts automatically? maybe?
/**
 * @brief Reset the parser to initial state
 * @param p: ptr to command_parser struct
 */
void command_parser_reset(struct command_parser *p);

/**
 * @brief feed one byte to the parser and return the next state
 *  CMD_DISPATCH: initial state, parsing command name
 *  CMD_ARGS:     parsing command arguments
 *  CMD_CRLF:     parsing CRLF, end of argument
 *  CMD_OK:       command parsed successfully
 *  CMD_INVALID:  command not recognized, no CRLF was found
 * @param p: ptr to command_parser struct
 * @return next state after parsing the byte
 */
enum command_state parse_byte_command(buffer *inputBuffer, struct command_parser *p);

#endif