/**
 * @file sound.c
 * @brief Sound acquisition and processing module for SPH0645 microphone (I2S).
 *
 * This module provides initialization, reading, and task management for the SPH0645 digital microphone
 * using the ESP-IDF I2S driver. It includes SPL (Sound Pressure Level) calculation and FreeRTOS task integration.
 */

#include "sound.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/i2s_std.h" 
#include <math.h>


#define I2S_BCK_IO        25   ///< GPIO pin for I2S BCLK
#define I2S_WS_IO         13   ///< GPIO pin for I2S WS
#define I2S_DATA_IN_IO    14   ///< GPIO pin for I2S data input
#define SAMPLE_RATE       16000 ///< Audio sample rate (Hz)
#define MIC_DBFS_BUFFER_SIZE 1024 ///< Buffer size for SPL calculation

static const char* TAG = "sound";
static i2s_chan_handle_t rx_handle = NULL;
int32_t buffer[MIC_DBFS_BUFFER_SIZE];

/**
 * @brief Initialize the SPH0645 microphone (I2S).
 *
 * Configures the I2S peripheral and prepares the SPH0645 microphone for data acquisition.
 *
 * @return ESP_OK on success, or an error code from the ESP-IDF I2S driver.
 */
esp_err_t mic_init(void)
{
    // I2S configuration
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S channel creation failed");
        return ret;
    }
    
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_IN_IO,
        },
    };
    
    ret = i2s_channel_init_std_mode(rx_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S standard mode initialization failed");
        return ret;
    }
    
    ret = i2s_channel_enable(rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2S channel enable failed");
        return ret;
    }
    
    ESP_LOGI(TAG, "SPH0645 detected and initialized");
    return ESP_OK;
}

/**
 * @brief Read raw audio data from the SPH0645 microphone.
 *
 * Reads a buffer of audio samples from the I2S interface.
 *
 * @param[out] buffer Pointer to the buffer where audio data will be stored.
 * @param[in] buffer_size Size of the buffer in bytes.
 * @param[out] bytes_read Pointer to store the number of bytes actually read.
 * @return ESP_OK on success, or an error code from the ESP-IDF I2S driver.
 */
esp_err_t mic_read(int32_t *buffer, size_t buffer_size, size_t *bytes_read)
{
    if (rx_handle == NULL) {
        ESP_LOGE(TAG, "Microphone not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    return i2s_channel_read(rx_handle, buffer, buffer_size, bytes_read, portMAX_DELAY);
}

/**
 * @brief Read and return the current SPL (Sound Pressure Level) value.
 *
 * @return The SPL value in dB SPL, or -1.0f on error.
 */
float sound_read_spl(void) {
    size_t bytes_read = 0;
    if (mic_read(buffer, sizeof(buffer), &bytes_read) != ESP_OK) {
        return -1.0f;
    }
    size_t samples = bytes_read / sizeof(int32_t);
    if (samples == 0) {
        return -1.0f;
    }
    double max = 0;
    for (size_t i = 0; i < samples; i++) {
        int32_t sample = buffer[i] >> 14;
        if (fabs(sample) > max) max = fabs(sample);
    }
    double db_peak = 20.0 * log10(max / 131071.0);
    double db_spl = (db_peak + 20) * 9.25 + 15.6;
    return (float)db_spl;
}

/**
 * @brief FreeRTOS task for SPL acquisition and averaging.
 *
 * This task waits for a start signal, acquires SPL values at 1 Hz for the specified sample count,
 * computes the average, and signals completion via a semaphore.
 *
 * @param pvParameters Pointer to a CapteurContext structure.
 */
void sound_task(void *pvParameters) {
    CapteurContext *ctx = (CapteurContext *)pvParameters;
    while (1) {
        // Wait for start signal
        xSemaphoreTake(ctx->start_signal, portMAX_DELAY);
        float sum = 0;
        for (int i = 0; i < ctx->sample_count; i++) {
            float spl = sound_read_spl();
            ctx->buffer[i] = spl;
            sum += spl;
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ctx->average = sum / ctx->sample_count;
        xSemaphoreGive(ctx->done_semaphore);
        vTaskSuspend(NULL); // Suspend task until next cycle
    }
}