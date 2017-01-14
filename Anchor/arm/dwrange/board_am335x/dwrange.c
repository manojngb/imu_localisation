#include "deca_spi.h"
#include "deca_debug.h"
#include "deca_ranging.h"
#include "deca_rng.h"
#include "leds.h"

#include <stdio.h>


void dw_range_complete(DWR_RangeEst_t *range)
{
  leds_toggle(LED_BLUE);
  printf("Estimated range: %u\n", range->rangeEst);
}

int main(void)
{
  // initialize the led hardware
	leds_init();

  // initialize DecaWave SPI
  dw_spi_init();
  
  // initialize DecaWave RNG
  dw_rng_init();
 
  // initialize DecaWave ranging
  int dw_status;
  dw_status = deca_ranging_init(
                  0xAE70,            // PAN address (constant for all nodes)
                  0x0002,            // SHORT address (unique for all nodes)
                  NODE_TYPE_FIXED,  // Node Type (fixed, mobile, or mobile responsive)
                  dw_range_complete, // Ranging complete handler
                  dw_spi_sendpacket, // SPI send packet 
                  dw_spi_readpacket, // SPI read packet 
                  dw_debug_handler,  // Handle ranging events
                  dw_sleep_msec,     // sleep in milliseconds
                  dw_rng_float       // random number generator (float)
                );

  // If there is an error
  if( dw_status == DWR_RETURN_ERR ) 
  {
    printf("Error\n");
    leds_error();
  }

  uint8_t char_usb; 
  for(;;) 
  {
    /* Polling block until IRQ is returned */
    if (dw_spi_poll())
    {
      deca_ranging_isr();
      //dwt_rxenable(0);
      printf("IRQ\n");
    }
  }
  
  return 0;
}
