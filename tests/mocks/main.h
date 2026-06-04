#ifndef MOCK_MAIN_H
#define MOCK_MAIN_H

// Provide embedded C types (e.g., int16_t, uint32_t) for host builds.
#include <stdint.h>
#include <stddef.h>

// Dummy HAL status definitions used by audio_dsp in tests.
typedef enum {
    HAL_OK = 0U,
    HAL_ERROR = 1U,
    HAL_BUSY = 2U,
    HAL_TIMEOUT = 3U
} HAL_StatusTypeDef;

/* Mock I2S Hardware Type */
typedef struct {
    void* Instance;
} I2S_HandleTypeDef;

/* Fake external hardware handle */
extern I2S_HandleTypeDef hi2s2;

/* Mock HAL Functions */
static inline int HAL_I2S_Receive_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size) {
    return HAL_OK;
}

static inline int HAL_I2S_DMAStop(I2S_HandleTypeDef *hi2s) {
    return HAL_OK;
}

#endif /* MOCK_MAIN_H */