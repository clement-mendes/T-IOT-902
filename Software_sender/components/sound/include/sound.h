#ifndef SOUND_H
#define SOUND_H

#include "driver/i2s_std.h"   // For new generation I2S API
#include "esp_err.h"
#include "capteur_context.h"

/**
 * @file sound.h
 * @brief API for sound sensor/microphone acquisition and processing.
 */

/**
 * @brief Initialize the microphone (I2S configuration).
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t mic_init(void);

/**
 * @brief Read data from the microphone.
 * @param buffer Pointer to the buffer to store samples.
 * @param buffer_size Number of samples to read.
 * @param bytes_read Pointer to store the number of bytes actually read.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t mic_read(int32_t *buffer, size_t buffer_size, size_t *bytes_read);

/**
 * @brief Stop the microphone and release resources.
 */
void mic_stop(void);

/**
 * @brief Read the current SPL (Sound Pressure Level) in dB SPL.
 * @return The SPL value in dB SPL (float).
 */
float sound_read_spl(void);

/**
 * @brief FreeRTOS task for sound acquisition and averaging.
 * @param pvParameters Pointer to CapteurContext.
 */
void sound_task(void *pvParameters);

#endif