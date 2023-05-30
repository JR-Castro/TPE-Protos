#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <parser.h>
#include <macros.h>
#include <commands_parser.h>

enum states {
    S0_OK,
    S1_CMD_USER,
    S2_CMD_USER,
    S3_CMD_USER,
    S4_CMD_USER,
    S5_USERNAME,
    S6_USERNAME_INVALID,
    S7_CMD_PASS,
    S8_CMD_PASS,
    S9_CMD_PASS,
    S10_CMD_PASS,
    S11_PWD,
    S12_PWD_INVALID,
//    S13,
//    S14,
//    S15,
    S16_CMD_INVALID,
};

enum event_type {
    EVENT_OK,
    EVENT_CMD_USER,
    EVENT_USERNAME,
    EVENT_USERNAME_INVALID,
    EVENT_CMD_PASS,
    EVENT_PWD,
    EVENT_PWD_INVALID,
    EVENT_CMD_QUIT,
    EVENT_CMD_STAT,
    EVENT_CMD_LIST,
    EVENT_CMD_RETR,
    EVENT_CMD_DELE,
    EVENT_CMD_NOOP,
    EVENT_CMD_RSET,
    EVENT_CMD_TOP,
    EVENT_CMD_INVALID,
};

static void
command_user(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_CMD_USER;
    ret->n = 1;
    ret->data[0] = c;
}

static void
username(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_USERNAME;
    ret->n = 1;
    ret->data[0] = c;
}

static void
username_invalid(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_USERNAME_INVALID;
    ret->n = 1;
    ret->data[0] = c;
}

static void
st_ok(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_OK;
    ret->n = 1;
    ret->data[0] = c;
}

static void
command_pass(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_CMD_PASS;
    ret->n = 1;
    ret->data[0] = c;
}

static void
pwd(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_PWD;
    ret->n = 1;
    ret->data[0] = c;
}

static void
pwd_invalid(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_PWD_INVALID;
    ret->n = 1;
    ret->data[0] = c;
}

//static void
//command_quit(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_QUIT;
//    ret->n = 4;
//    strncpy(ret->data, "QUIT", 4);
//}
//
//static void
//command_stat(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_STAT;
//    ret->n = 4;
//    strncpy(ret->data, "STAT", 4);
//}
//
//static void
//command_list(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_LIST;
//    ret->n = 4;
//    strncpy(ret->data, "LIST", 4);
//}
//
//static void
//command_retr(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_RETR;
//    ret->n = 4;
//    strncpy(ret->data, "RETR", 4);
//}
//
//static void
//command_dele(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_DELE;
//    ret->n = 4;
//    strncpy(ret->data, "DELE", 4);
//}
//
//static void
//command_noop(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_NOOP;
//    ret->n = 4;
//    strncpy(ret->data, "NOOP", 4);
//}
//
//static void
//command_rset(struct parser_event *ret, const uint8_t c) {
//    ret->type = COMMAND_RSET;
//    ret->n = 4;
//    strncpy(ret->data, "RSET", 4);
//}
//
//static void
//command_top(struct parser_event *ret, const uint8_t c) {
//
//}

static void
command_invalid(struct parser_event *ret, const uint8_t c) {
    ret->type = EVENT_CMD_INVALID;
    ret->n = 1;
    ret->data[0] = c;
}

static const struct parser_state_transition ST_S0_OK[] = {
        {.when = 'U',     .dest = S1_CMD_USER,          .act1 = command_user,},
        {.when = 'P',     .dest = S7_CMD_PASS,          .act1 = command_pass,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S1_CMD_USER[] = {
        {.when = 'S',     .dest = S2_CMD_USER,          .act1 = command_user,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S2_CMD_USER[] = {
        {.when = 'E',     .dest = S3_CMD_USER,          .act1 = command_user,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S3_CMD_USER[] = {
        {.when = 'R',     .dest = S4_CMD_USER,          .act1 = command_user,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S4_CMD_USER[] = {
        {.when = ' ',     .dest = S5_USERNAME,          .act1 = username,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S5_USERNAME[] = {
        {.when = USERNAME_VALID_CHAR, .dest = S5_USERNAME,          .act1 = username,},
        {.when = '\n',                .dest = S0_OK,                .act1 = st_ok,},
        {.when = '\r',                .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
        {.when = ANY,                 .dest = S6_USERNAME_INVALID,  .act1 = username_invalid,},
};

static const struct parser_state_transition ST_S6_USERNAME_INVALID[] = {
        {.when = ANY,     .dest = S6_USERNAME_INVALID,  .act1 = username_invalid,},
};

static const struct parser_state_transition ST_S7_CMD_PASS[] = {
        {.when = 'A',     .dest = S8_CMD_PASS,          .act1 = command_pass,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S8_CMD_PASS[] = {
        {.when = 'S',     .dest = S9_CMD_PASS,          .act1 = command_pass,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S9_CMD_PASS[] = {
        {.when = 'S',     .dest = S10_CMD_PASS,         .act1 = command_pass,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S10_CMD_PASS[] = {
        {.when = ' ',     .dest = S11_PWD,              .act1 = pwd,},
        {.when = ANY,     .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
};

static const struct parser_state_transition ST_S11_PWD[] = {
        {.when = PWD_VALID_CHAR,    .dest = S11_PWD,              .act1 = pwd,},
        {.when = '\n',              .dest = S0_OK,                .act1 = st_ok,},
        {.when = '\r',              .dest = S16_CMD_INVALID,      .act1 = command_invalid,},
        {.when = ANY,               .dest = S12_PWD_INVALID,      .act1 = pwd_invalid,},
};

static const struct parser_state_transition ST_S12_PWD_INVALID[] = {
        {.when = ANY,        .dest = S12_PWD_INVALID,      .act1 = pwd_invalid,},
};

//static const struct parser_state_transition ST_S13[] = {
//        {.when = ANY, .dest = S10, .act1 = command_stat,},
//};
//
//static const struct parser_state_transition ST_S14[] = {
//        {.when = 'I', .dest = S10, .act1 = command_list,},
//};
//
//static const struct parser_state_transition ST_S15[] = {
//        {.when = 'I', .dest = S10, .act1 = command_list,},
//};
//
static const struct parser_state_transition ST_S16_CMD_INVALID[] = {
        {.when = ANY,  .dest = S16_CMD_INVALID,  .act1 = command_invalid,},
};

static const struct parser_state_transition *states[] = {
        ST_S0_OK,
        ST_S1_CMD_USER,
        ST_S2_CMD_USER,
        ST_S3_CMD_USER,
        ST_S4_CMD_USER,
        ST_S5_USERNAME,
        ST_S6_USERNAME_INVALID,
        ST_S7_CMD_PASS,
        ST_S8_CMD_PASS,
        ST_S9_CMD_PASS,
        ST_S10_CMD_PASS,
        ST_S11_PWD,
        ST_S12_PWD_INVALID,
//        ST_S13,
//        ST_S14,
//        ST_S15,
        ST_S16_CMD_INVALID,
};

static const size_t states_n[] = {
        N(ST_S0_OK),
        N(ST_S1_CMD_USER),
        N(ST_S2_CMD_USER),
        N(ST_S3_CMD_USER),
        N(ST_S4_CMD_USER),
        N(ST_S5_USERNAME),
        N(ST_S6_USERNAME_INVALID),
        N(ST_S7_CMD_PASS),
        N(ST_S8_CMD_PASS),
        N(ST_S9_CMD_PASS),
        N(ST_S10_CMD_PASS),
        N(ST_S11_PWD),
        N(ST_S12_PWD_INVALID),
//        N(ST_S13),
//        N(ST_S14),
//        N(ST_S15),
        N(ST_S16_CMD_INVALID),
};

// Parser definintion
struct parser_definition cmd_parser_definition = {
        .states_count = N(states),
        .states       = states,
        .states_n     = states_n,
        .start_state  = S0_OK,
};
