#include <stdlib.h>
#include <check.h>

#include "manager_protocol.h"
#include "buffer.h"

static void ck_assert_request(struct manager_request request, struct manager_request expected) {
    ck_assert_int_eq(request.version, expected.version);
    ck_assert_int_eq(request.type,    expected.type);
    ck_assert_int_eq(request.cmd,     expected.cmd);
    ck_assert_int_eq(request.id,      expected.id);
}

static void ck_assert_response(struct manager_response response, struct manager_response expected) {
    ck_assert_int_eq(response.version, expected.version);
    ck_assert_int_eq(response.status,  expected.status);
    ck_assert_int_eq(response.type,    expected.type);
    ck_assert_int_eq(response.cmd,     expected.cmd);
    ck_assert_int_eq(response.id,      expected.id);
}

/**
 *                    RESPONSE
 *   +-----+--------+------+-----+----+----------+
 *   | VER | STATUS | TYPE | CMD | ID |   DATA   |
 *   +-----+--------+------+-----+----+----------+
 *   |  1  |   1    |  1   |  2  |  2 | VARIABLE |
 *   +-----+--------+------+-----+----+----------+
 */
START_TEST(test_manager_packet_to_response) {
    // Raw packet for testing
    uint8_t raw_packet[] = {
            0x01,             // version
            0x00,             // status
            0x00,             // type
            0x00, 0x04,   // cmd (in network byte order, big-endian)
            0x00, 0x02    // id  (in network byte order, big-endian)
    };

    struct manager_response expected_response;
    expected_response.version = MANAGER_VERSION_1;
    expected_response.status  = SC_OK;
    expected_response.type    = TYPE_GET;
    expected_response.cmd     = GET_PAGE_SIZE;   // ntohs(0x0001)
    expected_response.id      = 2;               // ntohs(0x0002)

    struct manager_response response;
    int result = manager_packet_to_response(raw_packet, &response);

    ck_assert_int_eq(result, 0);
    ck_assert_response(response, expected_response);
    ck_assert_mem_eq(&response, &expected_response, sizeof(struct manager_response) - sizeof(union manager_current_data));
} END_TEST

START_TEST(test_manager_request_to_packet) {
    char *token = "12345678";
    // request structure for testing
    struct manager_request req;
    req.version = MANAGER_VERSION_1;
    req.type    = TYPE_SET;
    // Stop has no parameters so output should be the just a header
    req.cmd     = STOP_SERVER;
    req.id      = 5;
    req.token   = strtol(token, NULL, 16);

    // Expected output (should have big-endian byte order)
    uint8_t expected_output[] = {
            0x01,                               // version
            0x01,                               // type
            0x00, 0x00,                     // cmd (in network byte order)
            0x00, 0x05,                     // id  (in network byte order)
            0x12, 0x34, 0x56, 0x78  // token (in network byte order)
    };

    uint8_t output[30];
    size_t output_size;

    int result = manager_request_to_packet(&req, output, &output_size);

    ck_assert_int_eq(result, 0);
    ck_assert_int_eq(output_size, sizeof(expected_output));
    ck_assert_mem_eq(output, expected_output, output_size);
} END_TEST

Suite *
suite(void) {
    Suite *s   = suite_create("manager_protocol");
    TCase *tc  = tcase_create("manager_protocol");

    tcase_add_test(tc, test_manager_packet_to_response);
    tcase_add_test(tc, test_manager_request_to_packet);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    SRunner *sr  = srunner_create(suite());
    int number_failed;

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}