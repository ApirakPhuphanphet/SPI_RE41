//! \file:  utilities.h
//! \brief: This file provides all basic utility functions.

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
// UPDATE: 2020-09-23
//
// REFERENCE DOCUMENT:
// 1) -
//
// NOTE: -
//
//----------------------------------------------------------------------------//
 
#ifndef UTILITIES_H
#define UTILITIES_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include <stdint.h>

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
void dataToHex(uint8_t *dataOut, uint8_t dataIn);
void customToLower(uint8_t *dataStr, uint8_t dataStrLen);
uint8_t customParseHEX(uint8_t *s, uint8_t sLen, uint8_t *valHEX);
void bytesToChars(uint8_t *byteUID, uint16_t inputLen,
                  uint8_t *charUID, uint16_t  *outputlen);
uint8_t isDigit(uint8_t c);
uint8_t isHexAlpha(uint8_t c);
uint8_t isHexDigit(uint8_t c);
uint8_t hexDigitToNum(uint8_t c);
void reverseByte(uint8_t *arr, uint16_t n);
uint8_t parseHexStr(uint8_t *dIn, uint16_t dInLen,
                                uint8_t *dOut, uint16_t *dOutLen, uint8_t opt);
void reverse8BitData(uint8_t dIn, uint8_t *dOut);
void reverse16BitData(uint16_t dIn, uint16_t *dOut);

#endif /* UTILITIES_H */
