#include <stdio.h>

#include "pop3_commands.h"
#include "pop3_files.h"
#include "pop3.h"
#include "users.h"
#include "definitions.h"
#include "logger.h"

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

    command_parser_reset(data->commandParser);

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

        if (fill_file_array(key) < 0) {
            errResponse(data, "Error reading mails");
            return POP3_ERROR;
        }

        okResponse(data, "Logged in");
    } else {
        errResponse(data, "Invalid credentials");
    }

    return POP3_WRITE;
}

static enum pop3_state executeQuit(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->isLoggedIn) {
        // SYNC CHANGES TO FILE SYSTEM
        sync_to_maildrop(key);
    } else {
        okResponse(data, "Bye");
    }

    data->closed = true;

    return POP3_WRITE;
}

static enum pop3_state executeStat(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    if (data->fileArray == NULL) {
        errResponse(data, "Error reading mails");
        return POP3_ERROR;
    }

    char response[MAX_ONELINE_LENGTH];
    snprintf(response, MAX_ONELINE_LENGTH, "%d %u", data->fileArraySize, data->totalMailSize);
    okResponse(data, response);

    return POP3_WRITE;
}

static enum pop3_state executeList(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    okResponse(data, "");

    int sent = 0;

    for (int i = 0; i < data->fileArraySize; ++i) {
        if (!data->fileArray[i].deleted) {
            char response[MAX_ONELINE_LENGTH];
            snprintf(response, MAX_ONELINE_LENGTH, "%u %u", i, data->fileArray[i].size);
            normalResponse(data, response);
            sent++;
        }
    }

    normalResponse(data, ".");

    return sent ? POP3_WRITE : POP3_READ;
}

static enum pop3_state executeDele(struct selector_key *key, struct command *command) {

    struct client_data *data = key->data;

    if (command->args1 == NULL) {
        errResponse(key->data, "Invalid arguments");
        goto finally;
    }

    unsigned int mailIndex = strtoul((char*)command->args1, NULL, 10);

    if (mailIndex >= data->fileArraySize) {
        errResponse(key->data, "Invalid mail index");
        goto finally;
    }

    if (data->fileArray[mailIndex].deleted) {
        errResponse(key->data, "Mail already deleted");
        goto finally;
    }

    data->fileArray[mailIndex].deleted = true;
    char response[MAX_ONELINE_LENGTH];
    snprintf(response, MAX_ONELINE_LENGTH, "Message %u deleted", mailIndex);
    okResponse(key->data, response);

finally:
    return POP3_WRITE;
}

static enum pop3_state executeRset(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;

    for (int i = 0; i < data->fileArraySize; ++i) {
        data->fileArray[i].deleted = false;
    }

    char response[MAX_ONELINE_LENGTH];
    snprintf(response, MAX_ONELINE_LENGTH, "Maildrop has %u messages (%u octets)", data->fileArraySize, data->totalMailSize);
    okResponse(key->data, response);

    return POP3_WRITE;
}

static enum pop3_state executeRetr(struct selector_key *key, struct command *command) {
    if (command->args1 == NULL) {
        errResponse(key->data, "Invalid arguments");
        goto finally;
    }

    struct client_data *data = key->data;
    struct file_array_item *fa = data->fileArray;
    const char * filename = (char *) command->args1;
    char filepath[MAX_PATH_LENGTH];
    FILE * fstream;
    int file_index = -1;

    if (fa == NULL
        || get_file_path_user(filepath, data->user.username, filename) < 0) {
        log(ERROR, "[RETR] Error getting file path");
        goto finally;
    }

    fstream = fopen(filepath, "r");
    if (!fstream || is_file_deleted(key, filename, &file_index)) {
        errResponse(data, "No such message");
        goto finally;

    } else {
        char aux[10];
        char line[MAX_ONELINE_LENGTH] = {0};
        unsigned int fp_size = fa[file_index].size;

        sprintf(aux, "%u", fp_size);
        strcat(line, aux);
        strcat(line, " octets");
        okResponse(data, line);
        memset(line, 0, MAX_ONELINE_LENGTH);

        while ((fgets(line, MAX_ONELINE_LENGTH, fstream)) != NULL) {
            buffer_snprintf(&(data->outputBuffer), "%s", line);
        }
        buffer_snprintf(&(data->outputBuffer), "\n.\n");
        fclose(fstream);
    }


finally:
    return POP3_WRITE;
}