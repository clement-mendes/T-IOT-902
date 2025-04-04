/**
 * @file dust_sensor.c
 * @brief Implementation of the dust sensor module for measuring air quality.
 *
 * This module provides functions to initialize and read data from a dust sensor.
 * The sensor uses an LED and an ADC to measure the density of dust particles in the air.
 * The measured values are filtered and converted into a dust density in µg/m³.
 */

#include "dust_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define ILED_PIN GPIO_NUM_4
#define NO_DUST_VOLTAGE 400.0f  // Tension sans poussière en mv
#define COV_RATIO 0.2f        // Facteur de conversion pour la densité en µg/m³

/**
 * @brief Initializes the dust sensor module.
 *
 * This function configures the GPIO pin for the LED and sets up the ADC for reading
 * the sensor's output. The LED is turned off initially.
 */
void dust_sensor_init(void) {
    gpio_set_direction(ILED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ILED_PIN, 1);  // Éteindre la LED au départ

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);  // GPIO32
}

/**
 * @brief Reads the dust density from the sensor.
 *
 * This function performs the following steps:
 * - Turns on the LED to activate the sensor.
 * - Waits for a short period to stabilize the reading.
 * - Reads the raw ADC value from the sensor.
 * - Applies a simple moving average filter to smooth the readings.
 * - Converts the filtered ADC value into a voltage.
 * - Calculates the dust density in µg/m³ based on the voltage.
 *
 * @return The calculated dust density in µg/m³. Returns 0.0 if the voltage is below
 *         the no-dust threshold.
 */
float dust_sensor_read(void) {
    gpio_set_level(ILED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    int adc_value = adc1_get_raw(ADC1_CHANNEL_0);

    gpio_set_level(ILED_PIN, 0); 

    // Suming filter
    static int buffer[10] = {0};
    static int index = 0;
    static int sum = 0;
    sum -= buffer[index];
    buffer[index] = adc_value;
    sum += buffer[index];
    index = (index + 1) % 10;
    int filtered_value = sum / 10;

    // Convert ADC value to voltage
    float voltage = ( 3300/ 4096.0f) * filtered_value * 11.0f;
    if (voltage >= NO_DUST_VOLTAGE) {
        voltage -= NO_DUST_VOLTAGE;
        return voltage * COV_RATIO;
    } else {
        return 0.0f;
    }
}
