/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 */
#ifndef __NTB_TCPSERVER_H__
#define __NTB_TCPSERVER_H__

// NTB server options
#define NTB_TCP_PORT (23458)
#define NTB_TCP_MAXCLIENTS (4)

// client streaming options
#define NTB_TCPOPT_STREAM_RANGE     (0x01)
#define NTB_TCPOPT_STREAM_POW_1     (0x02)
#define NTB_TCPOPT_STREAM_POW_10    (0x04)
#define NTB_TCPOPT_STREAM_UART      (0x08)

// client read/query options
#define NTB_TCPOPT_QUERY_UID        (0x10)

#include "lwip/tcp.h"
#include <stdint.h>


 /* ECHO protocol states */
enum ntb_tcpserver_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
typedef struct
{
  uint8_t clientId;
  uint8_t state;          /* current connection state */
  struct tcp_pcb *pcb;    /* pointer to the current tcp_pcb */
  struct pbuf *pin;		  /* incoming pbuf pointer */
  struct pbuf *pout;      /* outgoing pbuf pointer */
  uint8_t options;		  /* client-specific NTB streaming options */
} ntb_tcpclient_t;


void ntb_tcpserver_init(void);
void ntb_tcpserver_setcallback(void (*callback_fcn)(uint8_t * cmd, int len, ntb_tcpclient_t* clnt));
void ntb_tcpserver_send(struct tcp_pcb *tpcb, ntb_tcpclient_t *clnt);
void ntb_bcast_tcp(uint8_t* data, int len, uint8_t opt_mask);
void ntb_ucast_tcp(uint8_t* data, int len, ntb_tcpclient_t* clnt);

#endif /* __NTB_TCPSERVER_H__ */
