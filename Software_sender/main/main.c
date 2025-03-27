#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lora.h"

typedef enum
{
	INIT,
	ACQUISITION,
	TRANSMISSION,
	SLEEPMODE,
	WAKEUP,
	ERROR
} LoRaState;

LoRaState state = INIT;

/**
 * @file main.c
 * @brief Programme to display Hello World.
 */

 void sendPacket()
 {
	 const char *msg = "Hello World";
	 int send_len = strlen(msg);
 
	 lora_send_packet((uint8_t *)msg, send_len);
	 ESP_LOGI("STATE", "%d byte packet sent...", send_len);
 }
 

void app_main(void)
{
	while (1)
	{
		switch (state)
		{
		case INIT:
			ESP_LOGE(pcTaskGetName(NULL), "Init mode");
			if (lora_init() == 0)
			{
				ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the lora module verify the connections and configuration");
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
			sendPacket();
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