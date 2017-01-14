#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f4xx.h"
#include "ntb_cmnds.h"

// NTB-specific hardware definitions
#define LED_ARM_PORT  (GPIOE)
#define LED_ARM_GREEN (GPIO_Pin_4)
#define LED_ARM_RED   (GPIO_Pin_5)
#define LED_ARM_BLUE  (GPIO_Pin_6)

#define UID_PORT (GPIOD)
#define UIDB4 (GPIO_Pin_14)
#define UIDB3 (GPIO_Pin_13)
#define UIDB2 (GPIO_Pin_12)
#define UIDB1 (GPIO_Pin_11)
#define UIDB0 (GPIO_Pin_10)

#define GPIO_PORT (GPIOD)
#define GPIO_P0  (GPIO_Pin_3)
#define GPIO_P1  (GPIO_Pin_4)
#define GPIO_P2  (GPIO_Pin_5)
#define GPIO_P3  (GPIO_Pin_6)

#define RST_PORT (GPIOE)
#define RST_PIN  (GPIO_Pin_9)

#define ETHSWITCH_PORT (GPIOD)
#define ETHSWITCH_RST  (GPIO_Pin_2)

#define PCTRL_PORT (GPIOE)
#define PCTRL_SHTDN_3V3 (GPIO_Pin_12)
#define PCTRL_SHTDN_5V0 (GPIO_Pin_13)

#define PMEAS_RSENSE_3V3 (0.240)
#define PMEAS_RSENSE_5V0 (0.040)
#define PMEAS_IC_ADDR    (0x4D)
#define PMEAS_REG_CONFIG (0x00)
#define PMEAS_REG_CONVRT (0x01)
#define PMEAS_REG_ONESHT (0x02)
#define PMEAS_REG_VS1CFG (0x0B)
#define PMEAS_REG_VS2CFG (0x0C)
#define PMEAS_REG_VS1HI  (0x0D)
#define PMEAS_REG_VS1LO  (0x0E)
#define PMEAS_REG_VS2HI  (0x0F)
#define PMEAS_REG_VS2LO  (0x10)
#define PMEAS_REG_V1HI   (0x11)
#define PMEAS_REG_V1LO   (0x12)
#define PMEAS_REG_V2HI   (0x13)
#define PMEAS_REG_V2LO   (0x14)
#define PMEAS_REG_PR1HI  (0x15)
#define PMEAS_REG_PR1LO  (0x16)
#define PMEAS_REG_PR2HI  (0x17)
#define PMEAS_REG_PR2LO  (0x18)
#define PMEAS_REG_PRODID (0xFD)


// Hardware function declarations
void ntb_init_leds();
void ntb_init_uid();
void ntb_deinit_spiexp();
void ntb_init_gpio();
void ntb_init_hw();

uint16_t ntb_uid_read();

void ntb_led_on(uint8_t led);
void ntb_led_off(uint8_t led);
void ntb_led_toggle(uint8_t led);
void ntb_led_error();

void ntb_gpio_on(uint8_t pin);
void ntb_gpio_off(uint8_t pin);
void ntb_gpio_toggle(uint8_t pin);


void ntb_pctrl_on(uint8_t rail);
void ntb_pctrl_off(uint8_t rail);
void ntb_pctrl_toggle(uint8_t rail);
void ntb_pctrl_rst(uint8_t rail);

void ntb_init_pmeas();
void ntb_pmeas_i2c_start(uint8_t address, uint8_t direction);
void ntb_pmeas_i2c_repstart(uint8_t address, uint8_t direction);
void ntb_pmeas_i2c_write(uint8_t data);
uint8_t ntb_pmeas_i2c_readack();
uint8_t ntb_pmeas_i2c_readnack();
void ntb_pmeas_i2c_stop();
void ntb_pmeas_writereg(uint8_t reg, uint8_t *byte, int len);
void ntb_pmeas_readreg(uint8_t reg, uint8_t *buf, int len);
void ntb_pmeas_read(uint8_t rail, float *v, float *i, float *p);
void ntb_soft_reset();

void ntb_ethswitch_off();
void ntb_ethswitch_on();

void ntb_busywait_ms(int ms);


#endif // NTB_HWCTRL_H_