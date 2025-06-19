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
void init_microphone(void);

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
int read_microphone_db(int16_t *buffer, size_t length);

#endif