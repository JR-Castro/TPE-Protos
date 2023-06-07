#ifndef COMMANDS_PARSER_H
#define COMMANDS_PARSER_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <macros.h>
#include <queue.h>
#include "buffer.h"

enum command_type {
    CMD_USER,
    CMD_PASS,
    CMD_STAT,
    CMD_LIST,
    CMD_RETR,
    CMD_DELE,
    CMD_NOOP,
    CMD_QUIT,
    CMD_ERROR,
};

typedef struct command_description {
    char * name;
    size_t length;
    size_t args_min;
    size_t args_max;
    enum command_type type;
    struct parser * parser;
    bool active;
} command_description;

// Command definitions
struct command_description commands[] = {
    { .name = "USER ",      .length = 4,  .args_min = 1,   .args_max = 512 - 7,  .type = CMD_USER,   .parser = NULL, .active = true, },
    { .name = "PASS ",      .length = 4,  .args_min = 1,   .args_max = 512 - 7,  .type = CMD_PASS,   .parser = NULL, .active = true, },
    { .name = "STAT\r\n",   .length = 4,  .args_min = 0,   .args_max = 0,        .type = CMD_STAT,   .parser = NULL, .active = true, },
    { .name = "LIST ",      .length = 4,  .args_min = 0,   .args_max = 1,        .type = CMD_LIST,   .parser = NULL, .active = true, },
    { .name = "RETR ",      .length = 4,  .args_min = 0,   .args_max = 1,        .type = CMD_RETR,   .parser = NULL, .active = true, },
    { .name = "DELE ",      .length = 4,  .args_min = 0,   .args_max = 1,        .type = CMD_DELE,   .parser = NULL, .active = true, },
    { .name = "NOOP\r\n",   .length = 4,  .args_min = 0,   .args_max = 0,        .type = CMD_NOOP,   .parser = NULL, .active = true, },
    { .name = "QUIT\r\n",   .length = 4,  .args_min = 0,   .args_max = 1,        .type = CMD_QUIT,   .parser = NULL, .active = true, },
};

struct command {
    struct command_description * description;
    uint8_t * args;
};

enum command_state {
    CMD_DISPATCH,
    CMD_MAYEQ,
    CMD_ARGS,
    CMD_OK,
    CMD_INVALID,
};

struct command_parser {
    enum command_state state;
    queue * commands_queue; // result of parsed commands: Queue<Commands>
    queue * available_commands_queue; // Queue of available commands: Queue<Command_Description>
};

////////////////////////////////////////////////////////////
// PROTOTYPES

/**
 * @brief Initialize commands parser and queues
 * @return ptr to command_parser struct
 */
struct command_parser * command_parser_init();

/**
 * @brief feed one byte to the parser and filter possible commands
 * @param p: ptr to command_parser struct
 * @param c: byte to feed
 * @return next state after parsing the byte
 */
enum command_state command_parser_feed(struct command_parser *p, uint8_t c);

/**
 * @brief consume buffer until crlf is found, or buffer is empty
 * @param b ptr to buffer to consume
 * @param ret ptr to return response ('\0' terminated to easy print)
 * @return true if crlf was found, false otherwise
 */
bool consume_until_crlf (buffer *b, uint8_t *ret, size_t *n);

/**
 * @brief comparator function to be used in queue comparison operations
 * @param a ptr to first element
 * @param b ptr to second element
 * @return true if elements are equal, false otherwise
 */
bool compareElements (void * a, void * b);

////////////////////////////////////////////////////////////
// PROTOTYPES DEBUG ONLY

/**
 * @brief iterate queue of command_description elements and print available command's names (queue is not modified)
 * @param q ptr to queue of available commands
 */
void printAvailableCommandsNames (queue * q);

/**
 * @brief iterate queue of command elements and print command's names (queue is not modified)
 * @param q ptr to queue of commands
 */
void printParsedCommandsNames (queue * q);

#endif