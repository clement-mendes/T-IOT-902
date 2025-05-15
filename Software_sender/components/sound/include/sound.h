#ifndef SOUND_H
#define SOUND_H

#include <stdint.h> // Include standard integer types
#include <stdlib.h> // Include standard library for size_t

/**
 * @brief Initializes the sound module.
 * 
 * This function sets up the necessary hardware or software resources
 * required for sound processing.
 */
void sound_init(void);

/**
 * @brief Reads sound data into the provided buffer.
 * 
 * @param buffer Pointer to the buffer where sound data will be stored.
 * @param length Number of samples to read.
 * @return int Number of samples successfully read, or an error code.
 * 
 * This function captures sound data and stores it in the provided buffer.
 * The length parameter specifies the number of samples to read.
 */
int sound_read(int16_t *buffer, size_t length);

/**
 * @brief Measures the current sound level in decibels.
 * 
 * @return float The measured sound level in dB.
 */
float sound_measure_db(void);

/**
 * @brief Calculates the average sound level over multiple measurements.
 * 
 * @return float The average sound level in dB.
 */
float sound_get_average_db(void);

#endif
