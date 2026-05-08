/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    spi.c
 * @brief   This file provides code for the configuration
 *          of the SPI instances.
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
#include "spi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (spiHandle->Instance == SPI1)
  {
    /* USER CODE BEGIN SPI1_MspInit 0 */

    /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA1     ------> SPI1_SCK
    PB4 (NJTRST)     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USER CODE BEGIN SPI1_MspInit 1 */

    /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *spiHandle)
{

  if (spiHandle->Instance == SPI1)
  {
    /* USER CODE BEGIN SPI1_MspDeInit 0 */

    /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA1     ------> SPI1_SCK
    PB4 (NJTRST)     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4 | GPIO_PIN_5);

    /* USER CODE BEGIN SPI1_MspDeInit 1 */

    /* USER CODE END SPI1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
HAL_StatusTypeDef Read_Register(uint8_t reg_addr, uint8_t *data)
{
  uint8_t tx_data[2];
  uint8_t rx_data[2];

  tx_data[0] = (reg_addr << 1) | 0x80;
  tx_data[1] = 0x00;

  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 2, 100);
  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);

  if (status == HAL_OK)
  {
    *data = rx_data[1];
  }
  else
  {
    *data = 0xFF; // Indicate error
  }
  return status;
}

HAL_StatusTypeDef Read_Multiple_Register(uint8_t *reg_addr, uint8_t *data, uint8_t len)
{
  uint8_t tx_data[len + 1];
  uint8_t rx_data[len + 1];

  for (uint8_t i = 0; i < len; i++)
  {
    if (i == 0)
    {
      /* byte 0 MSB = 1 */
      tx_data[i] = (reg_addr[i] << 1) | 0x80;
    }
    else
    {
      /* other bytes MSB = 0 */
      tx_data[i] = (reg_addr[i] << 1) & 0x7F;
    }
    rx_data[i] = 0x00;
  }

  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
  HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, len + 1, 100);
  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);

  if (status == HAL_OK)
  {
    for (uint8_t i = 0; i < len; i++)
    {
      data[i] = rx_data[i + 1];
    }
  }
  else
  {
    for (uint8_t i = 0; i < len; i++)
    {
      data[i] = 0xFF; // Indicate error
    }
  }
  return status;
}

HAL_StatusTypeDef Write_Register(uint8_t reg_addr, uint8_t data)
{
  uint8_t tx_data[2];

  tx_data[0] = (reg_addr << 1);
  tx_data[1] = data;

  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
  HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, tx_data, 2, 100);
  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);

  return status;
}

void Reset_RE41(void)
{
  // reset
  HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RE41_PDRST_GPIO_Port, RE41_PDRST_Pin, GPIO_PIN_RESET);
  HAL_Delay(3);
}

HAL_StatusTypeDef Write_Multiple_Register(uint8_t reg_addr, uint8_t *data, uint8_t len)
{
  uint8_t tx_data[len + 1];
  uint8_t rx_data[len + 1];

  tx_data[0] = (reg_addr << 1); // First byte MSB = 0 for write
  //|0address0|value0|value1|...|valueN
  for (uint8_t i = 0; i < len; i++)
  {
    /* other bytes MSB = 0 */
    tx_data[i + 1] = data[i];
    rx_data[i] = 0x00;
  }

  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET);
  // HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, len + 1, 100);
  HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, tx_data, len + 1, 100);
  HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET);

  if (status == HAL_OK)
  {
    for (uint8_t i = 0; i < len; i++)
    {
      data[i] = rx_data[i + 1];
    }
  }
  else
  {
    for (uint8_t i = 0; i < len; i++)
    {
      data[i] = 0xFF; // Indicate error
    }
  }
  return status;
}

/* USER CODE END 1 */
