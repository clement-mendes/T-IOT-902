#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lora.h"

/**
 * @file main.c
 * @brief Programme to display Hello World.
 */

 void task_rx(void *pvParameters)
 {
     ESP_LOGI(pcTaskGetName(NULL), "Start");
     uint8_t buf[256]; // Maximum Payload size of SX1276/77/78/79 is 255
     while(1) {
         lora_receive(); // put into receive mode
         if (lora_received()) {
             int rxLen = lora_receive_packet(buf, sizeof(buf));
             ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received:[%.*s]", rxLen, rxLen, buf);
         }
         vTaskDelay(1); // Avoid WatchDog alerts
     } // end while
 }

void app_main(void) {
    if (lora_init() == 0) {
		ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the module");
		while(1) {
			vTaskDelay(1);
		}
	}
    ESP_LOGI(pcTaskGetName(NULL), "Frequency is 866MHz");
	lora_set_frequency(868e6);   
    
    lora_enable_crc();

	int cr = 1;
	int bw = 7;
	int sf = 7;
    lora_set_coding_rate(cr);

	ESP_LOGI(pcTaskGetName(NULL), "coding_rate=%d", cr);

	lora_set_bandwidth(bw);

	ESP_LOGI(pcTaskGetName(NULL), "bandwidth=%d", bw);

	lora_set_spreading_factor(sf);

	ESP_LOGI(pcTaskGetName(NULL), "spreading_factor=%d", sf);
    xTaskCreate(&task_rx, "TX", 1024*3, NULL, 5, NULL);

}