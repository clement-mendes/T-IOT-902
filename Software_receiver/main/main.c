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
        ESP_LOGI("API", "Données envoyées à l'API avec succès");
    }
    else
    {
        ESP_LOGE("API", "Échec de l'envoi des données à l'API");
    }
    esp_http_client_cleanup(client);
}

void app_main(void)
{
    // Énumération des états du système
    typedef enum
    {
        INIT,
        ACQUISITION,
        WIFITRANSMISSION,
        ERROR
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
            ESP_LOGI("MAIN", "État INIT");

            wifi_init_sta();

            // Wait for WiFi connection before proceeding
            int wifi_retry = 0;
            while (!wifi_is_connected() && wifi_retry < 50)
            { // 50 * 100ms = 5s max wait
                vTaskDelay(pdMS_TO_TICKS(100));
                wifi_retry++;
            }
            if (!wifi_is_connected())
            {
                ESP_LOGE("MAIN", "WiFi non connecté");
                state = ERROR;
                break;
            }

            if (lora_init() == 0)
            {
                ESP_LOGE("MAIN", "Module LoRa non détecté");
                state = ERROR;
                break;
            }

            lora_set_frequency(868e6);

            state = ACQUISITION;
            break;

        case ACQUISITION:
            ESP_LOGI("MAIN", "État ACQUISITION - en attente de message LoRa");

            uint8_t buf[256];
            lora_receive(); // Met le module en mode réception

            while (!lora_received())
            {
                vTaskDelay(pdMS_TO_TICKS(100)); // Attente active avec délai
            }

            int rxLen = lora_receive_packet(buf, sizeof(buf));
            if (rxLen > 0)
            {
                state = WIFITRANSMISSION;
            }
            else
            {
                ESP_LOGE("MAIN", "Erreur de réception LoRa");
                state = ERROR;
            }
            break;

        case WIFITRANSMISSION:
            ESP_LOGI("MAIN", "Paquet reçu (%d bytes): [%.*s]", rxLen, rxLen, buf);
            // Envoyer les données reçues à l'API
            char json_data[300];
            // Si les données reçues sont déjà un tableau JSON, il suffit de les transférer
            snprintf(json_data, sizeof(json_data), "%.*s", rxLen, buf);
            send_data_to_api(json_data);
            state = ACQUISITION;
            break;

        case ERROR:
            ESP_LOGE("MAIN", "État ERREUR - redémarrage dans 5 sec");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = INIT;
            break;
        }
    }
}
