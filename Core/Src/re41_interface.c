#include "iso14443a.h"
#include "hfreader.h"
#include "usart.h"

#include "re41.h"
#include "iso14443a.h"
#include "iso14443a_MFClassic.h"
#include "iso14443a_MFUltralight.h"
#include "iso14443b.h"
#include "iso15693.h"
#include "utilities.h"
#include "re41_interface.h"

#include <string.h>
#include <stdio.h>

#define CLI_SUCCESS 0x00
#define CLI_INVALID_ARGS 0x01
#define CLI_ERROR 0xE0
extern UART_HandleTypeDef huart2;

uint8_t read_type_A(uint8_t addr)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t runningState = 0;
    uint8_t rxData[64] = {0};
    uint16_t rxDataLen = 0;

    RE41_rfOperate(RFOFF);
    HAL_Delay(6);
    RE41_rfOperate(RFON);

    // Scanning a Tag ISO14443A
    RE41_configuration(ISO14443A_106KBPS);

    // WUPA mode
    status = iso14443a_WUPA(rxData, &rxDataLen);
    if (status != HFAL_NO_RESPONSE) // 1st skip "No response" printout
    {
        // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWUPA:", 7, HAL_MAX_DELAY);
        // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    }
    // check
    if (status != HFAL_SUCCESS)
    {
        if (runningState != 0)
        {
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
        return status;
    }

    //------------------------------------------------------------------------//
    runningState++;
    // SLEEP A mode (TypeA)
    status = iso14443a_HLTA(rxData, &rxDataLen);
    // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    // check
    if (status != HFAL_SUCCESS)
    {
        if (runningState != 0)
        {
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
        return status;
    }

    //------------------------------------------------------------------------//
    runningState++;
    // WUPA mode
    // combo command type A : Request + AntiColl + Select
    // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWUPA+AC+SEL:", 14, HAL_MAX_DELAY);
    status = iso14443a_Req_Anti_Sel(SEND_WUPA_CMD, 0, rxData, &rxDataLen);
    if (status == HFAL_SUCCESS)
    {
        for (uint16_t k = 0; k < (rxDataLen - 2); k++)
        {
            rxData[k] = rxData[k + 2];
        }
        rxDataLen -= 2; // neglect cascade level and SAK
    }
    // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);

    //------------------------------------------------------------------------//
    runningState++;
    // Read Data mode
    // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nRead Data:", 11, HAL_MAX_DELAY);
    status = iso14443a_ReadBlock(addr, rxData, &rxDataLen);

    if (status != HFAL_SUCCESS)
    {
        // 💡 เทคนิค: ถ้าอ่านไม่ได้ ไม่ต้อง return ให้พัง
        // เพราะอาจจะแปลว่าเราอ่านจน "สุดปลายหน่วยความจำบัตร" แล้ว
        char *endMsg = " [END OF MEMORY or ERROR]";
        HAL_UART_Transmit(&huart2, (uint8_t *)endMsg, strlen(endMsg), HAL_MAX_DELAY);
    }

    // พิมพ์ข้อมูล 16 ไบต์ที่อ่านได้
    // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    Response(rxData, 4);
    //------------------------------------------------------------------------//

    if (status == HFAL_SUCCESS)
    {
        return 0;
    }
    else
    {
        if (runningState != 0)
        {
            // No execute next command and off field
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
    }

    return 0xFF;
}

uint8_t dump_mem(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t runningState = 0;
    uint8_t rxData[64] = {0};
    uint16_t rxDataLen = 0;

    RE41_rfOperate(RFOFF);
    HAL_Delay(6);
    RE41_rfOperate(RFON);

    // Scanning a Tag ISO14443A
    RE41_configuration(ISO14443A_106KBPS);

    // WUPA mode
    status = iso14443a_WUPA(rxData, &rxDataLen);
    if (status != HFAL_NO_RESPONSE) // 1st skip "No response" printout
    {
        // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWUPA:", 7, HAL_MAX_DELAY);
        // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    }
    // check
    if (status != HFAL_SUCCESS)
    {
        if (runningState != 0)
        {
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
        return status;
    }

    //------------------------------------------------------------------------//
    runningState++;
    // SLEEP A mode (TypeA)
    status = iso14443a_HLTA(rxData, &rxDataLen);
    // check
    if (status != HFAL_SUCCESS)
    {
        if (runningState != 0)
        {
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
        return status;
    }

    //------------------------------------------------------------------------//
    runningState++;
    // WUPA mode
    // combo command type A : Request + AntiColl + Select
    status = iso14443a_Req_Anti_Sel(SEND_WUPA_CMD, 0, rxData, &rxDataLen);
    if (status == HFAL_SUCCESS)
    {
        for (uint16_t k = 0; k < (rxDataLen - 2); k++)
        {
            rxData[k] = rxData[k + 2];
        }
        rxDataLen -= 2; // neglect cascade level and SAK
    }

    //------------------------------------------------------------------------//
    runningState++;
    // Read Data mode
    for (uint8_t i = 0; i < 48; i++) // Read all 16 blocks
    {
        status = iso14443a_ReadBlock(i, rxData, &rxDataLen);

        if (status != HFAL_SUCCESS)
        {
            // 💡 เทคนิค: ถ้าอ่านไม่ได้ ไม่ต้อง return ให้พัง
            // เพราะอาจจะแปลว่าเราอ่านจน "สุดปลายหน่วยความจำบัตร" แล้ว
            char *endMsg = " [END OF MEMORY or ERROR]";
            HAL_UART_Transmit(&huart2, (uint8_t *)endMsg, strlen(endMsg), HAL_MAX_DELAY);
        }
        // re41_cli_rspPrintout(status, rxData, 4, 0);
        Response(rxData, 4);
        //------------------------------------------------------------------------//
    }
    if (status == HFAL_SUCCESS)
    {
        return 0;
    }
    else
    {
        if (runningState != 0)
        {
            // No execute next command and off field
            RE41_rfOperate(RFOFF);
            HAL_Delay(6);
            RE41_rfOperate(RFON);
        }
    }

    return 0xFF;
}

uint8_t write_type_A(uint8_t pageNo, uint8_t *data, uint16_t dataLen)
{
    uint8_t runState = 0;
    hfalStatus_t status = HFAL_ERROR;
    uint8_t rxData[64] = {0};
    uint16_t rxDataLen = 0;

    RE41_rfOperate(RFOFF);
    HAL_Delay(6);
    RE41_rfOperate(RFON);

    // Scanning a Tag ISO14443A
    RE41_configuration(ISO14443A_106KBPS);

    // ------------------------------------------------------------------------//
    runState = 0;
    // 1. WUPA mode
    status = iso14443a_WUPA(rxData, &rxDataLen);
    if (status != HFAL_NO_RESPONSE)
    {
        // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWUPA:", 7, HAL_MAX_DELAY);
        // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    }
    if (status != HFAL_SUCCESS)
    {
        return status;
    }

    //------------------------------------------------------------------------//
    runState++;
    // 2. SLEEP A mode (Reset Tag State)
    // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nSLEEPA:", 9, HAL_MAX_DELAY);
    status = iso14443a_HLTA(rxData, &rxDataLen);
    // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);
    if (status != HFAL_SUCCESS)
    {
        return status;
    }

    //------------------------------------------------------------------------//
    runState++;
    // 3. WUPA + AntiColl + Select
    // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWUPA+AC+SEL:", 14, HAL_MAX_DELAY);
    status = iso14443a_Req_Anti_Sel(SEND_WUPA_CMD, 0, rxData, &rxDataLen);
    if (status == HFAL_SUCCESS)
    {
        for (uint16_t k = 0; k < (rxDataLen - 2); k++)
        {
            rxData[k] = rxData[k + 2];
        }
        rxDataLen -= 2; // neglect cascade level and SAK
    }
    // re41_cli_rspPrintout(status, rxData, rxDataLen, 0);

    if (status != HFAL_SUCCESS)
    {
        return status;
    }

    //------------------------------------------------------------------------//
    runState++;
    // 4. Write Data mode
    // HAL_UART_Transmit(&huart2, (uint8_t *)"\r\nWRITE CMD:", 12, HAL_MAX_DELAY);
    status = iso14443a_WritePage(pageNo, data, dataLen);

    if (status == HFAL_SUCCESS)
    {
        // HAL_UART_Transmit(&huart2, (uint8`_t *)" SUCCESS (ACK 0x0A received)", 28, HAL_MAX_DELAY);
    }
    else
    {
        // HAL_UART_Transmit(&huart2, (uint8_t *)" FAILED (NACK or Timeout)", 25, HAL_MAX_DELAY);
    }

    //------------------------------------------------------------------------//
    return status;
}

uint8_t re41_cli_rspPrintout(hfalStatus_t sts, uint8_t *rsp, uint16_t rspLen, uint8_t opt)
{
    uint8_t tmpBuffer[4] = {0};

    if (opt == 1)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
    }

    if (sts == HFAL_SUCCESS)
    {
        while (rspLen > 0)
        {
            dataToHex(&tmpBuffer[0], *rsp);
            HAL_UART_Transmit(&huart2, tmpBuffer, 2, HAL_MAX_DELAY);
            rsp++;
            rspLen--;
        }
        return CLI_SUCCESS;
    }
    else if (sts == HFAL_NO_RESPONSE)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"No response", 11, HAL_MAX_DELAY);
        return CLI_ERROR;
    }
    else if (sts == HFAL_ASIC_EXE_TIMEOUT)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Timeout", 7, HAL_MAX_DELAY);
        return CLI_ERROR;
    }
    else
    {
        return CLI_ERROR;
    }
}