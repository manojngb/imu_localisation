#include "main.h"
#include "deca_spi.h"
#include "deca_debug.h"
#include "deca_ranging.h"
#include "deca_rng.h"
#include "leds.h"
#include "usb_vcp_api.h"

#define DWRANGE_POLL_PERIOD_MS (100)

void dw_range_complete(DWR_RangeEst_t *range)
{
  leds_toggle(LED_BLUE);
  VCP_TxStrInt(" ", (int)(range->rangeEst));
}

int main(void)
{
  // initialize the led hardware
	leds_init();
  // initialize the USB virtual comm.
  usb_vcp_init();
  // initialize DecaWave SPI
  dw_spi_init();
  // initialize DecaWave RNG
  dw_rng_init();
  // initialize DecaWave ranging
  int dw_status;
  dw_status = deca_ranging_init(
                  0xAE70,           // PAN address (constant for all nodes)
                  0x0001,           // SHORT address (unique for all nodes)
                  NODE_TYPE_MOBILE,  // Node Type (fixed, mobile, or mobile responsive)
                  dw_range_complete,// Ranging complete handler
                  dw_spi_sendpacket,// SPI send packet 
                  dw_spi_readpacket,// SPI read packet 
                  dw_debug_handler, // Handle ranging events
                  dw_sleep_msec,    // sleep in milliseconds
                  dw_rng_float      // random number generator (float)
                );

  if( dw_status == DWR_RETURN_ERR ) leds_error();

  uint8_t char_usb;

  for(;;) 
  {
  	dw_sleep_msec(DWRANGE_POLL_PERIOD_MS);
    deca_ranging_poll(DWRANGE_POLL_PERIOD_MS);
    leds_toggle(LED_BLUE);

    // USB configured OK, drivers OK
    if (usb_vcp_getstatus() == USB_VCP_CONNECTED) {
      // If something arrived at VCP
      if (usb_vcp_getc(&char_usb) == TM_USB_VCP_DATA_OK) {
        // Return data back
        if(char_usb == ' ')
          VCP_FlushMessages();
      }
    }
    
  }
  return 0;
}
