/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// standard library
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// RE41 Lib
#include "hfreader.h"
#include "re41.h"
#include "re41_ex.h"

#include "re41_interface.h"
// Take SPI From spi file.
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RE41_IRQ_CFG 1   // 0: disable, 1: enable
#define RE41_RSTPD_CFG 0 // 0: disable, 1: enable
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern SPI_HandleTypeDef hspi1;
extern uint8_t uart_buf[50];
extern volatile uint8_t uart_size;
extern volatile uint8_t payload_length;
extern volatile bool packet_complete;

uint8_t response_data;
HFREADER_DrvTypeDef *hfReaderDrv;
re41_configProtocol_t g_tagType;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  hfalStatus_t status = HFAL_ERROR;
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
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  // RE41 initial standard
  RE41_init(RE41_IRQ_CFG, RE41_RSTPD_CFG, &hfReaderDrv);
  // RE41 initial extended
  RE41Ext_init(RE41_IRQ_CFG, RE41_RSTPD_CFG);

  // RE41 reset before using
  HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, RESET);
  HAL_Delay(10);

  // analyze interface between MCU and RE41 reader
  status = RE41_analyze();
  if (status != HFAL_SUCCESS)
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nRE41-Interface-Error", 22, HAL_MAX_DELAY);
    while (1)
      ; // break here
  }

  // configure RE41 reader
  status = RE41_configuration(ISO14443A_106KBPS);
  if (status != HFAL_SUCCESS)
  {
    while (1)
    {
      HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nRE41-Unknown-Series", 21, HAL_MAX_DELAY);
    }
  }
  g_tagType = ISO14443A_106KBPS;

  RE41_rfOperate(RFOFF);
  HAL_Delay(25);
  RE41_rfOperate(RFON);
  HAL_Delay(25);

  HAL_UARTEx_ReceiveToIdle_IT(&huart2, uart_buf, 50);
  // Enable debug in sleep mode for testing with ST-Link
  HAL_DBGMCU_EnableDBGSleepMode();
  // Delay to allow debugger to connect before entering main loop
  HAL_Delay(2000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Enter sleep mode, will wake up on UART receive interrupt
    HAL_SuspendTick();
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    HAL_ResumeTick();

    if (packet_complete)
    {
      Data_StatusTypeDef status = Data_Verify(uart_buf, uart_size);

      if (status == CHECKSUM_OK)
      {
        uint8_t cmd = uart_buf[CMD_INDEX];
        uint8_t address = uart_buf[ADDRESS_INDEX];
        switch (cmd)
        {
        case SPI_WRITE:
        {
          response_data = uart_buf[4];
          Write_Register(address, response_data);
          Read_Register(address, &response_data);
          Response(&response_data, 1);
          break;
        }
        case SPI_READ_SINGLE:
        {
          Read_Register(address, &response_data);
          Response(&response_data, 1);
          break;
        }
        case SPI_RESET:
          Reset_RE41();
          response_data = 0x00;        // Indicate success
          Response(&response_data, 1); // Acknowledge reset
          break;
        case SPI_READ_MULTIPLE:
        {
          uint8_t num_bytes = uart_buf[1] - 2; // Total length - CMD and BCC
          uint8_t spi_data_buffer[num_bytes];
          Read_Multiple_Register(address, spi_data_buffer, num_bytes);
          Response(spi_data_buffer, num_bytes);
          break;
        }
        case SPI_WRITE_MULTIPLE:
        {
          uint8_t num_bytes = uart_buf[1] - 3; // Total length - CMD, ADDR, and BCC
          uint8_t spi_data_buffer[num_bytes];
          for (uint8_t i = 0; i < num_bytes; i++)
          {
            spi_data_buffer[i] = uart_buf[4 + i];
          }
          Write_Multiple_Register(address, spi_data_buffer, num_bytes);
          Read_Register(address, &response_data);
          Response(&response_data, 1);
          break;
        }
        case TAG_TYPE_A_WRITE:
          // data is uart_buf[4] to uart_buf[7]
          write_type_A(address, (uint8_t *)uart_buf + 4, 4);
          read_type_A(address);
          break;
        case TAG_TYPE_A_READ:
          read_type_A(address);
          break;
        case TAG_TYPE_A_DUMP:
          dump_mem();
          break;
        case TAG_TYPE_A_RESET:
          // reset_type_A(address);
          break;
        case TAG_TYPE_A_READ_UID:
          // read_uid_type_A(address);
          break;
        default:
          break;
        }
      }
      else
      {
        Response(&status, 1);
      }
      Prepare_Next_Packet();
    }
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */

    /* USER CODE END 3 */
  }
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
