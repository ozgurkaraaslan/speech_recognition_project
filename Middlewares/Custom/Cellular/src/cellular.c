#include "cellular.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

static char internal_resp[256];

/* Helper to simplify IOCTL calls */
static atCommandErrorCodes_t SendCommand(const char* cmd, const char* expect, uint32_t timeout) {
    AtCommandReq_t req = {
        .command = (char*)cmd,
        .expected_resp = (char*)expect,
        .timeout_ms = timeout,
        .resp_buffer = internal_resp,
        .resp_buffer_len = sizeof(internal_resp)
    };
    return AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req);
}

// Helper function: wait for network registration
static atCommandErrorCodes_t Cellular_WaitForRegistration(void)
{
    uint8_t retry_count = 0;
    const uint8_t max_retries = 30; // 30 retries * 2s = 60s total wait

    AtCommandReq_t req;

    while (retry_count < max_retries)
    {
        // 1. Zero the request struct
        memset(&req, 0, sizeof(AtCommandReq_t));
        req.command = "AT+CGREG?";
        req.expected_resp = "OK";
        req.timeout_ms = 2000;
        req.resp_buffer = internal_resp;
        req.resp_buffer_len = sizeof(internal_resp);

        // 2. Send command (safe within driver mutex)
        if (AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req) == E_AT_ERR_NONE)
        {
            // 3. Iterate parsed lines returned by the driver
            for(int i = 0; i < req.line_count; i++)
            {
                if (req.lines[i] != NULL)
                {
                    // No NUL issues since lines are checked individually
                    if (strstr(req.lines[i], ",1") != NULL || strstr(req.lines[i], ",5") != NULL)
                    {
                        return E_AT_ERR_NONE; // Success
                    }
                }
            }
        }

        // Not registered yet: wait and retry
        osDelay(2000);
        retry_count++;
    }

    return E_AT_ERR_TIMEOUT;
}

// Helper function: is PDP context active? (e.g., context_id = 1)
static atCommandErrorCodes_t Cellular_CheckPDPContext(uint8_t context_id)
{
    AtCommandReq_t req;
    char search_str[32];

    snprintf(search_str, sizeof(search_str), "+CGACT: %d,1", context_id);

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = "AT+CGACT?";
    req.expected_resp = "OK";
    req.timeout_ms = 2000;
    req.resp_buffer = internal_resp;
    req.resp_buffer_len = sizeof(internal_resp);

    if (AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req) == E_AT_ERR_NONE)
    {
        for(int i = 0; i < req.line_count; i++)
        {
            if (req.lines[i] != NULL)
            {
                if (strstr(req.lines[i], search_str) != NULL)
                {
                    return E_AT_ERR_NONE; // Success
                }
            }
        }
    }
    return E_AT_ERR_UNEXPECTED_RESPONSE;
}

atCommandErrorCodes_t Cellular_CheckDevice(void) {
    // Basic check
    if (SendCommand("AT", "OK", 1000) != E_AT_ERR_NONE) return E_AT_ERR_TIMEOUT;
    return E_AT_ERR_NONE;
}

// MAIN FUNCTION: performs full network setup and checks
atCommandErrorCodes_t Cellular_SetupNetwork(void)
{
    atCommandErrorCodes_t status;
    char cmd_buf[64];

    // --- STEP 1: Wait for network registration ---
    status = Cellular_WaitForRegistration();
    if (status != E_AT_ERR_NONE) {
        return status;
    }

    // --- STEP 2: Configure APN ---
    sprintf(cmd_buf, "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", CELL_APN);
    status = SendCommand(cmd_buf, "OK", 2000);
    if (status != E_AT_ERR_NONE) {
        return status;
    }

    // --- STEP 3: PDP context check and activation ---
    // If not active, attempt activation
    if (Cellular_CheckPDPContext(1) != E_AT_ERR_NONE)
    {
        // Activate PDP context for Quectel TCP/IP engine
        status = SendCommand("AT+QIACT=1", "OK", 10000);

        if (status != E_AT_ERR_NONE) {
            // If ERROR returned, double-check activation before failing
            if (Cellular_CheckPDPContext(1) != E_AT_ERR_NONE) {
                return E_AT_ERR_HW_ERROR; // Failed to obtain IP
            }
        }
    }

    return E_AT_ERR_NONE; // All checks passed, ready to access network
}

atCommandErrorCodes_t Cellular_ConnectBroker(void) {
    char conn_cmd[128];
    // Open TCP connection to Broker
    sprintf(conn_cmd, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,0", CELL_BROKER_IP, CELL_BROKER_PORT);

    // EG25-G returns "+QIOPEN: 0,0" on successful connection
    return SendCommand(conn_cmd, "+QIOPEN: 0,0", 15000);
}

// Raw data transmission (for MQTT packets)
atCommandErrorCodes_t Cellular_TransmitRaw(uint8_t* data, uint16_t len) {
    /*
    char cmd[32];

    // Notify modem to send 'len' bytes on TCP socket 0
    sprintf(cmd, "AT+QISEND=0,%d", len);

    // EG25-G responds with ">" when ready to receive bytes
    if (SendCommand(cmd, ">", 2000) == E_AT_ERR_NONE)
    {
        // Transmit raw bytes over UART
        HAL_UART_Transmit(&AT_HW_USART, data, len, 5000);

        // Wait for modem to report "SEND OK"
        return SendCommand("AT", "SEND OK", 5000);
    }*/
    return E_AT_ERR_UNKNOWN;
}
