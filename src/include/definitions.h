#ifndef TPE_PROTOS_DEFINITIONS_H
#define TPE_PROTOS_DEFINITIONS_H

// Used in pop3.h
// TODO: Define a proper buffer size
#define BUFFER_SIZE 4096
// POP3 defines a max of 512 chars, including "-ERR " and "\r\n"
// 255 - 4 - 2 (\r\n) + 1 (\0) = 250
#define MAX_ERROR_LENGTH 507
// 255 - 3 ("+OK") - 2 + 1 = 251
#define MAX_ONELINE_LENGTH 508
#define BUFFER_FREE_SPACE 512

// Used in pop3_files.h
#define FILE_BUFFER_SIZE 4096
#define MAX_EMAILS 10
#define MAX_PATH_LENGTH 256

// Used in users.h
#define MAX_USERS 1024
// Max argument size on POP3 is 40 characters, +1 for \0
#define MAX_USERNAME 41
#define MAX_PASSWORD 41

#define UDP_BUFFER_SIZE 4096
#define MAX_PAGE_SIZE 200
#define MIN_PAGE_SIZE 1
#define MAX_UDP_SIZE 65507
#define MANAGER_REQUEST_HEADER_SIZE 10
#define MANAGER_RESPONSE_HEADER_SIZE 7

#endif //TPE_PROTOS_DEFINITIONS_H
