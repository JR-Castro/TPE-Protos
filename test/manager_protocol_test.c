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
            0x02,             // status
            0x03,             // type
            0x01, 0x00,   // cmd (in network byte order, big-endian)
            0x02, 0x00    // id  (in network byte order, big-endian)
    };

    struct manager_response expected_response;
    expected_response.version = 0x01;
    expected_response.status  = 0x02;
    expected_response.type    = 0x03;
    expected_response.cmd     = 0x0001;   // ntohs(0x0100)
    expected_response.id      = 0x0002;   // ntohs(0x0200)

    struct manager_response response;
    int result = manager_packet_to_response(raw_packet, &response);

    ck_assert_int_eq(result, 0);
//    ck_assert_response(response, expected_response);
//    ck_assert_mem_eq(&response, &expected_response, sizeof(struct manager_response));
} END_TEST

START_TEST(test_manager_request_to_packet) {
    // Create a request structure for testing
    struct manager_request request;
    request.version = 0x01;
    request.type    = 0x02;
    request.cmd     = 0x001;
    request.id      = 0x0005;
    request.token   = 0x12345678;

    // Define the expected output
    uint8_t expected_output[] = {
            0x01,                               // version
            0x02,                               // type
            0x01, 0x00,                     // cmd (in network byte order)
            0x02, 0x00,                     // id  (in network byte order)
            0x78, 0x56, 0x34, 0x12  // token (in network byte order)
    };

    uint8_t output[30];
    size_t output_size;

    int result = manager_request_to_packet(&request, output, &output_size);

    ck_assert_int_eq(result, 0);
    // Check the output buffer and size
    ck_assert_int_eq(output_size, sizeof(expected_output));
    ck_assert_mem_eq(&output, &expected_output, output_size);
    // Print the output buffer and size for verification (optional)
    printf("Output Size: %zu\n", output_size);
    for (size_t i = 0; i < output_size; i++) {
        printf("%02x ", output[i]);
    }
    printf("\n");
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