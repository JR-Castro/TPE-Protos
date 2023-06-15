#include <stdlib.h>
#include <check.h>

#include "users.h"

START_TEST(test_user_add)
    {
        user_add("test", "test");
        ck_assert_int_eq(user_exists("test"), 1);
    }
END_TEST

START_TEST(test_user_remove)
    {
        user_add("test", "test");
        user_remove("test");
        ck_assert_int_eq(user_exists("test"), 0);
    }
END_TEST

START_TEST(test_user_authenticate)
    {
        user_add("test", "test");
        ck_assert_int_eq(user_authenticate("test", "test"), 1);
    }
END_TEST

START_TEST(test_user_change_password)
    {
        user_add("test", "test");
        user_change_password("test", "test2");
        ck_assert_int_eq(user_authenticate("test", "test2"), 1);
    }
END_TEST

START_TEST(test_user_change_username)
    {
        user_add("test", "test");
        user_change_username("test", "test2");
        ck_assert_int_eq(user_exists("test2"), 1);
        ck_assert_int_eq(user_exists("test"), 0);
    }
END_TEST

Suite *users_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Users");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_user_add);
    tcase_add_test(tc_core, test_user_remove);
    tcase_add_test(tc_core, test_user_authenticate);
    tcase_add_test(tc_core, test_user_change_password);
    tcase_add_test(tc_core, test_user_change_username);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = users_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
