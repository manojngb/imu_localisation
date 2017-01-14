

#define USE_STDPERIPH_DRIVER
#include "stm32f4xx.h"

#define LED_PORT GPIOE
#define LED_GREEN (GPIO_Pin_4)
#define LED_RED   (GPIO_Pin_5)
#define LED_BLUE  (GPIO_Pin_6)

#define UID_PORT GPIOD
#define UIDB4 (GPIO_Pin_14)
#define UIDB3 (GPIO_Pin_13)
#define UIDB2 (GPIO_Pin_12)
#define UIDB1 (GPIO_Pin_11)
#define UIDB0 (GPIO_Pin_10)

 uint8_t uid;

//Quick hack, approximately 1ms delay
void ms_delay(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}

//Configure pins and clocks
void leds_init()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Enable Port E
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = LED_GREEN | LED_RED | LED_BLUE;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(LED_PORT, &GPIO_InitStruct);

}

void uid_init()
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

void spi_exp_deinit()
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

void gpio_init()
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


//Flash orange LED at about 1hz
int main(void)
{
    leds_init();
    //uid_init();
    spi_exp_deinit();
    gpio_init();

    // Read UID


    // Init LED values
    GPIO_ResetBits(LED_PORT, LED_GREEN);
    GPIO_ResetBits(LED_PORT, LED_RED);
    GPIO_ResetBits(LED_PORT, LED_BLUE);

    GPIO_SetBits(LED_PORT, LED_RED);
    ms_delay(1000);
    GPIO_ResetBits(LED_PORT, LED_RED);
    // reset network switch
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
    ms_delay(100);
    GPIO_SetBits(GPIOD, GPIO_Pin_2);

    for (;;) {
       // Ramp on
       ms_delay(150);
       GPIO_SetBits(LED_PORT, LED_GREEN);
       ms_delay(150);
       GPIO_SetBits(LED_PORT, LED_RED);
       ms_delay(150);
       GPIO_SetBits(LED_PORT, LED_BLUE);

       // Ramp off
       ms_delay(150);
       GPIO_ResetBits(LED_PORT, LED_GREEN);
       ms_delay(150);
       GPIO_ResetBits(LED_PORT, LED_RED);
       ms_delay(150);
       GPIO_ResetBits(LED_PORT, LED_BLUE);



    }
}
