// Includes
#include "deca_ranging.h"
#include <string.h>
// to be removed in library version:
#include "ntb_hwctrl.h"
// REMOVED BY ANDREW
//#include "usb_vcp_api.h"

// local node variables
uint8_t local_frame_seqNum = 0;
uint8_t local_panId[2];
uint8_t local_shortAddress[2];
uint8_t local_longAddress[8];
uint8_t local_nodeType;

// state machine variables
uint8_t  current_rangeState;
uint8_t  current_rangeSeq;
uint64_t time_tx_init;
uint64_t time_rx_init;
uint64_t time_tx_resp;
uint64_t time_rx_resp;
uint64_t time_tx_fin;
uint64_t time_rx_fin;

// rx data buffer
uint8_t rx_buffer[STANDARD_FRAME_SIZE];

// platform specific SPI functions
int (*spi_send_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body);
int (*spi_read_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body);

// platform specific debug function
void (*debug_callback)(uint8_t eventType);

// platform specific random number generator
float (*platform_rng)(void);

// platform specific sleep function
void (*sleep_msec)(uint32_t tmsec);

// platform specific range complete handler
void (*range_complete)(DWR_RangeEst_t *range);

// has the decawave been initialized?
int dwr_initialized = 0;

// channel configuration (taken from mode 3 of EVK1000)
dwt_config_t ch_config = {
	.chan = 2,
	.rxCode = 9,
	.txCode = 9,
	.prf = DWT_PRF_64M,
	.dataRate = DWT_BR_6M8,
	.txPreambLength = DWT_PLEN_128,
	.rxPAC = DWT_PAC8,
	.nsSFD = 0,
	.phrMode = DWT_PHRMODE_STD,
	.sfdTO = (256+64-32),
	.smartPowerEn = 0
};

// calibrated tx values
dwt_txconfig_t tx_calib = {
	.PGdly = 0xC2, // ch2
	//.power = 0x07274767
	.power = 0x00000000
};

// most recent ranging estimate
DWR_RangeEst_t dist_est_last;

// ========== Ranging Engine Initialization ==========
int deca_ranging_init(
            uint16_t panId,
            uint16_t shortAddr,
            uint8_t  nodeType,
            void (*user_range_complete)(DWR_RangeEst_t *range),
            int (*user_spi_send_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body),
            int (*user_spi_read_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body),
            void (*user_debug_callback)(uint8_t eventType),
            void (*user_sleep_msec)(uint32_t tmsec),
            float (*user_platform_rng)(void)
		)
{
	// initialization result
	int result;

	// copy network IDs
	local_panId[0] = (uint8_t)( panId       & 0xFF);
	local_panId[1] = (uint8_t)((panId >> 8) & 0xFF);
	local_shortAddress[0] = (uint8_t)( shortAddr       & 0xFF);
	local_shortAddress[1] = (uint8_t)((shortAddr >> 8) & 0xFF);
	local_nodeType = nodeType;	

	// copy user-provided SPI handlers
	spi_send_packet = user_spi_send_packet;
	spi_read_packet = user_spi_read_packet;

	// copy user-provided debug callback
	debug_callback = user_debug_callback;

	// copy user-provided sleep function
	sleep_msec = user_sleep_msec;

	// copy user-provided rng
	platform_rng = user_platform_rng;

	// copy user-provided range complete handler
	range_complete = user_range_complete;

	// wire up the SPI and sleep functions to the DW driver
	dwt_wire_spifcns(spi_send_packet, spi_read_packet);
	dwt_wire_sleepfcns(sleep_msec);

	// reset the DW1000
	ntb_led_on(LED_ARM_GREEN);
	dwt_softreset();
	ntb_led_off(LED_ARM_GREEN);

	// One-time-programmable (OTP) memory loading
	int otp_options = DWT_LOADUCODE | DWT_LOADLDO | DWT_LOADXTALTRIM;

	// load from OTP and initialize DW1000
	result = dwt_initialise(otp_options);
	if( result != DWT_SUCCESS ) return DWR_RETURN_ERR;

	// configure the channel
	dwt_configure(&ch_config, otp_options);
	dwt_configuretxrf(&tx_calib);

    // antenna delay calibration
    uint16_t tx_antennadelay = (uint16_t)( (DWT_PRF_64M_RFDLY/2.0)*1e-9/DWT_TIME_UNITS );
    dwt_setrxantennadelay(tx_antennadelay);
    dwt_settxantennadelay(tx_antennadelay);

    // configure RX mode. Only low power (mobile nodes that don't listen) should have RX timeout
    dwt_setrxmode(DWT_RX_NORMAL, 0, 0);
    dwt_setautorxreenable(1);

    if( local_nodeType == NODE_TYPE_MOBILE )
    	dwt_setrxtimeout(DWR_RX_TIMEOUT_MS*1e3);
    else
		dwt_setrxtimeout(0);

	// only allow 802.15.4 data frames
	//dwt_enableframefilter(DWT_FF_DATA_EN);
	dwt_enableframefilter(DWT_FF_NOTYPE_EN);

	// only allow interrupts on good rx frames and good tx frames
	dwt_setinterrupt( DWT_INT_RFCG | DWT_INT_TFRS, 1 );

	// set PAN and short address
	dwt_setpanid( panId );
	dwt_setaddress16( shortAddr );

	// if we're a fixed node or a responding mobile node, we should now listen for packets
	if( local_nodeType != NODE_TYPE_MOBILE )
		dw_listen();

	// and that's it! We're done initializing
	dwr_initialized = 1;
}

uint32_t deca_ranging_devid()
{
	return dwt_readdevid();
}


void deca_ranging_poll(uint32_t poll_period_ms)
{
	// If we're a tag or a response tag (one that will respond to range requests), 
	// occasionally query nearby nodes for distance measurements

	switch( local_nodeType )
	{
		case NODE_TYPE_FIXED:
			// bootstrap reception if it somehow got disabled.
			// TODO !!!
			break;
		case NODE_TYPE_MOBILE:
		case NODE_TYPE_MOBILE_RSP:
			// poll nearby nodes for range estimates
			dw_send_range_init();
			break;
		default:
			break;
	}
}

void deca_ranging_isr()
{
	// If we're not initialized, don't do anything
	if( !dwr_initialized )
		return;

	// Let user know an IRQ was triggered
	dw_debug(DWRANGE_EVENT_IRQ);

	// Read the event status
	uint32_t status = dwt_read32bitreg(SYS_STATUS_ID);
    uint32_t bitsToClear = 0x00;

    if(status & SYS_STATUS_RXFCG) // Receiver FCS Good
	{
		// let the user know a good frame was received
		dw_debug(DWRANGE_EVENT_RXGOOD);

		bitsToClear |= status & CLEAR_ALLRXGOOD_EVENTS;
	    dwt_write32bitreg(SYS_STATUS_ID, bitsToClear);

		// bug 634 - overrun overwrites the frame info data... so both frames should be discarded
		// read frame info and other registers and check for overflow again
		// if overflow set then discard both frames... 
		if (status & SYS_STATUS_RXOVRR) 
		{ 
			//when the overrun happens the frame info data of the buffer A (which contains the older frame e.g. seq. num = x) 
			//will be corrupted with the latest frame (seq. num = x + 2) data, both the host and IC are pointing to buffer A
			//we are going to discard this frame - turn off transceiver and reset receiver
			dwt_forcetrxoff();
			dwt_rxreset();	
			dwt_write16bitoffsetreg(SYS_CTRL_ID,0,(uint16_t)SYS_CTRL_RXENAB) ;
		}
		else //no overrun condition - proceed to process the frame
		{
			// clear all receive status bits
	        bitsToClear |= status & CLEAR_ALLRXGOOD_EVENTS;
	        dwt_write32bitreg(SYS_STATUS_ID, bitsToClear);

			// length of received data
			uint16_t frameLen = dwt_read16bitoffsetreg(RX_FINFO_ID, 0) & 0x3FF;

			// read entire frame into buffer
			dwt_readfromdevice(RX_BUFFER_ID, 0, frameLen, rx_buffer);

			// get frame type (bits 0-2 of frame)
			uint8_t frameType = rx_buffer[0] & 0x03;

			// ---------- HANDLE DATA -------------
			if(frameType == FRAME_DATA)
			{
				// read rx timestamp
				uint8_t rx_time_bytes[5];
				dwt_readrxtimestamp(rx_time_bytes);
				uint64_t rx_time = ((uint64_t)rx_time_bytes[4] << 32) + ((uint64_t)rx_time_bytes[3] << 24) + ((uint64_t)rx_time_bytes[2] << 16) + 
								   ((uint64_t)rx_time_bytes[1] <<  8) + ((uint64_t)rx_time_bytes[0]);

				// extract frame data
				ieee_frame_dsss_t *frame = (ieee_frame_dsss_t*)(rx_buffer);

				// extract payload
				DWR_MsgData_t *msg = (DWR_MsgData_t*)(frame->msgData);

				switch( msg->msgType )
				{
					case MSG_RANGE_INIT:
						// some node wants range information to nearby nodes. We should
						// respond if we're a fixed node or a responding mobile node
						if( local_nodeType == NODE_TYPE_FIXED || local_nodeType == NODE_TYPE_MOBILE_RSP )
						{
							// update range state variable and times
							current_rangeState = STATE_RANGE_INIT;
							// update new range sequence id
							current_rangeSeq = msg->rangeSeq;
							// update rx time of the init message
							time_rx_init = rx_time;
							// extract tx time of the init message
							time_tx_init = ((uint64_t)msg->t_tx[4]<<32) + ((uint64_t)msg->t_tx[3]<<24) + 
										   ((uint64_t)msg->t_tx[2]<<16) + ((uint64_t)msg->t_tx[1]<< 8) + (uint64_t)msg->t_tx[0];
							// schedule a delayed response with random dithering
						    uint32_t tx_time_min = (uint32_t)(rx_time>>8) + ( DWR_REPLY_DELAY_US )*1e3/DWR_TIMER_UNITS_HI_NS;
						    uint32_t tx_dither = (uint32_t)(platform_rng()*DWR_RANGE_RESP_MAX_DITHER_MS*1e6/DWR_TIMER_UNITS_HI_NS);
						    // if dithering would have overflowed the timer, ignore the dithering
						    //uint32_t tx_time = tx_time_min + tx_dither;
						    uint32_t tx_time = tx_time_min;
							// schedule the (delayed) response - takes high 32 bits of 40 bit timer
							dw_send_range_resp(frame->sourceAddr, tx_time);
							// update the tx time of the response message (expand hi 32 bits to 40 bit timer)
							time_tx_resp = (uint64_t)(tx_time)<<8;
							// update new range state
							current_rangeState = STATE_RANGE_RESP;
							// send debug command
							dw_debug(DWRANGE_EVENT_RXINIT);
						}
						break;
					case MSG_RANGE_RESP:
						// A node we sent a range init to has responded. Store the rx timestamp and send
						// a reply to that same node. First, check to make sure this data is for the correct
						// range sequence. And that we were in the range init state prior to this
						if( current_rangeSeq != msg->rangeSeq || current_rangeState != STATE_RANGE_INIT )
						{
							dwt_forcetrxoff();
							dwt_rxenable(0);
							current_rangeState = STATE_RANGE_ERROR;
							break;
						}
						// *** TIME CRITICAL PORTION ***
						// Otherwise, the range sequence is correct and the state transition was correct.
						// Store the time we received this message
						time_rx_resp = rx_time;
						// no dithering required here for response time. Do it as fast as possible
						uint32_t tx_time = (uint32_t)(rx_time>>8) + ( DWR_REPLY_DELAY_US )*1e3/DWR_TIMER_UNITS_HI_NS;
						// schedule the (delayed) response - takes high 32 bits of 40 bit timer
						dw_send_range_fin(frame->sourceAddr, time_rx_resp, tx_time);
						// Note: we do not update the new range state to "FINAL" because we may still be waiting
						// on additional range response packets from other nodes.
						break;
					case MSG_RANGE_FINAL:
						// A node that we've been ranging with has sent the final ranging information required to
						// calculate a range estimate. First let's make sure the range sequence and range state are
						// correct.
						if( current_rangeSeq != msg->rangeSeq || current_rangeState != STATE_RANGE_RESP )
						{
							dwt_forcetrxoff();
							dwt_rxenable(0);
							current_rangeState = STATE_RANGE_ERROR;
							break;
						}

						// Otherwise, extract the final timing information and calculate range.
						time_rx_fin  = rx_time;
						time_tx_fin  = ((uint64_t)msg->t_tx[4]<<32) + ((uint64_t)msg->t_tx[3]<<24) + 
									   ((uint64_t)msg->t_tx[2]<<16) + ((uint64_t)msg->t_tx[1]<<8 ) + (uint64_t)msg->t_tx[0];
					  	time_rx_resp = ((uint64_t)msg->t_rx[4]<<32) + ((uint64_t)msg->t_rx[3]<<24) + 
									   ((uint64_t)msg->t_rx[2]<<16) + ((uint64_t)msg->t_rx[1]<<8 ) + (uint64_t)msg->t_rx[0];

						// And now we have enough information to estimate range and inform the host application
						// TODO: If mobile response node, send a range summary packet
						// !!!

						// calculate time of flight in resolution of high-freq DW timer
						float ToF_dw = 0.25*(2*time_rx_resp - time_tx_init - 2*time_tx_resp + time_rx_init + time_rx_fin - time_tx_fin);

						// convert time of flight to picoseconds
						float ToF_sec = ToF_dw/1.0e9*(DWR_TIMER_UNITS_LO_PS);

						// update range estimate struct (distance in centimeters)
						dist_est_last.nodeType = msg->nodeType;
						dist_est_last.nodeAddr[0] = frame->sourceAddr[0];
						dist_est_last.nodeAddr[1] = frame->sourceAddr[1];
						dist_est_last.rangeEst = ToF_sec*SPEED_OF_LIGHT*100;

						// range complete debug message
						dw_debug(DWRANGE_EVENT_RXFIN);

						// pass estimate to the user
						range_complete(&dist_est_last);

						// reset the receiver
						dwt_forcetrxoff();
						dwt_rxenable(0);

						break;
					case MSG_RANGE_SUMMARY:
						// TODO: relay this information to the server
						// !!!
						break;
					default:
						break;
				}

			}

			// ---------- HANDLE OTHER ------------
			else
			{
				// we don't handle any other frame types. turn rx back on.
				dwt_rxenable(0);
			}

		}//end of no overrun 

    } // end if CRC is good
    else
    //
    // Check for TX frame sent event and signal to upper layer.
    //
    if (status & SYS_STATUS_TXFRS)  // Transmit Frame Sent
    {
        bitsToClear |= CLEAR_ALLTX_EVENTS;
        dwt_write32bitreg(SYS_STATUS_ID, bitsToClear) ;       

		// let the user know the transmission was successful
		dw_debug(DWRANGE_EVENT_TXGOOD);


    }
    else if (status & SYS_STATUS_RXRFTO) // Receiver Frame Wait timeout:
    {
        bitsToClear |= status & SYS_STATUS_RXRFTO;
        dwt_write32bitreg(SYS_STATUS_ID, bitsToClear);       

    }
    else if(status & CLEAR_ALLRXERROR_EVENTS) //catches all other error events 
    {
        bitsToClear |= status & CLEAR_ALLRXERROR_EVENTS;
        dwt_write32bitreg(SYS_STATUS_ID, bitsToClear);    

		dwt_forcetrxoff(); //this will clear all events
		dwt_rxreset();	//reset the RX

		status &= CLEAR_ALLTX_EVENTS;
    }

}

static void dw_send_range_init()
{
	// choose new, random range sequence
	current_rangeSeq = (uint8_t)(platform_rng()*0xFF);

	// calculate time to send broadcast in the future
	uint32_t time_now = dwt_readsystimestamphi32();
	uint32_t txtime = time_now + ( DWR_REPLY_DELAY_US )*1e3/DWR_TIMER_UNITS_HI_NS;

	// create probe message with new random range sequence
	DWR_MsgData_t msg;
	msg.msgType  = MSG_RANGE_INIT;
	msg.rangeSeq = current_rangeSeq;
	msg.nodeType = local_nodeType;
	msg.t_tx[0] = 0x00;
	msg.t_tx[1] = (uint8_t)(txtime);
	msg.t_tx[2] = (uint8_t)(txtime>>8);
	msg.t_tx[3] = (uint8_t)(txtime>>16);
	msg.t_tx[4] = (uint8_t)(txtime>>24);

	// send broadcast data frame
	dw_send_ieee_broadcast(&msg, txtime);

	// update our state
	current_rangeState = STATE_RANGE_INIT;
}

static void dw_send_range_resp(uint8_t* destAddr, uint32_t txtime)
{
	// create range response message
	DWR_MsgData_t msg;
	msg.msgType  = MSG_RANGE_RESP;
	msg.rangeSeq = current_rangeSeq;
	msg.nodeType = local_nodeType;

	// send delayed data frame
	dw_send_ieee_data_delayed(destAddr, &msg, txtime);
}

static void dw_send_range_fin(uint8_t* destAddr, uint64_t rxtime, uint32_t txtime)
{
	// create range response message
	DWR_MsgData_t msg;
	msg.msgType  = MSG_RANGE_FINAL;
	msg.rangeSeq = current_rangeSeq;
	msg.nodeType = local_nodeType;
	msg.t_tx[0] = 0x00;
	msg.t_tx[1] = (uint8_t)(txtime);
	msg.t_tx[2] = (uint8_t)(txtime>>8);
	msg.t_tx[3] = (uint8_t)(txtime>>16);
	msg.t_tx[4] = (uint8_t)(txtime>>24);
	msg.t_rx[0] = (uint8_t)(rxtime);
	msg.t_rx[1] = (uint8_t)(rxtime>>8);
	msg.t_rx[2] = (uint8_t)(rxtime>>16);
	msg.t_rx[3] = (uint8_t)(rxtime>>24);
	msg.t_rx[4] = (uint8_t)(rxtime>>32);

	// send delayed data frame
	dw_send_ieee_data_delayed(destAddr, &msg, txtime);
}

static void dw_send_ieee_broadcast(DWR_MsgData_t *msg, uint32_t txtime)
{
	uint8_t bcast_addr[2];
	bcast_addr[0] = 0xFF;
	bcast_addr[1] = 0xFF;
	dw_send_ieee_data_delayed(bcast_addr, msg, txtime);
}

static void dw_send_ieee_data_now(uint8_t* destAddr, DWR_MsgData_t *msg)
{
	// ======== 802.15.4 Data Frame: ========

	// [15-14] Source Address mode 			1,0 (Short)
	// [13-12] Frame Version                0,0 (15.4-2003)
	// [11-10] Dest. Address mode           1,0 (Short)
	// [9-8]   Reserved                     0,0

	// [7]     Reserved                     0
	// [6]     PAN ID Compress              1 (Yes)
	// [5]     ACK Request                  0 (No)
	// [4]     Frame Pending                0 (No)
	// [3]     Security Enabled             0 (No)
	// [2-0]   Frame Type                   0,0,1 (Data)

	//	( [1] = 0x88, [0] = 0x41 )

	// data frame header (dest=short, source=short [DsSs])
	ieee_frame_dsss_t frame;
	frame.frameCtrl[0] = 0x41;
	frame.frameCtrl[1] = 0x88;
	frame.seqNum = local_frame_seqNum++;
	frame.panId[0] = local_panId[0];
	frame.panId[1] = local_panId[1];
	frame.destAddr[0] = destAddr[0];
	frame.destAddr[1] = destAddr[1];
	frame.sourceAddr[0] = local_shortAddress[0];
	frame.sourceAddr[1] = local_shortAddress[1];

	// data custom payload
	memcpy(frame.msgData, (uint8_t*)msg, PAYLOAD_DATA_LEN);

	// write transmit binary data
	dwt_writetxdata(STANDARD_FRAME_SIZE, (uint8_t*)(&frame), 0);
	// frame length w/ 2-byte CRC, offset
	dwt_writetxfctrl(STANDARD_FRAME_SIZE, 0) ;
	// force transceiver back into idle
    dwt_forcetrxoff();
	// send tx -- immediate with response expected
	dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
}

static void dw_send_ieee_data_delayed(uint8_t* destAddr, DWR_MsgData_t *msg, uint32_t txtime)
{
		// ======== 802.15.4 Data Frame: ========

	// [15-14] Source Address mode 			1,0 (Short)
	// [13-12] Frame Version                0,0 (15.4-2003)
	// [11-10] Dest. Address mode           1,0 (Short)
	// [9-8]   Reserved                     0,0

	// [7]     Reserved                     0
	// [6]     PAN ID Compress              1 (Yes)
	// [5]     ACK Request                  0 (No)
	// [4]     Frame Pending                0 (No)
	// [3]     Security Enabled             0 (No)
	// [2-0]   Frame Type                   0,0,1 (Data)

	//	( [1] = 0x88, [0] = 0x41 )

	// data frame header (dest=short, source=short [DsSs])
	ieee_frame_dsss_t frame;
	frame.frameCtrl[0] = 0x41;
	frame.frameCtrl[1] = 0x88;
	frame.seqNum = local_frame_seqNum++;
	frame.panId[0] = local_panId[0];
	frame.panId[1] = local_panId[1];
	frame.destAddr[0] = destAddr[0];
	frame.destAddr[1] = destAddr[1];
	frame.sourceAddr[0] = local_shortAddress[0];
	frame.sourceAddr[1] = local_shortAddress[1];

	// data custom payload
	memcpy(frame.msgData, (uint8_t*)(msg), PAYLOAD_DATA_LEN);

	// write transmit binary data
	dwt_writetxdata(STANDARD_FRAME_SIZE, (uint8_t*)(&frame), 0);
	// frame length w/ 2-byte CRC, offset
	dwt_writetxfctrl(STANDARD_FRAME_SIZE, 0) ;
	// set future tx time (accepts high 32 bits of 40 bit timer)
	dwt_setdelayedtrxtime(txtime);
	// force transceiver back into idle
    dwt_forcetrxoff();
	// send tx -- immediate with RSP expected
	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
}


static void dw_listen()
{
	dwt_rxenable(0);
}


static void dw_debug(uint8_t eventType)
{
	debug_callback(eventType);
}