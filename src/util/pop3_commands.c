#include <stdio.h>

#include "pop3_commands.h"
#include "pop3.h"
#include "users.h"

struct command_function {
    char *name;
    int (*function)(struct selector_key *key, struct command *command);
};

static int executeUser(struct selector_key *key, struct command *command);
static int executePass(struct selector_key *key, struct command *command);
static int executeQuit(struct selector_key *key, struct command *command);
static int executeCapa(struct selector_key *key, struct command *command);

static int executeStat(struct selector_key *key, struct command *command);
static int executeList(struct selector_key *key, struct command *command);
static int executeRetr(struct selector_key *key, struct command *command);
static int executeDele(struct selector_key *key, struct command *command);
static int executeNoop(struct selector_key *key, struct command *command);
static int executeRset(struct selector_key *key, struct command *command);


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

int executeCommand(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;
    struct command_function *commandFunction;
    int i;

    commandFunction = data->isLoggedIn ? authCommands : nonauthCommands;

    for (i = 0; commandFunction[i].name != NULL; i++) {
        if (strcmp(commandFunction[i].name, (char*)command->data) == 0) {
            return commandFunction[i].function(key, command);
        }
    }

    return -2;
}

static int executeNoop(struct selector_key *key, struct command *command) {
    okResponse(key->data, "");

    return 0;
}

static int executeCapa(struct selector_key *key, struct command *command) {
    okResponse(key->data, "+OK\r\n"
                     "USER\r\n"
                     "PIPELINING\r\n"
                     ".\r\n");

    return 0;
}

static int executeUser(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) return -1;
    if (command->args1 == NULL) return -2;

    strncpy(data->user.username, (char*)command->args1, MAX_USERNAME);
    okResponse(data, "");

    return 0;
}

static int executePass(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) return -1;
    if (command->args1 == NULL) return -2;

    if (user_authenticate(data->user.username, (char*)command->args1)) {
        data->isLoggedIn = 1;
        strncpy(data->user.password, (char*)command->args1, MAX_PASSWORD);

        // TODO: We should use a lock in the mail.

        okResponse(data, "Logged in");
    } else {
        errResponse(data, "Invalid credentials");
    }

}

static int executeQuit(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}

static int executeStat(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}

static int executeList(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}

static int executeDele(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}

static int executeRset(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}

static int executeRetr(struct selector_key *key, struct command *command) {
    errResponse(key->data, "Not implemented");
    return -1;
}
