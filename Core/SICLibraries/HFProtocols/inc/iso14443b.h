//! \file:  iso14443b.h
//! \brief: This file provides basic code for using iso14443b protocol

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION: -
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
//
// NOTE: -
//
//----------------------------------------------------------------------------//
 
#ifndef ISO14443B_H
#define ISO14443B_H

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
hfalStatus_t iso14443b_WUPB(uint8_t AFI, uint8_t Num_Slots_N,
                        uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443b_REQB(uint8_t AFI, uint8_t Num_Slots_N,
                        uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443b_ATTRIB(uint8_t *PUPI, uint8_t *param, uint8_t *hLayer,
                         uint16_t hLayerLen, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443b_HLTB(uint8_t *PUPI, uint8_t *dOut, uint16_t *dOutLen);

#endif // ISO14443B_H
