#include "CppUTest/TestHarness.h"
#include <string.h>

extern "C" {
    #include "microphone.h"
    
    // We need to call the callback functions manually to simulate interrupts!
    void HAL_I2S_RxHalfCpltCallback(void *hi2s);
    void HAL_I2S_RxCpltCallback(void *hi2s);
}

/* Spy variables */
int mock_semaphore_count = 0;
I2S_HandleTypeDef hi2s2; /* Fake I2S Handle Instance */
float fir_coef[258] = {0}; /* Fake FIR coefficients to avoid linker error */

TEST_GROUP(Microphone_TestGroup)
{
    void setup() {
        mock_semaphore_count = 0;
        Microphone_Open(nullptr); /* Initialize the driver */
    }
    
    void teardown() {
        Microphone_Close(nullptr);
    }
};

/* Test: DMA Callback should trigger the Semaphore and process data */
TEST(Microphone_TestGroup, ReadShouldWaitAndProcessDMAInterrupt)
{
    int16_t out_buffer[16] = {0};
    micErrorCodes_t res;

    /* 1. Try to read BEFORE any interrupt happens. 
       Semaphore is 0, it should return Timeout/Unknown error! */
    res = Microphone_Read(out_buffer, 16);
    CHECK_EQUAL(E_MIC_ERR_UNKNOWN, res);

    /* 2. SIMULATE HARDWARE INTERRUPT (DMA Half Transfer Complete) */
    /* This will release the semaphore and set data_half_ready flag */
    HAL_I2S_RxHalfCpltCallback(&hi2s2);

    /* 3. Try to read AFTER the interrupt. 
       Semaphore is 1, so it should process data successfully! */
    res = Microphone_Read(out_buffer, 16);
    CHECK_EQUAL(E_MIC_ERR_NONE, res);

    /* 4. Verify if FIR Filter was actually called. 
       Our fake FIR filter writes 55.0f which becomes 55 in int16 */
    CHECK_EQUAL(55, out_buffer[0]);
}