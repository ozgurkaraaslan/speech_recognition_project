#include "microphone.h"
#include "microphone_hal.h"
#include "pdm2pcm_glo.h"
#include <string.h>
#include "cmsis_os.h"

static uint8_t isMicOpen = 0;
static osSemaphoreId_t rx_semaphore = NULL;

// Voice Recognition Calculations:
// 1ms audio @ 16kHz = 16 PCM samples.
// Library requires 1024 bits (64 x 16-bit PDM) to produce 16 PCM samples.
// Total 128 used for Ping-Pong (Double Buffering) implementation.
#define PDM_BUFFER_SIZE 128
static uint16_t pdm_rx_buffer[PDM_BUFFER_SIZE];

// PDM2PCM Library Variables
static PDM_Filter_Handler_t PDM_FilterHandler[1];
static PDM_Filter_Config_t PDM_FilterConfig[1];

static volatile uint8_t data_half_ready = 0;
static volatile uint8_t data_full_ready = 0;

// Start the driver
micErrorCodes_t Microphone_Open(void* vpParam)
{
    if (isMicOpen) return E_MIC_ERR_NONE;

    memset(pdm_rx_buffer, 0, sizeof(pdm_rx_buffer));
    data_half_ready = 0;
    data_full_ready = 0;

    // PDM2PCM library initialization
    PDM_FilterHandler[0].bit_order = PDM_FILTER_BIT_ORDER_LSB;
    PDM_FilterHandler[0].endianness = PDM_FILTER_ENDIANNESS_LE;
    PDM_FilterHandler[0].high_pass_tap = 2122358088; // DC filter
    PDM_FilterHandler[0].out_ptr_channels = 1;       // Mono voice
    PDM_FilterHandler[0].in_ptr_channels = 1;        // Single PDM mic
    PDM_Filter_Init(&PDM_FilterHandler[0]);

    // PDM2PCM library configuration
    PDM_FilterConfig[0].output_samples_number = 16;  // 1 ms of audio at 16 kHz = 16 PCM samples
    PDM_FilterConfig[0].mic_gain = 12;               // Gain for better voice recognition performance
    PDM_FilterConfig[0].decimation_factor = PDM_FILTER_DEC_FACTOR_64; // 64 PDM bits to produce 16 PCM samples
    PDM_Filter_setConfig(&PDM_FilterHandler[0], &PDM_FilterConfig[0]);

    //  We create a binary semaphore that is empty (0) and can take a maximum value of 1
	if (rx_semaphore == NULL) {
		rx_semaphore = osSemaphoreNew(1, 0, NULL);
	}

    // Start I2S DMA reception for PDM data
    if (HAL_I2S_Receive_DMA(&MIC_HW_I2S, pdm_rx_buffer, PDM_BUFFER_SIZE) != HAL_OK) {
        return E_MIC_ERR_HW_ERROR;
    }

    isMicOpen = 1;
    return E_MIC_ERR_NONE;
}

// Ioctl commands
micErrorCodes_t Microphone_Ioctl(MIC_IOCTL_COMMANDS_T eCommand, void* vpParam)
{
    if (!isMicOpen) return E_MIC_ERR_HW_ERROR;

    switch (eCommand) {
        case E_MIC_IOCTL_PAUSE:
            HAL_I2S_DMAPause(&MIC_HW_I2S);
            break;
        case E_MIC_IOCTL_RESUME:
            HAL_I2S_DMAResume(&MIC_HW_I2S);
            break;
        case E_MIC_IOCTL_GET_VERSION:
            if (vpParam != NULL) { *((float*)vpParam) = MICROPHONE_DRIVER_SW_VERSION; }
            break;
        default:
            return E_MIC_ERR_WRONG_IOCTL_CMD;
    }
    return E_MIC_ERR_NONE;
}

micErrorCodes_t Microphone_Read(int16_t* pBuffer, uint16_t size)
{
    if (!isMicOpen) return E_MIC_ERR_HW_ERROR;
    if (pBuffer == NULL) return E_MIC_ERR_BUFFER_NULL;

    uint16_t samples_read = 0;

    // AI bizden 16000 (size) veri istiyor.
    // DMA her kesmede bize 16 veri (1ms) veriyor.
    // O halde bu döngü 1000 defa çalışıp 1 saniyelik sesi birleştirecek.
    while (samples_read < size)
    {
        // 1ms'lik yeni verinin gelmesini bekle (Timeout: 10ms)
        if (osSemaphoreAcquire(rx_semaphore, 10) == osOK)
        {
            uint8_t *pdm_ptr = NULL;

            if (data_half_ready) {
                pdm_ptr = (uint8_t*)&pdm_rx_buffer[0];
                data_half_ready = 0;
            }
            else if (data_full_ready) {
                pdm_ptr = (uint8_t*)&pdm_rx_buffer[PDM_BUFFER_SIZE / 2];
                data_full_ready = 0;
            }

            if (pdm_ptr != NULL) {
                // Filtrelenmiş 16 adet PCM verisini, buffer'da kaldığımız sıraya (offset) yaz
                PDM_Filter(pdm_ptr, (uint16_t*)&pBuffer[samples_read], &PDM_FilterHandler[0]);

                // Okunan örnek sayısını artır
                samples_read += 16;
            }
        }
        else
        {
            // Eğer 10ms boyunca DMA'dan hiç ses gelmezse (Mikrofon takıldıysa) hata dön
            return E_MIC_ERR_UNKNOWN;
        }
    }

    return E_MIC_ERR_NONE; // Tam 16000 veri dolduğunda başarıyla çık
}

// Close the driver and stop the I2S DMA
micErrorCodes_t Microphone_Close(void* vpParam)
{
    HAL_I2S_DMAStop(&MIC_HW_I2S);
    isMicOpen = 0;
    return E_MIC_ERR_NONE;
}

// DMA Interrupts
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == MIC_HW_I2S.Instance) {
        data_half_ready = 1;
        osSemaphoreRelease(rx_semaphore);
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == MIC_HW_I2S.Instance) {
        data_full_ready = 1;
        osSemaphoreRelease(rx_semaphore);
    }
}
