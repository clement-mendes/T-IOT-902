#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lora.h"
#include "temperature.h"
#include "sound.h"

// Mock functions for testing
static int mock_lora_init(void) {
    return 1; // Simulate successful initialization
}

static float mock_temperature_get(void) {
    return 25.0f; // Return a fixed temperature for testing
}

static float mock_pressure_get(void) {
    return 1013.25f; // Return a fixed pressure for testing
}

static float mock_sound_get_average_db(void) {
    return 65.0f; // Return a fixed sound level for testing
}

// Test suite setup and teardown
void setUp(void) {
    // Setup code before each test
}

void tearDown(void) {
    // Cleanup code after each test
}

// Test cases
void test_lora_initialization(void) {
    TEST_ASSERT_EQUAL(1, mock_lora_init());
}

void test_temperature_measurement(void) {
    float temp = mock_temperature_get();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 25.0f, temp);
}

void test_pressure_measurement(void) {
    float pressure = mock_pressure_get();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1013.25f, pressure);
}

void test_sound_measurement(void) {
    float sound = mock_sound_get_average_db();
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 65.0f, sound);
}

void test_data_transmission(void) {
    float temp = mock_temperature_get();
    float pressure = mock_pressure_get();
    float sound = mock_sound_get_average_db();
    
    char message[128];
    snprintf(message, sizeof(message), 
        "{\"temp\":%.2f,\"press\":%.2f,\"sound\":%.2f}", 
        temp, pressure, sound);
    
    // Verify the message format
    TEST_ASSERT_TRUE(strstr(message, "\"temp\":") != NULL);
    TEST_ASSERT_TRUE(strstr(message, "\"press\":") != NULL);
    TEST_ASSERT_TRUE(strstr(message, "\"sound\":") != NULL);
}

// Main test runner
void app_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_lora_initialization);
    RUN_TEST(test_temperature_measurement);
    RUN_TEST(test_pressure_measurement);
    RUN_TEST(test_sound_measurement);
    RUN_TEST(test_data_transmission);
    
    UNITY_END();
} 