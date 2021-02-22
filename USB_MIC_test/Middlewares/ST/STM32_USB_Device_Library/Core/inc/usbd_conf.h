

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

/* Includes ------------------------------------------------------------------*/
//#include "stm32f4_discovery.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#define USBD_IN_AUDIO_FREQ                (uint16_t)(16000) //MIC frequency in Hz.

/* Audio frequency in Hz */
#ifndef EXTERNAL_CRYSTAL_25MHz
 #define USBD_AUDIO_FREQ                (uint16_t)(16000) /* Audio frequency in Hz.
                                                 It is advised to set standard frequencies >= 24KHz to get best quality
                                                 except for STM32F10 devices, when the HSE value is 25MHz, it is advised to
                                                 set audio frequencies reachable with this HSE value (refer to RM0008 for
                                                 more details). ie. it is advised to set 16KHz value in this case. 
                                                 Note that maximum allowed audio frequency is 96KHz (this limitation is 
                                                 due to the codec used on the Evaluation board. The STM32 I2S cell allows
                                                 to  reach 192KHz frequency).
                                                 @note
                                                    When modifying this value, make sure that the I2S PLL configuration allows
                                                    to get minimum error on audio frequency. This configuration is set in file
                                                    system_stm32f2xx.c or system_stm32f10x.c.*/
#else
 #define USBD_AUDIO_FREQ                16000  /* Audio frequency in Hz for STM32F10x devices family when 25MHz HSE value
                                                  is used. */
#endif /* EXTERNAL_CRYSTAL_25MHz */

#define DEFAULT_VOLUME                  80    /* Default volume in % (Mute=0%, Max = 100%) in Logarithmic values.
                                                 To get accurate volume variations, it is possible to use a logarithmic
                                                 coversion table to convert from percentage to logarithmic law.
                                                 In order to keep this example code simple, this conversion is not used.*/
#define AUDIO_TOTAL_IF_NUM              0x02
#define USBD_CFG_MAX_NUM                1
#define USBD_ITF_MAX_NUM                1
#define USB_MAX_STR_DESC_SIZ            200 

#define MSC_INTERFACE_IDX 0x01                            	// Index of MSC interface
#define AUDIO_INTERFACE_IDX 0x0                            	// Index of CDC interface
 
// endpoints numbers
// endpoints numbers
#define MSC_EP_IDX                      0x02
#define AUDIO_EP_IDX                      0x01

#define IN_EP_DIR						0x80 // Adds a direction bit

#define MSC_OUT_EP                      MSC_EP_IDX                  /* EP1 for BULK OUT */
#define MSC_IN_EP                       MSC_EP_IDX | IN_EP_DIR      /* EP1 for BULK IN */

#define AUDIO_OUT_EP                      AUDIO_EP_IDX                  /* EP3 for data OUT */
#define AUDIO_IN_EP                       AUDIO_EP_IDX | IN_EP_DIR      /* EP3 for data IN */



//#define AUDIO_IN_EP                     0x81

//#define MSC_IN_EP                    0x81
//#define MSC_OUT_EP                   0x01


#define MSC_MAX_PACKET               64
#define MSC_MEDIA_PACKET             4096
#define MSC_EPIN_SIZE                MSC_MAX_PACKET 
#define MSC_EPOUT_SIZE               MSC_MAX_PACKET                                                    /* Memory management macros */

/** Alias for memory allocation. */
#define USBD_malloc         malloc

/** Alias for memory release. */
#define USBD_free           free

/** Alias for memory set. */
#define USBD_memset         memset

/** Alias for memory copy. */
#define USBD_memcpy         memcpy

/** Alias for delay. */
#define USBD_Delay          HAL_Delay
#endif //__USBD_CONF__H__

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

