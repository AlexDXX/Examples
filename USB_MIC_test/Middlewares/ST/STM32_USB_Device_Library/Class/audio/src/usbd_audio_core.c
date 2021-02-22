#include "usbd_audio_core.h"
//#include "usbd_audio_out_if.h"
#include "usbd_msc_core.h"
#include "usbd_msc_bot.h"
#include "usbd_msc_mem.h"
/*********************************************
   AUDIO Device library callbacks
 *********************************************/
//static uint8_t  usbd_audio_Init       (void  *pdev, uint8_t cfgidx);
static uint8_t  usbd_msc_audio_Init       (void  *pdev, uint8_t cfgidx);

//static uint8_t  usbd_audio_DeInit     (void  *pdev, uint8_t cfgidx);
static uint8_t usbd_msc_audio_DeInit     (void  *pdev, uint8_t cfgidx);

static uint8_t  usbd_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);
static uint8_t  usbd_msc_audio_Setup      (void  *pdev, USB_SETUP_REQ *req);

static uint8_t  usbd_audio_EP0_RxReady(void *pdev);
static uint8_t  usbd_audio_DataIn     (void *pdev, uint8_t epnum);
static uint8_t  usbd_msc_audio_DataIn     (void *pdev, uint8_t epnum);

static uint8_t  usbd_audio_DataOut    (void *pdev, uint8_t epnum);
static uint8_t  usbd_msc_audio_DataOut    (void *pdev, uint8_t epnum);
static uint8_t  usbd_audio_SOF        (void *pdev);
static uint8_t  usbd_audio_OUT_Incplt (void  *pdev);
static uint8_t  usbd_audio_IN_Incplt (void  *pdev);


/*********************************************
   AUDIO Requests management functions
 *********************************************/
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req);
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req);
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length);
/**
  * @}
  */ 

/** @defgroup usbd_audio_Private_Variables
  * @{
  */ 
__ALIGN_BEGIN extern uint8_t  USBD_MSC_MaxLun  __ALIGN_END = 0;
__ALIGN_BEGIN static uint8_t  USBD_MSC_AltSet  __ALIGN_END = 0;
//extern volatile uint16_t Mic_To_USB_Buffer[40];

/* Main Buffer for Audio Control Rrequests transfers and its relative variables */
uint8_t  AudioCtl[64];
uint8_t  AudioCtlCmd = 0;
uint32_t AudioCtlLen = 0;
uint8_t  AudioCtlUnit = 0;

uint32_t PlayFlag = 0;

static __IO uint32_t  usbd_audio_AltSet = 0;
static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE];
extern uint8_t PlayStatus;
 uint8_t TX_buffer_state = MIC_TX_BUFFER_EMPTY;
// uint8_t TX_buffer1_state = MIC_TX_BUFFER_EMPTY;
 int16_t MicOutBuffer0[AUDIO_IN_PACKET]; //Buffer 0
// int16_t MicOutBuffer1[AUDIO_IN_PACKET]; //Buffer 1

/* AUDIO interface class callbacks structure */
USBD_Class_cb_TypeDef  AUDIO_cb = 
{
  usbd_msc_audio_Init,
  usbd_msc_audio_DeInit,
  usbd_audio_Setup,
  NULL, /* EP0_TxSent */
  usbd_audio_EP0_RxReady,
  usbd_msc_audio_DataIn,
  usbd_msc_audio_DataOut,
  usbd_audio_SOF,
  usbd_audio_IN_Incplt,
  usbd_audio_OUT_Incplt,
  USBD_audio_GetCfgDesc,
  
};

/* USB AUDIO device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t usbd_audio_CfgDesc[AUDIO_CONFIG_DESC_SIZE] __ALIGN_END =
{
    /* USB Microphone Configuration Descriptor */
  0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
  USB_CONFIGURATION_DESCRIPTOR_TYPE,                // CONFIGURATION descriptor type (0x02)
  LOBYTE(AUDIO_CONFIG_DESC_SIZE),       /* wTotalLength  109 bytes*/
  HIBYTE(AUDIO_CONFIG_DESC_SIZE),
  3,                                  // Number of interfaces in this cfg
  1,                                   // Index value of this configuration
  0,                                  // Configuration string index
  0x80,                               // Attributes, see usb_device.h
  50,                                 // Max power consumption (2X mA)

      /************* IAD to associate the two Audio interfaces *******/
  0x8,                                /* bLength */
  USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  AUDIO_INTERFACE_IDX,               /* bFirstInterface */
  2,                                 /* bInterfaceCount */
  USB_DEVICE_CLASS_AUDIO,            /* bFunctionClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,                                 /* bFunctionSubClass */
  0,                                 /* bFunctionProtocol */
  0x04,                              /* iFunction (Index of string descriptor describing this function) */

  /* USB Microphone Standard AC Interface Descriptor  */
  0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
  USB_INTERFACE_DESCRIPTOR_TYPE, // INTERFACE descriptor type
  AUDIO_INTERFACE_IDX,    // Interface Number
  0x00,                          // Alternate Setting Number
  0x00,                          // Number of endpoints in this intf
  USB_DEVICE_CLASS_AUDIO,        // Class code
  AUDIO_SUBCLASS_AUDIOCONTROL,   // Subclass code
  0x00,                          // Protocol code
  0x00,                          // Interface string index


  /* USB Microphone Class-specific AC Interface Descriptor  (CODE == 9)*/
  0x09,                         // Size of this descriptor, in bytes.
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type 0x24
  AUDIO_CONTROL_HEADER,         // HEADER descriptor subtype 0x01
  0x00,0x01,                    // Audio Device compliant to the USB Audio specification version 1.00
  0x1E,0x00,                    // Total number of bytes returned for the class-specific AudioControl interface descriptor.
                                // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
  0x01,                         // The number of AudioStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs
  0x01,                         // AudioStreaming interface 1 belongs to this AudioControl interface.

  /*USB Microphone Input Terminal Descriptor */
  0x0C,                         // Size of the descriptor, in bytes
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type
  AUDIO_CONTROL_INPUT_TERMINAL,    // INPUT_TERMINAL descriptor subtype
  0x01,                         // ID of this Terminal.
  0x01,0x02,                    // Terminal is Microphone (0x01,0x02)
  0x00,                         // No association
  0x01,                         // One channel
  0x00,0x00,                    // Mono sets no position bits
  0x00,                         // Unused.
  0x00,                         // Unused.

  /* USB Microphone Output Terminal Descriptor */
  0x09,                            // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType)
  AUDIO_CONTROL_OUTPUT_TERMINAL,   // OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
  0x02,                            // ID of this Terminal. (bTerminalID)
  0x01, 0x01,                      // USB Streaming. (wTerminalType
  0x00,                            // unused         (bAssocTerminal)
  0x01,                            // From Input Terminal.(bSourceID)
  0x00,                            // unused  (iTerminal)

  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) (CODE == 3)*/ //zero-bandwidth interface
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_INTERFACE_DESCRIPTOR_TYPE,    // INTERFACE descriptor type (bDescriptorType) 0x04
  AUDIO_INTERFACE_IDX + 1, // Index of this interface. (bInterfaceNumber) ?????????? (3<) (1<<) (1<M)
  0x00,                         // Index of this alternate setting. (bAlternateSetting)
  0x00,                         // 0 endpoints.   (bNumEndpoints)
  USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
  AUDIO_SUBCLASS_AUDIOSTREAMING, // AUDIO_STREAMING (bInterfaceSubclass)
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)

  /* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) (CODE == 4)*/
  0x09,                         // Size of the descriptor, in bytes (bLength)
  USB_INTERFACE_DESCRIPTOR_TYPE,     // INTERFACE descriptor type (bDescriptorType)
  AUDIO_INTERFACE_IDX + 1, // Index of this interface. (bInterfaceNumber)
  0x01,                         // Index of this alternate setting. (bAlternateSetting)
  0x01,                         // 1 endpoint (bNumEndpoints)
  USB_DEVICE_CLASS_AUDIO,       // AUDIO (bInterfaceClass)
  AUDIO_SUBCLASS_AUDIOSTREAMING,   // AUDIO_STREAMING (bInterfaceSubclass)
  0x00,                         // Unused. (bInterfaceProtocol)
  0x00,                         // Unused. (iInterface)

  /*  USB Microphone Class-specific AS General Interface Descriptor (CODE == 5)*/
  0x07,                         // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE, // CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
  AUDIO_STREAMING_GENERAL,         // GENERAL subtype (bDescriptorSubtype) 0x01
  0x02,             // Unit ID of the Output Terminal.(bTerminalLink)
  0x01,                         // Interface delay. (bDelay)
  0x01,0x00,                    // PCM Format (wFormatTag)

  /*  USB Microphone Type I Format Type Descriptor (CODE == 6)*/
  0x0B,                        // Size of the descriptor, in bytes (bLength)
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,// CS_INTERFACE Descriptor Type (bDescriptorType) 0x24
  AUDIO_STREAMING_FORMAT_TYPE,   // FORMAT_TYPE subtype. (bDescriptorSubtype) 0x02
  0x01,                        // FORMAT_TYPE_I. (bFormatType)
  0x01,                        // One channel.(bNrChannels)
  0x02,                        // Two bytes per audio subframe.(bSubFrameSize)
  0x10,                        // 16 bits per sample.(bBitResolution)
  0x01,                        // One frequency supported. (bSamFreqType)
  (USBD_IN_AUDIO_FREQ&0xFF),((USBD_IN_AUDIO_FREQ>>8)&0xFF),0x00,  // 16000Hz. (tSamFreq) (NOT COMPLETE!!!)

  /*  USB Microphone Standard Endpoint Descriptor (CODE == 8)*/ //Standard AS Isochronous Audio Data Endpoint Descriptor
  0x09,                       // Size of the descriptor, in bytes (bLength)
  0x05,                       // ENDPOINT descriptor (bDescriptorType)
  AUDIO_IN_EP,                    // IN Endpoint 1. (bEndpointAddress)
  USB_ENDPOINT_TYPE_ISOCHRONOUS, // Isochronous, not shared. (bmAttributes)//USB_ENDPOINT_TYPE_asynchronous USB_ENDPOINT_TYPE_ISOCHRONOUS
  (AUDIO_IN_PACKET&0xFF),((AUDIO_IN_PACKET>>8)&0xFF),                  //bytes per packet (wMaxPacketSize)
  0x01,                       // One packet per frame.(bInterval)
  0x00,                       // Unused. (bRefresh)
  0x00,                       // Unused. (bSynchAddress)

  /* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor (CODE == 7) OK - подтверждено документацией*/
  0x07,                       // Size of the descriptor, in bytes (bLength)
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,    // CS_ENDPOINT Descriptor Type (bDescriptorType) 0x25
  AUDIO_ENDPOINT_GENERAL,            // GENERAL subtype. (bDescriptorSubtype) 0x01
  0x00,                              // No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
  0x00,                              // Unused. (bLockDelayUnits)
  0x00,0x00,                         // Unused. (wLockDelay)
  
     /********************  Mass Storage interface ********************/
  0x09,   /* bLength: Interface Descriptor size */
  0x04,   /* bDescriptorType: */
  0x02,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints*/
  0x08,   /* bInterfaceClass: MSC Class */
  0x06,   /* bInterfaceSubClass : SCSI transparent command set*/
  0x50,   /* nInterfaceProtocol */
  0x05,          /* iInterface: */
  /********************  Mass Storage Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  MSC_IN_EP,   /*Endpoint address (IN, address 1) */
  0x02,   /*Bulk endpoint type */
  0x40,
  0x00,
  0x00,   /*Polling interval in milliseconds */

  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  MSC_OUT_EP,   /*Endpoint address (OUT, address 1) */
  0x02,   /*Bulk endpoint type */
  0x40,
  0x00,
  0x00,     /*Polling interval in milliseconds*/
  
 
};


/**
* @brief  usbd_audio_Init
*         Initilaizes the AUDIO interface.
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/

static uint8_t  usbd_msc_audio_Init (void  *pdev, 
                                 uint8_t cfgidx)
{  
    /* Open EP IN */
  DCD_EP_Open(pdev,
              AUDIO_IN_EP,
              (AUDIO_IN_PACKET),
              USB_OTG_EP_ISOC);

   DCD_EP_Flush(pdev,AUDIO_IN_EP);
  //USBD_MSC_DeInit(pdev , cfgidx );
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              MSC_IN_EP,
              MSC_EPIN_SIZE,
              USB_OTG_EP_BULK);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              MSC_OUT_EP,
              MSC_EPOUT_SIZE,
              USB_OTG_EP_BULK);
 
  /* Init the BOT  layer */
  MSC_BOT_Init(pdev);   
  
  
  /* Initialize the Audio Hardware layer */

  return USBD_OK;
}

/**
* @brief  usbd_audio_DeInit
*         DeInitializes the AUDIO layer.
* @param  pdev: device instance
* @param  cfgidx: Configuration index
* @retval status
*/
//static uint8_t  usbd_audio_DeInit (void  *pdev, 
//                                   uint8_t cfgidx)
//{ 
////  if(USBInterfaceModeSelect ==0){
////    USBD_MSC_DeInit((USBD_HandleTypeDef *)pdev, cfgidx);
////  }
////  else{
//  DCD_EP_Close (pdev , AUDIO_IN_EP);
//  //}
//  /* DeInitialize the Audio output Hardware layer */
//
//  return USBD_OK;
//}

static uint8_t  usbd_msc_audio_DeInit (void  *pdev, 
                                   uint8_t cfgidx)
{ 

  DCD_EP_Close (pdev , AUDIO_IN_EP);
  DCD_EP_Close (pdev , MSC_IN_EP);
  DCD_EP_Close (pdev , MSC_OUT_EP);
  /* Un Init the BOT layer */
   MSC_BOT_DeInit(pdev);   
  
  //}
  /* DeInitialize the Audio output Hardware layer */

  return USBD_OK;
}
/**
  * @brief  usbd_audio_Setup
  *         Handles the Audio control request parsing.
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
    static uint8_t  usbd_audio_Setup (void  *pdev,  USB_SETUP_REQ *req)
    {
      uint16_t len;
      uint8_t  *pbuf;
      
      switch (req->bmRequest & USB_REQ_TYPE_MASK)
      {
        /* AUDIO Class Requests -------------------------------*/
      case USB_REQ_TYPE_CLASS :    
        switch (req->bRequest)
        {
        case AUDIO_REQ_GET_CUR: //запрос состояния mute
          AUDIO_Req_GetCurrent(pdev, req);
          break;
          
        case AUDIO_REQ_SET_CUR:
          AUDIO_Req_SetCurrent(pdev, req);   
          break;
          
        case BOT_GET_MAX_LUN :
          
          if((req->wValue  == 0) && 
             (req->wLength == 1) &&
               ((req->bmRequest & 0x80) == 0x80))
          {
            USBD_MSC_MaxLun = USBD_STORAGE_fops->GetMaxLun();
            if(USBD_MSC_MaxLun > 0)
            {
              USBD_CtlSendData (pdev,
                                &USBD_MSC_MaxLun,
                                1);
            }
            else
            {
              USBD_CtlError(pdev , req);
              return USBD_FAIL; 
              
            }
          }
          else
          {
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
          }
          break;
          
        case BOT_RESET :
          if((req->wValue  == 0) && 
             (req->wLength == 0) &&
               ((req->bmRequest & 0x80) != 0x80))
          {      
            MSC_BOT_Reset(pdev);
          }
          else
          {
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
          }
          break;   
          
        default:
          USBD_CtlError (pdev, req);
          return USBD_FAIL;
        }
        break;
        
        /* Standard Requests -------------------------------*/
      case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR: 
          if( (req->wValue >> 8) == AUDIO_DESCRIPTOR_TYPE)
          {
            pbuf = usbd_audio_CfgDesc + 18;
            len = MIN(USB_AUDIO_DESC_SIZ , req->wLength);
          }
          
          USBD_CtlSendData (pdev, pbuf, len);
          break;
          
        case USB_REQ_GET_INTERFACE :
          if(req->wIndex ==0){
            USBD_CtlSendData (pdev,
                              &USBD_MSC_AltSet,
                              1);
          }
          else
            USBD_CtlSendData (pdev, (uint8_t *)&usbd_audio_AltSet, 1);
          break;
          
        case USB_REQ_SET_INTERFACE :
          
          if ((uint8_t)(req->wValue) < AUDIO_TOTAL_IF_NUM && (uint8_t)(req->wIndex)==1) //Alt Setting for audio
          {
            usbd_audio_AltSet = (uint8_t)(req->wValue);
            if (usbd_audio_AltSet == 1) //выбор используемого интерфейса
            {
              PlayFlag = 1;
              
            } 
            else 
            { 
              PlayFlag = 0;
              DCD_EP_Flush (pdev,AUDIO_IN_EP);
            }
          }
          else if ((uint8_t)(req->wIndex)!=1) //Alt Setting for CDC
          {
            USBD_MSC_AltSet = (uint8_t)(req->wValue);
          }
          else
          {
            /* Call the error management function (command will be nacked */
            USBD_CtlError (pdev, req);
          }
          break;
        case USB_REQ_CLEAR_FEATURE:  
          
          /* Flush the FIFO and Clear the stall status */    
          DCD_EP_Flush(pdev, (uint8_t)req->wIndex);
          
          /* Re-activate the EP */      
          DCD_EP_Close (pdev , (uint8_t)req->wIndex);
          if((((uint8_t)req->wIndex) & 0x80) == 0x80)
          {
            DCD_EP_Open(pdev,
                        ((uint8_t)req->wIndex),
                        MSC_EPIN_SIZE,
                        USB_OTG_EP_BULK);
          }
          else
          {
            DCD_EP_Open(pdev,
                        ((uint8_t)req->wIndex),
                        MSC_EPOUT_SIZE,
                        USB_OTG_EP_BULK);
          }
          
          /* Handle BOT error */
          MSC_BOT_CplClrFeature(pdev, (uint8_t)req->wIndex);
          break;
          
        }
      }
      // }
      return USBD_OK;
    }

static uint8_t  usbd_msc_audio_Setup (void  *pdev, 
                                  USB_SETUP_REQ *req)
{
  	if(((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_INTERFACE && req->wIndex == MSC_INTERFACE_IDX) ||
		((req->bmRequest & USB_REQ_RECIPIENT_MASK) == USB_REQ_RECIPIENT_ENDPOINT && ((req->wIndex & 0x7F) == MSC_EP_IDX)))
	{
		return USBD_MSC_Setup(pdev, req);
	}

	return usbd_audio_Setup(pdev, req);
  }


/**
  * @brief  usbd_audio_EP0_RxReady
  *         Handles audio control requests data.
  * @param  pdev: device device instance
  * @retval status
  */
static uint8_t  usbd_audio_EP0_RxReady (void  *pdev)
{ 

  return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Handles the audio IN data stage.
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
//handle request from HOST
static uint8_t  usbd_audio_DataIn (void *pdev, uint8_t epnum)
{
  DCD_EP_Flush(pdev,AUDIO_IN_EP);//very important!!!
  if(PlayStatus == PLAY_STATUS_RUN){
    if(TX_buffer_state == MIC_TX_BUFFER_FULL){
      DCD_EP_Tx (pdev, AUDIO_IN_EP, (uint8_t*)MicOutBuffer0, AUDIO_IN_PACKET);//length in words to bytes
      TX_buffer_state = MIC_TX_BUFFER_EMPTY;
    } 
  }
  
  return USBD_OK;
}

static uint8_t  usbd_msc_audio_DataIn (void *pdev, uint8_t epnum)
{
 	if(epnum == MSC_EP_IDX)
		return USBD_MSC_DataIn(pdev, epnum);

	return usbd_audio_DataIn(pdev, epnum);
}



/**
  * @brief  usbd_audio_DataOut
  *         Handles the Audio Out data stage.
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_audio_DataOut (void *pdev, uint8_t epnum)
{     

  return USBD_OK;
}

static uint8_t  usbd_msc_audio_DataOut (void *pdev, uint8_t epnum)
{     
 if(epnum == MSC_EP_IDX)
		return USBD_MSC_DataOut(pdev, epnum);
	return usbd_audio_DataOut(pdev, epnum);
}


/**
  * @brief  usbd_audio_SOF
  *         Handles the SOF event (data buffer update and synchronization).//start-of-frame
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  usbd_audio_SOF (void *pdev)
{     
  // Check if there are available data in stream buffer.
  if (PlayFlag == 1) {
    
    DCD_EP_Tx (pdev,AUDIO_IN_EP, NULL, AUDIO_IN_PACKET);
    PlayFlag = 2;
  }
  return USBD_OK;
}


/******************************************************************************
     AUDIO Class requests management
******************************************************************************/
/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_GetCurrent(void *pdev, USB_SETUP_REQ *req)
{  
  /* Send the current mute state */
  USBD_CtlSendData (pdev, 
                    AudioCtl,
                    req->wLength);
}

/**
  * @brief  AUDIO_Req_SetCurrent
  *         Handles the SET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void AUDIO_Req_SetCurrent(void *pdev, USB_SETUP_REQ *req)
{ 
  if (req->wLength)
  {
    /* Prepare the reception of the buffer over EP0 */
    USBD_CtlPrepareRx (pdev, 
                       AudioCtl,
                       req->wLength);
    
    /* Set the global variables indicating current request and its length 
    to the function usbd_audio_EP0_RxReady() which will process the request */
    AudioCtlCmd = AUDIO_REQ_SET_CUR;     /* Set the request value */
    AudioCtlLen = req->wLength;          /* Set the request data length */
    AudioCtlUnit = HIBYTE(req->wIndex);  /* Set the request target unit */
  }
}

/**
  * @brief  USBD_audio_GetCfgDesc 
  *         Returns configuration descriptor.
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_audio_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (usbd_audio_CfgDesc);
  return usbd_audio_CfgDesc;
}
/**
  * @}
  */ 

static uint8_t  usbd_audio_OUT_Incplt (void  *pdev)
{
  return USBD_OK;
}

static uint8_t  usbd_audio_IN_Incplt (void  *pdev)
{
  return USBD_OK;
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
