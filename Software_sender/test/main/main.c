#include <stdio.h>
#include "unity.h"
#include "test_sound.h"
#include "test_temperature.h"

void app_main(void)
{
    printf("\n=== Démarrage des tests ===\n\n");
    
    // Tests du module son
    printf("\n--- Tests du module son ---\n");
    UNITY_BEGIN();
    RUN_TEST(test_sound_init);
    RUN_TEST(test_sound_measure_db);
    RUN_TEST(test_sound_get_average_db);
    UNITY_END();
    
    // Tests du module température
    printf("\n--- Tests du module température ---\n");
    UNITY_BEGIN();
    RUN_TEST(test_temperature_init);
    RUN_TEST(test_temperature_get);
    RUN_TEST(test_pressure_get);
    RUN_TEST(test_multiple_readings);
    UNITY_END();
    
    printf("\n=== Fin des tests ===\n");
} 