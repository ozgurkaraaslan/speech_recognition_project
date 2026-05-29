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

// Yardımcı Fonksiyon: Şebeke Kaydını Bekle
static atCommandErrorCodes_t Cellular_WaitForRegistration(void)
{
    uint8_t retry_count = 0;
    const uint8_t max_retries = 30; // 30 deneme * 2 saniye = 60 sn bekleme

    AtCommandReq_t req;

    while (retry_count < max_retries)
    {
        // 1. Struct'ı güvenli şekilde sıfırla
        memset(&req, 0, sizeof(AtCommandReq_t));
        req.command = "AT+CGREG?";
        req.expected_resp = "OK";
        req.timeout_ms = 2000;
        req.resp_buffer = internal_resp;
        req.resp_buffer_len = sizeof(internal_resp);

        // 2. Komutu gönder (Mutex driver'ın içinde olduğu için güvendeyiz)
        if (AtCommand_Ioctl(E_AT_IOCTL_SEND_CMD, &req) == E_AT_ERR_NONE)
        {
            // 3. Sürücünün bizim için ayırdığı satırları (lines) tek tek dön!
            for(int i = 0; i < req.line_count; i++)
            {
                if (req.lines[i] != NULL)
                {
                    // Artık \0 sorunu yok, çünkü her satırı ayrı ayrı inceliyoruz
                    if (strstr(req.lines[i], ",1") != NULL || strstr(req.lines[i], ",5") != NULL)
                    {
                        return E_AT_ERR_NONE; // Başarılı!
                    }
                }
            }
        }

        // Eğer kayıt olmadıysa uyu ve tekrar dene
        osDelay(2000);
        retry_count++;
    }

    return E_AT_ERR_TIMEOUT;
}

// Yardımcı Fonksiyon: PDP Context Aktif mi? (Örn: context_id = 1)
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
                    return E_AT_ERR_NONE; // Başarılı!
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

// ANA FONKSİYON: Tüm Ağ Kurulumunu ve Kontrollerini Yapar
atCommandErrorCodes_t Cellular_SetupNetwork(void)
{
    atCommandErrorCodes_t status;
    char cmd_buf[64];

    // --- 1. ADIM: ŞEBEKE KAYDINI BEKLE ---
    status = Cellular_WaitForRegistration();
    if (status != E_AT_ERR_NONE) {
        return status;
    }

    // --- 2. ADIM: APN KONFİGÜRASYONU ---
    sprintf(cmd_buf, "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", CELL_APN);
    status = SendCommand(cmd_buf, "OK", 2000);
    if (status != E_AT_ERR_NONE) {
        return status;
    }

    // --- 3. ADIM: PDP CONTEXT KONTROLÜ VE AKTİVASYONU ---
    // Eğer halihazırda aktif değilse aktifleştirmeyi dene
    if (Cellular_CheckPDPContext(1) != E_AT_ERR_NONE)
    {
        // Quectel'in TCP/IP motorunu kullanacağımız için context'i aktifleştiriyoruz
        status = SendCommand("AT+QIACT=1", "OK", 10000);

        if (status != E_AT_ERR_NONE) {
            // Eğer ERROR dönerse, her ihtimale karşı gerçekten aktif olup olmadığını son kez sor
            if (Cellular_CheckPDPContext(1) != E_AT_ERR_NONE) {
                return E_AT_ERR_HW_ERROR; // Maalesef IP alamadık
            }
        }
    }

    return E_AT_ERR_NONE; // Tüm kontrollerden geçti, internete çıkmaya hazır!
}

atCommandErrorCodes_t Cellular_ConnectBroker(void) {
    char conn_cmd[128];
    // Open TCP connection to Broker
    sprintf(conn_cmd, "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,0", CELL_BROKER_IP, CELL_BROKER_PORT);

    // EG25-G returns "+QIOPEN: 0,0" on successful connection
    return SendCommand(conn_cmd, "+QIOPEN: 0,0", 15000);
}

// Ham Veri Gönderme (MQTT Paketleri için)
atCommandErrorCodes_t Cellular_TransmitRaw(uint8_t* data, uint16_t len) {
	/*
    char cmd[32];

    // 0 numaralı TCP soketinden 'len' kadar veri göndereceğimizi bildir
    sprintf(cmd, "AT+QISEND=0,%d", len);

    // EG25-G komutu alınca veri beklediğini belirtmek için ">" işareti döner.
    if (SendCommand(cmd, ">", 2000) == E_AT_ERR_NONE)
    {
        // ">" işaretini gördük! Şimdi HAM BAYTLARI UART'a yollayabiliriz.
        HAL_UART_Transmit(&AT_HW_USART, data, len, 5000);

        // Veriyi yolladıktan sonra modemin "SEND OK" demesini bekle
        return SendCommand("AT", "SEND OK", 5000);
    }*/
    return E_AT_ERR_UNKNOWN;
}
