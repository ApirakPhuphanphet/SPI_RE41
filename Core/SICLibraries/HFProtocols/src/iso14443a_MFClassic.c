//! \file:  iso14443a_MFClassic.c
//! \brief: This file provides basic code for using iso14443a protocol to 
//          communicate with NFC Tag MIFARE

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
// UPDATE: 2021-02-11
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
#include "iso14443a_MFClassic.h"

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
 *  \fn      mfClassic_loadKey(uint8_t *key)
 *  \brief   MIFARE classic load master key command (6 bytes)
 *  \param   key : Pointer points to 6 bytes of master key to load
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfClassic_loadKey(uint8_t *key)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  i;
    uint8_t  nibbleMSB;
    uint8_t  nibbleLSB;
    uint8_t  tmpKey;
    uint8_t  convertKey[12] = {0};

    // disable TXCRC, RXCRC
    hfReaderDrv->crcSetting(HFAL_DISABLE, HFAL_DISABLE);
    // convert 6 bytes key to 12 bytes key
    for(i = 0; i < 6; i++)
    {
        tmpKey = *(key + i);
        nibbleMSB = (tmpKey >> 4) & 0x0F;
        nibbleLSB = tmpKey & 0x0F;
        convertKey[i * 2] = (((~nibbleMSB) << 4) & 0xF0) | nibbleMSB;
        convertKey[(i * 2) + 1] = (((~nibbleLSB) << 4) & 0xF0) | nibbleLSB;
    }
    // use reader load key function to load master key to FIFO
    status = hfReaderDrv->loadKey(convertKey);

    return status;
}

/*!
 *  \fn      mfClassic_authentication(mfClassic_selectKey_t selectKey,
 *                                             uint8_t blockNum, uint8_t *uid)
 *  \brief   MIFARE classic authentication command
 *  \param   selectKey : Select key to authenticate
 *           blockNum  : The address of data block to be authenticated
 *           uid : Last 4 bytes of UID of Tag MIFARE
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfClassic_authentication(mfClassic_selectKey_t selectKey,
                                               uint8_t blockNum, uint8_t *uid)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  i;
    uint8_t  param[6] = {0};

    if(selectKey == USE_KEY_B)
    {
        param[0] = MIFARE_AUTHENT_KEY_B;
    }
    else
    {
        param[0] = MIFARE_AUTHENT_KEY_A;
    }

    param[1] = blockNum;
    for(i = 0; i < 4; i++)
    {
        param[2 + i] = *(uid + i);
    }

    // send authentication command (fixed 6-bytes)
    status = hfReaderDrv->authentication(param);
    
    return status;
}

/*!
 *  \fn      mfClassic_writeBlock(uint8_t blockNum, uint8_t *dIn,
 *                                           uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   MIFARE classic write memory command (16 bytes data to write)
 *  \param   blockNum : The address of data block to be written
 *           dIn      : Data to write (16 bytes)
 *           dOut     : Response data
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfClassic_writeBlock(uint8_t blockNum, uint8_t *dIn,
                                             uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  param[16]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint8_t  i = 0;
    
    param[0] = 0xA0; // MIFARE write command
    param[1] = blockNum; // address to write

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
        if(status == HFAL_SUCCESS)
        {
            if((rxDataLen == 1) && (rxData[0] == 0x0A))
            {
                *(dOut)  = rxData[0];
                *dOutLen = rxDataLen;
            }
            else
            {
                *dOutLen = 0;
                status = HFAL_ERROR;
            }
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

/*!
 *  \fn      mfClassic_readBlock(uint8_t blockNum,
 *                                           uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   MIFARE classic read memory command (16 bytes data)
 *  \param   blockNum : The address of data block to be read
 *           dOut     : Response data
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t mfClassic_readBlock(uint8_t blockNum,
                                             uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  param[2]   = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint8_t  i = 0;

    param[0] = 0x30; // MIFARE classic read command
    param[1] = blockNum; // Address to read

    // enable TXCRC, RXCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 2, rxData, &rxDataLen, 2000);
    if((status == HFAL_SUCCESS) && (rxDataLen == 16))
    {
        for(i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
        *dOutLen = rxDataLen;
    }
    else
    {
        *dOutLen = 0;
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
