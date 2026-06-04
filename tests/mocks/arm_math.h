#ifndef MOCK_ARM_MATH_H
#define MOCK_ARM_MATH_H

#define ARM_MATH_SUCCESS 0

#include <stdint.h>

// Map the STM32 float type to the host float type.
typedef float float32_t;

// Minimal FFT instance mock used by tests.
typedef struct {
    uint16_t fftLenRFFT;
} arm_rfft_fast_instance_f32;

typedef struct {
    uint16_t numTaps;
} arm_fir_decimate_instance_f32;

// Mock FFT init function for host-side tests.
static inline void arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32 *S, uint16_t fftLen) {
    S->fftLenRFFT = fftLen;
}

static inline int arm_fir_decimate_init_f32(arm_fir_decimate_instance_f32 *S, uint16_t numTaps, uint8_t M, const float32_t *pCoeffs, float32_t *pState, uint32_t blockSize) {
    return ARM_MATH_SUCCESS;
}

static inline void arm_fir_decimate_f32(arm_fir_decimate_instance_f32 *S, float32_t *pSrc, float32_t *pDst, uint32_t blockSize) {
    /* FAKE FIR FILTER: Just put a dummy value (e.g. 55.0) into destination */
    for(int i=0; i<16; i++) {
        pDst[i] = 55.0f;
    }
}

#endif /* MOCK_ARM_MATH_H */