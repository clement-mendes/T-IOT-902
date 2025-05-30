#ifndef __LORA_H__
#define __LORA_H__

/**
 * @file lora.h
 * @brief LoRa driver API for ESP32 sender.
 */

/**
 * @brief Initialize the LoRa module.
 * @return 1 if successful, 0 if failed.
 */
int lora_init(void);

/**
 * @brief Set the carrier frequency.
 * @param frequency Frequency in Hz.
 */
void lora_set_frequency(long frequency);

/**
 * @brief Set the spreading factor.
 * @param sf Spreading factor (6-12).
 */
void lora_set_spreading_factor(int sf);

/**
 * @brief Set the bandwidth (bit rate).
 * @param sbw Signal bandwidth (0 to 9).
 */
void lora_set_bandwidth(int sbw);

/**
 * @brief Set the coding rate.
 * @param cr Coding rate (1 to 4).
 */
void lora_set_coding_rate(int cr);

/**
 * @brief Set the transmission power.
 * @param level Power level (2-17).
 */
void lora_set_tx_power(int level);

/**
 * @brief Send a packet.
 * @param buf Data buffer to send.
 * @param size Size of data.
 */
void lora_send_packet(uint8_t *buf, int size);

/**
 * @brief Get the number of lost packets.
 * @return Number of lost packets.
 */
int lora_packet_lost(void);

/**
 * @brief Shutdown the LoRa hardware.
 */
void lora_close(void);

/**
 * @brief Enable CRC for packets.
 */
void lora_enable_crc(void);

/**
 * @brief Disable CRC for packets.
 */
void lora_disable_crc(void);

#endif
