#include "ntb_hwctrl.h"
#include <stm32f4xx_i2c.h>


void ntb_init_hw()
{
  ntb_init_leds();
  ntb_init_uid();
  ntb_init_gpio();
  ntb_deinit_spiexp();
  ntb_init_spidw();
}

void ntb_init_leds()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  // Enable Port E
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // Configure PE4,5,6 (LEDs) in output pushpull mode
  GPIO_InitStruct.GPIO_Pin = LED_ARM_GREEN | LED_ARM_RED | LED_ARM_BLUE | LED_BRIGHT_GREEN | LED_BRIGHT_RED | LED_BRIGHT_BLUE | LED_BRIGHT_ORANGE;
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


void ntb_init_spidw()
{
  // ----------- Turn on orange LED to say we're busy -------------
  GPIO_InitTypeDef GPIO_InitStruct;
  SPI_InitTypeDef SPI_InitStruct;
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;

  SPI_I2S_DeInit(DW_SPI);

  /* ------------ Fire up SPI --------------- */
  RCC_APB1PeriphClockCmd(DW_SPI_PERIPH, ENABLE);

  /* configure SPI1 in Mode 0 
   * CPOL = 0 --> clock is low when idle
   * CPHA = 0 --> data is sampled at the first edge
   */
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;  
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;       
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // 4-64
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStruct.SPI_CRCPolynomial = 7;

  SPI_Init(DW_SPI, &SPI_InitStruct); 

  // Turn on port C
  RCC_AHB1PeriphClockCmd(DW_SPI_GPIOPERIPH, ENABLE);
   
  // Configure SCK, MISO, & MOSI
  GPIO_InitStruct.GPIO_Pin = DW_SCK_PIN | DW_MISO_PIN | DW_MOSI_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(DW_SPI_PORT, &GPIO_InitStruct);

  // Configure CS
  GPIO_InitStruct.GPIO_Pin = DW_CS_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DW_SPI_CS_PORT, &GPIO_InitStruct);

  // SPI alternate functions
  GPIO_PinAFConfig(DW_SPI_PORT, DW_SCK_PSOURCE,  DW_SPI_AF);
  GPIO_PinAFConfig(DW_SPI_PORT, DW_MISO_PSOURCE, DW_SPI_AF);
  GPIO_PinAFConfig(DW_SPI_PORT, DW_MOSI_PSOURCE, DW_SPI_AF);

  // Disable SPI SS output
  SPI_SSOutputCmd(DW_SPI, DISABLE);

  // start with CS high
  GPIO_SetBits(DW_SPI_CS_PORT, DW_CS_PIN);

  // Enable SPI
  SPI_Cmd(DW_SPI, ENABLE);

  // Configure DW reset line  
  RCC_AHB1PeriphClockCmd(DW_RESET_PERIPH, ENABLE);

  GPIO_InitStruct.GPIO_Pin = DW_RESET_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(DW_RESET_PORT, &GPIO_InitStruct);

  // ensure reset is high to begin
  GPIO_SetBits(DW_RESET_PORT, DW_RESET_PIN);

  // Configure the DW IRQ line
  GPIO_InitStruct.GPIO_Pin = DW_IRQ_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(DW_IRQ_PORT, &GPIO_InitStruct);
  SYSCFG_EXTILineConfig(DW_IRQ_EXTI, DW_IRQ_PSOURCE);

  // Configure the EXTI for DW IRQ
  EXTI_InitStruct.EXTI_Line = DW_IRQ_LINE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);
  NVIC_InitStruct.NVIC_IRQChannel = DW_IRQ_CHNL;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);

  // ---------- Turn off orange LED to say we're done -------------
  dw_sleep_msec(500);
}

void ntb_init_gpio()
{
	// Release expansion SPI for external control
	GPIO_InitTypeDef  GPIO_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Configure GPIO1 to output, high (this is DW1000 RSTn)
	GPIO_InitStruct.GPIO_Pin = GPIO_P1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIO_PORT, &GPIO_InitStruct);

	GPIO_SetBits(GPIO_PORT, GPIO_P1);

	// Configure GPIO2 to input, no pull (this is DW1000 IRQ)
	GPIO_InitStruct.GPIO_Pin = GPIO_P2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_Init(GPIO_PORT, &GPIO_InitStruct);

	GPIO_SetBits(GPIO_PORT, GPIO_P2);

	// Configure NETW_RST high (this is ethernet RSTn)
	GPIO_InitStruct.GPIO_Pin = ETHSWITCH_RST;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(UID_PORT, &GPIO_InitStruct);

	GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_RST);

	// Configure ethernet CS pin and set high (deselect)
	GPIO_InitStruct.GPIO_Pin = ETHSWITCH_CS;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(ETHSWITCH_PORT, &GPIO_InitStruct);
	GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

	// Configure power control pins
	GPIO_InitStruct.GPIO_Pin = PCTRL_SHTDN_3V3 | PCTRL_SHTDN_5V0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(PCTRL_PORT, &GPIO_InitStruct);

	// enable voltage rails by default
	GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_3V3 | PCTRL_SHTDN_5V0);

	// Configure DUT reset pin
	GPIO_InitStruct.GPIO_Pin = RST_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(RST_PORT, &GPIO_InitStruct);

	// active low reset high by default
	GPIO_SetBits(RST_PORT, RST_PIN);

	// the rest of the GPIO
	GPIO_InitStruct.GPIO_Pin = GPIO_P0 | GPIO_P3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIO_PORT, &GPIO_InitStruct);

	GPIO_ResetBits(GPIO_PORT, GPIO_P0 | GPIO_P3);
}

uint16_t ntb_uid_read()
{
	// Bit mapping:
	// 0 - 14 (shift 14)
	// 1 - 13 (shift 12)
	// 2 - 12 (shift 10)
	// 3 - 11 (shift 8)
	// 4 - 10 (shift 6)
	uint16_t uid_port = GPIO_ReadInputData(UID_PORT);
	uint16_t uid = ((uid_port >> 14) & 0x01) +
				   ((uid_port >> 12) & 0x02) +
				   ((uid_port >> 10) & 0x04) +
				   ((uid_port >>  8) & 0x08) +
				   ((uid_port >>  6) & 0x10);
	return uid;
}

uint16_t ntb_mode_read()
{
  // Bit mapping:
  // 0 - PC9
  // 1 - PC8
  // 2 - PC7
  uint16_t mode_port = GPIO_ReadInputData(MODE_PORT);
  uint16_t mode = ((mode_port >> 9) & 0x01) + ((mode_port >> 7) & 0x02) + ((mode_port >> 5) & 0x04);
  return mode;

}


// LED functions
void ntb_led_on(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_SetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    case NTB_LED_BRIGHT_GREEN:
      GPIO_SetBits(LED_BRIGHT_PORT, LED_BRIGHT_GREEN);
      break;
    case NTB_LED_BRIGHT_RED:
      GPIO_SetBits(LED_BRIGHT_PORT, LED_BRIGHT_RED);
      break;
    case NTB_LED_BRIGHT_BLUE:
      GPIO_SetBits(LED_BRIGHT_PORT, LED_BRIGHT_BLUE);
      break;
    case NTB_LED_BRIGHT_ORANGE:
      GPIO_SetBits(LED_BRIGHT_PORT, LED_BRIGHT_ORANGE);
      break;
    default:
      break;
  }
}

void ntb_led_off(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_ResetBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    case NTB_LED_BRIGHT_GREEN:
      GPIO_ResetBits(LED_BRIGHT_PORT, LED_BRIGHT_GREEN);
      break;
    case NTB_LED_BRIGHT_RED:
      GPIO_ResetBits(LED_BRIGHT_PORT, LED_BRIGHT_RED);
      break;
    case NTB_LED_BRIGHT_BLUE:
      GPIO_ResetBits(LED_BRIGHT_PORT, LED_BRIGHT_BLUE);
      break;
    case NTB_LED_BRIGHT_ORANGE:
      GPIO_ResetBits(LED_BRIGHT_PORT, LED_BRIGHT_ORANGE);
    default:
      break;
  }
}

void ntb_led_toggle(uint8_t led)
{
  switch(led)
  {
    case NTB_LED_ARM_GREEN:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_GREEN);
      break;
    case NTB_LED_ARM_RED:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_RED);
      break;
    case NTB_LED_ARM_BLUE:
      GPIO_ToggleBits(LED_ARM_PORT, LED_ARM_BLUE);
      break;
    case NTB_LED_BRIGHT_GREEN:
      GPIO_ToggleBits(LED_BRIGHT_PORT, LED_BRIGHT_GREEN);
      break;
    case NTB_LED_BRIGHT_RED:
      GPIO_ToggleBits(LED_BRIGHT_PORT, LED_BRIGHT_RED);
      break;
    case NTB_LED_BRIGHT_BLUE:
      GPIO_ToggleBits(LED_BRIGHT_PORT, LED_BRIGHT_BLUE);
      break;
    case NTB_LED_BRIGHT_ORANGE:
      GPIO_ToggleBits(LED_BRIGHT_PORT, LED_BRIGHT_ORANGE);
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
       ntb_led_on(NTB_LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_on(NTB_LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_on(NTB_LED_ARM_BLUE);

       // Ramp off
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_GREEN);
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_RED);
       ntb_busywait_ms(150);
       ntb_led_off(NTB_LED_ARM_BLUE);
  }
}


void ntb_gpio_on(uint8_t pin)
{
	switch(pin)
	{
		case NTB_GPIO_0:
			GPIO_SetBits(GPIO_PORT, GPIO_P0);
			break;
		case NTB_GPIO_1:
			GPIO_SetBits(GPIO_PORT, GPIO_P1);
			break;
		case NTB_GPIO_2:
			GPIO_SetBits(GPIO_PORT, GPIO_P2);
			break;
		case NTB_GPIO_3:
			GPIO_SetBits(GPIO_PORT, GPIO_P3);
			break;
		case NTB_GPIO_RST:
			GPIO_SetBits(RST_PORT, RST_PIN);
			break;
		default:
			break;
	}
}

void ntb_gpio_off(uint8_t pin)
{
	switch(pin)
	{
		case NTB_GPIO_0:
			GPIO_ResetBits(GPIO_PORT, GPIO_P0);
			break;
		case NTB_GPIO_1:
			GPIO_ResetBits(GPIO_PORT, GPIO_P1);
			break;
		case NTB_GPIO_2:
			GPIO_ResetBits(GPIO_PORT, GPIO_P2);
			break;
		case NTB_GPIO_3:
			GPIO_ResetBits(GPIO_PORT, GPIO_P3);
			break;
		case NTB_GPIO_RST:
			GPIO_ResetBits(RST_PORT, RST_PIN);
			break;
		default:
			break;
	}
}

void ntb_gpio_toggle(uint8_t pin)
{
	switch(pin)
	{
		case NTB_GPIO_0:
			GPIO_ToggleBits(GPIO_PORT, GPIO_P0);
			break;
		case NTB_GPIO_1:
			GPIO_ToggleBits(GPIO_PORT, GPIO_P1);
			break;
		case NTB_GPIO_2:
			GPIO_ToggleBits(GPIO_PORT, GPIO_P2);
			break;
		case NTB_GPIO_3:
			GPIO_ToggleBits(GPIO_PORT, GPIO_P3);
			break;
		case NTB_GPIO_RST:
			GPIO_ToggleBits(RST_PORT, RST_PIN);
			break;
		default:
			break;
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

void ntb_ethswitch_rst()
{
  ntb_ethswitch_off();
  ntb_busywait_ms(10);
  ntb_ethswitch_on();
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

void ntb_pmeas_read(uint8_t rail, float *v, float *i, float *p)
{
  float FSC = 0;
  uint8_t vsense_reg, vsource_reg, power_reg;
  uint8_t tmp_buf16[2];

  switch(rail)
  {
    case NTB_POW_3V3:
      // full scale current
      FSC = 80.0e-3/PMEAS_RSENSE_3V3;
      // Vsense reg
      vsense_reg = PMEAS_REG_VS1HI;
      // Vsource reg
      vsource_reg = PMEAS_REG_V1HI;
      // Power reg
      power_reg = PMEAS_REG_PR1HI;
      break;
    case NTB_POW_5V0:
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


void ntb_pctrl_on(uint8_t rail)
{
  switch(rail)
  {
    case NTB_POW_3V3:
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case NTB_POW_5V0:
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_off(uint8_t rail)
{
  switch(rail)
  {
    case NTB_POW_3V3:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case NTB_POW_5V0:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_toggle(uint8_t rail)
{
  switch(rail)
  {
    case NTB_POW_3V3:
      GPIO_ToggleBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case NTB_POW_5V0:
      GPIO_ToggleBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

void ntb_pctrl_rst(uint8_t rail)
{
  switch(rail)
  {
    case NTB_POW_3V3:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      ntb_busywait_ms(20);
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_3V3);
      break;
    case NTB_POW_5V0:
      GPIO_ResetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      ntb_busywait_ms(20);
      GPIO_SetBits(PCTRL_PORT, PCTRL_SHTDN_5V0);
      break;
    default:
      break;
  }
}

// Note!! This conflicts with DW operation and does not use a mutex, so we MUST
// ensure that the two do not collide. We also must ensure this SPI engine is configured.
uint8_t ntb_ethswitch_trx(uint8_t byte)
{  
  while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
  SPI_I2S_SendData(SPI3, byte);
  while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE));
  return SPI_I2S_ReceiveData(SPI3);
}

// write ethernet word
void ntb_ethswitch_write16(uint16_t reg_addr, uint16_t data)
{
  // find closest address with 00 as last 2 bits
  uint16_t byte_offset = (reg_addr % 4);
  uint16_t addr_base = reg_addr - byte_offset;
  uint8_t bytemask = (ETHSWITCH_BYTE0 | ETHSWITCH_BYTE1) << byte_offset;

  // construct and send packet to ethernet switch
  // structure: [read = 0] [9-bit reg. addr.] [4 bit byte enable] [2 bit TA] [1-4 Bytes payload]
  uint8_t cmd_hi, cmd_lo;
  // high byte is read (0) and then top 7 bits of 11-bit address
  cmd_hi = 0x80 | ( (addr_base >> 4) & 0x7F);
  // low byte is bits 3 and 2 of address followed by 4-bit bitmask and 2 DCs
  cmd_lo = (((addr_base >> 2) & 0x03) << 6) | (bytemask << 2);

  // select ethernet switch CS
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

  ntb_ethswitch_trx(cmd_hi);
  ntb_ethswitch_trx(cmd_lo);
  ntb_ethswitch_trx( (uint8_t)(data     ) );
  ntb_ethswitch_trx( (uint8_t)(data >> 8) );

  // deselect ethernet switch CS
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_CS);
}

// write ethernet byte
void ntb_ethswitch_write8(uint16_t reg_addr, uint8_t data)
{
  // find closest address with 00 as last 2 bits
  uint16_t byte_offset = (reg_addr % 4);
  uint16_t addr_base = reg_addr - byte_offset;
  uint8_t bytemask = (ETHSWITCH_BYTE0) << byte_offset;

  // construct and send packet to ethernet switch
  // structure: [read = 0] [9-bit reg. addr.] [4 bit byte enable] [2 bit TA] [1-4 Bytes payload]
  uint8_t cmd_hi, cmd_lo;
  // high byte is read (0) and then top 7 bits of 11-bit address
  cmd_hi = 0x80 | ( (addr_base >> 4) & 0x7F);
  // low byte is bits 3 and 2 of address followed by 4-bit bitmask and 2 DCs
  cmd_lo = (((addr_base >> 2) & 0x03) << 6) | (bytemask << 2);

  // select ethernet switch CS
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_CS);
  
  ntb_ethswitch_trx(cmd_hi);
  ntb_ethswitch_trx(cmd_lo);
  ntb_ethswitch_trx(data);

  // deselect ethernet switch CS
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_CS);
}

// read ethernet word
uint16_t ntb_ethswitch_read16(uint16_t reg_addr)
{
  // find closest address with 00 as last 2 bits
  uint16_t byte_offset = (reg_addr % 4);
  uint16_t addr_base = reg_addr - byte_offset;
  uint8_t bytemask = (ETHSWITCH_BYTE0 | ETHSWITCH_BYTE1) << byte_offset;

  // construct and send packet to ethernet switch
  // structure: [read = 0] [9-bit reg. addr.] [4 bit byte enable] [2 bit TA] [1-4 Bytes payload]
  uint8_t cmd_hi, cmd_lo;
  // high byte is read (0) and then top 7 bits of 11-bit address
  cmd_hi = 0x00 | ( (addr_base >> 4) & 0x7F);
  // low byte is bits 3 and 2 of address followed by 4-bit bitmask and 2 DCs
  cmd_lo = (((addr_base >> 2) & 0x03) << 6) | (bytemask << 2);

  // select ethernet switch CS
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

  ntb_ethswitch_trx(cmd_hi);
  ntb_ethswitch_trx(cmd_lo);
  uint8_t lo = ntb_ethswitch_trx(0x00);
  uint8_t hi = ntb_ethswitch_trx(0x00);

  // deselect ethernet switch CS
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

  return lo + (hi<<8);
}

// read ethernet word
uint8_t ntb_ethswitch_read8(uint16_t reg_addr)
{
  // find closest address with 00 as last 2 bits
  uint16_t byte_offset = (reg_addr % 4);
  uint16_t addr_base = reg_addr - byte_offset;
  uint8_t bytemask = (ETHSWITCH_BYTE0) << byte_offset;

  // construct and send packet to ethernet switch
  // structure: [read = 0] [9-bit reg. addr.] [4 bit byte enable] [2 bit TA] [1-4 Bytes payload]
  uint8_t cmd_hi, cmd_lo;
  // high byte is read (0) and then top 7 bits of 11-bit address
  cmd_hi = 0x00 | ( (addr_base >> 4) & 0x7F);
  // low byte is bits 3 and 2 of address followed by 4-bit bitmask and 2 DCs
  cmd_lo = (((addr_base >> 2) & 0x03) << 6) | (bytemask << 2);

  // select ethernet switch CS
  GPIO_ResetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

  ntb_ethswitch_trx(cmd_hi);
  ntb_ethswitch_trx(cmd_lo);
  uint8_t lo = ntb_ethswitch_trx(0x00);

  // deselect ethernet switch CS
  GPIO_SetBits(ETHSWITCH_PORT, ETHSWITCH_CS);

  return lo;
}

// set up GPIO 3 for immediate GPIO control (LED)
void ntb_ethswitch_led_on()
{
  uint16_t tmp;

  // Disable trigger output unit 1
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG_EN, 0x0000);

  // Set up for immediate triggering (trigger now) and + edge
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG1_CFG_1, 0x3E13);

  // Enable trigger output unit 1
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG_EN, 0x0001);
}

// set up GPIO 3 for immediate GPIO control (LED)
void ntb_ethswitch_led_off()
{
  uint16_t tmp;

  // Disable trigger output unit 1
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG_EN, 0x0000);

  // Set up for immediate triggering (trigger now) and - edge
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG1_CFG_1, 0x3E03);

  // Enable trigger output unit 1
  ntb_ethswitch_write16(ETHSWITCH_REG_TRIG_EN, 0x0001);
}



// set eth switch to p2p transparent clock mode
void ntb_ethswitch_set_tclk()
{
  // intialize clock (reset, load clock, enable clock)
  // reset = 1, enable = 0
  ntb_ethswitch_write8(ETHSWITCH_REG_PTP_CLK_CTL, 0x01);
  ntb_busywait_ms(10);
  // load the ptp clock
  ntb_ethswitch_write8(ETHSWITCH_REG_PTP_CLK_CTL, 0x08);
  ntb_busywait_ms(10);
  // enable the clock
  ntb_ethswitch_write8(ETHSWITCH_REG_PTP_CLK_CTL, 0x02);
  ntb_busywait_ms(10);

  // eth PTP, E2E TC, One-step (for some reason this is the only thing that works with P2P)
  ntb_ethswitch_write8(ETHSWITCH_REG_PTP_MSG_CFG_1, 0x79);

  // disable alternate master (also don't know why this works)
  ntb_ethswitch_write16(ETHSWITCH_REG_PTP_MSG_CFG_2, 0x0004);

}


void ntb_soft_reset()
{
	NVIC_SystemReset();
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