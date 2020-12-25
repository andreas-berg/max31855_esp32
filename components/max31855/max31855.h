/*
 * max31855.h
 *
 */

#ifndef _COMPONENTS_MAX31855_H_
#define _COMPONENTS_MAX31855_H_
#include "driver/spi_master.h"

#ifdef CONFIG_MAX31855_SPI_MISO 
#define PIN_NUM_MISO CONFIG_MAX31855_SPI_MISO
#else
#define PIN_NUM_MISO 12
#endif

#ifdef CONFIG_MAX31855_SPI_CLK 
#define PIN_NUM_CLK CONFIG_MAX31855_SPI_CLK
#else
#define PIN_NUM_CLK  14
#endif

#ifdef CONFIG_MAX31855_SPI_CS 
#define PIN_NUM_CS CONFIG_MAX31855_SPI_CS
#else
#define PIN_NUM_CS   15
#endif

#if defined (CONFIG_MAX31855_SPI_VSPI)
#define SPI_BUS VSPI_HOST
#else
#define SPI_BUS HSPI_HOST
#endif

#define DEBUG_4BYTE_TO_BINARY_PATTERN "(0x%08x) %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"
#define DEBUG_4BYTE_TO_BINARY(byte) \
  (byte),                    \
  (byte & 0x80000000 ? '1' : '0'), \
  (byte & 0x40000000 ? '1' : '0'), \
  (byte & 0x20000000 ? '1' : '0'), \
  (byte & 0x10000000 ? '1' : '0'), \
  (byte & 0x08000000 ? '1' : '0'), \
  (byte & 0x04000000 ? '1' : '0'), \
  (byte & 0x02000000 ? '1' : '0'), \
  (byte & 0x01000000 ? '1' : '0'), \
  (byte & 0x00800000 ? '1' : '0'), \
  (byte & 0x00400000 ? '1' : '0'), \
  (byte & 0x00200000 ? '1' : '0'), \
  (byte & 0x00100000 ? '1' : '0'), \
  (byte & 0x00080000 ? '1' : '0'), \
  (byte & 0x00040000 ? '1' : '0'), \
  (byte & 0x00020000 ? '1' : '0'), \
  (byte & 0x00010000 ? '1' : '0'), \
  (byte & 0x00008000 ? '1' : '0'), \
  (byte & 0x00004000 ? '1' : '0'), \
  (byte & 0x00002000 ? '1' : '0'), \
  (byte & 0x00001000 ? '1' : '0'), \
  (byte & 0x00000800 ? '1' : '0'), \
  (byte & 0x00000400 ? '1' : '0'), \
  (byte & 0x00000200 ? '1' : '0'), \
  (byte & 0x00000100 ? '1' : '0'), \
  (byte & 0x00000080 ? '1' : '0'), \
  (byte & 0x00000040 ? '1' : '0'), \
  (byte & 0x00000020 ? '1' : '0'), \
  (byte & 0x00000010 ? '1' : '0'), \
  (byte & 0x00000008 ? '1' : '0'), \
  (byte & 0x00000004 ? '1' : '0'), \
  (byte & 0x00000002 ? '1' : '0'), \
  (byte & 0x00000001 ? '1' : '0') 

#define MAX31855_THERMOCOUPLE_OK            0
#define MAX31855_THERMOCOUPLE_SHORT_TO_VCC  1
#define MAX31855_THERMOCOUPLE_SHORT_TO_GND  2
#define MAX31855_THERMOCOUPLE_NOT_CONNECTED 3
#define MAX31855_THERMOCOUPLE_UNKNOWN       4
#define MAX31855_THERMOCOUPLE_READ_FAIL     5

#define MAX31855_CONVERSION_POWER_UP_TIME   200    //in milliseconds
#define MAX31855_CONVERSION_TIME            100    //in milliseconds, 9..10Hz sampling rate 
#define MAX31855_THERMOCOUPLE_RESOLUTION    0.25   //in °C per dac step
#define MAX31855_COLD_JUNCTION_RESOLUTION   0.0625 //in °C per dac step


typedef struct max31856_cfg_t {
  spi_device_handle_t spi;
  float coldjunction_c;
  float coldjunction_f;
  float thermocouple_c;
  float thermocouple_f;
  uint8_t fault;
} max31855_cfg_t;

max31855_cfg_t max31855_init();
uint8_t max31855_get_temperature(max31855_cfg_t *max31855);

#endif /* _COMPONENTS_MAX31855_H_ */
