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
#include "lwio.h"

#define SYSTEMTICK_PERIOD_MS  (10)
#define DWRANGE_POLL_PERIOD_MS (500)
#define MAX_TCP_CLIENTS (4)
#define MAX_DITHER_MS (10)
//Dither was 40
// TWR broadcasting sequencing
// uid = index, trigger = element
#define MAX_TWR_NODES (18)

// Timing variables
volatile uint32_t localtime_ms = 0;
uint32_t last_1hz_time = 0;
uint32_t last_10hz_time = 0;
uint32_t last_100hz_time = 0;
// Local node identifier
uint16_t local_uid = 0;
// Operating mode
uint16_t operating_mode = 0;
// Local power measurement variables
float voltage_3v3 = 0;
float current_3v3 = 0;
float power_3v3 = 0;
float voltage_5v0 = 0;
float current_5v0 = 0;
float power_5v0 = 0;
// TCP client streaming options (OR'ed)
uint8_t tcp_streamopts = 0;
// last byte of MAC addr (expected by LwIP as #define)
// be sure to assign before initializing LwIP!
uint8_t MAC_ADDR5 = 0;
// local DW1000 beacon and twr rate
uint16_t beacon_delay_ms = 0;
uint32_t last_beacon_time = 0;
uint16_t twr_delay_ms = 0;
uint32_t last_twr_time = 0;
int random_dither = 0;
uint8_t twr_queued = 0;

// ===== Public Functions (main.h) ====
void SetBeaconDelay(uint16_t delay_ms)
{
  beacon_delay_ms = delay_ms;
}

void SetTwrDelay(uint16_t delay_ms)
{
  twr_delay_ms = delay_ms;
}

void sendTwrOneShot()
{
  decaranging_sendtwr();
}


// this is called when a new inbound TCP command arrives
void tcp_cmd_handler(uint8_t* raw_cmd, int len, ntb_tcpclient_t* clnt)
{
  // command must have at least 3 bytes, but 1 more for \n is ok.
  if( len < sizeof(ntb_cmnd_t))
    return;

  // handle raw command and store options on client struct
  ntb_processCmd(raw_cmd, &(clnt->options));


  // handle specific query requests
  if( clnt->options & NTB_TCPOPT_QUERY_UID)
  {
    // clear flag
    clnt->options &= ~(NTB_TCPOPT_QUERY_UID);

    // send UID in ASCII
    uint8_t uid_ascii[32];
    int len = int2str(local_uid, uid_ascii, 10);
    uid_ascii[len++] = '\n';
    ntb_ucast_tcp(uid_ascii, len, clnt);
  }

  // update local copy of "all" streaming options (OR of all clients)
  tcp_streamopts =  ntb_tcp_allstreamopts();
}

void dw_range_complete(DWR_TwrTiming_t *timeStats)
{
  // construct ascii response
  uint8_t msg_ascii[128];
  int msglen = 0;
  uint64_t msg[14];
  msg[0] = timeStats->dstAddr;
  // msg type = 0
  msg[1] = 0;
  // node source address
  msg[2] = timeStats->srcAddr;
  // range sequence
  msg[3] = timeStats->seq;
  // timestamps 1 -> 6
  msg[4] = timeStats->tstamp1;
  msg[5] = timeStats->tstamp2;
  msg[6] = timeStats->tstamp3;
  msg[7] = timeStats->tstamp4;
  msg[8] = timeStats->tstamp5;
  msg[9] = timeStats->tstamp6;
  // rx quality numbers
  msg[10] = -100*timeStats->fppwr;
  msg[11] = timeStats->cirp;
  msg[12] = -100*timeStats->fploss;
  msg[13] = (uint64_t)(decaranging_getovrf());
  // convert to ascii CSV list
  msglen = uintlist2str(msg_ascii, msg, 14);

  // blink the bright blue LED to indicate a range complete measurement
  ntb_led_toggle(NTB_LED_BRIGHT_ORANGE);

  // send to all subscribed tcp clients
  ntb_bcast_tcp(msg_ascii, msglen, NTB_TCPOPT_STREAM_RANGE );


}

/*
// TODO: this is currently called from an ISR--it should be put in an outgoing
// queue and serviced opportunistically.
void dw_range_complete(DWR_RangeEst_t *range)
{
  // ignore improbable ranges
  if( range->rangeEst < 0 || range->rangeEst > 5000)
    return;

  // construct ascii response
  uint8_t msg_ascii[64];
  int msglen = 0;
  int msg[5];
  msg[0] = local_uid;
  // range type = 0
  msg[1] = 0;
  msg[2] = range->nodeAddr;
  msg[3] = (int)(range->rangeEst);
  msg[4] = (int)(range->pathLoss*100);
  msglen = intlist2str(msg_ascii, msg, 5);

  // send to all subscribed tcp clients
  ntb_bcast_tcp(msg_ascii, msglen, NTB_TCPOPT_STREAM_RANGE );

  // blink the bright blue LED to indicate a range complete measurement
  ntb_led_toggle(NTB_LED_BRIGHT_ORANGE);
}
*/

void dw_beacon_received(DWR_Beacon_t *beacon)
{
  // construct ascii response
  uint8_t msg_ascii[64];
  int msglen = 0;
  int msg[17];
  msg[0] = local_uid;
  // beacon type = 1
  msg[1] = 1;
  // source
  msg[2] = beacon->nodeAddr;
  // range sequence (1 byte)
  msg[3] = beacon->seq;
  // tx bytes (x5)
  msg[4] = (beacon->txstamp>>32) & 0xFF;
  msg[5] = (beacon->txstamp>>24) & 0xFF;
  msg[6] = (beacon->txstamp>>16) & 0xFF;
  msg[7] = (beacon->txstamp>>8) & 0xFF;
  msg[8] = (beacon->txstamp>>0) & 0xFF;
  // rx bytes (x5)
  msg[9] = (beacon->rxstamp>>32) & 0xFF;
  msg[10] = (beacon->rxstamp>>24) & 0xFF;
  msg[11] = (beacon->rxstamp>>16) & 0xFF;
  msg[12] = (beacon->rxstamp>>8) & 0xFF;
  msg[13] = (beacon->rxstamp>>0) & 0xFF;
  // fppwr
  msg[14] = (int)(beacon->fppwr*100);
  // cirp
  msg[15] = (int)(beacon->cirp);
  // fploss
  msg[16] = (int)(beacon->fploss*100);

  msglen = intlist2str(msg_ascii, msg, 17);

  // send to all subscribed tcp clients
  ntb_bcast_tcp(msg_ascii, msglen, NTB_TCPOPT_STREAM_RANGE );

  // blink the bright red LED to indicate a beacon has been received
  ntb_led_toggle(NTB_LED_BRIGHT_ORANGE);
}

// send voltage, current, and power for 3v3 and 5v0 lines over TCP
void tcp_send_power()
{
  // construct ascii messages
  uint8_t msg_ascii[128];
  int msglen = 0;
  int msg[7];
  msg[0] = local_uid;
  msg[1] = (int)(voltage_3v3*1000);
  msg[2] = (int)(current_3v3*1000);
  msg[3] = (int)(power_3v3*1000);
  msg[4] = (int)(voltage_5v0*1000);
  msg[5] = (int)(current_5v0*1000);
  msg[6] = (int)(power_5v0*1000);
  msglen = intlist2str(msg_ascii, msg, 7);

  // send to all subscribed tcp clients
  ntb_bcast_tcp(msg_ascii, msglen, NTB_TCPOPT_STREAM_POWER );
}

int genRandomDither(int maxOfst)
{
  // pick random int between 0 and 2*maxOfst
  int tmp = (int)(dw_rng_float()*2*maxOfst);
  // return a number between -maxOfst and +maxOfst
  return tmp - maxOfst;
}


int main(void)
{
  // Set vector table offset immediately (used for IAP)
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);

  // initialize NTB hardware, red led on until we're done
  ntb_init_hw();
  ntb_led_on(NTB_LED_ARM_RED);

  // Read UID value and set mac accordingly
  local_uid = ntb_uid_read();
  MAC_ADDR5 = local_uid;

  // read operating mode
  operating_mode = ntb_mode_read();

  // Reset ethernet switch
  ntb_ethswitch_rst();

  // init power measurement
  ntb_init_pmeas();

  // set ethernet switch to transparent clock mode
  ntb_ethswitch_set_tclk();

  // decaranging options: always listen, respond to beacons, respond to TWR init
  DWR_NodeOpts nodeOpts = DWR_OPT_RXALL | DWR_OPT_RSPTWR | DWR_OPT_SENDSUM;

  // set alpha to relay range summaries
  if( local_uid == 0 )
    nodeOpts |= DWR_OPT_GATEWAY;

  // initialize DecaWave ranging
  // configure ranging library
  DWR_Config_t dwr_config = {
    .panId = 0xAE70,
    .addr = local_uid,
    .nodeOpts = nodeOpts,
    .spiSendPacket = dw_spi_sendpacket,
    .spiReadPacket = dw_spi_readpacket,
    .sleepms = dw_sleep_msec,
    .rng = dw_rng_float,
    .cbRangeComplete = dw_range_complete,
    .cbBeaconReceived = dw_beacon_received,
    .cbDebug = dw_debug_handler
  };

  // random number gen.
  dw_rng_init();
  int dw_status = decaranging_init( dwr_config );
  if( dw_status == DWR_RETURN_ERR )
    ntb_led_error();

  // DWR done initializing. Increase SPI speed
  dw_spi_configprescaler(SPI_BaudRatePrescaler_4);

  // configure ethernet (GPIOs, clocks, MAC, DMA)
  ETH_BSP_Config();
    
  // Initilaize the LwIP stack
  LwIP_Init();
  
  // tcp echo server Init
  ntb_tcpserver_init(local_uid);
  ntb_tcpserver_setcallback(tcp_cmd_handler);

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(NTB_LED_ARM_RED);
  ntb_led_on(NTB_LED_ARM_BLUE);

  // initialize the random dither for beacon TX
  random_dither = genRandomDither(MAX_DITHER_MS);

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
      ntb_led_toggle(NTB_LED_BRIGHT_GREEN);

      // check for timer overflows
      //decaranging_checkovrf();
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

      // check for queued TWRs
      if( twr_queued == 1)
      {
        twr_queued = 0;
        decaranging_sendtwr();
        ntb_led_toggle(NTB_LED_BRIGHT_BLUE);
      }

      // take and send power measurements if a client has requested
      if( tcp_streamopts & NTB_TCPOPT_STREAM_POWER )
      {
        ntb_pmeas_read(NTB_POW_3V3, &voltage_3v3, &current_3v3, &power_3v3);
        ntb_pmeas_read(NTB_POW_5V0, &voltage_5v0, &current_5v0, &power_5v0);
        tcp_send_power();
        ntb_led_toggle(NTB_LED_BRIGHT_GREEN);
      }
    }

    // periodic DW beacons (if applicable)
    if(beacon_delay_ms && localtime_ms - last_beacon_time >= beacon_delay_ms + random_dither )
    {
      // pick new dither for next tx
      random_dither = genRandomDither(MAX_DITHER_MS);
      last_beacon_time = localtime_ms;
      // tx beacon
      decaranging_sendbcn();
      ntb_led_toggle(NTB_LED_BRIGHT_BLUE);
    }

    // periodic DW twr (if applicable)
    if(twr_delay_ms && localtime_ms - last_twr_time >= twr_delay_ms + random_dither )
    {
        // pick new dither for next tx
        random_dither = genRandomDither(MAX_DITHER_MS);
        last_twr_time = localtime_ms;
        decaranging_sendtwr();
        ntb_led_toggle(NTB_LED_BRIGHT_BLUE);
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

