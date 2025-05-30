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

typedef struct {
    float *buffer;
    int sample_count;
    float average;
    SemaphoreHandle_t done_semaphore;
    SemaphoreHandle_t start_signal;
} CapteurContext;

void temperature_task(void *pvParameters) {
    CapteurContext *ctx = (CapteurContext *)pvParameters;
    while (1) {
        xSemaphoreTake(ctx->start_signal, portMAX_DELAY);
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->buffer[i] = temperature_get();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ctx->average = 0.0;
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->average += ctx->buffer[i];
        }
        ctx->average /= ctx->sample_count;
        xSemaphoreGive(ctx->done_semaphore);
        vTaskSuspend(NULL);
    }
}

void pressure_task(void *pvParameters) {
    CapteurContext *ctx = (CapteurContext *)pvParameters;
    while (1) {
        xSemaphoreTake(ctx->start_signal, portMAX_DELAY);
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->buffer[i] = pressure_get();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ctx->average = 0.0;
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->average += ctx->buffer[i];
        }
        ctx->average /= ctx->sample_count;
        xSemaphoreGive(ctx->done_semaphore);
        vTaskSuspend(NULL);
    }
}

void humidity_task(void *pvParameters) {
    CapteurContext *ctx = (CapteurContext *)pvParameters;
    while (1) {
        xSemaphoreTake(ctx->start_signal, portMAX_DELAY);
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->buffer[i] = humidity_get();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ctx->average = 0.0;
        for (int i = 0; i < ctx->sample_count; i++) {
            ctx->average += ctx->buffer[i];
        }
        ctx->average /= ctx->sample_count;
        xSemaphoreGive(ctx->done_semaphore);
        vTaskSuspend(NULL);
    }
}

/*
// Pour ajouter plus tard :
void sound_task(void *pvParameters) { ... }
void air_quality_task(void *pvParameters) { ... }
*/

void app_main(void) {
    enum LoRaState {
        INIT,
        ACQUISITION,
        TRANSMISSION,
        SLEEPMODE,
        WAKEUP,
        ERROR
    };

    static RTC_DATA_ATTR enum LoRaState state = INIT;

    // Définir combien d’échantillons on veut (1 par seconde pendant 10 sec)
    const int sample_count = 10;

    // Buffers alloués sur la pile
    float temp_buffer[sample_count];
    float pressure_buffer[sample_count];
    float humidity_buffer[sample_count]; // Buffer pour l'humidité

    // Contextes capteurs
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

    // Création des tâches capteurs
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
                ESP_LOGE("LoRa", "Module LoRa non détecté.");
                state = ERROR;
                break;
            }

            lora_set_frequency(868e6);
            temperature_init();
            state = ACQUISITION;
            break;

        case ACQUISITION:
            ESP_LOGI("STATE", "ACQUISITION");

            // Reprendre les tâches capteurs
            vTaskResume(temp_task_handle);
            vTaskResume(pressure_task_handle);
            vTaskResume(humidity_task_handle);

            // Déclencher les mesures synchronisées
            xSemaphoreGive(temp_ctx.start_signal);
            xSemaphoreGive(pressure_ctx.start_signal);
            xSemaphoreGive(humidity_ctx.start_signal);

            // Attendre que chaque tâche ait fini
            xSemaphoreTake(temp_ctx.done_semaphore, portMAX_DELAY);
            xSemaphoreTake(pressure_ctx.done_semaphore, portMAX_DELAY);
            xSemaphoreTake(humidity_ctx.done_semaphore, portMAX_DELAY);

            printf("Température moyenne : %.2f°C | Pression moyenne : %.2f hPa | Humidité moyenne : %.2f%%\n",
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
                ESP_LOGI("LoRa", "Message envoyé : %s", message);
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
                ESP_LOGI("WAKEUP", "Raison du réveil : %d", cause);
            }
            break;

        case ERROR:
            ESP_LOGE("STATE", "ERREUR - redémarrage dans 5 sec");
            vTaskDelay(pdMS_TO_TICKS(5000));
            state = INIT;
            break;
        }
    }
}
