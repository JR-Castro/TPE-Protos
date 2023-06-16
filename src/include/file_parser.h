#ifndef TPE_PROTOS_FILE_PARSER_H
#define TPE_PROTOS_FILE_PARSER_H

#include <stdint.h>

enum file_parser_event {
    /*
     * It's a normal character, simply paste it to the buffer
     */
    NORMAL,
    /*
     * Maybe it's a CRLF, maybe not. Print it to the buffer
     */
    MAYBE_CRLF,
    /*
     * This is a CRLF, print it to the buffer.
     */
    CRLF,
    /*
     * This is a dot after a CRLF, byte-stuff it and print it to the buffer.
     */
    CRLF_DOT,
};

struct file_parser {
    enum file_parser_event state;
};

/*
 * Initialize the file parser in previously allocated memory
 */
void file_parser_init(struct file_parser *p);

enum file_parser_event file_parser_feed(struct file_parser *p, uint8_t b);

#endif //TPE_PROTOS_FILE_PARSER_H
