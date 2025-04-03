#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include <stdlib.h>

// Initialise l'I2S pour récupérer le son
void sound_init(void);

// Lit un nombre de données dans un buffer et renvoie le nombre d'échantillons lus
int sound_read(int16_t *buffer, size_t length);

#endif
