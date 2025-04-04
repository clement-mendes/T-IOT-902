#include "temperature.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <stdint.h>

#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        21
#define I2C_MASTER_SCL_IO        13
// Adresse I²C du BME280 (vérifiez si c'est 0x76 ou 0x77 selon votre module)
#define BME280_ADDR              0x76

// Registres du BME280
#define REG_ID                   0xD0
#define REG_CTRL_MEAS            0xF4
#define REG_CONFIG               0xF5
// Début des données brutes de pression et température
#define REG_DATA_START           0xF7

static const char *TAG = "temperature";

// Fonctions d'écriture et de lecture sur I²C

static esp_err_t bme280_write_reg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = i2c_master_start(cmd);
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_WRITE, true);
    ret |= i2c_master_write_byte(cmd, reg, true);
    ret |= i2c_master_write_byte(cmd, value, true);
    ret |= i2c_master_stop(cmd);
    ret |= i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t bme280_read_reg(uint8_t reg, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = i2c_master_start(cmd);
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_WRITE, true);
    ret |= i2c_master_write_byte(cmd, reg, true);
    ret |= i2c_master_start(cmd);
    ret |= i2c_master_write_byte(cmd, (BME280_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        ret |= i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    ret |= i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    ret |= i2c_master_stop(cmd);
    ret |= i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void temperature_init(void)
{
    // Configuration de l'I²C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Erreur de configuration I2C");
    }
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de l'installation du driver I2C");
    }
    
    // Vérification de la présence du BME280
    uint8_t chip_id = 0;
    err = bme280_read_reg(REG_ID, &chip_id, 1);
    if (err == ESP_OK && chip_id == 0x60) {
        ESP_LOGI(TAG, "BME280 détecté, ID: 0x%02X", chip_id);
    } else {
        ESP_LOGE(TAG, "BME280 non détecté ! ID lu: 0x%02X", chip_id);
    }
    
    // Configuration minimale du capteur : oversampling x1 pour température et pression, mode normal
    uint8_t ctrl_meas = (1 << 5) | (1 << 2) | 3;  // [temp oversampling, pression oversampling, mode normal]
    err = bme280_write_reg(REG_CTRL_MEAS, ctrl_meas);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Écriture du registre CTRL_MEAS échouée");
    }
    
    // Vous pouvez configurer REG_CONFIG si nécessaire (filtre, temps d'attente, etc.)
}

// Pour la compensation, une variable globale est utilisée
static int32_t t_fine;

// Ces paramètres de compensation doivent être lus depuis le capteur.
// Ici nous utilisons des valeurs d'exemple pour simplifier.
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

float temperature_get(void)
{
    uint8_t data[6];
    // Lecture des 6 octets à partir du registre de départ (pression et température)
    esp_err_t err = bme280_read_reg(REG_DATA_START, data, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la lecture des données du capteur");
        return 0.0;
    }
    
    // Les octets [3,4,5] contiennent la température brute
    int32_t adc_T = ((int32_t)data[3] << 12) | ((int32_t)data[4] << 4) | ((int32_t)data[5] >> 4);
    
    // Compensation de la température (algorithme fourni dans la datasheet)
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    float temperature = (t_fine * 5 + 128) >> 8;
    
    return temperature / 100.0;
}

float pressure_get(void)
{
    uint8_t data[6];
    esp_err_t err = bme280_read_reg(REG_DATA_START, data, 6);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Échec de la lecture des données du capteur");
        return 0.0;
    }
    
    // Les octets [0,1,2] contiennent la pression brute
    int32_t adc_P = ((int32_t)data[0] << 12) | ((int32_t)data[1] << 4) | ((int32_t)data[2] >> 4);
    
    // Calcul de la pression en utilisant t_fine (assurez-vous d'avoir appelé temperature_get() au préalable)
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = (((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12));
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0; // évite une division par zéro
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    
    return ((float)p) / 25600.0; // Renvoie la pression en hPa
}
