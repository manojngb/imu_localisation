#ifndef NTB_CMNDS_H_
#define NTB_CMNDS_H_

#include "stm32f4xx.h"

// Subsystem definitions
#define NTB_SYS_PCTRL 	(0x01)
#define NTB_SYS_PMEAS 	(0x02)
#define NTB_SYS_LED		(0x03)
#define NTB_SYS_UART	(0x04)
#define NTB_SYS_UID		(0x05)
#define NTB_SYS_GPIO	(0x06)

// Power Control Commands
// PCTRL commands
#define NTB_PCTRL_POWON 	(0x01)
#define NTB_PCTRL_POWOFF 	(0x02)
#define NTB_PCTRL_POWRST	(0x03)
// PCTRL options
#define NTB_POW_1V8			(0x01)
#define NTB_POW_2V5			(0x02)
#define NTB_POW_3V3			(0x03)
#define NTB_POW_5V0			(0x04)

// Power Measurement Commands
// PMEAS commands
#define NTB_PMEAS_STREAM_1HZ	(0x01)
#define NTB_PMEAS_STREAM_10HZ	(0x02)
#define NTB_PMEAS_STREAM_100HZ	(0x03)
// PMEAS options - same as PCTRL

// LED Commands
#define NTB_LED_ON		(0x01)
#define NTB_LED_OFF		(0x02)
#define NTB_LED_TOGGLE	(0x03)
// LED options
#define NTB_LED_ARM_GREEN 	(0x01)
#define NTB_LED_ARM_RED		(0x02)
#define NTB_LED_ARM_BLUE	(0x03)

// GPIO Commands
#define NTB_GPIO_ON		(0x01)
#define NTB_GPIO_OFF	(0x02)
#define NTB_GPIO_TOGGLE	(0x03)
// GPIO Options
#define NTB_GPIO_0		(0x01)
#define NTB_GPIO_1		(0x02)
#define NTB_GPIO_2		(0x03)
#define NTB_GPIO_3		(0x04)
#define NTB_GPIO_DB_RST	(0x05)

// UART Commands
#define NTB_UART_STREAM	(0x01)
// UART Options
#define NTB_UART_DBGALL	(0x00)
#define NTB_UART_DBG_1	(0x01)
#define NTB_UART_DBG_2	(0x02)
// ...


// Command protocol
typedef struct
{
	uint8_t sys; // sub-system reference
	uint8_t cmd; // command
	uint8_t opt; // command option
} ntb_cmnd_t;


// Function declarations
void ntb_process_cmd(uint8_t *raw_cmd);



#endif // NTB_CMNDS_H_