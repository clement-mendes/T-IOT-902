#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/*
 * Register definitions
 */
#define REG_FIFO                       0x00
#define REG_OP_MODE                    0x01
#define REG_FRF_MSB                    0x06
#define REG_FRF_MID                    0x07
#define REG_FRF_LSB                    0x08
#define REG_PA_CONFIG                  0x09
#define REG_LNA                        0x0c
#define REG_FIFO_ADDR_PTR              0x0d
#define REG_FIFO_TX_BASE_ADDR          0x0e
#define REG_FIFO_RX_BASE_ADDR          0x0f
#define REG_FIFO_RX_CURRENT_ADDR       0x10
#define REG_IRQ_FLAGS                  0x12
#define REG_RX_NB_BYTES                0x13
#define REG_PKT_SNR_VALUE              0x19
#define REG_PKT_RSSI_VALUE             0x1a
#define REG_MODEM_CONFIG_1             0x1d
#define REG_MODEM_CONFIG_2             0x1e
#define REG_PREAMBLE_MSB               0x20
#define REG_PREAMBLE_LSB               0x21
#define REG_PAYLOAD_LENGTH             0x22
#define REG_MODEM_CONFIG_3             0x26
#define REG_RSSI_WIDEBAND              0x2c
#define REG_DETECTION_OPTIMIZE         0x31
#define REG_DETECTION_THRESHOLD        0x37
#define REG_SYNC_WORD                  0x39
#define REG_DIO_MAPPING_1              0x40
#define REG_DIO_MAPPING_2              0x41
#define REG_VERSION                    0x42

/*
 * Transceiver modes
 */
#define MODE_LONG_RANGE_MODE           0x80
#define MODE_SLEEP                     0x00
#define MODE_STDBY                     0x01
#define MODE_TX                        0x03
#define MODE_RX_CONTINUOUS             0x05
#define MODE_RX_SINGLE                 0x06

/*
 * PA configuration
 */
#define PA_BOOST                       0x80

/*
 * IRQ masks
 */
#define IRQ_TX_DONE_MASK               0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK     0x20
#define IRQ_RX_DONE_MASK               0x40

#define PA_OUTPUT_RFO_PIN              0
#define PA_OUTPUT_PA_BOOST_PIN         1

#define TIMEOUT_RESET                  100

// SPI Stuff
#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST
#endif


#define TAG "LORA"

static spi_device_handle_t _spi;
static int _implicit;
static long _frequency;
static int _send_packet_lost = 0;
static int _cr = 0;
static int _sbw = 0;
static int _sf = 0;

// use spi_device_transmit
#define SPI_TRANSMIT 1

// use buffer io
// A little faster
#define BUFFER_IO 1


/**
 * Write a value to a register.  * @param reg Register index.
 * @param val Value to write.
 */
void 
lora_write_reg(int reg, int val)
{
   uint8_t out[2] = { 0x80 | reg, val };
   uint8_t in[2];

   spi_transaction_t t = {
      .flags = 0,
      .length = 8 * sizeof(out),
      .tx_buffer = out,
      .rx_buffer = in  
   };

   //gpio_set_level(CONFIG_CS_GPIO, 0);
#if SPI_TRANSMIT
   spi_device_transmit(_spi, &t);
#else
   spi_device_polling_transmit(_spi, &t);
#endif
   //gpio_set_level(CONFIG_CS_GPIO, 1);
}

/**
 * Write a buffer to a register.
 * @param reg Register index.
 * @param val Value to write.
 * @param len Byte length to write.
 */
void
lora_write_reg_buffer(int reg, uint8_t *val, int len)
{
   uint8_t *out;
   out = (uint8_t *)malloc(len+1);
   out[0] = 0x80 | reg;
   for (int i=0;i<len;i++) {
      out[i+1] = val[i];
   }

   spi_transaction_t t = {
      .flags = 0,
      .length = 8 * (len+1),
      .tx_buffer = out,
      .rx_buffer = NULL
   };

   //gpio_set_level(CONFIG_CS_GPIO, 0);
#if SPI_TRANSMIT
   spi_device_transmit(_spi, &t);
#else
   spi_device_polling_transmit(_spi, &t);
#endif
   //gpio_set_level(CONFIG_CS_GPIO, 1);
   free(out);
}

/**
 * Read the current value of a register.
 * @param reg Register index.
 * @return Value of the register.
 */
int
lora_read_reg(int reg)
{
   uint8_t out[2] = { reg, 0xff };
   uint8_t in[2];

   spi_transaction_t t = {
      .flags = 0,
      .length = 8 * sizeof(out),
      .tx_buffer = out,
      .rx_buffer = in
   };

   //gpio_set_level(CONFIG_CS_GPIO, 0);
#if SPI_TRANSMIT
   spi_device_transmit(_spi, &t);
#else
   spi_device_polling_transmit(_spi, &t);
#endif
   //gpio_set_level(CONFIG_CS_GPIO, 1);
   return in[1];
}

/**
 * Read the current value of a register.
 * @param reg Register index.
 * @return Value of the register.
 * @param len Byte length to read.
 */
void
lora_read_reg_buffer(int reg, uint8_t *val, int len)
{
   uint8_t *out;
   uint8_t *in;
   out = (uint8_t *)malloc(len+1);
   in = (uint8_t *)malloc(len+1);
   out[0] = reg;
   for (int i=0;i<len;i++) {
      out[i+1] = 0xff;
   }

   spi_transaction_t t = {
      .flags = 0,
      .length = 8 * (len+1),
      .tx_buffer = out,
      .rx_buffer = in
   };

   //gpio_set_level(CONFIG_CS_GPIO, 0);
#if SPI_TRANSMIT
   spi_device_transmit(_spi, &t);
#else
   spi_device_polling_transmit(_spi, &t);
#endif
   //gpio_set_level(CONFIG_CS_GPIO, 1);
   for (int i=0;i<len;i++) {
      val[i] = in[i+1];
   }
   free(out);
   free(in);
}

/**
 * Perform physical reset on the Lora chip
 */
void 
lora_reset(void)
{
   gpio_set_level(CONFIG_RST_GPIO, 0);
   vTaskDelay(pdMS_TO_TICKS(1));
   gpio_set_level(CONFIG_RST_GPIO, 1);
   vTaskDelay(pdMS_TO_TICKS(10));
}

/**
 * Sets the radio transceiver in idle mode.
 * Must be used to change registers and access the FIFO.
 */
void 
lora_idle(void)
{
   lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
}

/**
 * Sets the radio transceiver in sleep mode.
 * Low power consumption and FIFO is lost.
 */
void 
lora_sleep(void)
{ 
   lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
}

/**
 * Configure power level for transmission
 * @param level 2-17, from least to most power
 */
void 
lora_set_tx_power(int level)
{
   // RF9x module uses PA_BOOST pin
   if (level < 2) level = 2;
   else if (level > 17) level = 17;
   lora_write_reg(REG_PA_CONFIG, PA_BOOST | (level - 2));
}

/**
 * Set carrier frequency.
 * @param frequency Frequency in Hz
 */
void 
lora_set_frequency(long frequency)
{
   _frequency = frequency;

   uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

   lora_write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
   lora_write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
   lora_write_reg(REG_FRF_LSB, (uint8_t)(frf >> 0));
}

/**
 * Set spreading factor.
 * @param sf 6-12, Spreading factor to use.
 */
void 
lora_set_spreading_factor(int sf)
{
   if (sf < 6) sf = 6;
   else if (sf > 12) sf = 12;

   if (sf == 6) {
      lora_write_reg(REG_DETECTION_OPTIMIZE, 0xc5);
      lora_write_reg(REG_DETECTION_THRESHOLD, 0x0c);
   } else {
      lora_write_reg(REG_DETECTION_OPTIMIZE, 0xc3);
      lora_write_reg(REG_DETECTION_THRESHOLD, 0x0a);
   }

   lora_write_reg(REG_MODEM_CONFIG_2, (lora_read_reg(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));
   _sf = sf;
}

/**
 * Get spreading factor.
 */
int 
lora_get_spreading_factor(void)
{
   return (lora_read_reg(REG_MODEM_CONFIG_2) >> 4);
}

/**
 * Set bandwidth (bit rate)
 * @param sbw Signal bandwidth(0 to 9)
 */
void 
lora_set_bandwidth(int sbw)
{
   if (sbw < 10) {
      lora_write_reg(REG_MODEM_CONFIG_1, (lora_read_reg(REG_MODEM_CONFIG_1) & 0x0f) | (sbw << 4));
      _sbw = sbw;
   }
}

/**
 * Get bandwidth (bit rate)
 * @param sbw Signal bandwidth(0 to 9)
 */
int 
lora_get_bandwidth(void)
{
   //int bw;
   //bw = lora_read_reg(REG_MODEM_CONFIG_1) & 0xf0;
   //ESP_LOGD(TAG, "bw=0x%02x", bw);
   //bw = bw >> 4;
   //return bw;
   return ((lora_read_reg(REG_MODEM_CONFIG_1) & 0xf0) >> 4);
}

/**
 * Set coding rate 
 * @param cr Coding Rate(1 to 4)
 */ 
void 
lora_set_coding_rate(int cr)
{
   //if (denominator < 5) denominator = 5;
   //else if (denominator > 8) denominator = 8;

   //int cr = denominator - 4;
   if (cr < 1) cr = 1;
   else if (cr > 4) cr = 4;
   lora_write_reg(REG_MODEM_CONFIG_1, (lora_read_reg(REG_MODEM_CONFIG_1) & 0xf1) | (cr << 1));
   _cr = cr;
}

/**
 * Get coding rate 
 */ 
int 
lora_get_coding_rate(void)
{
   return ((lora_read_reg(REG_MODEM_CONFIG_1) & 0x0E) >> 1);
}

/**
 * Enable appending/verifying packet CRC.
 */
void 
lora_enable_crc(void)
{
   lora_write_reg(REG_MODEM_CONFIG_2, lora_read_reg(REG_MODEM_CONFIG_2) | 0x04);
}

/**
 * Disable appending/verifying packet CRC.
 */
void 
lora_disable_crc(void)
{
   lora_write_reg(REG_MODEM_CONFIG_2, lora_read_reg(REG_MODEM_CONFIG_2) & 0xfb);
}

/**
 * Perform hardware initialization.
 */
int 
lora_init(void)
{
   esp_err_t ret;

   /*
    * Configure CPU hardware to communicate with the radio chip
    */
   gpio_reset_pin(CONFIG_RST_GPIO);
   gpio_set_direction(CONFIG_RST_GPIO, GPIO_MODE_OUTPUT);
   gpio_reset_pin(CONFIG_CS_GPIO);
   gpio_set_direction(CONFIG_CS_GPIO, GPIO_MODE_OUTPUT);
   gpio_set_level(CONFIG_CS_GPIO, 1);

   spi_bus_config_t bus = {
      .miso_io_num = CONFIG_MISO_GPIO,
      .mosi_io_num = CONFIG_MOSI_GPIO,
      .sclk_io_num = CONFIG_SCK_GPIO,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0
   };
           
   //ret = spi_bus_initialize(VSPI_HOST, &bus, 0);
   ret = spi_bus_initialize(HOST_ID, &bus, SPI_DMA_CH_AUTO);
   assert(ret == ESP_OK);

   spi_device_interface_config_t dev = {
      .clock_speed_hz = 9000000,
      .mode = 0,
      .spics_io_num = CONFIG_CS_GPIO,
      .queue_size = 7,
      .flags = 0,
      .pre_cb = NULL
   };
   //ret = spi_bus_add_device(VSPI_HOST, &dev, &_spi);
   ret = spi_bus_add_device(HOST_ID, &dev, &_spi);
   assert(ret == ESP_OK);

   /*
    * Perform hardware reset.
    */
   lora_reset();

   /*
    * Check version.
    */
   uint8_t version;
   uint8_t i = 0;
   while(i++ < TIMEOUT_RESET) {
      version = lora_read_reg(REG_VERSION);
      ESP_LOGD(TAG, "version=0x%02x", version);
      if(version == 0x12) break;
      vTaskDelay(2);
   }
   ESP_LOGD(TAG, "i=%d, TIMEOUT_RESET=%d", i, TIMEOUT_RESET);
   if (i == TIMEOUT_RESET + 1) return 0; // Illegal version
   //assert(i < TIMEOUT_RESET + 1); // at the end of the loop above, the max value i can reach is TIMEOUT_RESET + 1

   /*
    * Default configuration.
    */
   lora_sleep();
   lora_write_reg(REG_FIFO_RX_BASE_ADDR, 0);
   lora_write_reg(REG_FIFO_TX_BASE_ADDR, 0);
   lora_write_reg(REG_LNA, lora_read_reg(REG_LNA) | 0x03);
   lora_write_reg(REG_MODEM_CONFIG_3, 0x04);
   lora_set_tx_power(17);

   lora_idle();
   return 1;
}

/**
 * Return lost send packet count.
 */
int 
lora_packet_lost(void)
{
   return (_send_packet_lost);
}

/**
 * Send a packet.
 * @param buf Data to be sent
 * @param size Size of data.
 */
void 
lora_send_packet(uint8_t *buf, int size)
{
   /*
    * Transfer data to radio.
    */
   lora_idle();
   lora_write_reg(REG_FIFO_ADDR_PTR, 0);

#if BUFFER_IO
   lora_write_reg_buffer(REG_FIFO, buf, size);
#else
   for(int i=0; i<size; i++) 
      lora_write_reg(REG_FIFO, *buf++);
#endif
   
   lora_write_reg(REG_PAYLOAD_LENGTH, size);
   
   /*
    * Start transmission and wait for conclusion.
    */
   lora_write_reg(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);
#if 0
   while((lora_read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0)
      vTaskDelay(2);
#endif
   int loop = 0;
   int max_retry;
   if (_sbw < 2) {
      max_retry = 500;
   } else if (_sbw < 4) {
      max_retry = 250;
   } else if (_sbw < 6) {
      max_retry = 125;
   } else if (_sbw < 8) {
      max_retry = 60;
   } else {
      max_retry = 30;
   }
   ESP_LOGD(TAG, "_sbw=%d max_retry=%d", _sbw, max_retry);
   while(1) {
      int irq = lora_read_reg(REG_IRQ_FLAGS);
      ESP_LOGD(TAG, "lora_read_reg=0x%x", irq);
      if ((irq & IRQ_TX_DONE_MASK) == IRQ_TX_DONE_MASK) break;
      loop++;
      if (loop == max_retry) break;
      vTaskDelay(2);
   }
   if (loop == max_retry) {
      _send_packet_lost++;
      ESP_LOGE(TAG, "lora_send_packet Fail");
   }
   lora_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
}

/**
 * Shutdown hardware.
 */
void 
lora_close(void)
{
   lora_sleep();
//   close(__spi);  FIXME: end hardware features after lora_close
//   close(__cs);
//   close(__rst);
//   _spi = -1;
//   __cs = -1;
//   __rst = -1;
}