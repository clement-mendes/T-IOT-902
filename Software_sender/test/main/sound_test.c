#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "sound.h"

// Fonction de setup avant chaque test
void setUp(void)
{
    // Initialisation du module de son
    sound_init();
}

// Fonction de cleanup après chaque test
void tearDown(void)
{
    // Pas de cleanup nécessaire pour ce module
}

// Test de l'initialisation du module
void test_sound_init(void)
{
    // Vérification des connexions du capteur
    bool connections_ok = sound_check_connections();
    TEST_ASSERT_TRUE(connections_ok);
}

// Test de la mesure de niveau sonore
void test_sound_measure_db(void)
{
    float db = sound_measure_db();
    
    // Vérification que le niveau sonore est dans une plage raisonnable
    // Entre 30 et 90 dB (plage de fonctionnement du capteur)
    TEST_ASSERT_FLOAT_WITHIN(30.0, 60.0, db); // 60 dB est un niveau sonore moyen
    
    // Vérification que la valeur n'est pas négative
    TEST_ASSERT_GREATER_THAN_OR_EQUAL(30.0, db);
    TEST_ASSERT_LESS_THAN_OR_EQUAL(90.0, db);
}

// Test de la lecture brute
void test_sound_read_raw(void)
{
    int16_t buffer[64];
    int samples_read = sound_read_raw(buffer, 64);
    
    // Vérification que le nombre d'échantillons lus est correct
    TEST_ASSERT_EQUAL(64, samples_read);
    
    // Vérification que les valeurs sont dans la plage valide
    for (int i = 0; i < samples_read; i++) {
        TEST_ASSERT_GREATER_THAN_OR_EQUAL(0, buffer[i]);
        TEST_ASSERT_LESS_THAN_OR_EQUAL(4095, buffer[i]); // ADC 12-bit
    }
}

// Test de la moyenne des mesures
void test_sound_get_average_db(void)
{
    float avg_db = sound_get_average_db();
    
    // Vérification que la moyenne est dans une plage raisonnable
    TEST_ASSERT_FLOAT_WITHIN(30.0, 60.0, avg_db);
    
    // Vérification que la valeur n'est pas négative
    TEST_ASSERT_GREATER_THAN_OR_EQUAL(30.0, avg_db);
    TEST_ASSERT_LESS_THAN_OR_EQUAL(90.0, avg_db);
}

// Test de la stabilité des mesures
void test_sound_measurement_stability(void)
{
    float db1 = sound_measure_db();
    float db2 = sound_measure_db();
    
    // Les mesures successives devraient être relativement stables
    // On accepte une variation de 10 dB entre deux mesures
    TEST_ASSERT_FLOAT_WITHIN(10.0, db1, db2);
}

void app_main(void)
{
    // Démarrage des tests
    UNITY_BEGIN();
    
    // Exécution des tests
    RUN_TEST(test_sound_init);
    RUN_TEST(test_sound_measure_db);
    RUN_TEST(test_sound_read_raw);
    RUN_TEST(test_sound_get_average_db);
    RUN_TEST(test_sound_measurement_stability);
    
    // Fin des tests
    UNITY_END();
} 