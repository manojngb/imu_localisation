#include "ntb_cmnds.h"
#include "ntb_hwctrl.h"

void ntb_processCmd(uint8_t *raw_cmd, uint8_t *stream_opts)
{
	ntb_cmnd_t *cmd = (ntb_cmnd_t*)raw_cmd;

	switch(cmd->sys)
	{
		case NTB_SYS_CTRL:
			// ----- Gen. control commands -----
			switch(cmd->cmd)
			{
				case NTB_CTRL_RST:
					// reset the ARM processor
					ntb_soft_reset();
					break;
				case NTB_CTRL_IAP:
					// ensure bootlock is enabled
					(*stream_opts) |= 0x20;
					break;
				case NTB_CTRL_APP:
					// boot into user application
					(*stream_opts) |= 0x40;
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_PCTRL:
			// ----- Power control commands -----
			switch(cmd->cmd)
			{
				case NTB_PCTRL_POWON:
					ntb_pctrl_on(cmd->opt);
					break;
				case NTB_PCTRL_POWOFF:
					ntb_pctrl_off(cmd->opt);
					break;
				case NTB_PCTRL_POWRST:
					ntb_pctrl_rst(cmd->opt);
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_PMEAS:
			// ----- Power meas. commands -----
			switch(cmd->cmd)
			{
				case NTB_PMEAS_STREAM_1HZ:
					break;
				case NTB_PMEAS_STREAM_10HZ:
					break;
				case NTB_PMEAS_STREAM_100HZ:
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_LED:
			// ----- LED control commands -----
			switch(cmd->cmd)
			{
				case NTB_LED_ON:
					ntb_led_on(cmd->opt);
					break;
				case NTB_LED_OFF:
					ntb_led_off(cmd->opt);
					break;
				case NTB_LED_TOGGLE:
					ntb_led_toggle(cmd->opt);
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_UART:
			// ----- UART control commands -----
			switch(cmd->cmd)
			{
				case NTB_UART_STREAM:
					break;
				case NTB_UART_BAUD:
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_UID:
			// ----- UID commands -----
			switch(cmd->cmd)
			{
				case NTB_UID_READ:
					(*stream_opts) |= 0x10;
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_GPIO:
			// ----- GPIO control commands -----
			switch(cmd->cmd)
			{
				case NTB_GPIO_ON:
					ntb_gpio_on(cmd->opt);
					break;
				case NTB_GPIO_OFF:
					ntb_gpio_off(cmd->opt);
					break;
				case NTB_GPIO_TOGGLE:
					ntb_gpio_toggle(cmd->opt);
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_RANGE:
			// ----- RANGE control commands -----

			//#define NTB_TCP_MAXCLIENTS (4)
			//#define NTB_TCPOPT_STREAM_RANGE 	(0x01)
			//#define NTB_TCPOPT_STREAM_POW_1 	(0x02)
			//#define NTB_TCPOPT_STREAM_POW_10	(0x04)
			//#define NTB_TCPOPT_STREAM_POW_100	(0x08)

			switch(cmd->cmd)
			{
				case NTB_RANGE_STARTSTREAM:
					(*stream_opts) |= 0x01;
					break;
				case NTB_RANGE_STOPSTREAM:
					(*stream_opts) &= ~(0x01);
					break;
				default:
					break;
			}
			break;
		case NTB_SYS_ETH:
			// ----- ETH control commands -----

			switch(cmd->cmd)
			{
				case NTB_ETH_RESET:
					ntb_ethswitch_off();
					ntb_busywait_ms(10);
					ntb_ethswitch_on();
					break;

				case NTB_ETH_LED:
					if( cmd->opt == '0' )
						ntb_ethswitch_led_off();
					else
						ntb_ethswitch_led_on();

				case NTB_ETH_PTPCFG1_LO:
					ntb_ethswitch_write8(ETHSWITCH_REG_PTP_MSG_CFG_1, cmd->opt);
					break;

				case NTB_ETH_PTPCFG2_LO:
					ntb_ethswitch_write8(ETHSWITCH_REG_PTP_MSG_CFG_2, cmd->opt);
					break;

				case NTB_ETH_PTPCFG2_HI:
					// ETHSWITCH_REG_PTP_MSG_CFG_2 = 0x622, so HI byte is 0x623
					ntb_ethswitch_write8(0x623, cmd->opt);
					break;

				case NTB_ETH_PTPCLK:
					ntb_ethswitch_write8(ETHSWITCH_REG_PTP_CLK_CTL, cmd->opt);
					break;

				default:
					break;
			}
		default:
			break;
	}
}