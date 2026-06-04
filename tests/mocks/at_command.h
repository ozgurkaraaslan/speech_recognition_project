#ifndef MOCK_AT_COMMAND_H
#define MOCK_AT_COMMAND_H

#include <stdint.h>
#include <string.h>

/* Mock error codes */
typedef enum {
    E_AT_ERR_NONE = 0,
    E_AT_ERR_TIMEOUT,
    E_AT_ERR_UNEXPECTED_RESPONSE,
    E_AT_ERR_HW_ERROR,
    E_AT_ERR_WRONG_IOCTL_CMD,
    E_AT_ERR_UNKNOWN
} atCommandErrorCodes_t;

/* Mock IOCTL commands */
typedef enum {
    E_AT_IOCTL_SEND_CMD = 0,
    E_AT_IOCTL_SEND_RAW
} AT_IOCTL_COMMANDS_T;

#define AT_MAX_LINES 8

/* Mock structs from the real at_command.h */
typedef struct {
    const char* command;
    const char* expected_resp;
    uint32_t timeout_ms;
    char* resp_buffer;
    uint16_t resp_buffer_len;
    char* lines[AT_MAX_LINES];
    uint8_t line_count;
} AtCommandReq_t;

typedef struct {
    uint8_t* data;
    uint16_t length;
    uint32_t timeout_ms;
} AtCommandRawReq_t;

/* Spy variables to capture outputs AND inject inputs */
extern char mock_last_at_command[256];
extern uint8_t mock_last_raw_data[256];
extern char mock_at_injected_line[128]; /* NEW: For injecting fake modem text lines! */

/* Mock IOCTL function */
static inline atCommandErrorCodes_t AtCommand_Ioctl(AT_IOCTL_COMMANDS_T cmd, void* param) {
    if (cmd == E_AT_IOCTL_SEND_CMD) {
        AtCommandReq_t* req = (AtCommandReq_t*)param;
        if (req && req->command) {
            strncpy(mock_last_at_command, req->command, 255);
            mock_last_at_command[255] = '\0';
        }
        
        if (req && req->expected_resp && req->resp_buffer) {
            strncpy(req->resp_buffer, req->expected_resp, req->resp_buffer_len - 1);
            req->resp_buffer[req->resp_buffer_len - 1] = '\0'; 
        }

        /* NEW: If the test provided a fake line, give it to the driver! */
        if (req && mock_at_injected_line[0] != '\0') {
            req->lines[0] = mock_at_injected_line;
            req->line_count = 1;
        } else if (req) {
            req->line_count = 0;
        }
    }
    else if (cmd == E_AT_IOCTL_SEND_RAW) {
        AtCommandRawReq_t* req = (AtCommandRawReq_t*)param;
        if (req && req->data) {
            if (req->length == 1 && req->data[0] == 0x1A) {
                return E_AT_ERR_NONE; 
            }
            memcpy(mock_last_raw_data, req->data, req->length);
            mock_last_raw_data[req->length] = '\0'; 
        }
    }
    return E_AT_ERR_NONE;
}

#endif /* MOCK_AT_COMMAND_H */