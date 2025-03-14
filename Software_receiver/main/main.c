#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "wifi.h" // Inclure le module WiFi
#include "sdkconfig.h"

/**
 * @file main.c
 * @brief Programme to connect to a WiFi network
 */

void app_main(void) {
    printf("SSID - %s\n",CONFIG_SSID);
    printf("PASSWORD - %s\n",CONFIG_PASSWORD);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}