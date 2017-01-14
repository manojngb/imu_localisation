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
#define MAX_DITHER_MS (40)

// TWR broadcasting sequencing
// uid = index, trigger = element
#define MAX_TWR_NODES (8)

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


void dw_range_complete(DWR_TwrTiming_t *timeStats)
{
  
}


void dw_beacon_received(DWR_Beacon_t *beacon)
{
 
}

// send voltage, current, and power for 3v3 and 5v0 lines over TCP
void tcp_send_power()
{
  
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
  // initialize NTB hardware, red led on until we're done
  ntb_init_hw();
  ntb_led_on(NTB_LED_ARM_RED);

  // Read UID value and set mac accordingly
  local_uid = 9;
  MAC_ADDR5 = local_uid;

  // read operating mode
  operating_mode = 0;

  // decaranging options: always listen, respond to beacons, respond to TWR init
  DWR_NodeOpts nodeOpts = DWR_OPT_RXALL | DWR_OPT_RSPTWR | DWR_OPT_SENDSUM;

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

  // turn off red led and turn on blue to show we're done with init
  ntb_led_off(NTB_LED_ARM_RED);
  ntb_led_on(NTB_LED_ARM_BLUE);

  // initialize the random dither for beacon TX
  random_dither = genRandomDither(MAX_DITHER_MS);

  while (1)
  {  
    // ----- 1 Hz Loop -----
    if( localtime_ms - last_1hz_time >= 1000 )
    {
      last_1hz_time = localtime_ms;
      ntb_led_toggle(NTB_LED_ARM_GREEN);
      decaranging_sendtwr();
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
        ntb_led_toggle(NTB_LED_BRIGHT_ORANGE);
      }
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

