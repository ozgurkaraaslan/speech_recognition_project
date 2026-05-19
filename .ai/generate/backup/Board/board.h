#ifndef BOARD_H_
#define BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f4xx_hal.h"

// Definitions for the modem's UART interface
#define AT_MODEM_USART_INSTANCE    USART3
#define AT_MODEM_BAUDRATE          115200

// Microphone I2S interface definitions
#define MIC_I2S_INSTANCE    SPI2

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
