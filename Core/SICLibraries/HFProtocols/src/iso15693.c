//! \file:  iso15693.c
//! \brief: This file provides basic code for using iso15693 protocol

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

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "iso15693.h"

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
 *  \fn      iso15693_inv1SlotCmd(uint8_t AFI, uint8_t maskLen,
 *                         uint8_t *maskVal, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO15693 inventory 1 slot command
 *  \param   AFI : Tag's AFI specification
 *           maskLen : The number of significant bits of the mask value
 *           maskVal : An integer number of bytes
 *           dOut    : All response data from a tag
 *           dOutLen : Length of response data (8 bytes UID + 1 byte flag +
 *                     1 byte DSFID)
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_inv1SlotCmd(uint8_t AFI, uint8_t maskLen,
                           uint8_t *maskVal, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  i = 0;
    uint8_t  param[12]  = {0};
    uint8_t  paramIndex = 0;
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    hfalStatus_t status = HFAL_ERROR;

    // check AFI
    if(AFI == NON_CHECK_AFI)
    {
        // set Command
        param[paramIndex++] = 0x26; // request flag
        param[paramIndex++] = 0x01; // inventory command
    }
    else
    {
        // Set Command
        param[paramIndex++] = 0x36; // request flag
        param[paramIndex++] = 0x01; // inventory command
        param[paramIndex++] = AFI;  // AFI number
    }
    param[paramIndex++] = maskLen; // the number of significant bit of the mask value

    // check mask length
    if((maskLen % 8) != 0)
    {
        maskLen = (maskLen / 8) + 1;
    }
    else
    {
        maskLen = maskLen / 8;
    }

    // add mask value
    for(i = 0; i < maskLen; i++)
    {
        param[paramIndex++] = *(maskVal + i);
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, paramIndex, rxData, &rxDataLen, 2000);
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
 *  \fn      iso15693_inv16SlotCmd(uint8_t AFI,
 *                                 uint8_t maskLen,   uint8_t  *maskVal,
 *                                 uint8_t *collFlag, uint8_t  *collPos,
 *                                 uint8_t *uid,      uint8_t  *totalTag,
 *                                 uint8_t *dOut,     uint16_t *dOutLen)
 *  \brief   ISO15693 inventory 16 slot command
 *  \param   AFI : Tag's AFI specification
 *           maskLen  : The number of significant bits of the mask value
 *           maskVal  : An integer number of bytes
 *           collFlag : Collision flag will be set if the collision occurs
 *           collPos  : Collision position (Number of slot which is collision)
 *           uid      : UID of tags (1 byte DSFID + 8 bytes UID)
 *           totalTag : Number of found tag without collision
 *           dOut     : All response data from the tag(s)
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_inv16SlotCmd(uint8_t AFI,
                                   uint8_t maskLen,   uint8_t  *maskVal,
                                   uint8_t *collFlag, uint8_t  *collPos,
                                   uint8_t *uid,      uint8_t  *totalTag,
                                   uint8_t *dOut,     uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  param[16]  = {0};
    uint8_t  paramIndex = 0;
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    uint16_t counter;
    uint8_t  slotMarker;
    uint8_t  uidCount;
    uint8_t  index = 0;
    uint8_t  uidStore[16][9] = {0};
    uint8_t  i;

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);

    *totalTag = 0;

    // check AFI mode
    if(AFI == NON_CHECK_AFI)
    {
        // none AFI mode, inventory 16 slots command
        param[paramIndex++] = 0x06; // request flag
        param[paramIndex++] = 0x01; // inventory command
    }
    else
    {
        // AFI mode, Inventory 16 slots command
        param[paramIndex++] = 0x16; // request flag
        param[paramIndex++] = 0x01; // inventory command
        param[paramIndex++] = AFI;  // AFI number
    }
    param[paramIndex++] = maskLen ;

    // check mask length
    if((maskLen % 8) != 0)
    {
        maskLen = (maskLen / 8) + 1;
    }
    else
    {
        maskLen = maskLen / 8;
    }

    // add mask value
    for(i = 0; i < maskLen; i++)
    {
        param[paramIndex++] = *(maskVal + i);
    }

    // count numbers of slot
    for(slotMarker = 0; slotMarker < 16; slotMarker++)
    {
        // slot Number
        if(slotMarker == 0x00)
        {
            status = hfReaderDrv->transceiveCommand(param, paramIndex,
                                                    rxData, &rxDataLen, 2000);
        }
        else
        {
            // sending EOF only
            // Tx Config, set bit send 1 pulse, Bit 7
            hfReaderDrv->send1PulseBit(HFAL_SETBIT);
            // flush FIFO
            hfReaderDrv->flushFIFO();

            // delay Time for high rx data rate: (512*13*8 + 4192)/fc = 4.24 ms
            // delay time for low  rx data rate: (2048*13*8 + 4192)/fc = 16ms
            hfReaderDrv->utilDelay(5);
            status = hfReaderDrv->transceiveCommand(param, 0, rxData,
                                                    &rxDataLen, 2000);
        }

        // check status
        if(status == HFAL_ASIC_EXE_TIMEOUT)
        {
            // Tx Config, clear bit send 1 pulse, Bit 7
            hfReaderDrv->send1PulseBit(HFAL_CLRBIT);
            *(totalTag) = 0;

            return status;
        }
        else if((status == HFAL_SUCCESS) && (rxDataLen == 10))
        {
            *(dOut + (index++)) = slotMarker;
            *(dOut + (index++)) = status;
            *(dOut + (index++)) = rxDataLen - 1;

            // store UID
            for(counter = 0; counter < (rxDataLen - 1); counter++)
            {
                uidStore[*totalTag][counter] = rxData[counter + 1];

                *(dOut + (index++)) = rxData[counter + 1];
            }

            *totalTag = *totalTag + 1;
        }
        else if(status != HFAL_NO_RESPONSE)
        {
            *(dOut + (index++)) = slotMarker;
            *(dOut + (index++)) = status;
            *(dOut + (index++)) = rxDataLen;
            for(counter = 0; counter < rxDataLen; counter++)
            {
                *(dOut + (index++)) = rxData[counter];
            }

            *collFlag = 1;
            *(collPos + slotMarker) = 1;
        }
    }

    // Tx Config, clear bit send 1 pulse, Bit 7
    hfReaderDrv->send1PulseBit(HFAL_CLRBIT);
    // store length of response data
    *dOutLen = index;

    // clear index for store UID
    index = 0;
    for(counter = 0; counter < (*totalTag); counter++)
    {
        for(uidCount = 0; uidCount < 9; uidCount++)
        {
            uid[index++] = uidStore[counter][uidCount];
        }
    }

    // check
    if(*totalTag != 0)
    {
        return HFAL_SUCCESS;
    }
    else
    {
        return HFAL_ERROR;
    }
}

/*!
 *  \fn      iso15693_stayQuiet(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO15693 stay quiet command
 *  \param   uid     : Specify tag's UID to be quieted
 *           dOut    : Response data
 *           dOutLen : Length of response data (shall be 0 if the operation success)
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_stayQuiet(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  i = 0;
    uint8_t  param[10]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0x22;
    param[1] = 0x02; // Stay-Quiet command
    for(i = 0; i < 8; i++)
    {
        param[2 + i] = *(uid + i); // UID 8 Bytes
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 10, rxData, &rxDataLen, 2000);
    if(status == HFAL_NO_RESPONSE)
    {
        status = HFAL_SUCCESS;
        *(dOutLen) = 0;
    }
    else
    {
        status = HFAL_ERROR;
        *(dOutLen) = rxDataLen;
        for(i = 0; i < rxDataLen; i++)
        {
            *(dOut + i) = rxData[i];
        }
    }

    return status;
}

/*!
 *  \fn      iso15693_select(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO15693 select command
 *  \param   uid     : Specify tag's UID to be selected
 *           dOut    : Response data (Flag and Error code)
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_select(uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  i = 0;
    uint8_t  param[10]  = {0};
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    hfalStatus_t status = HFAL_ERROR;

    param[0] = 0x22; // request Flag
    param[1] = 0x25; // select command
    for(i = 0; i < 8; i++)
    {
        param[2 + i] = *(uid + i); // UID 8 Bytes
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, 10, rxData, &rxDataLen, 2000);
    if(status == HFAL_SUCCESS)
    {
        if((rxData[0] & ERROR_FLAG) != ERROR_FLAG)
        {
            *(dOutLen) = rxDataLen;
            for(i = 0; i < rxDataLen; i++)
            {
                *(dOut + i) = rxData[i];
            }
        }
        else // ERROR
        {
            *(dOutLen) = rxDataLen - 1;
            *dOut  = rxData[1];
            status = HFAL_ERROR;
        }
    }
    else
    {
        *(dOutLen) = 0;
    }

    return status;
}

/*!
 *  \fn      iso15693_read1Block(uint8_t uidOpt, uint8_t *uid, uint8_t blockNum,
 *                               uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   ISO15693 read single block command
 *  \param   uidOpt   : UID is optional/not optional
 *           uid      : Specify tag's UID to read memory
 *           blockNum : Block number of memory to read
 *           dOut     : Response data
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_read1Block(uint8_t uidOpt, uint8_t *uid, uint8_t blockNum,
                                 uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  i = 0;
    uint8_t  param[16]  = {0};
    uint8_t  paramIndex = 0;
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    hfalStatus_t status = HFAL_ERROR;

    if(uidOpt == UID_OPTIONAL)
    {
        param[paramIndex++] = 0x02; // address flag is clear
        param[paramIndex++] = 0x20; // read single block command
    }
    else
    {
        param[paramIndex++] = 0x22; // address flag is set
        param[paramIndex++] = 0x20; // read single block command
        for(i = 0; i < 8; i++)
        {
            param[paramIndex++] = *(uid + i); //UID 8 bytes
        }
    }
    param[paramIndex++] = blockNum;

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, paramIndex, rxData,
                                            &rxDataLen, 2000);
    if(status == HFAL_SUCCESS)
    {
        if((rxData[0] & ERROR_FLAG) != ERROR_FLAG)
        {
            *(dOutLen) = rxDataLen;
            for(i = 0; i < rxDataLen; i++)
            {
                *(dOut + i) = rxData[i];
            }
        }
        else
        {
            *(dOutLen) = rxDataLen - 1;
            *dOut  = rxData[1];
            status = HFAL_ERROR;
        }
    }
    else
    {
        *(dOutLen) = 0;
    }

    return status;
}

/*!
 *  \fn      iso15693_write1Block(uint8_t uidOpt,
 *                                uint8_t *uid,       uint8_t blockNum,
 *                                uint8_t *blockData, uint8_t blockLen,
 *                                uint8_t *dOut,      uint16_t *dOutLen)
 *  \brief   ISO15693 write single block command
 *  \param   uidOpt    : UID is optional/not optional
 *           uid       : Specify tag's UID to write memory
 *           blockNum  : Block number of memory to write
 *           blockData : Block data to write
 *           blockLen  : Length of data
 *           dOut      : Response data
 *           dOutLen   : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t iso15693_write1Block(uint8_t uidOpt,
                                  uint8_t *uid,       uint8_t blockNum,
                                  uint8_t *blockData, uint8_t blockLen,
                                  uint8_t *dOut,      uint16_t *dOutLen)
{
    uint8_t  i = 0;
    uint8_t  param[16]  = {0};
    uint16_t paramIndex = 0;
    uint8_t  rxData[16] = {0};
    uint16_t rxDataLen  = 0;
    hfalStatus_t status = HFAL_ERROR;

    if(uidOpt == UID_OPTIONAL)
    {
        param[paramIndex++] = 0x02; // address flag is set
        param[paramIndex++] = 0x21; // write single block command
    }
    else
    {
        param[paramIndex++] = 0x22; // address flag is set
        param[paramIndex++] = 0x21; // write single block command
        for(i = 0; i < 8; i++)
        {
            param[paramIndex++] = *(uid + i); // UID 8 bytes
        }
    }
    param[paramIndex++] = blockNum;

    for(i = 0; i < blockLen; i++)
    {
        param[paramIndex++] = *(blockData + i);
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // execute
    status = hfReaderDrv->transceiveCommand(param, paramIndex, rxData,
                                           &rxDataLen, 2000);
    if(status == HFAL_SUCCESS)
    {
        if((rxData[0] & ERROR_FLAG) != ERROR_FLAG)
        {
            *(dOutLen) = rxDataLen;
            for(i = 0; i < rxDataLen; i++)
            {
                *(dOut + i) = rxData[i];
            }
        }
        else
        {
            *(dOutLen) = rxDataLen - 1;
            *(dOut) = rxData[1];
            status  = HFAL_ERROR;
        }
    }
    else
    {
        *(dOutLen) = 0;
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
