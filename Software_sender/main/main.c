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

#include <math.h>
#include "sound.h"

/**
 * @brief Function to send a packet using LoRa.
 *
 * This function sends a "Hello World" message using the LoRa module.
 * It is currently commented out and can be enabled if needed.
 */

//  void sendPacket()
//  {
// 	 const char *msg = "Hello World";
// 	 int send_len = strlen(msg);
 
// 	 lora_send_packet((uint8_t *)msg, send_len);
// 	 ESP_LOGI("STATE", "%d byte packet sent...", send_len);
//  }
 

/**
 * @brief Main function of the FreeRTOS application.
 *
 * This function implements a state machine to manage
 * the different operating modes of the LoRa module.
 */
void app_main(void)
{

  sound_init();
    
    // Allocation d'un buffer pour les échantillons audio
    int16_t audio_buffer[1024];

    while (1)
    {
        // Lecture des données audio
        int samples_read = sound_read(audio_buffer, 1024);
        if(samples_read <= 0) {
            printf("Erreur lors de la lecture des données audio\n");
            continue;
        }

        // Calcul du RMS (Root Mean Square)
        long sum = 0;
        for (int i = 0; i < samples_read; i++) {
            sum += audio_buffer[i] * audio_buffer[i];
        }
        float rms = sqrt((float)sum / samples_read);

        // Conversion en dB
        // La valeur de référence peut être calibrée en fonction de votre configuration.
        // Ici, on considère une référence arbitraire (par exemple 1.0).
        float db = 20 * log10(rms);

        printf("Niveau sonore: %.2f dB\n", db);

        // Délai pour limiter la fréquence d'affichage (ex. 500 ms)
        vTaskDelay(pdMS_TO_TICKS(500));
    }

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
			ESP_LOGE(pcTaskGetName(NULL), "Init mode");
			if (lora_init() == 0)
			{
				ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the LoRa module. Verify the connections and configuration.");
				state = ERROR;
			}
			lora_set_frequency(868e6);
			state = ACQUISITION;
			break;
		case ACQUISITION:
			ESP_LOGE(pcTaskGetName(NULL), "Acquisition mode");
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = TRANSMISSION;
			break;
		case TRANSMISSION:
			ESP_LOGE(pcTaskGetName(NULL), "Transmission mode");
			// sendPacket();
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = SLEEPMODE;
			break;
		case SLEEPMODE:
			ESP_LOGE(pcTaskGetName(NULL), "Sleep mode");
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = WAKEUP;
			break;
		case WAKEUP:
			ESP_LOGE(pcTaskGetName(NULL), "Wake up mode");
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = ACQUISITION;
			break;
		case ERROR:
			ESP_LOGE(pcTaskGetName(NULL), "Error mode");
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = INIT;
			break;
		}
	}
}