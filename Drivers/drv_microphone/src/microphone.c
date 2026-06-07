#include "microphone.h"
#include "cmsis_os.h"
#include "arm_math.h"
#include "FIR_Filter_Coef.h" // FIR filter coefficients
#include <string.h>

extern I2S_HandleTypeDef hi2s2; // I2S hardware handle from HAL

static uint8_t isMicOpen = 0;
static osSemaphoreId_t rx_semaphore = NULL;

// --- FIR filter and buffer configuration ---
#define FIR_TAPS                     258U
#define FIR_DECIMATION_FACTOR        64U
#define PDM_SAMPLE_BITS              16U

#define PDM_WORDS_PER_MS             64   // 1 ms audio = 64 16-bit PDM words
#define PDM_BUFFER_DOUBLE_SIZE       128  // Double buffer size for ping-pong
#define PCM_SAMPLES_PER_MS           16   // Number of PCM samples produced per ms
#define FLOAT32_PDM_BUFFER_SIZE      1024 // 64 * 16 bit
#define FIR_STATE_BUFFER_SIZE        (FLOAT32_PDM_BUFFER_SIZE + FIR_TAPS - 1)

// --- Memory regions ---
static uint16_t pdm_rx_buffer[PDM_BUFFER_DOUBLE_SIZE];
static float32_t pdm_float_buffer[FLOAT32_PDM_BUFFER_SIZE];
static float32_t pcm_float_buffer[PCM_SAMPLES_PER_MS];
static float32_t fir_state[FIR_STATE_BUFFER_SIZE];

static arm_fir_decimate_instance_f32 fir_decimate_inst;

static volatile uint8_t data_half_ready = 0;
static volatile uint8_t data_full_ready = 0;

// --- Helper functions ---
static void convert_to_pdm_float32(uint16_t* src, float32_t* dst, uint8_t buffer_index) {
    uint32_t bitIndex = 0;
    uint16_t modifiedCurrentWord = 0;
    memset(dst, 0, sizeof(float32_t) * FLOAT32_PDM_BUFFER_SIZE);

    for (bitIndex = 0; bitIndex < FLOAT32_PDM_BUFFER_SIZE; bitIndex++) {
        if (bitIndex % 16 == 0) {
            modifiedCurrentWord = src[(bitIndex / 16) + (buffer_index * PDM_WORDS_PER_MS)];
        }
        dst[bitIndex] = (modifiedCurrentWord & 0x8000) ? 32767.0f : -32768.0f;
        modifiedCurrentWord <<= 1;
    }
}

static void convert_pcm_f32_to_int16(float32_t* src, int16_t* dst, size_t len) {
    for (size_t i = 0; i < len; i++) {
        float32_t sample = src[i];
        if (sample > 32767.0f) dst[i] = 32767;
        else if (sample < -32768.0f) dst[i] = -32768;
        else dst[i] = (int16_t)sample;
    }
}

// --- Driver API ---
micErrorCodes_t Microphone_Open(void* vpParam)
{
    if (isMicOpen) return E_MIC_ERR_NONE;

    data_half_ready = 0;
    data_full_ready = 0;

    if (rx_semaphore == NULL) {
        rx_semaphore = osSemaphoreNew(1, 0, NULL);
    }

    // Initialize CMSIS-DSP FIR decimator
    if (arm_fir_decimate_init_f32(&fir_decimate_inst, FIR_TAPS, FIR_DECIMATION_FACTOR, fir_coef, fir_state, FLOAT32_PDM_BUFFER_SIZE) != ARM_MATH_SUCCESS) {
        return E_MIC_ERR_HW_ERROR;
    }

    // Start I2S DMA receive
    if (HAL_I2S_Receive_DMA(&hi2s2, pdm_rx_buffer, PDM_BUFFER_DOUBLE_SIZE) != HAL_OK) {
        return E_MIC_ERR_HW_ERROR;
    }

    isMicOpen = 1;
    return E_MIC_ERR_NONE;
}

micErrorCodes_t Microphone_Read(int16_t* pBuffer, uint16_t size)
{
    if (!isMicOpen) return E_MIC_ERR_HW_ERROR;
    if (pBuffer == NULL) return E_MIC_ERR_BUFFER_NULL;

    uint16_t samples_read = 0;
    int16_t pcm_int16_temp[PCM_SAMPLES_PER_MS];

    while (samples_read < size)
    {
        // Wait for new 1ms data via semaphore
        if (osSemaphoreAcquire(rx_semaphore, 10) == osOK)
        {
            uint8_t buffer_index = 0xFF; // Invalid index

            // Determine which half-buffer filled
            if (data_half_ready) {
                buffer_index = 0;
                data_half_ready = 0;
            }
            else if (data_full_ready) {
                buffer_index = 1;
                data_full_ready = 0;
            }

            if (buffer_index != 0xFF) {
                // Step 1: Convert PDM bits to float samples (+32767/-32768)
                convert_to_pdm_float32(pdm_rx_buffer, pdm_float_buffer, buffer_index);

                // Step 2: FIR decimation (float -> float)
                arm_fir_decimate_f32(&fir_decimate_inst, pdm_float_buffer, pcm_float_buffer, FLOAT32_PDM_BUFFER_SIZE);

                // Step 3: Convert float to int16
                convert_pcm_f32_to_int16(pcm_float_buffer, pcm_int16_temp, PCM_SAMPLES_PER_MS);

                // Step 4: Copy results into user's main buffer
                memcpy(&pBuffer[samples_read], pcm_int16_temp, PCM_SAMPLES_PER_MS * sizeof(int16_t));

                samples_read += PCM_SAMPLES_PER_MS;
            }
        }
        else {
            return E_MIC_ERR_UNKNOWN;
        }
    }
    return E_MIC_ERR_NONE;
}

micErrorCodes_t Microphone_Close(void* vpParam)
{
    HAL_I2S_DMAStop(&hi2s2);
    isMicOpen = 0;
    return E_MIC_ERR_NONE;
}

micErrorCodes_t Microphone_Ioctl(MIC_IOCTL_COMMANDS_T eCommand, void* vpParam)
{
    if (!isMicOpen) return E_MIC_ERR_HW_ERROR;
    // IOCTL operations (pause/resume, etc. may be added)
    return E_MIC_ERR_NONE;
}

// --- DMA Interrupt Functions ---
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == hi2s2.Instance) {
        data_half_ready = 1;
        osSemaphoreRelease(rx_semaphore);
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == hi2s2.Instance) {
        data_full_ready = 1;
        osSemaphoreRelease(rx_semaphore);
    }
}
