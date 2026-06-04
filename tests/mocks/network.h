#ifndef MOCK_NETWORK_H
#define MOCK_NETWORK_H

#include <stdint.h>

// Network buffer sizes
#define STAI_NETWORK_IN_1_SIZE 4880
#define STAI_NETWORK_OUT_1_SIZE 3
#define STAI_NETWORK_ACTIVATIONS_SIZE_BYTES 1024
#define STAI_NETWORK_CONTEXT_SIZE 128

// AI Data Types
typedef int stai_return_code;
#define STAI_SUCCESS 0
#define STAI_MODE_SYNC 1

typedef void stai_network;
typedef void* stai_ptr;

// Mock control variables for testing
extern int mock_ai_run_call_count;
extern float* mock_ai_input_ptr;
extern float* mock_ai_output_ptr;

// Mock AI Functions
static inline stai_return_code stai_network_init(stai_network* net) { return STAI_SUCCESS; }
static inline stai_return_code stai_network_set_activations(stai_network* net, stai_ptr* acts, int num) { return STAI_SUCCESS; }

static inline stai_return_code stai_network_set_inputs(stai_network* net, stai_ptr* inputs, int num) {
    mock_ai_input_ptr = (float*)inputs[0];
    return STAI_SUCCESS;
}

static inline stai_return_code stai_network_set_outputs(stai_network* net, stai_ptr* outputs, int num) {
    mock_ai_output_ptr = (float*)outputs[0];
    return STAI_SUCCESS;
}

static inline stai_return_code stai_network_run(stai_network* net, int mode) {
    mock_ai_run_call_count++;
    
    // Set mock output to simulate "YES" classification
    if(mock_ai_output_ptr) {
        mock_ai_output_ptr[0] = 0.1f; // NOISE
        mock_ai_output_ptr[1] = 0.1f; // NO
        mock_ai_output_ptr[2] = 0.8f; // YES
    }
    return STAI_SUCCESS;
}

#endif /* MOCK_NETWORK_H */