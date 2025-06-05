#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "lora.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "test_lora.h"

static int tests_passed = 0;

void setUp_lora(void)
{
    // Ne pas initialiser ici, on le fait dans le test
}

void tearDown_lora(void)
{
    lora_close();
}

void test_lora_init(void)
{
    // S'assurer que le module est fermé avant de commencer
    lora_close();
    
    // Attendre un peu pour s'assurer que le module est bien réinitialisé
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Tenter l'initialisation
    int result = lora_init();
    
    // Vérifier le résultat
    if (result != 0) {
        printf("Échec de l'initialisation LoRa (code: %d)\n", result);
        printf("Vérifiez que :\n");
        printf("1. Le module LoRa est correctement connecté\n");
        printf("2. Les broches SPI sont correctement configurées\n");
        printf("3. L'alimentation du module est stable\n");
    }
    
    TEST_ASSERT_EQUAL(0, result);
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
    // Vérifier si un paquet a été perdu
    int lost = lora_packet_lost();
    TEST_ASSERT_GREATER_OR_EQUAL(0, lost);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_lora_config(void)
{
    // Test de la configuration LoRa
    lora_set_frequency(868000000);  // 868 MHz
    lora_set_spreading_factor(7);
    lora_set_bandwidth(125000);     // 125 kHz
    lora_set_coding_rate(5);        // 4/5
    lora_set_tx_power(14);          // dBm
    lora_enable_crc();              // Activer CRC
    
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
} 