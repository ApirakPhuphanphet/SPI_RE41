//! \file:  iso14443a.h
//! \brief: This file provide basic code for using SIC reader with ISO/IEC 14443 Type A
//
//
//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION:
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
//
// NOTE:-
//
//----------------------------------------------------------------------------//

#ifndef ISO14443A_H
#define ISO14443A_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
typedef enum
{
    SEND_WUPA_CMD = 0x00,
    SEND_REQA_CMD
} iso14443a_startCommand_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
hfalStatus_t iso14443a_REQA(uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_WUPA(uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_HLTA(uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_AC(uint8_t level, uint8_t collMaskVal, uint8_t *numColl,
                          uint8_t *collPos, uint8_t *uid, uint16_t *uidLen);
hfalStatus_t iso14443a_SELECT(uint8_t level, uint8_t *UID,
                              uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_Req_Anti_Sel(iso14443a_startCommand_t cmd,
                                    uint8_t collMaskVal, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_RATS(uint8_t param, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_PPS(uint8_t CID, uint8_t PPS0, uint8_t PPS1,
                           uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_ReadBlock(uint8_t blockNo, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso14443a_WritePage(uint8_t pageNo, uint8_t *data, uint16_t dataLen);

#endif // ISO14443A_H
