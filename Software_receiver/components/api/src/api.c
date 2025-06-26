#include "api.h"
#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"


void send_data_to_api(const char *json_data)
{
    esp_http_client_config_t config = {
        .url = "http://172.20.10.9:3000/espdata", // TODO: make configurable
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
