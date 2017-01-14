#ifndef DECA_SPI_H_
#define DECA_SPI_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"

// Spi Hardware Setup
#define DW_SPI_PORT 	(GPIOC)
#define DW_SPI 			(SPI3)
#define DW_SPI_AF		(GPIO_AF_SPI3)

#define DW_SCK_PIN (GPIO_Pin_10)
#define DW_SCK_PSOURCE (GPIO_PinSource10)

#define DW_MISO_PIN (GPIO_Pin_11)
#define DW_MISO_PSOURCE (GPIO_PinSource11)

#define DW_MOSI_PIN (GPIO_Pin_12)
#define DW_MOSI_PSOURCE (GPIO_PinSource12)

#define DW_SPI_CS_PORT 	(GPIOD)
#define DW_CS_PIN (GPIO_Pin_1)
#define DW_SPI_CLEAR_CS_FAST	{DW_SPI_CS_PORT->BRR = DW_CS_PIN;}
#define DW_SPI_SET_CS_FAST		{DW_SPI_CS_PORT->BSRR = DW_CS_PIN;}

#define DW_SPI_PERIPH (RCC_APB1Periph_SPI3)
#define DW_SPI_GPIOPERIPH (RCC_AHB1Periph_GPIOC)

// DW Reset Pin
#define DW_RESET_PORT (GPIOD)
#define DW_RESET_PIN (GPIO_Pin_4) // D4
#define DW_RESET_PERIPH (RCC_AHB1Periph_GPIOD)

// DW IRQ Pin
#define DW_IRQ_PORT 	(GPIOD)
#define DW_IRQ_PIN		(GPIO_Pin_6)
#define DW_IRQ_PERIPH (RCC_AHB1Periph_GPIOD)
#define DW_IRQ_LINE (EXTI_Line6)
#define DW_IRQ_CHNL (EXTI9_5_IRQn) // lines 5-9
#define DW_IRQ_EXTI	  (EXTI_PortSourceGPIOD)
#define DW_IRQ_PSOURCE (EXTI_PinSource6)

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