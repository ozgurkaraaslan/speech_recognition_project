#include "CppUTest/TestHarness.h"
#include <string.h>

extern "C" {
    #include "mqtt.h"
}

/* Provide the extern mutex that mqtt.c expects */
void* at_driver_mutexHandle = nullptr;

/* Define spy variables here */
char mock_last_at_command[256] = {0};
uint8_t mock_last_raw_data[256] = {0};

TEST_GROUP(MQTT_TestGroup)
{
    void setup() {
        /* Clear spy variables before each test */
        memset(mock_last_at_command, 0, sizeof(mock_last_at_command));
        memset(mock_last_raw_data, 0, sizeof(mock_last_raw_data));
    }
    void teardown() {}
};

/* Test 1: Check AT command format for broker connection */
TEST(MQTT_TestGroup, OpenBrokerShouldFormatCommandCorrectly)
{
    /* Act */
    MQTT_OpenBroker("broker.hivemq.com", 1883);

    /* Assert: Did mqtt.c create the exact string expected by the BG96 modem? */
    STRCMP_EQUAL("AT+QMTOPEN=0,\"broker.hivemq.com\",1883", mock_last_at_command);
}

/* Test 2: Check AT command format for MQTT publish */
TEST(MQTT_TestGroup, PublishShouldFormatTopicAndPayloadCorrectly)
{
    /* Act */
    MQTT_Publish("my_sensor/temp", "24.5");

    /* Assert: Check the AT command for topic setup */
    STRCMP_EQUAL("AT+QMTPUB=0,1,1,0,\"my_sensor/temp\"", mock_last_at_command);
    
    /* Assert: Check the raw data injection for payload */
    STRCMP_EQUAL("24.5", (char*)mock_last_raw_data);
}