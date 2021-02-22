/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file is responsible to offer board support package and is 
  *          configurable by user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "usbd_conf.h"
#include "stm32f4xx_hal.h"
//#include "gpio.h"
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/

/** @defgroup USB_BSP
* @brief This file is responsible to offer board support package
* @{
*/ 

/** @defgroup USB_BSP_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USB_BSP_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 





/** @defgroup USB_BSP_Private_Macros
* @{
*/ 
/**
* @}
*/ 

/** @defgroup USBH_BSP_Private_Variables
* @{
*/ 

/**
* @}
*/ 

/** @defgroup USBH_BSP_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 

/** @defgroup USB_BSP_Private_Functions
* @{
*/ 


/**
* @brief  USB_OTG_BSP_Init
*         Initilizes BSP configurations
* @param  None
* @retval None
*/

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  ;
  /* Configure SOF ID DM DP Pins */
  GPIO_InitStructure.Pin = GPIO_PIN_11;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
  
  GPIO_InitStructure.Pin = GPIO_PIN_12;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  
//  GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_OTG1_FS) ;

  
  /* Configure  VBUS Pin */
  GPIO_InitStructure.Pin = GPIO_PIN_9;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pull = GPIO_NOPULL ;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);    
  
  /* Configure ID pin */
  GPIO_InitStructure.Pin =  GPIO_PIN_10;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStructure.Pull = GPIO_PULLUP ;  
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Alternate =  GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);  


 __HAL_RCC_SYSCFG_CLK_ENABLE() ;
 RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;  
 /// RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ; 

}
/**
* @brief  USB_OTG_BSP_EnableInterrupt
*         Enabele USB Global interrupt
* @param  None
* @retval None
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
 
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_1);
  HAL_NVIC_SetPriority(OTG_FS_IRQn,1, 2);
  HAL_NVIC_EnableIRQ(OTG_FS_IRQn);


}
/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
  uint32_t count = 0;
  const uint32_t utime = ((SystemCoreClock/1000000) * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);
}


/**
* @brief  USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
  USB_OTG_BSP_uDelay(msec * 1000);   
}
/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
