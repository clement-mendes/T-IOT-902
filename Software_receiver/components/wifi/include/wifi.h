#ifndef WIFI_H
#define WIFI_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#define WIFI_SSID      CONFIG_SSID
#define WIFI_PASS      CONFIG_PASSWORD
#define MAX_RETRY      5

void wifi_init_sta(void);

#endif // WIFI_H