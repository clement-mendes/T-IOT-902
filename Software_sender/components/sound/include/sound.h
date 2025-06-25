#ifndef SOUND_H
#define SOUND_H

#include "driver/i2s_std.h"   // Pour l’API I2S nouvelle génération
#include "esp_err.h"

// Initialise le microphone
esp_err_t mic_init(void);

// Lit les données du microphone
esp_err_t mic_read(int32_t *buffer, size_t buffer_size, size_t *bytes_read);

// Arrête le microphone
void mic_stop(void);

void sound_print_dbfs(void);

#endif