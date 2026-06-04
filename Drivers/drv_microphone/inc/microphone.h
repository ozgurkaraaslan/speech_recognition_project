#ifndef MICROPHONE_H_
#define MICROPHONE_H_

#include "main.h"

// Hata kodları ve IOCTL komutları (Senin orijinal yapın)
typedef enum {
    E_MIC_ERR_NONE = 0,
    E_MIC_ERR_HW_ERROR,
    E_MIC_ERR_BUFFER_NULL,
    E_MIC_ERR_WRONG_IOCTL_CMD,
    E_MIC_ERR_UNKNOWN
} micErrorCodes_t;

typedef enum {
    E_MIC_IOCTL_PAUSE,
    E_MIC_IOCTL_RESUME,
    E_MIC_IOCTL_GET_VERSION
} MIC_IOCTL_COMMANDS_T;

#define MICROPHONE_DRIVER_SW_VERSION 2.0f // FIR Filtreli Yeni Versiyon

/* Public Functions */
micErrorCodes_t Microphone_Open(void* vpParam);
micErrorCodes_t Microphone_Read(int16_t* pBuffer, uint16_t size);
micErrorCodes_t Microphone_Ioctl(MIC_IOCTL_COMMANDS_T eCommand, void* vpParam);
micErrorCodes_t Microphone_Close(void* vpParam);

#endif /* MICROPHONE_H_ */
