#include "CppUTest/TestHarness.h"
#include <string.h>

extern "C" {
    #include "cellular.h"
}

/* Connect to our spy variables */
extern char mock_last_at_command[256];
char mock_at_injected_line[128] = {0};

TEST_GROUP(Cellular_TestGroup)
{
    void setup() {
        memset(mock_last_at_command, 0, sizeof(mock_last_at_command));
        memset(mock_at_injected_line, 0, sizeof(mock_at_injected_line));
    }
    void teardown() {}
};

/* TEST 1: Basic device check */
TEST(Cellular_TestGroup, CheckDeviceSendsBasicAT)
{
    atCommandErrorCodes_t res = Cellular_CheckDevice();
    CHECK_EQUAL(E_AT_ERR_NONE, res);
    STRCMP_EQUAL("AT", mock_last_at_command);
}

/* TEST 2: Full Network Setup Flow */
TEST(Cellular_TestGroup, SetupNetworkConfiguresAPNSuccessfully)
{
    /* 1. ARRANGE
       We use a brilliant TDD trick here! 
       Cellular_SetupNetwork needs ",1" for CGREG and "+CGACT: 1,1" for CGACT.
       We inject a single combined string that satisfies BOTH checks using strstr! */
    strcpy(mock_at_injected_line, "+CGREG: 0,1 +CGACT: 1,1");

    /* 2. ACT
       Start the whole network setup sequence */
    atCommandErrorCodes_t res = Cellular_SetupNetwork();

    /* 3. ASSERT
       Did it pass all steps (Registration -> APN -> Context) without errors? */
    CHECK_EQUAL(E_AT_ERR_NONE, res);
    
    /* Since Context check is the very last step in Cellular_SetupNetwork, 
       the last AT command sent must be "AT+CGACT?" */
    STRCMP_EQUAL("AT+CGACT?", mock_last_at_command);
}