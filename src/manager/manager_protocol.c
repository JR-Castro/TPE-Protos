#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>

#include "manager_protocol.h"

static void buffer_to_data(union manager_current_data *output, enum manager_data_type type, uint8_t *raw);
static void data_to_buffer(union manager_current_data data, enum manager_data_type type, uint8_t *output);

int manager_packet_to_request(uint8_t *raw, struct manager_request *request) {
    if (raw == NULL || request == NULL) {
        return -1;
    }

    /* Convert version */
    request->version = *raw;
    raw += sizeof(uint8_t);

    /* Convert type */
    request->type = *raw;
    raw += sizeof(uint8_t);

    /* Convert cmd */
    request->cmd = ntohs(*(uint16_t *) raw);
    raw += sizeof(uint16_t);

    /* Convert id */
    request->id = ntohs(*(uint16_t *) raw);
    raw += sizeof(uint16_t);

    /* Convert token */
    request->token = ntohl(*(uint32_t *) raw);
    raw += sizeof(uint32_t);

    buffer_to_data(&request->data,
                   get_req_data_type(request->type, request->cmd),
                   raw);

    return 0;
}

enum manager_data_type get_req_data_type(enum manager_type type, uint16_t cmd) {
    switch (type) {
        case TYPE_GET:
            if (cmd == GET_USERS)
                return STRING_DATA;
            return EMPTY_DATA;
        case TYPE_SET:
            switch (cmd) {
                case ADD_USER:
                case DEL_USER:
                    return STRING_DATA;
                case SET_PAGE_SIZE:
                    return UINT8_DATA;
                case STOP_SERVER:
                default:
                    return EMPTY_DATA;
            }
    }
}

enum manager_data_type get_res_data_type(enum manager_type type, uint16_t cmd) {
    switch (type) {
        case TYPE_GET:
            switch (cmd) {
                case GET_USERS:
                    return STRING_DATA;
                case GET_HISTORIC:
                case GET_SENT:
                    return UINT64_DATA;
                case GET_CONCURRENT:
                    return UINT16_DATA;
                case GET_PAGE_SIZE:
                    return UINT8_DATA;
                default:
                        return EMPTY_DATA;
            }
        case TYPE_SET:
            return EMPTY_DATA;
    }
}

static void buffer_to_data(union manager_current_data *output, enum manager_data_type type, uint8_t *raw) {
    switch (type) {
        case UINT8_DATA:
            output->uint8_data = *((uint8_t*) raw);
            break;
        case UINT16_DATA:
            output->uint16_data = *((uint16_t*) raw);
            break;
        case UINT64_DATA:
            output->uint64_data = *((uint64_t*) raw);
            break;
        case STRING_DATA:
            strcpy(output->string, (char*) raw);
            break;
        case EMPTY_DATA:
        default:
            output->string[0] = 0;

    }
}

static void data_to_buffer(union manager_current_data data, enum manager_data_type type, uint8_t *output) {

    switch (type) {
        case UINT8_DATA:
            *((uint8_t*) output) = data.uint8_data;
            break;
        case UINT16_DATA:
            *((uint16_t*) output) = htons(data.uint16_data);
            break;
        case UINT64_DATA:
            *((uint64_t*) output) = htonl(data.uint64_data);
            break;
        case STRING_DATA:
            strcpy((char*) output, data.string);
            break;
        case EMPTY_DATA:
        default:
            break;
    }

}

static size_t get_packet_size(enum manager_packet_type packetType, enum manager_type type, uint16_t cmd, uint8_t *data) {
    size_t size = 0;
    enum manager_data_type dataType;
    if (packetType == REQUEST) {
        dataType = get_req_data_type(type, cmd);
        size += MANAGER_REQUEST_HEADER_SIZE;
    }
    else {
        dataType = get_res_data_type(type, cmd);
        size += MANAGER_RESPONSE_HEADER_SIZE;
    }

    switch (dataType) {
        case UINT8_DATA:
            size += sizeof(uint8_t);
            break;
        case UINT16_DATA:
            size += sizeof(uint16_t);
            break;
        case UINT64_DATA:
            size += sizeof(uint64_t);
            break;
        case STRING_DATA:
            size += (data != NULL) ? strlen((char*) data) + 1 : 0;
            break;
        default:
            break;
    }

    return size;
}

int manager_response_to_packet(struct manager_response *response, uint8_t *output, size_t *output_size) {
    if (response == NULL || output == NULL || output_size == NULL) {
        return -1;
    }

    int aux;
    *output_size = get_packet_size(RESPONSE, response->type, response->cmd, NULL);
    uint8_t *buffer_p = output;

    /* Convert version */
    // Since they are one byte long, we can just copy them
    *buffer_p = response->version;
    buffer_p += sizeof(uint8_t);

    /* Convert status */
    // Since they are one byte long, we can just copy them
    *buffer_p = response->status;
    buffer_p += sizeof(uint8_t);

    /* Convert type */
    *output = response->type;
    output += sizeof(uint8_t);

    /* Convert cmd */
    *(uint16_t *) output = htons(response->cmd);
    output += sizeof(uint16_t);

    /* Convert id */
    *(uint16_t *) output = htons(response->id);
    output += sizeof(uint16_t);

    /* Convert data */
    if (response->status == SC_OK)
        data_to_buffer(response->data, get_res_data_type(response->type, response->cmd), output);

    return 0;
}

int manager_packet_to_response(uint8_t *raw, struct manager_response *response) {
    if (raw == NULL || response == NULL) {
        return -1;
    }

    /* Convert version */
    // Since they are one byte long, we can just copy them
    response->version = *raw;
    raw += sizeof(uint8_t);

    /* Convert status */
    // Since they are one byte long, we can just copy them
    response->status = *raw;
    raw += sizeof(uint8_t);

    /* Convert type */
    response->type = *raw;
    raw += sizeof(uint8_t);

    /* Convert cmd */
    response->cmd = ntohs(*(uint16_t *) raw);
    raw += sizeof(uint16_t);

    response->id = ntohs(*(uint16_t *) raw);
    raw += sizeof(uint16_t);

    /* Convert data */
    if (response->status == SC_OK)
        buffer_to_data(&response->data, get_res_data_type(response->type, response->cmd), raw);

    return 0;
}

int manager_request_to_packet(struct manager_request *request, uint8_t *output, size_t *output_size) {
    if (request == NULL || output == NULL || output_size == NULL) {
        return -1;
    }

    *output_size = get_packet_size(REQUEST, request->type, request->cmd, request->data.string);
    uint8_t *buffer_p = output;

    /* Convert version */
    // Since they are one byte long, we can just copy them
    *buffer_p = request->version;
    buffer_p += sizeof(uint8_t);

    /* Convert type */
    // Since they are one byte long, we can just copy them
    *buffer_p = request->type;
    buffer_p += sizeof(uint8_t);

    /* Convert cmd */
    *(uint16_t *) buffer_p = htons(request->cmd);
    buffer_p += sizeof(uint16_t);

    /* Convert id */
    *(uint16_t *) buffer_p = htons(request->id);
    buffer_p += sizeof(uint16_t);

    /* Convert token */
    *(uint64_t *) buffer_p = htonl(request->token);
    buffer_p += sizeof(uint64_t);

    /* Convert data */
    data_to_buffer(request->data, get_req_data_type(request->type, request->cmd), buffer_p);

    return 0;
}
