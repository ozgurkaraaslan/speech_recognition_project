#include "mqtt.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

// Reference mutex created in main.c
extern osMutexId_t at_driver_mutexHandle;

// Internal buffer for MQTT middleware
static char mqtt_resp_buffer[256];

// 1. Open broker connection (open TCP socket)
atCommandErrorCodes_t MQTT_OpenBroker(const char* host, uint16_t port)
{
    AtCommandReq_t req;
    char cmd_buf[128];

    // AT+QMTOPEN=0,"broker.hivemq.com",1883
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTOPEN=0,\"%s\",%d", host, port);

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    // Successful connection indicated by URC: "+QMTOPEN: 0,0"
    req.expected_resp = "+QMTOPEN: 0,0";
    req.timeout_ms = 15000; // Allow extra time for DNS resolution and TCP
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    // Acquire mutex to protect UART transactions
    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK) {
        res = AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req);
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}

atCommandErrorCodes_t MQTT_CloseBroker(void)
{
    AtCommandReq_t req;
    char cmd_buf[32];

    // Close TCP socket (0: client id)
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTCLOSE=0");

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    // Close URC may return ERROR if already closed; tolerate failure (cleanup purpose)
    req.expected_resp = "OK";
    req.timeout_ms = 5000;
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK) {
        // Optionally attempt to disconnect client first
        AtCommandReq_t disc_req;
        memset(&disc_req, 0, sizeof(AtCommandReq_t));
        disc_req.command = "AT+QMTDISC=0";
        disc_req.expected_resp = "OK";
        disc_req.timeout_ms = 3000;
        disc_req.resp_buffer = mqtt_resp_buffer;
        disc_req.resp_buffer_len = sizeof(mqtt_resp_buffer);
        AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &disc_req); // Ignore result; best-effort disconnect

        osDelay(500); // Allow modem to recover

        // Sonra TCP soketini zorla kapat
        res = AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req);
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}

// 2. Client connect (MQTT CONNECT)
atCommandErrorCodes_t MQTT_ConnectClient(const char* client_id, const char* username, const char* password)
{
    AtCommandReq_t req;
    char cmd_buf[256];

    // Build command depending on username/password presence
    if (username == NULL || password == NULL) {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTCONN=0,\"%s\"", client_id);
    } else {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", client_id, username, password);
    }

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    // Success URC: +QMTCONN: 0,0,0 (client id, success, accepted)
    req.expected_resp = "+QMTCONN: 0,0,0";
    req.timeout_ms = 10000;
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK) {
        res = AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req);
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}

// 3. Publish message (MQTT PUBLISH)
atCommandErrorCodes_t MQTT_Publish(const char* topic, const char* payload)
{
    AtCommandReq_t req;
    char cmd_buf[128];

    // STEP 1: Send publish command (QOS=1, Retain=0, MsgID=1)
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTPUB=0,1,1,0,\"%s\"", topic);

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    req.expected_resp = ">"; // Modem responds with '>' when ready to receive payload
    req.timeout_ms = 5000;
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK)
    {
        // Send command and wait for '>' prompt
        if (AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req) == E_AT_ERR_NONE)
        {
            AtCommandRawReq_t raw_req;

            // STEP 2: Send payload bytes to modem
            raw_req.data = (uint8_t*)payload;
            raw_req.length = (uint16_t)strlen(payload);
            raw_req.timeout_ms = 2000;
            AtCommand_Ioctl(E_AT_IOCTL_SEND_RAW, &raw_req);

            // STEP 3: Terminate transmission with CTRL+Z (0x1A)
            uint8_t ctrl_z = 0x1A;
            raw_req.data = &ctrl_z;
            raw_req.length = 1;
            raw_req.timeout_ms = 1000;
            res = AtCommand_Ioctl(E_AT_IOCTL_SEND_RAW, &raw_req);

            // Note: In ideal flow a publish URC (+QMTPUB: 0,1,0) may follow.
            // To avoid async delays we consider the operation successful here.
        }
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}
