#include "CppUTest/TestHarness.h"

extern "C" {
    #include "audio_dsp.h"
    #include <stdint.h>
}

// Mock call counter (reset before each test)
int mock_spectrogram_call_count = 0;

TEST_GROUP(AudioDSP_TestGroup)
{
    void setup() {
        AudioDSP_Init();
        mock_spectrogram_call_count = 0; // Reset before each test
    }
    void teardown() {}
};

// Verifies output buffer is filled as expected.
TEST(AudioDSP_TestGroup, ProcessAudioShouldFillTheOutputBuffer)
{
    int16_t dummy_pcm_input[16000] = {0};
    float dummy_log_mel_output[122 * 40] = {0};
    Process_Audio_To_MelSpectrogram(dummy_pcm_input, dummy_log_mel_output);
    DOUBLES_EQUAL(1.23f, dummy_log_mel_output[0], 0.01f);
    DOUBLES_EQUAL(1.23f, dummy_log_mel_output[4879], 0.01f);
}

// Verifies spectrogram generation produces exactly 122 columns.
TEST(AudioDSP_TestGroup, ProcessAudioShouldGenerateExactly122Columns)
{
    // Arrange
    int16_t dummy_pcm_input[16000] = {0};
    float dummy_log_mel_output[122 * 40] = {0};

    // Act
    Process_Audio_To_MelSpectrogram(dummy_pcm_input, dummy_log_mel_output);

    // Assert
    // The model expects a 122x40 input shape.
    // The spectrogram column function must be called exactly 122 times.
    CHECK_EQUAL(122, mock_spectrogram_call_count);
}