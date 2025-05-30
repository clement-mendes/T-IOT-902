#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "lora.h"

// Variables globales pour les tests
static uint8_t test_buffer[256];
static int test_buffer_size = 0;

// Fonction de setup avant chaque test
void setUp(void)
{
    // Initialisation du module LoRa
    TEST_ASSERT_EQUAL(1, lora_init());
}

// Fonction de cleanup après chaque test
void tearDown(void)
{
    // Fermeture propre du module LoRa
    lora_close();
}

// Test de l'initialisation du module
void test_lora_init(void)
{
    TEST_ASSERT_EQUAL(1, lora_init());
}

// Test de la configuration de la fréquence
void test_lora_set_frequency(void)
{
    long test_freq = 915000000; // 915 MHz
    lora_set_frequency(test_freq);
    // Note: Pas de getter direct pour la fréquence, donc on ne peut pas vérifier directement
}

// Test de la configuration du spreading factor
void test_lora_set_spreading_factor(void)
{
    int test_sf = 7;
    lora_set_spreading_factor(test_sf);
    TEST_ASSERT_EQUAL(test_sf, lora_get_spreading_factor());
}

// Test de la configuration de la bande passante
void test_lora_set_bandwidth(void)
{
    int test_bw = 7;
    lora_set_bandwidth(test_bw);
    TEST_ASSERT_EQUAL(test_bw, lora_get_bandwidth());
}

// Test de la configuration du coding rate
void test_lora_set_coding_rate(void)
{
    int test_cr = 4;
    lora_set_coding_rate(test_cr);
    TEST_ASSERT_EQUAL(test_cr, lora_get_coding_rate());
}

// Test de l'envoi d'un paquet
void test_lora_send_packet(void)
{
    const char* test_message = "Hello LoRa!";
    int message_len = strlen(test_message);
    
    // Copie du message dans le buffer de test
    memcpy(test_buffer, test_message, message_len);
    test_buffer_size = message_len;
    
    // Envoi du paquet
    lora_send_packet(test_buffer, test_buffer_size);
    
    // Vérification du nombre de paquets perdus
    TEST_ASSERT_EQUAL(0, lora_packet_lost());
}

void app_main(void)
{
    // Démarrage des tests
    UNITY_BEGIN();
    
    // Exécution des tests
    RUN_TEST(test_lora_init);
    RUN_TEST(test_lora_set_frequency);
    RUN_TEST(test_lora_set_spreading_factor);
    RUN_TEST(test_lora_set_bandwidth);
    RUN_TEST(test_lora_set_coding_rate);
    RUN_TEST(test_lora_send_packet);
    
    // Fin des tests
    UNITY_END();
} 