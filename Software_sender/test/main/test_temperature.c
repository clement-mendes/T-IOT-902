#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "temperature.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TEMPERATURE_TEST";

void setUp(void)
{
    // Initialisation avant chaque test
    temperature_init();
}

void tearDown(void)
{
    // Nettoyage après chaque test
    // Ajoutez ici le code de nettoyage si nécessaire
}

void test_temperature_init(void)
{
    // Test de l'initialisation
    temperature_init();
    // Vérifier que l'initialisation ne cause pas d'erreur
    // Note: Nous ne pouvons pas vérifier directement ESP_OK car la fonction est void
    TEST_PASS();
}

void test_temperature_get(void)
{
    // Test de la lecture de température
    float temp = temperature_get();
    
    // Vérifier que la température est dans une plage raisonnable
    // Plage typique pour un capteur BME280: -40°C à +85°C
    TEST_ASSERT_GREATER_OR_EQUAL(-40.0f, temp);
    TEST_ASSERT_LESS_OR_EQUAL(85.0f, temp);
    
    // Vérifier que la température n'est pas 0 (ce qui indiquerait une erreur de lecture)
    TEST_ASSERT_NOT_EQUAL(0.0f, temp);
}

void test_pressure_get(void)
{
    // Test de la lecture de pression
    float pressure = pressure_get();
    
    // Vérifier que la pression est dans une plage raisonnable
    // Plage typique pour un capteur BME280: 300hPa à 1100hPa
    TEST_ASSERT_GREATER_OR_EQUAL(300.0f, pressure);
    TEST_ASSERT_LESS_OR_EQUAL(1100.0f, pressure);
    
    // Vérifier que la pression n'est pas 0 (ce qui indiquerait une erreur de lecture)
    TEST_ASSERT_NOT_EQUAL(0.0f, pressure);
}

void test_multiple_readings(void)
{
    // Test de lectures multiples pour vérifier la stabilité
    float temp1 = temperature_get();
    float temp2 = temperature_get();
    float temp3 = temperature_get();
    
    // Vérifier que les lectures sont cohérentes entre elles
    // La différence entre deux lectures consécutives ne devrait pas être trop grande
    TEST_ASSERT_FLOAT_WITHIN(5.0f, temp1, temp2);  // Tolérance de 5°C
    TEST_ASSERT_FLOAT_WITHIN(5.0f, temp2, temp3);
    
    float press1 = pressure_get();
    float press2 = pressure_get();
    float press3 = pressure_get();
    
    // Vérifier que les lectures de pression sont cohérentes
    TEST_ASSERT_FLOAT_WITHIN(10.0f, press1, press2);  // Tolérance de 10hPa
    TEST_ASSERT_FLOAT_WITHIN(10.0f, press2, press3);
}

void app_main(void)
{
    // Configuration des tests
    UNITY_BEGIN();
    
    // Exécution des tests
    RUN_TEST(test_temperature_init);
    RUN_TEST(test_temperature_get);
    RUN_TEST(test_pressure_get);
    RUN_TEST(test_multiple_readings);
    
    // Finalisation des tests
    UNITY_END();
} 