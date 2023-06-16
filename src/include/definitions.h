#ifndef TPE_PROTOS_DEFINITIONS_H
#define TPE_PROTOS_DEFINITIONS_H

// Used in pop3.h
// TODO: Define a proper buffer size
#define BUFFER_SIZE 4096
// POP3 defines a max of 255 chars, including "-ERR " and "\r\n"
// 255 - 4 - 2 (\r\n) + 1 (\0) = 250
#define MAX_ERROR_LENGTH 250
// 255 - 3 ("+OK") - 2 + 1 = 251
#define MAX_ONELINE_LENGTH 251

// Used in pop3_files.h
#define FILE_BUFFER_SIZE 4096
#define MAX_EMAILS 10
#define MAX_PATH_LENGTH 256

// Used in users.h
#define MAX_USERS 1024
#define MAX_USERNAME 64
#define MAX_PASSWORD 64

#endif //TPE_PROTOS_DEFINITIONS_H
