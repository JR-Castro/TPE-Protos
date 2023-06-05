#include <stdio.h>
#include <logger.h>
#include <commands_parser.h>
#include <parser_utils.h>
#include <emalloc.h>

////////////////////////////////////////////////////////////
// STATIC PROTOTYPES
struct command_parser * command_parser_init();

static queue * available_commands_queue_init (struct command_parser *p);

static struct command * newCommand (struct command_description * desc);

////////////////////////////////////////////////////////////
// FUNCTIONS

bool compareElements (void * a, void * b) {
    return ((struct command_description *) a)->name == ((struct command_description *) b)->name;
}

struct command_parser * command_parser_init() {
    log(DEBUG, "Initializing command parser...");
    struct command_parser *p = emalloc(sizeof(struct command_parser));
    p->state = CMD_DISPATCH;
    p->commands_queue = newQueue(sizeof(struct command));
    p->available_commands_queue = available_commands_queue_init(p);

    log(DEBUG, "command parser initialization DONE!");
    return p;
}

static queue * available_commands_queue_init(struct command_parser *p) {
    log(DEBUG, "Initializing available commands queue...");
    queue * q = newQueue(sizeof(struct command_description));

//    for (int i = 0; i < N(commands); i++) {
//        const struct parser_definition def = parser_utils_strcmpi(commands[i].name);
//        commands[i].parser = parser_init(parser_no_classes(), &def);
//        q = add(q, &(commands[i]));
//    }

    // tried to use a for loop, but it didn't work. Complained about const assignment.
    // Weird! so I'm using this ugly code instead.
    const struct parser_definition user = parser_utils_strcmpi("USER ");
    commands[CMD_USER].parser = parser_init(parser_no_classes(), &user);
    q = add(q, &(commands[CMD_USER]));

    const struct parser_definition pass = parser_utils_strcmpi("PASS ");
    commands[CMD_PASS].parser = parser_init(parser_no_classes(), &pass);
    q = add(q, &(commands[CMD_PASS]));

    const struct parser_definition stat = parser_utils_strcmpi("STAT\r\n");
    commands[CMD_STAT].parser = parser_init(parser_no_classes(), &stat);
    q = add(q, &(commands[CMD_STAT]));

    const struct parser_definition list = parser_utils_strcmpi("LIST ");
    commands[CMD_LIST].parser = parser_init(parser_no_classes(), &list);
    q = add(q, &(commands[CMD_LIST]));

    const struct parser_definition retr = parser_utils_strcmpi("RETR ");
    commands[CMD_RETR].parser = parser_init(parser_no_classes(), &retr);
    q = add(q, &(commands[CMD_RETR]));

    const struct parser_definition dele = parser_utils_strcmpi("DELE ");
    commands[CMD_DELE].parser = parser_init(parser_no_classes(), &dele);
    q = add(q, &(commands[CMD_DELE]));

    const struct parser_definition noop = parser_utils_strcmpi("NOOP\r\n");
    commands[CMD_NOOP].parser = parser_init(parser_no_classes(), &noop);
    q = add(q, &(commands[CMD_NOOP]));

    const struct parser_definition quit = parser_utils_strcmpi("QUIT\r\n");
    commands[CMD_QUIT].parser = parser_init(parser_no_classes(), &quit);
    q = add(q, &(commands[CMD_QUIT]));

    p->available_commands_queue = q;
    log(DEBUG, "available commands queue initialization DONE!");

    return p->available_commands_queue;
}

enum command_state command_parser_feed(struct command_parser *p, uint8_t c) {
    enum command_state next_state = CMD_DISPATCH;

    // Iterating over commands array
    for (int i = 0; i < N(commands); i++) {
        if (!commands[i].active) {
            continue;
        }
        struct parser *cmd_parser = commands[i].parser;
        const struct parser_event *event = parser_feed(cmd_parser, c);
        if (event->type == STRING_CMP_EQ) {
            log(DEBUG, "Command matched: %s", commands[i].name);
            p->commands_queue = add(p->commands_queue, newCommand(&(commands[i])));
            // active all parser for next iteration
            for (int i = 0; i < N(commands); i++) {
                commands[i].active = true;
            }
            next_state = CMD_ARGS;
            break;
        } else if (event->type == STRING_CMP_MAYEQ) {
            log(DEBUG, "may matched: %s", commands[i].name);
            next_state = CMD_MAYEQ;
        } else {
            log(DEBUG, "Command did not matched: %s", commands[i].name);
            commands[i].active = false;
        }
    }
    if (next_state == CMD_DISPATCH) {
        log(DEBUG, "No command matched");
        struct command_description * invalid_cmd = emalloc(sizeof(struct command_description));
        invalid_cmd->name = "INVALID";
        invalid_cmd->type = CMD_INVALID;
        p->commands_queue = add(p->commands_queue, newCommand(invalid_cmd));
        next_state = CMD_INVALID;
    }

    return next_state;
}

bool consume_until_crlf (buffer *b, uint8_t *ret, size_t *n) {
    bool crlf = false;
    size_t i = 0;
    while (buffer_can_read(b)) {
        ret[i++] = buffer_read(b);
        if (i > 2 && ret[i-1] == '\n' && ret[i-2] == '\r') {
            log(DEBUG, "Found CRLF")
            crlf = true;
            ret[i-2] = '\0';
            break;
        }
    }
    n = &i;

    return crlf;
}

void command_parser_destroy(struct command_parser *p) {
   // TODO
}

static struct command * newCommand (struct command_description * desc) {
    struct command * cmd = emalloc(sizeof(struct command));
    cmd->description = emalloc(sizeof(struct command_description));
    cmd->description = desc;
    cmd->args = NULL;

    return cmd;
}

////////////////////////////////////////////////////////////
// FUNCTIONS DEBUG ONLY

void printAvailableCommandsNames (queue * q) {
    log(DEBUG, "===========================================");
    log(DEBUG, "Printing available commands in queue...");
    queueIterator * it = newQueueIterator(q);
    while (hasNext(it)) {
        log(DEBUG, "name: %s", ((struct command_description *) next(it))->name);
    }
    free(it);
    log(DEBUG, "===========================================");
}

void printParsedCommandsNames (queue * q) {
    log(DEBUG, "===========================================");
    log(DEBUG, "Printing parsed commands in queue...");
    queueIterator * it = newQueueIterator(q);
    while (hasNext(it)) {
        struct command *cmd = ((struct command *) next(it));
        log(DEBUG, "name: %s", cmd->description->name);
        log(DEBUG, "args: %s", cmd->args);
    }
    free(it);
    log(DEBUG, "===========================================");
}