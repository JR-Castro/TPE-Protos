#include "file_handlers.h"
#include "selector.h"
#include "logger.h"

static void fileRead(struct selector_key *key);
static void fileClose(struct selector_key *key);

static const fd_handler file_handlers = {
    .handle_read = fileRead,
    .handle_close = fileClose,
};

const fd_handler* get_file_handlers() {
    return &file_handlers;
}

static void fileRead(struct selector_key *key) {
    struct file_data *data = ((struct file_data *) key->data);

    size_t limit;       // Max we can write to buffer
    ssize_t count;      // How much we actually wrote to buffer
    uint8_t *buffer;    // Pointer to write position in buffer
    selector_status status;

    buffer = buffer_write_ptr(&data->readBuffer, &limit);
    count = read(key->fd, buffer, limit);

    if (count < 0) goto handle_error;

    // EOF
    if (count == 0) {
        // Send .CRLF to client since we reached EOF
        // Maybe check parser state to see if file finished with \r\n
        char *end = NULL;
        switch (data->parser.state) {
            case NORMAL:
                end = "\r\n.\r\n";
                break;
            case MAYBE_CRLF:
                end = "\n.\r\n";
                break;
            case CRLF:
                end = ".\r\n";
                break;
            case CRLF_DOT:
                end = "\r\n";
                break;
        }

        if (close(key->fd) == -1) {
            log(ERROR, "Error closing file descriptor %d", key->fd);
            goto handle_error;
        }

        return;
    }

    buffer_write_adv(&data->readBuffer, count);

    while (buffer_can_read(&data->readBuffer) && buffer_fits(&data->clientData->outputBuffer, 2)) {

        uint8_t byte = buffer_read(&data->readBuffer);

        // Feed to parser and based on response copy to output buffer of client data.
        enum file_parser_event event = file_parser_feed(&data->parser, byte);

        switch (event) {
            case CRLF_DOT:
                buffer_snprintf(&data->clientData->outputBuffer, "..");
                break;
            default:
                buffer_write(&data->clientData->outputBuffer, byte);
                break;
        }

    }

    // We can't read anymore, either input buffer is empty or output buffer is full
    // Wake up client to write

    status = selector_set_interest(key->s, data->clientFd, OP_WRITE);
    if (status != SELECTOR_SUCCESS) goto handle_error;


handle_error:
    return;
}

static void fileClose(struct selector_key *key) {
    struct file_data *data = ((struct file_data *) key->data);
    struct client_data *clientData = data->clientData;

    clientData->emailFinished = true;
    clientData->emailFd = 0; // Is this necessary?

    free(data);
}
