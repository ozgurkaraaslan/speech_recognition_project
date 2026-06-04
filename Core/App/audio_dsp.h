#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include "main.h"

// Başlatma fonksiyonu
void AudioDSP_Init(void);

// Sesi işleyip spektrograma çeviren ana fonksiyon
void Process_Audio_To_MelSpectrogram(int16_t* pcm_input, float* log_mel_output);

#endif /* AUDIO_DSP_H */
