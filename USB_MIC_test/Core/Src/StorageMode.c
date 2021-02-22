#include "StorageMode.h"
#include  "string.h"
#define STORAGE_LUN_NBR 1
#define STANDARD_INQUIRY_DATA_LEN                   0x24U
#define STORAGE_BLK_NBR                  143
#define STORAGE_BLK_SIZ                  0x200

extern SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypeDef CardInfo;
//uint8_t Pseudo_FS[STORAGE_BLK_NBR*STORAGE_BLK_SIZ];

const int8_t STORAGE_Inquirydata[] = {/* 36 */

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

int8_t STORAGE_Init(uint8_t lun)
{
	return 0;
}

int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    HAL_SD_GetCardInfo(&hsd,&CardInfo);
  *block_num  = CardInfo.BlockNbr;
   *block_size = CardInfo.BlockSize;
  //*block_size = STORAGE_BLK_SIZ;
  //*block_num = STORAGE_BLK_NBR;
  return 0;
}

int8_t STORAGE_IsReady(uint8_t lun)
{
  return 0;
}

int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
	return 0;
}

int8_t STORAGE_Read(
	uint8_t lun,        // logical unit number
	uint8_t *buf,       // Pointer to the buffer to save data
	uint32_t blk_addr,  // address of 1st block to be read
	uint16_t blk_len)   // nmber of blocks to be read
{
 /// memcpy(buf, (Pseudo_FS + blk_addr*STORAGE_BLK_SIZ), blk_len*STORAGE_BLK_SIZ);
   HAL_SD_ReadBlocks(&hsd, buf, blk_addr, (uint32_t) blk_len, 10);
	
	return 0;
}

int8_t STORAGE_Write(uint8_t lun,
	uint8_t *buf,
	uint32_t blk_addr,
	uint16_t blk_len)
{
  //memcpy((Pseudo_FS + blk_addr*STORAGE_BLK_SIZ), buf, blk_len*STORAGE_BLK_SIZ);
   HAL_SD_WriteBlocks(&hsd, buf, blk_addr, (uint32_t)blk_len,10);
	return 0;
}

int8_t STORAGE_GetMaxLun(void)
{
  return (STORAGE_LUN_NBR - 1);
}

USBD_STORAGE_cb_TypeDef STORAGE_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef *USBD_STORAGE_fops = &STORAGE_fops;

void storage_mode_init(void)
{
}
