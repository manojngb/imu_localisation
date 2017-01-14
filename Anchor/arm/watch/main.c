/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ntb_hwctrl.h"
#include "deca_ranging.h"
#include "deca_debug.h"
#include "deca_rng.h"
#include "deca_spi.h"
#include "ntb_mpu9250.h"
#include "ntb_btbridge.h"

#define SYSTEMTICK_PERIOD_MS  (10)
#define DWRANGE_POLL_PERIOD_MS (500)


// Timing variables
volatile uint32_t localtime_ms = 0;
uint32_t last_1hz_time = 0;
uint32_t last_10hz_time = 0;
uint32_t last_100hz_time = 0;
// Local node identifier
uint16_t local_uid = 0;

// BT message struct to be reused
bt_tx_msg_t bt_tx_msg;


void dw_range_complete(DWR_TwrTiming_t *timeStats)
{
  // TODO !!!
}

void dw_beacon_received(DWR_Beacon_t *beacon)
{
  // TODO !!!  
}

void btbridge_msgrxed(bt_rx_msg_t msg)
{
  // TODO !!!
}

void btbridge_sendimu(vec3_t acc, vec3_t gyr, vec3_t mag)
{
  // create header
  bt_tx_msg_header_t header = {.timestamp=localtime_ms, .type=MSGTYPE_IMU};
  // create imu struct
  imu_t imu = {.acc=acc, .gyr=gyr, .mag=mag};
  // create payload struct
  bt_tx_msg_payload_t payload = {.imu=imu};
  // create message struct
  bt_tx_msg_t msg = {.header=header, .payload=payload};

  // send the message
  btbridge_send(&msg);
}


int main(void)
{
  // initialize NTB hardware, red led on until we're done
  ntb_init_hw();
  SysTick_Config(SystemCoreClock / 100);

  // register callback for bt messages
  btbridge_register_cb( btbridge_msgrxed );

  // Set local UID
  local_uid = 8;

  // decaranging options: always listen, respond to beacons, respond to TWR init
  DWR_NodeOpts nodeOpts = DWR_OPT_RXALL | DWR_OPT_RSPTWR;

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

  int dw_status = decaranging_init( dwr_config );
  if( dw_status == DWR_RETURN_ERR )
    ntb_led_error();

  // DWR done initializing. Increase SPI speed
  dw_spi_configprescaler(SPI_BaudRatePrescaler_4); // 18 MHz

  ntb_led_on();

  while (1)
  {  
    // ----- 1 Hz Loop -----
    if( localtime_ms - last_1hz_time >= 1000 )
    {
	/*
      last_1hz_time = localtime_ms;
      // send dummy IMU data over UART to BT
      vec3_t acc = {.x=1, .y=2, .z=3};
      vec3_t gyr = {.x=4, .y=5, .z=6};
      vec3_t mag = {.x=7, .y=8, .z=9};
      btbridge_sendimu(acc, gyr, mag);
	  */
    }

    // ----- 10 Hz Loop -----
    if( localtime_ms - last_10hz_time >= 100 )
    {
      last_10hz_time = localtime_ms;
      ntb_led_toggle();
      decaranging_sendtwr();
    }

    // ----- 100 Hz Loop -----
    if( localtime_ms - last_100hz_time >= 10 )
    {
      last_100hz_time = localtime_ms;
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
  {
    ntb_led_asserterror();
  }
}
#endif

