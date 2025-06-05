#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "sound.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SOUND_TEST";

void setUp(void)
{
    // Initialisation avant chaque test
    sound_init();
}

void tearDown(void)
{
    // Nettoyage après chaque test
    // Ajoutez ici le code de nettoyage si nécessaire
}

void test_sound_init(void)
{
    // Test de l'initialisation
    TEST_ASSERT_EQUAL(ESP_OK, sound_init());
}

void test_sound_check_connections(void)
{
    // Test de la vérification des connexions
    bool result = sound_check_connections();
    TEST_ASSERT_TRUE(result);
}

void test_sound_measure_db(void)
{
    // Test de la mesure du niveau sonore
    float db = sound_measure_db();
    TEST_ASSERT_GREATER_OR_EQUAL(30.0f, db);  // Minimum 30 dB
    TEST_ASSERT_LESS_OR_EQUAL(90.0f, db);     // Maximum 90 dB
}

void test_sound_read_raw(void)
{
    // Test de la lecture des données brutes
    int16_t buffer[64];
    int result = sound_read_raw(buffer, 64);
    TEST_ASSERT_EQUAL(64, result);
    
    // Vérifier que les valeurs sont dans une plage raisonnable
    for (int i = 0; i < 64; i++) {
        TEST_ASSERT_GREATER_OR_EQUAL(0, buffer[i]);
        TEST_ASSERT_LESS_OR_EQUAL(4095, buffer[i]);
    }
}

void test_sound_get_average_db(void)
{
    // Test de la moyenne des mesures
    float avg_db = sound_get_average_db();
    TEST_ASSERT_GREATER_OR_EQUAL(30.0f, avg_db);  // Minimum 30 dB
    TEST_ASSERT_LESS_OR_EQUAL(90.0f, avg_db);     // Maximum 90 dB
}

void app_main(void)
{
    // Configuration des tests
    UNITY_BEGIN();
    
    // Exécution des tests
    RUN_TEST(test_sound_init);
    RUN_TEST(test_sound_check_connections);
    RUN_TEST(test_sound_measure_db);
    RUN_TEST(test_sound_read_raw);
    RUN_TEST(test_sound_get_average_db);
    
    // Finalisation des tests
    UNITY_END();
} 