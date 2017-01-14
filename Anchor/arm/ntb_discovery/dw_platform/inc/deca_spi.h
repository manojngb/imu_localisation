#ifndef DECA_SPI_H_
#define DECA_SPI_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "ntb_hwctrl.h"

// Busy waits
#define CLOCK_TICKS_PER_USEC (33)

// Function definitions
void dw_spi_init();
uint8_t dw_spi_transferbyte(uint8_t byte);
int dw_spi_sendpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body);
int dw_spi_readpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body);
void dw_spi_busysleep(uint32_t usec);
void dw_sleep_usec(uint32_t usec);
void dw_sleep_msec(uint32_t msec);


#endif // DECA_SPI_H_