#ifndef TPE_PROTOS_USERS_H
#define TPE_PROTOS_USERS_H

#include <stdint.h>
#include "definitions.h"

struct user {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
};

/**
 * @brief Recevies a username and password and checks if they are valid
 * @param username
 * @param password
 * @return 1 if the user is valid, 0 otherwise
 */
int user_authenticate(char *username, char *password);

int user_add(char *username, char *password);

int user_add_basic(char *userpass);

int user_remove(char *username);

int user_exists(char *username);

int user_change_password(char *username, char *new_password);

int user_change_username(char *username, char *new_username);

size_t user_count();

char *user_get_username(size_t index);

#endif //TPE_PROTOS_USERS_H
