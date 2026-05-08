//! \file:  iso14443a.c
//! \brief: This file provides basic code for using iso14443a protocol

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
#include "iso14443a.h"

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
 *  \fn      iso14443a_REQA(uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A request command
 *  \param   dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_REQA(uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t reqCmd = 0x26;
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    uint16_t i = 0;
    hfalStatus_t status = HFAL_ERROR;

    // clear Crypto
    hfReaderDrv->clearCryptoBit();
    // set Bit Framing 7 bit
    hfReaderDrv->setBitFraming(0, 7);
    // disable CRC
    hfReaderDrv->crcSetting(HFAL_DISABLE, HFAL_DISABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(&reqCmd, 1, rxData, &rxDataLen, 2000);
    if (status == HFAL_SUCCESS)
    {
        if (rxDataLen != 2)
        {
            status = HFAL_ERROR;
        }

        for (i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
        *dOutLen = rxDataLen;
    }

    return status;
}

/*!
 *  \fn      iso14443a_WUPA(uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A wake up command
 *  \param   dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_WUPA(uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t wkpCmd = 0x52;
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    uint16_t i = 0;
    hfalStatus_t status = HFAL_ERROR;

    // clear Crypto
    hfReaderDrv->clearCryptoBit();
    // set Bit Framing 7 bit
    hfReaderDrv->setBitFraming(0, 7);
    // disable CRC
    hfReaderDrv->crcSetting(HFAL_DISABLE, HFAL_DISABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(&wkpCmd, 1, rxData, &rxDataLen, 2000);
    if (status == HFAL_SUCCESS)
    {
        if (rxDataLen != 2)
        {
            status = HFAL_ERROR;
        }

        for (i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
        *dOutLen = rxDataLen;
    }

    return status;
}

/*!
 *  \fn      iso14443a_HLTA(uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A halt A command
 *  \param   dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_HLTA(uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t param[2] = {0};
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;

    // set HLTA Command
    param[0] = 0x50;
    param[1] = 0x00;
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 2, rxData, &rxDataLen, 2000);
    if (status == HFAL_NO_RESPONSE)
    {
        *dOutLen = 0;
        status = HFAL_SUCCESS; // HLTA command: No response
    }
    else
    {
        status = HFAL_ERROR;
    }

    return status;
}

/*!
 *  \fn      iso14443a_Read(uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A read command
 *  \param   dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_ReadBlock(uint8_t blockNo, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t param[2];
    uint8_t rxData[18] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0x30;
    param[1] = blockNo;

    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    hfReaderDrv->flushFIFO();

    status = hfReaderDrv->transceiveCommand(param, 2, rxData, &rxDataLen, 2000);

    if (status == HFAL_SUCCESS && rxDataLen >= 16)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            dOut[i] = rxData[i];
        }

        *dOutLen = 16;
        return HFAL_SUCCESS;
    }
    else
    {
        *dOutLen = 0;
        return HFAL_ERROR;
    }
}

/*!
 *  \fn      iso14443a_Write(uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A write command
 *  \param   dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */

hfalStatus_t iso14443a_WritePage(uint8_t pageNo, uint8_t *data, uint16_t dataLen)
{
    // 1 page bytes = 4 bytes
    uint8_t param[6] = {0};
    uint8_t rxData[4] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;
    if (dataLen != 4)
    {
        return HFAL_ERROR; // Invalid data length for write page
    }

    param[0] = 0xA2;
    param[1] = pageNo;

    for (uint8_t i = 0; i < 4; i++)
    {
        param[i + 2] = data[i];
    }

    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_DISABLE);
    hfReaderDrv->flushFIFO();
    status = hfReaderDrv->transceiveCommand(param, 6, rxData, &rxDataLen, 2000);
    if (status == HFAL_SUCCESS && rxDataLen == 1 && rxData[0] == 0x0A)
    {
        return HFAL_SUCCESS; // ACK received
    }
    else
    {

        return HFAL_ERROR; // Write failed
    }

    return status;
}

/*!
 *  \fn      iso14443a_AC(uint8_t level, uint8_t collMaskVal, uint8_t *numColl,
 *                        uint8_t *collPos, uint8_t *uid, uint16_t *uidLen)
 *  \brief   ISO14443A Anti-Collision command
 *  \param   level : Cascade level
 *           collMaskVal : Value of collision bit
 *           numColl : Number of collision occurred
 *           collPos : Bit position of collision occurred
 *           uid     : UID of the tag
 *           uidLen  : Length of UID
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_AC(uint8_t level, uint8_t collMaskVal, uint8_t *numColl,
                          uint8_t *collPos, uint8_t *uid, uint16_t *uidLen)
{
    uint16_t i;
    uint8_t cmptFlag = 0;
    uint8_t SEL;

    uint8_t checksumUID = 0;
    uint16_t tmpUIDLen = 0;
    uint8_t tmpUID[8] = {0};
    uint8_t tmpNumColl = 0;
    uint8_t tmpCollPos = 0;

    uint8_t totalUIDBit = 0;
    uint8_t rxByteLen = 0, rxBitLen = 0;
    uint8_t rxBytePos = 0, rxBitPos = 0;
    uint8_t txByteLen = 0;
    uint8_t txBytePos = 0, txBitPos = 0;

    uint8_t txData[8] = {0};
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;

    // disable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_DISABLE, HFAL_DISABLE);
    // check collision mask bit
    if (collMaskVal == 1)
    {
        hfReaderDrv->setCollMaskVal(1);
    }
    else
    {
        hfReaderDrv->setCollMaskVal(0);
    }
    // select code depend on cascade level
    if (level == 2)
    {
        SEL = 0x95;
    }
    else if (level == 3)
    {
        SEL = 0x97;
    }
    else
    {
        SEL = 0x93;
    }
    txData[0] = SEL;  // select code
    txData[1] = 0x20; // NVB command

    do
    {
        // set bit framing
        hfReaderDrv->setBitFraming(rxBitPos, txBitPos);
        // execute
        status = hfReaderDrv->transceiveCommand(txData, txByteLen + 2,
                                                rxData, &rxDataLen, 2000);
        if (status == HFAL_RF_COLLISION_ERR)
        {
            tmpCollPos = hfReaderDrv->readCollPos();
            if (tmpCollPos > 0) // collision
            {
                totalUIDBit = totalUIDBit + tmpCollPos;
                *(collPos + tmpNumColl) = totalUIDBit;
                tmpNumColl++;

                // RxD -------------------------------------------------------//
                rxByteLen = (rxBitPos + tmpCollPos) / 8;
                rxBitLen = (rxBitPos + tmpCollPos) % 8;
                if (rxBitLen != 0)
                {
                    rxByteLen++;
                }
                else
                {
                    ; // do nothing
                }

                tmpUID[rxBytePos] |= ((0xFF << rxBitPos) & rxData[0]);
                for (i = 1; i < rxByteLen; i++)
                {
                    tmpUID[rxBytePos + i] = rxData[i];
                }

                // TxD -------------------------------------------------------//
                txBytePos = totalUIDBit / 8;
                txBitPos = totalUIDBit % 8;
                if (txBitPos != 0)
                {
                    txByteLen = txBytePos + 1;
                }
                else
                {
                    txByteLen = txBytePos;
                }

                txData[0] = SEL; // select code
                txData[1] = ((txBytePos + 2) * 16) +
                            ((txBitPos) * 1); // NVB command
                for (i = 0; i < txByteLen; i++)
                {
                    txData[i + 2] = tmpUID[i];
                }

                rxBitPos = txBitPos;
                rxBytePos = txBytePos;

                hfReaderDrv->utilDelay(1); // delay -> actual  ((128*48 + 1172) / fc) = 536 us
            }
            else
            {
                cmptFlag = 1;
                status = HFAL_RF_FRAMING_ERR;
                *(collPos) = 0;
                tmpUIDLen = rxDataLen;
                for (i = 0; i < rxDataLen; i++)
                {
                    tmpUID[i] = rxData[i];
                }
            }
        }
        else
        {
            cmptFlag = 1;
            if (tmpNumColl != 0)
            {
                tmpUIDLen = rxBytePos + rxDataLen;
                tmpUID[rxBytePos] |= ((0xFF << rxBitPos) & rxData[0]);
                for (i = 1; i < rxDataLen; i++)
                {
                    tmpUID[rxBytePos + i] = rxData[i];
                }
            }
            else
            {
                *(collPos) = 0;
                tmpUIDLen = rxDataLen;
                for (i = 0; i < rxDataLen; i++)
                {
                    tmpUID[i] = rxData[i];
                }
            }
        }
    } while ((cmptFlag == 0) && (tmpNumColl <= 32));

    if (tmpNumColl > 32) // UID 4 Byte -> Max collision occur = 32 times
    {
        status = HFAL_ERROR;
    }

    if ((status == HFAL_SUCCESS) && (tmpUIDLen == 5))
    {
        for (i = 0; i < 4; i++) // calculate checksum of UID 4 bytes
        {
            checksumUID ^= tmpUID[i];
        }

        if (checksumUID == tmpUID[4]) // check with BCC
        {
            tmpUIDLen = 4; // send only UID 4 bytes
        }
        else
        {
            status = HFAL_ERROR; // return UID 4 Bytes and BCC
        }
    }
    else
    {
        status = HFAL_ERROR;
    }

    // pack data out
    *(numColl) = tmpNumColl;
    *(uidLen) = tmpUIDLen;
    for (i = 0; i < tmpUIDLen; i++)
    {
        *(uid + i) = tmpUID[i];
    }

    return status;
}

/*!
 *  \fn      iso14443a_SELECT(uint8_t level, uint8_t *UID,
 *                                           uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A select command
 *  \param   level   : Cascade level
 *           UID     : UID of the tag
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_SELECT(uint8_t level, uint8_t *UID,
                              uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t i;
    uint8_t BCC;
    uint8_t txData[8] = {0};
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;

    if (level == 2)
    {
        txData[0] = 0x95; // ISO14443A_CASCADE_LEVEL_2_RF_CMD
    }
    else if (level == 3)
    {
        txData[0] = 0x97; // ISO14443A_CASCADE_LEVEL_3_RF_CMD
    }
    else
    {
        txData[0] = 0x93; // ISO14443A_CASCADE_LEVEL_1_RF_CMD
    }

    txData[1] = 0x70; // NVB
    BCC = 0;
    for (i = 0; i < 4; i++)
    {
        txData[i + 2] = *(UID + i); // UID 4 bytes
        BCC ^= *(UID + i);
    }
    txData[6] = BCC; // BCC

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 7, rxData, &rxDataLen, 2000);
    if ((status == HFAL_SUCCESS) && (rxDataLen != 1)) // LEN_ISO14443A_SELECT_RF_RESP
    {
        status = HFAL_ERROR;
    }

    *(dOutLen) = rxDataLen;
    for (i = 0; i < rxDataLen; i++)
    {
        *(dOut + i) = rxData[i];
    }

    return status;
}

/*!
 *  \fn      iso14443a_Req_Anti_Sel(iso14443a_startCommand_t cmd,
 *                                  uint8_t collMaskVal,
 *                                  uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A combo command (Request/Wakeup + AntiCollision + Select)
 *           to get UID
 *  \param   cmd: Start command to get tag's uid
 *               - SEND_REQA_CMD: Start command with request command
 *               - SEND_WUPA_CMD: Start command with wake up command
 *           collMaskVal : Value of collision bit
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_Req_Anti_Sel(iso14443a_startCommand_t cmd,
                                    uint8_t collMaskVal, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t rxData[8] = {0};
    uint16_t rxDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;
    uint16_t i = 0;
    uint8_t cmptFlag = 0;
    uint8_t level = 0;
    uint8_t errIndx = 0;
    uint8_t numColl = 0;
    uint8_t collPos[32] = {0};
    uint8_t tmpUID[4] = {0};
    uint8_t UID[8] = {0};
    uint8_t UIDLen = 0;
    uint8_t indxCmd = 1;

    level = 1; // cascade level-1

    if (cmd == SEND_REQA_CMD)
    {
        status = iso14443a_REQA(rxData, &rxDataLen); // request command
    }
    else
    {
        status = iso14443a_WUPA(rxData, &rxDataLen); // wake up command
    }

    if (status == HFAL_SUCCESS)
    {
        do
        {
            // anti-collision command
            status = iso14443a_AC(level, collMaskVal, &numColl, collPos, rxData, &rxDataLen);
            if (status == HFAL_SUCCESS)
            {
                for (i = 0; i < 4; i++) // store UID
                {
                    tmpUID[i] = rxData[i];
                }
                // select command
                status = iso14443a_SELECT(level, tmpUID, rxData, &rxDataLen);
                if (status == HFAL_SUCCESS)
                {
                    if ((rxData[0] & 0x04) != 0x04) // SAK_UID_COMPLETE_MASK
                    {
                        for (i = 0; i < 4; i++) // UID completed
                        {
                            UID[UIDLen++] = tmpUID[i];
                        }
                        cmptFlag = 1;
                    }
                    else // UID not completed
                    {
                        for (i = 1; i < 4; i++)
                        {
                            UID[UIDLen++] = tmpUID[i];
                        }
                        level++;
                    }
                }
                else
                {
                    cmptFlag = 1;          // select error
                    errIndx = indxCmd + 2; // error location
                }
            }
            else
            {
                cmptFlag = 1;          // anti-collision error
                errIndx = indxCmd + 1; // error location
            }
        } while ((cmptFlag == 0) && (level <= 3));

        if (level > 3)
        {
            status = HFAL_ERROR;   // select Err (UID not completed)
            errIndx = indxCmd + 2; // error location
        }
    }
    else
    {
        errIndx = indxCmd;
        rxDataLen = 0;
    }

    // pack data out
    if (status == HFAL_SUCCESS)
    {
        *(dOut++) = level;
        *(dOut++) = rxData[0]; // SAK
        for (i = 0; i < UIDLen; i++)
        {
            *(dOut++) = UID[i];
        }
        *(dOutLen) = UIDLen + 2;
    }
    else
    {
        *(dOut++) = errIndx;
        for (i = 0; i < rxDataLen; i++)
        {
            *(dOut++) = rxData[i];
        }
        *(dOutLen) = rxDataLen + 1;
    }

    return status;
}

/*!
 *  \fn      iso14443a_RATS(uint8_t param, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A RATS (request for answer to select) command
 *  \param   param :
 *               FSDI defines the max size of a frame which the reader is able
 *               to receive. (Bit8:5)
 *               '0' = 16 bytes
 *               '1' = 24 bytes
 *               '2' = 32 bytes
 *               '3' = 40 bytes
 *               '4' = 48 bytes
 *               '5' = 64 bytes
 *               '6' = 96 bytes
 *               '7' = 128 bytes
 *               '8' = 256 bytes
 *               CID defines the logical number of the addressed tag in the
 *               range from 0 to 14. (Bit4:1)
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_RATS(uint8_t param, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t txData[2] = {0};
    uint8_t rxData[16] = {0};
    uint16_t rxDataLen = 0;
    uint16_t i = 0;

    txData[0] = 0xE0;  // RATS start byte
    txData[1] = param; // parameter byte code FSDI and CID
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 2, rxData, &rxDataLen, 2000);
    if (status == HFAL_SUCCESS)
    {
        if (rxDataLen >= 1)
        {
            if (rxDataLen != rxData[0])
            {
                status = HFAL_ERROR;
            }
        }
        else
        {
            status = HFAL_ERROR;
        }
    }

    *(dOutLen) = rxDataLen;
    for (i = 0; i < rxDataLen; i++)
    {
        *(dOut + i) = rxData[i];
    }

    return status;
}

/*!
 *  \fn      iso14443a_PPS(uint8_t CID, uint8_t PPS0, uint8_t PPS1,
 *                                      uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO14443A PPS(Protocol and Parameter Selection request) command
 *  \param   CID : CID defines the logical number of the addressed tag in the
 *               range from 0 to 14. (Bit4:1)
 *           PPS0 : Set/Clear bit-5
 *               - PPS1 is transmitted, if set to 1
 *               - PPS1 isn't transmitted, if set to 0
 *           PPS1 : DSI (Bit4:3) the selected divisor integer from tag to reader
 *                  DRI (Bit2:1) the selected divisor integer from reader to tag
 *           dOut    : Response data
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso14443a_PPS(uint8_t CID, uint8_t PPS0, uint8_t PPS1,
                           uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t txData[4] = {0};
    uint8_t rxData[16] = {0};
    uint16_t rxDataLen = 0;
    uint16_t i = 0;

    txData[0] = 0xD0 | (0x0F & CID); // PPSS Start byte (b8-b5:1101, b4-b1:CID)
    txData[1] = PPS0;                // PPS0 Parameter0 codes presence of PPS1
    txData[2] = PPS1;                // PPS1 Parameter1 codes DRI and DSI
    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // execute
    status = hfReaderDrv->transceiveCommand(txData, 3, rxData, &rxDataLen, 2000);
    if (status == HFAL_SUCCESS)
    {
        if (rxDataLen != 1)
        {
            status = HFAL_ERROR;
        }
    }

    *(dOutLen) = rxDataLen;
    for (i = 0; i < rxDataLen; i++)
    {
        *(dOut + i) = rxData[i];
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
