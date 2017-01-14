#include "deca_spi.h"
#include "ntb_hwctrl.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"

void dw_spi_init()
{
	// ----------- Turn on orange LED to say we're busy -------------
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	SPI_I2S_DeInit(DW_SPI);

	/* ------------ Fire up SPI --------------- */
	RCC_APB1PeriphClockCmd(DW_SPI_PERIPH, ENABLE);

	/* configure SPI1 in Mode 0 
	 * CPOL = 0 --> clock is low when idle
	 * CPHA = 0 --> data is sampled at the first edge
	 */
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;  
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;       
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; // 4-64
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;

	SPI_Init(DW_SPI, &SPI_InitStruct); 

	// Turn on port C
	RCC_AHB1PeriphClockCmd(DW_SPI_GPIOPERIPH, ENABLE);
	 
	// Configure SCK, MISO, & MOSI
	GPIO_InitStruct.GPIO_Pin = DW_SCK_PIN | DW_MISO_PIN | DW_MOSI_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DW_SPI_PORT, &GPIO_InitStruct);

	// Configure CS
	GPIO_InitStruct.GPIO_Pin = DW_CS_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DW_SPI_CS_PORT, &GPIO_InitStruct);

	// SPI alternate functions
	GPIO_PinAFConfig(DW_SPI_PORT, DW_SCK_PSOURCE,  DW_SPI_AF);
	GPIO_PinAFConfig(DW_SPI_PORT, DW_MISO_PSOURCE, DW_SPI_AF);
	GPIO_PinAFConfig(DW_SPI_PORT, DW_MOSI_PSOURCE, DW_SPI_AF);

	// Disable SPI SS output
	SPI_SSOutputCmd(DW_SPI, DISABLE);

	// start with CS high
	GPIO_SetBits(DW_SPI_CS_PORT, DW_CS_PIN);

	// Enable SPI
	SPI_Cmd(DW_SPI, ENABLE);

	// Configure DW reset line	
	RCC_AHB1PeriphClockCmd(DW_RESET_PERIPH, ENABLE);

	GPIO_InitStruct.GPIO_Pin = DW_RESET_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(DW_RESET_PORT, &GPIO_InitStruct);

	// ensure reset is high to begin
	GPIO_SetBits(DW_RESET_PORT, DW_RESET_PIN);

	// Configure the DW IRQ line
	GPIO_InitStruct.GPIO_Pin = DW_IRQ_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(DW_IRQ_PORT, &GPIO_InitStruct);
	SYSCFG_EXTILineConfig(DW_IRQ_EXTI, DW_IRQ_PSOURCE);

	// Configure the EXTI for DW IRQ
	EXTI_InitStruct.EXTI_Line = DW_IRQ_LINE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	NVIC_InitStruct.NVIC_IRQChannel = DW_IRQ_CHNL;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	// ---------- Turn off orange LED to say we're done -------------
	dw_sleep_msec(500);
}

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
