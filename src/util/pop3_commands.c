#include <stdio.h>

#include "pop3_commands.h"
#include "pop3.h"
#include "users.h"

struct command_function {
    char *name;
    enum pop3_state (*function)(struct selector_key *key, struct command *command);
};

static enum pop3_state executeUser(struct selector_key *key, struct command *command);
static enum pop3_state executePass(struct selector_key *key, struct command *command);
static enum pop3_state executeQuit(struct selector_key *key, struct command *command);
static enum pop3_state executeCapa(struct selector_key *key, struct command *command);

static enum pop3_state executeStat(struct selector_key *key, struct command *command);
static enum pop3_state executeList(struct selector_key *key, struct command *command);
static enum pop3_state executeRetr(struct selector_key *key, struct command *command);
static enum pop3_state executeDele(struct selector_key *key, struct command *command);
static enum pop3_state executeNoop(struct selector_key *key, struct command *command);
static enum pop3_state executeRset(struct selector_key *key, struct command *command);


static struct command_function nonauthCommands[] = {
        {"USER",    executeUser},
        {"PASS",    executePass},
        {"QUIT",    executeQuit},
        {"CAPA",    executeCapa},
        {NULL,      NULL},
};

static struct command_function authCommands[] = {
        {"STAT",    executeStat},
        {"LIST",    executeList},
        {"RETR",    executeRetr},
        {"DELE",    executeDele},
        {"NOOP",    executeNoop},
        {"RSET",    executeRset},
        {"QUIT",    executeQuit},
        {"CAPA",    executeCapa},
        {NULL,      NULL},
};

enum pop3_state executeCommand(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;
    struct command_function *commandFunction;
    int i;

    commandFunction = data->isLoggedIn ? authCommands : nonauthCommands;

    for (i = 0; commandFunction[i].name != NULL; i++) {
        if (strcasecmp(commandFunction[i].name, (char*)command->data) == 0) {
            return commandFunction[i].function(key, command);
        }
    }

    errResponse(key->data, "Command not recognized");

    return POP3_WRITE;
}

static enum pop3_state executeNoop(struct selector_key *key, struct command *command) {
    okResponse(key->data, "");

    return POP3_WRITE;
}

static enum pop3_state executeCapa(struct selector_key *key, struct command *command) {
    okResponse(key->data, "+OK\r\n"
                     "USER\r\n"
                     "PIPELINING\r\n"
                     ".\r\n");

    return POP3_WRITE;
}

static enum pop3_state executeUser(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) { return POP3_ERROR; }
    if (command->args1 == NULL) {
        errResponse(data, "Invalid arguments");
        return POP3_WRITE;
    }

    strncpy(data->user.username, (char*)command->args1, MAX_USERNAME);
    okResponse(data, "");

    return POP3_WRITE;
}

static enum pop3_state executePass(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) return POP3_ERROR;
    if (command->args1 == NULL) {
        errResponse(data, "Invalid arguments");
        return POP3_WRITE;
    }

    if (user_authenticate(data->user.username, (char*)command->args1)) {
        data->isLoggedIn = 1;
        strncpy(data->user.password, (char*)command->args1, MAX_PASSWORD);

        // TODO: We should use a lock in the mail.

        okResponse(data, "Logged in");
    } else {
        errResponse(data, "Invalid credentials");
    }

    return POP3_WRITE;
}

static enum pop3_state executeQuit(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    /*if (data->isLoggedIn) {
        // SYNC CHANGES TO FILE SYSTEM
    }*/

    data->closed = true;
    okResponse(data, "Bye");

    return POP3_WRITE;
}

static enum pop3_state executeStat(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return POP3_WRITE;
}

static enum pop3_state executeList(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return POP3_WRITE;
}

static enum pop3_state executeDele(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return POP3_WRITE;
}

static enum pop3_state executeRset(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return POP3_WRITE;
}

static enum pop3_state executeRetr(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return POP3_WRITE;
}
