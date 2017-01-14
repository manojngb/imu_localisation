/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ntb_hwctrl.h"
#include "usb_vcp_api.h"
#include "deca_ranging.h"
#include "deca_debug.h"
#include "deca_rng.h"
#include "deca_spi.h"

/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

#define DWRANGE_POLL_PERIOD_MS (500)

/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;


void dw_range_complete(DWR_RangeEst_t *range)
{
  ntb_led_toggle(LED_ARM_BLUE);
  VCP_TxStrInt(" ", (int)(range->rangeEst));
}



int main(void)
{

  // initialize NTB hardware
  ntb_init_hw();

  // Turn on red led while initializing
  ntb_led_on(LED_ARM_RED);

  // Read UID value
  uint16_t uid = ntb_uid_read();

  // Reset ethernet switch
  ntb_ethswitch_off();
  ntb_busywait_ms(100);
  ntb_ethswitch_on();

  // init power measurement
  //ntb_init_pmeas();

  // init USB VCP
  usb_vcp_init();

  dw_spi_init();
  // initialize DecaWave RNG
  dw_rng_init();
  // initialize DecaWave ranging
  int dw_status;
  dw_status = deca_ranging_init(
                  0xAE70,           // PAN address (constant for all nodes)
                  0x0002,           // SHORT address (unique for all nodes)
                  NODE_TYPE_FIXED,  // Node Type (fixed, mobile, or mobile responsive)
                  dw_range_complete,// Ranging complete handler
                  dw_spi_sendpacket,// SPI send packet 
                  dw_spi_readpacket,// SPI read packet 
                  dw_debug_handler, // Handle ranging events
                  dw_sleep_msec,    // sleep in milliseconds
                  dw_rng_float      // random number generator (float)
                );

  if( dw_status == DWR_RETURN_ERR ) ntb_led_error();

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(LED_ARM_RED);
  ntb_led_on(LED_ARM_BLUE);
   
  /* Infinite loop */
  float voltage, current, power;

  uint8_t c;
  uint32_t id;

  while (1)
  {  
    dw_sleep_msec(DWRANGE_POLL_PERIOD_MS);
    deca_ranging_poll(DWRANGE_POLL_PERIOD_MS);
    ntb_led_toggle(LED_ARM_BLUE);

    uid = ntb_uid_read();

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
}


void Delay(uint32_t nCount)
{
  /* Capture the current local time */
  timingdelay = LocalTime + nCount;  

  /* wait until the desired delay finish */  
  while (timingdelay > LocalTime);
}

void Time_Update(void)
{
  LocalTime += SYSTEMTICK_PERIOD_MS;
}


#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

