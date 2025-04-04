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

	temperature_init();

    while (1)
    {
        // Récupération de la température
   // Lecture initiale des données du capteur
    float current_temp = temperature_get();
    float current_pressure = pressure_get();
    printf("Temperature actuelle: %.2f °C\n", current_temp);
    printf("Pression actuelle: %.2f hPa\n", current_pressure);

    // Délai court avant de démarrer la machine à états (si nécessaire)
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
