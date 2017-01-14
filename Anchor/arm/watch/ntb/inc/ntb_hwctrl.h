#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f30x.h"

// Watch LED settings
#define ARM_LED_PORT (GPIOB)
#define ARM_LED_PIN (GPIO_Pin_4)

// DW1000 SPI Settings
#define DW_SPI_PORT 	(GPIOA)
#define DW_SPI 			(SPI1)
#define DW_SPI_AF		(GPIO_AF_5)

#define DW_SCK_PIN (GPIO_Pin_5)
#define DW_SCK_PSOURCE (GPIO_PinSource5)

#define DW_MISO_PIN (GPIO_Pin_6)
#define DW_MISO_PSOURCE (GPIO_PinSource6)

#define DW_MOSI_PIN (GPIO_Pin_7)
#define DW_MOSI_PSOURCE (GPIO_PinSource7)

#define DW_SPI_CS_PORT 	(GPIOA)
#define DW_CS_PIN (GPIO_Pin_8)

#define DW_SPI_PERIPH (RCC_APB2Periph_SPI1)
#define DW_SPI_GPIOPERIPH (RCC_AHBPeriph_GPIOA)

// DW Reset Pin
#define DW_RESET_PORT (GPIOB)
#define DW_RESET_PIN (GPIO_Pin_8) // D4
#define DW_RESET_PERIPH (RCC_AHBPeriph_GPIOB)

// DW IRQ Pin
#define DW_IRQ_PORT 	(GPIOB)
#define DW_IRQ_PIN		(GPIO_Pin_9)
#define DW_IRQ_PERIPH (RCC_AHBPeriph_GPIOB)
#define DW_IRQ_LINE (EXTI_Line9)
#define DW_IRQ_CHNL (EXTI9_5_IRQn) // lines 5-9
#define DW_IRQ_EXTI	  (EXTI_PortSourceGPIOB)
#define DW_IRQ_PSOURCE (EXTI_PinSource9)


// Hardware function declarations
void ntb_init_hw();
void ntb_init_gpio();
void ntb_init_spi();
void ntb_init_i2c();
void ntb_init_uart();

void ntb_led_on();
void ntb_led_off();
void ntb_led_toggle();
void ntb_led_error();
void ntb_busywait_ms(int ms);


#endif // NTB_HWCTRL_H_