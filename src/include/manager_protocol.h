#ifndef TPE_PROTOS_MANAGER_PROTOCOL_H
#define TPE_PROTOS_MANAGER_PROTOCOL_H

#include "definitions.h"

enum manager_packet_type {
    REQUEST,
    RESPONSE
};

enum manager_version {
    MANAGER_VERSION_1 = 1
};

enum manager_type {
    TYPE_GET,
    TYPE_SET
};

enum manager_get_cmd {
    GET_USERS = 0,
    GET_HISTORIC,
    GET_CONCURRENT,
    GET_SENT,
    GET_PAGE_SIZE,
};
#define GET_CMD_QTY 5

enum manager_set_cmd {
    STOP_SERVER = 0,
    ADD_USER,
    DEL_USER,
    SET_PAGE_SIZE,
};
#define SET_CMD_QTY 4

enum manager_status_code {
    SC_OK = 0,
    SC_INVALID_VERSION,
    SC_INVALID_TOKEN,
    SC_INVALID_TYPE,
    SC_INVALID_CMD,
    SC_INVALID_ARG,
    SC_USERS_FULL,
    SC_USER_ALREADY_EXISTS,
    SC_USER_NOT_FOUND,
    SC_SERVER_ERROR,
};

enum manager_data_type {
    UINT8_DATA,
    UINT16_DATA,
    UINT64_DATA,
    STRING_DATA,
    EMPTY_DATA,
};

union manager_current_data {
    uint8_t uint8_data;
    uint16_t uint16_data;
    uint64_t uint64_data;
    uint8_t string[MAX_UDP_SIZE - MANAGER_REQUEST_HEADER_SIZE];
};

/*          REQUEST HEADER
 +------+-------+-----+-----+---------+
 | VER  | TYPE  | CMD |  ID |  TOKEN  |
 +------+-------+-----+-----+---------+
 |  1   |  1    |  2  |  2  |    4    |
 +------+-------+-----+-----+---------+
*/

struct manager_request {
    enum manager_version version;
    enum manager_type type;
    uint16_t cmd;
    uint16_t id;
    uint32_t token;
    union manager_current_data data;
};

struct manager_response {
    enum manager_version version;
    enum manager_status_code status;
    enum manager_type type;
    uint16_t cmd;
    uint16_t id;
    union manager_current_data data;
};

int manager_packet_to_request(uint8_t *raw, struct manager_request *request);

int manager_response_to_packet(struct manager_response *response, uint8_t *output, size_t *output_size);

int manager_packet_to_response(uint8_t *raw, struct manager_response *response);

int manager_request_to_packet(struct manager_request *request, uint8_t *output, size_t *output_size);

enum manager_data_type get_req_data_type(enum manager_type type, uint16_t cmd);

#endif //TPE_PROTOS_MANAGER_PROTOCOL_H
