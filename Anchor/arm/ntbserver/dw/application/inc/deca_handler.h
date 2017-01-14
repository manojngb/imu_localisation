// Includes
#include "deca_device_api.h"
#include "deca_param_types.h"
#include "deca_regs.h"
#include "deca_version.h"
#include "deca_spi.h"
#include "stm32f4xx_conf.h"

// ===== Hardware Definitions =====
#define DW_RESET_PORT 	GPIOD
#define DW_RESET_PIN	GPIO_Pin_0
#define DW_IRQ_PORT 	GPIOD
#define DW_IRQ_PIN		GPIO_Pin_2

// ===== Custom Packet Definitions =====
#define DW_MSG_BEACON	0xAB
#define DW_MSG_BEACON_LEN 1

// ===== Transmission Definitions =====
#define SPEED_OF_LIGHT      (299702547.0)     // in m/s in air
#define DWT_PRF_64M_RFDLY   (514.462f)
#define DWT_PRF_16M_RFDLY   (513.9067f)

#define MASK_40BIT			(0x00FFFFFFFFFF)  // MP counter is 40 bits
#define MASK_TXDTS			(0x00FFFFFFFE00)  //The TX timestamp will snap to 8 ns resolution - mask lower 9 bits.
#define USING_64BIT_ADDR (1) //when set to 0 - the DecaRanging application will use 16-bit addresses
#define EXTRA_LENGTH	 (2)

#define SIG_RX_ACK				5		// Frame Received is an ACK (length 5 bytes)
#define SIG_RX_BLINK			7		// Received ISO EUI 64 blink message
#define SIG_RX_BLINKDW			8		// Received ISO EUI 64 DW blink message
#define SIG_RX_UNKNOWN			99		// Received an unknown frame

#define STANDARD_FRAME_SIZE         127

#define ADDR_BYTE_SIZE_L            (8)
#define ADDR_BYTE_SIZE_S            (2)

#define FRAME_CONTROL_BYTES         2
#define FRAME_SEQ_NUM_BYTES         1
#define FRAME_PANID                 2
#define FRAME_CRC					2
#define FRAME_SOURCE_ADDRESS_S        (ADDR_BYTE_SIZE_S)
#define FRAME_DEST_ADDRESS_S          (ADDR_BYTE_SIZE_S)
#define FRAME_SOURCE_ADDRESS_L        (ADDR_BYTE_SIZE_L)
#define FRAME_DEST_ADDRESS_L          (ADDR_BYTE_SIZE_L)
#define FRAME_CTRLP					(FRAME_CONTROL_BYTES + FRAME_SEQ_NUM_BYTES + FRAME_PANID) //5
#define FRAME_CRTL_AND_ADDRESS_L    (FRAME_DEST_ADDRESS_L + FRAME_SOURCE_ADDRESS_L + FRAME_CTRLP) //21 bytes for 64-bit addresses)
#define FRAME_CRTL_AND_ADDRESS_S    (FRAME_DEST_ADDRESS_S + FRAME_SOURCE_ADDRESS_S + FRAME_CTRLP) //9 bytes for 16-bit addresses)
#define FRAME_CRTL_AND_ADDRESS_LS	(FRAME_DEST_ADDRESS_L + FRAME_SOURCE_ADDRESS_S + FRAME_CTRLP) //15 bytes for 1 16-bit address and 1 64-bit address)
#define MAX_USER_PAYLOAD_STRING_LL     (STANDARD_FRAME_SIZE-FRAME_CRTL_AND_ADDRESS_L-FRAME_CRC) //127 - 21 - 16 - 2 = 88
#define MAX_USER_PAYLOAD_STRING_SS     (STANDARD_FRAME_SIZE-FRAME_CRTL_AND_ADDRESS_S-FRAME_CRC) //127 - 9 - 16 - 2 = 100
#define MAX_USER_PAYLOAD_STRING_LS     (STANDARD_FRAME_SIZE-FRAME_CRTL_AND_ADDRESS_LS-FRAME_CRC) //127 - 15 - 16 - 2 = 94

//NOTE: the user payload assumes that there are only 88 "free" bytes to be used for the user message (it does not scale according to the addressing modes)
#define MAX_USER_PAYLOAD_STRING	MAX_USER_PAYLOAD_STRING_LL

#define BLINK_FRAME_CONTROL_BYTES       1
#define BLINK_FRAME_SEQ_NUM_BYTES       1
#define BLINK_FRAME_CRC					2
#define BLINK_FRAME_SOURCE_ADDRESS      8
#define BLINK_FRAME_CTRLP				(BLINK_FRAME_CONTROL_BYTES + BLINK_FRAME_SEQ_NUM_BYTES) //2
#define BLINK_FRAME_CRTL_AND_ADDRESS    (BLINK_FRAME_SOURCE_ADDRESS + BLINK_FRAME_CTRLP) //10 bytes


// Message data structs, based on address lengths -- (d)estination X and (s)ource Y

typedef struct
{
    uint8_t frameCtrl[2];                         	//  frame control bytes 00-01
    uint8_t seqNum;                               	//  sequence_number 02
    uint8_t panID[2];                             	//  PAN ID 03-04
    uint8_t destAddr[ADDR_BYTE_SIZE_L];             	//  05-12 using 64 bit addresses
    uint8_t sourceAddr[ADDR_BYTE_SIZE_L];           	//  13-20 using 64 bit addresses
    uint8_t messageData[MAX_USER_PAYLOAD_STRING_LL] ; //  22-124 (application data and any user payload)
    uint8_t fcs[2] ;                              	//  125-126  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} srd_msg_dlsl ;

typedef struct
{
    uint8_t frameCtrl[2];                         	//  frame control bytes 00-01
    uint8_t seqNum;                               	//  sequence_number 02
    uint8_t panID[2];                             	//  PAN ID 03-04
    uint8_t destAddr[ADDR_BYTE_SIZE_S];             	//  05-06
    uint8_t sourceAddr[ADDR_BYTE_SIZE_S];           	//  07-08
    uint8_t messageData[MAX_USER_PAYLOAD_STRING_SS] ; //  09-124 (application data and any user payload)
    uint8_t fcs[2] ;                              	//  125-126  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} srd_msg_dsss ;

typedef struct
{
    uint8_t frameCtrl[2];                         	//  frame control bytes 00-01
    uint8_t seqNum;                               	//  sequence_number 02
    uint8_t panID[2];                             	//  PAN ID 03-04
    uint8_t destAddr[ADDR_BYTE_SIZE_L];             	//  05-12 using 64 bit addresses
    uint8_t sourceAddr[ADDR_BYTE_SIZE_S];           	//  13-14
    uint8_t messageData[MAX_USER_PAYLOAD_STRING_LS] ; //  15-124 (application data and any user payload)
    uint8_t fcs[2] ;                              	//  125-126  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} srd_msg_dlss ;

typedef struct
{
    uint8_t frameCtrl[2];                         	//  frame control bytes 00-01
    uint8_t seqNum;                               	//  sequence_number 02
    uint8_t panID[2];                             	//  PAN ID 03-04
    uint8_t destAddr[ADDR_BYTE_SIZE_S];             	//  05-06
    uint8_t sourceAddr[ADDR_BYTE_SIZE_L];           	//  07-14 using 64 bit addresses
    uint8_t messageData[MAX_USER_PAYLOAD_STRING_LS] ; //  15-124 (application data and any user payload)
    uint8_t fcs[2] ;                              	//  125-126  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} srd_msg_dssl ;

//12 octets for Minimum IEEE ID blink
typedef struct
{
    uint8_t frameCtrl;                         		//  frame control bytes 00
    uint8_t seqNum;                               	//  sequence_number 01
    uint8_t tagID[BLINK_FRAME_SOURCE_ADDRESS];        //  02-09 64 bit address
    uint8_t fcs[2] ;                              	//  10-11  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} iso_IEEE_EUI64_blink_msg ;

//18 octets for IEEE ID blink with Temp and Vbat values
typedef struct
{
    uint8_t frameCtrl;                         		//  frame control bytes 00
    uint8_t seqNum;                               	//  sequence_number 01
    uint8_t tagID[BLINK_FRAME_SOURCE_ADDRESS];        //  02-09 64 bit addresses
	uint8_t enchead[2];								//  10-11 2 bytes (encoded header and header extension)
	uint8_t messageID;								//  12 message ID (0xD1) - DecaWave message
	uint8_t temp;										//  13 temperature value
	uint8_t vbat;										//  14 voltage value
	uint8_t gpio;										//  15 gpio status
    uint8_t fcs[2] ;                              	//  16-17  we allow space for the CRC as it is logically part of the message. However ScenSor TX calculates and adds these bytes.
} iso_IEEE_EUI64_blinkdw_msg ;

typedef struct
{
    uint8_t frameCtrl[2];                         	//  frame control bytes 00-01
    uint8_t seqNum;                               	//  sequence_number 02
    uint8_t fcs[2] ;                              	//  03-04  CRC
} ack_msg ;

typedef struct
{
    uint8_t channelNumber ;       // valid range is 1 to 11
    uint8_t preambleCode ;        // 00 = use NS code, 1 to 24 selects code
    uint8_t pulseRepFreq ;        // NOMINAL_4M, NOMINAL_16M, or NOMINAL_64M
    uint8_t dataRate ;            // DATA_RATE_1 (110K), DATA_RATE_2 (850K), DATA_RATE_3 (6M81)
    uint8_t preambleLen ;         // values expected are 64, (128), (256), (512), 1024, (2048), and 4096
    uint8_t pacSize ;
    uint8_t nsSFD ;
    uint16_t sfdTO;  //!< SFD timeout value (in symbols) e.g. preamble length (128) + SFD(8) - PAC + some margin ~ 135us... DWT_SFDTOC_DEF; //default value
} instanceConfig_t ;


/******************************************************************************************************************
*******************************************************************************************************************/

#define MAX_EVENT_NUMBER (10)
//NOTE: Accumulators don't need to be stored as part of the event structure as when reading them only one RX event can happen...
//the receiver is singly buffered and will stop after a frame is received

typedef struct
{
	uint8_t  type;			// event type
	uint8_t  type2;			// holds the event type - does not clear (not used to show if event has been processed)
	//uint8_t  broadcastmsg;	// specifies if the rx message is broadcast message
	uint16_t rxLength ;

	uint64_t  timeStamp ;		// last timestamp (Tx or Rx)

	uint32_t  timeStamp32l ;		   // last tx/rx timestamp - low 32 bits
	uint32_t  timeStamp32h ;		   // last tx/rx timestamp - high 32 bits

	union {
			//holds received frame (after a good RX frame event)
			uint8_t   frame[STANDARD_FRAME_SIZE];
    		srd_msg_dlsl rxmsg_ll ; //64 bit addresses
			srd_msg_dssl rxmsg_sl ;
			srd_msg_dlss rxmsg_ls ;
			srd_msg_dsss rxmsg_ss ; //16 bit addresses
			ack_msg rxackmsg ; //holds received ACK frame
			iso_IEEE_EUI64_blink_msg rxblinkmsg;
			iso_IEEE_EUI64_blinkdw_msg rxblinkmsgdw;
	}msgu;

	//uint32_t  eventtime ;
	//uint32_t  eventtimeclr ;
	//uint8_t gotit;
}event_data_t ;

typedef struct {
                uint8_t PGdelay;

                //TX POWER
                //31:24     BOOST_0.125ms_PWR
                //23:16     BOOST_0.25ms_PWR-TX_SHR_PWR
                //15:8      BOOST_0.5ms_PWR-TX_PHR_PWR
                //7:0       DEFAULT_PWR-TX_DATA_PWR
                uint32_t  txPwr[2]; //
}tx_config_t;


typedef struct {
                uint8_t PGdelay;

                //TX POWER
                //31:24     BOOST_0.125ms_PWR
                //23:16     BOOST_0.25ms_PWR-TX_SHR_PWR
                //15:8      BOOST_0.5ms_PWR-TX_PHR_PWR
                //7:0       DEFAULT_PWR-TX_DATA_PWR
                uint32_t txPwr[2]; //
}tx_struct;

// Function Definitions
void dw_init();
int dw_setup(uint16_t flags);
int dw_chconfig(dwt_config_t conf, int flags);
void dw_reset();
void dw_sendbeacon();
void dw_listen();
void dw_handle_rxdone();
void dw_handle_txdone();