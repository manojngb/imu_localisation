#include "ntb_hwctrl.h"

void ntb_init_hw()
{
  ntb_init_leds();
  ntb_init_uid();
  ntb_init_gpio();
  ntb_deinit_spiexp();
}

void ntb_init_leds()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Enable Port E
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = LED_ARM_GREEN | LED_ARM_RED | LED_ARM_BLUE | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(LED_ARM_PORT, &GPIO_InitStruct);

}

void ntb_init_uid()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = UIDB0 | UIDB1 | UIDB2 | UIDB3 | UIDB4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);
}

void ntb_deinit_spiexp()
{
  // Release expansion SPI for external control
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Chip Select
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure PD1 in input, no pull
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  // MOSI / MISO / SCK
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  // Configure PD1 in input, no pull
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

}

void ntb_init_gpio()
{
  // Release expansion SPI for external control
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure GPIO1 to output, high (this is DW1000 RSTn)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_4);

  // Configure GPIO3 to input, no pull (this is DW1000 IRQ)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_6);

  // Configure NETW_RST high (this is ethernet RSTn)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_2);
}


// LED functions
void ntb_led_on(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_off(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_toggle(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
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
       ntb_led_on(LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_on(LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_on(LED_ARM_BLUE);
       ntb_busywait_ms(150);
       GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_9);
       ntb_busywait_ms(150);
       GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_10);
       ntb_busywait_ms(150);
       GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_11);
       ntb_busywait_ms(150);
       GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_12);

       // Ramp off
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_BLUE);
       ntb_busywait_ms(150);
       GPIO_ResetBits(LED_ARM_PORT, GPIO_Pin_9);
       ntb_busywait_ms(150);
       GPIO_ResetBits(LED_ARM_PORT, GPIO_Pin_10);
       ntb_busywait_ms(150);
       GPIO_ResetBits(LED_ARM_PORT, GPIO_Pin_11);
       ntb_busywait_ms(150);
       GPIO_ResetBits(LED_ARM_PORT, GPIO_Pin_12);
  }
}

void ntb_ethswitch_off()
{
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_RST);
}

void ntb_ethswitch_on()
{
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_RST);
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