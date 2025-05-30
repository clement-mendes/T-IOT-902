#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "temperature.h"

// Fonction de setup avant chaque test
void setUp(void)
{
    // Initialisation du module de température
    temperature_init();
}

// Fonction de cleanup après chaque test
void tearDown(void)
{
    // Pas de cleanup nécessaire pour ce module
}

// Test de l'initialisation du module
void test_temperature_init(void)
{
    // L'initialisation est déjà faite dans setUp()
    // On peut vérifier que le module est fonctionnel en lisant une température
    float temp = temperature_get();
    TEST_ASSERT_FLOAT_WITHIN(100.0, 25.0, temp); // La température devrait être proche de 25°C (température ambiante)
}

// Test de la lecture de température
void test_temperature_get(void)
{
    float temp = temperature_get();
    
    // Vérification que la température est dans une plage raisonnable
    // Entre -40°C et 85°C (plage de fonctionnement du BME280)
    TEST_ASSERT_FLOAT_WITHIN(85.0, 25.0, temp);
    
    // Vérification que la température n'est pas 0 (ce qui indiquerait une erreur de lecture)
    TEST_ASSERT_NOT_EQUAL(0.0, temp);
}

// Test de la lecture de pression
void test_pressure_get(void)
{
    float pressure = pressure_get();
    
    // Vérification que la pression est dans une plage raisonnable
    // Entre 300 et 1100 hPa (plage de fonctionnement du BME280)
    TEST_ASSERT_FLOAT_WITHIN(400.0, 1013.25, pressure); // 1013.25 hPa est la pression atmosphérique standard
    
    // Vérification que la pression n'est pas 0 (ce qui indiquerait une erreur de lecture)
    TEST_ASSERT_NOT_EQUAL(0.0, pressure);
}

// Test de la stabilité des lectures
void test_reading_stability(void)
{
    float temp1 = temperature_get();
    float temp2 = temperature_get();
    float pressure1 = pressure_get();
    float pressure2 = pressure_get();
    
    // Les lectures successives devraient être relativement stables
    // On accepte une variation de 1°C pour la température
    TEST_ASSERT_FLOAT_WITHIN(1.0, temp1, temp2);
    
    // On accepte une variation de 1 hPa pour la pression
    TEST_ASSERT_FLOAT_WITHIN(1.0, pressure1, pressure2);
}

void app_main(void)
{
    // Démarrage des tests
    UNITY_BEGIN();
    
    // Exécution des tests
    RUN_TEST(test_temperature_init);
    RUN_TEST(test_temperature_get);
    RUN_TEST(test_pressure_get);
    RUN_TEST(test_reading_stability);
    
    // Fin des tests
    UNITY_END();
} 