#include "driver/gpio.h"
#include "esp_log.h"

#define SOUND_SENSOR_GPIO GPIO_NUM_4

void sound_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SOUND_SENSOR_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

int sound_read(void) {
    return gpio_get_level(SOUND_SENSOR_GPIO);
}