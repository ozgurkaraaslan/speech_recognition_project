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
atCommandErrorCodes_t MQTT_OpenBroker(const char* host, uint16_t port);
atCommandErrorCodes_t MQTT_CloseBroker(void);
atCommandErrorCodes_t MQTT_ConnectClient(const char* client_id, const char* username, const char* password);
atCommandErrorCodes_t MQTT_Publish(const char* topic, const char* payload);

#ifdef __cplusplus
}
#endif

#endif /* INC_MQTT_H_ */
