#include <stdlib.h>
#include <check.h>
#include "users.h"

// 1. testear que permita atender a múltiples clientes en forma concurrente y simultánea (al menos 500).
START_TEST(test_multiple_clients) {
    ck_assert_int_eq(1, 1);
}
END_TEST

// 2.  testear que soporte autenticación usuario / contraseña y pipelining [RFC2449].
START_TEST(test_authentication) {
//    ck_assert_int_eq(1, 1);
    user_add("user", "pass");

    ck_assert_int_eq(user_exists("user"), 1);
    ck_assert_int_eq(user_authenticate("user", "pass"), 1);
}
END_TEST

// 3.  testear que soporte conexiones IPv4 e IPV6.
START_TEST(test_ipv4) {
    ck_assert_int_eq(1, 1);
}
END_TEST

START_TEST(test_ipv6) {
    ck_assert_int_eq(1, 1);
}
END_TEST

//START_TEST(test_pipelining) {
//    ck_assert_int_eq(1, 1);
//}

// 4.  reportar los fallos a los clientes usando toda la potencia del protocolo.
START_TEST(test_report_failures) {
    ck_assert_int_eq(1, 1);
}
END_TEST

// 5.  implementar mecanismos que permitan recolectar métricas que ayuden a monitorear la operación del sistema.
START_TEST(test_metrics) {
    ck_assert_int_eq(1, 1);
}
END_TEST

// 6. implementar mecanismos que permitan a los usuarios cambiar la configuración del servidor en tiempo de ejecución
//    sin reiniciar el servidor.  Las diferentes implementaciones PUEDEN decidir disponibilizar otros cambios de
//    ejecución en tiempo de ejecución de otras configuraciones (memoria utilizada en I/O, timeouts,etc).
START_TEST(test_config) {
    ck_assert_int_eq(1, 1);
}
END_TEST

//7.  implementar un registro de acceso que permitan a un administrador entender los accesos de cada uno de los usuarios.
//    Pensar en el caso de que llega una queja externa y el administrador debe saber quien fue el que se conectó a cierto sitio
//    web y cuando.
START_TEST(test_access_log) {
    ck_assert_int_eq(1, 1);
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
    tcase_add_test(tc_core, test_ipv4);
    tcase_add_test(tc_core, test_ipv6);
    tcase_add_test(tc_core, test_report_failures);
    tcase_add_test(tc_core, test_metrics);
    tcase_add_test(tc_core, test_config);
    tcase_add_test(tc_core, test_access_log);

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
