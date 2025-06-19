#include "sound.h"
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/i2s.h"
#include "esp_log.h"

#define I2S_NUM I2S_NUM_0
#define I2S_WS 15  // LRCL
#define I2S_SD 25  // DOUT
#define I2S_SCK 13 // BCLK

#define TAG "MIC_SENSOR"

void init_microphone()
{
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX,
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD};

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM);
}

float read_microphone_db()
{
    int32_t buffer[64];
    size_t bytes_read;

    i2s_read(I2S_NUM, (void *)buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

    float sum_squares = 0.0f;
    for (int i = 0; i < 64; i++)
    {
        float sample = (float)(buffer[i] >> 14);
        sum_squares += sample * sample;
    }

    float rms = sqrtf(sum_squares / 64.0f);
    if (rms < 1.0f)
        rms = 1.0f;

    float db = 20.0f * log10f(rms);
    float db_spl = 3.47f * db - 240.0f;

    ESP_LOGI(TAG, "RMS = %.2f | dB = %.2f | dB SPL ~= %.2f", rms, db, db_spl);
    return db_spl;
}

void sound_task(void *pvParameters)
{
    CapteurContext *ctx = (CapteurContext *)pvParameters;
    while (1)
    {
        xSemaphoreTake(ctx->start_signal, portMAX_DELAY);
        for (int i = 0; i < ctx->sample_count; i++)
        {
            ctx->buffer[i] = read_microphone_db();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ctx->average = 0.0;
        for (int i = 0; i < ctx->sample_count; i++)
        {
            ctx->average += ctx->buffer[i];
        }
        ctx->average /= ctx->sample_count;
        xSemaphoreGive(ctx->done_semaphore);
        vTaskSuspend(NULL);
    }
}