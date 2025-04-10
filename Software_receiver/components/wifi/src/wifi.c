#include "wifi.h" // Include the header file for Wi-Fi module declarations

// Event group handle for synchronizing Wi-Fi connection events
static EventGroupHandle_t s_wifi_event_group;

// Event bit to indicate a successful Wi-Fi connection
static const int WIFI_CONNECTED_BIT = BIT0;

// Counter for retry attempts when connecting to Wi-Fi
static int s_retry_num = 0;

// Tag used for logging messages
static const char *TAG = "wifi_station";

/**
 * @brief Event handler for Wi-Fi and IP events.
 *
 * Handles events such as Wi-Fi start, disconnection, and IP address acquisition.
 *
 * @param arg User-defined argument (unused here).
 * @param event_base Event base (e.g., WIFI_EVENT or IP_EVENT).
 * @param event_id Event ID (e.g., WIFI_EVENT_STA_START, WIFI_EVENT_STA_DISCONNECTED).
 * @param event_data Event-specific data.
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // Attempt to connect to the Wi-Fi network
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // Handle Wi-Fi disconnection
        if (s_retry_num < MAX_RETRY)
        {
            // Retry connecting to the Wi-Fi network
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retrying to connect to the AP...");
        }
        else
        {
            // Notify other tasks that the connection failed
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
        ESP_LOGI(TAG, "Failed to connect to SSID: %s", WIFI_SSID);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // Handle successful IP address acquisition
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0; // Reset retry counter
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initializes the Wi-Fi in station mode.
 *
 * Configures the ESP32 to connect to a Wi-Fi network using the specified SSID and password.
 * Sets up event handlers for Wi-Fi and IP events.
 */
void wifi_init_sta(void)
{
    // Create an event group for Wi-Fi connection events
    s_wifi_event_group = xEventGroupCreate();

    // Initialize the network interface
    ESP_ERROR_CHECK(esp_netif_init());

    // Create a default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create a default Wi-Fi station interface
    esp_netif_create_default_wifi_sta();

    // Prepare the default Wi-Fi initialization configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers for Wi-Fi and IP events
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // Configure the Wi-Fi connection settings (SSID and password)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Require WPA2 security
        },
    };

    // Set the Wi-Fi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Apply the Wi-Fi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // Start the Wi-Fi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    // Log that Wi-Fi has been initialized
    ESP_LOGI(TAG, "WiFi initialized in station mode.");
}