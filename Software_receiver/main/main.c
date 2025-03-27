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
    WIFITRANSMISSION,
    ERROR
} LoRaState;

LoRaState state = INIT; // Initialisation de l'état

/**
 * @file main.c
 * @brief Programme to display Hello World.
 */

void task_rx(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(NULL), "Start RX Task");
    uint8_t buf[256]; // Taille max du payload SX1276/77/78/79 = 255

    while (1)
    {
        lora_receive();

        if (lora_received())
        {
            int rxLen = lora_receive_packet(buf, sizeof(buf));
            ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received: [%.*s]", rxLen, rxLen, buf);
            state = WIFITRANSMISSION;
        }
        vTaskDelay(1);
    }
}

void app_main(void)
{
    while (1)
    {
        switch (state)
        {
        case INIT:
            if (lora_init() == 0)
            {
                ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the lora module verify the connections and configuration");
                state = ERROR;
            }
            lora_set_frequency(868e6);
            state = ACQUISITION;
            break;
        case ACQUISITION:
            xTaskCreate(&task_rx, "RX", 1024 * 3, NULL, 5, NULL);
            break;
        case WIFITRANSMISSION:
            ESP_LOGE(pcTaskGetName(NULL), "Transmission mode");
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