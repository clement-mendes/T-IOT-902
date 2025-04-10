#include "temperature.h" // Include the header file for temperature module declarations
#include "driver/i2c.h" // Include the I²C driver for ESP32
#include "esp_log.h" // Include ESP-IDF logging library
#include <stdint.h> // Include standard integer types

// I²C configuration
#define I2C_MASTER_NUM           I2C_NUM_0 // I²C bus number
#define I2C_MASTER_FREQ_HZ       50000 // I²C clock frequency in Hz
#define I2C_MASTER_SDA_IO        21 // GPIO pin for I²C SDA
#define I2C_MASTER_SCL_IO        22 // GPIO pin for I²C SCL

// BME280 I²C address
#define BME280_ADDR              0x76 // I²C address of the BME280 sensor

// BME280 registers
#define REG_ID                   0xD0 // Register containing the sensor ID
#define REG_CTRL_MEAS            0xF4 // Measurement control register
#define REG_CONFIG               0xF5 // General configuration register
#define REG_DATA_START           0xF7 // Start of raw pressure and temperature data

static const char *TAG = "temperature"; // Tag for logging

/**
 * @brief Writes a value to a specific register of the BME280 sensor.
 *
 * This function communicates with the BME280 sensor to write a single byte
 * to a specified register. It is typically used to configure the sensor
 * or update its settings.
 *
 * @param reg The register address to write to.
 * @param value The value to write to the specified register.
 * @return 
 *     - ESP_OK: Success.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments.
 *     - ESP_FAIL: Communication with the sensor failed.
 */
static esp_err_t bme280_write_reg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // Create an I²C command link
    esp_err_t ret = i2c_master_start(cmd); // Start I²C transmission
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_WRITE, true); // Send sensor address with write bit
    ret |= i2c_master_write_byte(cmd, reg, true); // Send register address
    ret |= i2c_master_write_byte(cmd, value, true); // Send value to write
    ret |= i2c_master_stop(cmd); // Stop I²C transmission
    ret |= i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000)); // Execute the I²C command
    i2c_cmd_link_delete(cmd); // Delete the I²C command link
    return ret; // Return the result of the operation
}

/**
 * @brief Reads data from a specified register of the BME280 sensor.
 *
 * This function communicates with the BME280 sensor to read a specified number
 * of bytes from a given register address. The data read from the sensor is
 * stored in the provided buffer.
 *
 * @param[in] reg The register address to read from.
 * @param[out] data Pointer to the buffer where the read data will be stored.
 * @param[in] len The number of bytes to read from the register.
 *
 * @return
 *     - ESP_OK: Success.
 *     - ESP_ERR_INVALID_ARG: Invalid arguments.
 *     - ESP_FAIL: Communication with the sensor failed.
 */
static esp_err_t bme280_read_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // Create an I²C command link
    esp_err_t ret = i2c_master_start(cmd); // Start I²C transmission
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_WRITE, true); // Send sensor address with write bit
    ret |= i2c_master_write_byte(cmd, reg, true); // Send register address
    ret |= i2c_master_start(cmd); // Restart I²C transmission
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_READ, true); // Send sensor address with read bit
    if (len > 1) {
        ret |= i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK); // Read multiple bytes with acknowledgment
    }
    ret |= i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK); // Read the last byte with no acknowledgment
    ret |= i2c_master_stop(cmd); // Stop I²C transmission
    ret |= i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000)); // Execute the I²C command
    i2c_cmd_link_delete(cmd); // Delete the I²C command link
    return ret; // Return the result of the operation
}

/**
 * @brief Initializes the temperature sensor module.
 *
 * This function sets up the necessary configurations and resources
 * required for the temperature sensor to operate. It should be called
 * once during the initialization phase of the application.
 */
void temperature_init(void)
{
    // Configure the I²C interface
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER, // Set I²C to master mode
        .sda_io_num = I2C_MASTER_SDA_IO, // Set SDA pin
        .scl_io_num = I2C_MASTER_SCL_IO, // Set SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up for SDA
        .scl_pullup_en = GPIO_PULLUP_ENABLE, // Enable pull-up for SCL
        .master.clk_speed = I2C_MASTER_FREQ_HZ, // Set I²C clock speed
    };
    // Install the I²C driver
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf); // Configure I²C parameters
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur de configuration I2C");
    }
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0); // Install the I²C driver
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de l'installation du driver I2C");
    }
    
    // Check for the presence of the BME280 sensor
    uint8_t chip_id = 0;
    err = bme280_read_reg(REG_ID, &chip_id, 1); // Read the sensor ID
    if (err == ESP_OK && chip_id == 0x60) {
        ESP_LOGI(TAG, "BME280 détecté, ID: 0x%02X", chip_id);
    } else {
        ESP_LOGE(TAG, "BME280 non détecté ! ID lu: 0x%02X", chip_id);
    }
    
    // Minimal sensor configuration: oversampling x1 for temperature and pressure, normal mode
    uint8_t ctrl_meas = (1 << 5) | (1 << 2) | 3; // Configure oversampling and mode
    err = bme280_write_reg(REG_CTRL_MEAS, ctrl_meas); // Write to the control register
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Écriture du registre CTRL_MEAS échouée");
    }
}

// For compensation, a global variable is used
static int32_t t_fine;

// These compensation parameters must be read from the sensor.
// Here we use example values for simplicity.
// Compensation parameters provided by the manufacturer
// dig_T1		Linear base compensation
// dig_T2		Quadratic correction
// dig_T3		Cubic correction
static int16_t dig_T1 = 27504;
static int16_t dig_T2 = 26435;
static int16_t dig_T3 = -1000;
static int32_t dig_P1 = 36477;
static int16_t dig_P2 = -10685;
static int16_t dig_P3 = 3024;
static int16_t dig_P4 = 2855;
static int16_t dig_P5 = 140;
static int16_t dig_P6 = -7;
static int16_t dig_P7 = 15500;
static int16_t dig_P8 = -14600;
static int16_t dig_P9 = 6000;

/**
 * @brief Retrieves the current temperature reading.
 * 
 * This function is used to obtain the current temperature value
 * from the temperature sensor. The returned value is a floating-point
 * number representing the temperature in degrees Celsius.
 * 
 * @return float The current temperature in degrees Celsius.
 */
float temperature_get(void)
{
    uint8_t data[6];
    // Read 6 bytes starting from the start register (pressure and temperature)
    esp_err_t err = bme280_read_reg(REG_DATA_START, data, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la lecture des données du capteur");
        return 0.0;
    }
    
    // Bytes [3,4,5] contain the raw temperature
    int32_t adc_T = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);
    
    // Temperature compensation formula (algorithm provided in the datasheet) to convert raw_temp to temperature in °C
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    float temperature = (t_fine * 5 + 128) >> 8;
    
    return temperature / 100.0;
}

/**
 * @brief Retrieves the current pressure value.
 * 
 * This function is responsible for obtaining the current pressure
 * measurement from the appropriate sensor or data source.
 * 
 * @return float The current pressure value.
 */
float pressure_get(void)
{
    uint8_t data[6];
    // Read 6 bytes
    esp_err_t err = bme280_read_reg(REG_DATA_START, data, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la lecture des données du capteur");
        return 0.0;
    }
    
    // Bytes [0,1,2] contain the raw pressure
    int32_t adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);
    
    // Calculate pressure using t_fine
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = (((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12));
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0; 
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return ((float)p) / 25600.0; // Return pressure in hPa
}
