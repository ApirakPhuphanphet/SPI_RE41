/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.h
 * @brief   This file contains all the function prototypes for
 *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  extern UART_HandleTypeDef huart1;

  extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
#define HEADER_INDEX 0
#define LENGTH_INDEX 1
#define CMD_INDEX 2
#define ADDRESS_INDEX 3

  typedef enum
  {
    CHECKSUM_OK = 0xFF,
    HEADER_ERROR = 0xFE,
    CMD_ERROR = 0xFD,
    CHECKSUM_ERROR = 0xFC,
    LENGTH_ERROR = 0xFB,
    NULL_PTR_ERROR = 0xFA
  } Data_StatusTypeDef;

  typedef enum
  {
    SPI_WRITE = 0x00,
    SPI_READ_SINGLE = 0x01,
    SPI_RESET = 0x02,
    SPI_READ_MULTIPLE = 0x03,
    SPI_WRITE_MULTIPLE = 0x04,
    TAG_TYPE_A_WRITE = 0x05,
    TAG_TYPE_A_READ = 0x06,
    TAG_TYPE_A_DUMP = 0x07,
    TAG_TYPE_A_READ_UID = 0x08
  } CMD_TypeDef;
  /* USER CODE END Private defines */

  void MX_USART1_UART_Init(void);
  void MX_USART2_UART_Init(void);

  /* USER CODE BEGIN Prototypes */

  void Prepare_Next_Packet(void);
  void Response(uint8_t *data, uint8_t data_len);
  Data_StatusTypeDef Data_Verify(uint8_t *data, uint16_t total_length);
  /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
