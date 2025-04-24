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
 * @brief Function to initialize the sound system.
 *
 * This function initializes the I2S interface for audio input.
 */
// void sound_init()

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

	/**
	 * @var state
	 * @brief Current state of the LoRa system.
	 */
	enum LoRaState state = INIT;

	float avg_temperature = 0.0;
	float avg_pressure = 0.0;
	float avg_air_quality = 0.0; // Placeholder for air quality
	float avg_sound_level = 0.0; // Placeholder for sound level

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
			temperature_init();
			state = ACQUISITION;
			break;
		case ACQUISITION:
			ESP_LOGE(pcTaskGetName(NULL), "Acquisition mode");
			{
				float temp_sum = 0.0;
				float pressure_sum = 0.0;
				float air_quality_sum = 0.0;
				float sound_level_sum = 0.0;

				for (int i = 0; i < 10; i++)
				{
					temp_sum += temperature_get();
					pressure_sum += pressure_get();
					// air_quality_sum += air_quality_get(); 
					// sound_level_sum += sound_level_get(); 
					vTaskDelay(pdMS_TO_TICKS(1000)); // Attendre 1 seconde
				}

				avg_temperature = temp_sum / 10.0;
				avg_pressure = pressure_sum / 10.0;
				// avg_air_quality = air_quality_sum / 10.0;
				// avg_sound_level = sound_level_sum / 10.0;

				printf("Moyenne température: %.2f °C\n", avg_temperature);
				printf("Moyenne pression: %.2f hPa\n", avg_pressure);
				// printf("Moyenne qualité de l'air: %.2f\n", avg_air_quality);
				// printf("Moyenne niveau sonore: %.2f dB\n", avg_sound_level);
			}
			state = TRANSMISSION;
			break;
		case TRANSMISSION:
			ESP_LOGE(pcTaskGetName(NULL), "Transmission mode");
			{
				char message[128];
				snprintf(message, sizeof(message), 
					"{\"temp\":%.2f,\"press\":%.2f}", 
					avg_temperature, avg_pressure);
				lora_send_packet((uint8_t *)message, strlen(message));
				ESP_LOGI("TRANSMISSION", "Données envoyées: %s", message);
			}
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