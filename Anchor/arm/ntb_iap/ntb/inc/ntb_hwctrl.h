#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f4xx.h"
#include "ntb_cmnds.h"

// NTB-specific hardware definitions
#define LED_ARM_PORT  (GPIOE)
#define LED_ARM_GREEN (GPIO_Pin_4)
#define LED_ARM_RED   (GPIO_Pin_5)
#define LED_ARM_BLUE  (GPIO_Pin_6)
#define LED_BRIGHT_PORT   (GPIOE)
#define LED_BRIGHT_GREEN  (GPIO_Pin_9)
#define LED_BRIGHT_RED    (GPIO_Pin_10)
#define LED_BRIGHT_BLUE   (GPIO_Pin_11)
#define LED_BRIGHT_ORANGE (GPIO_Pin_12)

#define UID_PORT (GPIOD)
#define UIDB4 (GPIO_Pin_10)
#define UIDB3 (GPIO_Pin_11)
#define UIDB2 (GPIO_Pin_12)
#define UIDB1 (GPIO_Pin_13)
#define UIDB0 (GPIO_Pin_14)

#define GPIO_PORT (GPIOD)
#define GPIO_P0  (GPIO_Pin_3)
#define GPIO_P1  (GPIO_Pin_4)
#define GPIO_P2  (GPIO_Pin_5)
#define GPIO_P3  (GPIO_Pin_6)

#define RST_PORT (GPIOE)
#define RST_PIN  (GPIO_Pin_13)

#define ETHSWITCH_PORT (GPIOD)
#define ETHSWITCH_RST  (GPIO_Pin_2)
#define ETHSWITCH_CS   (GPIO_Pin_0)
#define ETHSWITCH_BYTE0 (0x01)
#define ETHSWITCH_BYTE1 (0x02)
#define ETHSWITCH_BYTE2 (0x04)
#define ETHSWITCH_BYTE3 (0x08)
#define ETHSWITCH_REG_PTP_MSG_CFG_1 (0x620)
#define ETHSWITCH_REG_PTP_MSG_CFG_2 (0x622)
#define ETHSWITCH_REG_GPIO_OEN (0x682)
#define ETHSWITCH_REG_TRIG_EN (0x206)
#define ETHSWITCH_REG_TRIG1_CFG_1 (0x228)
#define ETHSWITCH_REG_TRIG1_CFG_2 (0x22A)
#define ETHSWITCH_REG_TRIG1_TGT_SH (0x226)
#define ETHSWITCH_REG_TRIG1_TGT_SL (0x224)
#define ETHSWITCH_REG_PTP_CLK_CTL  (0x600)

#define ETHSWITCH_GPIO_LED (3)


#define PCTRL_PORT (GPIOE)
#define PCTRL_SHTDN_3V3 (GPIO_Pin_14)
#define PCTRL_SHTDN_5V0 (GPIO_Pin_15)

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

// ethernet switch
uint8_t ntb_ethswitch_trx(uint8_t byte);
void ntb_ethswitch_off();
void ntb_ethswitch_on();
void ntb_ethswitch_set_tclk();
void ntb_ethswitch_write(uint16_t address, uint8_t bytemask, uint8_t *data);
void ntb_ethswitch_read(uint16_t address, uint8_t bytemask, uint8_t *data);
void ntb_ethswitch_write8(uint16_t reg_addr, uint8_t data);
void ntb_ethswitch_write16(uint16_t reg_addr, uint16_t data);
uint16_t ntb_ethswitch_read16(uint16_t reg_addr);
uint8_t ntb_ethswitch_read8(uint16_t reg_addr);
void ntb_ethswitch_led_on();
void ntb_ethswitch_led_off();


void ntb_busywait_ms(int ms);


#endif // NTB_HWCTRL_H_