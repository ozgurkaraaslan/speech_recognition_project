#include "cellular.h"
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

atCommandErrorCodes_t Cellular_InitDevice(void) {
    // Basic check and echo off
    if (SendCommand("AT", "OK", 1000) != E_AT_ERR_NONE) return E_AT_ERR_TIMEOUT;
    SendCommand("ATE0", "OK", 1000);
    SendCommand("AT+CMEE=2", "OK", 1000); // Enable verbose error reports
    return E_AT_ERR_NONE;
}

atCommandErrorCodes_t Cellular_SetupNetwork(void) {
    // Configure APN
    char apn_cmd[64];
    sprintf(apn_cmd, "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", CELL_APN);

    return SendCommand(apn_cmd, "OK", 2000);
}

atCommandErrorCodes_t Cellular_ConnectBroker(void) {
    char conn_cmd[128];
    // Open TCP connection to Broker
    sprintf(conn_cmd, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,0", CELL_BROKER_IP, CELL_BROKER_PORT);

    // EG25-G returns "+QIOPEN: 0,0" on successful connection
    return SendCommand(conn_cmd, "+QIOPEN: 0,0", 15000);
}
