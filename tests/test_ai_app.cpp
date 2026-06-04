#include "CppUTest/TestHarness.h"

extern "C" {
    #include "ai_app.h"
}

// Mock state variables
int mock_ai_run_call_count = 0;
float* mock_ai_input_ptr = nullptr;
float* mock_ai_output_ptr = nullptr;

TEST_GROUP(AI_App_TestGroup)
{
    void setup() {
        // Reset mock state and initialize the AI app before each test
        mock_ai_run_call_count = 0;
        mock_ai_input_ptr = nullptr;
        mock_ai_output_ptr = nullptr;
        My_AI_Init();
    }
    void teardown() {}
};

// Verify AI run uses correct input/output buffers
TEST(AI_App_TestGroup, AIRunShouldExecuteAndSetCorrectPointers)
{
    // Arrange
    float dummy_in[STAI_NETWORK_IN_1_SIZE] = {0};
    float dummy_out[STAI_NETWORK_OUT_1_SIZE] = {0};

    // Act
    My_AI_Run(dummy_in, dummy_out);

    // Assert
    // Network_Run should be called once
    CHECK_EQUAL(1, mock_ai_run_call_count);
    
    // Provided buffers should be passed correctly to the ST library
    CHECK_TRUE(mock_ai_input_ptr == dummy_in);
    CHECK_TRUE(mock_ai_output_ptr == dummy_out);
    
    // Output should contain mocked "YES" score (0.8)
    DOUBLES_EQUAL(0.8f, dummy_out[2], 0.01f);
}