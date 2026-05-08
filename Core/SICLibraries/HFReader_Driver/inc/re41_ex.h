//! \file:  re41_ex.h
//! \brief: This file provides code extension for using RE41 reader chip to
//          support FELICA tag type.

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
// UPDATE: 2020-11-25
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RE41-R1.3-20190405
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef RE41_EX_H
#define RE41_EX_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
#define FELICA_ATQC_LEN          0x13

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
void RE41Ext_init(uint8_t irqEn, uint8_t rstpdEn);
void RE41Ext_receiveMultiple(uint8_t rxMultiEn);
hfalStatus_t RE41Ext_transmitCommand(uint8_t *dataTx, uint16_t dataTxLen, uint16_t timeOut);
hfalStatus_t RE41Ext_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut);
hfalStatus_t RE41Ext_receiveMultiSlots(uint8_t numSlot, uint8_t *dataRx,
                                       uint16_t *dataRxLen, uint16_t timeOut);
hfalStatus_t RE41Ext_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                       uint8_t *dataRx, uint16_t *dataRxLen,
                                       uint16_t timeOut);
hfalStatus_t RE41Ext_transceiveMultiSlots(uint8_t numSlot,
                                          uint8_t *dataTx, uint16_t  dataTxLen,
                                          uint8_t *dataRx, uint16_t *dataRxLen,
                                          uint16_t timeOut);

#endif  // #ifndef RE41_EX_H
