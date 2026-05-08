//! \file:  iso14443a_MFUltralight.c
//! \brief: This file provides basic code for using iso14443a protocol to 
//          communicate with NFC Tag 2 Type (MIFARE ULTRALIGHT)

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
// UPDATE: 2020-02-11
//
// REFERENCE DOCUMENT:
// 1) ISO/IEC FCD 14443-3
// 2) ISO/IEC FCD 14443-4
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "iso14443a_MFUltralight.h"

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
 *  \fn      mfUltra_readMemory(uint8_t page, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   MIFARE ultra light read memory command (16 byte data)
 *  \param   page    : Specify page memory (address) to read
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfUltra_readMemory(uint8_t page, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  param[2]   = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t i = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0x30; // MIFARE ultra light read command
    param[1] = page; // page number

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 2, rxData, &rxDataLen, 2000);
    if(status == HFAL_SUCCESS)
    {
        for(i = 0; i < rxDataLen; i++)
        {
            dOut[i] = rxData[i];
        }
        *dOutLen = rxDataLen;
    }

    return status;
}

/*!
 *  \fn      mfUltra_writeMemory(uint8_t page, uint8_t *dIn,
 *                                           uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   MIFARE ultra light write memory command (4 bytes data to write)
 *  \param   page    : Specify page memory (address) to write
 *           dIn     : Data to write (4 bytes)
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfUltra_writeMemory(uint8_t page, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  param[6]   = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t i = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0xA2; // MIFARE ultra light write command
    param[1] = page; // address to write
    // data to write (4 bytes)
    for(i = 0; i < 4; i++)
    {
        param[2 + i] = *(dIn + i);
    }
    
    // enable TxCRC, Disable RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_DISABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 6, rxData, &rxDataLen, 2000);
    if(status == HFAL_SUCCESS)
    {
        if((rxDataLen == 1) && (rxData[0] == 0x0A))
        {
            *(dOut) = rxData[0];
            *dOutLen = rxDataLen;
        }
    }

    return status;
}

/*!
 *  \fn      mfUltra_compat_writeMemory(uint8_t page, uint8_t *dIn,
 *                                           uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   MIFARE ultra light compatible write memory command
 *           (16 bytes data to write)
 *  \param   page    : Specify page memory (address) to write
 *           dIn     : Data buffer to write
 *           dOut    : Data buffer stores response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfUltra_compat_writeMemory(uint8_t page, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  param[16]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t i = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0xA0; // MIFARE ultra light compatible write command
    param[1] = page; // Address to write

    // enable TxCRC, Disable RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_DISABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 2, rxData, &rxDataLen, 2000);
    if((status == HFAL_SUCCESS) && (rxDataLen == 1) && (rxData[0] == 0x0A))
    {
        // store 16 bytes of data to write
        for(i = 0; i < 16; i++)
        {
            param[i] = *(dIn + i);
        }
        // execute
        status = hfReaderDrv->transceiveCommand(param, 16, rxData, &rxDataLen, 2000);
        if((status == HFAL_SUCCESS) && (rxDataLen == 1) && (rxData[0] == 0x0A))
        {
            *(dOut)  = rxData[0];
            *dOutLen = rxDataLen;
        }
        else
        {
            *dOutLen = 0;
        }
    }
    else
    {
        *dOutLen = 0;
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
