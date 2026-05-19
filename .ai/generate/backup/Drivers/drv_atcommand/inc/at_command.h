#ifndef AT_COMMAND_H_
#define AT_COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define AT_COMMAND_DRIVER_SW_VERSION	(1.0)

typedef enum {
    E_AT_ERR_NONE = 0,
    E_AT_ERR_TIMEOUT,
    E_AT_ERR_UNEXPECTED_RESPONSE,
    E_AT_ERR_HW_ERROR,
    E_AT_ERR_WRONG_IOCTL_CMD,
    E_AT_ERR_UNKNOWN
} atCommandErrorCodes_t;

typedef enum {
    E_AT_IOCTL_NONE = 0,
    E_AT_IOCTL_SEND_CMD
} AT_COMMAND_IOCTL_COMMANDS_T;

#define AT_MAX_LINES    8

typedef struct {
    // INPUTS
    const char* command;         // AT Command string (e.g., "AT+CSQ?")
    const char* expected_resp;   // Expected response substring to look for (e.g., "OK")
    uint32_t    timeout_ms;      // Timeout for waiting for the response in milliseconds

    // OUTPUTS
    char* resp_buffer;           // Location to store the raw response from the modem
    uint16_t resp_buffer_len;    // Size of the response buffer provided by the user

    char* lines[AT_MAX_LINES];   // Parsed lines from the response (e.g., if response is "OK\r\n+CSQ: 15,99\r\n", lines[0] = "+CSQ: 15,99")
    uint8_t line_count;          // Number of valid lines parsed into the 'lines' array
} AtCommandReq_t;

// API Function Prototypes
atCommandErrorCodes_t AtCommand_Open(void* vpParam);

atCommandErrorCodes_t AtCommand_Ioctl(AT_COMMAND_IOCTL_COMMANDS_T eCommand, void* vpParam);

atCommandErrorCodes_t AtCommand_Close(void* vpParam);

#ifdef __cplusplus
}
#endif

#endif /* AT_COMMAND_H_ */
