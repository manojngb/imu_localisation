#include "deca_debug.h"
#include "deca_ranging.h"
#include "ntb_hwctrl.h"
#include "stm32f30x.h"

// handle debugging events from the decaranging lib
void dw_debug_handler(uint8_t event)
{
	switch( event )
	{
		case DWR_EVENT_ERROR:
			break;
		case DWR_EVENT_RXGOOD:
			break;
		case DWR_EVENT_TXGOOD:
			break;
		case DWR_EVENT_IRQ:
			break;
		case DWR_EVENT_RXINIT:
			break;
		case DWR_EVENT_RXFIN:
			break;
		case DWR_EVENT_UNKNOWN_IRQ:
			break;
		case DWR_EVENT_BADFRAME:
			break;
		case DWR_EVENT_RXSUM:
			break;
		default:
			break;
	}
}

