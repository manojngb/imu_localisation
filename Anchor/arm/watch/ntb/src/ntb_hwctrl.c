#include "ntb_hwctrl.h"


void ntb_init_hw()
{
  ntb_init_gpio();
  ntb_init_spi();
  ntb_init_i2c();
  ntb_init_uart();
  dw_sleep_msec(500);
}


void ntb_init_spi()
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
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // 4.5 MHz
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

void ntb_init_i2c()
{
  I2C_InitTypeDef I2C_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  I2C_DeInit(I2C2);

  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;  
  I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  I2C_InitStructure.I2C_DigitalFilter = 0x00;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  //I2C_InitStructure.I2C_Timing = 0x00; 

  I2C_Init(I2C2, &I2C_InitStructure);  
  I2C_Cmd(I2C2, ENABLE);
}

void ntb_init_uart()
{
  USART_InitTypeDef USART_InitStructure;
  USART_StructInit(&USART_InitStructure);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_Init(USART2, &USART_InitStructure);
  USART_Cmd(USART2, ENABLE);
  NVIC_EnableIRQ(USART2_IRQn);
}

void ntb_init_gpio()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

	// Enable LED on Port B
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = ARM_LED_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(ARM_LED_PORT, &GPIO_InitStruct);
	GPIO_ResetBits(ARM_LED_PORT, ARM_LED_PIN);

  // Configure CS
  RCC_AHBPeriphClockCmd(DW_SPI_GPIOPERIPH, ENABLE);
  GPIO_InitStruct.GPIO_Pin = DW_CS_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DW_SPI_CS_PORT, &GPIO_InitStruct);
  GPIO_SetBits(DW_SPI_CS_PORT, DW_CS_PIN);

  // Configure SCLK, MOSI, MISO
  RCC_AHBPeriphClockCmd(DW_SPI_GPIOPERIPH, ENABLE);
  GPIO_InitStruct.GPIO_Pin = DW_SCK_PIN | DW_MISO_PIN | DW_MOSI_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(DW_SPI_PORT, &GPIO_InitStruct);

  // SPI alternate functions
  GPIO_PinAFConfig(DW_SPI_PORT, DW_SCK_PSOURCE,  DW_SPI_AF);
  GPIO_PinAFConfig(DW_SPI_PORT, DW_MISO_PSOURCE, DW_SPI_AF);
  GPIO_PinAFConfig(DW_SPI_PORT, DW_MOSI_PSOURCE, DW_SPI_AF);

  // Configure DW reset line  
  RCC_AHBPeriphClockCmd(DW_RESET_PERIPH, ENABLE);
  GPIO_InitStruct.GPIO_Pin = DW_RESET_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(DW_RESET_PORT, &GPIO_InitStruct);

  // ensure reset is high to begin
  GPIO_SetBits(DW_RESET_PORT, DW_RESET_PIN);

  // Configure the DW IRQ line
  // Enable SYSCFG's APB interface clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
  GPIO_InitStruct.GPIO_Pin = DW_IRQ_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(DW_IRQ_PORT, &GPIO_InitStruct);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource9);

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

  // I2C for MPU-9250 - I2C2, SCL: PA9, SDA: PA10
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_4);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_4);

  // UART lines: USART2 tx(PA14) and rx(PA15)
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource14,  GPIO_AF_7);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_7);
}


// LED functions
void ntb_led_on()
{
  GPIO_SetBits(ARM_LED_PORT, ARM_LED_PIN);
}

void ntb_led_off()
{
  GPIO_ResetBits(ARM_LED_PORT, ARM_LED_PIN);
}

void ntb_led_toggle()
{
  ARM_LED_PORT->ODR ^= ARM_LED_PIN;
}

void ntb_led_error()
{
  int i;

  while(1)
  {
    ntb_led_on();
    ntb_busywait_ms(100);
    ntb_led_off();
    ntb_busywait_ms(100);
  }
}

void ntb_led_asserterror()
{
  int i;

  while(1)
  {
    ntb_led_on();
    ntb_busywait_ms(50);
    ntb_led_off();
    ntb_busywait_ms(50);
  }
}

//Quick hack, approximately 1ms delay
void ntb_busywait_ms(int ms)
{
   while (ms-- > 0) {
      volatile int x=6000;
      while (x-- > 0)
         __asm("nop");
   }
}
