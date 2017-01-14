#include "ntb_hwctrl.h"
#include <stm32f4xx_i2c.h>


void ntb_init_hw()
{
  ntb_init_leds();
  //ntb_deinit_uart();
  ntb_init_uart();
  ntb_init_spidw();
}

void ntb_init_leds()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Enable Port
  RCC_AHB1PeriphClockCmd(LED_ARM_PERIPH, ENABLE);

  // Configure LEDs in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = LED_ARM_GREEN | LED_ARM_RED | LED_ARM_BLUE;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(LED_ARM_PORT, &GPIO_InitStruct);

}

void ntb_init_uart()
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  // de-init uart to start
  USART_DeInit(UART4);

  // Make sure port A is enabled
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // Configure pins
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);

  // Configure UART
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_Mode = USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(UART4, &USART_InitStruct);

  // enable UART
  USART_Cmd(UART4, ENABLE);
}

void ntb_deinit_uart()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // set PA0 and PA1 to inputs (high Z)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  // Configure pins
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ntb_init_spidw()
{
  // ----------- Turn on orange LED to say we're busy -------------
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  SPI_I2S_DeInit(DW_SPI);

  /* ------------ Fire up SPI --------------- */
  RCC_APB2PeriphClockCmd(DW_SPI_PERIPH, ENABLE);

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

  // Turn on port A
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

  // ensure reset is high to begin0
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

  // sleep a bit to let things settle
  dw_sleep_msec(500);
}

// LED functions
void ntb_led_on(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_off(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_toggle(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_error()
{
  int i;

  while(1)
  {
       // Ramp on
       ntb_busywait_ms(150);
       ntb_led_on(NTB_LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_on(NTB_LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_on(NTB_LED_ARM_BLUE);

       // Ramp off
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_BLUE);
  }
}

void ntb_soft_reset()
{
	NVIC_SystemReset();
}

//Quick hack, approximately 1ms delay
void ntb_busywait_ms(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}