#include "audio_dsp.h"
#include "feature_extraction.h"

// File-local (static) DSP memory and configuration
static SpectrogramTypeDef        spectrogram_conf;
static MelFilterTypeDef          mel_filter_conf;
static MelSpectrogramTypeDef     mel_spectrogram_conf;
static LogMelSpectrogramTypeDef  log_mel_conf;

static arm_rfft_fast_instance_f32 rfft_fast_inst;
static float32_t scratch_fft[512];
static float32_t window_buffer[512];

static uint32_t  mel_start_idx[40];
static uint32_t  mel_stop_idx[40];
static float32_t mel_coeffs[600];

void AudioDSP_Init(void)
{
    Window_Init(window_buffer, 512, WINDOW_HANN);

    spectrogram_conf.pRfft    = &rfft_fast_inst;
    spectrogram_conf.Type     = SPECTRUM_TYPE_POWER;
    spectrogram_conf.pWindow  = window_buffer;
    spectrogram_conf.SampRate = 16000;
    spectrogram_conf.FrameLen = 512;
    spectrogram_conf.FFTLen   = 512;
    spectrogram_conf.pScratch = scratch_fft;

    arm_rfft_fast_init_f32(&rfft_fast_inst, spectrogram_conf.FFTLen);

    mel_filter_conf.pStartIndices      = mel_start_idx;
    mel_filter_conf.pStopIndices       = mel_stop_idx;
    mel_filter_conf.pCoefficients      = mel_coeffs;
    mel_filter_conf.NumMels            = 40;
    mel_filter_conf.FFTLen             = 512;
    mel_filter_conf.SampRate           = 16000;
    mel_filter_conf.FMin               = 300.0f;
    mel_filter_conf.FMax               = 8000.0f;
    mel_filter_conf.Formula            = MEL_SLANEY;
    mel_filter_conf.Normalize          = 1;
    mel_filter_conf.Mel2F              = 1;

    MelFilterbank_Init(&mel_filter_conf);

    mel_spectrogram_conf.SpectrogramConf = &spectrogram_conf;
    mel_spectrogram_conf.MelFilter       = &mel_filter_conf;

    log_mel_conf.MelSpectrogramConf = &mel_spectrogram_conf;
    log_mel_conf.LogFormula         = LOGMELSPECTROGRAM_SCALE_DB;
    log_mel_conf.Ref                = 1.0f;
    log_mel_conf.TopdB              = 80.0f;
}

void Process_Audio_To_MelSpectrogram(int16_t* pcm_input, float* log_mel_output)
{
    float32_t float_frame[512];
    float32_t mel_col[40];

    int hop_length = 128;
    int frame_length = 512;
    int total_columns = ((16000 - frame_length) / hop_length) + 1;

    for (int col = 0; col < total_columns; col++)
    {
        buf_to_float_normed(&pcm_input[col * hop_length], float_frame, frame_length);
        LogMelSpectrogramColumn(&log_mel_conf, float_frame, mel_col);

        for (int m = 0; m < 40; m++) {
            log_mel_output[(col * 40) + m] = mel_col[m];
        }
    }
}
