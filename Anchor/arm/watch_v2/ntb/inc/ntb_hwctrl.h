#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f4xx.h"

// NTB-specific hardware definitions
#define LED_ARM_PORT  (GPIOC)
#define LED_ARM_PERIPH (RCC_AHB1Periph_GPIOC)
#define LED_ARM_GREEN (GPIO_Pin_11)
#define LED_ARM_RED   (GPIO_Pin_10)
#define LED_ARM_BLUE  (GPIO_Pin_12)
#define NTB_LED_ARM_GREEN 	('g')
#define NTB_LED_ARM_RED		('r')
#define NTB_LED_ARM_BLUE	('b')

// DW1000 SPI Settings
#define DW_SPI_PORT 	(GPIOA)
#define DW_SPI			(SPI1)
#define DW_SPI_AF		(GPIO_AF_SPI1)
#define DW_SPI_PERIPH (RCC_APB2Periph_SPI1)
#define DW_SPI_GPIOPERIPH (RCC_AHB1Periph_GPIOA)

#define DW_SCK_PIN (GPIO_Pin_5)
#define DW_SCK_PSOURCE (GPIO_PinSource5)

#define DW_MISO_PIN (GPIO_Pin_6)
#define DW_MISO_PSOURCE (GPIO_PinSource6)

#define DW_MOSI_PIN (GPIO_Pin_7)
#define DW_MOSI_PSOURCE (GPIO_PinSource7)

#define DW_SPI_CS_PORT 	(GPIOA)
#define DW_CS_PIN (GPIO_Pin_4)

// DW Reset Pin
#define DW_RESET_PORT (GPIOA)
#define DW_RESET_PIN (GPIO_Pin_3)
#define DW_RESET_PERIPH (RCC_AHB1Periph_GPIOA)

// DW IRQ Pin
#define DW_IRQ_PORT 	(GPIOA)
#define DW_IRQ_PIN		(GPIO_Pin_2)
#define DW_IRQ_PERIPH (RCC_AHB1Periph_GPIOA)
#define DW_IRQ_LINE (EXTI_Line2)
#define DW_IRQ_CHNL (EXTI2_IRQn) // line 2
#define DW_IRQ_EXTI	  (EXTI_PortSourceGPIOA)
#define DW_IRQ_PSOURCE (EXTI_PinSource2)

// Hardware function declarations
void ntb_init_leds();
void ntb_init_spidw();
void ntb_init_hw();
void ntb_deinit_uart();
void ntb_init_uart();

void ntb_led_on(uint8_t led);
void ntb_led_off(uint8_t led);
void ntb_led_toggle(uint8_t led);
void ntb_led_error();

void ntb_soft_reset();
void ntb_busywait_ms(int ms);


#endif // NTB_HWCTRL_H_