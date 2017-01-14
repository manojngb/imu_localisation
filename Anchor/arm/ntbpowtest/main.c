/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ntb_hwctrl.h"
#include "usb_vcp_api.h"
/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

int main(void)
{

  // initialize NTB hardware
  ntb_init_hw();

  // Turn on red led while initializing
  ntb_led_on(LED_ARM_RED);

  // todo: read UID value
  uint16_t uid = ntb_uid_read();

  // Reset ethernet switch
  ntb_ethswitch_off();
  ntb_busywait_ms(100);
  ntb_ethswitch_on();

  // init power measurement
  ntb_init_pmeas();

  // init USB VCP
  usb_vcp_init();

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(LED_ARM_RED);
  ntb_led_on(LED_ARM_BLUE);

   
  /* Infinite loop */
  float voltage, current, power;

  uint8_t c;

  while (1)
  {  
    ntb_busywait_ms(200);
    ntb_led_toggle(LED_ARM_BLUE);
    uid = ntb_uid_read();

    ntb_pmeas_read(POWER_5V0, &voltage, &current, &power);
    VCP_TxStrInt("UID: ", (int)uid);
    VCP_TxStrFloat("V: ", voltage);
    VCP_TxStrFloat("I: ", current);
    VCP_TxStrFloat("P: ", power);

    // USB configured OK, drivers OK
    if (usb_vcp_getstatus() == USB_VCP_CONNECTED) {
      // If something arrived at VCP
      if (usb_vcp_getc(&c) == TM_USB_VCP_DATA_OK) {
        // Return data back
        if(c == ' ')
          VCP_FlushMessages();
        if(c == 'p')
          ntb_pctrl_toggle( POWER_5V0 );
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

