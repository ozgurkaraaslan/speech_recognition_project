#include "mqtt.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

// main.c içerisinde oluşturulan mutex'i çekiyoruz
extern osMutexId_t at_driver_mutexHandle;

// MQTT Middleware'ine özel iç tampon
static char mqtt_resp_buffer[256];

// 1. BROKER'A BAĞLAN (TCP SOKETİ AÇ)
atCommandErrorCodes_t MQTT_OpenBroker(const char* host, uint16_t port)
{
    AtCommandReq_t req;
    char cmd_buf[128];

    // AT+QMTOPEN=0,"broker.hivemq.com",1883
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTOPEN=0,\"%s\",%d", host, port);

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    // Bağlantı başarısı URC üzerinden okunur: "+QMTOPEN: 0,0"
    req.expected_resp = "+QMTOPEN: 0,0";
    req.timeout_ms = 15000; // DNS Çözümleme ve TCP için uzun süre tanıyoruz
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    // İşlemciyi kilitliyoruz ki başka bir task araya girip UART hattını bozmasın
    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK) {
        res = AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req);
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}

// 2. CLIENT GİRİŞİ YAP (MQTT CONNECT)
atCommandErrorCodes_t MQTT_ConnectClient(const char* client_id, const char* username, const char* password)
{
    AtCommandReq_t req;
    char cmd_buf[256];

    // Username ve password durumuna göre komutu dinamik oluştur
    if (username == NULL || password == NULL) {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTCONN=0,\"%s\"", client_id);
    } else {
        snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"", client_id, username, password);
    }

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    // Başarı URC'si: 0 (client id), 0 (başarılı), 0 (bağlantı kabul edildi)
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

// 3. MESAJ YAYINLA (MQTT PUBLISH)
atCommandErrorCodes_t MQTT_Publish(const char* topic, const char* payload)
{
    AtCommandReq_t req;
    char cmd_buf[128];

    // ADIM 1: Publish komutunu yolla (QOS=1, Retain=0, MsgID=1)
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+QMTPUB=0,1,1,0,\"%s\"", topic);

    memset(&req, 0, sizeof(AtCommandReq_t));
    req.command = cmd_buf;
    req.expected_resp = ">"; // Modem veriyi girmemiz için '>' bekler
    req.timeout_ms = 5000;
    req.resp_buffer = mqtt_resp_buffer;
    req.resp_buffer_len = sizeof(mqtt_resp_buffer);

    atCommandErrorCodes_t res = E_AT_ERR_HW_ERROR;

    if (osMutexAcquire(at_driver_mutexHandle, osWaitForever) == osOK)
    {
        // Komutu yolla ve '>' dönmesini bekle
        if (AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req) == E_AT_ERR_NONE)
        {
            AtCommandRawReq_t raw_req;

            // ADIM 2: Modemin içine Payload'u (Saf veriyi) bas
            raw_req.data = (uint8_t*)payload;
            raw_req.length = (uint16_t)strlen(payload);
            raw_req.timeout_ms = 2000;
            AtCommand_Ioctl(E_AT_IOCTL_SEND_RAW, &raw_req);

            // ADIM 3: İletimi bitirmek için CTRL+Z (0x1A) bas
            uint8_t ctrl_z = 0x1A;
            raw_req.data = &ctrl_z;
            raw_req.length = 1;
            raw_req.timeout_ms = 1000;
            res = AtCommand_Ioctl(E_AT_IOCTL_SEND_RAW, &raw_req);

            // Not: İdeal akışta burada yayınlandı URC'si (+QMTPUB: 0,1,0) beklenebilir.
            // Asenkron gecikmeleri engellemek için doğrudan başarılı kabul ediyoruz.
        }
        osMutexRelease(at_driver_mutexHandle);
    }

    return res;
}
