#include "util.h"

#include <stdbool.h>

bool isAlphanum(uint8_t c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9')) ? true : false;

}

bool isValidUsernameChar(uint8_t c) {
    return (isAlphanum(c) || c == '_' || c == '-' || c == '.') ? true : false;
}

bool isValidPwdChar(uint8_t c) {
    return (isValidUsernameChar(c) || c == '!' || c == '$' || c == '#' || c == '*') ? true : false;
}
