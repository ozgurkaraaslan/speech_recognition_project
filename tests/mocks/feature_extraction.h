#ifndef MOCK_FEATURE_EXTRACTION_H
#define MOCK_FEATURE_EXTRACTION_H

#include "arm_math.h"

// Constant definitions (matching ST library)
#define WINDOW_HANN 1
#define SPECTRUM_TYPE_POWER 1
#define MEL_SLANEY 1
#define LOGMELSPECTROGRAM_SCALE_DB 1

// Declare external counter used by tests
extern int mock_spectrogram_call_count;

// Structures
typedef struct {
    void* pRfft;
    int Type;
    float32_t* pWindow;
    int SampRate;
    int FrameLen;
    int FFTLen;
    float32_t* pScratch;
} SpectrogramTypeDef;

typedef struct {
    uint32_t* pStartIndices;
    uint32_t* pStopIndices;
    float32_t* pCoefficients;
    int NumMels;
    int FFTLen;
    int SampRate;
    float FMin;
    float FMax;
    int Formula;
    int Normalize;
    int Mel2F;
} MelFilterTypeDef;

typedef struct {
    SpectrogramTypeDef* SpectrogramConf;
    MelFilterTypeDef* MelFilter;
} MelSpectrogramTypeDef;

typedef struct {
    MelSpectrogramTypeDef* MelSpectrogramConf;
    int LogFormula;
    float Ref;
    float TopdB;
} LogMelSpectrogramTypeDef;

// Mock functions

// Initialization mocks: no-op for tests
static inline void Window_Init(float32_t *pWindow, uint16_t window_length, uint32_t window_type) {}
static inline void MelFilterbank_Init(MelFilterTypeDef *MelFilterStruct) {}

// Mock normalization: set all output samples to 0
static inline void buf_to_float_normed(int16_t *pInSignal, float32_t *pOutSignal, uint16_t len) {
    for(int i=0; i<len; i++) {
        pOutSignal[i] = 0.0f;
    }
}

static inline void LogMelSpectrogramColumn(LogMelSpectrogramTypeDef *LogMelStruct, float32_t *pInSignal, float32_t *pOutCol) {
    // Increment call counter for test verification
    mock_spectrogram_call_count++;

    // Fill output column with constant mock values
    for (int i = 0; i < 40; i++) {
        pOutCol[i] = 1.23f;
    }
}

#endif /* MOCK_FEATURE_EXTRACTION_H */