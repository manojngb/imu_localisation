/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "ntb_tcpserver.h"
#include "ntb_hwctrl.h"
#include "lwip/tcp.h"
#include "tftpserver.h"

#define SYSTEMTICK_PERIOD_MS  (10)
#define MAX_TCP_CLIENTS (4)
#define BOOTUP_TIME_MS (20000)
typedef  void (*pFunction)(void);

// Timing variables
volatile uint32_t localtime_ms = 0;

// Booting into application
volatile uint8_t bootlock = 0;
pFunction Jump_To_Application;
uint32_t JumpAddress;

// Local variables
uint16_t local_uid = 0;

// last byte of MAC addr (expected by LwIP as #define)
// be sure to assign before initializing LwIP!
uint8_t MAC_ADDR5 = 0;

void tcp_cmd_handler(uint8_t* raw_cmd, int len, ntb_tcpclient_t* clnt)
{
  // command must have at least 3 bytes, but 1 more for \n is ok.
  if( len < sizeof(ntb_cmnd_t))
    return;

  // handle raw command and store options on client struct
  ntb_processCmd(raw_cmd, &(clnt->options));

  // handle query requests
  if( clnt->options & NTB_TCPOPT_GOTO_IAP )
  {
    // we're already in IAP, just set the bootlock
    bootlock = 1;
  }

  if( clnt->options & NTB_TCPOPT_GOTO_APP )
  {
    ntb_led_off(NTB_LED_BRIGHT_GREEN);
    ntb_busywait_ms(300);
    ntb_led_off(NTB_LED_BRIGHT_RED);
    ntb_busywait_ms(300);
    ntb_led_off(NTB_LED_BRIGHT_BLUE);
    ntb_busywait_ms(300);
    ntb_led_off(NTB_LED_BRIGHT_ORANGE);
    ntb_busywait_ms(300);

    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
    Jump_To_Application();
  }
}

int main(void)
{
  // initialize NTB hardware
  ntb_init_hw();

  // IAP LED sequence
  ntb_led_on(NTB_LED_BRIGHT_GREEN);
  ntb_busywait_ms(300);
  ntb_led_on(NTB_LED_BRIGHT_RED);
  ntb_busywait_ms(300);
  ntb_led_on(NTB_LED_BRIGHT_BLUE);
  ntb_busywait_ms(300);
  ntb_led_on(NTB_LED_BRIGHT_ORANGE);
  ntb_busywait_ms(300);

  // Read UID value and set mac accordingly
  local_uid = ntb_uid_read();
  MAC_ADDR5 = local_uid;

  // Reset ethernet switch
  ntb_ethswitch_off();
  ntb_busywait_ms(10);
  ntb_ethswitch_on();
  ntb_busywait_ms(100);

  // configure ethernet (GPIOs, clocks, MAC, DMA)
  ETH_BSP_Config();
    
  // Initilaize the LwIP stack
  LwIP_Init();
  
  // tcp command server init
  ntb_tcpserver_init(local_uid);
  ntb_tcpserver_setcallback(tcp_cmd_handler);

  // IAP tftp server init
  IAP_tftpd_init();

  while (1)
  {  
    // check if any packet received 
    if (ETH_CheckFrameReceived()) { 
      // process received ethernet packet
      LwIP_Pkt_Handle();
    }
    // handle periodic timers for LwIP
    LwIP_Periodic_Handle(localtime_ms);

    // after enough time, boot into application
    if (localtime_ms > BOOTUP_TIME_MS && bootlock == 0)
    {
      // Power down the LEDS to indicate we're about to boot
      ntb_led_off(NTB_LED_BRIGHT_GREEN);
      ntb_busywait_ms(300);
      ntb_led_off(NTB_LED_BRIGHT_RED);
      ntb_busywait_ms(300);
      ntb_led_off(NTB_LED_BRIGHT_BLUE);
      ntb_busywait_ms(300);
      ntb_led_off(NTB_LED_BRIGHT_ORANGE);
      ntb_busywait_ms(300);

      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS + 4);
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
      Jump_To_Application();
    }

  }   
}

void Time_Update(void)
{
  localtime_ms += SYSTEMTICK_PERIOD_MS;
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

