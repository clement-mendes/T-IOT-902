#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/**
 * @brief Initialise l’I²C et configure le capteur BME280.
 */
void temperature_init(void);

/**
 * @brief Lit et compense la température à partir du BME280.
 *
 * @return La température en °C.
 */
float temperature_get(void);

/**
 * @brief Lit et compense la pression atmosphérique à partir du BME280.
 *
 * @return La pression en hPa.
 */
float pressure_get(void);

#endif // TEMPERATURE_H
