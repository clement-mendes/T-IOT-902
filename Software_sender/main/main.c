/**
 * @file main.c
 * @brief Main program using LoRa with FreeRTOS.
 *
 * This program manages a state machine for LoRa communication,
 * including INIT, ACQUISITION, TRANSMISSION, SLEEPMODE, WAKEUP, and ERROR states.
 * It uses FreeRTOS for task management and delays.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lora.h"
#include "temperature.h"
#include <math.h>
#include "sound.h"

// Définition du tag pour les logs
static const char *TAG = "MAIN";

// Taille du buffer pour les données audio
#define SOUND_BUFFER_SIZE 1024

// Buffer pour stocker les données audio
int16_t sound_buffer[SOUND_BUFFER_SIZE];

/**
 * @brief Structure pour stocker les données des capteurs
 */
typedef struct {
    float temperature;
    float pressure;
    float sound_level_db;  // Modifié en float pour les dB
} sensor_data_t;

/**
 * @brief Fonction pour calculer le niveau sonore en décibels (dB) à partir des données brutes
 * 
 * @param buffer Tableau contenant les échantillons audio
 * @param length Nombre d'échantillons dans le tableau
 * @return float Niveau sonore en dB
 */
float calculate_sound_level_db(int16_t *buffer, size_t length) {
    // Calculer la valeur RMS (Root Mean Square) des échantillons
    long sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += (long)buffer[i] * buffer[i];
    }
    
    // Éviter la division par zéro
    if (length == 0) return 0;
    
    // Calculer la racine carrée de la moyenne des carrés
    float rms = sqrt((float)sum / length);
    
    // Valeur de référence pour I2S ESP32
    const float reference = 32767.0f;
    
    // Calcul des dB : 20 * log10(rms/reference)
    // Si rms est très petit, éviter le log de zéro
    if (rms < 1.0f) rms = 1.0f;
    
    float db = 20.0f * log10f(rms / reference);
    
    // Les valeurs seront négatives (en dessous de la référence)
    // On ajoute un offset pour avoir des valeurs plus faciles à interpréter
    db += 90.0f;
    
    return db;
}

/**
 * @brief Fonction pour envoyer les données des capteurs via LoRa
 * 
 * @param data Structure contenant les données des capteurs
 */
void send_sensor_data(sensor_data_t data) {
    char msg[128];
    // Formatage des données en chaîne de caractères avec le niveau sonore en dB
    snprintf(msg, sizeof(msg), "T:%.2f,P:%.2f,S:%.2f", 
             data.temperature, data.pressure, data.sound_level_db);
    
    int send_len = strlen(msg);
    lora_send_packet((uint8_t *)msg, send_len);
    ESP_LOGI(TAG, "%d byte packet sent: %s", send_len, msg);
}

/**
 * @brief Main function of the FreeRTOS application.
 *
 * This function implements a state machine to manage
 * the different operating modes of the LoRa module.
 */
void app_main(void)
{
    /**
     * @enum LoRaState
     * @brief Possible states of the LoRa system.
     */
    enum LoRaState
    {
        INIT,
        ACQUISITION,
        TRANSMISSION,
        SLEEPMODE,
        WAKEUP,
        ERROR
    };

    // Variable pour stocker les données des capteurs
    sensor_data_t sensor_data = {0};

    /**
     * @var state
     * @brief Current state of the LoRa system.
     */
    enum LoRaState state = INIT;
    while (1)
    {
        switch (state)
        {
        case INIT:
            ESP_LOGI(TAG, "Init mode");
            
            // Initialisation du module LoRa
            if (lora_init() == 0)
            {
                ESP_LOGE(TAG, "Does not recognize the LoRa module. Verify the connections and configuration.");
                state = ERROR;
                break;
            }
            lora_set_frequency(868e6);
            
            // Initialisation du module de température
            ESP_LOGI(TAG, "Initializing temperature module");
            temperature_init();
            
            // Initialisation du module son
            ESP_LOGI(TAG, "Initializing sound module");
            sound_init();
            
            state = ACQUISITION;
            break;
            
        case ACQUISITION:
            ESP_LOGI(TAG, "Acquisition mode");
            
            // Acquisition des données de température et pression
            sensor_data.temperature = temperature_get();
            sensor_data.pressure = pressure_get();
            ESP_LOGI(TAG, "Temperature: %.2f °C, Pressure: %.2f hPa", 
                     sensor_data.temperature, sensor_data.pressure);
            
            // Acquisition des données sonores et calcul en dB
            int samples_read = sound_read(sound_buffer, SOUND_BUFFER_SIZE);
            if (samples_read > 0) {
                sensor_data.sound_level_db = calculate_sound_level_db(sound_buffer, samples_read);
                ESP_LOGI(TAG, "Sound level: %.2f dB", sensor_data.sound_level_db);
            } else {
                ESP_LOGW(TAG, "Failed to read sound data, samples read: %d", samples_read);
            }
            
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = TRANSMISSION;
            break;
            
        case TRANSMISSION:
            ESP_LOGI(TAG, "Transmission mode");
            
            // Envoi des données des capteurs via LoRa
            send_sensor_data(sensor_data);
            
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = SLEEPMODE;
            break;
            
        case SLEEPMODE:
            ESP_LOGI(TAG, "Sleep mode");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = WAKEUP;
            break;
            
        case WAKEUP:
            ESP_LOGI(TAG, "Wake up mode");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = ACQUISITION;
            break;
            
        case ERROR:
            ESP_LOGE(TAG, "Error mode");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = INIT;
            break;
        }
    }
}