/**
 * @file main.c
 * @brief LoRa receiver main application for ESP32.
 * 
 * This application receives data via LoRa, connects to Wi-Fi,
 * and forwards received data to a remote API using HTTP POST.
 * 
 * @author Epitech
 * @date  30/05/2025
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "esp_log.h"
#include "lora.h"
#include "esp_http_client.h"

/**
 * @brief Send JSON data to the remote API via HTTP POST.
 * 
 * @param json_data The JSON string to send.
 */
void send_data_to_api(const char *json_data)
{
    esp_http_client_config_t config = {
        .url = "http://172.20.10.9:3000/espdata", 
        .method = HTTP_METHOD_POST,
        .timeout_ms = 3000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI("API", "Data sent to API successfully");
    }
    else
    {
        ESP_LOGE("API", "Failed to send data to API");
    }
    esp_http_client_cleanup(client);
}

/**
 * @brief Main application entry point.
 * 
 * Implements a state machine for LoRa reception and API forwarding.
 */
void app_main(void)
{
    /**
     * @enum LoRaState
     * @brief State machine for LoRa receiver logic.
     */
    typedef enum
    {
        INIT,               ///< Initialization state
        ACQUISITION,        ///< Waiting for LoRa data
        WIFITRANSMISSION,   ///< Data received, sending to API
        ERROR               ///< Error state
    } LoRaState;

    LoRaState state = INIT;

    // Initialize NVS before using WiFi or any component that needs NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    while (1)
    {
        switch (state)
        {
        case INIT:
            ESP_LOGI("MAIN", "State: INIT");

            // Initialize Wi-Fi in station mode
            wifi_init_sta();

            // Wait for WiFi connection before proceeding (max 5 seconds)
            int wifi_retry = 0;
            while (!wifi_is_connected() && wifi_retry < 50)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
                wifi_retry++;
            }
            if (!wifi_is_connected())
            {
                ESP_LOGE("MAIN", "WiFi not connected");
                state = ERROR;
                break;
            }

            // Initialize LoRa module
            if (lora_init() == 0)
            {
                ESP_LOGE("MAIN", "LoRa module not detected");
                state = ERROR;
                break;
            }

            lora_set_frequency(868e6);

            state = ACQUISITION;
            break;

        case ACQUISITION:
            ESP_LOGI("MAIN", "State: ACQUISITION - waiting for LoRa message");

            uint8_t buf[256];
            lora_receive(); // Set LoRa module to receive mode

            // Wait until a LoRa packet is received
            while (!lora_received())
            {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            int rxLen = lora_receive_packet(buf, sizeof(buf));
            if (rxLen > 0)
            {
                state = WIFITRANSMISSION;
            }
            else
            {
                ESP_LOGE("MAIN", "LoRa receive error");
                state = ERROR;
            }
            break;

        case WIFITRANSMISSION:
            ESP_LOGI("MAIN", "Packet received (%d bytes): [%.*s]", rxLen, rxLen, buf);
            // Send received data to API
            char json_data[300];
            snprintf(json_data, sizeof(json_data), "%.*s", rxLen, buf);
            send_data_to_api(json_data);
            state = ACQUISITION;
            break;

        case ERROR:
            ESP_LOGE("MAIN", "State: ERROR - restarting in 5 seconds");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = INIT;
            break;
        }
    }
}
