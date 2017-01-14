#include "deca_spi.h"
#include "ntb_hwctrl.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"


uint8_t dw_spi_transferbyte(uint8_t byte)
{	 
	while(!SPI_I2S_GetFlagStatus(DW_SPI, SPI_I2S_FLAG_TXE));
	SPI_I2S_SendData(DW_SPI, byte);
	while(!SPI_I2S_GetFlagStatus(DW_SPI, SPI_I2S_FLAG_RXNE));

	return SPI_I2S_ReceiveData(DW_SPI);
}

int dw_spi_sendpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body)
{
	int i;
	uint8_t tmp;

	// CS Low and wait a bit
	GPIO_ResetBits(DW_SPI_CS_PORT, DW_CS_PIN);
	//dw_sleep_usec(1);

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
	//dw_sleep_usec(1);

	return 0;
}

int dw_spi_readpacket(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body)
{
	int i;
	uint8_t tmp;

	// CS Low and wait a bit
	GPIO_ResetBits(DW_SPI_CS_PORT, DW_CS_PIN);
	//dw_sleep_usec(1);

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
	//dw_sleep_usec(1);

	return 0;
}

void dw_sleep_usec(uint32_t usec)
{
	int i;
	for( i=0; i<usec*CLOCK_TICKS_PER_USEC; i++ )
	{
		__asm("nop");
	}
}

void dw_sleep_msec(uint32_t msec)
{
	dw_sleep_usec(msec*1000);
}


void dw_spi_configprescaler(uint16_t scalingfactor)
{
	SPI_InitTypeDef SPI_InitStructure;

	SPI_I2S_DeInit(DW_SPI);

	// SPI Mode setup
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	 //
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = scalingfactor; //sets BR[2:0] bits - baudrate in SPI_CR1 reg bits 4-6
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(DW_SPI, &SPI_InitStructure);

	// Enable SPI
	SPI_Cmd(DW_SPI, ENABLE);
}
