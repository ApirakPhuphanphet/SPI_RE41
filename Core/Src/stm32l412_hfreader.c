//! \file:  stm32l412_hfreader.c
//! \brief: This file is low level implementation to control RA12 reader

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION:
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

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "stm32l412_hfreader.h"

//----------- (2) GLOBAL VARIABLES -------------------------------------------//
volatile uint32_t g_timCnt = 0;

//----------- (3) PRIVATE TYPEDEF --------------------------------------------//
// N/A

//----------- (4) PRIVATE DEFINE ---------------------------------------------//
// N/A

//----------- (5) PRIVATE MACRO ----------------------------------------------//
// N/A

//----------- (6) PRIVATE VARIABLES ------------------------------------------//

//----------- (7) PRIVATE FUNCTION PROTOTYPES --------------------------------//

// Link functions for HF-Reader peripheral
void HFREADER_IO_init(uint8_t irqEn, uint8_t rstpdEn);
void HFREADER_IO_read1Reg(uint8_t addr, uint8_t *data);
void HFREADER_IO_write1Reg(uint8_t addr, uint8_t data);
uint8_t HFREADER_IO_readIrq(void);
void HFREADER_IO_timStart(uint32_t timeout);
void HFREADER_IO_timStop(void);
uint8_t HFREADER_IO_timIsExpired(void);
void HFREADER_IO_delay(uint32_t ms);

//----------- (8) PRIVATE FUNCTIONS ------------------------------------------//
// Configure HF-Reader IO interface
void HFREADER_IO_init(uint8_t irqEn, uint8_t rstpdEn)
{
    // de-select
    HFREADER_NSS_SET();

    // IRQ if enable
    if (irqEn)
    {
        ; // do nothing because STM32CubeMx has done it
    }
    // RSTPD if enable
    if (rstpdEn)
    {
        ; // do nothing because STM32CubeMx has done it
    }

    // TIMER initialization
    // do nothing because STM32 SystemTick has done it
}

void HFREADER_IO_read1Reg(uint8_t addr, uint8_t *data)
{
    uint8_t tmpAddr = 0;
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];

    HFREADER_NSS_CLR();

    tmpAddr = (((uint8_t)(addr << 1) & 0x7E) | 0x80);
    txBuffer[0] = tmpAddr;
    txBuffer[1] = 0x00; // dummy data

    // SPI transmit and receive data
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 2, HAL_MAX_DELAY);
    *data = rxBuffer[1];

    HFREADER_NSS_SET();
}

void HFREADER_IO_write1Reg(uint8_t addr, uint8_t data)
{
    uint8_t tmpAddr = 0;
    uint8_t txBuffer[2];
    uint8_t rxBuffer[2];

    HFREADER_NSS_CLR();

    tmpAddr = (uint8_t)(addr << 1) & 0x7E;
    txBuffer[0] = tmpAddr;
    txBuffer[1] = data;

    // SPI transmit and receive data
    HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 2, HAL_MAX_DELAY);

    HFREADER_NSS_SET();
}

// read IRQ Pin
uint8_t HFREADER_IO_readIrq(void)
{
    return HFREADER_ReadIRQ();
}

void HFREADER_IO_timStart(uint32_t timeout)
{
    g_timCnt = timeout;
}

void HFREADER_IO_timStop(void)
{
    g_timCnt = 0;
}

uint8_t HFREADER_IO_timIsExpired(void)
{
    if (g_timCnt > 0)
    {
        return 0; // FALSE
    }
    else
    {
        return 1; // TRUE
    }
}

void HFREADER_IO_delay(uint32_t ms)
{
    g_timCnt = ms;
    while (g_timCnt != 0)
        ;
}

// STM32 using SystemTick - timebase 1 mS
void HAL_SYSTICK_Callback(void)
{
    if (g_timCnt > 0)
    {
        g_timCnt--;
    }
    else
    {
        ; // do nothing
    }
}

//----------- (9) END OF FILE ------------------------------------------------//
