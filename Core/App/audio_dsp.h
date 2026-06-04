#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include "main.h"

// Initialize DSP resources
void AudioDSP_Init(void);

// Process audio and produce log-mel spectrogram output
void Process_Audio_To_MelSpectrogram(int16_t* pcm_input, float* log_mel_output);

#endif /* AUDIO_DSP_H */
