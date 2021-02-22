/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v1.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */

       		  
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  200
#define STORAGE_BLK_SIZ                  512

/* USER CODE BEGIN PRIVATE_DEFINES */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_11   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_11  +  GetSectorSize(ADDR_FLASH_SECTOR_11) -1 /* End @ of user Flash area : sector start address + sector size -1 */
uint8_t usbMassStorage[STORAGE_BLK_NBR][STORAGE_BLK_SIZ];
#define FLASH_DISK_START_ADDRESS 0x08020000
#define FLASH_PAGE_SIZE	2048 	
#define WAIT_TIMEOUT    5000

uint8_t Pseudo_FS[STORAGE_BLK_NBR*STORAGE_BLK_SIZ];
extern SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypeDef CardInfo;
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */

  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */
uint32_t FirstSector = 0, FirstSector_temp, NbOfSectors = 0, Address = 0;
uint32_t SectorError = 0;

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */
    
  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */
		//*block_num = STORAGE_BLK_NBR;
		//*block_size = STORAGE_BLK_SIZ;
    // BSP_SD_GetCardInfo(&CardInfo);
  //  HAL_SD_GetCardInfo(&hsd,&CardInfo);
	
    *block_num  = STORAGE_BLK_NBR;
    *block_size = STORAGE_BLK_SIZ;

 // *block_size = STORAGE_BLK_SIZ;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */
	
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 6 */

  
//  switch(lun)
//  {
//  case 0:
//    memcpy(buf, (void *)(FLASH_USER_START_ADDR + (STORAGE_BLK_SIZ * blk_addr)), (STORAGE_BLK_SIZ * blk_len));
//    //            for(i =0; i < (blk_len*STORAGE_BLK_SIZ); i+=1)
//    //            {
//    //                buf[i] = *((uint32_t*)(FLASH_USER_START_ADDR + (blk_addr*STORAGE_BLK_SIZ)+ i));
//    //            }
//    //            break;
//  case 1:
//    break;
//  default:
//    return (USBD_FAIL);
//  }
    
  
		 memcpy(buf, &Pseudo_FS[blk_addr*STORAGE_BLK_SIZ], blk_len*STORAGE_BLK_SIZ);
   

	//HAL_SD_ReadBlocks(&hsd, buf, blk_addr, (uint32_t) blk_len, 10);
//uint16_t i;

//    
//    switch (lun)
//    {
//        case 0:
//            for(i = 0; i < blk_len; i += 4) 
//            {
//                buf[i >> 2] = ((uint16_t*)(FLASH_DISK_START_ADDRESS + blk_addr))[i >> 2]; 
//            }
//            break;
//        case 1:
//            break;
//        default:
//            return (USBD_FAIL);
//    }

return (USBD_OK);

  /* USER CODE END 6 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 7 */
//  	uint8_t i = 0;
//	uint16_t j = 0;
//  uint32_t startsector = 0, sectorerror = 0;
//			
//  HAL_StatusTypeDef status;
//  HAL_FLASH_Unlock(); 
//  FLASH_EraseInitTypeDef eraseinitstruct;
//		for(int i = 0; i < (blk_len*STORAGE_BLK_SIZ); i+=1)
//			{
//			//	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, FLASH_USER_START_ADDR + (blk_addr*STORAGE_BLK_SIZ) + i, (uint32_t)buf[i]);
//				//*(uint8_t* )(PseudoFS + i + blk_addr*STORAGE_BLK_SIZ) = *(buf + i);  
//			}
   memcpy(&Pseudo_FS[blk_addr*STORAGE_BLK_SIZ], buf, blk_len*STORAGE_BLK_SIZ);
//	//HAL_SD_WriteBlocks(&hsd, buf, blk_addr, (uint32_t)blk_len,10);

    //uint8_t copy_buf[STORAGE_CAPACITY];
  
//			
//			for (i = 0; i < STORAGE_BLK_NBR; i++){//128
//				for(j = 0; j < STORAGE_BLK_SIZ; j++){//512
//						usbMassStorage[i][j] = *((volatile uint8_t*)(FLASH_USER_START_ADDR + (i*STORAGE_BLK_SIZ) + j));
////					
//				}
//			}
//			/* Get the number of sector */
//			startsector = GetSector(FLASH_USER_START_ADDR);
//			eraseinitstruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//			eraseinitstruct.Sector = startsector;
//			eraseinitstruct.NbSectors = 1;
//			eraseinitstruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
//			status = HAL_FLASHEx_Erase(&eraseinitstruct, &sectorerror);			
//			
//			//modify
//			for (i = 0; i < blk_len; i++){
//				for(j = 0; j < STORAGE_BLK_SIZ; j++){//512
//					usbMassStorage[(i + blk_addr)][j] = buf[j + (i*STORAGE_BLK_SIZ)];
////			
//				}
//			}			
//			// write
//			for (i = 0; i < STORAGE_BLK_NBR; i++){
//				for(j = 0; j < STORAGE_BLK_SIZ; j++){
//					FLASH_WaitForLastOperation(WAIT_TIMEOUT);     
//					HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (FLASH_USER_START_ADDR + (i*STORAGE_BLK_SIZ) + j), usbMassStorage[i][j]);
//				}
//			}
//			FLASH_WaitForLastOperation(WAIT_TIMEOUT);
//			HAL_FLASH_Lock();
////    switch(lun)
//    {
//               case 0:
//            for(i = 0; i < Transfer_Length; i += FLASH_PAGE_SIZE)
//            {
//                if (FLASH_WaitForLastOperation(WAIT_TIMEOUT) != FLASH_TIMEOUT)
//                {
//                    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
//                }
              //  FLASH_ErasePage(FLASH_USER_START_ADDR + Memory_Offset + i);
//            } 
//            for(i = 0; i < Transfer_Length; i += 4)
//            {
//                if(FLASH_WaitForLastOperation(WAIT_TIMEOUT) != FLASH_TIMEOUT)
//                {
//                    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
//                }
//                FLASH_ProgramWord(FLASH_USER_START_ADDR + Memory_Offset + i , Writebuff[i >> 2]);
//            }            
//            break;
//        case 1:
//            break;
//
//        case 1:
//            break;
//        default:
//            return (USBD_FAIL);
//    }
	

  return (USBD_OK);

  /* USER CODE END 7 */
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */
  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */


/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
