/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "ntb_tcpserver.h"
#include "ntb_hwctrl.h"
#include "lwip/tcp.h"
#include "deca_ranging.h"
#include "deca_debug.h"
#include "deca_rng.h"
#include "deca_spi.h"
#include "usb_vcp_api.h"

#define SYSTEMTICK_PERIOD_MS  (10)
#define DWRANGE_POLL_PERIOD_MS (500)
#define MAX_TCP_CLIENTS (4)

// Timing variables
volatile uint32_t localtime_ms = 0;
uint32_t last_1hz_time = 0;
uint32_t last_10hz_time = 0;
uint32_t last_100hz_time = 0;
uint32_t last_poll = 0;

// Local variables
uint16_t local_uid = 0;


void dw_range_complete(DWR_RangeEst_t *range)
{
  // ignore improbable ranges
  if( range->rangeEst < 0 || range->rangeEst > 5000)
    return;

  // construct ascii response
  uint8_t uid_ascii[16];
  uint8_t range_ascii[16];
  uint8_t addr_ascii[16];
  uint8_t msg_ascii[64];
  int msglen = 0;
  int i;

  int uid_len   = num2str(local_uid, uid_ascii, 10);
  int range_len = num2str(range->rangeEst, range_ascii, 10);
  int addr_len  = num2str(range->nodeAddr[0], addr_ascii, 10);

  // add local UID
  for(i = 0; i < uid_len; i++)
  {
  	msg_ascii[msglen++] = uid_ascii[i++];
  }

  // type '0' = range estimate
  msg_ascii[msglen++] = ',';
  msg_ascii[msglen++] = '0';
  msg_ascii[msglen++] = ',';

  // add remote node address
  for(i = 0; i < addr_len; i++)
  {
  	msg_ascii[msglen++] = addr_ascii[i++];
  }

  // add range estimate
  msg_ascii[msglen++] = ',';
  for(i = 0; i < range_len; i++)
  {
  	msg_ascii[msglen++] = range_ascii[i];
  }

  // final null and newline
  msg_ascii[msglen++] = '\n';

  // send to all subscribed tcp clients
  ntb_bcast_tcp(msg_ascii, msglen, NTB_TCPOPT_STREAM_RANGE );
}

void tcp_cmd_handler(uint8_t* raw_cmd, int len, ntb_tcpclient_t* clnt)
{
  // command must have at least 3 bytes, but 1 more for \n is ok.
  if( len < sizeof(ntb_cmnd_t))
    return;

  // handle raw command and store options on client struct
  ntb_processCmd(raw_cmd, &(clnt->options));

  // handle query requests
  if( clnt->options & NTB_TCPOPT_QUERY_UID)
  {
    // clear flag
    clnt->options &= ~(NTB_TCPOPT_QUERY_UID);

    // send UID in ASCII
    uint8_t uid_ascii[32];
    int len = num2str(local_uid, uid_ascii, 10);
    uid_ascii[len++] = '\n';
    ntb_ucast_tcp(uid_ascii, len, clnt);
  }
}


int main(void)
{

  // initialize NTB hardware
  ntb_init_hw();

  // Turn on red led while initializing
  ntb_led_on(NTB_LED_ARM_RED);

  // Read UID value
  local_uid = ntb_uid_read();

  // Reset ethernet switch
  ntb_ethswitch_off();
  ntb_busywait_ms(100);
  ntb_ethswitch_on();
  ntb_busywait_ms(1000);

  // init power measurement
  //ntb_init_pmeas();

  // init USB VCP
  usb_vcp_init();

  dw_spi_init();
  // initialize DecaWave RNG
  dw_rng_init();
  // initialize DecaWave ranging
  int dw_status;
  dw_status = decaranging_init(
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

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(NTB_LED_ARM_RED);
  ntb_led_on(NTB_LED_ARM_BLUE);

  // configure ethernet (GPIOs, clocks, MAC, DMA)
  ETH_BSP_Config();
    
  // Initilaize the LwIP stack
  LwIP_Init();
  
  // tcp echo server Init
  ntb_tcpserver_init();
  ntb_tcpserver_setcallback(tcp_cmd_handler);
   
  float voltage, current, power;

  uint8_t c;
  uint32_t id;

  while (1)
  {  
    // check if any packet received 
    if (ETH_CheckFrameReceived()) { 
      // process received ethernet packet
      LwIP_Pkt_Handle();
    }
    // handle periodic timers for LwIP
    LwIP_Periodic_Handle(localtime_ms);

    // ----- 1 Hz Loop -----
    if( localtime_ms - last_1hz_time >= 1000 )
    {
      last_1hz_time = localtime_ms;
      //ntb_tcpserver_sendToClients();
    }

    // ----- 10 Hz Loop -----
    if( localtime_ms - last_10hz_time >= 100 )
    {
      last_10hz_time = localtime_ms;
    }

    // ----- 100 Hz Loop -----
    if( localtime_ms - last_100hz_time >= 10 )
    {
      last_100hz_time = localtime_ms;
    }



    // periodic DW Range poll
    if(localtime_ms - last_poll >= DWRANGE_POLL_PERIOD_MS)
    {
      last_poll = localtime_ms;
      decaranging_poll(DWRANGE_POLL_PERIOD_MS);
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

