#ifndef NTB_HWCTRL_H_
#define NTB_HWCTRL_H_

#include "stm32f30x.h"

// MPU Address
#define MPU_ADDR (0b1101001)

// Important registers


// Data structures
typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
} acc3_t;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
} gyr3_t;

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t z;
} mag3_t;

// API
void ntb_mpu_i2c_write(uint8_t data);
uint8_t ntb_mpu_i2c_readack();
uint8_t ntb_mpu_i2c_readnack();
void ntb_mpu_i2c_stop();
void ntb_mpu_writereg(uint8_t reg, uint8_t *byte, int len);
void ntb_mpu_readreg(uint8_t reg, uint8_t *buf, int len);
void ntb_mpu_readall(acc3_t* a, gyr3_t* g, mag3_t* m);


#endif