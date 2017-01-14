// Includes
#include "leds.h"


//Configure pins and clocks
void leds_init()
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	// ---------- SysTick timer -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (true);    // Capture error
	}

	// Enable Port A, C, and D for VCP + LEDs
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Configure PD12, PD13, PD14 and PD15 (LEDs) in output pushpull mode
	GPIO_InitStruct.GPIO_Pin = LED_GREEN_PIN|LED_ORANGE_PIN|LED_RED_PIN|LED_BLUE_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIOD, &GPIO_InitStruct);

}

// LED functions
void led_set(uint8_t led)
{
	switch(led)
	{
		case LED_GREEN:
			GPIO_SetBits(LED_GREEN_GPIO_PORT, LED_GREEN_PIN);
			break;
		case LED_ORANGE:
			GPIO_SetBits(LED_ORANGE_GPIO_PORT, LED_ORANGE_PIN);
			break;
		case LED_RED:
			GPIO_SetBits(LED_RED_GPIO_PORT, LED_RED_PIN);
			break;
		case LED_BLUE:
			GPIO_SetBits(LED_BLUE_GPIO_PORT, LED_BLUE_PIN);
			break;
		default:
			break;
	}
}

void led_clear(uint8_t led)
{
	switch(led)
	{
		case LED_GREEN:
			GPIO_ResetBits(LED_GREEN_GPIO_PORT, LED_GREEN_PIN);
			break;
		case LED_ORANGE:
			GPIO_ResetBits(LED_ORANGE_GPIO_PORT, LED_ORANGE_PIN);
			break;
		case LED_RED:
			GPIO_ResetBits(LED_RED_GPIO_PORT, LED_RED_PIN);
			break;
		case LED_BLUE:
			GPIO_ResetBits(LED_BLUE_GPIO_PORT, LED_BLUE_PIN);
			break;
		default:
			break;
	}
}

void led_toggle(uint8_t led)
{
	switch(led)
	{
		case LED_GREEN:
			GPIO_ToggleBits(LED_GREEN_GPIO_PORT, LED_GREEN_PIN);
			break;
		case LED_ORANGE:
			GPIO_ToggleBits(LED_ORANGE_GPIO_PORT, LED_ORANGE_PIN);
			break;
		case LED_RED:
			GPIO_ToggleBits(LED_RED_GPIO_PORT, LED_RED_PIN);
			break;
		case LED_BLUE:
			GPIO_ToggleBits(LED_BLUE_GPIO_PORT, LED_BLUE_PIN);
			break;
		default:
			break;
	}
}

void leds_error()
{
	int i;

	while(1)
	{
		led_clear(LED_GREEN);
		led_clear(LED_ORANGE);
		led_clear(LED_RED);
		led_clear(LED_BLUE);

		for( i=0; i<1e6; i++)
		{
			__asm("nop");
		}

		led_set(LED_GREEN);
		led_set(LED_ORANGE);
		led_set(LED_RED);
		led_set(LED_BLUE);

		for( i=0; i<1e6; i++)
		{
			__asm("nop");
		}
	}
}