// Inclusion guard to prevent multiple inclusions of this header file
#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/**
 * @brief Initializes the I²C interface and configures the BME280 sensor.
 * 
 * This function sets up the I²C communication and prepares the BME280
 * sensor for temperature and pressure measurements.
 */
void temperature_init(void);

/**
 * @brief Reads and compensates the temperature from the BME280 sensor.
 * 
 * @return float The temperature in degrees Celsius (°C).
 * 
 * This function retrieves the raw temperature data from the BME280 sensor,
 * applies compensation algorithms, and returns the temperature in °C.
 */
float temperature_get(void);

/**
 * @brief Reads and compensates the atmospheric pressure from the BME280 sensor.
 * 
 * @return float The pressure in hectopascals (hPa).
 * 
 * This function retrieves the raw pressure data from the BME280 sensor,
 * applies compensation algorithms, and returns the pressure in hPa.
 */
float pressure_get(void);

#endif // TEMPERATURE_H
