/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   Header for usbd_cdc_vcp.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_VCP_API_H
#define __USB_VCP_API_H

/* Includes ------------------------------------------------------------------*/
#include "tm_stm32f4_usb_vcp.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Defines
#define MAX_INT_BUFFER ( 128 )
#define MAX_VCP_MESSAGES (16)
#define MAX_VCP_MESSAGE_SIZE (128)

#define VCP_TxByte TM_USB_VCP_Putc
#define VCP_RxByte TM_USB_VCP_Getc

uint16_t VCP_FlushMessages();
void VCP_AddMessage(uint8_t* bytes, uint16_t len);
uint16_t VCP_TxStr(uint8_t *str);
uint16_t VCP_TxStrInt(uint8_t *str, const int num);
uint16_t VCP_TxStrLong(uint8_t *str, uint32_t bytes);
uint16_t VCP_TxStrBytes(uint8_t *str, const uint8_t* bytes, const int len);
uint8_t* itoa(int num, uint8_t* str, int base);
void reverse(uint8_t str[], int length);


#endif /* __USBD_CDC_VCP_H */

