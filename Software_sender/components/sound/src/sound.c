#include "sound.h" // Include the header file for sound module declarations
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"  // Utilisation du nouveau driver ADC
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
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
#define ADC_ATTEN       ADC_ATTEN_DB_11  // Increased attenuation for wider voltage range
#define SAMPLES_COUNT   64   // Reduced sample count for better stability
#define NUM_SAMPLES     5    // Reduced number of samples
#define DELAY_MS        100  // Shorter delay between samples

// Désactiver l'alimentation par GPIO
#define SOUND_VCC_PIN   -1    // Utiliser une alimentation externe stable
#define SOUND_GND_PIN   -1    // Connecter à GND directement
#define SOUND_OUT_PIN   34    // ADC1_CHANNEL_6 correspond au GPIO34

// Improved filtering parameters
#define ADC_SAMPLES_PER_READ 16    // Reduced samples per read
#define ADC_FILTER_THRESHOLD 1000  // More permissive threshold
#define MOVING_AVG_SIZE      4     // Smaller moving average window
#define MIN_VALID_READING    100   // Minimum valid reading threshold
#define MAX_VALID_READING    4000  // Maximum valid reading threshold

static const char *TAG = "SOUND";
static adc_oneshot_unit_handle_t adc1_handle;
static float moving_average_buffer[MOVING_AVG_SIZE];
static int buffer_index = 0;

bool sound_check_connections(void)
{
    ESP_LOGI(TAG, "Vérification des connexions du capteur de son...");

    // Configuration du pin VCC en sortie (si utilisé)
    if (SOUND_VCC_PIN >= 0) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << SOUND_VCC_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
        gpio_set_level(SOUND_VCC_PIN, 1);  // Alimenter le capteur
        ESP_LOGI(TAG, "VCC configuré sur GPIO%d", SOUND_VCC_PIN);
    }

    // Test de lecture ADC
    int adc_raw;
    int min_val = 4095;
    int max_val = 0;
    int zero_count = 0;
    int max_count = 0;
    int mid_range_count = 0;
    bool signal_variation = false;

    // Prendre plusieurs lectures pour voir si le signal varie
    ESP_LOGI(TAG, "Test de lecture ADC sur 3 secondes...");
    for (int i = 0; i < 30; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));
        
        if (adc_raw < min_val) min_val = adc_raw;
        if (adc_raw > max_val) max_val = adc_raw;

        // Compter les types de lectures
        if (adc_raw == 0) zero_count++;
        else if (adc_raw == 4095) max_count++;
        else mid_range_count++;

        ESP_LOGI(TAG, "Lecture ADC #%d: %d", i+1, adc_raw);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Analyser les résultats
    int variation = max_val - min_val;
    signal_variation = (variation > 100);

    ESP_LOGI(TAG, "Résultats du diagnostic détaillé:");
    ESP_LOGI(TAG, "- Pin OUT (GPIO%d): %s", SOUND_OUT_PIN,
             (min_val < 4095 && max_val > 0) ? "Connecté" : "Problème détecté");
    ESP_LOGI(TAG, "- Variation du signal: %d (min: %d, max: %d)",
             variation, min_val, max_val);
    ESP_LOGI(TAG, "- Statistiques des lectures:");
    ESP_LOGI(TAG, "  * Lectures à 0: %d/30", zero_count);
    ESP_LOGI(TAG, "  * Lectures à 4095: %d/30", max_count);
    ESP_LOGI(TAG, "  * Lectures intermédiaires: %d/30", mid_range_count);

    // Analyse des problèmes potentiels
    if (zero_count > 5 && max_count > 5) {
        ESP_LOGE(TAG, "ERREUR: Signal instable - Vérifiez les connexions physiques");
    }
    if (max_count > 20) {
        ESP_LOGE(TAG, "ERREUR: Signal majoritairement saturé - Vérifiez l'alimentation ou réduisez le gain");
    }
    if (zero_count > 20) {
        ESP_LOGE(TAG, "ERREUR: Signal majoritairement nul - Vérifiez l'alimentation");
    }
    if (mid_range_count == 0) {
        ESP_LOGE(TAG, "ERREUR: Aucune lecture intermédiaire - Problème de connexion ou de configuration");
    }

    return signal_variation && (mid_range_count > 0);
}

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

    // Vérifier les connexions au démarrage
    if (!sound_check_connections()) {
        ESP_LOGE(TAG, "Problème détecté avec le capteur de son!");
    }
}

static float apply_moving_average(float new_value) {
    moving_average_buffer[buffer_index] = new_value;
    buffer_index = (buffer_index + 1) % MOVING_AVG_SIZE;
    
    float sum = 0;
    for(int i = 0; i < MOVING_AVG_SIZE; i++) {
        sum += moving_average_buffer[i];
    }
    return sum / MOVING_AVG_SIZE;
}

static float filter_adc_reading(void)
{
    int32_t sum = 0;
    int valid_readings = 0;
    int32_t last_reading = -1;
    
    // Take multiple quick readings with stabilization delay
    for(int i = 0; i < ADC_SAMPLES_PER_READ; i++) {
        int adc_raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adc_raw));
        
        // Basic validity check
        if(adc_raw >= MIN_VALID_READING && adc_raw <= MAX_VALID_READING) {
            // Only include readings that don't change too drastically
            if(last_reading == -1 || abs(adc_raw - last_reading) < ADC_FILTER_THRESHOLD) {
                sum += adc_raw;
                valid_readings++;
            }
        }
        
        last_reading = adc_raw;
        esp_rom_delay_us(500);  // Increased delay between readings
    }
    
    if(valid_readings < ADC_SAMPLES_PER_READ / 2) {
        return -1.0f;  // Not enough valid readings
    }
    
    return (float)sum / valid_readings;
}

float sound_measure_db(void)
{
    float db_level = 30.0f;
    int valid_samples = 0;
    float sum = 0.0f;
    
    // Take multiple filtered samples with longer delays
    for(int i = 0; i < SAMPLES_COUNT; i++) {
        float filtered_value = filter_adc_reading();
        if(filtered_value >= 0) {
            sum += filtered_value;
            valid_samples++;
        }
        esp_rom_delay_us(1000);  // 1ms delay between samples
    }
    
    // Calculate average of valid readings
    if(valid_samples > 0) {
        float avg_value = sum / valid_samples;
        avg_value = apply_moving_average(avg_value);
        
        // Improved dB calculation with better scaling
        if(avg_value > 0) {
            // Normalize to 0-1 range and calculate dB
            float normalized = (avg_value - MIN_VALID_READING) / (MAX_VALID_READING - MIN_VALID_READING);
            db_level = 30.0f + (normalized * 60.0f);  // Scale to 30-90 dB range
            
            // Apply soft limiting
            if(db_level < 30) db_level = 30;
            if(db_level > 90) db_level = 90;
        }
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
