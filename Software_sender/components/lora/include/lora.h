#ifndef __LORA_H__
#define __LORA_H__

int lora_init(void); 
void lora_set_frequency(long frequency); 
void lora_set_spreading_factor(int sf);
void lora_set_bandwidth(int sbw);
void lora_set_coding_rate(int cr); 
void lora_set_tx_power(int level); 
void lora_send_packet(uint8_t *buf, int size);
int lora_packet_lost(void);
void lora_close(void);
void lora_enable_crc(void);
void lora_disable_crc(void);

#endif
