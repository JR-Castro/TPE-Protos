#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"


START_TEST (test_is_alphanum) {
    ck_assert_int_eq(1, (int)isAlphanum('f'));
    ck_assert_int_eq(1, (int)isAlphanum('F'));
    ck_assert_int_eq(1, (int)isAlphanum('o'));
    ck_assert_int_eq(1, (int)isAlphanum('O'));
    ck_assert_int_eq(1, (int)isAlphanum('5'));
    ck_assert_int_eq(1, (int)isAlphanum('0'));
    ck_assert_int_eq(0, (int)isAlphanum('_'));
    ck_assert_int_eq(0, (int)isAlphanum('#'));
    ck_assert_int_eq(0, (int)isAlphanum('!'));
    ck_assert_int_eq(0, (int)isAlphanum('-'));
    ck_assert_int_eq(0, (int)isAlphanum(' '));
}
END_TEST

START_TEST (test_is_valid_username_char) {
    ck_assert_int_eq(1, (int)isValidUsernameChar('a'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('f'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('F'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('o'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('O'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('5'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('0'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('_'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('-'));
    ck_assert_int_eq(1, (int)isValidUsernameChar('.'));

    ck_assert_int_eq(0, (int)isValidUsernameChar('!'));
    ck_assert_int_eq(0, (int)isValidUsernameChar('#'));
    ck_assert_int_eq(0, (int)isValidUsernameChar('?'));
    ck_assert_int_eq(0, (int)isValidUsernameChar(' '));
    ck_assert_int_eq(0, (int)isValidUsernameChar(','));
    ck_assert_int_eq(0, (int)isValidUsernameChar('<'));
}
END_TEST

START_TEST (test_is_valid_pwd_char) {
    ck_assert_int_eq(1, (int)isValidPwdChar('a'));
    ck_assert_int_eq(1, (int)isValidPwdChar('f'));
    ck_assert_int_eq(1, (int)isValidPwdChar('F'));
    ck_assert_int_eq(1, (int)isValidPwdChar('o'));
    ck_assert_int_eq(1, (int)isValidPwdChar('O'));
    ck_assert_int_eq(1, (int)isValidPwdChar('5'));
    ck_assert_int_eq(1, (int)isValidPwdChar('0'));
    ck_assert_int_eq(1, (int)isValidPwdChar('_'));
    ck_assert_int_eq(1, (int)isValidPwdChar('-'));
    ck_assert_int_eq(1, (int)isValidPwdChar('.'));
    ck_assert_int_eq(1, (int)isValidPwdChar('!'));
    ck_assert_int_eq(1, (int)isValidPwdChar('#'));
    ck_assert_int_eq(1, (int)isValidPwdChar('$'));
    ck_assert_int_eq(1, (int)isValidPwdChar('*'));

    ck_assert_int_eq(0, (int)isValidPwdChar('?'));
    ck_assert_int_eq(0, (int)isValidPwdChar(' '));
    ck_assert_int_eq(0, (int)isValidPwdChar(','));
    ck_assert_int_eq(0, (int)isValidPwdChar('<'));
}
END_TEST

Suite *
suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("util_tests");

    /* Core tests case */
    tc = tcase_create("util_tests");

    tcase_add_test(tc, test_is_alphanum);
    tcase_add_test(tc, test_is_valid_username_char);
    tcase_add_test(tc, test_is_valid_pwd_char);
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