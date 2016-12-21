#ifndef __MAC_H__
#define __MAC_H__

#include <stdint.h>

#include "locodeck.h"

// Packet format with compressed PAN and 64Bit addresses
// Maximum 64 bytes payload
typedef struct packet_s {

	uint8_t frameCtrl[2];
	uint8_t seqNum;
    uint8_t panId[2];
	uint16_t destAddress;
	uint16_t sourceAddress;
    uint8_t payload[64];
	//uint8_t fcs[2];
} __attribute__((packed)) packet_t;

#define MAC80215_PACKET_INIT(packet)  packet.frameCtrl[0] = 0x41; \
  packet.frameCtrl[1] = 0x88; \
  packet.seqNum = 0; \
  packet.panId[0] = 0xAE70 & 0xFF; \
  packet.panId[1] = (0xAE70 >> 8) & 0xFF; \
  packet.sourceAddress = 9; 


#define MAC802154_TYPE_BEACON 0
#define MAC802154_TYPE_DATA 1
#define MAC802154_TYPE_ACK 2
#define MAC802154_TYPE_CMD 3

#define MAC802154_HEADER_LENGTH 9

#endif
