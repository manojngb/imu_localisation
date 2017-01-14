/**
 ******************************************************************************
 * @file    usbd_cdc_vcp.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    22-July-2011
 * @brief   Generic media access Layer.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usb_vcp_api.h"
#include "stm32f4xx_conf.h"
#include "stm32f4_discovery.h"

// TX message buffer
uint8_t VCP_MessageBuffer[MAX_VCP_MESSAGES][MAX_VCP_MESSAGE_SIZE];
uint16_t VCP_NumQueuedMessages = 0;
uint16_t VCP_MessageBufferHead = 0;
uint16_t VCP_MessageBufferTail = 0;

uint16_t VCP_FlushMessages()
{
  int byte_idx = 0;

  while( VCP_NumQueuedMessages > 0 )
  {
    byte_idx = 0;
    while( VCP_MessageBuffer[VCP_MessageBufferTail][byte_idx] != '\0' && byte_idx < MAX_VCP_MESSAGE_SIZE )
    {
      VCP_TxByte(VCP_MessageBuffer[VCP_MessageBufferTail][byte_idx++]);
    }

    // Add newline and carriage returns
    VCP_TxByte('\n');
    VCP_TxByte('\r');

    // get new message index
    VCP_MessageBufferTail = (VCP_MessageBufferTail + 1) % MAX_VCP_MESSAGES;
    VCP_NumQueuedMessages--;
  }

  return USBD_OK;
}

void VCP_AddMessage(uint8_t* bytes, uint16_t len)
{
  // add at the head
  if(len + 1 < MAX_VCP_MESSAGE_SIZE)
  {
    memcpy(VCP_MessageBuffer[VCP_MessageBufferHead], bytes, len);
  }

  // add a null-terminator
  VCP_MessageBuffer[VCP_MessageBufferHead][len] = '\0';

  // increment head and check for overflow / wrap-around
  VCP_MessageBufferHead = (VCP_MessageBufferHead + 1) % MAX_VCP_MESSAGES;
  if( VCP_NumQueuedMessages < MAX_VCP_MESSAGES )
  {
    VCP_NumQueuedMessages++;
  }
  else
  {
    VCP_MessageBufferTail = (VCP_MessageBufferTail + 1) % MAX_VCP_MESSAGES;
  }
}


// transmit a null-terminated string. Up to callers of this fcn to null-terminate
uint16_t VCP_TxStr(uint8_t *str)
{
   uint16_t i = 0;
   uint8_t bytes[MAX_VCP_MESSAGE_SIZE];

   // Tx String
   while ( str[i] != '\0' ) 
   {
      bytes[i] = str[i++];
   }
   VCP_AddMessage(bytes, i);

   return USBD_OK;
}

uint16_t VCP_TxStrInt(uint8_t *str, const int num)
{
   uint16_t len = 0;
   uint32_t i = 0;
   uint8_t bytes[MAX_VCP_MESSAGE_SIZE];

   // Tx String
   while ( str[i] != '\0' ) 
   {
      bytes[len++] = str[i++];
   }

   // Tx integer
   uint8_t intbuf[MAX_INT_BUFFER];
   VCP_TxStr( itoa(num, intbuf, 10) );
   uint32_t j = 0;
   while ( intbuf[j] != '\0' )
   {
      bytes[len++] = intbuf[j++];
   }

   // Add to Queue
   VCP_AddMessage(bytes, len);

   return USBD_OK;
}

uint16_t VCP_TxStrLong(uint8_t *str, uint32_t bytes)
{
  uint8_t buf[4] = { (uint8_t)(bytes>>24), (uint8_t)(bytes>>16), (uint8_t)(bytes>>8), (uint8_t)(bytes) };
  VCP_TxStrBytes( str, buf, 4);

  return USBD_OK;
}

uint16_t VCP_TxStrFloat(uint8_t *str, float num)
{
  uint8_t buf[16];
  int i = 0;
  int len = 0;
  int intVal = (int)num;
  int decVal = (int)(num*10000) % 10000;

  uint8_t bytes[MAX_VCP_MESSAGE_SIZE];

  // Tx String
  while ( str[i] != '\0' ) 
  {
    bytes[len++] = str[i++];
  }

  // Tx integer
  uint8_t intbuf[MAX_INT_BUFFER];
  itoa(intVal, intbuf, 10);
  uint32_t j = 0;
  while ( intbuf[j] != '\0' )
  {
    bytes[len++] = intbuf[j++];
  }

  // decimal point
  bytes[len++] = '.';

  // Tx decimal
  uint8_t decbuf[MAX_INT_BUFFER];
  itoa(decVal, decbuf, 10);
  j = 0;
  while ( decbuf[j] != '\0' )
  {
    bytes[len++] = decbuf[j++];
  }

  // Add to Queue
  VCP_AddMessage(bytes, len);

  return USBD_OK;
}


uint16_t VCP_TxStrBytes(uint8_t *str, const uint8_t* bytes, const int len)
{
   uint32_t qlen = 0;
   uint32_t i,j = 0;
   uint8_t to_queue[MAX_VCP_MESSAGE_SIZE];

   // Tx String
   while ( str[i] != '\0' ) 
   {
      to_queue[qlen++] = str[i++];
   }

   // Tx Bytes
   uint8_t intbuf[MAX_INT_BUFFER];
   for( i=0; i<len; i++ )
   {
    // convert byte to string
    itoa(bytes[i], intbuf, 10);

    // add string to queue buffer
    for( j=0; j<(MAX_INT_BUFFER); j++ )
    {
      if( intbuf[j] == '\0' )
        break;
      to_queue[qlen++] = intbuf[j];
    }
    to_queue[qlen++] = ' ';
   }

   // Add to Queue
   VCP_AddMessage(to_queue, qlen);

   return USBD_OK;
}

// Implementation of itoa()
uint8_t* itoa(int num, uint8_t* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled only with 
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

/* A utility function to reverse a string  */
void reverse(uint8_t str[], int length)
{
    int start = 0;
    int end = length -1;
    uint8_t tmp;
    while (start < end)
    {
        tmp = *(str+end);
        *(str+end) = *(str+start);
        *(str+start) = tmp; 
        start++;
        end--;
    }
}