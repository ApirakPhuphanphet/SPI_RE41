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
#include <string.h>
#include <stdio.h>
// Take SPI From spi file.
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern SPI_HandleTypeDef hspi1;

uint8_t tx_data[2] = {0xA6, 0x00};
uint8_t rx_data[2] = {0};
char uart_buf[50];

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
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    HAL_Delay(3000);
    // read
    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "0) Register 0x13 Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // reset
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_RESET);
    sprintf(uart_buf, "0.1) Reset register!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    // delay for reset complete
    HAL_Delay(3);

    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "1) Register 0x13 Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);
    // write
    tx_data[0] = 0x26; // write 0x13
    tx_data[1] = 0x36;
    sprintf(uart_buf, "2) Write Register 0x13 Value: 0x%02X\r\n", tx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    HAL_Delay(1000);

    // read after write
    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "3) Register 0x13 after write Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // write over flow
    tx_data[0] = 0x26; // write 0x13
    tx_data[1] = 0x41; // over flow
    sprintf(uart_buf, "4) Write Register 0x13 Value: 0x%02X overflow\r\n", tx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    HAL_Delay(1000);

    // read after write
    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "5) Register 0x13 after write overflow Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // reset
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_RESET);
    sprintf(uart_buf, "6) Reset register!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // read after reset
    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "7) Register 0x13 after Reset Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // reset again
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_RESET);
    sprintf(uart_buf, "8) Reset register again!\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);

    // read after reset
    tx_data[0] = 0xA6; // read 0x13
    tx_data[1] = 0x00;
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
    HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);
    sprintf(uart_buf, "9) Register 0x13 after Reset again Value: 0x%02X\r\n", rx_data[1]);
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    HAL_Delay(1000);
    sprintf(uart_buf, "-----------------------------------------------\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t *)uart_buf, strlen(uart_buf), 100);
    //	  HAL_Delay(2500);
    while (1)
      ;
    /* USER CODE BEGIN 3 */
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
