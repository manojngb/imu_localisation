#include "ntb_hwctrl.h"
#include <stm32f4xx_i2c.h>


void ntb_init_hw()
{
  ntb_init_leds();
  ntb_init_uid();
  ntb_init_gpio();
  ntb_deinit_spiexp();
}

void ntb_init_leds()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Enable Port E
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = LED_ARM_GREEN | LED_ARM_RED | LED_ARM_BLUE;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(LED_ARM_PORT, &GPIO_InitStruct);

}

void ntb_init_uid()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = UIDB0 | UIDB1 | UIDB2 | UIDB3 | UIDB4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);
}

void ntb_deinit_spiexp()
{
  // Release expansion SPI for external control
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Chip Select
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure PD1 in input, no pull
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  // MOSI / MISO / SCK
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  // Configure PD1 in input, no pull
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

}

void ntb_init_gpio()
{
  // Release expansion SPI for external control
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Configure GPIO1 to output, high (this is DW1000 RSTn)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_4);

  // Configure GPIO3 to input, no pull (this is DW1000 IRQ)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_6);

  // Configure NETW_RST high (this is ethernet RSTn)
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(UID_PORT, &GPIO_InitStruct);

  GPIO_SetBits(GPIOD, GPIO_Pin_2);

  // Configure power control pins
  GPIO_InitStruct.GPIO_Pin = PCTRL_SHTDN_3V3 | PCTRL_SHTDN_5V0 | PCTRL_SHTDN_DUT;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(PCTRL_PORT, &GPIO_InitStruct);

  // enable voltage rails by default
  GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_3V3 | PCTRL_SHTDN_5V0);
}

uint16_t ntb_uid_read()
{
  uint16_t uid_port = GPIO_ReadInputData(UID_PORT);
  // only extract bits 10->14 and convert to uint
  return (uid_port & 0x7C00) >> 10;
}


// LED functions
void ntb_led_on(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_off(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_toggle(led_t led)
{
  switch(led)
  {
    case LED_ARM_GREEN:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case LED_ARM_RED:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case LED_ARM_BLUE:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    default:
      break;
  }
}

void ntb_led_error()
{
  int i;

  while(1)
  {
       // Ramp on
       ntb_busywait_ms(150);
       ntb_led_on(LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_on(LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_on(LED_ARM_BLUE);

       // Ramp off
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_off(LED_ARM_BLUE);
  }
}

void ntb_ethswitch_off()
{
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_RST);
}

void ntb_ethswitch_on()
{
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_RST);
}


void ntb_init_pmeas()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  I2C_InitTypeDef I2C_InitStruct;
  
  // enable APB1 peripheral clock for I2C1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  // enable clock for SCL and SDA pins
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  /* setup SCL and SDA pins
   * You can connect I2C1 to two different
   * pairs of pins:
   * 1. SCL on PB6 and SDA on PB7 
   * 2. SCL on PB8 and SDA on PB9
   */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; 
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;  
  GPIO_InitStruct.GPIO_OType = GPIO_OType_OD; 
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  // Connect I2C1 pins to AF  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); // SCL
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1); // SDA
  
  // configure I2C1 
  I2C_InitStruct.I2C_ClockSpeed = 100000;
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% --> standard
  I2C_InitStruct.I2C_OwnAddress1 = 0x00;      
  I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; 
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; 
  I2C_Init(I2C1, &I2C_InitStruct);
  
  // enable I2C1
  I2C_Cmd(I2C1, ENABLE);

  // wait a little bit for enable to kick in
  ntb_busywait_ms(100);

  // now configure the power measurement IC
  // [1] set vscale to +/- 80 mV (nothing to do, default)
  // [2] set vsense time to every 10 ms
  uint8_t data = 0x23;
  ntb_pmeas_writereg(PMEAS_REG_VS1CFG, &data, 1);
  ntb_pmeas_writereg(PMEAS_REG_VS2CFG, &data, 1);

}


/* This function issues a start condition and 
 * transmits the slave address + R/W bit
 * 
 * Parameters:
 *    I2Cx --> the I2C peripheral e.g. I2C1
 *    address --> the 7 bit slave address
 *    direction --> the tranmission direction can be:
 *            I2C_Direction_Tranmitter for Master transmitter mode
 *            I2C_Direction_Receiver for Master receiver
 */
void ntb_pmeas_i2c_start(uint8_t address, uint8_t direction){
  // wait until I2C1 is not busy anymore
  while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
  
  // Send I2C1 START condition 
  I2C_GenerateSTART(I2C1, ENABLE);
    
  // wait for I2C1 EV5 --> Slave has acknowledged start condition
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
  // Send slave Address for write 
  I2C_Send7bitAddress(I2C1, address, direction);
    
  /* wait for I2C1 EV6, check if 
   * either Slave has acknowledged Master transmitter or
   * Master receiver mode, depending on the transmission
   * direction
   */ 
  if(direction == I2C_Direction_Transmitter){
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  }
  else if(direction == I2C_Direction_Receiver){
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  }
}

void ntb_pmeas_i2c_repstart(uint8_t address, uint8_t direction){

  // Send I2C1 START condition 
  I2C_GenerateSTART(I2C1, ENABLE);

  // wait for I2C1 EV5 --> Slave has acknowledged start condition
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

  // Send slave Address for write 
  I2C_Send7bitAddress(I2C1, address, direction);
    
  /* wait for I2C1 EV6, check if 
   * either Slave has acknowledged Master transmitter or
   * Master receiver mode, depending on the transmission
   * direction
   */
  if(direction == I2C_Direction_Transmitter){
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  }
  else if(direction == I2C_Direction_Receiver){
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  }
}

/* This function transmits one byte to the slave device
 * Parameters:
 *    I2Cx --> the I2C peripheral e.g. I2C1 
 *    data --> the data byte to be transmitted
 */
void ntb_pmeas_i2c_write(uint8_t data)
{
  I2C_SendData(I2C1, data);
  // wait for I2C1 EV8_2 --> byte has been transmitted
  while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/* This function reads one byte from the slave device 
 * and acknowledges the byte (requests another byte)
 */
uint8_t ntb_pmeas_i2c_readack(){
  // enable acknowledge of recieved data
  I2C_AcknowledgeConfig(I2C1, ENABLE);
  // wait until one byte has been received
  while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
  // read data from I2C data register and return data byte
  uint8_t data = I2C_ReceiveData(I2C1);
  return data;
}

/* This function reads one byte from the slave device
 * and doesn't acknowledge the recieved data 
 */
uint8_t ntb_pmeas_i2c_readnack(){
  // disabe acknowledge of received data
  // nack also generates stop condition after last byte received
  // see reference manual for more info
  I2C_AcknowledgeConfig(I2C1, DISABLE);
  I2C_GenerateSTOP(I2C1, ENABLE);
  // wait until one byte has been received
  while( !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) );
  // read data from I2C data register and return data byte
  uint8_t data = I2C_ReceiveData(I2C1);
  return data;
}

/* This funtion issues a stop condition and therefore
 * releases the bus
 */
void ntb_pmeas_i2c_stop(){
  // Send I2C1 STOP Condition 
  I2C_GenerateSTOP(I2C1, ENABLE);
}

void ntb_pmeas_writereg(uint8_t reg, uint8_t *byte, int len)
{
  int b=0;

  // [1] write address (STM library writes 1 for TX, 0 for RX)
  ntb_pmeas_i2c_start((PMEAS_IC_ADDR<<1), I2C_Direction_Transmitter);

  // [2] write reg
  ntb_pmeas_i2c_write(reg);

  // [3] write data
  for(b=0; b<len; b++)
  {
    ntb_pmeas_i2c_write(byte[b]);
  }

  // [4] done!
  ntb_pmeas_i2c_stop();

}

void ntb_pmeas_readreg(uint8_t reg, uint8_t *buf, int len)
{
  int b=0;

  // [1] write address (STM library writes 1 for TX, 0 for RX)
  ntb_pmeas_i2c_start((PMEAS_IC_ADDR<<1), I2C_Direction_Transmitter);

  // [2] write reg
  ntb_pmeas_i2c_write(reg);

  // [3] read data, each requiring repeated start signal 
  ntb_pmeas_i2c_repstart((PMEAS_IC_ADDR<<1), I2C_Direction_Receiver);

  // read with ACK all but final byte
  for( b=0; b<(len-1); b++ )
  {
    buf[b] = ntb_pmeas_i2c_readack();
  }

  // read final byte with NACK
  buf[b] = ntb_pmeas_i2c_readnack();

  // [4] done! - stop auto-generated by NACK
}

void ntb_pmeas_read(powrail_t rail, float *v, float *i, float *p)
{
  float FSC = 0;
  uint8_t vsense_reg, vsource_reg, power_reg;
  uint8_t tmp_buf16[2];

  switch(rail)
  {
    case POWER_3V3:
      // full scale current
      FSC = 80.0e-3/PMEAS_RSENSE_3V3;
      // Vsense reg
      vsense_reg = PMEAS_REG_VS1HI;
      // Vsource reg
      vsource_reg = PMEAS_REG_V1HI;
      // Power reg
      power_reg = PMEAS_REG_PR1HI;
      break;
    case POWER_5V0:
      // full scale current
      FSC = 80.0e-3/PMEAS_RSENSE_5V0;
      // Vsense reg
      vsense_reg = PMEAS_REG_VS2HI;
      // Vsource reg
      vsource_reg = PMEAS_REG_V2HI;
      // Power reg
      power_reg = PMEAS_REG_PR2HI;
      break;
    default:
      break;
  }

  // [1] Read Vsense
  int16_t vsense;
  ntb_pmeas_readreg(vsense_reg, tmp_buf16, 2);
  vsense = (int16_t)( (tmp_buf16[0] << 8) | tmp_buf16[1] ) >> 7; // sign+8bit FIX
  
  // [2] Calculate Ibus
  float denom = 255.0;
  (*i) = (float)(FSC*vsense/denom);

  // [3] Read Vsource
  float FSV = 40 - 40/1024.0;
  denom = 1023.0;
  uint16_t vsource;
  ntb_pmeas_readreg(vsource_reg, tmp_buf16, 2);
  vsource = (uint16_t)( (tmp_buf16[0] << 8) | tmp_buf16[1] ) >> 6; // 10bits
  (*v) = (float)(FSV*vsource/denom);

  // [4] Calculate Power
  float FSP = FSC*FSV;
  ntb_pmeas_readreg(power_reg, tmp_buf16, 2);
  int16_t pratio = (int16_t)( (tmp_buf16[0] << 8) | tmp_buf16[1]);
  (*p) = FSP*pratio/65535.0;
}


void ntb_pctrl_on(powrail_t rail)
{
  switch(rail)
  {
    case POWER_3V3:
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case POWER_5V0:
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_off(powrail_t rail)
{
  switch(rail)
  {
    case POWER_3V3:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case POWER_5V0:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_toggle(powrail_t rail)
{
  switch(rail)
  {
    case POWER_3V3:
      GPIO_ToggleBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case POWER_5V0:
      GPIO_ToggleBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_rst(powrail_t rail)
{
  // TODO
}



//Quick hack, approximately 1ms delay
void ntb_busywait_ms(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}