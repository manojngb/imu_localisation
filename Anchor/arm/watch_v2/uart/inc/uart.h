#ifndef _UART_H_
#define _UART_H_

// ===== INCLUDES =====
#include <stdint.h>
#include <stdbool.h>

// ===== DEFINITIONS =====
#define PACKET_START 'S'
#define PACKET_STOP 'E'
#define PACKET_LEN (7)
#define PACKET_TYPE_IMU 'I'
#define PACKET_TYPE_RNG 'R'

// ===== STRUCTURES =====
typedef struct __attribute__((packed)){
  uint16_t dist_cm;
  int16_t angle_d;
} uart_imu_t;

typedef struct __attribute__((packed)){
  uint16_t aid;
  uint16_t range_cm;
} uart_rng_t;

typedef union __attribute__((packed)){
  uart_rng_t rng;
  uart_imu_t imu;
} uart_payload_t;

typedef struct __attribute__((packed)){
  uint8_t type;
  uart_payload_t payload;
} uart_data_t;

typedef struct __attribute__((packed)){
  uint8_t start;
  uart_data_t data;
  uint8_t stop;
} uart_frame_t;

// ===== PUBLIC API =====
void uart_tx_range(uint16_t ancId, float rangeCm);

#endif