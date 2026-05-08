//! \file:  felica.c
//! \brief: This file provides basic code for using FELICA protocol

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
// UPDATE: 2020-11-24
//
// REFERENCE DOCUMENT:
// 1) JIS-X-6319-4 : 2005
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "felica.h"

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
 *  \fn      felica_REQC(uint8_t *systemCode, uint8_t numSlot, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   FELICA REQC command
 *  \param   systemCode : Use to specify the application by PCD
 *           timeSlot : The maximum time slot number for PICC to avoid collision.
 *              - REQC_1_SLOT : Maximum 1 time slot
 *              - REQC_2_SLOT : Maximum 2 time slots
 *              - REQC_4_SLOT : Maximum 4 time slots
 *              - REQC_8_SLOT : Maximum 8 time slots
 *              - REQC_16_SLOT: Maximum 16 time slots
 *           dOut    : Response data.
 *           Frame Format -> slot + reader's status + length of data + data package
 *           Example -> 00(slot 0) + 01(SUCCESS) + 13(19 bytes) +
 *                      1201012E44A7A50F468800F100000001430040 (data package)
 *                      02(slot 2) + E4(CRC ERR) + 15(21 bytes) +
 *                      1201002600E7A507B16700700000000001000BBF48 (data error package)
 *                      07(slot 7) + 01(SUCCESS) + 13(19bytes) +
 *                      1201012E44A7A50E257700F100000001430040 (data package)
 *           dOutLen : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t felica_REQC(uint8_t *systemCode, uint8_t numSlot, uint8_t *dOut, uint16_t *dOutLen)
{
    uint8_t  reqCmd = 0x00;
    uint8_t  txData[12]  = {0};
    uint8_t  txDataLen = 0;
    hfalStatus_t status = HFAL_ERROR;

    // length of data (1 byte) + REQC data (5 bytes)
    txData[txDataLen++] = 0x06;
    // assign REQC command
    txData[txDataLen++] = reqCmd;
    // system code
    txData[txDataLen++] = *(systemCode);
    txData[txDataLen++] = *(systemCode + 1);
    // reserved byte
    txData[txDataLen++] = 0x00;

    // maximum time slot number
    switch(numSlot)
    {
        case REQC_1_SLOT:
            // time out = delay time (2.417 ms) + response 1 slot (1.208 ms)
            txData[txDataLen++] = REQC_1_SLOT;
            break;
        case REQC_2_SLOT:
            // time out = delay time (2.417 ms) + response 2 slot (1.208 * 2 ms)
            txData[txDataLen++] = REQC_2_SLOT;
            break;
        case REQC_4_SLOT:
            // time out = delay time (2.417 ms) + response 4 slot (1.208 * 4 ms)
            txData[txDataLen++] = REQC_4_SLOT;
            break;
        case REQC_8_SLOT:
            // time out = delay time (2.417 ms) + response 8 slot (1.208 * 8 ms)
            txData[txDataLen++] = REQC_8_SLOT;
            break;
        case REQC_16_SLOT:
            // time out = delay time (2.417 ms) + response 16 slot (1.208 * 16 ms)
            txData[txDataLen++] = REQC_16_SLOT;
            break;
        default:
            // time out = delay time (2.417 ms) + response 2 slot (1.208 ms)
            txData[txDataLen++] = REQC_1_SLOT;
            break;
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // transmit and receive data through RF with 500 ms timeout
    status = hfReaderDrv->transceiveMultiSlots(numSlot, txData, txDataLen,
                                                   dOut, dOutLen, 500);

    return status;
}

/*!
 *  \fn      felica_readCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                uint8_t *blockList,   uint8_t numBlock,
                                uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   FELICA read blocks command
 *  \param   serviceCode : The specified service code dependent on each FELICA
 *                         tag type. But the service code can be got by request
 *                         service command.
 *           numServiceCode : The number of services
 *           blockList : The block list element shall be composed of the element
 *                       length flag, the access mode, the order of services and
 *                       the record number. There are 2 block list element types
 *                       are 2-byte block list and 3-byte block list.
 *           numBlock : Number of blocks to be read.
 *           uid      : Specify NFCID of a tag.
 *           dOut     : Response data
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t felica_readCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                uint8_t *blockList,   uint8_t numBlock,
                                uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t readCmd = 0x06;
    uint8_t txData[64] = {0};
    uint8_t txDataLen = 0;
    uint8_t tmpDataLen;
    uint8_t blockListType;
    uint8_t i;

    // length of data (1 byte) + READ cmd (1 bytes) + IDm (8 bytes)
    // + number of service (1 byte, n) + service code (2xn bytes)
    // + number of block (1 byte, m) + block list (2xm to 3xm bytes)
    tmpDataLen = 12 + (numServiceCode * 2);

    // check block list element
    if(*blockList & 0x80)// 2 bytes of block list element
    {
        tmpDataLen += 2 * numBlock;
        blockListType = 2;
    }
    else// 3 bytes of block list element
    {
        tmpDataLen += 3 * numBlock;
        blockListType = 3;
    }

    // set length of data
    txData[txDataLen++] = tmpDataLen;
    // set FELICA read command
    txData[txDataLen++] = readCmd;
    // set IDm
    for(i = 0; i < 8; i++)
    {
        txData[txDataLen++] = *(uid + i);
    }
    // set service specification
    txData[txDataLen++] = numServiceCode;
    for(i = 0; i < (numServiceCode * 2); i++)
    {
        txData[txDataLen++] = *(serviceCode + i);
    }
    // set number of blocks
    txData[txDataLen++] = numBlock;
    // set block list element
    for(i = 0; i < (blockListType * numBlock); i++)
    {
        txData[txDataLen++] = *(blockList + i);
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // extended transmit and receive data through RF
    status = hfReaderDrv->extendedTransceive(txData, txDataLen, dOut,
                                                 dOutLen, 500);

    return status;
}

/*!
 *  \fn      felica_writeCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                 uint8_t *blockList,   uint8_t numBlock,
                                 uint8_t *blockData, uint8_t blockLen,
                                 uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
 *  \brief   FELICA write blocks command
 *  \param   serviceCode : The specified service code dependent on each FELICA
 *                         tag type. But the service code can be got by request
 *                         service command.
 *           numServiceCode : The number of services
 *           blockList : The block list element shall be composed of the element
 *                       length flag, the access mode, the order of services and
 *                       the record number. There are 2 block list element types
 *                       are 2-byte block list and 3-byte block list.
 *           numBlock : Number of blocks to be written.
 *           blockData : Block address to be written.
 *           blockLen : Length of data to be written.
 *           uid      : Specify NFCID of a tag.
 *           dOut     : Response data
 *           dOutLen  : Length of response data
 *  \return  HFAL_SUCCESS : The operation success
 *           otherwise    : The operation fail
 */
hfalStatus_t felica_writeCommand(uint8_t *serviceCode, uint8_t numServiceCode,
                                 uint8_t *blockList,   uint8_t numBlock,
                                 uint8_t *blockData,   uint8_t blockLen,
                                 uint8_t *uid, uint8_t *dOut, uint16_t *dOutLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t writeCmd = 0x08;
    uint8_t txData[64] = {0};
    uint8_t txDataLen = 0;
    uint8_t tmpDataLen;
    uint8_t blockListType;
    uint8_t i;

    // length of data (1 byte) + WRITE cmd (1 bytes) + IDm (8 bytes)
    // + number of service (1 byte, n) + service code (2xn bytes)
    // + number of block (1byte, m) + block list (2xm to 3xm bytes)
    // + block data (16xm bytes)
    tmpDataLen = 12 + (numServiceCode * 2) + (numBlock * 16);

    // check block list element
    if(*blockList & 0x80)// 2 bytes of block list element
    {
       tmpDataLen += 2 * numBlock;
        blockListType = 2;
    }
    else// 3 bytes of block list element
    {
        tmpDataLen += 3 * numBlock;
        blockListType = 3;
    }

    // set length of data
    txData[txDataLen++] = tmpDataLen;
    // set FELICA write command
    txData[txDataLen++] = writeCmd;
    // set IDm
    for(i = 0; i < 8; i++)
    {
        txData[txDataLen++] = *(uid + i);
    }
    // set service specification
    txData[txDataLen++] = numServiceCode;
    for(i = 0; i < (numServiceCode * 2); i++)
    {
        txData[txDataLen++] = *(serviceCode + i);
    }
    // set number of blocks
    txData[txDataLen++] = numBlock;
    // set block list element
    for(i = 0; i < (blockListType * numBlock); i++)
    {
        txData[txDataLen++] = *(blockList + i);
    }
    // set block data
    for(i = 0; i < blockLen; i++)
    {
        txData[txDataLen++] = *(blockData + i);
    }

    // enable TxCRC, RxCRC
    hfReaderDrv->crcSetting(HFAL_ENABLE, HFAL_ENABLE);
    // flush FIFO
    hfReaderDrv->flushFIFO();
    // extended transmit and receive data through RF
    status = hfReaderDrv->extendedTransceive(txData, txDataLen, dOut,
                                                 dOutLen, 1000);

    return status;
}


//----------- (9) END OF FILE ------------------------------------------------//
