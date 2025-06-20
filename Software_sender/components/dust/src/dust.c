#include "dust.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include <unistd.h> 

#define DUST_LED_GPIO GPIO_NUM_16   // GPIO connecté à la LED du capteur
#define DUST_ADC_CHANNEL ADC1_CHANNEL_6 // GPIO34 (ADC1_CH6) pour la sortie analogique du capteur

void dust_init(void) {
    // Configure LED pin as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DUST_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(DUST_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

int dust_read_raw(void) {
    // Allume la LED, attend, lit l'ADC, éteint la LED
    gpio_set_level(DUST_LED_GPIO, 0); // LED ON (actif bas)
    ets_delay_us(280); // 280us
    int raw = adc1_get_raw(DUST_ADC_CHANNEL);
    ets_delay_us(40);  // 40us (lecture)
    gpio_set_level(DUST_LED_GPIO, 1); // LED OFF
    ets_delay_us(9680); // 9680us (repos)
    return raw;
}

float dust_raw_to_ugm3(int raw) {
    // Conversion empirique (à ajuster selon calibration)
    float voltage = raw * (3.3 / 4095.0); // 12 bits ADC, 3.3V ref
    float dust = (voltage - 0.6) * 1000.0 / 0.5; // Formule datasheet 
    if (dust < 0) dust = 0;
    return dust;
}