#ifndef __LORA_H__
#define __LORA_H__

/**
 * @brief Initializes the LoRa module.
 * 
 * @return int Returns 0 on success, or an error code on failure.
 */
int lora_init(void);

/**
 * @brief Sets the frequency for LoRa communication.
 * 
 * @param frequency Frequency in Hz (e.g., 868000000 for 868 MHz).
 */
void lora_set_frequency(long frequency);

/**
 * @brief Sets the spreading factor for LoRa communication.
 * 
 * @param sf Spreading factor (valid values: 6 to 12).
 */
void lora_set_spreading_factor(int sf);

/**
 * @brief Sets the bandwidth for LoRa communication.
 * 
 * @param sbw Bandwidth in kHz (e.g., 125 for 125 kHz).
 */
void lora_set_bandwidth(int sbw);

/**
 * @brief Sets the coding rate for LoRa communication.
 * 
 * @param cr Coding rate (valid values: 5 to 8, representing 4/5 to 4/8).
 */
void lora_set_coding_rate(int cr);

/**
 * @brief Puts the LoRa module into receive mode.
 */
void lora_receive(void);

/**
 * @brief Receives a packet from the LoRa module.
 * 
 * @param buf Pointer to the buffer where the received data will be stored.
 * @param size Size of the buffer in bytes.
 * @return int Number of bytes received, or -1 on failure.
 */
int lora_receive_packet(uint8_t *buf, int size);

/**
 * @brief Checks if a packet has been received.
 * 
 * @return int Returns 1 if a packet is received, 0 otherwise.
 */
int lora_received(void);

/**
 * @brief Gets the RSSI (Received Signal Strength Indicator) of the last received packet.
 * 
 * @return int RSSI value in dBm.
 */
int lora_packet_rssi(void);

/**
 * @brief Gets the SNR (Signal-to-Noise Ratio) of the last received packet.
 * 
 * @return float SNR value in dB.
 */
float lora_packet_snr(void);

/**
 * @brief Resets the LoRa module.
 */
void lora_reset(void);

/**
 * @brief Enables CRC (Cyclic Redundancy Check) for packet transmission and reception.
 */
void lora_enable_crc(void);

/**
 * @brief Disables CRC (Cyclic Redundancy Check) for packet transmission and reception.
 */
void lora_disable_crc(void);

#endif
