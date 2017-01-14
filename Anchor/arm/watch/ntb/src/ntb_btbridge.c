#include "ntb_btbridge.h"
#include "stm32f30x.h"
#include <string.h>

// fcn return types
#define RETURN_ERROR (0)
#define RETURN_BYTEADDED (1)
#define RETURN_

// message framing
#define MSG_START ('S')
#define MSG_END ('E')

// rx buffer
#define MAX_RX_LEN (sizeof(bt_rx_msg_t))
uint8_t rxbuffer[MAX_RX_LEN];
uint8_t rxbuf_size = 0;

// tx buffer
#define MAX_TX_LEN (sizeof(bt_tx_msg_t))
uint8_t txbuffer[MAX_TX_LEN];

// message states
#define RXMSG_STATE_IDLE (0)
#define RXMSG_STATE_HEADER (1)
#define RXMSG_STATE_PAYLOAD (2)
#define RXMSG_STATE_ENDING (3)
uint8_t rxmsg_state = RXMSG_STATE_IDLE;

// callback user function
void (*user_cb_fcn)(bt_rx_msg_t msg);

// non-public functions
int advance_rxmsg_state(uint8_t c);
void uart_putc(uint8_t c);
int rxbuf_putc(uint8_t c);
void rxbuf_clear();

// send byte over uart
void uart_putc(uint8_t c)
{
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    USART_SendData(USART2, c);
}

// clear out and reset the rx buffer
void rxbuf_clear()
{
	// state vars
	rxmsg_state = RXMSG_STATE_IDLE;
	// buffer vars
	rxbuf_size = 0;
}

// add a byte from UART to buffer
int rxbuf_putc(uint8_t c)
{
	if( rxbuf_size < MAX_RX_LEN )
	{
		rxbuffer[rxbuf_size++] = c;
		return rxbuf_size;
	}
	else
	{
		// 0 = fail
		return 0;
	}
}


/* ========== PUBLIC API FUNCTIONS ========== */
int btbridge_rx(uint8_t c)
{
	// there was an error if the buffer is full
	if( rxbuf_size >= MAX_RX_LEN )
	{
		rxbuf_clear();
		return RETURN_ERROR;
	}
	// otherwise, process new data
	switch(rxmsg_state)
	{
		case RXMSG_STATE_IDLE:
			if( c == MSG_START )
			{
				rxmsg_state = RXMSG_STATE_HEADER;
				return RETURN_BYTEADDED;
			}
			break;
		case RXMSG_STATE_HEADER:
			rxbuf_putc(c);
			rxmsg_state = RXMSG_STATE_PAYLOAD;
			return RETURN_BYTEADDED;
			break;
		case RXMSG_STATE_PAYLOAD:
			rxbuf_putc(c);
			rxmsg_state = RXMSG_STATE_ENDING;
			return RETURN_BYTEADDED;
			break;
		case RXMSG_STATE_ENDING:
			if( c == MSG_END )
			{
				user_cb_fcn( *((bt_rx_msg_t*)rxbuffer) );
				return RETURN_BYTEADDED;
			}
			break;
	}

	// otherwise we got here in error
	rxbuf_clear();
	return RETURN_ERROR;
}


void btbridge_send(bt_tx_msg_t *msg)
{
	int i;
	memcpy(txbuffer, msg, MAX_TX_LEN);
	// send frame start
	uart_putc(MSG_START);
	// send msg
	for( i=0; i<MAX_TX_LEN; i++ )
	{
		uart_putc( txbuffer[i] );
	}
	// send frame end
	uart_putc(MSG_END);
}

void btbridge_register_cb(void (*cbfcn)(bt_rx_msg_t msg))
{
	user_cb_fcn = cbfcn;
}

