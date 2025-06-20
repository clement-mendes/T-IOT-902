#ifndef DUST_H
#define DUST_H

#include <stdint.h>

/**
 * @brief Initialize the GP2Y1010AU0F dust sensor (GPIO and ADC).
 */
void dust_init(void);

/**
 * @brief Read the dust sensor value (raw ADC).
 * @return Raw ADC value (int).
 */
int dust_read_raw(void);

/**
 * @brief Convert the raw ADC value to approximate dust concentration (ug/m3).
 * @param raw ADC value.
 * @return Dust concentration in ug/m3 (float).
 */
float dust_raw_to_ugm3(int raw);

#endif