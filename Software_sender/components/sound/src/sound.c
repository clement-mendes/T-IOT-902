#include "sound.h" // Include the header file for sound module declarations
#include "driver/i2s.h" // Include the I2S driver for ESP32
#include <math.h> // Include math library (not used in this code but may be needed elsewhere)

/**
 * @brief I2S device number to use.
 * 
 * Defines the I2S peripheral to be used. I2S_NUM_0 refers to the first I2S peripheral.
 */
#define I2S_NUM           I2S_NUM_0 

/**
 * @brief Audio sampling rate.
 * 
 * Defines the frequency at which audio samples are captured.
 */
#define I2S_SAMPLE_RATE   16000 

/**
 * @brief Bits per audio sample.
 * 
 * Specifies the resolution of each audio sample in bits.
 */
#define I2S_SAMPLE_BITS   16 

/**
 * @brief Number of DMA buffers.
 * 
 * Defines how many DMA buffers are used for temporary audio data storage.
 */
#define DMA_BUF_COUNT     8 

/**
 * @brief Length of each DMA buffer.
 * 
 * Specifies the size of each DMA buffer in bytes.
 */
#define DMA_BUF_LEN       1024 

/**
 * @brief I2S pin configuration.
 * 
 * Defines the GPIO pins used for I2S communication.
 */
#define I2S_BCK_IO        21   // GPIO pin for BCLK (Bit Clock)
#define I2S_WS_IO         13   // GPIO pin for LRCL (Word Select)
#define I2S_DATA_IN_IO    12   // GPIO pin for DOUT (Data Input)

/**
 * @brief Initializes the I2S sound module.
 * 
 * This function configures the I2S peripheral with the specified settings,
 * including sample rate, bit depth, and DMA buffer configuration. It also
 * sets up the GPIO pins for I2S communication.
 */
void sound_init(void)
{
    // Define the I2S configuration structure
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX, // Set I2S to master mode and enable reception
        .sample_rate = I2S_SAMPLE_RATE, // Set the sample rate
        .bits_per_sample = I2S_SAMPLE_BITS, // Set the bits per sample
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Use only the left channel (mono audio)
        .communication_format = I2S_COMM_FORMAT_I2S, // Set the communication format to I2S
        .intr_alloc_flags = 0, // No specific interrupt allocation flags
        .dma_buf_count = DMA_BUF_COUNT, // Set the number of DMA buffers
        .dma_buf_len = DMA_BUF_LEN, // Set the length of each DMA buffer
        .use_apll = false, // Do not use the APLL (Audio Phase-Locked Loop)
    };

    // Define the I2S pin configuration structure
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO, // Set the GPIO pin for BCLK
        .ws_io_num = I2S_WS_IO, // Set the GPIO pin for LRCL
        .data_out_num = I2S_PIN_NO_CHANGE, // No GPIO pin for data output
        .data_in_num = I2S_DATA_IN_IO // Set the GPIO pin for data input
    };

    // Install and initialize the I2S driver
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL); // Install the I2S driver with the specified configuration
    i2s_set_pin(I2S_NUM, &pin_config); // Configure the GPIO pins for I2S
}

/**
 * @brief Reads audio data from the I2S interface.
 * 
 * @param buffer Pointer to the buffer where audio data will be stored.
 * @param length Number of audio samples to read.
 * @return int Number of samples successfully read, or an error code.
 * 
 * This function reads audio data from the I2S interface into the provided buffer.
 * The function blocks until the specified number of samples is read.
 */
int sound_read(int16_t *buffer, size_t length)
{
    size_t bytes_read = 0; // Variable to store the number of bytes read
    // Perform a blocking read from the I2S interface
    i2s_read(I2S_NUM, (void *)buffer, length * sizeof(int16_t), &bytes_read, portMAX_DELAY); 
    // Return the number of samples read (convert bytes to samples)
    return bytes_read / sizeof(int16_t); 
}
