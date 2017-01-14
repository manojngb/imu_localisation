#include "stm32f4xx_it.h"
#include "stm32f4xx_conf.h"
#include "main.h"

#include "usb_core.h"
#include "usbd_core.h"
#include "usbd_cdc_core.h"

// decawave support
//#include "deca_port.h"

extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern USB_OTG_CORE_HANDLE USB_OTG_dev;

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
extern uint32_t USBD_OTG_EP1IN_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern uint32_t USBD_OTG_EP1OUT_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
#endif

/* System tick 32 bit variable defined by the platform */
__IO unsigned long time32_incr;

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  /* Go to infinite loop when NMI Fault exception occurs */
  while (1)
  {
  }
}


/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void pop_registers_from_fault_stack(unsigned int * hardfault_args)
{
  /* Go to infinite loop when Stack Fault exception occurs */
  while (1)
  {
  }
}


/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
  /* Go to infinite loop when SVCall Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
  /* Go to infinite loop when DebugMon Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
  /* Go to infinite loop when PendSV Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  /* IT_SEC is gone on F4...
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    // Clear the RTC Second interrupt
    RTC_ClearITPendingBit(RTC_IT_SEC);

    // Wait until last write operation on RTC registers has finished
    RTC_WaitForLastTask();
  }
  */
}


/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{

}

void EXTI0_IRQHandler(void)
{

}

// Handle DW1000 interrupts (IRQ)
void EXTI2_IRQHandler(void)
{
  /* Make sure that interrupt flag is set */
  if (EXTI_GetITStatus(EXTI_Line2) != RESET) {

    deca_ranging_isr();

    /* Clear interrupt flag */
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

void EXTI3_IRQHandler(void)
{
   // process_deca_irq();
    /* Clear EXTI Line 3 Pending Bit */
   // EXTI_ClearITPendingBit(EXTI_Line3);
}

void EXTI9_5_IRQHandler(void)
{
    //process_deca_irq();
    /* Clear EXTI Line 8 Pending Bit */
    //EXTI_ClearITPendingBit(DECAIRQ_EXTI);
}
