//! \file:  iso15693.h
//! \brief: This file provide basic code for using SIC reader with ISO/IEC 15693

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
// 1) ISO/IEC FCD 15693-3
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef ISO15693_H_
#define ISO15693_H_

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// N/A

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// Inventory Mode
#define INVENTORY_1_SLOT_MODE       0x01
#define INVENTORY_16_SLOTS_MODE     0x02
#define UID_OPTIONAL                0x03
#define UID_NOT_OPTIONAL            0x04
#define NON_CHECK_AFI               0x00

// REQUEST-FLAGs
#define SUBCARRIER_FLAG             0
#define DATARATE_FLAG               1
#define INVENTORY_FLAG              2
#define PROTOCOLEXT_FLAG            3
#define SELECT_FLAG                 4
#define ADDRESS_FLAG                5
#define AFI_FLAG                    4
#define NB_SLOTS_FLAG               5
#define CUSTOM_FLAG                 6
#define RFU                         7

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// ISO15693 Error code definition
#define ERROR_FLAG                  0x01
#define CMD_NOT_SUPPORT             0x01
#define CMD_NOT_RECOGNIZE           0x02
#define UNKNOW_ERR                  0x0F
#define BLOCK_UNAVAILABLE           0x10
#define ALREADY_LOCK                0x11
#define BLOCK_LOCK                  0x12
#define PROGRAM_NOT_SUCCESS         0x13
#define LOCK_NOT_SUCCESS            0x14

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
hfalStatus_t iso15693_inv1SlotCmd(uint8_t AFI, uint8_t maskLen,
                           uint8_t *maskVal, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso15693_inv16SlotCmd(uint8_t AFI,
                                   uint8_t maskLen,   uint8_t  *maskVal,
                                   uint8_t *collFlag, uint8_t  *collPos,
                                   uint8_t *uid,      uint8_t  *totalTag,
                                   uint8_t *dOut,     uint16_t *dOutLen);
hfalStatus_t iso15693_stayQuiet(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso15693_select(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso15693_read1Block(uint8_t uidOpt, uint8_t *uid, uint8_t blockNum,
                                 uint8_t *dOut, uint16_t *dOutLen);
hfalStatus_t iso15693_write1Block(uint8_t uidOpt,
                                  uint8_t *uid,       uint8_t blockNum,
                                  uint8_t *blockData, uint8_t blockLen,
                                  uint8_t *dOut,      uint16_t *dOutLen);

#endif // ISO15693_H_
