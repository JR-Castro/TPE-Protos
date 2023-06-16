#include <string.h>

#include "users.h"

static struct user users[MAX_USERS];
static size_t users_count = 0;

int user_authenticate(char *username, char *password) {
    for (int i = 0; i < users_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (strcmp(users[i].password, password) == 0) {
                return 1;
            }
            break;
        }
    }
    return 0;
}

int user_add(char *username, char *password) {
    if (users_count == MAX_USERS) return -1;
    strncpy(users[users_count].username, username, MAX_USERNAME);
    strncpy(users[users_count].password, password, MAX_PASSWORD);
    users_count++;
    return 0;
}

int user_add_basic(char *userpass) {
    if (users_count == MAX_USERS) return -1;
    char *username = strtok(userpass, ":");
    char *password = strtok(NULL, ":");
    return user_add(username, password);
}

int user_remove(char *username) {
    for (int i = 0; i < users_count; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            memmove(&users[i], &users[i + 1], sizeof(struct user) * (users_count - i - 1));
            users_count--;
            return 1;
        }
    }
    return 0;
}

int user_exists(char *username) {
    for (int i = 0; i < users_count; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int user_change_password(char *username, char *new_password) {
    for (int i = 0; i < users_count; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            strncpy(users[i].password, new_password, MAX_PASSWORD);
            return 1;
        }
    }
    return 0;
}

int user_change_username(char *username, char *new_username) {
    for (int i = 0; i < users_count; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            strncpy(users[i].username, new_username, MAX_USERNAME);
            return 1;
        }
    }
    return 0;
}