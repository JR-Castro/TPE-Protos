#ifndef UTIL_H
#define UTIL_H

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

bool isAlphanum (uint8_t c);

bool isValidUsernameChar (uint8_t c);

bool isValidPwdChar (uint8_t c);

#endif
