#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "buffer.h"
#include "macros.h"
#include "parser.h"
#include "commands_parser.h"
#include "../src/parsers/commands_parser.c"

#define CMD_MAX_LENGTH 255

START_TEST (test_parse_one_arg_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'U', 'S', 'E', 'R', ' ',
            'f', 'o', 'o', '\r', '\n'
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // U
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // S
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // E
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // R
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'f'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'o'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'o'
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_OK,       parse_byte_command(b, p)); // '\n'
    ck_assert_uint_eq('U', *(p->command->data));
    ck_assert_uint_eq('f', *(p->command->args1));
    ck_assert_ptr_eq(NULL, p->command->args2);

    command_parser_destroy(p);
} END_TEST

START_TEST (test_parse_two_args_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'T', 'O', 'P', ' ',
            '1', '0', ' ',
            '5', '\r', '\n'
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // T
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // O
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // P
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_ptr_eq(NULL,         p->command->args1);
    ck_assert_ptr_eq(NULL,         p->command->args2);
    ck_assert_int_eq(CMD_DISPATCH, p->prev_state);

    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '1'
    ck_assert_int_eq(CMD_ARGS,     p->prev_state);
    ck_assert_ptr_eq(NULL,         p->command->args2);
    ck_assert_uint_eq('1',         *(p->command->args1));

    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '0'
    ck_assert_int_eq(CMD_ARGS,     p->prev_state);
    ck_assert_ptr_eq(NULL,         p->command->args2);
    ck_assert_uint_eq('1',         *(p->command->args1));

    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_DISPATCH, p->prev_state);

    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '5'
    ck_assert_uint_eq('1',         *(p->command->args1));
    ck_assert_uint_eq('5',         *(p->command->args2));
    ck_assert_int_eq(CMD_ARGS,     p->prev_state);

    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_OK,       parse_byte_command(b, p)); // '\n'
    ck_assert_uint_eq('T', *(p->command->data));
    ck_assert_uint_eq('1', *(p->command->args1));
    ck_assert_uint_eq('5', *(p->command->args2));

    command_parser_destroy(p);
} END_TEST

START_TEST(test_parse_no_crlf_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'B', ' ',
            'f', '\r', '\r'
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // B
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'f'
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_INVALID,  parse_byte_command(b, p)); // '\r'

    command_parser_destroy(p);
} END_TEST

START_TEST(test_parse_duplicated_spaces_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'U', 'T', ' ', ' ', '\t', ' ',
            'f', 'o', 'o', '\r', '\n'
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // U
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // T
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '\t'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'f'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'o'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // 'o'
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_OK,       parse_byte_command(b, p)); // '\n'
    ck_assert_uint_eq('U', *(p->command->data));
    ck_assert_uint_eq('f', *(p->command->args1));
    ck_assert_ptr_eq(NULL, p->command->args2);

    command_parser_destroy(p);
} END_TEST

START_TEST(test_parse_no_arg_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'L', 'I', 'S', 'T', '\r', '\n',
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // L
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // I
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // S
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // T
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_OK,       parse_byte_command(b, p)); // '\n'
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    command_parser_destroy(p);
} END_TEST

START_TEST(test_parse_invalid_valid_command) {
    struct command_parser *p = command_parser_init();
    ck_assert_ptr_nonnull(p);
    ck_assert_int_eq(p->state, CMD_DISPATCH);
    ck_assert_ptr_eq(NULL, p->command->args1);
    ck_assert_ptr_eq(NULL, p->command->args2);

    struct buffer buf;
    buffer *b = &buf;
    uint8_t data[CMD_MAX_LENGTH];
    buffer_init(&buf, N(data), data);
    ck_assert_ptr_eq(&buf, b);
    ck_assert_int_eq(true,  buffer_can_write(b));
    ck_assert_int_eq(false, buffer_can_read(b));
    size_t wbytes = 0;
    uint8_t *ptr = buffer_write_ptr(b, &wbytes);
    ck_assert_uint_eq(CMD_MAX_LENGTH, wbytes);
    uint8_t user_cmd_input [] = {
            'B', ' ', '\r', ' ', '1', '\r', '\n',
            'T', 'O', 'P', ' ', '1', ' ', '5',
            '\r', '\n',
    };
    memcpy(ptr, user_cmd_input, sizeof(user_cmd_input));
    buffer_write_adv(b, sizeof(user_cmd_input));

    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // B
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_INVALID,  parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_INVALID,  parse_byte_command(b, p)); // '1'
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // '\n'
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // T
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // O
    ck_assert_int_eq(CMD_DISPATCH, parse_byte_command(b, p)); // P
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '1'
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // ' '
    ck_assert_int_eq(CMD_ARGS,     parse_byte_command(b, p)); // '5'
    ck_assert_int_eq(CMD_CRLF,     parse_byte_command(b, p)); // '\r'
    ck_assert_int_eq(CMD_OK,       parse_byte_command(b, p)); // '\n'

    ck_assert_uint_eq('T', *(p->command->data));
    ck_assert_uint_eq('1', *(p->command->args1));
    ck_assert_uint_eq('5', *(p->command->args2));

    command_parser_destroy(p);
} END_TEST

Suite *
suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("cmd_parser_utils");
    tc = tcase_create("cmd_parser_utils");

    tcase_add_test(tc, test_parse_no_arg_command);
    tcase_add_test(tc, test_parse_two_args_command);
    tcase_add_test(tc, test_parse_no_crlf_command);
    tcase_add_test(tc, test_parse_duplicated_spaces_command);
    tcase_add_test(tc, test_parse_invalid_valid_command);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
