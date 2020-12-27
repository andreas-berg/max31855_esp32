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
#define DEBUG_2BYTE_TO_BINARY_PATTERN "(0x%04x) %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c"
#define DEBUG_BYTE4_TO_BINARY(byte) \
  (byte & 0x80000000 ? '1' : '0'), \
  (byte & 0x40000000 ? '1' : '0'), \
  (byte & 0x20000000 ? '1' : '0'), \
  (byte & 0x10000000 ? '1' : '0'), \
  (byte & 0x08000000 ? '1' : '0'), \
  (byte & 0x04000000 ? '1' : '0'), \
  (byte & 0x02000000 ? '1' : '0'), \
  (byte & 0x01000000 ? '1' : '0')
#define DEBUG_BYTE3_TO_BINARY(byte) \
  (byte & 0x00800000 ? '1' : '0'), \
  (byte & 0x00400000 ? '1' : '0'), \
  (byte & 0x00200000 ? '1' : '0'), \
  (byte & 0x00100000 ? '1' : '0'), \
  (byte & 0x00080000 ? '1' : '0'), \
  (byte & 0x00040000 ? '1' : '0'), \
  (byte & 0x00020000 ? '1' : '0'), \
  (byte & 0x00010000 ? '1' : '0')
#define DEBUG_BYTE2_TO_BINARY(byte) \
  (byte & 0x00008000 ? '1' : '0'), \
  (byte & 0x00004000 ? '1' : '0'), \
  (byte & 0x00002000 ? '1' : '0'), \
  (byte & 0x00001000 ? '1' : '0'), \
  (byte & 0x00000800 ? '1' : '0'), \
  (byte & 0x00000400 ? '1' : '0'), \
  (byte & 0x00000200 ? '1' : '0'), \
  (byte & 0x00000100 ? '1' : '0')
#define DEBUG_BYTE1_TO_BINARY(byte) \
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

#define MAX31855_BITMASK_D16                0x10000    // faultbit D16 = 0b00000000000000010000000000000000 = 0x10000
#define MAX31855_BITMASK_D210               0x7        // D[2:0]       = 0b00000000000000000000000000000111 = 0x7
#define MAX31855_BITMASK_TC_DATA_13BIT      0x7FFC0000 // D[30:18]     = 0b01111111111111000000000000000000 = 0x7FFC0000
#define MAX31855_BITMASK_TC_DATA_SIGN       0x80000000 // sign D31     = 0b10000000000000000000000000000000 = 0x80000000
#define MAX31855_BITMASK_CJ_DATA_11BIT      0x7FF0     // D[14:4]      = 0b00000000000000000111111111110000 = 0x7FF0
#define MAX31855_BITMASK_CJ_DATA_SIGN       0x8000     // sign D15     = 0b00000000000000001000000000000000 = 0x8000

// see https://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static const int MultiplyDeBruijnBitPosition[32] = 
{
  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

// extract masked bits and shift right (f.e. 18 bits for MAX31855_BITMASK_TC_DATA_13BIT, use DeBruijnBitPosition to determine the count of trailing 0s)
#define MAX31855_EXTRACT_VALUE_WITH_BITMASK(DATA, MASK)  ((DATA & MASK) >> MultiplyDeBruijnBitPosition[((uint32_t)((MASK & -MASK) * 0x077CB531U)) >> 27])

// Negative tempererature values (sign = 1) are ORed with a mask of 1s to extend the signing bit to the full 16-bit word, 
// f.e. for a 13 bit mask/value length this sign-mask is 0b1110000000000000 = 0xE000
// the mask is created from 16bit all 1s : (1 << 16) - 1 = 0b111111111111111111 and
// right-shifting LEN steps (truncating) and left-shifting (zero-padding) the same LEN
// ORed with VALUE 0b1010101010101 (13bits) | OxE000 = 0b1111010101010101 (16bits)
// ORed with VALUE 0b10101010101 (11bits)   | OxF800 = 0b1111110101010101 (16bits)
// Positive values are returned unchanged. 
#define MAX31855_EXTEND_SIGN(SIGN, VALUE, LEN)  (SIGN == 1)?((((1 << 16)-1) >> LEN << LEN) | VALUE):VALUE

#define MAX31855_TEST_DATA1                 0b00000110010011000110010010010000 // TC: +100.75, CJ: +100.5625, Faults 0 (0x64C6490)
#define MAX31855_TEST_DATA2                 0b11111111111111001111111111110000 // TC: -0.25, CJ: -0.0625, Faults 0 (0xFFFCFFF0)
#define MAX31855_TEST_DATA3                 0b11110000011000011100100100000010 // TC: -250.00, CJ: -55.00, Faults D16=1 and SCG (ShortToGND)

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
