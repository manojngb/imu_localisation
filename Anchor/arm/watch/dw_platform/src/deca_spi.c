#include "deca_spi.h"
#include "ntb_hwctrl.h"
#include <stdint.h>
#include "stm32f30x.h"
#include "stm32f30x_spi.h"


uint8_t dw_spi_transferbyte(uint8_t byte)
{	 
	while(SPI_I2S_GetFlagStatus(DW_SPI, SPI_I2S_FLAG_TXE) == RESET);
	SPI_SendData8(DW_SPI, byte);
	while(SPI_I2S_GetFlagStatus(DW_SPI, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_ReceiveData8(DW_SPI);
}

int dw_spi_sendpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body)
{
	int i;
	uint8_t tmp;

	// CS Low and wait a bit
	GPIO_ResetBits(DW_SPI_CS_PORT, DW_CS_PIN);

	// send header
	for( i=0; i<headerLen; i++ )
	{
		tmp = dw_spi_transferbyte( header[i] );
	}
	// send body
	for( i=0; i<bodyLen; i++ )
	{
		tmp = dw_spi_transferbyte( body[i] );
	}

	// CS high and wait a bit
	GPIO_SetBits(DW_SPI_CS_PORT, DW_CS_PIN);

	return 0;
}

int dw_spi_readpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body)
{
	int i;
	uint8_t tmp;

	// CS Low and wait a bit
	GPIO_ResetBits(DW_SPI_CS_PORT, DW_CS_PIN);

	// send header
	for( i=0; i<headerLen; i++ )
	{
		tmp = dw_spi_transferbyte( header[i] );
	}
	// send body
	for( i=0; i<bodyLen; i++ )
	{
		body[i] = dw_spi_transferbyte( 0x00 );
	}

	// CS high and wait a bit
	GPIO_SetBits(DW_SPI_CS_PORT, DW_CS_PIN);
	return 0;
}

void dw_sleep_msec(uint32_t ms)
{
   while (ms-- > 0) {
      volatile int x=6000;
      while (x-- > 0)
         __asm("nop");
   }
}


void dw_spi_configprescaler(uint16_t scalingfactor)
{
  // init structures
  SPI_InitTypeDef SPI_InitStruct;

  /* ------------ Init SPI Engine --------------- */
  RCC_APB2PeriphClockCmd(DW_SPI_PERIPH, ENABLE);

  /* configure SPI1 in Mode 0 
   * CPOL = 0 --> clock is low when idle
   * CPHA = 0 --> data is sampled at the first edge
   */
  SPI_I2S_DeInit(DW_SPI);
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;  
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;       
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = scalingfactor; // 4.5 MHz
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStruct.SPI_CRCPolynomial = 7;
  SPI_Init(DW_SPI, &SPI_InitStruct);

  // ensure SPI_I2S_FLAG_RXNE is set after 1 byte (default is 2)
  SPI_RxFIFOThresholdConfig(DW_SPI, SPI_RxFIFOThreshold_QF);

  // Enable SPI
  SPI_Cmd(DW_SPI, ENABLE);

  // ensure RXNE is initially clear
  SPI_ReceiveData8(DW_SPI); 
}
