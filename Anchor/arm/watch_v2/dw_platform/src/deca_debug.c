#include "deca_debug.h"
#include "deca_ranging.h"
#include "ntb_hwctrl.h"
#include "stm32f4xx.h"
//#include "stm32f4_discovery.h"
#include "stm32f4xx_spi.h"

void dw_debug_handler(uint8_t event)
{
	switch( event )
	{
		case DWR_EVENT_ERROR:
			ntb_led_error();
			break;
		case DWR_EVENT_RXGOOD:
			//ntb_led_on(NTB_LED_ARM_RED);
			break;
		case DWR_EVENT_TXGOOD:
			//ntb_led_on(NTB_LED_ARM_GREEN);
			break;
		case DWR_EVENT_IRQ:
			//ntb_led_toggle(NTB_LED_ARM_RED);
			break;
		case DWR_EVENT_RXINIT:
			//ntb_led_toggle(LED_ARM_RED);
			break;
		case DWR_EVENT_RXFIN:
			break;
		case DWR_EVENT_UNKNOWN_IRQ:
			break;
		case DWR_EVENT_BADFRAME:
			break;
		case DWR_EVENT_RXSUM:
			//ntb_led_toggle(NTB_LED_BRIGHT_RED);
			break;
		default:
			break;
	}
}

