#include "sound.h"
#include "driver/i2s.h"
#include <math.h>

#define I2S_NUM           I2S_NUM_0 //Définit le numéro du périphérique I2S à utiliser, ici I2S_NUM_0, qui est le premier périphérique I2S de l'ESP32
#define I2S_SAMPLE_RATE   16000 //fréquence d'échantillonnage de l'audio
#define I2S_SAMPLE_BITS   16 //nombre de bits par échantillon
#define DMA_BUF_COUNT     8 //nombre de tampons DMA (Direct Memory Access) utilisés pour stocker temporairement les données audio 
#define DMA_BUF_LEN       1024 //longueur de chaque tampon DMA

// Exemple de configuration des broches (à adapter selon votre câblage)
#define I2S_BCK_IO        21   // Broche BCLK (horloge)
#define I2S_WS_IO         13   // Broche LRCL (Word Select)
#define I2S_DATA_IN_IO    12   // Broche DOUT (Données entrantes de l'audio)

void sound_init(void)
{
    // Configuration de base de l'I2S
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX, // mode master et reception
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_SAMPLE_BITS,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // données audio sont en mono
        .communication_format = I2S_COMM_FORMAT_I2S,//format de communication I2S
        .intr_alloc_flags = 0, // optionnel, dépend de votre configuration
        .dma_buf_count = DMA_BUF_COUNT,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false, //Utilisation de l'APLL
    };

    // Configuration des broches de l'I2S
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_PIN_NO_CHANGE, //Broche de sortie des données 
        .data_in_num = I2S_DATA_IN_IO //Broche de entré des données 
    };

    // Installation et configuration du driver I2S
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL); //Installe et initialise le driver I2S
    i2s_set_pin(I2S_NUM, &pin_config); //Configure les broches pour l'I2S 
}

int sound_read(int16_t *buffer, size_t length)
{
    size_t bytes_read = 0;
    // Lecture bloquante : lire "length" échantillons (chaque échantillon fait 16 bits)
    i2s_read(I2S_NUM, (void *)buffer, length * sizeof(int16_t), &bytes_read, portMAX_DELAY); //Lit des données depuis l'I2S dans le tampon buffer
    return bytes_read / sizeof(int16_t); //Retourne le nombre d'échantillons (en 16 bits) qui ont été lus avec succès 2 octet
}
