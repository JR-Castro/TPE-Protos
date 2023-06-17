#include "file_parser.h"

void file_parser_init(struct file_parser *p) {
    p->state = NORMAL;
}

enum file_parser_event file_parser_feed(struct file_parser *p, uint8_t b) {
    switch (p->state) {
        case NORMAL:
            if (b == '\r') {
                return p->state = MAYBE_CRLF;
            } else {
                return NORMAL;
            }
        case MAYBE_CRLF:
            switch (b) {
                case '\r':
                    return p->state = MAYBE_CRLF;
                case '\n':
                    return p->state = CRLF;
                default:
                    return p->state = NORMAL;
            }
        case CRLF:
            switch (b) {
                case '.':
                    return p->state = CRLF_DOT;
                case '\r':
                    return p->state = MAYBE_CRLF;
                default:
                    return p->state = NORMAL;
            }
        case CRLF_DOT:
            if (b == '\r')
                return p->state = MAYBE_CRLF;
            else
                return p->state = NORMAL;
    }
}
