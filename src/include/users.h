#ifndef TPE_PROTOS_USERS_H
#define TPE_PROTOS_USERS_H

#include <stdint.h>

#define MAX_USERS 1024
#define MAX_USERNAME 64
#define MAX_PASSWORD 64

struct user {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
};

int user_authenticate(char *username, char *password);

int user_add(char *username, char *password);

int user_add_basic(char *userpass);

int user_remove(char *username);

int user_exists(char *username);

int user_change_password(char *username, char *new_password);

int user_change_username(char *username, char *new_username);

#endif //TPE_PROTOS_USERS_H
