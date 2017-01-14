#include "deca_debug.h"
#include "deca_ranging.h"
#include "leds.h"
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"
#include "stm32f4xx_spi.h"

void dw_debug_handler(uint8_t event)
{
	switch( event )
	{
		case DWRANGE_EVENT_ERROR:
			leds_error();
			break;
		case DWRANGE_EVENT_RXGOOD:
			break;
		case DWRANGE_EVENT_TXGOOD:
			leds_toggle(LED_GREEN);
			break;
		case DWRANGE_EVENT_IRQ:
			break;
		case DWRANGE_EVENT_RXINIT:
			leds_toggle(LED_RED);
			break;
		case DWRANGE_EVENT_RXFIN:
			leds_toggle(LED_ORANGE);
			break;
		case DWRANGE_EVENT_UNKNOWN_IRQ:
			break;
		case DWRANGE_EVENT_BADFRAME:
			break;
		default:
			break;
	}
}

