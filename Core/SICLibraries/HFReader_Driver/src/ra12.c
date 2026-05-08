//! \file:  ra12.c
//! \brief: This file provides basic code for using RA12 reader chip.

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
// 1) PD-FM-51-DTS-RA12-R1.2-20191219
//
// NOTE: -
//
//----------------------------------------------------------------------------//

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "ra12.h"

//----------- (2) GLOBAL VARIABLES -------------------------------------------//
HFREADER_DrvTypeDef ra12Drv_hfReader =
{
    RA12_flushFIFO,
    RA12_clearState,
    RA12_crcSetting,
    RA12_send1PulseBit,
    RA12_clearCryptoBit,
    RA12_setBitFraming,
    RA12_setCollMaskVal,
    RA12_readCollPos,
    RA12_transmitCommand,
    RA12_receiveCommand,
    RA12_transceiveCommand,
    RA12_loadKey,
    RA12_authenticate,
    RA12_utilDelay,
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
static ra12_configParam_t config_iso14443aParam_106Kbps =
{
    0x5B, // TxControl
    0x3F, // TxCfgCW
    0x10, // TxCfgMod
    0x19, // CoderControl
    0x11, // ModWidth
    0x11, // ModWidthSOF
    0x3B, // TypeBFraming

    0x6B, // RxControl1
    0x28, // DecoderControl
    0x3D, // BitPhase
    0x6C, // RxThreshold
    0x02, // BPSKDemControl
    0x41, // RxControl2
    0xC3, // RxControl3

    0x04, // RxWait
    0x03, // ChannelRedundancy
    0x63, // CRCPresetMSB
    0x63, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0A, // TimerClock
    0x06, // TimerControl
    0xFF, // TimerReloadValue
    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x00, // CDControl
    0x28, // Rx43AOption
    0x02  // GainST3
};

// default configuration for ISO14443B protocol with data rate 106 kbps
static ra12_configParam_t config_iso14443bParam_106Kbps =
{
    0x4B, // TxControl
    0x3F, // TxCfgCW
    0x10, // TxCfgMod
    0x20, // CoderControl
    0x3F, // ModWidth
    0x3F, // ModWidthSOF
    0x0D, // TypeBFraming

    0x6B, // RxControl1
    0x19, // DecoderControl
    0x3D, // BitPhase
    0x6C, // RxThreshold
    0x02, // BPSKDemControl
    0x41, // RxControl2
    0xC3, // RxControl3

    0x03, // RxWait
    0x2E, // ChannelRedundancy
    0xFF, // CRCPresetMSB
    0xFF, // CRCPresetLSB

    0x10, // FIFOLevel
    0x0A, // TimerClock
    0x06, // TimerControl
    0xFF, // TimerReloadValue
    0x19, // ManualFilter
    0x45, // FilterAdjust

    0x00, // CDControl
    0x28, // Rx43AOption
    0x02  // GainST3
};

// default configuration for ISO15693 protocol with Tx: 1 out of 4, Rx: 1 Sub Hi
static ra12_configParam_t config_iso15693Param_1Out4_1SubHi =
{
    0x5B, // TxControl
    0x3F, // TxCfgCW
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

    0x00, // CDControl
    0x80, // Rx43AOption
    0x12  // GainST3
};

//----------- (7) PRIVATE FUNCTION PROTOTYPES --------------------------------//
// N/A

//----------- (8) PRIVATE FUNCTIONS ------------------------------------------//
/*!
 *  \fn      RA12_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
 *  \brief   Set RA12 HF-Reader IO Initialization.
 *  \param   irqEn : Enable/Disable IRQ pin
 *  \param   rstpdEn : Enable/Disable RSTPD pin
 *  \param   hfDriver : HF reader driver interface
 *  \return  None
 */
void RA12_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver)
{
    g_irqEn   = irqEn;
    g_rstpdEn = rstpdEn;
    *hfDriver = &ra12Drv_hfReader;

    HFREADER_IO_init(g_irqEn, g_rstpdEn);
}

/*!
 *  \fn      RA12_checkIRQ(uint8_t irqEn)
 *  \brief   Check IRQ of RA12 reader by reading IRQ pin or IRQ flag register.
 *  \param   irqEn : IRQ pin configuration from RA12_init function
 *  \return  1 : There is an IRQ is set
 *           0 : There is no any IRQ is set
 */
uint8_t RA12_checkIRQ(uint8_t irqEn)
{
    uint8_t  regVal1 = 0;
    uint8_t  regVal2 = 0;
    
    if(irqEn)
    {
        return HFREADER_IO_readIrq();
    }
    else
    {
        HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal1);
        HFREADER_IO_read1Reg(RA12_IRQEN, &regVal2);

        return (regVal1 & regVal2 & 0x7F);
    }
}

/*!
 *  \fn      RA12_utilDelay(uint32_t ms)
 *  \brief   Delay time in milli-seconds
 *  \param   ms : Timeout value in milli-seconds
 *  \return  None
 */
void RA12_utilDelay(uint32_t ms)
{
    HFREADER_IO_delay(ms);
}

/*!
 *  \fn      RA12_analyze(void)
 *  \brief   Analyze RA12 reader interface by reading/writing FIFO register
 *  \param   None
 *  \return  HFAL_SUCCESS : the operation success
 *           otherwise    : the operation fail
 */
hfalStatus_t RA12_analyze(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t rdData[2] = {0};
    uint8_t wrData = 0xAA;

    // clear state of RA12 reader to idle state
    RA12_clearState();

    // flush FIFO and check FIFO length
    RA12_flushFIFO();
    HFREADER_IO_read1Reg(RA12_FIFOLEN, &rdData[0]);
    if(rdData[0] == 0)
    {
        // write FIFO then read FIFO length and read FIFO
        HFREADER_IO_write1Reg(RA12_FIFODATA, wrData);
        HFREADER_IO_read1Reg(RA12_FIFOLEN, &rdData[0]);
        HFREADER_IO_read1Reg(RA12_FIFODATA, &rdData[1]);
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
        if(RA12_getFlagIRQ(LOALERT_IRQ) == LOALERT_IRQ)
        {
            // flush FIFO and return SUCCESS
            RA12_flushFIFO();
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
 *  \fn      RA12_getRevision(void)
 *  \brief   Get RA12 revision
 *  \param   None
 *  \return  RA12 device revision (RevA, RevB or Unknown)
 */
ra12_deviceRev_t RA12_getRevision(void)
{
    uint8_t regVal = 0;
    ra12_deviceRev_t ra12Rev = 0;

    // select sector 1
    HFREADER_IO_write1Reg(RA12_SECTOR, 0x01);
    HFREADER_IO_read1Reg(RA12_REV, &regVal);
    // check RA12 version
    switch(regVal)
    {
        case 0x10:
            ra12Rev = RA12_REVA;
            break;
        case 0x20:
            ra12Rev = RA12_REVB;
            break;
        default:
            ra12Rev = RA12_REV_UNKNOWN;
            break;
    }
    // select sector 0
    HFREADER_IO_write1Reg(RA12_SECTOR, 0x00);

    return ra12Rev;
}

/*!
 *  \fn      RA12_configRFParam(ra12_configParam_t *param, ra12_configProtocol_t protocol)
 *  \brief   Assign default parameter value by the specified protocol
 *  \param   param : Pointer points to RA12 configure parameter structure
 *           protocol : RF protocol specification
 *  \return  None
 */
static void RA12_configRFParam(ra12_configParam_t *param, ra12_configProtocol_t protocol)
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

        param->CDControl         = config_iso14443aParam_106Kbps.CDControl;
        param->Rx43AOption       = config_iso14443aParam_106Kbps.Rx43AOption;
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

        param->CDControl         = config_iso14443bParam_106Kbps.CDControl;
        param->Rx43AOption       = config_iso14443bParam_106Kbps.Rx43AOption;
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

        param->CDControl         = config_iso15693Param_1Out4_1SubHi.CDControl;
        param->Rx43AOption       = config_iso15693Param_1Out4_1SubHi.Rx43AOption;
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

        param->CDControl         = config_iso14443aParam_106Kbps.CDControl;
        param->Rx43AOption       = config_iso14443aParam_106Kbps.Rx43AOption;
        param->GainST3           = config_iso14443aParam_106Kbps.GainST3;
    }
}

/*!
 *  \fn      RA12_configuration(ra12_configProtocol_t configProtocol)
 *  \brief   Configure register of RA12 follow the specified protocol
 *  \param   configProtocol : RF protocol structure
 *  \return  HFAL_SUCCESS : The reader configuration success
 *           otherwise : The reader configuration failed
 */
hfalStatus_t RA12_configuration(ra12_configProtocol_t configProtocol)
{
    ra12_configParam_t configParam;
    ra12_deviceRev_t deviceRev;

    // get RA12 revision
    deviceRev = RA12_getRevision();

    if(deviceRev != RA12_REV_UNKNOWN)
    {
        // assign register value
        RA12_configRFParam(&configParam, configProtocol);


        // select sector 0
        HFREADER_IO_write1Reg(RA12_SECTOR, 0x00);// Select Page

        // register on sector 0
        HFREADER_IO_write1Reg(RA12_TXCTRL,        configParam.TxControl);
        HFREADER_IO_write1Reg(RA12_TXCFGPWR,      configParam.TxCfgCW);
        HFREADER_IO_write1Reg(RA12_TXCFGMOD,      configParam.TxCfgMod);
        HFREADER_IO_write1Reg(RA12_CODERCTRL,     configParam.CoderControl);
        HFREADER_IO_write1Reg(RA12_MODWIDTH,      configParam.ModWidth);
        HFREADER_IO_write1Reg(RA12_MODWIDTHSOF,   configParam.ModWidthSOF);
        HFREADER_IO_write1Reg(RA12_TYPEBFRAMING,  configParam.TypeBFraming);

        HFREADER_IO_write1Reg(RA12_RXCTRL1,       configParam.RxControl1);
        HFREADER_IO_write1Reg(RA12_DECODERCTRL,   configParam.DecoderControl);
        HFREADER_IO_write1Reg(RA12_BITPHASE,      configParam.BitPhase);
        HFREADER_IO_write1Reg(RA12_RXTHRESHOLD,   configParam.RxThreshold);
        HFREADER_IO_write1Reg(RA12_BPSKDEMOD,     configParam.BPSKDemControl);
        HFREADER_IO_write1Reg(RA12_RXCTRL2,       configParam.RxControl2);
        HFREADER_IO_write1Reg(RA12_RXCTRL3,       configParam.RxControl3);

        HFREADER_IO_write1Reg(RA12_RXWAIT,        configParam.RxWait);
        HFREADER_IO_write1Reg(RA12_CRCSETTING,    configParam.ChannelRedundancy);
        HFREADER_IO_write1Reg(RA12_CRCPRESETLSB,  configParam.CRCPresetMSB);
        HFREADER_IO_write1Reg(RA12_CRCPRESETMSB,  configParam.CRCPresetLSB);

        HFREADER_IO_write1Reg(RA12_FIFOLEVEL,     configParam.FIFOLevel);
        HFREADER_IO_write1Reg(RA12_TIMERCLOCK,    configParam.TimerClock);
        HFREADER_IO_write1Reg(RA12_TIMERCONTROL,  configParam.TimerControl);
        HFREADER_IO_write1Reg(RA12_TIMERRELOAD,   configParam.TimerReloadValue);

        // select sector 1
        HFREADER_IO_write1Reg(RA12_SECTOR, 0x01);
        if(deviceRev == RA12_REVA)
        {
            HFREADER_IO_write1Reg(RA12_ADC_FULLSCALE_ADJ, 0x00);
        }
        else if(deviceRev == RA12_REVB)
        {
            HFREADER_IO_write1Reg(RA12_ADC_FULLSCALE_ADJ, 0x06);
        }
        HFREADER_IO_write1Reg(RA12_MANFILTERCTRL,  configParam.ManualFilter);
        HFREADER_IO_write1Reg(RA12_FILTER_CORCOEF, configParam.FilterAdjust);

        // select sector 0
        HFREADER_IO_write1Reg(RA12_SECTOR, 0x00);
        HFREADER_IO_write1Reg(RA12_CDCTRL,        configParam.CDControl);
        HFREADER_IO_write1Reg(RA12_RX43A_OPTION,  configParam.Rx43AOption);
        HFREADER_IO_write1Reg(RA12_GAIN_ST3,      configParam.GainST3);

        return HFAL_SUCCESS;
    }
    else
    {
        return HFAL_ERROR;
    }
}

/*!
 *  \fn      RA12_enableIRQ(uint8_t irqEnable)
 *  \brief   Enable interrupt of RA12
 *  \param   irqEnable : Required interrupt to enable
 *  \return  None
 */
void RA12_enableIRQ(uint8_t irqEnable)
{
    uint8_t regVal;

    // set bit-7 to enable interrupt
    regVal = SETIRQ_EN | (irqEnable & ALL_IRQ);
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal);
}

/*!
 *  \fn      RA12_disableIRQ(uint8_t irqDisable)
 *  \brief   Disable interrupt and clear interrupt Flag
 *  \param   irqDisable : Required interrupt will be disabled and cleared
 *  \return  None
 */
void RA12_disableIRQ(uint8_t irqDisable)
{
    uint8_t regVal;

    regVal = irqDisable & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal); // disable interrupt
    HFREADER_IO_write1Reg(RA12_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RA12_clearFlagIRQ(uint8_t irqFlag)
 *  \brief   Clear interrupt flag of RA12
 *  \param   irqFlag : Required interrupt flag to set
 *  \return  None
 */
void RA12_clearFlagIRQ(uint8_t irqFlag)
{
    uint8_t regVal;

    regVal = irqFlag & 0x7F;  // force MSB to '0'
    HFREADER_IO_write1Reg(RA12_IRQFLAG, regVal); // clear interrupt flag
}

/*!
 *  \fn      RA12_getFlagIRQ(uint8_t getFlag)
 *  \brief   Get interrupt flag
 *  \param   getFlag : Specified interrupt will be checked
 *  \return  Interrupt requested flag if matched
 */
uint8_t RA12_getFlagIRQ(uint8_t getFlag)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);
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
 *  \fn      RA12_timerStart(void)
 *  \brief   Start RA12 reader's timer
 *  \param   None
 *  \return  None
 */
void RA12_timerStart(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_CTRLREG, &regVal); // read control register
    regVal |= TSTART_BIT; // set start timer bit (bit-1)
    HFREADER_IO_write1Reg(RA12_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RA12_timerStop(void)
 *  \brief   Stop RA12 reader's timer
 *  \param   None
 *  \return  None
 */
void RA12_timerStop(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_CTRLREG, &regVal); // read control register
    regVal |= TSTOP_BIT; // set stop timer bit (bit-2)
    HFREADER_IO_write1Reg(RA12_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RA12_timerAutoRestart(uint8_t restartEn)
 *  \brief   Auto-Restart RA12 reader's timer
 *  \param   restartEn : Configure auto restart timer bit
 *              - HFAL_ENABLE  : Enable  auto restart timer
 *              - HFAL_DISABLE : Disable auto restart timer
 *  \return  None
 */
void RA12_timerAutoRestart(uint8_t restartEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_TIMERCLOCK, &regVal);
    if(restartEn == HFAL_ENABLE)
    {
        regVal |= SET_TAUTORESTART;
    }
    else
    {
        regVal &= CLR_TAUTORESTART;
    }
    HFREADER_IO_write1Reg(RA12_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RA12_timerConfigure(uint8_t prescale, uint8_t reload)
 *  \brief   Configure RA12's timer period
 *  \param   prescale : Timer's prescale value (LSB 5 bits)
 *           reload   : Timer's reload value
 *  \return  None
 */
void RA12_timerConfigure(uint8_t prescale, uint8_t reload)
{
    uint8_t regVal;

    // configure timer reload value
    HFREADER_IO_write1Reg(RA12_TIMERRELOAD, reload);

    // configure timer prescaler value
    HFREADER_IO_read1Reg(RA12_TIMERCLOCK, &regVal);
    regVal = (uint8_t)((regVal & 0xE0) | (prescale & 0x1F));
    HFREADER_IO_write1Reg(RA12_TIMERCLOCK, regVal);
}

/*!
 *  \fn      RA12_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
 *  \brief   Get RE41's timer configuration
 *  \param   prescale : Pointer to a buffer to stores timer's prescale value
 *           reload   : Pointer to a buffer to stores timer's reload value
 *  \return  None
 */
void RA12_getTimerConfigure(uint8_t *prescale, uint8_t *reload)
{
    uint8_t regVal;

    // get timer reload
    HFREADER_IO_read1Reg(RA12_TIMERRELOAD, &regVal);
    *prescale = regVal;

    // get timer prescaler
    HFREADER_IO_read1Reg(RA12_TIMERCLOCK, &regVal);
    regVal &= 0x1F;
    *reload = regVal;
}

/*!
 *  \fn      ra12_timerWkupPrescaler(uint8_t prescale)
 *  \brief   Configure prescale of wake up timer
 *           Equation : Twkup = (WkTReloadValue * (2^WkTPrescaler))/(16.38 KHz)
 *           or       : Twkup = (m * (2^n)) / (16.38 KHz))
 *  \param   prescale : Prescaler of wake up timer
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_timerWkupPrescaler(uint8_t prescale)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  nVal;
    uint8_t  regVal;

    // mask last 4 bits of data
    nVal = prescale & 0x0F;
    // clear last 4 bits of wake up timer control register
    HFREADER_IO_read1Reg(RA12_WKTIMERCONTROL, &regVal);
    regVal = regVal & (~WKUP_TIMER_PRESCALER);
    // write prescaler value to wake up timer control register
    regVal |= nVal;
    HFREADER_IO_write1Reg(RA12_WKTIMERCONTROL, regVal);
    // read back to check
    HFREADER_IO_read1Reg(RA12_WKTIMERCONTROL, &regVal);
    regVal = regVal & WKUP_TIMER_PRESCALER;
    if(regVal == nVal)
    {
        status = HFAL_SUCCESS;
    }
    else
    {
        status = HFAL_ERROR;
    }

    return status;
}


/*!
 *  \fn      ra12_timerWkupReload(uint8_t reload)
 *  \brief   Configure reload value of wake up timer (1 period)
 *           Equation : Twkup = (WkTReloadValue * (2^WkTPrescaler)) / (16.38 KHz)
 *           or       : Twkup = (m * (2^n)) / (16.38 KHz))
 *  \param   reload : Reload value of wake up timer
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_timerWkupReload(uint8_t reload)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal;
    uint8_t  mVal;

    // reload value must not be zero
    if(reload != 0)
    {
        // store reload value to mVal
        mVal = reload;
        // write wake up timer reload
        regVal = reload;
        HFREADER_IO_write1Reg(RA12_WKTIMERRELOAD, regVal);
        // read back to check
        HFREADER_IO_read1Reg(RA12_WKTIMERRELOAD, &regVal);
        if(regVal == mVal)
        {
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
 *  \fn      ra12_timerWkupTrigTime(uint8_t trigTime)
 *  \brief   Define number of TWkUp under flow counter to generate timer
 *           interrupt flag.
 *           Equation : TwkIrq = (2^WkTrigTime) * TwkUp
 *           Example  : If TwkUp is set to 1 second when WkTrigTime = 3,
 *                      TimerIrq will be set every 8 second.
 *  \param   trigTime : Timer wake up under flow counter value.
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_timerWkupTrigTime(uint8_t trigTime)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal;
    uint8_t  WkTrigTime;

    // counter value must not be zero
    if(trigTime != 0)
    {
        // store trig time value to under flow counter
        WkTrigTime = trigTime;
        // write wake up timer under flow counter
        regVal = trigTime & WKUP_TIMER_TRIGTIME;
        HFREADER_IO_write1Reg(RA12_WKTRIGTIME, regVal);
        // read back to check
        HFREADER_IO_read1Reg(RA12_WKTRIGTIME, &regVal);
        regVal &= WKUP_TIMER_TRIGTIME;
        if(regVal == WkTrigTime)
        {
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
 *  \fn      ra12_timerWkupConfig(ra12_wkupTimer_t wkupTimer)
 *  \brief   Configure wake up timer (trigTime, prescaler, reload)
 *  \param   wkupTimer : Structure of wake up timer.
 *  \return  None
 */
void RA12_timerWkupConfig(ra12_wkupTimer_t wkupTimer)
{
    RA12_timerWkupTrigTime(wkupTimer.TWKUP_TRIGTIME);
    RA12_timerWkupPrescaler(wkupTimer.TWKUP_PRESCALER);
    RA12_timerWkupReload(wkupTimer.TWKUP_RELOAD);
}

/*!
 *  \fn      ra12_timerWkupStart(void)
 *  \brief   Start wake up timer
 *  \param   None
 *  \return  None
 */
void RA12_timerWkupStart(void)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RA12_WKTIMERCONTROL, &regVal);
    regVal = regVal | (WKUP_TSTART_NOW | WKUP_TAUTO_RESTART);
    HFREADER_IO_write1Reg(RA12_WKTIMERCONTROL, regVal);
}

/*!
 *  \fn      ra12_timerWkupStop(void)
 *  \brief   Stop wake up timer
 *  \param   None
 *  \return  None
 */
void RA12_timerWkupStop(void)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RA12_WKTIMERCONTROL, &regVal);
    regVal = regVal | WKUP_TSTOP_NOW;
    HFREADER_IO_write1Reg(RA12_WKTIMERCONTROL, regVal);
}

/*!
 *  \fn      ra12_timerWkupGetStatus(void)
 *  \brief   Get status of wake up timer
 *  \param   None
 *  \return  None
 */
uint8_t RA12_timerWkupGetStatus(void)
{
    uint8_t regVal;
    uint8_t chkStatus;

    HFREADER_IO_read1Reg(RA12_WKTIMERCONTROL, &regVal);
    chkStatus = regVal & WKUP_TRUNNING;
    // check wake up timer is running or not
    if(chkStatus == WKUP_TRUNNING)
    {
        return WKUP_TIMER_RUNNING;
    }
    else
    {
        return WKUP_TIMER_STOP;
    }
}

/*!
 *  \fn      RA12_calInternalADC(void)
 *  \brief   Calibrate internal ADC before using.
 *  \param   None
 *  \return  HFAL_SUCCESS : If the internal calibration complete.
 *           Other if the internal calibration failed.
 */
hfalStatus_t RA12_calInternalADC(void)
{
    uint8_t  irqFlag;
    uint8_t  cmd;

    // write ADC calibration command
    cmd = ADC_CAL_CMD;
    HFREADER_IO_write1Reg(RA12_CMDREG, cmd);

    // waiting for idle IRQ
    HFREADER_IO_timStart(500);//Timeout 500 ms
    do
    {
        irqFlag = RA12_getFlagIRQ(IDLE_IRQ);
    }
    while((irqFlag != IDLE_IRQ) || (!HFREADER_IO_timIsExpired()));

    // disable and Clear all IRQ
    RA12_disableIRQ(ALL_IRQ);

    //Check flag IRQ
    if(irqFlag == IDLE_IRQ)
    {
        return HFAL_SUCCESS;
    }
    else
    {
        return HFAL_ERROR;
    }
}


/*!
 *  \fn      RA12_getADC(uint8_t *adcI, uint8_t *adcQ)
 *  \brief   Get ADC result (both I and Q phase)
 *  \param   adcI : ADC result in I-Phase
 *           adcQ : ADC result in Q Phase
 *  \return  None
 */
void RA12_getADC(uint8_t *adcI, uint8_t *adcQ)
{
    uint8_t regVal;

    HFREADER_IO_read1Reg(RA12_ADCRESULT_I, &regVal);
    *adcI = regVal;

    HFREADER_IO_read1Reg(RA12_ADCRESULT_Q, &regVal);
    *adcQ = regVal;
}

/*!
 *  \fn      RA12_wkupCDInit(ra12_wkupCDProperties_t wkupCDProp)
 *  \brief   Wake up card detection init function on RA12
 *  \param   wkupCDProp : Structure of wake up card detection
 *  \return  None
 */
void RA12_wkupCDInit(ra12_wkupCDProperties_t wkupCDProp)
{
    uint8_t wkupCDCtrl = 0x00;
    uint8_t regVal;

    // store wake up card detection properties
    wkupCDCtrl = (wkupCDProp.CD_GoActive & WKUP_CD_GOACTIVE) |
                 (wkupCDProp.FD_Ignore & WKUP_FD_IGNORE) |
                 (wkupCDProp.FD_Enable & WKUP_FD_EN);
    // read wake up CD control register
    HFREADER_IO_read1Reg(RA12_CDCTRL, &regVal);
    regVal &= WKUP_CD_CLEAR_PROPERTY;
    // write wake up CD properties
    regVal |= wkupCDCtrl;
    HFREADER_IO_write1Reg(RA12_CDCTRL, regVal);
}

/*!
 *  \fn      RA12_CDInit(uint8_t CDTxDelay, uint8_t CDaverage)
 *  \brief   Card detection control init function on RA12.
 *  \param   CDTxDelay : Define ADC delay time after Tx is enable.
 *                       CD_64_OUT_OF_FREQ : 4.72 us delay time.
 *                       CD_32_OUT_OF_FREQ : 2.36 us delay time.
 *           CDaverage : Define the number of averaging time in each
 *                       conversion (0 - 3).
 *           Equation is : Number of averaging = 2^CDAvg
 *  \return  None
 */
void RA12_CDInit(uint8_t CDTxDelay, uint8_t CDaverage)
{
    uint8_t regVal;

    // mask bit for CD average
    CDaverage &= CD_AVG;
    // read CD control register
    HFREADER_IO_read1Reg(RA12_CDCTRL, &regVal);
    // check ADC delay time
    if(CDTxDelay == CD_64_OUT_OF_FREQ)
    {
        regVal |= CD_TX_DELAY;
    }
    else
    {
        regVal &= (~CD_TX_DELAY);
    }
    // clear and set CD average
    regVal &= (~CD_AVG);
    regVal |= CDaverage;
    // write to CD control register
    HFREADER_IO_write1Reg(RA12_CDCTRL, regVal);
}

/*!
 *  \fn      RA12_CDThresholdConfig(ra12_CDThreshold_t CDThreshold)
 *  \brief   CD threshold configuration for CD mode
 *  \param   CDThreshold : Structure of CD threshold in I and Q phase.
 *  \return  None
 */
void RA12_CDThresholdConfig(ra12_CDThreshold_t CDThreshold)
{
    uint8_t threshold_I_Low;
    uint8_t threshold_I_High;
    uint8_t threshold_Q_Low;
    uint8_t threshold_Q_High;

    threshold_I_Low  = CDThreshold.CD_Threshold_I_L;
    threshold_I_High = CDThreshold.CD_Threshold_I_H;
    threshold_Q_Low  = CDThreshold.CD_Threshold_Q_L;
    threshold_Q_High = CDThreshold.CD_Threshold_Q_H;

    HFREADER_IO_write1Reg(RA12_CDTHRESHOLD_I_L, threshold_I_Low);
    HFREADER_IO_write1Reg(RA12_CDTHRESHOLD_I_H, threshold_I_High);
    HFREADER_IO_write1Reg(RA12_CDTHRESHOLD_Q_L, threshold_Q_Low);
    HFREADER_IO_write1Reg(RA12_CDTHRESHOLD_Q_H, threshold_Q_High);
}

/*!
 *  \fn      RA12_CDModeSelect(ra12_CDmode_t cdMode)
 *  \brief   CD mode selection on RA12
 *  \param   cdMode : Structure of CD mode selection (Idle, normal or exit).
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_CDModeSelect(ra12_CDmode_t cdMode)
{
    hfalStatus_t status = HFAL_ERROR;
    ra12_wkupCDProperties_t wkupCardDetection;
    uint8_t wkupCD;

    // RA12 still in CD mode after woke up
    if(cdMode == RA12_IDLE_CD_MODE)
    {
        // configure wake up card detection properties
        wkupCardDetection.CD_GoActive = ~WKUP_CD_GOACTIVE;
        wkupCardDetection.FD_Ignore   = WKUP_FD_IGNORE;
        wkupCardDetection.FD_Enable   = ~WKUP_FD_EN;
        RA12_wkupCDInit(wkupCardDetection);
    }
    // RA12 will enter to active mode after woke up in CD mode
    else
    {
        // configure wake up card detection properties
        wkupCardDetection.CD_GoActive = WKUP_CD_GOACTIVE;
        wkupCardDetection.FD_Ignore   = WKUP_FD_IGNORE;
        wkupCardDetection.FD_Enable   = ~WKUP_FD_EN;
        RA12_wkupCDInit(wkupCardDetection);
    }

    // enter to CD mode
    if(cdMode != RA12_EXIT_CD_MODE)
    {
        // Check wake up CD mode status
        HFREADER_IO_read1Reg(RA12_CTRLREG, &wkupCD);
        wkupCD &= WKUP_CD_MODE;

        // RA12 is already in wake up CD mode
        if(wkupCD == WKUP_CD_MODE)
        {
            status = HFAL_ERROR;
        }
        // enables wake up CD mode
        else
        {
            // start wake up timer
            RA12_timerWkupStart();

            // enter to CD mode
            HFREADER_IO_read1Reg(RA12_CTRLREG, &wkupCD);
            wkupCD |= WKUP_CD_MODE;
            HFREADER_IO_write1Reg(RA12_CTRLREG, wkupCD);

            // check wake up CD mode status
            HFREADER_IO_read1Reg(RA12_CTRLREG, &wkupCD);
            if((wkupCD & WKUP_CD_MODE) == WKUP_CD_MODE)
            {
                status = HFAL_SUCCESS;
            }
            else
            {
                status = HFAL_ERROR;
            }
        }
    }
    // exit from CD mode
    else
    {
        // stop wake up timer of wake up cd mode
        RA12_timerWkupStop();

        // clear all mode in RA12 system control register
        HFREADER_IO_read1Reg(RA12_CTRLREG, &wkupCD);
        wkupCD = wkupCD & CLEAR_ALL_MODE;
        HFREADER_IO_write1Reg(RA12_CTRLREG, wkupCD);
        
        // delay time 1 ms to wait XTAL on RA12 reader
        HFREADER_IO_delay(1);
        
        // check wake up CD mode status
        HFREADER_IO_read1Reg(RA12_CTRLREG, &wkupCD);
        if((wkupCD & WKUP_CD_MODE) == WKUP_CD_MODE)
        {
            status = HFAL_ERROR;
        }
        else
        {
            status = HFAL_SUCCESS;
        }
    }

    return status;
}

/*!
 *  \fn      RA12_CDPreCal(ra12_wkupTimer_t wkupTimer)
 *  \brief   Pre-Calibrate before using CD mode.
 *  \param   wkupTimer : Configure wake up timer period for calibration.
 *  \return  HFAL_SUCCESS : If step of pre-calibration complete.
 *           Other : If the steps of pre-calibration failed.
 */
hfalStatus_t RA12_CDPreCal(ra12_wkupTimer_t wkupTimer)
{
    hfalStatus_t status = HFAL_ERROR;
    ra12_CDThreshold_t cdThreshold;

    // RA12 ADC calibration
    status = RA12_calInternalADC();
    if(status == HFAL_SUCCESS)
    {
        // reverse CD threshold for CD IRQ always set
        cdThreshold.CD_Threshold_I_H = 0x00;
        cdThreshold.CD_Threshold_I_L = 0xFF;
        cdThreshold.CD_Threshold_Q_H = 0x00;
        cdThreshold.CD_Threshold_Q_L = 0xFF;
        RA12_CDThresholdConfig(cdThreshold);

        // configure wake up timer period
        RA12_timerWkupConfig(wkupTimer);

        // enable CDIRQ on RA12
        RA12_enableIRQ(CD_IRQ);
        // start wkup timer
        RA12_timerWkupStart();
    }
    else
    {
        return status;
    }

    // enter to CD mode
    status = RA12_CDModeSelect(RA12_IDLE_CD_MODE);

    return status;
}

/*!
 *  \fn      RA12_CDPostCal(ra12_wkupTimer_t wkupTimer,
                            ra12_CDThreshold_t cdThreshold)
 *  \brief   Post-Calibrate before using CD mode.
 *  \param   wkupTimer : Configure wake up timer period for cd mode.
 *           cdThreshold : Configure cd threshold for cd mode.
 *  \return  HFAL_SUCCESS : Exit from CD mode success.
 *           Other : RA12 isn't run in CD mode.
 */
hfalStatus_t RA12_CDPostCal(ra12_wkupTimer_t wkupTimer,
                                    ra12_CDThreshold_t cdThreshold)
{
    hfalStatus_t status = HFAL_ERROR;

    // disable and exit from CD mode
    RA12_disableIRQ(ALL_IRQ);
    // exit from CD mode
    status = RA12_CDModeSelect(RA12_EXIT_CD_MODE);
    // check exiting from CD mode status
    if(status == HFAL_SUCCESS)
    {
        // configure wake up timer
        RA12_timerWkupConfig(wkupTimer);
        // configure CD threshold
        RA12_CDThresholdConfig(cdThreshold);
    }

    return status;
}

/*!
 *  \fn      RA12_rfOperate(ra12_rfControl_t rfCtrl)
 *  \brief   Turn on/off RF Field
 *  \param   rfCtrl : Structure of RF control (on, off, Tx1En or Tx2En)
 *  \return  None
 */
void RA12_rfOperate(ra12_rfControl_t rfCtrl)
{
    uint8_t  rdVal;
    uint8_t  wrVal;

    switch(rfCtrl)
    {
        case RFON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RA12_TXCTRL, &rdVal);
            // set Only Bit TX2RFEn and TX1RFEn
            wrVal = rdVal | TX2RFEn_Mask | TX1RFEn_Mask;
            HFREADER_IO_write1Reg(RA12_TXCTRL, wrVal);
            break;
        case RFOFF:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RA12_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_Mask) & (~TX1RFEn_Mask);
            HFREADER_IO_write1Reg(RA12_TXCTRL, wrVal);
            break;
        case TX1ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RA12_TXCTRL, &rdVal);
            // set bit-TX1 and clear bit-TX2
            wrVal = (rdVal | TX1RFEn_Mask) & (~TX2RFEn_Mask);
            HFREADER_IO_write1Reg(RA12_TXCTRL, wrVal);
            break;
        case TX2ON:
            // read TXCTRL register
            HFREADER_IO_read1Reg(RA12_TXCTRL, &rdVal);
            // set bit-TX2 and clear bit-TX1
            wrVal = (rdVal | TX2RFEn_Mask) & (~TX1RFEn_Mask);
            HFREADER_IO_write1Reg(RA12_TXCTRL, wrVal);
            break;
        // off field
        default :
            // read TXCTRL register
            HFREADER_IO_read1Reg(RA12_TXCTRL, &rdVal);
            // clear only bit-TX2RFEn and TX1RFEn
            wrVal = rdVal & (~TX2RFEn_Mask) & (~TX1RFEn_Mask);
            HFREADER_IO_write1Reg(RA12_TXCTRL, wrVal);
            break;
    }
}

/*!
 *  \fn      RA12_flushFIFO(void)
 *  \brief   Flush RA12 reader's FIFO
 *  \param   None
 *  \return  None
 */
void RA12_flushFIFO(void)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RA12_CTRLREG, &regVal); // read control register
    regVal |= 0x01; // set Flush FIFO bit (bit-0)
    HFREADER_IO_write1Reg(RA12_CTRLREG, regVal); // write control register
}

/*!
 *  \fn      RA12_clearState(void)
 *  \brief   Clear state of RA12 reader
 *  \param   None
 *  \return  None
 */
void RA12_clearState(void)
{
    RA12_disableIRQ(ALL_IRQ); // disable All IRQ
    HFREADER_IO_write1Reg(RA12_CMDREG, IDLE_CMD); // write IDLE command to command register
    RA12_flushFIFO();
}

/*!
 *  \fn      RA12_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
 *  \brief   Set RF transmit/receive CRC
 *  \param   txCRCEn : Data transmission with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data transmission
 *               - HFAL_DISABLE : Disable CRC for data transmission
 *           rxCRCEn : Data receiving with CRC setting
 *               - HFAL_ENABLE  : Enable  CRC for data receiving
 *               - HFAL_DISABLE : Disable CRC for data receiving
 *  \return  None
 */
void RA12_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn)
{
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RA12_CRCSETTING, &regVal);
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
    HFREADER_IO_write1Reg(RA12_CRCSETTING, regVal);
}

/*!
 *  \fn      RA12_send1PulseBit(uint8_t onePulseBit)
 *  \brief   One pulse bit setting of the only one modulation to switch to the
 *           next time slot (ISO15693 protocol)
 *  \param   onePluseBit : One pulse bit setting
 *               - SET1PULSE_BIT : Send 1 pulse bit
 *               - CLR1PULSE_BIT : Doesn't send 1 pulse bit
 *  \return  None
 */
void RA12_send1PulseBit(uint8_t onePulseBit)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_CODERCTRL, &regVal);
    if((onePulseBit == SET1PULSE_BIT) || (onePulseBit == HFAL_SETBIT))
    {
        HFREADER_IO_write1Reg(RA12_CODERCTRL, (regVal | SET1PULSE_BIT));
    }
    else
    {
        HFREADER_IO_write1Reg(RA12_CODERCTRL, (regVal & CLR1PULSE_BIT));
    }
}

/*!
 *  \fn      RA12_clearCryptoBit(void)
 *  \brief   CryptoM engine bit is switched on to encrypted RF communication
 *           which is set only if the authentication process is successful.
 *  \param   None
 *  \return  None
 */
void RA12_clearCryptoBit(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_CTRLREG, &regVal);
    regVal &= ~(1 << 3); // clear Crypto1 bit-3
    HFREADER_IO_write1Reg(RA12_CTRLREG, regVal);
}

/*!
 *  \fn      RA12_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
 *  \brief   Set BitFraming TxD, RxD by configure BitFraming register
 *  \param   RxAling
 *           TxLastBit
 *  \return  None
 */
void RA12_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit)
{
    uint8_t regVal = 0;

    regVal = (RxAlign << 4) | TxLastBit;
    HFREADER_IO_write1Reg(RA12_BITFRAMING, regVal);
}

/*!
 *  \fn      RA12_setCollMaskVal(uint8_t collEn)
 *  \brief   Set/Clear CollMaskVal Bit in Decoder Control register
 *  \param   collEn :
 *               - HFAL_ENABLE  : set
 *               - HFAL_DISABLE : clear
 *  \return  None
 */
void RA12_setCollMaskVal(uint8_t collEn)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_DECODERCTRL, &regVal);
    if(collEn)
    {
        regVal |= (1 << 6); // set bit-6
    }
    else
    {
        regVal &= ~(1 << 6); // clear bit-6
    }
    HFREADER_IO_write1Reg(RA12_DECODERCTRL, regVal);
}

/*!
 *  \fn      RA12_readCollPos(void)
 *  \brief   Read collision position register
 *  \param   None
 *  \return  Data on register
 */
uint8_t RA12_readCollPos(void)
{
    uint8_t regVal = 0;

    HFREADER_IO_read1Reg(RA12_COLLPOS, &regVal);

    return regVal;
}

/*!
 *  \fn      RA12_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut)
 *  \brief   RA12 reader transmits RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t regVal = 0;
    uint8_t  i = 0;
    uint8_t  txCompleteFlag = 0;

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal);

    // external TIM counter
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RA12_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    RA12_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);

                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < dataTxLen; i++)
                    {
                        HFREADER_IO_write1Reg(RA12_FIFODATA, *(dataTx + i));
                    }
                    RA12_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RA12_CMDREG, TRANSCEIVE_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RA12_clearState();
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
            RA12_clearState();
        }
    } while(!(txCompleteFlag));

    HFREADER_IO_timStop();

    return status;
}

/*!
 *  \fn      RA12_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut)
 *  \brief   RA12 reader receives RF data
 *  \param   dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t regVal = 0;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  rxCompleteFlag = 0;

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
            HFREADER_IO_write1Reg(RA12_IRQEN, regVal);

    // external TIM counter
    HFREADER_IO_timStart(timeOut);
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RA12_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RA12_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RA12_FIFODATA, (dataRx + i));
                    }
                    *(dataRxLen) = FIFOLen;
                    RA12_disableIRQ(ALL_IRQ);
                    rxCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RA12_clearState();
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
            RA12_clearState();
            *(dataRxLen) = 0;
        }
    } while(!(rxCompleteFlag));

    HFREADER_IO_timStop();

    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RA12_checkRFErr();
    }

    return status;
}


/*!
 *  \fn      RA12_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
 *                                  uint8_t *dataRx, uint16_t *dataRxLen,
 *                                  uint16_t timeOut)
 *  \brief   RA12 reader transmits and receives RF data
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen : Length of data receiving
 *           timeOut : External timeout to prevent the reader stuck in unexpected loop
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen,
                                    uint16_t timeOut)
{
    hfalStatus_t status = HFAL_ERROR;

    // disable all IRQ
    RA12_disableIRQ(ALL_IRQ);
    // flush FIFO
    RA12_flushFIFO();
    // set RA12 internal timer
    HFREADER_IO_write1Reg(RA12_TIMERCONTROL, (RA12_TSTART_TXEND | RA12_TSTOP_RXBEGIN));

    // Transmit
    status = RA12_transmitCommand(dataTx, dataTxLen, timeOut);
    if(status == HFAL_SUCCESS)
    {
        // Receive
        status = RA12_receiveCommand(dataRx, dataRxLen, timeOut);
    }
    
    return status;
}

/*!
 *  \fn      RA12_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
 *                                   uint8_t *dataRx, uint16_t *dataRxLen,
 *                                   uint8_t txCRCEn, uint8_t rxCRCEn,
 *                                   uint8_t timeOut)
 *  \brief   RA12 reader transparent command with/without CRC
 *  \param   dataTx : Pointer points to data which will be transmitted
 *           dataTxLen  : Length of data transmission
 *           dataRx : Pointer points to the buffer which will be used to store
 *                    received data
 *           dataRxLen  : Length of data receiving
 *           txCRCEn  : Enable/Disable Tx CRC
 *           rxCRCEn  : Enable/Disable Rx CRC
 *           timeOut: RA12 reader's timeout by using RA12 timeout definition
 *                       RA12_TIMEOUT_DEFAULT: Use current timer value
 *                       RA12_TIMEOUT_1MS   : Timeout 1 ms
 *                       RA12_TIMEOUT_2MS   : TImeout 2 ms
 *                       RA12_TIMEOUT_4MS   : Timeout 4 ms
 *                       RA12_TIMEOUT_8MS   : Timeout 8 ms
 *                       RA12_TIMEOUT_16MS  : Timeout 16 ms
 *                       RA12_TIMEOUT_32MS  : Timeout 32 ms
 *                       RA12_TIMEOUT_64MS  : Timeout 64 ms
 *                       RA12_TIMEOUT_128MS : Timeout 128 ms
 *                       RA12_TIMEOUT_256MS : Timeout 256 ms
 *                       RA12_TIMEOUT_512MS : Timeout 512 ms
 *                       RA12_TIMEOUT_1SEC  : Timeout 1 sec
 *                       RA12_TIMEOUT_2SEC  : Timeout 2 sec
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
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
    HFREADER_IO_read1Reg(RA12_TIMERCLOCK, &timerPrescaler);
    HFREADER_IO_read1Reg(RA12_TIMERRELOAD, &timerReload);

    if(timeOut !=  RA12_TIMEOUT_DEFAULT)
    {
        // read register timer clock
        HFREADER_IO_read1Reg(RA12_TIMERCLOCK, &tmpPrescaler);
        // change TPreScaler (Bit4:0)
        tmpPrescaler = (tmpPrescaler & 0xE0) | (timeOut & 0x1F);
        // set TPrescaler and TReload
        HFREADER_IO_write1Reg(RA12_TIMERCLOCK, tmpPrescaler);
        HFREADER_IO_write1Reg(RA12_TIMERRELOAD, tmpReload);
    }
    else
    {
        HFREADER_IO_write1Reg(RA12_TIMERCLOCK,  timerPrescaler);
        HFREADER_IO_write1Reg(RA12_TIMERRELOAD, timerReload);
    }

    // set TX and RX CRC
    RA12_crcSetting(txCRCEn, rxCRCEn);
    // execute
    status = RA12_transceiveCommand(dataTx, dataTxLen, dataRx, dataRxLen, 2000);

    // load back the timer value
    HFREADER_IO_write1Reg(RA12_TIMERCLOCK,  timerPrescaler);
    HFREADER_IO_write1Reg(RA12_TIMERRELOAD, timerReload);

    return status;
}

/*!
 *  \fn      RA12_loadKey(uint8_t *key)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   key : key to load for tag MIFARE (12 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_loadKey(uint8_t *key)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  txCompleteFlag = 0;
    uint8_t  regVal;
    uint8_t  i;

    // disable all IRQ
    RA12_disableIRQ(ALL_IRQ);
    // flush FIFO
    RA12_flushFIFO();
    // not use RA12 internal timer
    HFREADER_IO_write1Reg(RA12_TIMERCONTROL, 0x00);

    // write load key command
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RA12_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) || ((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RA12_disableIRQ(ALL_IRQ);
                    RA12_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 12; i++)
                    {
                        HFREADER_IO_write1Reg(RA12_FIFODATA, *(key + i));
                    }
                    RA12_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RA12_CMDREG, LOADKEY_FIFO_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RA12_clearState();
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
            RA12_clearState();
        }
    } while(!(txCompleteFlag));
    
    HFREADER_IO_timStop();
    
    // check RF error
    if(status == HFAL_SUCCESS)
    {
        status = RA12_checkRFErr();
    }
    
    return status;
}

/*!
 *  \fn      RA12_authenticate(uint8_t *dataAuthent)
 *  \brief   RA12 reader load key 12 bytes to master key
 *  \param   dataAuthent: Input data to be authenticated (6 bytes)
 *                        Select key(1 byte) + Block number(1 bytes) + UID (4 bytes)
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_authenticate(uint8_t *dataAuthent)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  respData[64] = {0};
    uint8_t  regVal;
    uint8_t  i = 0;
    uint8_t  FIFOLen = 0;
    uint8_t  txCompleteFlag = 0;
    uint8_t  rxCompleteFlag = 0;

    // disable all IRQ
    RA12_disableIRQ(ALL_IRQ);
    // flush FIFO
    RA12_flushFIFO();
    // set RA12 internal Timer
    HFREADER_IO_write1Reg(RA12_TIMERCONTROL, (RA12_TSTART_TXEND | RA12_TSTOP_RXBEGIN));

    // transmit
    regVal = 0x80 | TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ;
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RA12_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);
                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||((regVal & TX_IRQ) == TX_IRQ))
                {
                    //RA12_disableIRQ(ALL_IRQ);
                    RA12_disableIRQ(TX_IRQ | LOALERT_IRQ | TIMER_IRQ | IDLE_IRQ);
                    txCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & LOALERT_IRQ) == LOALERT_IRQ)
                {
                    for(i = 0; i < 6; i++)
                    {
                        HFREADER_IO_write1Reg(RA12_FIFODATA, *(dataAuthent + i));
                    }
                    RA12_disableIRQ(LOALERT_IRQ);
                    HFREADER_IO_write1Reg(RA12_CMDREG, AUTHENT_CMD);
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RA12_clearState();
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
            RA12_clearState();
        }
    }while(!txCompleteFlag);

    HFREADER_IO_timStop();
    if(status != HFAL_SUCCESS)
    {
        return status;
    }

    // receive
    regVal = 0x80 | HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ;
    HFREADER_IO_write1Reg(RA12_IRQEN, regVal);

    HFREADER_IO_timStart(2000); // external TIM counter 2S
    do
    {
        if(!(HFREADER_IO_timIsExpired()))
        {
            if(RA12_checkIRQ(g_irqEn))
            {
                HFREADER_IO_read1Reg(RA12_IRQFLAG, &regVal);

                if(((regVal & IDLE_IRQ) == IDLE_IRQ) ||
                   ((regVal & RX_IRQ) == RX_IRQ))
                {
                    HFREADER_IO_read1Reg(RA12_FIFOLEN, &FIFOLen);
                    for(i = 0; i < FIFOLen; i++)
                    {
                        HFREADER_IO_read1Reg(RA12_FIFODATA, &respData[i]);
                    }
                    //RA12_disableIRQ(ALL_IRQ);
                    RA12_disableIRQ(HIALERT_IRQ | IDLE_IRQ | TIMER_IRQ | RX_IRQ);
                    rxCompleteFlag = 1;
                    status = HFAL_SUCCESS;
                }
                else if((regVal & TIMER_IRQ) == TIMER_IRQ)
                {
                    RA12_clearState();
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
            RA12_clearState();
        }
    }while(!rxCompleteFlag);

    HFREADER_IO_timStop();

    if(status == HFAL_SUCCESS)
    {
        status = RA12_checkRFErr();
    }

    // check Crypto1 hardware
    if(status != HFAL_SUCCESS)
    {
        return status;
    }
    else
    {
        HFREADER_IO_read1Reg(RA12_CTRLREG, &regVal);
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
 *  \fn      RA12_checkRFErr(void)
 *  \brief   Check error status in error flag register
 *  \param   None
 *  \return  HFAL_SUCCESS on success
 *           Otherwise an error code
 */
hfalStatus_t RA12_checkRFErr(void)
{
    hfalStatus_t status = HFAL_ERROR;
    uint8_t  regVal = 0;

    HFREADER_IO_read1Reg(RA12_ERRFLAG, &regVal);
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
