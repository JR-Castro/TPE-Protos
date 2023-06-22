#include <stdio.h>
#include <fcntl.h>

#include "pop3_commands.h"
#include "pop3_files.h"
#include "pop3.h"
#include "users.h"
#include "definitions.h"
#include "logger.h"
#include "file_handlers.h"
#include "selector.h"
#include "netutils.h"
#include "args.h"

extern struct pop3_args pop3_args;

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
    okResponse(key->data, "");
    normalResponse(key->data,"USER\r\n"
                     "PIPELINING\r\n"
                     "IMPLEMENTATION TPE-Protos-G05-v01\r\n"
                     ".");

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
        strncpy(data->user.password, (char*)command->args1, MAX_PASSWORD);

        // TODO: We should use a lock in the mail.
        if (fill_file_array(key) < 0) {
            errResponse(data, "Login failed due to an error reading emails folder");
            return POP3_ERROR;
        }

        log(INFO, "User \"%s\" logged in from %s", data->user.username, sockaddr_to_human_buffered((struct sockaddr*)&data->addr))
        data->isLoggedIn = 1;
        okResponse(data, "Logged in");
    } else {
        errResponse(data, "Invalid credentials");
        log(INFO, "Attempted log in as user \"%s\" from %s", data->user.username, sockaddr_to_human_buffered((struct sockaddr*)&data->addr));
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
        return POP3_WRITE;
    }

    char response[MAX_ONELINE_LENGTH];
    snprintf(response, MAX_ONELINE_LENGTH, "%d %u", data->fileArraySize, data->totalMailSize);
    okResponse(data, response);

    return POP3_WRITE;
}

static enum pop3_state executeList(struct selector_key *key, struct command *command) {
    struct client_data *data = key->data;
    char response[MAX_ONELINE_LENGTH];

    if (command->args1 == NULL) {
        okResponse(data, "Listing mails");
        for (int i = 0; i < data->fileArraySize; ++i) {
            if (!data->fileArray[i].deleted) {
                ;
                snprintf(response, MAX_ONELINE_LENGTH, "%u %u", data->fileArray[i].num, data->fileArray[i].size);
                normalResponse(data, response);
            }
        }
        normalResponse(data, ".");
    } else {
        unsigned int mailIndex = strtoul((char*)command->args1, NULL, 10) - 1;

        if (mailIndex >= data->fileArraySize) {
            errResponse(data, "Invalid mail index");
            goto finally;
        }

        if (data->fileArray[mailIndex].deleted) {
            errResponse(data, "Mail already deleted");
            goto finally;
        }

        snprintf(response, MAX_ONELINE_LENGTH, "%u %u", data->fileArray[mailIndex].num, data->fileArray[mailIndex].size);
        okResponse(data, response);
    }

    finally:
    return POP3_WRITE;
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

static int openFileForRetr(struct client_data *data, char *filepath) {
    int fd = -1;
    FILE *file = NULL;
    
    if (pop3_args.filter_command == NULL) {
        fd = open(filepath, O_RDONLY);
        if (fd < 0) {
            log(ERROR, "Error opening file");
        }
        return fd;
    }

    char auxbuffer[MAX_COMMAND_LENGTH] = {0};
    if (snprintf(auxbuffer, MAX_COMMAND_LENGTH, "cat %s | %s", filepath, pop3_args.filter_command) >= MAX_COMMAND_LENGTH) {
        log(ERROR, "Command too long");
        goto handle_error;
    }

    file = popen(auxbuffer, "r");
    if (file == NULL) {
        log(ERROR, "Error creating process");
        goto handle_error;
    }

    fd = fileno(file);
    if (fd < 0) {
        log(ERROR, "Error getting file descriptor")
        goto handle_error;
    }

    data->emailFile = file;
    return fd;

    handle_error:
    if (file != NULL)
        pclose(file);
    return -1;
}

static enum pop3_state executeRetr(struct selector_key *key, struct command *command) {

    int fileFd = -1;

    if (command->args1 == NULL) {
        errResponse(key->data, "Invalid arguments");
        goto handle_error;
    }

    struct client_data *data = key->data;
    struct file_array_item *fa = data->fileArray;
    // Number sent to user for first mail is 1, but index in array is 0
    unsigned int mailIndex = strtoul((char*)command->args1, NULL, 10) - 1;
    char filepath[MAX_PATH_LENGTH];

    if (mailIndex >= data->fileArraySize || fa[mailIndex].deleted) {
        errResponse(key->data, "no such message");
        goto handle_error;
    }

    char *filename = fa[mailIndex].filename;
    char octets[MAX_ONELINE_LENGTH - 6];
    sprintf(octets, "%u octets", fa[mailIndex].size);

    if (get_file_path_user(filepath, data->user.username, filename) < 0) {
        log(ERROR, "[RETR] Error getting file path");
        errResponse(data, "Could not get file path");
        goto handle_error;
    }

    fileFd = openFileForRetr(data, filepath);

    if (fileFd < 0) {
        errResponse(data, "Could not open file");
        goto handle_error;
    }

    data->emailFd = fileFd;

    // TODO: Handle ourselves on each read? or simply fail?
    if (selector_fd_set_nio(fileFd) < 0) {
        log(ERROR, "[RETR] Error setting file descriptor to non-blocking");
        errResponse(data, "Could not open file");
        goto handle_error;
    }

    struct file_data *fileData = calloc(1, sizeof(struct file_data));
    if (fileData == NULL) {
        errResponse(data, "Could not open file");
        goto handle_error;
    }

    buffer_init(&fileData->readBuffer, FILE_BUFFER_SIZE, fileData->readBufferData);
    fileData->clientFd = key->fd;
    fileData->clientData = data;
    file_parser_init(&fileData->parser);

    selector_status selectorStatus = selector_register(key->s, fileFd, get_file_handlers(), OP_READ, fileData);

    if (selectorStatus != SELECTOR_SUCCESS) {
        errResponse(data, "Could not open file");
        goto handle_error;
    }

    // Wait for file read to wake me
    selectorStatus = selector_set_interest_key(key, OP_NOOP);
    if (selectorStatus != SELECTOR_SUCCESS) {
        errResponse(data, "Could not open file");
        goto handle_error;
    }

    okResponse(data, octets);

    return POP3_FILE_WRITE;

    handle_error:
    if (fileFd != -1) {
        close(fileFd);
        data->emailFd = 0;
        if (selector_unregister_fd(key->s, fileFd) != SELECTOR_SUCCESS)
            free(fileData);
    }

    return POP3_WRITE;
}