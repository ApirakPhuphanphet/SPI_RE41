//! \file:  iso14443a_MFUltralight.h
//! \brief: This file provide basic code for using SIC reader with ISO/IEC 14443 
//          Type A protocol to communicate with NFC Tag 2 Type (MIFARE ULTRALIGHT)

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
// UPDATE: 2020-09-18
//
// REFERENCE DOCUMENT:
// 1) ISO/IEC FCD 14443-3
// 2) ISO/IEC FCD 14443-4
//
// NOTE: -
//
//----------------------------------------------------------------------------//
 
#ifndef ISO14443A_MFULTRA_H
#define ISO14443A_MFULTRA_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
hfalStatus_t mfUltra_readMemory(uint8_t page, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t mfUltra_writeMemory(uint8_t page, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t mfUltra_compat_writeMemory(uint8_t page, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen);

#endif // ISO14443A_MFULTRA_H
