#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "lora.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "test_lora.h"

static int tests_passed = 0;

void tearDown_lora(void)
{
    lora_close();
}

void test_lora_init(void)
{
    // Make sure the module is closed before starting
    lora_close();
    
    // Wait a bit to ensure the module is properly reset
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Try initialization
    int result = lora_init();
    
    // Check the result
    if (result != 1) {
        printf("LoRa initialization failed (code: %d)\n", result);
        printf("Check that:\n");
        printf("1. The LoRa module is properly connected\n");
        printf("2. The SPI pins are correctly configured\n");
        printf("3. The module power supply is stable\n");
    }
    
    TEST_ASSERT_EQUAL(1, result);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_lora_send(void)
{
    const char* test_message = "Test LoRa message";
    lora_send_packet((uint8_t*)test_message, strlen(test_message));
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_lora_receive(void)
{
    // Check if a packet was lost
    int lost = lora_packet_lost();
    TEST_ASSERT_GREATER_OR_EQUAL(0, lost);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_lora_config(void)
{
    // Test LoRa configuration
    lora_set_frequency(868000000);  // 868 MHz
    lora_set_spreading_factor(7);
    lora_set_bandwidth(125000);     // 125 kHz
    lora_set_coding_rate(5);        // 4/5
    lora_set_tx_power(14);          // dBm
    lora_enable_crc();              // Enable CRC
    
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
} 