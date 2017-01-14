#ifndef DECA_RANGING_H_
#define DECA_RANGING_H_

// Includes
#include "deca_device_api.h"
#include "deca_param_types.h"
#include "deca_regs.h"
#include "deca_version.h"

// max range estimates to keep in memory
#define DWR_MAX_RANGE_HISTORY (32)

// Superframe period, governing master beacons
#define DWR_SUPERFRAME_PERIOD_MS        (1000)

// Frames at 10 Hz during which each tag gets position estimate
#define DWR_FRAME_PERIOD_MS             (100)

// Subframes in each frame dedicated to a single tag
#define DWR_SUBFRAME_MAXQUERIES         (10)
#define DWR_SUBFRAME_PERIOD_MS (DWR_FRAME_PERIOD_MS/DWR_SUBFRAME_MAX)
#define DWR_SUBFRAME_MAXRESPONSES       (16)

// Transmission timing and dithering
#define DWR_RX_TIMEOUT_MS            (50)
#define DWR_REPLY_DELAY_US           (1000)
#define DWR_RANGE_INIT_MAX_DITHER_MS (50)
#define DWR_RANGE_RESP_MAX_DITHER_MS (6)

// DW High frequency timer units
#define DWR_TIMER_UNITS_HI_NS (4)
#define DWR_TIMER_UNITS_LO_PS (15)
#define DWR_TIMER_HI_31_BITS  (0xFFFFFFFE)

// Return types
#define DWR_RETURN_OK    (0)
#define DWR_RETURN_ERR   (-1)

// Propagation constants
#define SPEED_OF_LIGHT      (299702547.0)
#define DWM1000_ANT_DELAY (32820)


// Typical RF constants
#define DWT_PRF_64M_RFDLY   (514.462f)
#define DWT_PRF_16M_RFDLY   (513.9067f)

// Frame byte lengths
#define STANDARD_FRAME_SIZE   (32)
#define FRAME_CRC             (2)
#define FRAME_AND_ADDR_DATA   (9)
#define FRAME_AND_ADDR_BEACON (7)
#define MAX_PAYLOAD_DATA   (STANDARD_FRAME_SIZE - FRAME_AND_ADDR_DATA   - FRAME_CRC)

// Node Type
typedef enum 
{
  NODE_TYPE_FIXED   = 0x01,
  NODE_TYPE_MOBILE  = 0x02,
  NODE_TYPE_MOBILE_RSP = 0x03
}DWR_NodeType_t;

// Ranging message types -- See DW1000 manual pg. 213
typedef enum
{
  MSG_RANGE_INIT    = 0x01,
  MSG_RANGE_RESP    = 0x02,
  MSG_RANGE_FINAL   = 0x03,
  MSG_RANGE_SUMMARY = 0x04
}DWR_MsgType_t;

// Ranging states
typedef enum 
{
  STATE_RANGE_ERROR = 0x00,
  STATE_RANGE_INIT  = 0x01,
  STATE_RANGE_RESP  = 0x02,
  STATE_RANGE_FINAL = 0x03
}DWR_RangeState_t;

// Certain 802.15.4 frame types
typedef enum
{
  FRAME_BEACON = 0x00,
  FRAME_DATA   = 0x01,
  FRAME_ACK    = 0x02,
  FRAME_MAC    = 0x03,
  FRAME_RES1   = 0x04,
  FRAME_RES2   = 0x05,
  FRAME_RES3   = 0x06,
  FRAME_RES4   = 0x07
}DWR_FrameType_t;

// Debugging signals
typedef enum
{
  DWRANGE_EVENT_ERROR      = 0x00,
  DWRANGE_EVENT_RXGOOD     = 0x01,
  DWRANGE_EVENT_TXGOOD     = 0x02,
  DWRANGE_EVENT_IRQ        = 0x03,
  DWRANGE_EVENT_RXINIT     = 0x04,
  DWRANGE_EVENT_RXRESP     = 0x05,
  DWRANGE_EVENT_RXFIN      = 0x06,
  DWRANGE_EVENT_BADFRAME   = 0x07,
  DWRANGE_EVENT_UNKNOWN_IRQ= 0x08
}DWR_EventType_t;

// PROBE RESPONSE : ANC -> TAG
#define PAYLOAD_DATA_LEN (13)
typedef struct __attribute__((__packed__))
{
  uint8_t msgType;
  uint8_t rangeSeq;
  uint8_t nodeType;
  uint8_t t_tx[5];
  uint8_t t_rx[5];
} 
DWR_MsgData_t;

// Standard IEEE 802.15.4 Message Types
#define DSSS_FRAME_HEADER_LEN (9)
typedef struct __attribute__((__packed__))
{
  uint8_t frameCtrl[2];                           
  uint8_t seqNum;                               
  uint8_t panId[2];                       
  uint8_t destAddr[2];                    
  uint8_t sourceAddr[2];                        
  uint8_t msgData[MAX_PAYLOAD_DATA] ; 
  uint8_t fcs[2];                             
} ieee_frame_dsss_t ;

// Range estimation structure
typedef struct
{
  uint8_t nodeType;
  uint8_t nodeAddr[2];
  uint32_t rangeEst;
} DWR_RangeEst_t;

// Decawave configuration
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


// Public Function Definitions
int deca_ranging_init(
            uint16_t panId,
            uint16_t addr,
            uint8_t nodeType,
            void (*user_range_complete)(DWR_RangeEst_t *range),
            int (*spi_send_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, const uint8_t* body),
            int (*spi_read_packet)(uint16_t headerLen, const uint8_t* header, uint32_t bodyLen, uint8_t* body),
            void (*debug_callback)(uint8_t eventType),
            void (*sleep_msec)(uint32_t tmsec),
            float (*platform_rng)(void)
);

void deca_ranging_poll(uint32_t poll_period_ms);
void deca_ranging_isr();


// Private Transmit Functions
//static void dw_send_probe_req();
//static void dw_send_probe_rsp(uint8_t* destAddr);
static void dw_send_range_init();
static void dw_send_range_resp(uint8_t* destAddr, uint32_t txtime);
static void dw_send_range_fin(uint8_t* destAddr, uint64_t rxtime, uint32_t txtime);
static void dw_send_ieee_broadcast(DWR_MsgData_t *msg, uint32_t txtime);
static void dw_send_ieee_data_now(uint8_t* destAddr, DWR_MsgData_t *msg);
static void dw_send_ieee_data_delayed(uint8_t* destAddr, DWR_MsgData_t *msg, uint32_t txtime);

// Private Receive Functions
static void dw_listen();

void rxcallback(const dwt_callback_data_t *rxd);
void txcallback(const dwt_callback_data_t *txd);

// Private Debugging Functions
static void dw_debug(uint8_t eventType);


#endif // DECARANGING_H_
