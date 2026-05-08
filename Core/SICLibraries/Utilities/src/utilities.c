//! \file:  utilities.c
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

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "utilities.h"

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
 *  \fn      toAsciiHex(uint8_t data)
 *  \brief   Convert input data (Half-Byte) to ASCII Hex
 *  \param   data : Half byte of data to convert
 *  \return  ASCII Hex data
 */
uint8_t toAsciiHex(uint8_t data)
{
    if(data <= 9)
    {
        return ('0' + data);
    }
    else
    {
        return (('A' - 10) + data);
    }
}

/*!
 *  \fn      dataToHex(uint8_t *dataOut, uint8_t dataIn)
 *  \brief   Convert input data (1 byte of data) to ASCII Hex
 *  \param   dataOut : Data output in ASCII hex format
 *           dataIn  : Data input in hex format
 *  \return  None
 */
void dataToHex(uint8_t *dataOut, uint8_t dataIn)
{
    *(dataOut++) = toAsciiHex((dataIn & 0xF0) >> 4);
    *(dataOut++) = toAsciiHex( dataIn & 0x0F);
    *(dataOut)   = '\0';
}

/*!
 *  \fn      customToLower(uint8_t *dataStr, uint8_t dataStrLen)
 *  \brief   Convert all data input to lower case
 *  \param   dataStr    : String of data input
 *           dataStrLen : Length of data input
 *  \return  None
 */
void customToLower(uint8_t *dataStr, uint8_t dataStrLen)
{
    uint8_t  i = 0;
    uint8_t  tmpData = 0;

    for(i = 0; i < dataStrLen; i++)
    {
        tmpData = dataStr[i];
        if((tmpData >= 65) && (tmpData <= 90))
        {
            dataStr[i] |= 0x20;
        }
    }
}

/*!
 *  \fn      customParseHEX(uint8_t *s, uint8_t sLen, uint8_t *valHEX)
 *  \brief   Convert string input (ASCII HEX) to data hex
 *  \param   s : String of data input
 *           sLen   : Length of data input
 *           valHEX : Data output in hex format
 *  \return  0x00 if conversion is success
 *           0xFF if conversion is fail
 */
uint8_t customParseHEX(uint8_t *s, uint8_t sLen, uint8_t *valHEX)
{
    const uint8_t sftVal[4] = {0, 4, 8, 12};
    uint8_t  tmpData = 0;
    uint8_t  i = 0;
    uint16_t cal = 0;

    // check length
    if(sLen > 2)
    {
        return 0xFF; // error
    }

    for(i = 0; i < sLen; i++)
    {
        tmpData = s[i];
        if((tmpData >= '0') && (tmpData <= '9'))
        {
            tmpData = tmpData - 48;
        }
        else if((tmpData >= 'a') && (tmpData <= 'f'))
        {
            tmpData = (tmpData - 97) + 10;
        }
        else
        {
            return 0xFF; // error: value such as GHIJ
        }
        // calculate
        cal |= tmpData << (sftVal[(sLen - 1) - i]);
    }
    *valHEX = (uint8_t)(cal & 0xFF);

    return 0; // success
}

/*!
 *  \fn      bytesToChars(uint8_t *byteUID, uint16_t inputLen,
                          uint8_t *charUID, uint16_t *outputlen)
 *  \brief   Convert array of data input to ASCII hex data output
 *  \param   byteUID   : Array of data input
 *           inputLen  : Length of data input
 *           charUID   : Data output in ASCII hex format
 *           outputlen : Length of data output
 *  \return  None
 */
void bytesToChars(uint8_t *byteUID, uint16_t inputLen,
                  uint8_t *charUID, uint16_t *outputlen)
{
    uint8_t i;
    uint8_t dataH;
    uint8_t dataL;

    /*Looping following number of type A's UID*/
    for(i = 0; i < inputLen; i++)
    {
        dataH = (*(byteUID + i) & 0xF0) >> 4;
        *(charUID++) = toAsciiHex(dataH);

        dataL = (*(byteUID + i) & 0x0F);
        *(charUID++) = toAsciiHex(dataL);
    }

    *outputlen = inputLen * 2;
}

/*!
 *  \fn      isDigit(uint8_t c)
 *  \brief   Check character is digit number
 *  \param   c : Input character
 *  \return  0x00 if character is not digit number
 *           otherwise if character is digit number
 */
uint8_t isDigit(uint8_t c)
{
    return (c >= '0') && (c <= '9');
}

/*!
 *  \fn      isHexAlpha(uint8_t c)
 *  \brief   Check character is Hex alphabet
 *  \param   c : Input character
 *  \return  0x00 if character is not Hex alphabet
 *           otherwise if character is Hex alphabet
 */
uint8_t isHexAlpha(uint8_t c)
{
    return ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f'));
}

/*!
 *  \fn      isHexDigit(uint8_t c)
 *  \brief   Check character is Hex digit
 *  \param   c : Input character
 *  \return  0x00 if character is not Hex digit
 *           otherwise if character is Hex digit
 */
uint8_t isHexDigit(uint8_t c)
{
    return isDigit(c) || isHexAlpha(c);
}

/*!
 *  \fn      hexDigitToNum(uint8_t c)
 *  \brief   Convert character to value
 *  \param   c : Input character
 *  \return  Value of character
 */
uint8_t hexDigitToNum(uint8_t c)
{
    if(c >= '0' && c <= '9')
    {
        return (uint8_t)(c - '0');
    }
    if(c >= 'a' && c <= 'f')
    {
        return (uint8_t)(c - 'a') + 10;
    }
    if(c >= 'A' && c <= 'F')
    {
        return (uint8_t)(c - 'A') + 10;
    }

    return 16;
}

/*!
 *  \fn      reverseByte(uint8_t *arr, uint16_t n)
 *  \brief   Reverse byte array
 *  \param   arr : Input array
 *           n   : Size to reverse
 *  \return  None
 */
void reverseByte(uint8_t *arr, uint16_t n)
{
    uint16_t low  = 0;
    uint16_t high = 0;

    for(low = 0, high = (n - 1); low < high; low++, high--)
    {
        uint8_t tmp = arr[low];
        arr[low]  = arr[high];
        arr[high] = tmp;
    }
}

/*!
 *  \fn      parseHexStr(uint8_t *dIn, uint16_t dInLen, uint8_t *dOut,
 *                       uint16_t *dOutLen, uint8_t opt)
 *  \brief   Convert string hex data to hex data
 *  \param   dIn     : String hex data input
 *           dInLen  : Length of data input
 *           dOut    : Data output in hex data format
 *           dOutLen : Length of data output
 *           opt : Reverse/Non-Reverse sequence of data output
 *               - 0 : Non-Reverse
 *               - 1 : Reverse
 *  \return  0x00 if conversion is success
 *           0xFF if conversion is fail
 *  Example (1)
 *  opt:     0
 *  dIn:     "aabbccdd"
 *  dInLen:  8
 *  dOut:    0xaa, 0xbb, 0xcc, 0xdd
 *  dOutLen: 4
 *
 *  Example (2)
 *  opt:     1
 *  dIn:     "aabbccdd"
 *  dInLen:  8
 *  dOut:    0xdd, 0xcc, 0xbb, 0xaa
 *  dOutLen: 4
 */
uint8_t parseHexStr(uint8_t *dIn, uint16_t dInLen,
                                uint8_t *dOut, uint16_t *dOutLen, uint8_t opt)
{
    uint8_t  i = 0;
    uint16_t tLen = 0;

    // check length
    if((dInLen % 2) != 0)
    {
        return 0xFF; // FAILED
    }
    // check hex string
    for(i = 0; i < dInLen; i++)
    {
        if(isHexDigit(dIn[i]) == 0)
        {
            return 0xFF; // FAILED
        }
    }
    // calculate
    tLen = (dInLen / 2);
    for(i = 0; i < tLen; i++)
    {
        if(opt == 0)
        {
            dOut[i] = ((dIn[(i * 2)] % 32 + 9) % 25) * 16 +
                       (dIn[(i * 2) + 1] % 32 + 9) % 25;
        }
        else
        {
            dOut[(tLen - 1) - i] = ((dIn[(i * 2)] % 32 + 9) % 25) * 16 +
                                    (dIn[(i * 2) + 1] % 32 + 9) % 25;
        }
    }
    *dOutLen = tLen;

    return 0;
}

/*!
 *  \fn      reverse8BitData(uint8_t dIn, uint8_t *dOut)
 *  \brief   Reverse 8 bits of input data
 *  \param   dIn : Input data
 *           dOut: Reversed input data
 *  \return  None
 */
void reverse8BitData(uint8_t dIn, uint8_t *dOut)
{
    uint8_t  i;
    uint8_t  reverse = 0;

    for (i = 0; i < 8; i++)
    {
        reverse = reverse << 1 ;
        if((dIn & 0x01) == 1)
        {
            reverse = reverse | 1;
        }
        dIn = dIn >> 1;
    };

    *dOut = reverse;
}

/*!
 *  \fn      reverse16BitData(uint16_t dIn, uint16_t *dOut)
 *  \brief   Reverse 16 bits of input data
 *  \param   dIn : Input data
 *           dOut: Reversed input data
 *  \return  None
 */
void reverse16BitData(uint16_t dIn, uint16_t *dOut)
{
    uint8_t   i;
    uint16_t  reverse = 0;

    for (i = 0; i < 16; i++)
    {
        reverse = reverse << 1 ;
        if((dIn & 0x01) == 1)
        {
            reverse = reverse | 1;
        }
        dIn = dIn >> 1;
    };

    *dOut = reverse;
}
//----------- (9) END OF FILE ------------------------------------------------//
