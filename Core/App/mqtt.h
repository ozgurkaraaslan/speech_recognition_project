#ifndef INC_MQTT_H_
#define INC_MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "at_command.h"
#include <stdint.h>

/* --- Varsayılan Ayarlar --- */
#define MQTT_DEFAULT_PORT 1883

/* --- Export Edilen Fonksiyonlar --- */

/**
 * @brief MQTT Broker'a TCP seviyesinde bağlanır (Soket açar)
 * @param host Broker adresi (Örn: "broker.hivemq.com")
 * @param port Broker portu (Örn: 1883)
 * @return E_AT_ERR_NONE ise bağlantı başarılı
 */
atCommandErrorCodes_t MQTT_OpenBroker(const char* host, uint16_t port);

/**
 * @brief Açılan soket üzerinden MQTT Client girişi yapar
 * @param client_id Cihazın benzersiz kimliği (Örn: "AxisWater_01")
 * @param username Broker kullanıcı adı (Yoksa NULL girin)
 * @param password Broker şifresi (Yoksa NULL girin)
 * @return E_AT_ERR_NONE ise giriş başarılı
 */
atCommandErrorCodes_t MQTT_ConnectClient(const char* client_id, const char* username, const char* password);

/**
 * @brief Belirlenen Topic'e veri (payload) gönderir
 * @param topic Mesajın gönderileceği konu (Örn: "axis/water/status")
 * @param payload Gönderilecek asıl mesaj/veri
 * @return E_AT_ERR_NONE ise mesaj başarıyla Broker'a iletildi
 */
atCommandErrorCodes_t MQTT_Publish(const char* topic, const char* payload);

#ifdef __cplusplus
}
#endif

#endif /* INC_MQTT_H_ */
