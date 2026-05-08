//! \file:  re41_ex.c
//! \brief: This file provides extended code for using RE41 reader chip to
//          support FELICA tag type.

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
// UPDATE: 2020-11-23
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RE41-R1.3-20190405
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "re41.h"
#include "re41_ex.h"

//----------- (2) GLOBAL VARIABLES -------------------------------------------//
static uint8_t g_irqEn;
static uint8_t g_rstpdEn;
static uint8_t g_timerPrescale;
static uint8_t g_timerReload;

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
 *  \fn      RE41Ext_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef_Ext **hfDriver_Ext)
 *  \brief   Set RE41 HF-Reader IO Extension for FELICA.
 *  \param   irqEn : Enable/Disable IRQ pin
 *           rstpdEn : Enable/Disable RSTPD pin
 *           HFREADER_DrvTypeDef_Ext : HF reader driver extended interface
 *  \return  None
 */
void RE41Ext_init(uint8_t irqEn, uint8_t rstpdEn)
{
    g_irqEn   = irqEn;
    g_rstpdEn = rstpdEn;
}

/*!
 *  \fn      RE41Ext_receiveMultiple(uint8_t rxMultiEn)
 *  \brief   Enable RF multiple reception
 *  \param   rxMultiEn : Enable/Disable data continuous reception
 *               - HFAL_ENABLE  : Enable  RxMultiple for data reception
 *               - HFAL_DISABLE : Disable RxMultiple for data reception
 *  \return  None
 */
void RE41Ext_receiveMultiple(uint8_t rxMultiEn)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RE41_DECODERCTRL, &regVal);
    if(rxMultiEn == HFAL_ENABLE)
    {
        regVal |= SET_RXMULTIPLE;
    }
    else
    {
        regVal &= CLR_RXMULTIPLE;
    }
    HFREADER_IO_write1Reg(RE41_DECODERCTRL, regVal);
}

/*!
 *  \fn      RE41Ext_transmitCommand(uint8_t *dataTx, uint16_t dataTxLen, uint16_t timeOut)
 *  \brief   RE41 reader uses extended transmits RF data command
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41Ext_transmitCommand(uint8_t *dataTx, uint16_t dataTxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t regVal = 0;
    uint8_t i = 0;
    uint8_t txCompleteFlag = 0;

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    // external timer counter start
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
                if((regVal & TX_IRQ) == TX_IRQ)
                {
                    RE41_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ);

                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < dataTxLen; i++)
                    {
                        HFREADER_IO_write1Reg(RE41_FIFODATA, *(dataTx + i));
                    }
                    RE41_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE41_CMDREG, TRANSCEIVE_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE41_clearState();
                    txCompleteFlag = 1;
                    status = HFAL_READER_TIMEOUT;
                }
                else
                {
                    ; // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            txCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE41_clearState();
        }
    } while(!(txCompleteFlag));

    // external timer counter stop
    HFREADER_IO_timStop();

    return status;
}

/*!
 *  \fn      RE41Ext_receiveCommand(uint8_t *dataRx, uint16_t dataRxLen, uint16_t timeOut)
 *  \brief   RE41 reader uses extended receives RF data command
 *  \param   dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data reception
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41Ext_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  rxCompleteFlag = 0;

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    // external timer counter start
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RE41_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RE41_FIFODATA, (dataRx + i));
                    }
                    *(dataRxLen) = FIFOLen;
                    RE41_disableIRQ(ALL_IRQ);
                    rxCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE41_clearState();
                    rxCompleteFlag = 1;
                    status = HFAL_NO_RESPONSE;
                }
                else
                {
                    ; // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            rxCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE41_clearState();
            *(dataRxLen) = 0;
        }
    } while(!(rxCompleteFlag));

    HFREADER_IO_timStop();

    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RE41_checkRFErr();
    }

    return status;
}


/*!
 *  \fn      RE41Ext_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                       uint8_t *dataRx, uint16_t *dataRxLen,
                                       uint16_t timeOut)
 *  \brief   RE41 reader transmit and receive RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data reception
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41Ext_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                       uint8_t *dataRx, uint16_t *dataRxLen,
                                       uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;

    // disable all IRQ
    RE41_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE41_flushFIFO();
    // set RE41 internal timer
    HFREADER_IO_write1Reg(RE41_TIMERCONTROL, (RE41_TSTART_TXEND | RE41_TSTOP_RXBEGIN));

    // Transmit
    status = RE41Ext_transmitCommand(dataTx, dataTxLen, timeOut);
    if(status == HFAL_SUCCESS)
    {
        // Receive
        status = RE41Ext_receiveCommand(dataRx, dataRxLen, timeOut);
    }

    return status;
}

/*!
 *  \fn      RE41Ext_receiveMultiSlots(uint8_t numSlot, uint8_t *dataRx,
                                       uint16_t *dataRxLen, uint16_t timeOut)
 *  \brief   RE41 reader uses multiple slots RF data reception for FELICA multi-slot
 *           request command
 *  \param   numSlot : Specify the number of slot to request
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of received data
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41Ext_receiveMultiSlots(uint8_t numSlot, uint8_t *dataRx,
                                       uint16_t *dataRxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  waterLevel = 0;
    uint8_t  rxCompleteFlag = 0;
    uint16_t index = 0;
    uint8_t  totalSlot = 0;
    uint8_t  rxSlotFlag = 0;

    // enable receive multiple
    RE41Ext_receiveMultiple(HFAL_ENABLE);

    // receive
    regVal = 0x80 | HIALERT_IRQ | TIMER_IRQ | RX_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    // external TIM counter start
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
                if((regVal & RX_IRQ) == RX_IRQ)
                {
                    RE41_clearFlagIRQ(RX_IRQ);
                    HFREADER_IO_read1Reg(RE41_FIFOLEN, &FIFOLen);

                    dataRx[index++] = totalSlot++;
                    dataRx[index++] = RE41_checkRFErr();
                    dataRx[index++] = FIFOLen;
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RE41_FIFODATA, &dataRx[index++]);
                    }
                    rxSlotFlag = 1;
                }
                else if((regVal & HIALERT_IRQ) == HIALERT_IRQ)
                {
                    RE41_clearFlagIRQ(HIALERT_IRQ);
                    HFREADER_IO_read1Reg(RE41_FIFOLEVEL, &waterLevel);
                    for(i = 0; i < (64 - waterLevel); i++)
                    {
                        HFREADER_IO_read1Reg(RE41_FIFODATA, &dataRx[index++]);
                    }
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE41_clearFlagIRQ(TIMER_IRQ);
                    if((totalSlot < numSlot) && (rxSlotFlag == 1))
                    {
                        // clear receive single slot data flag
                        rxSlotFlag = 0;
                    }
                    else if((totalSlot < numSlot) && (rxSlotFlag == 0))
                    {
                        // increase the number of total slot
                        totalSlot++;
                    }
                    else
                    {
                        RE41_clearState();
                        RE41_disableIRQ(ALL_IRQ);
                        RE41_timerStop();
                        rxCompleteFlag = 1;

                        if(index == 0)
                        {
                            *(dataRxLen) = 0;
                            status = HFAL_NO_RESPONSE;
                        }
                        else
                        {
                            *(dataRxLen) = index;
                            status = HFAL_SUCCESS;
                        }
                    }
                }
                else
                {
                    ; // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            rxCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE41_clearState();
            RE41_disableIRQ(ALL_IRQ);
            RE41_timerStop();
            *(dataRxLen) = 0;
        }
    } while(!(rxCompleteFlag));

    HFREADER_IO_timStop();

    // enable receive multiple
    RE41Ext_receiveMultiple(HFAL_DISABLE);

    return status;
}



/*!
 *  \fn      RE41Ext_transceiveMultiSlots(uint8_t numSlot,
 *                                        uint8_t *dataTx, uint16_t  dataTxLen,
                                          uint8_t *dataRx, uint16_t *dataRxLen,
                                          uint16_t timeOut)
 *  \brief   RE41 reader transmit and receive RF data with multiple reception.
 *           This function is exclusive support for FELICA REQC command with
 *           multiple slots
 *  \param   numSlot : Specify the number of slot to request
 *           dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data reception
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41Ext_transceiveMultiSlots(uint8_t numSlot,
                                          uint8_t *dataTx, uint16_t  dataTxLen,
                                          uint8_t *dataRx, uint16_t *dataRxLen,
                                          uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;

    // get RE41's timer configuration
    RE41_getTimerConfigure(&g_timerPrescale, &g_timerReload);
    // configure RE41's timer (uses 2.417 ms + 1.208 ms to be reader timeout)
    RE41_timerConfigure(0x0B, 0x24);
    // enable timer auto restart
    RE41_timerAutoRestart(HFAL_ENABLE);

    // disable all IRQ
    RE41_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE41_flushFIFO();

    // set RE41's timer control
    HFREADER_IO_write1Reg(RE41_TIMERCONTROL, RE41_TSTART_TXEND);

    // Transmit
    status = RE41Ext_transmitCommand(dataTx, dataTxLen, timeOut);
    if(status == HFAL_SUCCESS)
    {
        // configure RE41's timer (use 1.208 ms to be reader timeout)
        RE41_timerConfigure(0x0B, 0x08);
        // Receive
        status = RE41Ext_receiveMultiSlots(numSlot, dataRx, dataRxLen, timeOut);
    }

    // configure RE41's timer to default
    RE41_timerConfigure(g_timerPrescale, g_timerReload);
    // enable timer auto restart
    RE41_timerAutoRestart(HFAL_DISABLE);

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
