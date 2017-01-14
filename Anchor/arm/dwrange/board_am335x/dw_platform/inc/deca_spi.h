#ifndef DECA_SPI_H_
#define DECA_SPI_H_

// Includes
#include <stdint.h>

// Device tree handle for decawave device
#define SPI_DEVICE "/dev/spidev2.0"
#define SPI_BITS   8
#define SPI_SPEED  3000000
#define SPI_DELAY  0
#define SPI_MODE   0

// Function definitions
void dw_spi_init();
void dw_spi_close();
uint8_t dw_spi_transferbyte(uint8_t byte);
int dw_spi_sendpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body);
int dw_spi_readpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body);
uint8_t dw_spi_poll();
void dw_sleep_usec(uint32_t usec);
void dw_sleep_msec(uint32_t msec);

#endif