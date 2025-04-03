#ifndef DUST_SENSOR_H
#define DUST_SENSOR_H

// Fonction d'initialisation du capteur
void dust_sensor_init(void);

// Fonction pour lire la densité de poussière
float dust_sensor_read(void);

#endif // DUST_SENSOR_H
