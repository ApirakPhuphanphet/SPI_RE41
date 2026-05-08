//! \file:  iso14443b.c
//! \brief: This file provides basic code for using iso14443b protocol

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
// UPDATE: 2020-09-28
//
// REFERENCE DOCUMENT:
// 1) ISO/IEC FCD 14443-3
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "iso14443b.h"

//----------- (2) GLOBAL VARIABLES -------------------------------------------//
// N/A

//----------- (3) PRIVATE TYPEDEF --------------------------------------------//
// N/A

//----------- (4) PRIVATE DEFINE ---------------------------------------------//
// N/A

//----------- (5) PRIVATE MACRO ----------------------------------------------//
// N/A

//----------- (6) PRIVATE VARIABLES ------------------------------------------//
// N/A

//----------- (7) PRIVATE FUNCTION PROTOTYPES --------------------------------//
// N/A

//----------- (8) PRIVATE FUNCTIONS ------------------------------------------//
/*!
 *  \fn      iso14443b_WUPB(uint8_t AFI, uint8_t Num_Slots_N,
 *                      uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443B wake up command
 *  \param   AFI : Application family identifier
 *           Num_Slots_N : Number of slots
 *           slotNumber  : The number of the 1st slot that card is found
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443b_WUPB(uint8_t AFI, uint8_t Num_Slots_N,
                        uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint16_t i = 0;
    uint8_t  Number_of_Slots = 1;
    uint8_t  Slot_Marker;
    uint8_t  txData[4]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;

    // check number of slots
    if(Num_Slots_N > 4)
    {
        Num_Slots_N = 4;
    }
    // calculate number of slots
    while(Num_Slots_N != 0)
    {
        Number_of_Slots *= 2;
        Num_Slots_N--;
    }

    txData[0] = 0x05; // prefix byte APf
    txData[1] = AFI;  // AFI if = 0x00 all tag process the REQB/WUPB
    txData[2] = (0x08 | Num_Slots_N); // PARAM (bit4 select REQB(0) or WUPB(1),
                                      // bit 1-3 (Number of slots))
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 3, rxData, &rxDataLen, 2000);

    // Slot-MARKER command
    Slot_Marker = 2;
    while((Slot_Marker <= Number_of_Slots) && (status != HFAL_SUCCESS))
    {
        txData[0] = (((Slot_Marker - 1) << 4) | 0x05); // APn
        status = hfReaderDrv->transceiveCommand(txData, 1, rxData, &rxDataLen, 2000);
        Slot_Marker++;
        hfReaderDrv->utilDelay(3); // (128 * 250) / fc = 2.35ms
    }

    if(status == HFAL_SUCCESS)
    {
        if(rxDataLen == 12)
        {
            *(slotNumber) = Slot_Marker - 1;
            *(dOutLen)    = rxDataLen - 1;
            for(i = 0; i < (rxDataLen - 1); i++)
            {
                *(dOut + i) = rxData[i + 1];
            }
        }
        else
        {
            status = HFAL_ERROR;
            *(slotNumber) = Slot_Marker - 1;
            *(dOutLen)    = rxDataLen;
            for(i = 0; i < rxDataLen; i++)
            {
                *(dOut + i) = rxData[i];
            }
        }
    }
    else
    {
        *(slotNumber) = 0;
        *(dOutLen)    = rxDataLen;
        for(i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
    }

    return status;
}

/*!
 *  \fn      iso14443b_REQB(uint8_t AFI, uint8_t Num_Slots_N,
 *                      uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443B request command
 *  \param   AFI : Application family identifier
 *           Num_Slots_N : Number of slots
 *           slotNumber  : The number of the 1st slot that card is found
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443b_REQB(uint8_t AFI, uint8_t Num_Slots_N,
                        uint8_t *slotNumber, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint16_t i = 0;
    uint8_t  Number_of_Slots = 1;
    uint8_t  Slot_Marker;
    uint8_t  txData[4]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;

    // check number of slots
    if(Num_Slots_N > 4)
    {
        Num_Slots_N = 4;
    }
    // calculate number of slots
    while(Num_Slots_N != 0)
    {
        Number_of_Slots *= 2;
        Num_Slots_N--;
    }

    txData[0] = 0x05; // prefix byte APf
    txData[1] = AFI;  // AFI if = 0x00 all tag process the REQB/WUPB
    txData[2] = (0x00 | Num_Slots_N); // PARAM (bit4 select REQB(0) or WUPB(1),
                                      // bit 1-3 (Number of slots))
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 3, rxData, &rxDataLen, 2000);

    // Slot-MARKER command
    Slot_Marker = 2;
    while((Slot_Marker <= Number_of_Slots) && (status != HFAL_SUCCESS))
    {
        txData[0] = (((Slot_Marker - 1) << 4) | 0x05); // APn
        status = hfReaderDrv->transceiveCommand(txData, 1, rxData, &rxDataLen, 2000);
        Slot_Marker++;
        hfReaderDrv->utilDelay(3); // (128 * 250) / fc = 2.35ms
    }

    if(status == HFAL_SUCCESS)
    {
        if(rxDataLen == 12)
        {
            *(slotNumber) = Slot_Marker - 1;
            *(dOutLen)    = rxDataLen - 1;
            for(i = 0; i < (rxDataLen - 1); i++)
            {
                *(dOut + i) = rxData[i + 1];
            }
        }
        else
        {
            status = HFAL_ERROR;
            *(slotNumber) = Slot_Marker - 1;
            *(dOutLen)    = rxDataLen;
            for(i = 0; i < rxDataLen; i++)
            {
                *(dOut + i) = rxData[i];
            }
        }
    }
    else
    {
        *(slotNumber) = 0;
        *(dOutLen)    = rxDataLen;
        for(i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
    }

    return status;
}

/*!
 *  \fn      iso14443b_ATTRIB(uint8_t *PUPI, uint8_t *param, uint8_t *hLayer,
 *                       uint16_t hLayerLen, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443B ATTRIB command
 *  \param   PUPI   : Pseudo-Unique PICC Identifier (4 bytes)
 *           param  : 4-bytes parameter following ISO14443B
 *           hLayer : Higher layer information following ISO14443B
 *           hLayerLen : Length of higher layer information
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443b_ATTRIB(uint8_t *PUPI, uint8_t *param, uint8_t *hLayer,
                         uint16_t hLayerLen, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  txData[32] = {0};
    uint16_t txDataLen  = 0;
    uint8_t  rxData[32] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t i = 0;

    txData[txDataLen++] = 0x1D; // ATTRIB command
    // PUPI
    for(i = 0; i < 4; i++)
    {
        txData[txDataLen++] = *(PUPI + i);
    }
    // Param
    for(i = 0; i < 4; i++)
    {
        txData[txDataLen++] = *(param + i);
    }
    // Higher layer INF
    for(i = 0; i < hLayerLen; i++)
    {
        txData[txDataLen++] = *(hLayer + i);
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(txData, txDataLen, rxData, &rxDataLen, 2000);

    *dOutLen = rxDataLen;
    for(i = 0; i < rxDataLen; i++)
    {
        *(dOut + i) = rxData[i];
    }

    return status;
}

/*!
 *  \fn      iso14443b_HLTB(uint8_t *PUPI, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443B halt command
 *  \param   PUPI : Pseudo-Unique PICC Identifier (4 bytes)
 *           dOut : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443b_HLTB(uint8_t *PUPI, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  txData[5]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t i = 0;

    txData[0] = 0x50; // HLTB command
    for(i = 0; i < 4; i++)
    {
        txData[i + 1] = *(PUPI + i);
    }
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 5, rxData, &rxDataLen, 2000);

    *dOutLen = rxDataLen;
    for(i = 0; i < rxDataLen; i++)
    {
        *(dOut + i) = rxData[i];
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
