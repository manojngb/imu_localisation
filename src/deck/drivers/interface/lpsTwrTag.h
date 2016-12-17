#ifndef __LPS_TWR_TAG_H__
#define __LPS_TWR_TAG_H__

#include "locodeck.h"
#include "libdw1000.h"

#include "mac.h"

#define LPS_TWR_POLL 0x01   // Poll is initiated by the tag
#define LPS_TWR_ANSWER 0x02
#define LPS_TWR_FINAL 0x03
#define LPS_TWR_REPORT 0x04 // Report contains all measurement from the anchor

#define LPS_TWR_TYPE 0
#define LPS_TWR_SEQ 1

extern uwbAlgorithm_t uwbTwrTagAlgorithm;

typedef struct {
  uint64_t dummy_polltx;	
  uint64_t pollRx;
  uint64_t answerTx;
  uint64_t dummy_answerRx;
  uint64_t dummy_finalTx;
  uint64_t finalRx;

/*  float pressure;
  float temperature;
  float asl;
  uint8_t pressure_ok; */
} __attribute__((packed)) lpsTwrTagReportPayload_t;


#endif // __LPS_TWR_TAG_H__