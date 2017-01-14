#ifndef NTB_CMNDS_H_
#define NTB_CMNDS_H_

#include "stm32f4xx.h"

// Subsystem definitions
#define NTB_SYS_CTRL 	('C')
#define NTB_SYS_PCTRL 	('P')
#define NTB_SYS_PMEAS 	('M')
#define NTB_SYS_LED		('L')
#define NTB_SYS_UART	('U')
#define NTB_SYS_UID		('I')
#define NTB_SYS_GPIO	('G')
#define NTB_SYS_RANGE	('R')
#define NTB_SYS_ETH		('E')

// System Control Commands
#define NTB_CTRL_RST 	('r')
#define NTB_CTRL_IAP	('i')
#define NTB_CTRL_APP	('a')

// Power Control Commands
// PCTRL commands
#define NTB_PCTRL_POWOFF 	('0')
#define NTB_PCTRL_POWON 	('1')
#define NTB_PCTRL_POWRST	('2')
// PCTRL options
#define NTB_POW_3V3			('3')
#define NTB_POW_5V0			('5')

// Power Measurement Commands
// PMEAS commands
#define NTB_PMEAS_STREAM_1HZ	('1')
#define NTB_PMEAS_STREAM_10HZ	('2')
#define NTB_PMEAS_STREAM_100HZ	('3')
// PMEAS options - same as PCTRL

// LED Commands
#define NTB_LED_OFF		('0')
#define NTB_LED_ON		('1')
#define NTB_LED_TOGGLE	('t')

// LED options
#define NTB_LED_ARM_GREEN 	('g')
#define NTB_LED_ARM_RED		('r')
#define NTB_LED_ARM_BLUE	('b')
#define NTB_LED_BRIGHT_GREEN  ('G')
#define NTB_LED_BRIGHT_RED    ('R')
#define NTB_LED_BRIGHT_BLUE   ('B')
#define NTB_LED_BRIGHT_ORANGE ('O')

// UID Commands
#define NTB_UID_READ	('r')

// GPIO Commands
#define NTB_GPIO_OFF	('0')
#define NTB_GPIO_ON		('1')
#define NTB_GPIO_TOGGLE	('t')

// GPIO Options
#define NTB_GPIO_0		('0')
#define NTB_GPIO_1		('1')
#define NTB_GPIO_2		('2')
#define NTB_GPIO_3		('3')
#define NTB_GPIO_RST	('r')

// UART Commands
#define NTB_UART_STREAM	('s')
#define NTB_UART_BAUD	('b')
// UART Options
#define NTB_UART_DBGALL	('a')
#define NTB_UART_DBG_1	('1')
#define NTB_UART_DBG_2	('2')

// RANGE Commands
#define NTB_RANGE_STARTSTREAM ('s')
#define NTB_RANGE_STOPSTREAM  ('e')

// ETH Switch Commands
#define NTB_ETH_RESET 		('r')
#define NTB_ETH_LED			('l')
#define NTB_ETH_PTPCFG1_LO	('1')
#define NTB_ETH_PTPCFG2_LO	('2')
#define NTB_ETH_PTPCFG2_HI	('3')
#define NTB_ETH_PTPCLK		('c')


// Command protocol
typedef struct
{
	uint8_t sys; // sub-system reference
	uint8_t cmd; // command
	uint8_t opt; // command option
} ntb_cmnd_t;


// ===== Public function declarations =====
void ntb_processCmd(uint8_t *raw_cmd, uint8_t *stream_opts);


#endif // NTB_CMNDS_H_