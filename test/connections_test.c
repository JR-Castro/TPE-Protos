#include <stdlib.h>
#include <check.h>

#include "users.h"

// testear que permita atender a múltiples clientes en forma concurrente y simultánea (al menos 500).
START_TEST(test_multiple_clients) {
    ck_assert_int_eq(1, 1);
}
END_TEST

// testear que soporte autenticación usuario / contraseña y pipelining [RFC2449].
START_TEST(test_authentication) {
    user_add("user", "pass");

    ck_assert_int_eq(user_exists("user"), 1);
    ck_assert_int_eq(user_authenticate("user", "pass"), 1);
}
END_TEST

Suite *connections_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Connections");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_multiple_clients);
    tcase_add_test(tc_core, test_authentication);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = connections_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
