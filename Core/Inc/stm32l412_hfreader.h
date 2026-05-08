//! \file:  stm32l412_hfreader.h
//! \brief: low level implementation header file

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION: -
//
// AUTHOR:  1) WORRAKAMOL BOONPARIPHAN,
//          2) APIRAK RATSAMEESAWANG
// CONTACT: 1) worrakamol@sic.co.th
//          2) apirak@sic.co.th
//
// UPDATE: 2020-09-29
//
// REFERENCE DOCUMENT:
// 1) -
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef STM32L412_HFREADER_H
#define STM32L412_HFREADER_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "main.h"
#include "spi.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
#define HFREADER_NSS_SET()      HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_SET)
#define HFREADER_NSS_CLR()      HAL_GPIO_WritePin(RE41_NCS_GPIO_Port, RE41_NCS_Pin, GPIO_PIN_RESET)
#define HFREADER_ReadIRQ()      HAL_GPIO_ReadPin(RE41_IRQ_GPIO_Port, RE41_IRQ_Pin);

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
// N/A

#endif  // #ifndef STM32L412_HFREADER_H
