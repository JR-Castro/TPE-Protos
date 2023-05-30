#include <stdio.h>
#include <stdlib.h>
#include <check.h>

#include <macros.h>
#include <parser.h>

#include "../src/parsers/commands_parser.c"

static void assert_event(const unsigned type, const int c, const struct parser_event *e) {
    ck_assert_ptr_eq(NULL, e->next);
    ck_assert_uint_eq(1, e->n);
    ck_assert_uint_eq(type, e->type);
    ck_assert_uint_eq(c, e->data[0]);
}

START_TEST (test_parse_valid_username) {
    // Parser initialization
    struct parser *parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    // Test valid USER + username command
    assert_event(EVENT_CMD_USER, 'U', parser_feed(parser, 'U'));
    assert_event(EVENT_CMD_USER, 'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_USER, 'E', parser_feed(parser, 'E'));
    assert_event(EVENT_CMD_USER, 'R', parser_feed(parser, 'R'));
    assert_event(EVENT_USERNAME, ' ', parser_feed(parser, ' '));
    assert_event(EVENT_USERNAME, 'N', parser_feed(parser, 'N'));
    assert_event(EVENT_USERNAME, '1', parser_feed(parser, '1'));
    assert_event(EVENT_USERNAME, 'c', parser_feed(parser, 'c'));
    assert_event(EVENT_USERNAME, 'k', parser_feed(parser, 'k'));
    assert_event(EVENT_OK, '\n', parser_feed(parser, '\n'));

    parser_destroy(parser);
}
END_TEST

START_TEST (test_parse_invalid_username) {
    // Parser initialization
    struct parser *parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    assert_event(EVENT_CMD_USER,    'U', parser_feed(parser, 'U'));
    assert_event(EVENT_CMD_USER,    'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_INVALID, 'R', parser_feed(parser, 'R'));
    assert_event(EVENT_CMD_INVALID, ' ', parser_feed(parser, ' '));
    assert_event(EVENT_CMD_INVALID, 'N', parser_feed(parser, 'N'));
    assert_event(EVENT_CMD_INVALID, '1', parser_feed(parser, '1'));
    assert_event(EVENT_CMD_INVALID, '\n', parser_feed(parser, '\n'));

    // Parser initialization
    parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    // Test invalid username
    assert_event(EVENT_CMD_USER,         'U', parser_feed(parser, 'U'));
    assert_event(EVENT_CMD_USER,         'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_USER,         'E', parser_feed(parser, 'E'));
    assert_event(EVENT_CMD_USER,         'R', parser_feed(parser, 'R'));
    assert_event(EVENT_USERNAME,         ' ', parser_feed(parser, ' '));
    assert_event(EVENT_USERNAME,         'N', parser_feed(parser, 'N'));
    assert_event(EVENT_USERNAME,         '1', parser_feed(parser, '1'));
    assert_event(EVENT_USERNAME_INVALID, '@', parser_feed(parser, '@'));

    // Parser initialization
    parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    assert_event(EVENT_CMD_USER, 'U', parser_feed(parser, 'U'));
    assert_event(EVENT_CMD_USER, 'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_USER, 'E', parser_feed(parser, 'E'));
    assert_event(EVENT_CMD_USER, 'R', parser_feed(parser, 'R'));
    assert_event(EVENT_USERNAME, ' ', parser_feed(parser, ' '));
    assert_event(EVENT_USERNAME, 'N', parser_feed(parser, 'N'));
    assert_event(EVENT_USERNAME, '1', parser_feed(parser, '1'));
    assert_event(EVENT_USERNAME, 'c', parser_feed(parser, 'c'));
    assert_event(EVENT_USERNAME, 'k', parser_feed(parser, 'k'));
    // End of a command must be '\n'
    assert_event(EVENT_CMD_INVALID, '\r', parser_feed(parser, '\r'));

    parser_destroy(parser);
}


START_TEST (test_parse_empty_username) {
    // Parser initialization
    struct parser *parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    assert_event(EVENT_CMD_USER,    'U', parser_feed(parser, 'U'));
    assert_event(EVENT_CMD_USER,    'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_USER,    'E', parser_feed(parser, 'E'));
    assert_event(EVENT_CMD_USER,    'R', parser_feed(parser, 'R'));
    assert_event(EVENT_USERNAME,    ' ', parser_feed(parser, ' '));
    // For now, the parser is allowing empty usernames
    // TODO The caller should validates the username length
    assert_event(EVENT_OK,          '\n', parser_feed(parser, '\n'));

    parser_destroy(parser);
}
END_TEST

START_TEST (test_parse_valid_pwd) {
    // Parser initialization
    struct parser *parser = parser_init(parser_no_classes(), &cmd_parser_definition);
    ck_assert_uint_eq(S0_OK, cmd_parser_definition.start_state);

    assert_event(EVENT_CMD_PASS,    'P', parser_feed(parser, 'P'));
    assert_event(EVENT_CMD_PASS,    'A', parser_feed(parser, 'A'));
    assert_event(EVENT_CMD_PASS,    'S', parser_feed(parser, 'S'));
    assert_event(EVENT_CMD_PASS,    'S', parser_feed(parser, 'S'));
    assert_event(EVENT_PWD,         ' ', parser_feed(parser, ' '));
    assert_event(EVENT_PWD,         '1', parser_feed(parser, '1'));
    assert_event(EVENT_PWD,         '2', parser_feed(parser, '2'));
    assert_event(EVENT_PWD,         '3', parser_feed(parser, '3'));
    assert_event(EVENT_PWD,         '4', parser_feed(parser, '4'));
    assert_event(EVENT_OK,          '\n', parser_feed(parser, '\n'));

    parser_destroy(parser);
}
END_TEST

Suite *
suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("cmd_parser_utils");

    tc = tcase_create("cmd_parser_utils");

    tcase_add_test(tc, test_parse_valid_username);
    tcase_add_test(tc, test_parse_invalid_username);
    tcase_add_test(tc, test_parse_empty_username);
    tcase_add_test(tc, test_parse_valid_pwd);
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
