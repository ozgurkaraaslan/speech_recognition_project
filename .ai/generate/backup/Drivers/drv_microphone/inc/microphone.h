#ifndef MICROPHONE_H_
#define MICROPHONE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MICROPHONE_DRIVER_SW_VERSION	(1.0)


typedef enum {
    E_MIC_ERR_NONE = 0,
    E_MIC_ERR_HW_ERROR,
    E_MIC_ERR_WRONG_IOCTL_CMD,
    E_MIC_ERR_BUFFER_NULL,
    E_MIC_ERR_UNKNOWN
} micErrorCodes_t;


typedef enum {
    E_MIC_IOCTL_NONE = 0,
    E_MIC_IOCTL_GET_VERSION,
    E_MIC_IOCTL_PAUSE,
    E_MIC_IOCTL_RESUME
} MIC_IOCTL_COMMANDS_T;

// API Function Prototypes
micErrorCodes_t Microphone_Open(void* vpParam);
micErrorCodes_t Microphone_Ioctl(MIC_IOCTL_COMMANDS_T eCommand, void* vpParam);
micErrorCodes_t Microphone_Read(int16_t* pBuffer, uint16_t size);
micErrorCodes_t Microphone_Close(void* vpParam);

#ifdef __cplusplus
}
#endif

#endif /* MICROPHONE_H_ */
