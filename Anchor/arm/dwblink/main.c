#include "main.h"



int main(void)
{
  // initialize the led GPIO
	leds_init();
  // initialize the USB
  usb_vcp_init();
  // initialize DecaWave
  dw_init();

  uint8_t c;

  for(;;) 
  {
  	Sleep(500);
    led_toggle(LED_BLUE);
    // Transmit packet
    dw_sendbeacon();

    // Receive packets
    //dw_listen();

    // read DW part id
    uint32_t pid = dwt_readdevid();
    VCP_TxStrLong("DW PID: ", pid);

    // read DW eui
    uint8_t eui[8];
    dwt_geteui(eui);
    VCP_TxStrBytes("EUI: ", eui, 8);

    // USB configured OK, drivers OK
    if (usb_vcp_getstatus() == USB_VCP_CONNECTED) {
      // If something arrived at VCP
      if (usb_vcp_getc(&c) == TM_USB_VCP_DATA_OK) {
        // Return data back
        if(c == ' ')
          VCP_FlushMessages();
      }
    }
    
  }
  return 0;
}
