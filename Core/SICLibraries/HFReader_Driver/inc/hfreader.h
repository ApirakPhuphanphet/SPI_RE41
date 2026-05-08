//! \file  hfreader.h
//! \brief HF reader interface object for all SIC HF reader.

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION: -
//
// AUTHOR: 1) WORRAKAMOL BOONPARIPHAN
//         2) APIRAK RATSAMEESAWANG
// CONTACT: 1) worrakamol@sic.co.th
//          2) apirak@sic.co.th
//
// UPDATE: 2020-08-24
//
// REFERENCE DOCUMENT:
// 1) ...
//
// NOTE:
// 1) ...
//
// 2) ...
//
//----------------------------------------------------------------------------//

#ifndef HFREADER_H
#define HFREADER_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include <stdint.h>

//----------- (2) EXPORTED TYPES ---------------------------------------------//
typedef enum
{
    // SIC interface status
    HFAL_IRQ_SET = 0xA0,
    HFAL_IRQ_CLR = 0xA1,
    HFAL_TIMEOUT_START = 0xA2,
    HFAL_TIMEOUT_STOP = 0xA3,
    HFAL_TIMEOUT_RUN = 0xA4,
    HFAL_TIMEOUT_FAIL = 0xA5,

    // SIC reader RF status
    HFAL_RF_FRAMING_ERR = 0xE1,
    HFAL_RF_COLLISION_ERR = 0xE2,
    HFAL_RF_PARITY_ERR = 0xE3,
    HFAL_RF_CRC_ERR = 0xE4,
    HFAL_RF_BUFFER_OVERFLOW_ERR = 0xF0,
    HFAL_KEY_ERR = 0xF1,

    // SIC reader status
    HFAL_SUCCESS = 0x01,         // Function Success
    HFAL_ERROR = 0xFF,           // Function Failed
    HFAL_NO_RESPONSE = 0xE0,     // No response from any tag
    HFAL_READER_TIMEOUT = 0xF3,  // Reader time out
    HFAL_ASIC_EXE_TIMEOUT = 0xF4 // MCU time out
} hfalStatus_t;

// HF-Reader driver structure standard
typedef struct
{
    void (*flushFIFO)(void);
    void (*clearState)(void);
    void (*crcSetting)(uint8_t, uint8_t);
    void (*send1PulseBit)(uint8_t);
    void (*clearCryptoBit)(void);
    void (*setBitFraming)(uint8_t, uint8_t);
    void (*setCollMaskVal)(uint8_t);
    uint8_t (*readCollPos)(void);
    hfalStatus_t (*transmitCommand)(uint8_t *, uint16_t, uint16_t);
    hfalStatus_t (*receiveCommand)(uint8_t *, uint16_t *, uint16_t);
    hfalStatus_t (*transceiveCommand)(uint8_t *, uint16_t, uint8_t *, uint16_t *, uint16_t);
    hfalStatus_t (*loadKey)(uint8_t *);
    hfalStatus_t (*authentication)(uint8_t *);
    void (*utilDelay)(uint32_t);

    // HF-Reader driver structure extension (for FELICA)
    hfalStatus_t (*extendedTransmit)(uint8_t *, uint16_t, uint16_t);
    hfalStatus_t (*extendedReceive)(uint8_t *, uint16_t *, uint16_t);
    hfalStatus_t (*extendedTransceive)(uint8_t *, uint16_t, uint8_t *,
                                       uint16_t *, uint16_t);
    hfalStatus_t (*transceiveMultiSlots)(uint8_t, uint8_t *, uint16_t,
                                         uint8_t *, uint16_t *, uint16_t);
} HFREADER_DrvTypeDef;

extern HFREADER_DrvTypeDef *hfReaderDrv;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
#ifndef NULL
#define NULL ((void *)0)
#endif

#define HFAL_DISABLE 0
#define HFAL_ENABLE 1
#define HFAL_SETBIT 0x01
#define HFAL_CLRBIT (~HFAL_SETBIT)

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
// N/A

#endif // #ifndef HFREADER_H
