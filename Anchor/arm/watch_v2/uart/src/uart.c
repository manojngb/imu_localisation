// includes
#include "uart.h"
#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
// private API
void uart_tx_byte(uint8_t b);
void uart_tx_array(uint8_t *ba, uint16_t len);

void uart_tx_range(uint16_t ancId, float rangeCm)
{
	// package into data unit
	uart_rng_t rng_data = {.aid=ancId, .range_cm=(uint16_t)rangeCm };
	uart_payload_t payload;
	payload.rng = rng_data;
	uart_data_t data = {.type=PACKET_TYPE_RNG, .payload=payload};

	// package into frame
	uart_frame_t frame = {.start=PACKET_START, .data=data, .stop=PACKET_STOP};

	// send frame bytes
	uart_tx_array((uint8_t*)&frame, PACKET_LEN);
}

void uart_tx_byte(uint8_t b)
{
	// wait for tx buffer to be ready
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
	// send byte
	USART_SendData(UART4, b);
}

void uart_tx_array(uint8_t *ba, uint16_t len)
{
	int i;
	for(i=0; i<len; i++)
	{
		uart_tx_byte(ba[i]);
	}
}