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
#include "esp_sleep.h"
#include "esp_system.h"

/**
 * @brief Function to send a packet using LoRa.
 *
 * This function sends a "Hello World" message using the LoRa module.
 * It is currently commented out and can be enabled if needed.
 */

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

	// Define a persistent variable in RTC memory
	static RTC_DATA_ATTR enum LoRaState state = INIT;

	float avg_temperature = 0.0;
	float avg_pressure = 0.0;
	// float avg_air_quality = 0.0;
	float avg_sound_level = 0.0; 

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
			sound_init();  // Initialize sound module
			state = ACQUISITION;
			break;
		case ACQUISITION:
			ESP_LOGE(pcTaskGetName(NULL), "Acquisition mode, waiting for data...");
			{
				float temp_sum = 0.0;
				float pressure_sum = 0.0;
				float sound_level_sum = 0.0;

				for (int i = 0; i < 10; i++)
				{
					temp_sum += temperature_get();
					pressure_sum += pressure_get();
					// air_quality_sum += air_quality_get(); 
					
					// Read sound data and calculate level
					int16_t sound_buffer[128];
					int samples_read = sound_read(sound_buffer, sizeof(sound_buffer)/sizeof(sound_buffer[0]));
					if (samples_read > 0) {
						// Debug: print first few samples
						ESP_LOGI("SOUND", "Samples read: %d", samples_read);
						ESP_LOGI("SOUND", "First 5 samples: %d, %d, %d, %d, %d", 
							sound_buffer[0], sound_buffer[1], sound_buffer[2], 
							sound_buffer[3], sound_buffer[4]);

						// Calculate RMS (Root Mean Square)
						float sum = 0;
						for (int j = 0; j < samples_read; j++) {
							sum += (float)sound_buffer[j] * sound_buffer[j];
						}
						float rms = sqrt(sum / samples_read);
						ESP_LOGI("SOUND", "RMS value: %.2f", rms);

						// Convert to decibels with adjusted reference and scaling
						float reference = 32768.0; // Maximum value for 16-bit audio
						float db = 20 * log10f(rms / reference);
						// Add 90 dB offset to match typical sound level ranges
						db = db + 90;
						if (db < 0) db = 0;
						if (db > 120) db = 120; // Cap at 120 dB for safety
						
						ESP_LOGI("SOUND", "Calculated dB: %.2f", db);
						sound_level_sum += db;
					} else {
						ESP_LOGE("SOUND", "Failed to read sound data: %d", samples_read);
					}
					
					vTaskDelay(pdMS_TO_TICKS(1000)); // Attendre 1 seconde
				}

				avg_temperature = temp_sum / 10.0;
				avg_pressure = pressure_sum / 10.0;
				// avg_air_quality = air_quality_sum / 10.0;
				avg_sound_level = sound_level_sum / 10.0;

				printf("Moyenne température: %.2f °C\n", avg_temperature);
				printf("Moyenne pression: %.2f hPa\n", avg_pressure);
				// printf("Moyenne qualité de l'air: %.2f\n", avg_air_quality);
				printf("Moyenne niveau sonore: %.2f dB\n", avg_sound_level);
			}
			state = TRANSMISSION;
			break;
		case TRANSMISSION:
			ESP_LOGE(pcTaskGetName(NULL), "Transmission mode");
			{
				char message[128];
				snprintf(message, sizeof(message), 
					"{\"temp\":%.2f,\"press\":%.2f,\"sound\":%.2f}", 
					avg_temperature, avg_pressure, avg_sound_level);
				lora_send_packet((uint8_t *)message, strlen(message));
				ESP_LOGI("TRANSMISSION", "Données envoyées: %s", message);
			}
			state = SLEEPMODE;
			break;
		case SLEEPMODE:
			ESP_LOGE(pcTaskGetName(NULL), "Sleep mode");
			{
				const int sleep_time_sec = 10; 
				ESP_LOGI("SLEEPMODE", "Entrée en mode Deep Sleep pour %d secondes", sleep_time_sec);

				// Save the next state before entering Deep Sleep
				state = INIT;

				// Configure the timer for wakeup
				esp_sleep_enable_timer_wakeup(sleep_time_sec * 1000000ULL);

				// Enter Deep Sleep mode
				esp_deep_sleep_start();
			}
			break;
		case WAKEUP:
			ESP_LOGE(pcTaskGetName(NULL), "Wake up mode");
			{
				// Check the reason for wakeup
				esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
				if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
				{
					ESP_LOGI("WAKEUP", "Réveillé par le timer");
				}
				else
				{
					ESP_LOGI("WAKEUP", "Réveillé par une autre raison");
				}
			}
			break;
		case ERROR:
			ESP_LOGE(pcTaskGetName(NULL), "Error mode");
			vTaskDelay(pdMS_TO_TICKS(5000));
			state = INIT;
			break;
		}
	}
}