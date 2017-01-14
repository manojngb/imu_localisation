#ifndef NTB_BTBRIDGE_H_
#define NTB_BTBRIDGE_H_

#include "stm32f30x.h"

#define MSGTYPE_IMU (0)

/* DATA STRUCTS FOR TRANSMISSION */
// 3d vector
typedef struct __attribute__((__packed__))
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
} vec3_t;

// 9 sensor IMU
typedef struct __attribute__((__packed__))
{
	vec3_t acc;
	vec3_t gyr;
	vec3_t mag;
} imu_t;

// range measurements
typedef struct __attribute__((__packed__))
{
	uint16_t src;
	uint16_t dst;
	uint16_t range;
} range_t;

// Payload union
typedef union payload
{
	uint8_t str[sizeof(imu_t)];
	imu_t imu;
	range_t range;
} bt_tx_msg_payload_t;

// Header struct for all messages
typedef struct __attribute__((__packed__))
{
	uint32_t timestamp;
	uint8_t type;
} bt_tx_msg_header_t;

// Bluetooth message
typedef struct __attribute__((__packed__))
{
	bt_tx_msg_header_t header;
	bt_tx_msg_payload_t payload;
} bt_tx_msg_t;

/* DATA STRUCTS FOR RECEIVING */
typedef struct __attribute__((__packed__))
{
	uint8_t type;
	uint8_t cmd;
} bt_rx_msg_t;


// API
int btbridge_rx(uint8_t c);
void btbridge_send(bt_tx_msg_t *msg);
void btbridge_register_cb(void (*cbfcn)(bt_rx_msg_t msg));

#endif