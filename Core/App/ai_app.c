#include "ai_app.h"
#include "cmsis_os.h"

// Public AI buffers
__attribute__((aligned(4))) float stai_input_buffer[STAI_NETWORK_IN_1_SIZE];
__attribute__((aligned(4))) float stai_output_buffer[STAI_NETWORK_OUT_1_SIZE];

// Private working buffers for the AI engine
static __attribute__((aligned(4))) uint8_t activations[STAI_NETWORK_ACTIVATIONS_SIZE_BYTES];
static __attribute__((aligned(8))) uint8_t stai_context_buffer[STAI_NETWORK_CONTEXT_SIZE];
static stai_network* my_network;

void My_AI_Init(void)
{
    stai_return_code ret;
    my_network = (stai_network*)stai_context_buffer;
    ret = stai_network_init(my_network);

    if (ret != STAI_SUCCESS) {
        while(1) { osDelay(10); } // Halt on initialization error
    }

    stai_ptr act_ptr[1];
    act_ptr[0] = (stai_ptr)activations;
    stai_network_set_activations(my_network, act_ptr, 1);
}

void My_AI_Run(float *in_data, float *out_data)
{
    stai_ptr input_ptrs[1];
    stai_ptr output_ptrs[1];

    input_ptrs[0] = (stai_ptr)in_data;
    output_ptrs[0] = (stai_ptr)out_data;

    stai_network_set_inputs(my_network, input_ptrs, 1);
    stai_network_set_outputs(my_network, output_ptrs, 1);

    stai_network_run(my_network, STAI_MODE_SYNC);
}
