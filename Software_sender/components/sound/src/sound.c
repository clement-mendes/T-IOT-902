#include "sound.h" // Include the header file for sound module declarations
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"  // Utilisation du nouveau driver ADC
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h> // Include math library (not used in this code but may be needed elsewhere)

/**
 * @brief I2S device number to use.
 * 
 * Defines the I2S peripheral to be used. I2S_NUM_0 refers to the first I2S peripheral.
 */
#define I2S_NUM           I2S_NUM_0 

/**
 * @brief Audio sampling rate.
 * 
 * Defines the frequency at which audio samples are captured.
 */
#define I2S_SAMPLE_RATE   16000 

/**
 * @brief Bits per audio sample.
 * 
 * Specifies the resolution of each audio sample in bits.
 */
#define I2S_SAMPLE_BITS   16 

/**
 * @brief Number of DMA buffers.
 * 
 * Defines how many DMA buffers are used for temporary audio data storage.
 */
#define DMA_BUF_COUNT     8 

/**
 * @brief Length of each DMA buffer.
 * 
 * Specifies the size of each DMA buffer in bytes.
 */
#define DMA_BUF_LEN       1024 

/**
 * @brief I2S pin configuration.
 * 
 * Defines the GPIO pins used for I2S communication.
 */
#define I2S_BCK_IO        25   // GPIO pin for BCLK (Bit Clock)
#define I2S_WS_IO         13   // GPIO pin for LRCL (Word Select)
#define I2S_DATA_IN_IO    14   // GPIO pin for DOUT (Data Input)

// ADC Configuration
#define ADC_UNIT        ADC_UNIT_1
#define ADC_CHANNEL     ADC_CHANNEL_6  // GPIO34
#define ADC_ATTEN       ADC_ATTEN_DB_12
#define SAMPLES_COUNT   64  // Nombre d'échantillons pour la mesure sonore
#define NUM_SAMPLES     10  // Nombre de mesures pour la moyenne
#define DELAY_MS        1000 // Délai entre les mesures en ms

static const char *TAG = "SOUND";
static adc_oneshot_unit_handle_t adc1_handle;

/**
 * @brief Initializes the I2S sound module.
 * 
 * This function configures the I2S peripheral with the specified settings,
 * including sample rate, bit depth, and DMA buffer configuration. It also
 * sets up the GPIO pins for I2S communication.
 */
void sound_init(void)
{
    // ADC initialization
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    // Channel configuration
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config));
}

float sound_measure_db(void)
{
    int adc_raw;
    int max_value = 0;
    int min_value = 4095;
    float db_level = 30.0f; // Base noise level
    
    // Take samples quickly
    for (int i = 0; i < SAMPLES_COUNT; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));
        
        // Track min and max for peak-to-peak calculation
        if (adc_raw > max_value) max_value = adc_raw;
        if (adc_raw < min_value) min_value = adc_raw;
        
        esp_rom_delay_us(50);
    }
    
    // Calculate peak-to-peak amplitude
    int peak_to_peak = max_value - min_value;
    
    // Convert to decibels if we have valid readings
    if (peak_to_peak > 0) {
        db_level = 20 * log10f(peak_to_peak / 100.0f);
        db_level = db_level + 50;  // Offset for 0dB attenuation
        
        // Clamp values to reasonable range
        if (db_level < 30) db_level = 30;
        if (db_level > 120) db_level = 120;
    }
    
    return db_level;
}

// Optional: Debug function to get raw sound data
int sound_read_raw(int16_t *buffer, size_t length)
{
    int adc_raw;
    int samples_to_read = (length < SAMPLES_COUNT) ? length : SAMPLES_COUNT;
    
    for (int i = 0; i < samples_to_read; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));
        buffer[i] = (int16_t)adc_raw;
        esp_rom_delay_us(50);
    }
    
    // Fill remaining buffer if necessary
    for (int i = samples_to_read; i < length; i++) {
        buffer[i] = buffer[samples_to_read - 1];
    }
    
    return length;
}

float sound_get_average_db(void)
{
    float sound_sum = 0.0f;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float db = sound_measure_db();
        sound_sum += db;
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    float avg_sound = sound_sum / NUM_SAMPLES;
    
    return avg_sound;
}
