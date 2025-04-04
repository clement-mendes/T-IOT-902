#ifndef DUST_SENSOR_H
#define DUST_SENSOR_H

/**
 * @brief Initializes the dust sensor module.
 * 
 * This function sets up the necessary configurations and hardware
 * required for the dust sensor to operate. It should be called
 * before using any other functions related to the dust sensor.
 */
void dust_sensor_init(void);

/**
 * @brief Reads the current dust concentration from the dust sensor.
 * 
 * This function interfaces with the dust sensor hardware to retrieve
 * the current dust concentration level. The value returned is typically
 * in micrograms per cubic meter (µg/m³) or another unit depending on the
 * sensor's specifications.
 * 
 * @return float The measured dust concentration.
 */
float dust_sensor_read(void);

#endif 
