#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h" // FreeRTOS base definitions
#include "freertos/event_groups.h" // FreeRTOS event groups for synchronization
#include "esp_wifi.h" // ESP-IDF Wi-Fi driver
#include "esp_event.h" // ESP-IDF event loop library
#include "esp_log.h" // ESP-IDF logging library
#include "nvs_flash.h" // Non-volatile storage (NVS) library
#include "sdkconfig.h" // Project configuration settings

// Wi-Fi SSID (configured in sdkconfig)
#define WIFI_SSID      CONFIG_SSID

// Wi-Fi password (configured in sdkconfig)
#define WIFI_PASS      CONFIG_PASSWORD

// Maximum number of connection retry attempts
#define MAX_RETRY      5

/**
 * @brief Initializes the Wi-Fi in station mode.
 */
void wifi_init_sta(void);

/**
 * @brief Returns true if the ESP32 is connected to Wi-Fi, false otherwise.
 */
bool wifi_is_connected(void);

#endif // WIFI_H