//! \file:  iso14443a_MFClassic.h
//! \brief: This file provide basic code for using SIC reader with ISO/IEC 14443
//          Type A protocol to communicate with NFC Tag MIFARE

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION:
//
// AUTHOR:  1) WORRAKAMOL BOONPARIPHAN
//          2) APIRAK RATSAMEESAWANG
// CONTACT: 1) worrakamol@sic.co.th
//          2) apirak@sic.co.th
//
// UPDATE: 2020-09-18
//
// REFERENCE DOCUMENT:
// 1) ISO/IEC FCD 14443-3
// 2) ISO/IEC FCD 14443-4
//
// NOTE:-
//
//----------------------------------------------------------------------------//
 
#ifndef ISO14443A_MFCLASSIC_H
#define ISO14443A_MFCLASSIC_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
typedef enum
{
    USE_KEY_A = 0x00,
    USE_KEY_B
} mfClassic_selectKey_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
#define MIFARE_AUTHENT_KEY_A        0x60
#define MIFARE_AUTHENT_KEY_B        0x61

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
hfalStatus_t mfClassic_loadKey(uint8_t *key);
hfalStatus_t mfClassic_authentication(mfClassic_selectKey_t selectKey,
                                               uint8_t blockNum, uint8_t *uid);
hfalStatus_t mfClassic_writeBlock(uint8_t blockNum, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t mfClassic_readBlock(uint8_t blockNum,
                                             uint8_t *dOut, uint16_t *dOutLen);

#endif // ISO14443A_MFCLASSIC_H
