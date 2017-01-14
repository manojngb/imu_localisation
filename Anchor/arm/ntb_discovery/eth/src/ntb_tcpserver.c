/**
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
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo example.
 *
 **/

 /* This file was modified by ST */


#include "lwip/debug.h"
#include "lwip/stats.h"
#include "ntb_tcpserver.h"
#include <stdint.h>

// temporary
#include "ntb_hwctrl.h"

#if LWIP_TCP

static struct tcp_pcb *ntb_tcpserver_pcb;


static err_t ntb_tcpserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t ntb_tcpserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void ntb_tcpserver_error(void *arg, err_t err);
static err_t ntb_tcpserver_poll(void *arg, struct tcp_pcb *tpcb);
static err_t ntb_tcpserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void ntb_tcpserver_connection_close(struct tcp_pcb *tpcb, ntb_tcpclient_t *es);

// client list functions
static void add_client(ntb_tcpclient_t* client);
static void remove_client(ntb_tcpclient_t* client);

// TCP client list
ntb_tcpclient_t* client_list[NTB_TCP_MAXCLIENTS];
uint8_t client_id = 0;
uint8_t num_clients = 0;
// TCP command callback
void (*user_tcpcallback)(uint8_t * cmd, int len, ntb_tcpclient_t* es);


void ntb_bcast_tcp(uint8_t* data, int len, uint8_t opt_mask)
{
	int i, j;

	// add client to empty slot
	for( i=0; i<NTB_TCP_MAXCLIENTS; i++)
	{
		if( client_list[i] != NULL )
		{
			ntb_tcpclient_t* clnt = client_list[i];

			if( clnt->options & opt_mask )
			{
				/* initialize LwIP tcp_sent callback function */
    			tcp_sent(clnt->pcb, ntb_tcpserver_sent);

    			/* Create new pbuf and append to output chain if need be */
				struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
				for( j=0; j<len; j++ )
					((uint8_t*)p->payload)[j] = data[j];

				if( clnt->pout == NULL )
					clnt->pout = p;
				else
					pbuf_chain(clnt->pout, p);

    			ntb_tcpserver_send(clnt->pcb, clnt);
			}
		}
	}
}

void ntb_ucast_tcp(uint8_t* data, int len, ntb_tcpclient_t* clnt)
{
	int i;

	/* initialize LwIP tcp_sent callback function */
	tcp_sent(clnt->pcb, ntb_tcpserver_sent);

	/* Create new pbuf and append to output chain if need be */
	struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_POOL);
	for( i=0; i<len; i++ )
		((uint8_t*)p->payload)[i] = data[i];

	if( clnt->pout == NULL )
		clnt->pout = p;
	else
		pbuf_chain(clnt->pout, p);

	ntb_tcpserver_send(clnt->pcb, clnt);
}

uint8_t ntb_tcp_allstreamopts()
{
  int i, j;
  uint8_t opts = 0;

  // add client to empty slot
  for( i=0; i<NTB_TCP_MAXCLIENTS; i++)
  {
    if( client_list[i] != NULL )
    {
      ntb_tcpclient_t* clnt = client_list[i];

      opts |= clnt->options;
    }
  }

  return opts;
}


static void add_client(ntb_tcpclient_t* client)
{
	int i;
	// add client to empty slot
	for( i=0; i<NTB_TCP_MAXCLIENTS; i++)
	{
		if( client_list[i] == NULL )
		{
			client_list[i] = client;
			return;
		}
	}
}

static void remove_client(ntb_tcpclient_t* client)
{
	int i;
	// remove pointer to this client from list
	for( i=0; i<NTB_TCP_MAXCLIENTS; i++)
	{
		if( client_list[i] == client )
		{
			client_list[i] = NULL;
			return;
		}
	}
}

/**
  * @brief  Initializes the tcp echo server
  * @param  None
  * @retval None
  */
void ntb_tcpserver_init(uint16_t uid)
{
  /* create new tcp pcb */
  ntb_tcpserver_pcb = tcp_new();

  if (ntb_tcpserver_pcb != NULL) {
    err_t err;
    
    /* bind pcb to port 23458 (NTB protocol) */
    err = tcp_bind(ntb_tcpserver_pcb, IP_ADDR_ANY, NTB_TCP_PORT);
    
    if (err == ERR_OK) {
      /* start tcp listening for echo_pcb */
      ntb_tcpserver_pcb = tcp_listen(ntb_tcpserver_pcb);
      
      /* initialize LwIP tcp_accept callback function */
      tcp_accept(ntb_tcpserver_pcb, ntb_tcpserver_accept);
    } else {
      //printf("Can not bind pcb\n");
    }
  } else {
    //printf("Can not create new pcb\n");
  }
}

void ntb_tcpserver_setcallback(void (*callback_fcn)(uint8_t * cmd, int len, ntb_tcpclient_t* clnt))
{
	user_tcpcallback = callback_fcn;
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used 
  * @retval err_t: error status
  */
static err_t ntb_tcpserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  if( num_clients >= NTB_TCP_MAXCLIENTS )
    return ERR_MEM;

  num_clients++;

  err_t ret_err;
  ntb_tcpclient_t *clnt;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  /* allocate client structure to maintain tcp connection information */
  clnt = (ntb_tcpclient_t *)mem_malloc(sizeof(ntb_tcpclient_t));
  if (clnt != NULL) {
    clnt->state = ES_ACCEPTED;
    clnt->pcb = newpcb;
    clnt->pout = NULL;
    clnt->pin = NULL;
    clnt->clientId = ++client_id;
    clnt->options = 0x00;
    
    /* pass newly allocated es structure as argument to newpcb */
    tcp_arg(newpcb, clnt);
    
    /* initialize lwip tcp_recv callback function for newpcb  */ 
    tcp_recv(newpcb, ntb_tcpserver_recv);
    
    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, ntb_tcpserver_error);
    
    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, ntb_tcpserver_poll, 1);

    /* add pointer to this connection in the client list */
    add_client(clnt);
    
    ret_err = ERR_OK;
  } else {
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;  
}


/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */
static err_t ntb_tcpserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  int i;

  ntb_tcpclient_t *clnt;
  err_t ret_err;

  LWIP_ASSERT("arg != NULL",arg != NULL);
  
  clnt = (ntb_tcpclient_t *)arg;
  
  /* if we receive an empty tcp frame from client => close connection */
  if (p == NULL) {
    /* remote host closed connection */
    clnt->state = ES_CLOSING;
    if (clnt->pout == NULL) {
       /* we're done sending, close connection */
       ntb_tcpserver_connection_close(tpcb, clnt);
    } else {
      /* we're not done yet */
      /* acknowledge received packet */
      tcp_sent(tpcb, ntb_tcpserver_sent);
      
      /* send remaining data*/
      ntb_tcpserver_send(tpcb, clnt);
    }
    ret_err = ERR_OK;
  }   
  /* else : a non empty frame was received from client but for some reason err != ERR_OK */
  else if(err != ERR_OK) {
    /* free received pbuf*/
    if (p != NULL) {
      //clnt->pin = NULL;
      pbuf_free(p);
    }
    ret_err = err;
  } else if(clnt->state == ES_ACCEPTED) {
    /* first data chunk in p->payload */
    clnt->state = ES_RECEIVED;

    // process received data
    user_tcpcallback((uint8_t*)p->payload, (int)p->len, clnt);

    // free this pbuf memory
	pbuf_free(p);
    
    ret_err = ERR_OK;
  } else if (clnt->state == ES_RECEIVED) {
    /* more data received from client and previous data already processed */
    if (clnt->pin == NULL) {

      // process received data
      user_tcpcallback((uint8_t*)p->payload, (int)p->len, clnt);

      // free this pbuf memory
	  pbuf_free(p);

    } else {
      /* TODO: temporarily we'll toss out the data */

      //struct pbuf *ptr;

      /* chain pbufs to the end of what we recv'ed previously  */
      //ptr = clnt->pin;
      //pbuf_chain(ptr,p);

    }
    ret_err = ERR_OK;
  }
  
  /* data received when connection already closed */
  else {
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);
    
    /* free pbuf and do nothing */
    clnt->pin = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs. 
  * @param  arg: pointer on argument parameter 
  * @param  err: not used
  * @retval None
  */
static void ntb_tcpserver_error(void *arg, err_t err)
{
  ntb_tcpclient_t *clnt;

  LWIP_UNUSED_ARG(err);

  clnt = (ntb_tcpclient_t *)arg;
  if (clnt != NULL) {
    /*  free es structure */
    mem_free(clnt);
  }
}

/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t ntb_tcpserver_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  ntb_tcpclient_t *clnt;

  clnt = (ntb_tcpclient_t *)arg;
  if (clnt != NULL) {
    if (clnt->pout != NULL) {

      /* there is a remaining pbuf (chain) , try to send data */
      ntb_tcpserver_send(tpcb, clnt);

    } else {

      /* no remaining pbuf (chain)  */
      if (clnt->state == ES_CLOSING) {
        /*  close tcp connection */
        ntb_tcpserver_connection_close(tpcb, clnt);
      }
    }
    ret_err = ERR_OK;
  } else {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  None
  * @retval None
  */
static err_t ntb_tcpserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  ntb_tcpclient_t *clnt;

  LWIP_UNUSED_ARG(len);

  clnt = (ntb_tcpclient_t *)arg;
  
  if (clnt->pout != NULL) {
    /* still got pbufs to send */
    ntb_tcpserver_send(tpcb, clnt);
  } else {
    /* if no more data to send and client closed connection*/
    if (clnt->state == ES_CLOSING) {
      ntb_tcpserver_connection_close(tpcb, clnt);
    }
  }
  return ERR_OK;
}


/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
void ntb_tcpserver_send(struct tcp_pcb *tpcb, ntb_tcpclient_t *clnt)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;
 
  while ((wr_err == ERR_OK) &&
         (clnt->pout != NULL) && 
         (clnt->pout->len <= tcp_sndbuf(tpcb)))
  {
    
    /* get pointer on pbuf from clnt structure */
    ptr = clnt->pout;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
    tcp_output(tpcb);
    
    if (wr_err == ERR_OK) {
      u16_t plen;

      plen = ptr->len;
     
      /* continue with next pbuf in chain (if any) */
      clnt->pout = ptr->next;
      
      if (clnt->pout != NULL) {
        /* increment reference count for clnt->pout */
        pbuf_ref(clnt->pout);
      }
      
      /* free pbuf: will free pbufs up to clnt->p (because clnt->p has a reference count > 0) */
      pbuf_free(ptr);

      /* Update tcp window size to be advertized : should be called when received
      data (with the amount plen) has been processed by the application layer */
      tcp_recved(tpcb, plen);

   } else if(wr_err == ERR_MEM) {
      /* we are low on memory, try later / harder, defer to poll */
     clnt->pout = ptr;
   } else {
     /* other problem ?? */
   }
  }
}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void ntb_tcpserver_connection_close(struct tcp_pcb *tpcb, ntb_tcpclient_t *clnt)
{
  
  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  /* remove from client list */
  remove_client(clnt);
  num_clients--;

  /* delete es structure */
  if (clnt != NULL) {
    mem_free(clnt);
  }  
  
  /* close tcp connection */
  tcp_close(tpcb);

}

#endif /* LWIP_TCP */
