/*
* main.h
*
*  Created on: 10 jul 2012
*      Author: BenjaminVe
*/

#ifndef LEDS_H_
#define LEDS_H_

// Includes	
#include <stdbool.h>
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "stm32f4_discovery.h"



// Function prototypes
void leds_init();
void leds_set(Led_TypeDef led);
void leds_clear(Led_TypeDef led);
void leds_toggle(Led_TypeDef led);
void leds_error();

#endif /* LEDS_H_ */
