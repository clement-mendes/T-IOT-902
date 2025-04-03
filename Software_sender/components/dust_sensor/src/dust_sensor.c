#include "dust_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define ILED_PIN GPIO_NUM_4
#define NO_DUST_VOLTAGE 400.0f  // Tension sans poussière en mv
#define COV_RATIO 0.2f        // Facteur de conversion pour la densité en µg/m³

void dust_sensor_init(void) {
    // Initialisation de la broche ILED pour contrôler la LED
    gpio_set_direction(ILED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ILED_PIN, 1);  // Éteindre la LED au départ

    // Initialisation de la broche DOUT pour la lecture analogique
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0);  // GPIO32
}

float dust_sensor_read(void) {
    // Allumer la LED pendant la mesure
    gpio_set_level(ILED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // Lire la valeur analogique
    int adc_value = adc1_get_raw(ADC1_CHANNEL_0);
    ESP_LOGI(pcTaskGetName(NULL), "Dust density: %d µg/m³", adc_value); // Affichage de la valeur

    gpio_set_level(ILED_PIN, 1);  // Éteindre la LED après la mesure

    // Filtrage simple (moyenne sur 10 lectures)
    static int buffer[10] = {0};
    static int index = 0;
    static int sum = 0;
    sum -= buffer[index];
    buffer[index] = adc_value;
    sum += buffer[index];
    index = (index + 1) % 10;
    int filtered_value = sum / 10;

    // Conversion en densité de poussière
    float voltage = ( 3300/ 4096.0f) * filtered_value * 11.0f;
    if (voltage >= NO_DUST_VOLTAGE) {
        voltage -= NO_DUST_VOLTAGE;
        return voltage * COV_RATIO;
    } else {
        return 0.0f;
    }
}
