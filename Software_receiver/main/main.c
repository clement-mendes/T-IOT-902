#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "esp_log.h"
#include "lora.h"

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

    while (1)
    {
        switch (state)
        {
        case INIT:
            ESP_LOGI("MAIN", "État INIT");

            // Initialisation WiFi désactivée pour l’instant
            // if (connect_wifi() != 0) {
            //     state = ERROR;
            //     break;
            // }

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
                ESP_LOGI("MAIN", "Paquet reçu (%d bytes): [%.*s]", rxLen, rxLen, buf);
                state = WIFITRANSMISSION;
            }
            else
            {
                ESP_LOGE("MAIN", "Erreur de réception LoRa");
                state = ERROR;
            }
            break;

        case WIFITRANSMISSION:
            ESP_LOGI("MAIN", "État WIFITRANSMISSION - (transmission simulée)");

            // Simulation de traitement / transmission
            vTaskDelay(pdMS_TO_TICKS(5000));

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
