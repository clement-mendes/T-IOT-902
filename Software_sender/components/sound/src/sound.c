#include "sound.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2s_std.h"   // Pour l'API I2S nouvelle génération
#include <math.h>


#define I2S_BCK_IO        25
#define I2S_WS_IO         13
#define I2S_DATA_IN_IO    14
#define SAMPLE_RATE       16000
#define MIC_DBFS_BUFFER_SIZE 1024

static const char* TAG = "SPH0645";
static i2s_chan_handle_t rx_handle = NULL;
int32_t buffer[MIC_DBFS_BUFFER_SIZE];


esp_err_t mic_init(void)
{
    // Configuration I2S
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    esp_err_t ret = i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur création canal I2S");
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
        ESP_LOGE(TAG, "Erreur init I2S");
        return ret;
    }
    
    ret = i2s_channel_enable(rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Erreur activation I2S");
        return ret;
    }
    
    ESP_LOGI(TAG, "Microphone initialisé");
    return ESP_OK;
}

esp_err_t mic_read(int32_t *buffer, size_t buffer_size, size_t *bytes_read)
{
    if (rx_handle == NULL) {
        ESP_LOGE(TAG, "Microphone non initialisé");
        return ESP_ERR_INVALID_STATE;
    }
    
    return i2s_channel_read(rx_handle, buffer, buffer_size, bytes_read, portMAX_DELAY);
}

void mic_stop(void)
{
    if (rx_handle != NULL) {
        i2s_channel_disable(rx_handle);
        i2s_del_channel(rx_handle);
        rx_handle = NULL;
        ESP_LOGI(TAG, "Microphone arrêté");
    }
}



void sound_print_dbfs(void) {
    size_t bytes_read = 0;
    if (mic_read(buffer, sizeof(buffer), &bytes_read) != ESP_OK) {
        printf("Erreur lecture micro\n");
        return;
    }
    size_t samples = bytes_read / sizeof(int32_t);
    if (samples == 0) {
        printf("Aucun échantillon lu\n");
        return;
    }
    double sum = 0;
    double max = 0;
    for (size_t i = 0; i < samples; i++) {
        int32_t sample = buffer[i] >> 14; // 18 bits utiles
        sum += sample * sample;
        if (fabs(sample) > max) max = fabs(sample);
    }
    double rms = sqrt(sum / samples);
    double db = 20.0 * log10(rms / 131071.0); // 131071 = 2^17-1
    double db_peak = 20.0 * log10(max / 131071.0);
    // Calcul SPL basé sur le pic
    double db_spl = (db_peak + 20) * 9.25 + 15.6;
    int bars = (int)((db_peak + 60) / 3); // 0 à 20 barres
    if (bars < 0) bars = 0;
    if (bars > 20) bars = 20;
    printf("Niveau sonore (pic) : %.2f dB SPL | ", db_spl);
    for (int i = 0; i < bars; i++) printf("|");
    printf("\n");
    printf("(dBFS RMS : %.2f | dBFS Pic : %.2f)\n", db, db_peak);
}