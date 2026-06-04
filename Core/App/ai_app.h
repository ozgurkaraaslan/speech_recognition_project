#ifndef AI_APP_H
#define AI_APP_H

#include "network.h"

// main.c'nin sonuçları okuyabilmesi veya ses verebilmesi için bu dizileri dışa açıyoruz
extern __attribute__((aligned(4))) float stai_input_buffer[STAI_NETWORK_IN_1_SIZE];
extern __attribute__((aligned(4))) float stai_output_buffer[STAI_NETWORK_OUT_1_SIZE];

void My_AI_Init(void);
void My_AI_Run(float *in_data, float *out_data);

#endif /* AI_APP_H */
