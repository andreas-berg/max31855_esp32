/***************************************************************************************************/
/*
   This is an esp-idf component C-library for the 14-bit MAX31855 K-Thermocouple to Digital Converter
   with 12-bit Cold Junction Compensation conneted to hardware 5Mhz SPI with maximum sampling
   rate ~9..10Hz.
   - MAX31855 maximum power supply voltage is 3.6v
   - Maximum SPI bus speed 5Mhz
   - K-type thermocouples have an absolute accuracy of around ±2°C..±6°C.
   - Measurement tempereture range -200°C..+700°C ±2°C or -270°C..+1372°C ±6°C
     with 0.25°C resolution/increment.
   - Cold junction compensation range -40°C..+125° ±3°C with 0.062°C resolution/increment.
     Optimal performance of cold junction compensation happens when the thermocouple cold junction
     & the MAX31855 are at the same temperature. Avoid placing heat-generating devices or components
     near the converter because this may produce an errors.
   - It is strongly recommended to add a 10nF/0.01mF ceramic surface-mount capacitor, placed across
     the T+ and T- pins, to filter noise on the thermocouple lines.
     
   based on Arduino library written by : enjoyneering79, https://github.com/enjoyneering/MAX31855
   written by : andreasberg
   source code: https://github.com/andreas-berg/max31855
   This sensor uses SPI bus to communicate, specials pins are required to interface
   Board:                                    MOSI        MISO        SCLK         CS          Level
   ESP32.................................... x           GPIO12      GPIO14       GPIO15      3v

   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/

#include "max31855.h"
#include <string.h>
// #include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/spi_master.h>
#include <driver/gpio.h>

#include <esp_log.h>
#include "max31855.h"
#include "esp32/rom/ets_sys.h"  // for ets_delay_us()

#define DEBUG 0

static const char *TAG = "max31855";

spi_device_handle_t spi_handle;

/* static prototypes */
static uint32_t read_raw_data();
static uint8_t check_thermocouple(uint32_t rawValue);


max31855_cfg_t max31855_init()
{
  ESP_LOGI(TAG, "Initialize");

  // manually set the CS line
  gpio_config_t io_conf;
  io_conf.pull_down_en = 0;
  io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << PIN_NUM_CS);
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = 0;
  gpio_config(&io_conf);
  // set CS to High when idle
  gpio_set_level(PIN_NUM_CS, 1);

  // setup normal SPI, MOSI not needed
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .miso_io_num = PIN_NUM_MISO,
      .mosi_io_num = -1,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0,
  };

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = 1000000, // max 5 MHz
      .dummy_bits = 0,
      .mode = 1,
      .flags = 0,
      .spics_io_num = -1, // Manually Control CS
      .queue_size = 1,
  };
  ret = spi_bus_initialize(SPI_BUS, &buscfg, 0); // No DMA
  ESP_ERROR_CHECK(ret);
  ret = spi_bus_add_device(SPI_BUS, &devcfg, &spi_handle);
  ESP_ERROR_CHECK(ret);

  vTaskDelay(MAX31855_CONVERSION_POWER_UP_TIME/portTICK_PERIOD_MS);

  return (max31855_cfg_t){
      .spi = spi_handle,
  };
}

/**************************************************************************/
/*
    check_thermocouple()
    Checks if thermocouple is open, shorted to GND, shorted to VCC
    Return:
    - 0 OK
    - 1 short to VCC
    - 2 short to GND
    - 3 not connected
    NOTE:
    - bit D16 is normally low & goes high if thermocouple is open, shorted to GND or VCC
    - bit D2  is normally low & goes high to indicate a hermocouple short to VCC (SCV Fault)
    - bit D1  is normally low & goes high to indicate a thermocouple short to GND (SCG Fault)
    - bit D0  is normally low & goes high to indicate a thermocouple open circuit (OC Fault)
*/
/**************************************************************************/
static uint8_t check_thermocouple(uint32_t rawValue)
{
  if (rawValue == 0) return MAX31855_THERMOCOUPLE_READ_FAIL;
  if (rawValue & MAX31855_BITMASK_D16) {
    if ((rawValue >> 2) & 1) return MAX31855_THERMOCOUPLE_SHORT_TO_VCC;
    if ((rawValue >> 1) & 1) return MAX31855_THERMOCOUPLE_SHORT_TO_GND;
    if ((rawValue >> 0) & 1) return MAX31855_THERMOCOUPLE_NOT_CONNECTED;
    else return MAX31855_THERMOCOUPLE_UNKNOWN;
  }
  return MAX31855_THERMOCOUPLE_OK;
}

/**************************************************************************/
/*
    read_raw_data()
    Reads raw data from MAX31855 via hardware SPI
    NOTE:
    - max SPI clock speed for MAX31855 is 5MHz
    - in SPI_MODE0 data available shortly after the rising edge of SCK
    - read of the cold-junction compensated thermocouple temperature requires
      14 clock cycles
    - read of the cold-junction compensated thermocouple temperature & reference
      junction temperatures requires 32 clock cycles
    - forcing CS low immediately stops any conversion process, force CS high
      to initiate a new measurement process
    - set CS low to enable the serial interface & force to output the first bit on the SO pin,
      apply 14/32 clock signals at SCK to read the results at SO on the falling edge of the SCK
    - bit D31 is the thermocouple temperature sign bit "+" is high & "-" is low,
      if T+ & T- pins are unconnected it goes low
    - bits D30..D18 contain the converted temperature in the order of MSB to LSB,
      if T+ & T- pins are unconnected they go high
    - bit D17 is low to provide a device ID for the MAX31855
    - bit D16 is normally low & goes high if thermocouple is open, shorted to GND or VCC
    - bit D15 is cold-junction temperature sign bit "+" is high & "-" is low
    - bits D14..D4 contain cold-junction temperature in the order of MSB to LSB
    - bit D3 is is low to provide a device ID for the MAX31855
    - bit D2 is normally low & goes high to indicate a hermocouple short to VCC
    - bit D1 is normally low & goes high to indicate a thermocouple short to GND
    - bit D0 is normally low & goes high to indicate a thermocouple open circuit
    - 32-bit 160MHz/250MHz ESP32 one clock cycle is 6.25nS/4nS
*/
static uint32_t read_raw_data()
{
  esp_err_t ret;
  uint32_t rawData = 0;
  spi_transaction_t spi_transaction;

  gpio_set_level(PIN_NUM_CS, 0);  // start conversion by toggling CS line-> Low
  ets_delay_us(1);                // 1 micro second delay
  gpio_set_level(PIN_NUM_CS, 1);  // reset CS line -> High

  vTaskDelay(MAX31855_CONVERSION_TIME/portTICK_PERIOD_MS); // wait a little

  memset(&spi_transaction, 0, sizeof(spi_transaction_t));
  spi_transaction.length = 32;    // rx_length is by default = length
  spi_transaction.flags = SPI_TRANS_USE_RXDATA;

  gpio_set_level(PIN_NUM_CS, 0);  // the max31855 will send its data (32bits) on MISO when master signals CS -> Low. 
  
  ret = spi_device_transmit(spi_handle, &spi_transaction); // MOSI is not connected and TXDATA is empty, so nothing sent.
  ESP_ERROR_CHECK(ret);

  gpio_set_level(PIN_NUM_CS, 1); // reset CS line -> High

  for (uint8_t i = 0; i < 4; i++) rawData = (rawData << 8) | (uint8_t)spi_transaction.rx_data[i]; // combine rx_data buffer (uint8_t[4]) to a single uint32_t 
#if defined DEBUG
  printf(" "DEBUG_4BYTE_TO_BINARY_PATTERN, rawData, DEBUG_BYTE4_TO_BINARY(rawData), DEBUG_BYTE3_TO_BINARY(rawData), DEBUG_BYTE2_TO_BINARY(rawData), DEBUG_BYTE1_TO_BINARY(rawData));
#endif
  return rawData;
}

/**************************************************************************/
/*
    max31855_get_temperature()
    Reads Temperature, C
    NOTE:
    - range -200°C..+700°C ±2°C or -270°C..+1372°C ±6°C with 0.25°C
      resolution/increment
    - thermocouple temperature data is 14-bit long
    - bit D31 is the thermocouple temperature sign bit "+" is high & "-" is low,
      if T+ and T- are unconnected it goes low
    - bits D30..D18 contain the converted temperature in the order of MSB to LSB,
      if T+ and T- are unconnected they go high
    - it is strongly recommended to add a 10nF/0.01mF ceramic surface-mount
      capacitor, placed across the T+ and T- pins, to filter noise on the
      thermocouple lines
*/
/**************************************************************************/
uint8_t max31855_get_temperature(max31855_cfg_t *max31855)
{
  // spi_device_handle_t spi = max31855->spi;
  uint32_t rawData = read_raw_data(max31855->spi);

  uint8_t err = check_thermocouple(rawData);
  if (err != MAX31855_THERMOCOUPLE_OK) {
    max31855->fault = (uint8_t)((rawData & MAX31855_BITMASK_D16) >> (16-3) | (rawData & MAX31855_BITMASK_D210)); // extract fault 4bit D16+D2+D1+D0
    return err;
  }
  
  // Read thermocouple temperature data
  uint32_t value = MAX31855_EXTRACT_VALUE_WITH_BITMASK(rawData, MAX31855_BITMASK_TC_DATA_13BIT);
  uint32_t sign = MAX31855_EXTRACT_VALUE_WITH_BITMASK(rawData, MAX31855_BITMASK_TC_DATA_SIGN);
#if defined DEBUG
   printf(" TC_TEMP "DEBUG_2BYTE_TO_BINARY_PATTERN, value, DEBUG_BYTE2_TO_BINARY(value), DEBUG_BYTE1_TO_BINARY(value));
   printf(" TC_SIGN 0x%02x",sign);
#endif
  float tc_temperature = (float)(MAX31855_THERMOCOUPLE_RESOLUTION * (int16_t)(MAX31855_EXTEND_SIGN(sign, value, 13)));

  // Read internal (cold junction) temperature data
  value = MAX31855_EXTRACT_VALUE_WITH_BITMASK(rawData, MAX31855_BITMASK_CJ_DATA_11BIT);
  sign = MAX31855_EXTRACT_VALUE_WITH_BITMASK(rawData, MAX31855_BITMASK_CJ_DATA_SIGN);
#if defined DEBUG
   printf(" CJ_TEMP "DEBUG_2BYTE_TO_BINARY_PATTERN, value, DEBUG_BYTE2_TO_BINARY(value), DEBUG_BYTE1_TO_BINARY(value));
   printf(" CJ_SIGN 0x%02x",sign);
#endif
  float cj_temperature = (float)(MAX31855_COLD_JUNCTION_RESOLUTION * (int16_t)(MAX31855_EXTEND_SIGN(sign, value, 11)));

#if defined DEBUG
  printf(" TC: %0.4f°C CJ: %0.4f°C\n", tc_temperature, cj_temperature );
#endif

  max31855->thermocouple_c = tc_temperature;
  max31855->thermocouple_f = (1.8 * tc_temperature) + 32.0;
  max31855->coldjunction_c = cj_temperature;
  max31855->coldjunction_f = (1.8 * cj_temperature) + 32.0;

  return MAX31855_THERMOCOUPLE_OK;
}

