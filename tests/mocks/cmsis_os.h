#ifndef MOCK_CMSIS_OS_H
#define MOCK_CMSIS_OS_H
#include <stdint.h>

/* Mock FreeRTOS definitions */
#define osWaitForever 0xFFFFFFFFU
#define osOK 0

/* Mock Mutex Handle type */
typedef void* osMutexId_t;

/* Mock functions: Do nothing, just return success */
static inline void osDelay(uint32_t ticks) {}
static inline int osMutexAcquire(osMutexId_t mutex, uint32_t timeout) { return osOK; }
static inline int osMutexRelease(osMutexId_t mutex) { return osOK; }

/* Mock Semaphore variables */
extern int mock_semaphore_count;

typedef void* osSemaphoreId_t;

static inline osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, void *attr) { 
    return (osSemaphoreId_t)1; /* Return a dummy handle */
}

/* If semaphore is available, take it. Otherwise return error (timeout) */
static inline int osSemaphoreAcquire(osSemaphoreId_t semaphore, uint32_t timeout) {
    if (mock_semaphore_count > 0) {
        mock_semaphore_count--;
        return osOK;
    }
    return -1; /* Timeout */
}

/* Releasing a semaphore increases the count */
static inline int osSemaphoreRelease(osSemaphoreId_t semaphore) {
    mock_semaphore_count++;
    return osOK;
}

#endif