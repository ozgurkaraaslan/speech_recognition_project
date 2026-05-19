#ifndef MICROPHONE_HAL_H_
#define MICROPHONE_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"
#include "board.h"

// Get the I2S handle from main.c
extern I2S_HandleTypeDef hi2s2;

// Hardware macro definitions
#define MIC_HW_I2S      hi2s2

#ifdef __cplusplus
}
#endif

#endif /* MICROPHONE_HAL_H_ */
