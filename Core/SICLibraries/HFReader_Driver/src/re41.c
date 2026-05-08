//! \file:  re41.c
//! \brief: This file provides basic code for using RE41 reader chip.

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
HFREADER_DrvTypeDef re41Drv_hfReader =
{
    RE41_flushFIFO,
    RE41_clearState,
    RE41_crcSetting,
    RE41_send1PulseBit,
    RE41_clearCryptoBit,
    RE41_setBitFraming,
    RE41_setCollMaskVal,
    RE41_readCollPos,
    RE41_transmitCommand,
    RE41_receiveCommand,
    RE41_transceiveCommand,
    RE41_loadKey,
    RE41_authenticate,
    RE41_utilDelay,
    // Extended function to support FELICA
    RE41Ext_transmitCommand,
    RE41Ext_receiveCommand,
    RE41Ext_transceiveCommand,
    RE41Ext_transceiveMultiSlots
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
static re41_configParam_t config_iso14443aParam_106Kbps =
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
static re41_configParam_t config_iso14443bParam_106Kbps =
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
static re41_configParam_t config_iso15693Param_1Out4_1SubHi =
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

// default configuration for FELICA protocol with data rate 212 kbps
static re41_configParam_t config_FELICAParam_212Kbps =
{
    0x4B, // TxControl
    0x3F, // TxCfgCW
    0x10, // TxCfgMod
    0x1A, // CoderControl
    0x0F, // ModWidth
    0x0F, // ModWidthSOF
    0x3B, // TypeBFraming

    0x1B, // RxControl1
    0x01, // DecoderControl
    0x20, // BitPhase
    0x4C, // RxThreshold
    0x02, // BPSKDemControl
    0x41, // RxControl2
    0xF0, // RxControl3

    0x12, // RxWait
    0x4C, // ChannelRedundancy
    0x00, // CRCPresetMSB
    0x00, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0C, // TimerClock
    0x06, // TimerControl
    0xFF, // TimerReloadValue

    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x09  // GainST3
};

//----------- (7) PRIVATE FUNCTION PROTOTYPES --------------------------------//
// N/A

//----------- (8) PRIVATE FUNCTIONS ------------------------------------------//
/*!
 *  \fn      RE41_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
 *  \brief   Set RE41 HF-Reader IO Initialization.
 *  \param   irqEn : Enable/Disable IRQ pin
 *           rstpdEn : Enable/Disable RSTPD pin
 *           hfDriver : HF reader driver interface
 *  \return  None
 */
void RE41_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
{
    g_irqEn   = irqEn;
    g_rstpdEn = rstpdEn;
    *hfDriver = &re41Drv_hfReader;

    HFREADER_IO_init(g_irqEn, g_rstpdEn);
}

/*!
 *  \fn      RE41_checkIRQ(uint8_t irqEn)
 *  \brief   Check IRQ of RE41 reader by reading IRQ pin or IRQ flag register.
 *  \param   irqEn : IRQ pin configuration from RE41_init function
 *  \return  1 : There is an IRQ is set
 *           0 : There is no any IRQ is set
 */
uint8_t RE41_checkIRQ(uint8_t irqEn)
{
    uint8_t  regVal1 = 0;
    uint8_t  regVal2 = 0;
    
    if(irqEn)
    {
        return HFREADER_IO_readIrq();
    }
    else
    {
        HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal1);
        HFREADER_IO_read1Reg(RE41_IRQEN, &regVal2);

        return (regVal1 & regVal2 & 0x7F);
    }
}

/*!
 *  \fn      RE41_utilDelay(uint32_t ms)
 *  \brief   Delay time in milli-seconds
 *  \param   ms : Timeout value in milli-seconds
 *  \return  None
 */
void RE41_utilDelay(uint32_t ms)
{
    HFREADER_IO_delay(ms);
}

/*!
 *  \fn      RE41_analyze(void)
 *  \brief   Analyze RE41 reader interface by reading/writing FIFO register
 *  \param   None
 *  \return  HFAL_SUCCESS : the operation success
 *           otherwise    : the operation fail
 */
hfalStatus_t RE41_analyze(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t rdData[2] = {0};
    uint8_t wrData = 0xAA;

    // clear state of RE41 reader to idle state
    RE41_clearState();

    // flush FIFO and check FIFO length
    RE41_flushFIFO();
    HFREADER_IO_read1Reg(RE41_FIFOLEN, &rdData[0]);
    if(rdData[0] == 0)
    {
        // write FIFO then read FIFO length and read FIFO
        HFREADER_IO_write1Reg(RE41_FIFODATA, wrData);
        HFREADER_IO_read1Reg(RE41_FIFOLEN, &rdData[0]);
        HFREADER_IO_read1Reg(RE41_FIFODATA, &rdData[1]);
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
        if(RE41_getFlagIRQ(LOALERT_IRQ) == LOALERT_IRQ)
        {
            // flush FIFO and return SUCCESS
            RE41_flushFIFO();
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
 *  \fn      RE41_getRevision(void)
 *  \brief   Get RE41 revision
 *  \param   None
 *  \return  RE41 device revision
 */
re41_deviceRev_t RE41_getRevision(void)
{
    uint8_t regVal = 0;
    re41_deviceRev_t re41Rev = 0;

    HFREADER_IO_read1Reg(RE41_DEVICETYPE, &regVal);
    // check RE41 version
    switch(regVal)
    {
        case 0x7E:
            re41Rev = RE41_CODE1;
            break;
        case 0xBF:
            re41Rev = RE41_CODE2;
            break;
        default:
            re41Rev = RE41_UNKNOWN;
            break;
    }

    return re41Rev;
}

/*!
 *  \fn      RE41_configRFParam(re41_configParam_t *param, re41_configProtocol_t protocol)
 *  \brief   Assign default parameter value by the specified protocol
 *  \param   param : Pointer points to RE41 configure parameter structure
 *           protocol : RF protocol specification
 *  \return  None
 */
static void RE41_configRFParam(re41_configParam_t *param, re41_configProtocol_t protocol)
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
    else if(protocol == FELICA_212KBPS)
    {
        param->TxControl         = config_FELICAParam_212Kbps.TxControl;
        param->TxCfgCW           = config_FELICAParam_212Kbps.TxCfgCW;
        param->TxCfgMod          = config_FELICAParam_212Kbps.TxCfgMod;
        param->CoderControl      = config_FELICAParam_212Kbps.CoderControl;
        param->ModWidth          = config_FELICAParam_212Kbps.ModWidth;
        param->ModWidthSOF       = config_FELICAParam_212Kbps.ModWidthSOF;
        param->TypeBFraming      = config_FELICAParam_212Kbps.TypeBFraming;

        param->RxControl1        = config_FELICAParam_212Kbps.RxControl1;
        param->DecoderControl    = config_FELICAParam_212Kbps.DecoderControl;
        param->BitPhase          = config_FELICAParam_212Kbps.BitPhase;
        param->RxThreshold       = config_FELICAParam_212Kbps.RxThreshold;
        param->BPSKDemControl    = config_FELICAParam_212Kbps.BPSKDemControl;
        param->RxControl2        = config_FELICAParam_212Kbps.RxControl2;
        param->RxControl3        = config_FELICAParam_212Kbps.RxControl3;

        param->RxWait            = config_FELICAParam_212Kbps.RxWait;
        param->ChannelRedundancy = config_FELICAParam_212Kbps.ChannelRedundancy;
        param->CRCPresetMSB      = config_FELICAParam_212Kbps.CRCPresetMSB;
        param->CRCPresetLSB      = config_FELICAParam_212Kbps.CRCPresetLSB;

        param->FIFOLevel         = config_FELICAParam_212Kbps.FIFOLevel;
        param->TimerClock        = config_FELICAParam_212Kbps.TimerClock;
        param->TimerControl      = config_FELICAParam_212Kbps.TimerControl;
        param->TimerReloadValue  = config_FELICAParam_212Kbps.TimerReloadValue;
        param->ManualFilter      = config_FELICAParam_212Kbps.ManualFilter;
        param->FilterAdjust      = config_FELICAParam_212Kbps.FilterAdjust;

        param->GainST3           = config_FELICAParam_212Kbps.GainST3;
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
 *  \fn      RE41_configuration(re41_configProtocol_t configProtocol)
 *  \brief   Configure register of RA12 follow the specified protocol
 *  \param   configProtocol : RF protocol structure
 *  \return  HFAL_SUCCESS : If the configuration success
 *           Otherwise    : If the device isn't RE41 reader IC
 */
hfalStatus_t RE41_configuration(re41_configProtocol_t configProtocol)
{
    re41_configParam_t configParam;
    re41_deviceRev_t deviceRev;

    // get RE41 revision
    deviceRev = RE41_getRevision();

    if(deviceRev != RE41_UNKNOWN)
    {
        // assign register value
        RE41_configRFParam(&configParam, configProtocol);

        HFREADER_IO_write1Reg(RE41_TXCTRL,        configParam.TxControl);
        HFREADER_IO_write1Reg(RE41_TXCFGPWR,      configParam.TxCfgCW);
        HFREADER_IO_write1Reg(RE41_TXCFGMOD,      configParam.TxCfgMod);
        HFREADER_IO_write1Reg(RE41_CODERCTRL,     configParam.CoderControl);
        HFREADER_IO_write1Reg(RE41_MODWIDTH,      configParam.ModWidth);
        HFREADER_IO_write1Reg(RE41_MODWIDTHSOF,   configParam.ModWidthSOF);
        HFREADER_IO_write1Reg(RE41_TYPEBFRAMING,  configParam.TypeBFraming);

        HFREADER_IO_write1Reg(RE41_RXCTRL1,       configParam.RxControl1);
        HFREADER_IO_write1Reg(RE41_DECODERCTRL,   configParam.DecoderControl);
        HFREADER_IO_write1Reg(RE41_BITPHASE,      configParam.BitPhase);
        HFREADER_IO_write1Reg(RE41_RXTHRESHOLD,   configParam.RxThreshold);
        HFREADER_IO_write1Reg(RE41_BPSKDEMOD,     configParam.BPSKDemControl);
        HFREADER_IO_write1Reg(RE41_RXCTRL2,       configParam.RxControl2);
        HFREADER_IO_write1Reg(RE41_RXCTRL3,       configParam.RxControl3);

        HFREADER_IO_write1Reg(RE41_RXWAIT,        configParam.RxWait);
        HFREADER_IO_write1Reg(RE41_CRCSETTING,    configParam.ChannelRedundancy);
        HFREADER_IO_write1Reg(RE41_CRCPRESETLSB,  configParam.CRCPresetMSB);
        HFREADER_IO_write1Reg(RE41_CRCPRESETMSB,  configParam.CRCPresetLSB);

        HFREADER_IO_write1Reg(RE41_FIFOLEVEL,     configParam.FIFOLevel);
        HFREADER_IO_write1Reg(RE41_TIMERCLOCK,    configParam.TimerClock);
        HFREADER_IO_write1Reg(RE41_TIMERCONTROL,  configParam.TimerControl);
        HFREADER_IO_write1Reg(RE41_TIMERRELOAD,   configParam.TimerReloadValue);
        HFREADER_IO_write1Reg(RE41_MANFILTERCTRL, configParam.ManualFilter);
        HFREADER_IO_write1Reg(RE41_FILTERADJUST,  configParam.FilterAdjust);

        HFREADER_IO_write1Reg(RE41_GAIN_ST3,      configParam.GainST3);

        return HFAL_SUCCESS;
    }
    else
    {
        return HFAL_ERROR;
    }
}

/*!
 *  \fn      RE41_enableIRQ(uint8_t irqEnable)
 *  \brief   Enable interrupt of RE41
 *  \param   irqEnable : Required interrupt to enable
 *  \return  None
 */
void RE41_enableIRQ(uint8_t irqEnable)
{
    uint8_t regVal;

    // set bit-7 to enable interrupt
    regVal = SETIRQ_EN | (irqEnable & ALL_IRQ);
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);
}

/*!
 *  \fn      RE41_disableIRQ(uint8_t irqDisable)
 *  \brief   Disable interrupt and clear interrupt Flag
 *  \param   irqDisable : Required interrupt will be disabled and cleared
 *  \return  None
 */
void RE41_disableIRQ(uint8_t irqDisable)
{
    uint8_t regVal;

    regVal = irqDisable & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal); // disable interrupt
    HFREADER_IO_write1Reg(RE41_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RE41_clearFlagIRQ(uint8_t irqFlag)
 *  \brief   Clear interrupt flag of RE41
 *  \param   irqFlag : Required interrupt flag to set
 *  \return  None
 */
void RE41_clearFlagIRQ(uint8_t irqFlag)
{
    uint8_t regVal;

    regVal = irqFlag & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RE41_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RE41_getFlagIRQ(uint8_t getFlag)
 *  \brief   Get interrupt flag
 *  \param   getFlag : Specified interrupt will be checked
 *  \return  Interrupt requested flag if matched
 */
uint8_t RE41_getFlagIRQ(uint8_t getFlag)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
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
 *  \fn      RE41_timerStart(void)
 *  \brief   Start RE41 reader's timer
 *  \param   None
 *  \return  None
 */
void RE41_timerStart(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_CTRLREG, &regVal); // read control register
    regVal |= TSTART_BIT; // set start timer bit (bit-1)
    HFREADER_IO_write1Reg(RE41_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE41_timerStop(void)
 *  \brief   Stop RE41 reader's timer
 *  \param   None
 *  \return  None
 */
void RE41_timerStop(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_CTRLREG, &regVal); // read control register
    regVal |= TSTOP_BIT; // set stop timer bit (bit-2)
    HFREADER_IO_write1Reg(RE41_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE41_timerAutoRestart(uint8_t restartEn)
 *  \brief   Auto-Restart RE41 reader's timer
 *  \param   restartEn : Configure auto restart timer bit
 *              - HFAL_ENABLE  : Enable  auto restart timer
 *              - HFAL_DISABLE : Disable auto restart timer
 *  \return  None
 */
void RE41_timerAutoRestart(uint8_t restartEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_TIMERCLOCK, &regVal);
    if(restartEn == HFAL_ENABLE)
    {
        regVal |= SET_TAUTORESTART;
    }
    else
    {
        regVal &= CLR_TAUTORESTART;
    }
    HFREADER_IO_write1Reg(RE41_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RE41_timerConfigure(uint8_t prescale, uint8_t reload)
 *  \brief   Configure RE41's timer period
 *  \param   prescale : Timer's prescale value (LSB 5 bits)
 *           reload   : Timer's reload value
 *  \return  None
 */
void RE41_timerConfigure(uint8_t prescale, uint8_t reload)
{
    uint8_t regVal;

    // configure timer reload value
    HFREADER_IO_write1Reg(RE41_TIMERRELOAD, reload);

    // configure timer prescaler value
    HFREADER_IO_read1Reg(RE41_TIMERCLOCK, &regVal);
    regVal = (uint8_t)((regVal & 0xE0) | (prescale & 0x1F));
    HFREADER_IO_write1Reg(RE41_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RE41_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
 *  \brief   Get RE41's timer configuration
 *  \param   prescale : Pointer to a buffer to stores timer's prescale value
 *           reload   : Pointer to a buffer to stores timer's reload value
 *  \return  None
 */
void RE41_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
{
    uint8_t regVal;

    // get timer reload
    HFREADER_IO_read1Reg(RE41_TIMERRELOAD, &regVal);
    *prescale = regVal;

    // get timer prescaler
    HFREADER_IO_read1Reg(RE41_TIMERCLOCK, &regVal);
    regVal &= 0x1F;
    *reload = regVal;
}

/*!
 *  \fn      RE41_flushFIFO(void)
 *  \brief   Flush RE41 reader's FIFO
 *  \param   None
 *  \return  None
 */
void RE41_flushFIFO(void)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE41_CTRLREG, &regVal); // read control register
    regVal |= FLUSHFIFO_BIT; // set Flush FIFO bit (bit-0)
    HFREADER_IO_write1Reg(RE41_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RE41_clearState(void)
 *  \brief   Clear state of RE41 reader
 *  \param   None
 *  \return  None
 */
void RE41_clearState(void)
{
    RE41_disableIRQ(ALL_IRQ); // disable All IRQ
    HFREADER_IO_write1Reg(RE41_CMDREG, IDLE_CMD); // write IDLE command to command register
    RE41_flushFIFO();
}

/*!
 *  \fn      RE41_rfOperate(re41_rfControl_t rfCtrl)
 *  \brief   Turn on/off RF Field
 *  \param   rfCtrl : Structure of RF control (on, off, Tx1En or Tx2En)
 *  \return  None
 */
void RE41_rfOperate(re41_rfControl_t rfCtrl)
{
    uint8_t  rdVal;
    uint8_t  wrVal;

    switch(rfCtrl)
    {
        case RFON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE41_TXCTRL, &rdVal);
            // set Only Bit TX2RFEn and TX1RFEn
            wrVal = rdVal | TX2RFEn_MASK | TX1RFEn_MASK;
            HFREADER_IO_write1Reg(RE41_TXCTRL, wrVal);
            break;
        case RFOFF:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE41_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE41_TXCTRL, wrVal);
            break;
        case TX1ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE41_TXCTRL, &rdVal);
            // set bit-TX1 and clear bit-TX2
            wrVal = (rdVal | TX1RFEn_MASK) & (~TX2RFEn_MASK);
            HFREADER_IO_write1Reg(RE41_TXCTRL, wrVal);
            break;
        case TX2ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE41_TXCTRL, &rdVal);
            // set bit-TX2 and clear bit-TX1
            wrVal = (rdVal | TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE41_TXCTRL, wrVal);
            break;
        // off field
        default :
            // read TXCTRL register
            HFREADER_IO_read1Reg(RE41_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_MASK) & (~TX1RFEn_MASK);
            HFREADER_IO_write1Reg(RE41_TXCTRL, wrVal);
            break;
    }
}

/*!
 *  \fn      RE41_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
 *  \brief   Set RF transmit/receive CRC
 *  \param   txCRCEn : Data transmission with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data transmission
 *               - HFAL_DISABLE : Disable CRC for data transmission
 *           rxCRCEn : Data receiving with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data receiving
 *               - HFAL_DISABLE : Disable CRC for data receiving
 *  \return  None
 */
void RE41_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE41_CRCSETTING, &regVal);
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
    HFREADER_IO_write1Reg(RE41_CRCSETTING, regVal);
}

/*!
 *  \fn      RE41_send1PulseBit(uint8_t onePulseBit)
 *  \brief   One pulse bit setting of the only one modulation to switch to the
 *           next time slot (ISO15693 protocol)
 *  \param   onePluseBit : One pulse bit setting
 *               - SET1PULSE_BIT : Send 1 pulse bit
 *               - CLR1PULSE_BIT : Doesn't send 1 pulse bit
 *  \return  None
 */
void RE41_send1PulseBit(uint8_t onePulseBit)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_CODERCTRL, &regVal);
    if((onePulseBit == SET1PULSE_BIT) || (onePulseBit == HFAL_SETBIT))
    {
        HFREADER_IO_write1Reg(RE41_CODERCTRL, (regVal | SET1PULSE_BIT));
    }
    else
    {
        HFREADER_IO_write1Reg(RE41_CODERCTRL, (regVal & CLR1PULSE_BIT));
    }
}

/*!
 *  \fn      RE41_clearCryptoBit(void)
 *  \brief   CryptoM engine bit is switched on to encrypted RF communication
 *           which is set only if the authentication process is successful.
 *  \param   None
 *  \return  None
 */
void RE41_clearCryptoBit(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_CTRLREG, &regVal);
    regVal &= ~(1 << 3); // clear Crypto1 bit-3
    HFREADER_IO_write1Reg(RE41_CTRLREG, regVal);
}

/*!
 *  \fn      RE41_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
 *  \brief   Set BitFraming TxD, RxD by configure BitFraming register
 *  \param   RxAling
 *           TxLastBit
 *  \return  None
 */
void RE41_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
{
    uint8_t regVal = 0;

    regVal = (RxAlign << 4) | TxLastBit;
    HFREADER_IO_write1Reg(RE41_BITFRAMING, regVal);
}

/*!
 *  \fn      RE41_setCollMaskVal(uint8_t collEn)
 *  \brief   Set/Clear CollMaskVal Bit in Decoder Control register
 *  \param   collEn : Collision mask value enable
 *               - HFAL_ENABLE  : set
 *               - HFAL_DISABLE : clear
 *  \return  None
 */
void RE41_setCollMaskVal(uint8_t collEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_DECODERCTRL, &regVal);
    if(collEn)
    {
        regVal |= (1 << 6); // set bit-6
    }
    else
    {
        regVal &= ~(1 << 6); // clear bit-6
    }
    HFREADER_IO_write1Reg(RE41_DECODERCTRL, regVal);
}

/*!
 *  \fn      RE41_readCollPos(void)
 *  \brief   Read collision position register
 *  \param   None
 *  \return  Data on register
 */
uint8_t RE41_readCollPos(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RE41_COLLPOS, &regVal);

    return regVal;
}

/*!
 *  \fn      RE41_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut)
 *  \brief   RE41 reader transmits RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  txCompleteFlag = 0;

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    // external TIM counter
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    RE41_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);

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

    HFREADER_IO_timStop();

    return status;
}

/*!
 *  \fn      RE41_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut)
 *  \brief   RE41 reader receives RF data
 *  \param   dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  rxCompleteFlag = 0;

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
            HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    // external TIM counter
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
 *  \fn      RE41_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen,
                                    uint16_t timeOut)
 *  \brief   RE41 reader transmits and receives RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
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
    status = RE41_transmitCommand(dataTx, dataTxLen, timeOut);
    if(status == HFAL_SUCCESS)
    {
        // Receive
        status = RE41_receiveCommand(dataRx, dataRxLen, timeOut);
    }
    
    return status;
}

/*!
 *  \fn      RE41_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
 *                                   uint8_t *dataRx, uint16_t *dataRxLen,
 *                                   uint8_t txCRCEn, uint8_t rxCRCEn,
 *                                   uint8_t timeOut)
 *  \brief   RE41 reader transparent command with/without CRC
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen  : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen  : Length of data receiving
 *           txCRCEn  : Enable/Disable Tx CRC
 *           rxCRCEn  : Enable/Disable Rx CRC
 *           timeOut: RE41 reader's timeout by using RA12 timeout definition
 *                       RE41_TIMEOUT_DEFAULT: Use current timer value
 *                       RE41_TIMEOUT_1MS   : Timeout 1 ms
 *                       RE41_TIMEOUT_2MS   : TImeout 2 ms
 *                       RE41_TIMEOUT_4MS   : Timeout 4 ms
 *                       RE41_TIMEOUT_8MS   : Timeout 8 ms
 *                       RE41_TIMEOUT_16MS  : Timeout 16 ms
 *                       RE41_TIMEOUT_32MS  : Timeout 32 ms
 *                       RE41_TIMEOUT_64MS  : Timeout 64 ms
 *                       RE41_TIMEOUT_128MS : Timeout 128 ms
 *                       RE41_TIMEOUT_256MS : Timeout 256 ms
 *                       RE41_TIMEOUT_512MS : Timeout 512 ms
 *                       RE41_TIMEOUT_1SEC  : Timeout 1 sec
 *                       RE41_TIMEOUT_2SEC  : Timeout 2 sec
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
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
    HFREADER_IO_read1Reg(RE41_TIMERCLOCK, &timerPrescaler);
    HFREADER_IO_read1Reg(RE41_TIMERRELOAD, &timerReload);

    if(timeOut !=  RE41_TIMEOUT_DEFAULT)
    {
        // read register timer clock
        HFREADER_IO_read1Reg(RE41_TIMERCLOCK, &tmpPrescaler);
        // change TPreScaler (Bit4:0)
        tmpPrescaler = (tmpPrescaler & 0xE0) | (timeOut & 0x1F);
        // set TPrescaler and TReload
        HFREADER_IO_write1Reg(RE41_TIMERCLOCK, tmpPrescaler);
        HFREADER_IO_write1Reg(RE41_TIMERRELOAD, tmpReload);
    }
    else
    {
        HFREADER_IO_write1Reg(RE41_TIMERCLOCK,  timerPrescaler);
        HFREADER_IO_write1Reg(RE41_TIMERRELOAD, timerReload);
    }

    // set TX and RX CRC
    RE41_crcSetting(txCRCEn, rxCRCEn);
    // execute
    status = RE41_transceiveCommand(dataTx, dataTxLen, dataRx, dataRxLen, 2000);

    // load back the timer value
    HFREADER_IO_write1Reg(RE41_TIMERCLOCK,  timerPrescaler);
    HFREADER_IO_write1Reg(RE41_TIMERRELOAD, timerReload);

    return status;
}

/*!
 *  \fn      RE41_loadKey(uint8_t *key)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   key : key to load for tag MIFARE (12 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_loadKey(uint8_t *key)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  txCompleteFlag = 0;
    uint8_t  regVal;
    uint8_t  i;

    // disable all IRQ
    RE41_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE41_flushFIFO();
    // not use RA12 internal timer
    HFREADER_IO_write1Reg(RE41_TIMERCONTROL, 0x00);

    // write load key command
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RE41_disableIRQ(ALL_IRQ);
                    RE41_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 12; i++)
                    {
                        HFREADER_IO_write1Reg(RE41_FIFODATA, *(key + i));
                    }
                    RE41_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE41_CMDREG, LOADKEY_FIFO_CMD);
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
    
    HFREADER_IO_timStop();
    
    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RE41_checkRFErr();
    }
    
    return status;
}

/*!
 *  \fn      RE41_authenticate(uint8_t *dataAuthent)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   dataAuthent: Input data to be authenticated (6 bytes)
 *                        Select key(1 byte) + Block number(1 bytes) + UID (4 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_authenticate(uint8_t *dataAuthent)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  respData[64] = {0};
    uint8_t  regVal;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  txCompleteFlag = 0;
    uint8_t  rxCompleteFlag = 0;

    // disable all IRQ
    RE41_disableIRQ(ALL_IRQ);
    // flush FIFO
    RE41_flushFIFO();
    // set RA12 internal Timer
    HFREADER_IO_write1Reg(RE41_TIMERCONTROL, (RE41_TSTART_TXEND | RE41_TSTOP_RXBEGIN));

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RE41_disableIRQ(ALL_IRQ);
                    RE41_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 6; i++)
                    {
                        HFREADER_IO_write1Reg(RE41_FIFODATA, *(dataAuthent + i));
                    }
                    RE41_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RE41_CMDREG, AUTHENT_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RE41_clearState();
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
            RE41_clearState();
        }
    }while(!txCompleteFlag);

    HFREADER_IO_timStop();
    if(status != HFAL_SUCCESS)
    {
        return status;
    }

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
    HFREADER_IO_write1Reg(RE41_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RE41_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RE41_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||
                   ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RE41_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RE41_FIFODATA, &respData[i]);
                    }
                    //RE41_disableIRQ(ALL_IRQ);
                    RE41_disableIRQ(HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ);
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
                    // do nothing
                }
            }
        }
        else
        {
            // external TIM is expired
            rxCompleteFlag = 1;
            status = HFAL_ASIC_EXE_TIMEOUT;
            RE41_clearState();
        }
    }while(!rxCompleteFlag);

    HFREADER_IO_timStop();

    if(status == HFAL_SUCCESS)
    {
        status = RE41_checkRFErr();
    }

    // check Crypto1 hardware
    if(status != HFAL_SUCCESS)
    {
        return status;
    }
    else
    {
        HFREADER_IO_read1Reg(RE41_CTRLREG, &regVal);
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
 *  \fn      RE41_checkRFErr(void)
 *  \brief   Check error status in error flag register
 *  \param   None
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RE41_checkRFErr(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RE41_ERRFLAG, &regVal);
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
