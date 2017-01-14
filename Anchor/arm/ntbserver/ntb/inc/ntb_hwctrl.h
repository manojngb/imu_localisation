#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f4xx.h"

typedef enum
{
	LED_ARM_GREEN,
	LED_ARM_RED,
	LED_ARM_BLUE,
	LED_BRGHT_GREEN,
	LED_BRGHT_RED,
	LED_BRGHT_BLUE
} led_t;

// NTB-specific hardware definitions
#define LED_ARM_PORT GPIOE
#define LED_ARM_GREEN (GPIO_Pin_4)
#define LED_ARM_RED   (GPIO_Pin_5)
#define LED_ARM_BLUE  (GPIO_Pin_6)

#define UID_PORT GPIOD
#define UIDB4 (GPIO_Pin_14)
#define UIDB3 (GPIO_Pin_13)
#define UIDB2 (GPIO_Pin_12)
#define UIDB1 (GPIO_Pin_11)
#define UIDB0 (GPIO_Pin_10)

#define ETHSWITCH_PORT (GPIOD)
#define ETHSWITCH_RST (GPIO_Pin_2)

// Hardware function declarations
void ntb_init_leds();
void ntb_init_uid();
void ntb_deinit_spiexp();
void ntb_init_gpio();
void ntb_init_hw();
void ntb_led_on(led_t led);
void ntb_led_off(led_t led);
void ntb_led_toggle(led_t led);
void ntb_led_error();
void ntb_busywait_ms(int ms);

void ntb_ethswitch_off();
void ntb_ethswitch_on();


#endif // NTB_HWCTRL_H_