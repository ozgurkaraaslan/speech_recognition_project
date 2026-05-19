#ifndef AT_COMMAND_HAL_H_
#define AT_COMMAND_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"
#include "board.h"

// Get the UART handle from main.c
extern UART_HandleTypeDef huart3;

// Hardware macro definitions
#define AT_HW_USART         huart3

#ifdef __cplusplus
}
#endif

#endif /* AT_COMMAND_HAL_H_ */
