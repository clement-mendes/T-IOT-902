/**
 * @file main.c
 * @brief Main program using LoRa with FreeRTOS.
 *
 * This program manages a state machine for LoRa communication,
 * including INIT, ACQUISITION, WIFITRANSMISSION, and ERROR states.
 * It uses FreeRTOS for task management and delays.
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "esp_log.h"
#include "lora.h"

/**
 * @brief LoRa reception task.
 *
 * This function continuously listens for LoRa packets and updates
 * the system state if data is received.
 *
 */
// void task_rx()
// {
//     ESP_LOGI(pcTaskGetName(NULL), "Start RX Task");
//     uint8_t buf[256]; // Max payload size for SX1276/77/78/79 is 255 bytes
//
//     while (1)
//     {
//         lora_receive();
//
//         if (lora_received())
//         {
//             int rxLen = lora_receive_packet(buf, sizeof(buf));
//             ESP_LOGI(pcTaskGetName(NULL), "%d byte packet received: [%.*s]", rxLen, rxLen, buf);
//             state = WIFITRANSMISSION;
//         }
//         vTaskDelay(1);
//     }
// }

/**
 * @brief Function to connect to WiFi.
 *
 * This function initializes the NVS flash and connects to the WiFi network.
 * It retries the connection up to 5 times before failing.
 */
// int connect_wifi()
// {
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
//     {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);

//     int retry_count = 0;
//     while (retry_count < 5)
//     {
//         if (wifi_init_sta() == ESP_OK)
//         {
//             ESP_LOGI("WiFi", "Connected successfully");
//             return 0;
//         }
//         ESP_LOGW("WiFi", "Connection attempt %d failed", retry_count + 1);
//         retry_count++;
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
//     ESP_LOGE("WiFi", "Failed to connect after 5 attempts");
//     return -1;
// }

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
        INIT,             /**< Initialization mode. */
        ACQUISITION,      /**< Data acquisition mode. */
        WIFITRANSMISSION, /**< WiFi data transmission mode. */
        ERROR             /**< Error mode. */
    };

    /**
     * @var state
     * @brief Current state of the LoRa system.
     */
    enum LoRaState state = INIT; // Initialization state

    while (1)
    {
        switch (state)
        {
        case INIT:
            // if (connect_wifi() != 0)
            // {
            //     state = ERROR;
            //     break;
            // }
            if (lora_init() == 0)
            {
                ESP_LOGE(pcTaskGetName(NULL), "Does not recognize the LoRa module. Verify the connections and configuration.");
                state = ERROR;
            }
            lora_set_frequency(868e6);
            state = ACQUISITION;
            break;
        case ACQUISITION:
            /**
             * @brief Creates a task to handle LoRa reception.
             *
             * This function spawns a FreeRTOS task that continuously listens
             * for incoming LoRa packets and updates the system state accordingly.
             */
            // xTaskCreate(&task_rx, "RX", 1024 * 3, NULL, 5, NULL);
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