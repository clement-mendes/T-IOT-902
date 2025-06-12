/**
 * @file main.c
 * @brief Main application for LoRa sender (ESP32).
 *
 * This application acquires sensor data, computes averages, and sends the results via LoRa.
 * It uses FreeRTOS tasks for sensor acquisition and a state machine for operation flow.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "lora.h"
#include "temperature.h"
#include "esp_sleep.h"
#include "esp_system.h"
// #include "sound.h"
// #include "air_quality.h"

/**
 * @brief Main application entry point.
 *
 * Implements a state machine for sensor acquisition, LoRa transmission, and sleep management.
 */
void app_main(void) {
    /**
     * @enum LoRaState
     * @brief State machine for LoRa sender logic.
     */
    enum LoRaState {
        INIT,           ///< Initialization state
        ACQUISITION,    ///< Sensor acquisition state
        TRANSMISSION,   ///< LoRa transmission state
        SLEEPMODE,      ///< Deep sleep state
        WAKEUP,         ///< Wakeup state
        ERROR           ///< Error state
    };

    static RTC_DATA_ATTR enum LoRaState state = INIT;

    // Define number of samples (1 per second for 10 seconds)
    const int sample_count = 10;

    // Buffers allocated on the stack
    float temp_buffer[sample_count];
    float pressure_buffer[sample_count];
    float humidity_buffer[sample_count];

    // Sensor contexts
    CapteurContext temp_ctx = {
        .buffer = temp_buffer,
        .sample_count = sample_count,
        .done_semaphore = xSemaphoreCreateBinary(),
        .start_signal = xSemaphoreCreateBinary()
    };

    CapteurContext pressure_ctx = {
        .buffer = pressure_buffer,
        .sample_count = sample_count,
        .done_semaphore = xSemaphoreCreateBinary(),
        .start_signal = xSemaphoreCreateBinary()
    };

    CapteurContext humidity_ctx = {
        .buffer = humidity_buffer,
        .sample_count = sample_count,
        .done_semaphore = xSemaphoreCreateBinary(),
        .start_signal = xSemaphoreCreateBinary()
    };

    // Create sensor tasks (now declared in temperature.h)
    TaskHandle_t temp_task_handle;
    TaskHandle_t pressure_task_handle;
    TaskHandle_t humidity_task_handle;

    xTaskCreate(temperature_task, "TempTask", 2048, &temp_ctx, 5, &temp_task_handle);
    xTaskCreate(pressure_task, "PressureTask", 2048, &pressure_ctx, 5, &pressure_task_handle);
    xTaskCreate(humidity_task, "HumidityTask", 2048, &humidity_ctx, 5, &humidity_task_handle);

    while (1) {
        switch (state) {
        case INIT:
            ESP_LOGI("STATE", "INIT");
            if (lora_init() == 0) {
                ESP_LOGE("LoRa", "LoRa module not detected.");
                state = ERROR;
                break;
            }

            lora_set_frequency(868e6);
            temperature_init();
            state = ACQUISITION;
            break;

        case ACQUISITION:
            ESP_LOGI("STATE", "ACQUISITION");

            // Resume sensor tasks
            vTaskResume(temp_task_handle);
            vTaskResume(pressure_task_handle);
            vTaskResume(humidity_task_handle);

            // Trigger synchronized measurements
            xSemaphoreGive(temp_ctx.start_signal);
            xSemaphoreGive(pressure_ctx.start_signal);
            xSemaphoreGive(humidity_ctx.start_signal);

            // Wait for each task to finish
            xSemaphoreTake(temp_ctx.done_semaphore, portMAX_DELAY);
            xSemaphoreTake(pressure_ctx.done_semaphore, portMAX_DELAY);
            xSemaphoreTake(humidity_ctx.done_semaphore, portMAX_DELAY);

            printf("Average temperature: %.2f°C | Average pressure: %.2f hPa | Average humidity: %.2f%%\n",
                   temp_ctx.average, pressure_ctx.average, humidity_ctx.average);

            state = TRANSMISSION;
            break;

        case TRANSMISSION:
            ESP_LOGI("STATE", "TRANSMISSION");
            {
                char message[128];
                snprintf(message, sizeof(message),
                         "{\"temp\":%.2f,\"press\":%.2f,\"hum\":%.2f}",
                         temp_ctx.average, pressure_ctx.average, humidity_ctx.average);
                lora_send_packet((uint8_t *)message, strlen(message));
                ESP_LOGI("LoRa", "Message sent: %s", message);
            }
            state = SLEEPMODE;
            break;

        case SLEEPMODE:
            ESP_LOGI("STATE", "SLEEPMODE");
            {
                const int sleep_time_sec = 10;
                state = INIT;
                esp_sleep_enable_timer_wakeup(sleep_time_sec * 1000000ULL);
                esp_deep_sleep_start();
            }
            break;

        case WAKEUP:
            ESP_LOGI("STATE", "WAKEUP");
            {
                esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
                ESP_LOGI("WAKEUP", "Wakeup cause: %d", cause);
            }
            break;

        case ERROR:
            ESP_LOGE("STATE", "ERROR - restarting in 5 sec");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = INIT;
            break;
        }
    }
}
