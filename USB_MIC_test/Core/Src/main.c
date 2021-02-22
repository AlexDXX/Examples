/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "tim.h"
#include "gpio.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_audio_core.h"
#include  "usbd_core.h"
#include "usbd_msc_core.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUTTON_PLAY_STOP_PRESSED GPIOA->IDR & GPIO_IDR_ID0
#define BUTTON_SELECT_USB_PRESSED GPIOA->IDR & GPIO_IDR_ID3
#define BUTTON_HOLD_DELAY 5000000

#define LED_PLAYER_RUN_ON  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET);
#define LED_PLAYER_RUN_OFF  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET);

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern FATFS SDFatFS;
extern char  SDPath[4];
FRESULT res;
///////////////////
extern int PlayFlag;
extern uint8_t TX_buffer_state;
extern int16_t MicOutBuffer0[AUDIO_IN_PACKET]; //TX Buffer 

///////////////////
USB_OTG_CORE_HANDLE  USB_OTG_dev ;
uint8_t PlayStatus = PLAY_STATUS_STOP;
uint8_t wavBuf[WAV_BUF_SIZE]; 
uint32_t wavDataSize = 0;
int WAVFileStatus = -1;
/////////////////////
uint8_t button1_state =0, button2_state = 0;
uint32_t delay_cnt =0;
////////////////////
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
int WAV_file_processing_from_SD(uint32_t * offset, uint32_t* PCM_frame_count);
void update_Buffer(uint32_t* Current_frame, uint32_t Total_frame_count, uint32_t offset);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */
  
  /* MCU Configuration--------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */
  
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SDIO_SD_Init();
  MX_TIM6_Init();
  MX_FATFS_Init();
  USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &AUDIO_cb, &USR_cb);
  
  /* USER CODE BEGIN 2 */
  uint32_t dataOffset = 0;
  uint32_t TotalFrameCountInWavBuffer = 0;
  uint32_t Current_frame =0;
  
  /*----------------Mount Fatfs filesystem for read SdCard---------------------*/
  res = f_mount(&SDFatFS, SDPath, 1);
  /*----------------------Read audio file and prepare data---------------------*/
  if(res == FR_OK){
    WAVFileStatus = WAV_file_processing_from_SD(&dataOffset, &TotalFrameCountInWavBuffer);
  }
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /*---------------------------Update Mic TX buffer-------------------------*/
    if(PlayStatus == PLAY_STATUS_RUN){
      if(TX_buffer_state == MIC_TX_BUFFER_EMPTY){
        update_Buffer(&Current_frame, TotalFrameCountInWavBuffer, dataOffset);
      }
    }
    /*----------------------------Start/Stop PLay---------------------------*/
    if(BUTTON_PLAY_STOP_PRESSED){
      button1_state = 1;
      delay_cnt++;
      if(delay_cnt > BUTTON_HOLD_DELAY){                                        //delay for button hold(for reduction contact bounce)
        if(button1_state == 1){
          button1_state = 0;
          delay_cnt = 0;
          if((PlayStatus == PLAY_STATUS_STOP) && (WAVFileStatus == FILE_STATUS_OK)){    //Check if play stopped and read file data is correct
            PlayStatus = PLAY_STATUS_RUN;                                       //Start wav play
            Current_frame = 0;
            PlayFlag = 1;
            LED_PLAYER_RUN_ON
          }
          else{
            PlayStatus = PLAY_STATUS_STOP;                                      //Stop wav play
            LED_PLAYER_RUN_OFF
          }
        }
      }
    }
    else{
      delay_cnt = 0;
      button1_state = 0;
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int WAV_file_processing_from_SD(uint32_t * offset, uint32_t* PCM_frame_count){
  uint8_t  PCM_data_found =0;
  char *data_start = "data"; 
  uint32_t readBytes = 0;
  /*-----------------------Read file from SD card---------------------*/
 uint8_t path[11] = "sound1.wav";
 FIL audioFile;
   res = f_open(&audioFile, (char*)path, FA_READ);
   res = f_read(&audioFile, (void *)wavBuf, WAV_BUF_SIZE, &readBytes);
   res = f_close(&audioFile);
   if(res == FR_OK){
   /*-----------------------Find Start PCM data---------------------*/   
    for (uint16_t i = 0; i < (WAV_BUF_SIZE - 3); i++){   
      if (memcmp((void*)&(wavBuf[i]),(void*)data_start, 4) == 0){
        *offset = i + 8;
        PCM_data_found = 1;
        break;
      }
    }
    if(PCM_data_found){
      *PCM_frame_count  = (WAV_BUF_SIZE - *offset)/ AUDIO_IN_PACKET;         // Calculate number of TX frames(32kB) in read samples
      return 0;
    }
    else
      return -2;
   }
   else
     return -1;
  }

 /** Update TX buffer*/
void update_Buffer(uint32_t* Current_frame, uint32_t Total_frame_count, uint32_t offset){
      if((*Current_frame) < Total_frame_count ){
        if(TX_buffer_state == MIC_TX_BUFFER_EMPTY){
          memcpy(MicOutBuffer0, (void*)(wavBuf + offset + (*Current_frame )* AUDIO_IN_PACKET), AUDIO_IN_PACKET); //Fill mic TX buffer
          TX_buffer_state = MIC_TX_BUFFER_FULL;
        }
        (*Current_frame)++;
      }
      else{
        *Current_frame = 0;
      }
}
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
