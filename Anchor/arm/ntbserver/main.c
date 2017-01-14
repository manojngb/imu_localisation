/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "ntb_tcpserver.h"
#include "ntb_hwctrl.h"
#include "lwip/tcp.h"

/* Private define ------------------------------------------------------------*/
#define SYSTEMTICK_PERIOD_MS  10

/* Private variables ---------------------------------------------------------*/
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */
uint32_t timingdelay;

extern struct ntb_tcpserver_struct *stream_es;
extern uint8_t streaming_enabled;
struct pbuf pbuf_stream;

int main(void)
{

  // initialize NTB hardware
  ntb_init_hw();

  // Turn on red led while initializing
  ntb_led_on(LED_ARM_RED);

  // todo: read UID value

  // Reset ethernet switch
  ntb_ethswitch_off();
  ntb_busywait_ms(100);
  ntb_ethswitch_on();
  ntb_busywait_ms(7000);

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(LED_ARM_RED);
  ntb_led_on(LED_ARM_BLUE);

  /*!< At this stage the microcontroller clock setting is already configured to 
       144 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */

  /* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
  ETH_BSP_Config();
    
  /* Initilaize the LwIP stack */
  LwIP_Init();
  
  /* tcp echo server Init */
  ntb_tcpserver_init();

  // Ramp on
  ntb_busywait_ms(150);
  ntb_led_on(LED_ARM_GREEN);
  ntb_busywait_ms(150);
  ntb_led_on(LED_ARM_RED);
  ntb_busywait_ms(150);
  ntb_led_on(LED_ARM_BLUE);
  ntb_busywait_ms(150);
  GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_9);
  ntb_busywait_ms(150);
  GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_10);
  ntb_busywait_ms(150);
  GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_11);
  ntb_busywait_ms(150);
  GPIO_SetBits(LED_ARM_PORT, GPIO_Pin_12);
   
  /* Infinite loop */
  while (1)
  {  

    /* check if any packet received */
    if (ETH_CheckFrameReceived()) { 
      /* process received ethernet packet */
      LwIP_Pkt_Handle();
    }
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(LocalTime);

    if( streaming_enabled == 1)
    {
      pbuf_stream.payload="we be streaming\n";
      pbuf_stream.len=17;
      stream_es->p = &pbuf_stream;
      ntb_tcpserver_send(stream_es->pcb, stream_es);
      Delay(10);
	  GPIO_ToggleBits(LED_ARM_PORT, GPIO_Pin_9);
	  GPIO_ToggleBits(LED_ARM_PORT, GPIO_Pin_10);
	  GPIO_ToggleBits(LED_ARM_PORT, GPIO_Pin_11);
	  GPIO_ToggleBits(LED_ARM_PORT, GPIO_Pin_12);
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

