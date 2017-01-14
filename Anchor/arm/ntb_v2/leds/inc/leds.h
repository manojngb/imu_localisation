/*
* main.h
*
*  Created on: 10 jul 2012
*      Author: BenjaminVe
*/

#ifndef LEDS_H_
#define LEDS_H_

// Includes	
#include <stdbool.h>
#include "stm32f4xx.h"


#define LED_GREEN  (0)
#define LED_ORANGE (1)
#define LED_RED    (2)
#define LED_BLUE   (3)

#define LEDn                             4

#define LED_GREEN_PIN                    GPIO_Pin_12
#define LED_GREEN_GPIO_PORT              GPIOD
#define LED_GREEN_GPIO_CLK               RCC_AHB1Periph_GPIOD  
  
#define LED_ORANGE_PIN                   GPIO_Pin_13
#define LED_ORANGE_GPIO_PORT             GPIOD
#define LED_ORANGE_GPIO_CLK              RCC_AHB1Periph_GPIOD  
  
#define LED_RED_PIN                      GPIO_Pin_14
#define LED_RED_GPIO_PORT                GPIOD
#define LED_RED_GPIO_CLK                 RCC_AHB1Periph_GPIOD  
  
#define LED_BLUE_PIN                     GPIO_Pin_15
#define LED_BLUE_GPIO_PORT               GPIOD
#define LED_BLUE_GPIO_CLK                RCC_AHB1Periph_GPIOD



// Function prototypes
void leds_init();
void leds_set(uint8_t led);
void leds_clear(uint8_t led);
void leds_toggle(uint8_t led);
void leds_error();

#endif /* LEDS_H_ */
