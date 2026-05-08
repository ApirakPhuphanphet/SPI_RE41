//! \file:  re31.c
//! \brief: This file provides basic code for using RE31 reader chip.

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
// UPDATE: 2021-03-22
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RE31-R1.3-20190405
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "re31.h"

//----------- (2) GLOBAL VARIABLES -------------------------------------------//
HFREADER_DrvTypeDef re31Drv_hfReader =
{
    RE31_flushFIFO,
    RE31_clearState,
    RE31_crcSetting,
    RE31_send1PulseBit,
    RE31_clearCryptoBit,
    RE31_setBitFraming,
    RE31_setCollMaskVal,
    RE31_readCollPos,
    RE31_transmitCommand,
    RE31_receiveCommand,
    RE31_transceiveCommand,
    RE31_loadKey,
    RE31_authenticate,
    RE31_utilDelay,
    // Extended function to support FELICA
    0,
    0,
    0,
    0
};

static uint8_t  g_irqEn   = 0;
static uint8_t  g_rstpdEn = 0;

//----------- (3) PRIVATE TYPEDEF --------------------------------------------//
// N/A

//----------- (4) PRIVATE DEFINE ---------------------------------------------//
// N/A

//----------- (5) PRIVATE MACRO ----------------------------------------------//
// N/A

//----------- (6) PRIVATE VARIABLES ------------------------------------------//
// default configuration for ISO14443A protocol with data rate 106 kbps
static re31_configParam_t config_iso14443aParam_106Kbps =
{
    0x5B, // TxControl
    0x3F, // TxCfgCW
    0x10, // TxCfgMod
    0x19, // CoderControl
    0x0F, // ModWidth
    0x0F, // ModWidthSOF
    0x3B, // TypeBFraming

    0x6B, // RxControl1
    0x28, // DecoderControl
    0x40, // BitPhase
    0x4C, // RxThreshold
    0x02, // BPSKDemControl
    0x41, // RxControl2
    0xF0, // RxControl3

    0x05, // RxWait
    0x03, // ChannelRedundancy
    0x63, // CRCPresetMSB
    0x63, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0A, // TimerClock
    0x06, // TimerControl
    0xFF, // TimerReloadValue
    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x09  // GainST3
};

// default configuration for ISO14443B protocol with data rate 106 kbps
static re31_configParam_t config_iso14443bParam_106Kbps =
{
    0x4B, // TxControl
    0x3F, // TxCfgCW
    0x10, // TxCfgMod
    0x20, // CoderControl
    0x3F, // ModWidth
    0x3F, // ModWidthSOF
    0x3B, // TypeBFraming

    0x6B, // RxControl1
    0x19, // DecoderControl
    0x20, // BitPhase
    0x4C, // RxThreshold
    0x02, // BPSKDemControl
    0x41, // RxControl2
    0xF0, // RxControl3

    0x03, // RxWait
    0x2E, // ChannelRedundancy
    0xFF, // CRCPresetMSB
    0xFF, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0A, // TimerClock
    0x00, // TimerControl
    0xFF, // TimerReloadValue
    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x09  // GainST3
};

// default configuration for ISO15693 protocol with Tx: 1 out of 4, Rx: 1 Sub Hi
static re31_configParam_t config_iso15693Param_1Out4_1SubHi =
{
    0x5B, // TxControl
    0x36, // TxCfgCW
    0x10, // TxCfgMod
    0x2F, // CoderControl
    0x3F, // ModWidth
    0x3F, // ModWidthSOF
    0x3B, // TypeBFraming

    0x93, // RxControl1
    0x30, // DecoderControl
    0x40, // BitPhase
    0x88, // RxThreshold
    0x04, // BPSKDemControl
    0x41, // RxControl2
    0xE3, // RxControl3

    0x01, // RxWait
    0x2C, // ChannelRedundancy
    0xFF, // CRCPresetMSB
    0xFF, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0A, // TimerClock
    0x06, // TimerControl
    0xFF, // TimerReloadValue
    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x21  // GainST3
};

//----------- (7) PRIVATE FUNCTION PROTOTYPES --------------------------------//
// N/A

//----------- (8) PRIVATE FUNCTIONS ------------------------------------------//
/*!
 *  \fn      RE31_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
 *  \brief   Set RE31 HF-Reader IO Initialization.
 *  \param   irqEn : Enable/Disable IRQ pin
 *           rstpdEn : Enable/Disable RSTPD pin
 *           hfDriver : HF reader driver interface
 *  \return  None
 */
void RE31_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
{
    g_irqEn   = irqEn;
    g_rstpdEn = rstpdEn;
    *hfDriver = &re31Drv_hfReader;

    HFREADER_IO_init(g_irqEn, g_rstpdEn);
}

/*!
 *  \fn      RE31_checkIRQ(uint8_t irqEn)
 *  \brief   Check IRQ of RE31 reader by reading IRQ pin or IRQ flag register.
 *  \param   irqEn : IRQ pin configuration from RE31_init function
 *  \return  1 : There is an IRQ is set
 *           0 : There is no any IRQ is set
 */
uint8_t RE31_checkIRQ(uint8_t irqEn)
{
    uint8_t  regVal1 = 0;
    uint8_t  regVal2 = 0;
    
    if(irqEn)
    {
        return HFREADER_IO_readIrq();
    }
    else
    {
        HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal1);
        HFREADER_IO_read1Reg(RE31_IRQEN, &regVal2);

        return (regVal1 & regVal2 & 0x7F);
    }
}

/*!
 *  \fn      RE31_utilDelay(uint32_t ms)
 *  \brief   Delay time in milli-seconds
 *  \param   ms : Timeout value in milli-seconds
 *  \return  None
 */
void RE31_utilDelay(uint32_t ms)
{
    HFREADER_IO_delay(ms);
}

/*!
 *  \fn      RE31_analyze(void)
 *  \brief   Analyze RE31 reader interface by reading/writing FIFO register
 *  \param   None
 *  \return  HFAL_SUCCESS : the operation success
 *           otherwise    : the operation fail
 */
hfalStatus_t RE31_analyze(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t rdData[2] = {0};
    uint8_t wrData = 0xAA;

    // clear state of RE31 reader to idle state
    RE31_clearState();

    // flush FIFO and check FIFO length
    RE31_flushFIFO();
    HFREADER_IO_read1Reg(RE31_FIFOLEN, &rdData[0]);
    if(rdData[0] == 0)
    {
        // write FIFO then read FIFO length and read FIFO
        HFREADER_IO_write1Reg(RE31_FIFODATA, wrData);
        HFREADER_IO_read1Reg(RE31_FIFOLEN, &rdData[0]);
        HFREADER_IO_read1Reg(RE31_FIFODATA, &rdData[1]);
        status = HFAL_SUCCESS;
    }
    else
    {
        status = HFAL_ERROR;
    }

    // check FIFO length and FIFO data
    if((rdData[0] == 1) && (rdData[1] == wrData) && (status == HFAL_SUCCESS))
    {
        // check Lo-Alert IRQ flag
        if(RE31_getFlagIRQ(LOALERT_IRQ) == LOALERT_IRQ)
        {
            // flush FIFO and return SUCCESS
            RE31_flushFIFO();
            status = HFAL_SUCCESS;
        }
        else
        {
            status = HFAL_ERROR;
        }
    }
    else
    {
        status = HFAL_ERROR;
    }

    return status;
}

/*!
 *  \fn      RE31_getRevision(void)
 *  \brief   Get RE31 revision
 *  \param   None
 *  \return  RE31 device revision
 */
re31_deviceRev_t RE31_getRevision(void)
{
    uint8_t regVal = 0;
    re31_deviceRev_t re31Rev = 0;

    HFREADER_IO_read1Reg(RE31_DEVICETYPE, &regVal);
    // check RE31 version
    switch(regVal)
    {
        case 0x3B:
            re31Rev = RE31_CODE;
            break;
        default:
            re31Rev = RE31_UNKNOWN;
            break;
    }

    return re31Rev;
}

/*!
 *  \fn      RE31_configRFParam(re31_configParam_t *param, re31_configProtocol_t protocol)
 *  \brief   Assign default parameter value by the specified protocol
 *  \param   param : Pointer points to RE31 configure parameter structure
 *           protocol : RF protocol specification
 *  \return  None
 */
static void RE31_configRFParam(re31_configParam_t *param, re31_configProtocol_t protocol)
{
    if(protocol == ISO14443A_106KBPS)
    {
        param->TxControl         = config_iso14443aParam_106Kbps.TxControl;
        param->TxCfgCW           = config_iso14443aParam_106Kbps.TxCfgCW;
        param->TxCfgMod          = config_iso14443aParam_106Kbps.TxCfgMod;
        param->CoderControl      = config_iso14443aParam_106Kbps.CoderControl;
        param->ModWidth          = config_iso14443aParam_106Kbps.ModWidth;
        param->ModWidthSOF       = config_iso14443aParam_106Kbps.ModWidthSOF;
        param->TypeBFraming      = config_iso14443aParam_106Kbps.TypeBFraming;

        param->RxControl1        = config_iso14443aParam_106Kbps.RxControl1;
        param->DecoderControl    = config_iso14443aParam_106Kbps.DecoderControl;
        param->BitPhase          = config_iso14443aParam_106Kbps.BitPhase;
        param->RxThreshold       = config_iso14443aParam_106Kbps.RxThreshold;
        param->BPSKDemControl    = config_iso14443aParam_106Kbps.BPSKDemControl;
        param->RxControl2        = config_iso14443aParam_106Kbps.RxControl2;
        param->RxControl3        = config_iso14443aParam_106Kbps.RxControl3;

        param->RxWait            = config_iso14443aParam_106Kbps.RxWait;
        param->ChannelRedundancy = config_iso14443aParam_106Kbps.ChannelRedundancy;
        param->CRCPresetMSB      = config_iso14443aParam_106Kbps.CRCPresetMSB;
        param->CRCPresetLSB      = config_iso14443aParam_106Kbps.CRCPresetLSB;

        param->FIFOLevel         = config_iso14443aParam_106Kbps.FIFOLevel;
        param->TimerClock        = config_iso14443aParam_106Kbps.TimerClock;
        param->TimerControl      = config_iso14443aParam_106Kbps.TimerControl;
        param->TimerReloadValue  = config_iso14443aParam_106Kbps.TimerReloadValue;
        param->ManualFilter      = config_iso14443aParam_106Kbps.ManualFilter;
        param->FilterAdjust      = config_iso14443aParam_106Kbps.FilterAdjust;

        param->GainST3           = config_iso14443aParam_106Kbps.GainST3;
    }
    else if(protocol == ISO14443B_106KBPS)
    {
        param->TxControl         = config_iso14443bParam_106Kbps.TxControl;
        param->TxCfgCW           = config_iso14443bParam_106Kbps.TxCfgCW;
        param->TxCfgMod          = config_iso14443bParam_106Kbps.TxCfgMod;
        param->CoderControl      = config_iso14443bParam_106Kbps.CoderControl;
        param->ModWidth          = config_iso14443bParam_106Kbps.ModWidth;
        param->ModWidthSOF       = config_iso14443bParam_106Kbps.ModWidthSOF;
        param->TypeBFraming      = config_iso14443bParam_106Kbps.TypeBFraming;

        param->RxControl1        = config_iso14443bParam_106Kbps.RxControl1;
        param->DecoderControl    = config_iso14443bParam_106Kbps.DecoderControl;
        param->BitPhase          = config_iso14443bParam_106Kbps.BitPhase;
        param->RxThreshold       = config_iso14443bParam_106Kbps.RxThreshold;
        param->BPSKDemControl    = config_iso14443bParam_106Kbps.BPSKDemControl;
        param->RxControl2        = config_iso14443bParam_106Kbps.RxControl2;
        param->RxControl3        = config_iso14443bParam_106Kbps.RxControl3;

        param->RxWait            = config_iso14443bParam_106Kbps.RxWait;
        param->ChannelRedundancy = config_iso14443bParam_106Kbps.ChannelRedundancy;
        param->CRCPresetMSB      = config_iso14443bParam_106Kbps.CRCPresetMSB;
        param->CRCPresetLSB      = config_iso14443bParam_106Kbps.CRCPresetLSB;

        param->FIFOLevel         = config_iso14443bParam_106Kbps.FIFOLevel;
        param->TimerClock        = config_iso14443bParam_106Kbps.TimerClock;
        param->TimerControl      = config_iso14443bParam_106Kbps.TimerControl;
        param->TimerReloadValue  = config_iso14443bParam_106Kbps.TimerReloadValue;
        param->ManualFilter      = config_iso14443bParam_106Kbps.ManualFilter;
        param->FilterAdjust      = config_iso14443bParam_106Kbps.FilterAdjust;

        param->GainST3           = config_iso14443bParam_106Kbps.GainST3;
    }
    else if(protocol == ISO15693_1OUT4_1SUB_HI)
    {
        param->TxControl         = config_iso15693Param_1Out4_1SubHi.TxControl;
        param->TxCfgCW           = config_iso15693Param_1Out4_1SubHi.TxCfgCW;
        param->TxCfgMod          = config_iso15693Param_1Out4_1SubHi.TxCfgMod;
        param->CoderControl      = config_iso15693Param_1Out4_1SubHi.CoderControl;
        param->ModWidth          = config_iso15693Param_1Out4_1SubHi.ModWidth;
        param->ModWidthSOF       = config_iso15693Param_1Out4_1SubHi.ModWidthSOF;
        param->TypeBFraming      = config_iso15693Param_1Out4_1SubHi.TypeBFraming;

        param->RxControl1        = config_iso15693Param_1Out4_1SubHi.RxControl1;
        param->DecoderControl    = config_iso15693Param_1Out4_1SubHi.DecoderControl;
        param->BitPhase          = config_iso15693Param_1Out4_1SubHi.BitPhase;
        param->RxThreshold       = config_iso15693Param_1Out4_1SubHi.RxThreshold;
        param->BPSKDemControl    = config_iso15693Param_1Out4_1SubHi.BPSKDemControl;
        param->RxControl2        = config_iso15693Param_1Out4_1SubHi.RxControl2;
        param->RxControl3        = config_iso15693Param_1Out4_1SubHi.RxControl3;

        param->RxWait            = config_iso15693Param_1Out4_1SubHi.RxWait;
        param->ChannelRedundancy = config_iso15693Param_1Out4_1SubHi.ChannelRedundancy;
        param->CRCPresetMSB      = config_iso15693Param_1Out4_1SubHi.CRCPresetMSB;
        param->CRCPresetLSB      = config_iso15693Param_1Out4_1SubHi.CRCPresetLSB;

        param->FIFOLevel         = config_iso15693Param_1Out4_1SubHi.FIFOLevel;
        param->TimerClock        = config_iso15693Param_1Out4_1SubHi.TimerClock;
        param->TimerControl      = config_iso15693Param_1Out4_1SubHi.TimerControl;
        param->TimerReloadValue  = config_iso15693Param_1Out4_1SubHi.TimerReloadValue;
        param->ManualFilter      = config_iso15693Param_1Out4_1SubHi.ManualFilter;
        param->FilterAdjust      = config_iso15693Param_1Out4_1SubHi.FilterAdjust;

        param->GainST3           = config_iso15693Param_1Out4_1SubHi.GainST3;
    }
    else // default configuration to support ISO14443A protocol
    {
        param->TxControl         = config_iso14443aParam_106Kbps.TxControl;
        param->TxCfgCW           = config_iso14443aParam_106Kbps.TxCfgCW;
        param->TxCfgMod          = config_iso14443aParam_106Kbps.TxCfgMod;
        param->CoderControl      = config_iso14443aParam_106Kbps.CoderControl;
        param->ModWidth          = config_iso14443aParam_106Kbps.ModWidth;
        param->ModWidthSOF       = config_iso14443aParam_106Kbps.ModWidthSOF;
        param->TypeBFraming      = config_iso14443aParam_106Kbps.TypeBFraming;

        param->RxControl1        = config_iso14443aParam_106Kbps.RxControl1;
        param->DecoderControl    = config_iso14443aParam_106Kbps.DecoderControl;
        param->BitPhase          = config_iso14443aParam_106Kbps.BitPhase;
        param->RxThreshold       = config_iso14443aParam_106Kbps.RxThreshold;
        param->BPSKDemControl    = config_iso14443aParam_106Kbps.BPSKDemControl;
        param->RxControl2        = config_iso14443aParam_106Kbps.RxControl2;
        param->RxControl3        = config_iso14443aParam_106Kbps.RxControl3;

        param->RxWait            = config_iso14443aParam_106Kbps.RxWait;
        param->ChannelRedundancy = config_iso14443aParam_106Kbps.ChannelRedundancy;
        param->CRCPresetMSB      = config_iso14443aParam_106Kbps.CRCPresetMSB;
        param->CRCPresetLSB      = config_iso14443aParam_106Kbps.CRCPresetLSB;

        param->FIFOLevel         = config_iso14443aParam_106Kbps.FIFOLevel;
        param->TimerClock        = config_iso14443aParam_106Kbps.TimerClock;
        param->TimerControl      = config_iso14443aParam_106Kbps.TimerControl;
        param->TimerReloadValue  = config_iso14443aParam_106Kbps.TimerReloadValue;
        param->ManualFilter      = config_iso14443aParam_106Kbps.ManualFilter;
        param->FilterAdjust      = config_iso14443aParam_106Kbps.FilterAdjust;

        param->GainST3           = config_iso14443aParam_106Kbps.GainST3;
    }
}

/*!
 *  \fn      RE31_configuration(re31_configProtocol_t configProtocol)
 *  \brief   Configure register of RE31 follow the specified protocol
 *  \param   configProtocol : RF protocol structure
 *  \return  HFAL_SUCCESS : The reader configuration success
 *           otherwise : The reader configuration failed
 */
hfalStatus_t RE31_configuration(re31_configProtocol_t configProtocol)
{
    re31_configParam_t configParam;
    re31_deviceRev_t deviceRev;

    // get RE31 revision
    deviceRev = RE31_getRevision();

    if(deviceRev != RE31_UNKNOWN)
    {
        // assign register value
        RE31_configRFParam(&configParam, configProtocol);

        HFREADER_IO_write1Reg(RE31_TXCTRL,        configParam.TxControl);
        HFREADER_IO_write1Reg(RE31_TXCFGPWR,      configParam.TxCfgCW);
        HFREADER_IO_write1Reg(RE31_TXCFGMOD,      configParam.TxCfgMod);
        HFREADER_IO_write1Reg(RE31_CODERCTRL,     configParam.CoderControl);
        HFREADER_IO_write1Reg(RE31_MODWIDTH,      configParam.ModWidth);
        HFREADER_IO_write1Reg(RE31_MODWIDTHSOF,   configParam.ModWidthSOF);
        HFREADER_IO_write1Reg(RE31_TYPEBFRAMING,  configParam.TypeBFraming);

        HFREADER_IO_write1Reg(RE31_RXCTRL1,       configParam.RxControl1);
        HFREADER_IO_write1Reg(RE31_DECODERCTRL,   configParam.DecoderControl);
        HFREADER_IO_write1Reg(RE31_BITPHASE,      configParam.BitPhase);
        HFREADER_IO_write1Reg(RE31_RXTHRESHOLD,   configParam.RxThreshold);
        HFREADER_IO_write1Reg(RE31_BPSKDEMOD,     configParam.BPSKDemControl);
        HFREADER_IO_write1Reg(RE31_RXCTRL2,       configParam.RxControl2);
        HFREADER_IO_write1Reg(RE31_RXCTRL3,       configParam.RxControl3);

        HFREADER_IO_write1Reg(RE31_RXWAIT,        configParam.RxWait);
        HFREADER_IO_write1Reg(RE31_CRCSETTING,    configParam.ChannelRedundancy);
        HFREADER_IO_write1Reg(RE31_CRCPRESETLSB,  configParam.CRCPresetMSB);
        HFREADER_IO_write1Reg(RE31_CRCPRESETMSB,  configParam.CRCPresetLSB);

        HFREADER_IO_write1Reg(RE31_FIFOLEVEL,     configParam.FIFOLevel);
        HFREADER_IO_write1Reg(RE31_TIMERCLOCK,    configParam.TimerClock);
        HFREADER_IO_write1Reg(RE31_TIMERCONTROL,  configParam.TimerControl);
        HFREADER_IO_write1Reg(RE31_TIMERRELOAD,   configParam.TimerReloadValue);
        HFREADER_IO_write1Reg(RE31_MANFILTERCTRL, configParam.ManualFilter);
        HFREADER_IO_write1Reg(RE31_FILTERADJUST,  configParam.FilterAdjust);

        HFREADER_IO_write1Reg(RE31_GAIN_ST3,      configParam.GainST3);

        return HFAL_SUCCESS;
    }
    else
    {
        return HFAL_ERROR;
    }
}

/*!
 *  \fn      RE31_enableIRQ(uint8_t irqEnable)
 *  \brief   Enable interrupt of RE31
 *  \param   irqEnable : Required interrupt to enable
 *  \return  None
 */
void RE31_enableIRQ(uint8_t irqEnable)
{
    uint8_t regVal;

    // set bit-7 to enable interrupt
    regVal = SETIRQ_EN | (irqEnable & ALL_IRQ);
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal);
}

/*!
 *  \fn      RE31_disableIRQ(uint8_t irqDisable)
 *  \brief   Disable interrupt and clear interrupt Flag
 *  \param   irqDisable : Required interrupt will be disabled and cleared
 *  \return  None
 */
void RE31_disableIRQ(uint8_t irqDisable)
{
    uint8_t regVal;

    regVal = irqDisable & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal); // disable interrupt
    HFREADER_IO_write1Reg(RE31_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RE31_clearFlagIRQ(uint8_t irqFlag)
 *  \brief   Clear interrupt flag of RE31
 *  \param   irqFlag : Required interrupt flag to set
 *  \return  None
 */
void RE31_clearFlagIRQ(uint8_t irqFlag)
{
    uint8_t regVal;

    regVal = irqFlag & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RE31_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RE31_getFlagIRQ(uint8_t getFlag)
 *  \brief   Get interrupt flag
 *  \param   getFlag : Specified interrupt will be checked
 *  \return  Interrupt requested flag if matched
 */
uint8_t RE31_getFlagIRQ(uint8_t getFlag)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);
    regVal &= 0x7F;
    if((getFlag & regVal) == getFlag)
    {
        return getFlag;
    }
    else
    {
        return regVal;
    }
}

/*!
 *  \fn      RE31_timerStart(void)
 *  \brief   Start RE31 reader's timer
 *  \param   None
 *  \return  None
 */
void RE31_timerStart(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_CTRLREG, &regVal); // read control register
    regVal |= TSTART_BIT; // set start timer bit (bit-1)
    HFREADER_IO_write1Reg(RE31_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE31_timerStop(void)
 *  \brief   Stop RE31 reader's timer
 *  \param   None
 *  \return  None
 */
void RE31_timerStop(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_CTRLREG, &regVal); // read control register
    regVal |= TSTOP_BIT; // set stop timer bit (bit-2)
    HFREADER_IO_write1Reg(RE31_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE31_timerAutoRestart(uint8_t restartEn)
 *  \brief   Auto-Restart RE31 reader's timer
 *  \param   restartEn : Configure auto restart timer bit
 *              - HFAL_ENABLE  : Enable  auto restart timer
 *              - HFAL_DISABLE : Disable auto restart timer
 *  \return  None
 */
void RE31_timerAutoRestart(uint8_t restartEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_TIMERCLOCK, &regVal);
    if(restartEn == HFAL_ENABLE)
    {
        regVal |= SET_TAUTORESTART;
    }
    else
    {
        regVal &= CLR_TAUTORESTART;
    }
    HFREADER_IO_write1Reg(RE31_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RE31_timerConfigure(uint8_t prescale, uint8_t reload)
 *  \brief   Configure RE31's timer period
 *  \param   prescale : Timer's prescale value (LSB 5 bits)
 *           reload   : Timer's reload value
 *  \return  None
 */
void RE31_timerConfigure(uint8_t prescale, uint8_t reload)
{
    uint8_t regVal;

    // configure timer reload value
    HFREADER_IO_write1Reg(RE31_TIMERRELOAD, reload);

    // configure timer prescaler value
    HFREADER_IO_read1Reg(RE31_TIMERCLOCK, &regVal);
    regVal = (uint8_t)((regVal & 0xE0) | (prescale & 0x1F));
    HFREADER_IO_write1Reg(RE31_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RE31_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
 *  \brief   Get RE31's timer configuration
 *  \param   prescale : Pointer to a buffer to stores timer's prescale value
 *           reload   : Pointer to a buffer to stores timer's reload value
 *  \return  None
 */
void RE31_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
{
    uint8_t regVal;

    // get timer reload
    HFREADER_IO_read1Reg(RE31_TIMERRELOAD, &regVal);
    *prescale = regVal;

    // get timer prescaler
    HFREADER_IO_read1Reg(RE31_TIMERCLOCK, &regVal);
    regVal &= 0x1F;
    *reload = regVal;
}

/*!
 *  \fn      RE31_flushFIFO(void)
 *  \brief   Flush RE31 reader's FIFO
 *  \param   None
 *  \return  None
 */
void RE31_flushFIFO(void)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE31_CTRLREG, &regVal); // read control register
    regVal |= FLUSHFIFO_BIT; // set Flush FIFO bit (bit-0)
    HFREADER_IO_write1Reg(RE31_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE31_clearState(void)
 *  \brief   Clear state of RE31 reader
 *  \param   None
 *  \return  None
 */
void RE31_clearState(void)
{
    RE31_disableIRQ(ALL_IRQ); // disable All IRQ
    HFREADER_IO_write1Reg(RE31_CMDREG, IDLE_CMD); // write IDLE command to command register
    RE31_flushFIFO();
}

/*!
 *  \fn      RE31_rfOperate(re31_rfControl_t rfCtrl)
 *  \brief   Turn on/off RF Field
 *  \param   rfCtrl : Structure of RF control (on, off, Tx1En or Tx2En)
 *  \return  None
 */
void RE31_rfOperate(re31_rfControl_t rfCtrl)
{
    uint8_t  rdVal;
    uint8_t  wrVal;

    switch(rfCtrl)
    {
        case RFON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE31_TXCTRL, &rdVal);
            // set Only Bit TX2RFEn and TX1RFEn
            wrVal = rdVal | TX2RFEn_MASK | TX1RFEn_MASK;
            HFREADER_IO_write1Reg(RE31_TXCTRL, wrVal);
            break;
        case RFOFF:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE31_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE31_TXCTRL, wrVal);
            break;
        case TX1ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE31_TXCTRL, &rdVal);
            // set bit-TX1 and clear bit-TX2
            wrVal = (rdVal | TX1RFEn_MASK) & (~TX2RFEn_MASK);
            HFREADER_IO_write1Reg(RE31_TXCTRL, wrVal);
            break;
        case TX2ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE31_TXCTRL, &rdVal);
            // set bit-TX2 and clear bit-TX1
            wrVal = (rdVal | TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE31_TXCTRL, wrVal);
            break;
        // off field
        default :
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE31_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE31_TXCTRL, wrVal);
            break;
    }
}

/*!
 *  \fn      RE31_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
 *  \brief   Set RF transmit/receive CRC
 *  \param   txCRCEn : Data transmission with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data transmission
 *               - HFAL_DISABLE : Disable CRC for data transmission
 *           rxCRCEn : Data receiving with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data receiving
 *               - HFAL_DISABLE : Disable CRC for data receiving
 *  \return  None
 */
void RE31_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE31_CRCSETTING, &regVal);
    // TxD
    if(txCRCEn)
    {
        regVal |= (1 << 2); // set bit-2
    }
    else
    {
        regVal &= ~(1 << 2); // clear bit-2
    }
    // RxD
    if(rxCRCEn)
    {
        regVal |= (1 << 3); // set bit-3
    }
    else
    {
        regVal &= ~(1 << 3); // clear bit-3
    }
    HFREADER_IO_write1Reg(RE31_CRCSETTING, regVal);
}

/*!
 *  \fn      RE31_send1PulseBit(uint8_t onePulseBit)
 *  \brief   One pulse bit setting of the only one modulation to switch to the
 *           next time slot (ISO15693 protocol)
 *  \param   onePluseBit : One pulse bit setting
 *               - SET1PULSE_BIT : Send 1 pulse bit
 *               - CLR1PULSE_BIT : Doesn't send 1 pulse bit
 *  \return  None
 */
void RE31_send1PulseBit(uint8_t onePulseBit)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_CODERCTRL, &regVal);
    if((onePulseBit == SET1PULSE_BIT) || (onePulseBit == HFAL_SETBIT))
    {
        HFREADER_IO_write1Reg(RE31_CODERCTRL, (regVal | SET1PULSE_BIT));
    }
    else
    {
        HFREADER_IO_write1Reg(RE31_CODERCTRL, (regVal & CLR1PULSE_BIT));
    }
}

/*!
 *  \fn      RE31_clearCryptoBit(void)
 *  \brief   CryptoM engine bit is switched on to encrypted RF communication
 *           which is set only if the authentication process is successful.
 *  \param   None
 *  \return  None
 */
void RE31_clearCryptoBit(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_CTRLREG, &regVal);
    regVal &= ~(1 << 3); // clear Crypto1 bit-3
    HFREADER_IO_write1Reg(RE31_CTRLREG, regVal);
}

/*!
 *  \fn      RE31_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
 *  \brief   Set BitFraming TxD, RxD by configure BitFraming register
 *  \param   RxAling
 *           TxLastBit
 *  \return  None
 */
void RE31_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
{
    uint8_t regVal = 0;

    regVal = (RxAlign << 4) | TxLastBit;
    HFREADER_IO_write1Reg(RE31_BITFRAMING, regVal);
}

/*!
 *  \fn      RE31_setCollMaskVal(uint8_t collEn)
 *  \brief   Set/Clear CollMaskVal Bit in Decoder Control register
 *  \param   collEn : Collision mask value enable
 *               - HFAL_ENABLE  : set
 *               - HFAL_DISABLE : clear
 *  \return  None
 */
void RE31_setCollMaskVal(uint8_t collEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_DECODERCTRL, &regVal);
    if(collEn)
    {
        regVal |= (1 << 6); // set bit-6
    }
    else
    {
        regVal &= ~(1 << 6); // clear bit-6
    }
    HFREADER_IO_write1Reg(RE31_DECODERCTRL, regVal);
}

/*!
 *  \fn      RE31_readCollPos(void)
 *  \brief   Read collision position register
 *  \param   None
 *  \return  Data on register
 */
uint8_t RE31_readCollPos(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE31_COLLPOS, &regVal);

    return regVal;
}

/*!
 *  \fn      RE31_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen)
 *  \brief   RE31 reader transmits RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  txCompleteFlag = 0;

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE31_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    RE31_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);

                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < dataTxLen; i++)
                    {
                        HFREADER_IO_write1Reg(RE31_FIFODATA, *(dataTx + i));
                    }
                    RE31_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE31_CMDREG, TRANSCEIVE_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE31_clearState();
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
            RE31_clearState();
        }
    } while(!(txCompleteFlag));

    HFREADER_IO_timStop();

    return status;
}

/*!
 *  \fn      RE31_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen)
 *  \brief   RE31 reader receives RF data
 *  \param   dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  rxCompleteFlag = 0;

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
            HFREADER_IO_write1Reg(RE31_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE31_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RE31_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RE31_FIFODATA, (dataRx + i));
                    }
                    *(dataRxLen) = FIFOLen;
                    RE31_disableIRQ(ALL_IRQ);
                    rxCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE31_clearState();
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
            RE31_clearState();
            *(dataRxLen) = 0;
        }
    } while(!(rxCompleteFlag));

    HFREADER_IO_timStop();

    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RE31_checkRFErr();
    }

    return status;
}


/*!
 *  \fn      RE31_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
 *                                  uint8_t *dataRx, uint16_t *dataRxLen)
 *  \brief   RE31 reader transmits and receives RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen)
{
    hfalStatus_t status = HFAL_ERROR;

    // disable all IRQ
    RE31_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE31_flushFIFO();
    // set RE31 internal timer
    HFREADER_IO_write1Reg(RE31_TIMERCONTROL, (RE31_TSTART_TXEND | RE31_TSTOP_RXBEGIN));

    // Transmit
    status = RE31_transmitCommand(dataTx, dataTxLen);
    if(status == HFAL_SUCCESS)
    {
        // Receive
        status = RE31_receiveCommand(dataRx, dataRxLen);
    }
    
    return status;
}

/*!
 *  \fn      RE31_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
 *                                   uint8_t *dataRx, uint16_t *dataRxLen,
 *                                   uint8_t txCRCEn, uint8_t rxCRCEn,
 *                                   uint8_t timeOut)
 *  \brief   RE31 reader transparent command with/without CRC
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen  : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen  : Length of data receiving
 *           txCRCEn  : Enable/Disable Tx CRC
 *           rxCRCEn  : Enable/Disable Rx CRC
 *           timeOut: RE31 reader's timeout by using RA12 timeout definition
 *                       RE31_TIMEOUT_DEFAULT: Use current timer value
 *                       RE31_TIMEOUT_1MS   : Timeout 1 ms
 *                       RE31_TIMEOUT_2MS   : TImeout 2 ms
 *                       RE31_TIMEOUT_4MS   : Timeout 4 ms
 *                       RE31_TIMEOUT_8MS   : Timeout 8 ms
 *                       RE31_TIMEOUT_16MS  : Timeout 16 ms
 *                       RE31_TIMEOUT_32MS  : Timeout 32 ms
 *                       RE31_TIMEOUT_64MS  : Timeout 64 ms
 *                       RE31_TIMEOUT_128MS : Timeout 128 ms
 *                       RE31_TIMEOUT_256MS : Timeout 256 ms
 *                       RE31_TIMEOUT_512MS : Timeout 512 ms
 *                       RE31_TIMEOUT_1SEC  : Timeout 1 sec
 *                       RE31_TIMEOUT_2SEC  : Timeout 2 sec
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                     uint8_t *dataRx, uint16_t *dataRxLen,
                                     uint8_t txCRCEn, uint8_t rxCRCEn,
                                     uint8_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t timerPrescaler;
    uint8_t timerReload;
    uint8_t tmpPrescaler;
    uint8_t tmpReload = 0x6A;

    // back up TPreScaler and TReloadValue
    HFREADER_IO_read1Reg(RE31_TIMERCLOCK, &timerPrescaler);
    HFREADER_IO_read1Reg(RE31_TIMERRELOAD, &timerReload);

    if(timeOut !=  RE31_TIMEOUT_DEFAULT)
    {
        // read register timer clock
        HFREADER_IO_read1Reg(RE31_TIMERCLOCK, &tmpPrescaler);
        // change TPreScaler (Bit4:0)
        tmpPrescaler = (tmpPrescaler & 0xE0) | (timeOut & 0x1F);
        // set TPrescaler and TReload
        HFREADER_IO_write1Reg(RE31_TIMERCLOCK, tmpPrescaler);
        HFREADER_IO_write1Reg(RE31_TIMERRELOAD, tmpReload);
    }
    else
    {
        HFREADER_IO_write1Reg(RE31_TIMERCLOCK,  timerPrescaler);
        HFREADER_IO_write1Reg(RE31_TIMERRELOAD, timerReload);
    }

    // set TX and RX CRC
    RE31_crcSetting(txCRCEn, rxCRCEn);
    // execute
    status = RE31_transceiveCommand(dataTx, dataTxLen, dataRx, dataRxLen);

    // load back the timer value
    HFREADER_IO_write1Reg(RE31_TIMERCLOCK,  timerPrescaler);
    HFREADER_IO_write1Reg(RE31_TIMERRELOAD, timerReload);

    return status;
}

/*!
 *  \fn      RE31_loadKey(uint8_t *key)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   key : key to load for tag MIFARE (12 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_loadKey(uint8_t *key)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  txCompleteFlag = 0;
    uint8_t  regVal;
    uint8_t  i;

    // disable all IRQ
    RE31_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE31_flushFIFO();
    // not use RA12 internal timer
    HFREADER_IO_write1Reg(RE31_TIMERCONTROL, 0x00);

    // write load key command
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE31_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RE31_disableIRQ(ALL_IRQ);
                    RE31_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 12; i++)
                    {
                        HFREADER_IO_write1Reg(RE31_FIFODATA, *(key + i));
                    }
                    RE31_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE31_CMDREG, LOADKEY_FIFO_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE31_clearState();
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
            RE31_clearState();
        }
    } while(!(txCompleteFlag));
    
    HFREADER_IO_timStop();
    
    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RE31_checkRFErr();
    }
    
    return status;
}

/*!
 *  \fn      RE31_authenticate(uint8_t *dataAuthent)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   dataAuthent: Input data to be authenticated (6 bytes)
 *                        Select key(1 byte) + Block number(1 bytes) + UID (4 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_authenticate(uint8_t *dataAuthent)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  respData[64] = {0};
    uint8_t  regVal;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  txCompleteFlag = 0;
    uint8_t  rxCompleteFlag = 0;

    // disable all IRQ
    RE31_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE31_flushFIFO();
    // set RA12 internal Timer
    HFREADER_IO_write1Reg(RE31_TIMERCONTROL, (RE31_TSTART_TXEND | RE31_TSTOP_RXBEGIN));

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE31_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RE31_disableIRQ(ALL_IRQ);
                    RE31_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 6; i++)
                    {
                        HFREADER_IO_write1Reg(RE31_FIFODATA, *(dataAuthent + i));
                    }
                    RE31_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE31_CMDREG, AUTHENT_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE31_clearState();
                    txCompleteFlag = 1;
                    status = HFAL_READER_TIMEOUT;
                }
                else
                {
                    // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            txCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE31_clearState();
        }
    }while(!txCompleteFlag);

    HFREADER_IO_timStop();
    if(status != HFAL_SUCCESS)
    {
        return status;
    }

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
    HFREADER_IO_write1Reg(RE31_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE31_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE31_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||
                   ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RE31_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RE31_FIFODATA, &respData[i]);
                    }
                    //RE31_disableIRQ(ALL_IRQ);
                    RE31_disableIRQ(HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ);
                    rxCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE31_clearState();
                    rxCompleteFlag = 1;
                    status = HFAL_NO_RESPONSE;
                }
                else
                {
                    // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            rxCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE31_clearState();
        }
    }while(!rxCompleteFlag);

    HFREADER_IO_timStop();

    if(status == HFAL_SUCCESS)
    {
        status = RE31_checkRFErr();
    }

    // check Crypto1 hardware
    if(status != HFAL_SUCCESS)
    {
        return status;
    }
    else
    {
        HFREADER_IO_read1Reg(RE31_CTRLREG, &regVal);
        if((regVal & CYRPTO1_MOn) == CYRPTO1_MOn)
        {
            status = HFAL_SUCCESS;
        }
        else
        {
            status = HFAL_ERROR;
        }
    }

    return status;
}

/*!
 *  \fn      RE31_checkRFErr(void)
 *  \brief   Check error status in error flag register
 *  \param   None
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE31_checkRFErr(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE31_ERRFLAG, &regVal);
    if((regVal & FRAMING_ERR_MASK) == FRAMING_ERR_MASK)
    {
        status = HFAL_RF_FRAMING_ERR;
    }
    else if((regVal & COLL_ERR_MASK) == COLL_ERR_MASK)
    {
        status = HFAL_RF_COLLISION_ERR;
    }
    else if((regVal & PARITY_ERR_MASK) == PARITY_ERR_MASK)
    {
        status = HFAL_RF_PARITY_ERR;
    }
    else if((regVal & CRC_ERR_MASK) == CRC_ERR_MASK)
    {
        status = HFAL_RF_CRC_ERR;
    }
    else if((regVal & FIFO_OVERFLOW_MASK) == FIFO_OVERFLOW_MASK)
    {
        status = HFAL_RF_BUFFER_OVERFLOW_ERR;
    }
    else
    {
        status = HFAL_SUCCESS;
    }

    return status;
}

//----------- (9) END OF FILE ------------------------------------------------//
