//! \file:  felica.h
//! \brief: This file provide basic code for using SIC reader with FELICA

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION: -
//
// AUTHOR:  1) WORAKAMOL BOONPARIPHAN
//          2) APIRAK RATSAMEESAWANG
// CONTACT: 1) worrakamol@sic.co.th
//          2) apirak@sic.co.th
//
// UPDATE: 2020-11-24
//
// REFERENCE DOCUMENT:
// 1) JIS-X-6319-4 : 2005
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef FELICA_H_
#define FELICA_H_

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// Maximum time slot number
#define REQC_1_SLOT                 0x00
#define REQC_2_SLOT                 0x01
#define REQC_4_SLOT                 0x03
#define REQC_8_SLOT                 0x07
#define REQC_16_SLOT                0x0F

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
hfalStatus_t felica_REQC(uint8_t *systemCode, uint8_t numSlot,
                         uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t felica_readCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                uint8_t *blockList,   uint8_t numBlock,
                                uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t felica_writeCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                 uint8_t *blockList,   uint8_t numBlock,
                                 uint8_t *blockData,   uint8_t blockLen,
                                 uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen);

#endif // FELICA_H_
