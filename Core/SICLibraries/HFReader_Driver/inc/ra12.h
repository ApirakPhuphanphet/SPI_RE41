//! \file:  ra12.h
//! \brief: This file provides basic code for using RA12 reader chip.

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION: -
//
// AUTHOR:  1) WORRAKAMOL BOONPARIPHAN,
//          2) APIRAK RATSAMEESAWANG
// CONTACT: 1) worrakamol@sic.co.th
//          2) apirak@sic.co.th
//
// UPDATE: 2020-09-18
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RA12-R1.2-20191219
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef RA12_H
#define RA12_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// ra12_configParam_t is a default register structure on RA12 reader to be set
// to support various RF protocols communication
typedef struct
{
    // Tx session
    uint8_t  TxControl;         // address: 0x11
    uint8_t  TxCfgCW;           // address: 0x12
    uint8_t  TxCfgMod;          // address: 0x13
    uint8_t  CoderControl;      // address: 0x14
    uint8_t  ModWidth;          // address: 0x15
    uint8_t  ModWidthSOF;       // address: 0x16
    uint8_t  TypeBFraming;      // address: 0x17

    // Rx session
    uint8_t  RxControl1;        // address: 0x19
    uint8_t  DecoderControl;    // address: 0x1A
    uint8_t  BitPhase;          // address: 0x1B
    uint8_t  RxThreshold;       // address: 0x1C
    uint8_t  BPSKDemControl;    // address: 0x1D
    uint8_t  RxControl2;        // address: 0x1E
    uint8_t  RxControl3;        // address: 0x1F

    // RF Timing, Prog timer and Rx adjust
    uint8_t  RxWait;            // address: 0x21
    uint8_t  ChannelRedundancy; // address: 0x22
    uint8_t  CRCPresetMSB;      // address: 0x23
    uint8_t  CRCPresetLSB;      // address: 0x24
    uint8_t  FIFOLevel;         // address: 0x29

    uint8_t  TimerClock;        // address: 0x2A
    uint8_t  TimerControl;      // address: 0x2B
    uint8_t  TimerReloadValue;  // address: 0x2C
    uint8_t  ManualFilter;      // address: 0x2E
    uint8_t  FilterAdjust;      // address: 0x2F

    uint8_t  CDControl;         // address: 0x31
    uint8_t  Rx43AOption;       // address: 0x3C
    uint8_t  GainST3;           // address: 0x3F
} ra12_configParam_t;

// Standard RF protocols communication support by RA12 reader are defined here
typedef enum
{
    ISO14443A_106KBPS = 0x00u,
    ISO14443B_106KBPS,
    ISO15693_1OUT4_1SUB_HI
} ra12_configProtocol_t;

// The supported device revision
typedef enum
{
    RA12_REVA           = 0x10,
    RA12_REVB           = 0x20,
    RA12_REV_UNKNOWN    = 0x00
} ra12_deviceRev_t;

// Structure of wake up timer
typedef struct
{
    uint8_t TWKUP_RELOAD;
    uint8_t TWKUP_PRESCALER;
    uint8_t TWKUP_TRIGTIME;
} ra12_wkupTimer_t;

// Structure of card detection properties
typedef struct
{
    uint8_t CD_GoActive;
    uint8_t FD_Ignore;
    uint8_t FD_Enable;
} ra12_wkupCDProperties_t;

// Structure of card detection threshold
typedef struct
{
    uint8_t CD_Threshold_I_H;
    uint8_t CD_Threshold_I_L;
    uint8_t CD_Threshold_Q_H;
    uint8_t CD_Threshold_Q_L;
} ra12_CDThreshold_t;

// Card detection mode running selection group
typedef enum
{
    RA12_NORMAL_CD_MODE = 0x00u,
    RA12_IDLE_CD_MODE,
    RA12_EXIT_CD_MODE
} ra12_CDmode_t;

// RF control group definition
typedef enum
{
    RFON = 0x00u,
    RFOFF,
    TX1ON,
    TX2ON
} ra12_rfControl_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// RA12 register's address definition
// sector selection
#define RA12_SECTOR                 0x00

// register address in sector 0
#define RA12_CMDREG                 0x01
#define RA12_FIFODATA               0x02
#define RA12_PRIMSTATUS             0x03
#define RA12_FIFOLEN                0x04
#define RA12_SECONDSTATUS           0x05
#define RA12_IRQEN                  0x06
#define RA12_IRQFLAG                0x07

#define RA12_CTRLREG                0x09
#define RA12_ERRFLAG                0x0A
#define RA12_COLLPOS                0x0B
#define RA12_TIMVALREG              0x0C
#define RA12_CRCRESULTLSB           0x0D
#define RA12_CRCRESULTMSB           0x0E
#define RA12_BITFRAMING             0x0F

#define RA12_TXCTRL                 0x11
#define RA12_TXCFGPWR               0x12
#define RA12_TXCFGMOD               0x13
#define RA12_CODERCTRL              0x14
#define RA12_MODWIDTH               0x15
#define RA12_MODWIDTHSOF            0x16
#define RA12_TYPEBFRAMING           0x17

#define RA12_RXCTRL1                0x19
#define RA12_DECODERCTRL            0x1A
#define RA12_BITPHASE               0x1B
#define RA12_RXTHRESHOLD            0x1C
#define RA12_BPSKDEMOD              0x1D
#define RA12_RXCTRL2                0x1E
#define RA12_RXCTRL3                0x1F

#define RA12_RXWAIT                 0x21
#define RA12_CRCSETTING             0x22
#define RA12_CRCPRESETLSB           0x23
#define RA12_CRCPRESETMSB           0x24
#define RA12_ADCCTRL                0x25
#define RA12_ADCRESULT_I            0x26
#define RA12_ADCRESULT_Q            0x27

#define RA12_FIFOLEVEL              0x29
#define RA12_TIMERCLOCK             0x2A
#define RA12_TIMERCONTROL           0x2B
#define RA12_TIMERRELOAD            0x2C
#define RA12_WKTIMERCONTROL         0x2D
#define RA12_WKTIMERRELOAD          0x2E
#define RA12_WKTRIGTIME             0x2F

#define RA12_FDCTRL                 0x30
#define RA12_CDCTRL                 0x31
#define RA12_FDTHRESHOLD_I_H        0x32
#define RA12_FDTHRESHOLD_Q_H        0x33
#define RA12_CDTHRESHOLD_I_L        0x34
#define RA12_CDTHRESHOLD_I_H        0x35
#define RA12_CDTHRESHOLD_Q_L        0x36
#define RA12_CDTHRESHOLD_Q_H        0x37

#define RA12_TDIRQCTRL              0x39
#define RA12_TESTANASEL             0x3A
#define RA12_RX43A_OPTION           0x3C
#define RA12_SP_CRYPTO              0x3E
#define RA12_GAIN_ST3               0x3F

// register address in sector 1
#define RA12_LFOTRIMRESULT          0x01
#define RA12_M_LFOTrim_VAL          0x02
#define RA12_LFOTRIMSEL             0x03
#define RA12_FD_CD_IRQCFG           0x04
#define RA12_ADC_FULLSCALE_ADJ      0x05

#define RA12_MASKSET                0x0D
#define RA12_PARAM                  0x0E
#define RA12_REV                    0x0F

#define RA12_TXFALLINGCTRL          0x10
#define RA12_TXRISINGCTRL           0x11
#define RA12_CFGCWFALL              0x12
#define RA12_CFGCWRISE              0x13

#define RA12_MANFILTERCTRL          0x2E
#define RA12_FILTER_CORCOEF         0x2F

#define RA12_SIGNAL_INDICATOR       0x37

// RA12 command for writing to address 0x01
#define STARTUP_CMD                 0x30
#define IDLE_CMD                    0x00
#define TX_CMD                      0x1A
#define RX_CMD                      0x16
#define TRANSCEIVE_CMD              0x1E
#define CALCRC_CMD                  0x12
#define LOADKEY_FIFO_CMD            0x19
#define AUTHENT_CMD                 0x1C
#define RXFILTER_TUNE_CMD           0x10
#define LFOTUNE_CMD                 0x20
#define ADC_CAL_CMD                 0x21
#define CARD_DETECT_CMD             0x22
#define FIELD_DETECT_CMD            0x23
#define SIGNATURE_CMD               0x31

// mask bit of interrupt reg (ADDR 0x06, 0x07)
#define SETIRQ_FLAG                 0x80
#define SETIRQ_EN                   0x80
#define ALL_IRQ                     0x7F
#define CD_IRQ                      0x40
#define TIMER_IRQ                   0x20
#define TX_IRQ                      0x10
#define RX_IRQ                      0x08
#define IDLE_IRQ                    0x04
#define HIALERT_IRQ                 0x02
#define LOALERT_IRQ                 0x01

// mask bit of system control (ADDR 0x09)
#define WKUP_CD_MODE                0x40
#define STANDBY_MODE                0x20
#define PWR_DOWN_MODE               0x10
#define CYRPTO1_MOn                 0x08
#define CLEAR_ALL_MODE              0x8F
#define CHECK_ALL_MODE              0x70
#define IDLE_CD_MODE                0x41
#define FLUSHFIFO_BIT               0x01
#define TSTART_BIT                  0x02
#define TSTOP_BIT                   0x04

// mask bit of error register (ADDR 0x0A)
#define KEY_ERR                     0x40
#define FIFO_OVERFLOW_MASK          0x10
#define CRC_ERR_MASK                0x08
#define FRAMING_ERR_MASK            0x04
#define PARITY_ERR_MASK             0x02
#define COLL_ERR_MASK               0x01

// mask bit of TxControl reg (ADDR 0x11)
#define TX1INV_Mask                 0x80
#define MODSRC_Mask                 0x60
#define FORCE100ASK_Mask            0x10
#define TX2INV_Mask                 0x08
#define TX2CW_Mask                  0x04
#define TX2RFEn_Mask                0x02
#define TX1RFEn_Mask                0x01

// set or clear one pulse bit (ADDR 0x14)
#define SET1PULSE_BIT               0x80
#define CLR1PULSE_BIT               0x7F

// set or clear RxMultiple bit (ADDR 0x1A)
#define SET_RXMULTIPLE              0x80
#define CLR_RXMULTIPLE              0x7F

// set or clear timer auto restart bit (ADDR 0x2A)
#define SET_TAUTORESTART            0x20
#define CLR_TAUTORESTART            0xDF

// mask bit of RA12 TIMER control (ADDR 0x2B)
#define RA12_TSTOP_RXEND            0x08
#define RA12_TSTOP_RXBEGIN          0x04
#define RA12_TSTART_TXEND           0x02
#define RA12_TSTART_TXBEGIN         0x01

// mask bit of WKUP TIMER control (ADDR 0x2D)
#define WKUP_TSTART_NOW             0x80
#define WKUP_TSTOP_NOW              0x40
#define WKUP_TRUNNING               0x20
#define WKUP_TAUTO_RESTART          0x10
#define WKUP_TIMER_PRESCALER        0x0F

// check WKUP Timer running status
#define WKUP_TIMER_RUNNING          0x21
#define WKUP_TIMER_STOP             0x22

// mask bit of WKUP TIME trig time (ADDR 0x2F)
#define WKUP_TIMER_TRIGTIME         0x0F

// mask bit of Wake up CD control (ADDR 0x31)
#define WKUP_CD_GOACTIVE            0x40
#define WKUP_FD_IGNORE              0x20
#define WKUP_FD_EN                  0x10
#define CD_TX_DELAY                 0x04
#define CD_AVG                      0x03
#define WKUP_CD_CLEAR_PROPERTY      0x8F
#define CD_32_OUT_OF_FREQ           0x14 // 2.36 us delay time (default)
#define CD_64_OUT_OF_FREQ           0x24 // 4.72 us delay time

// RA12 program timer for transparent command
#define RA12_TIMEOUT_DEFAULT        0x00
#define RA12_TIMEOUT_1MS            0x07
#define RA12_TIMEOUT_2MS            0x08
#define RA12_TIMEOUT_4MS            0x09
#define RA12_TIMEOUT_8MS            0x0A
#define RA12_TIMEOUT_16MS           0x0B
#define RA12_TIMEOUT_32MS           0x0C
#define RA12_TIMEOUT_64MS           0x0D
#define RA12_TIMEOUT_128MS          0x0E
#define RA12_TIMEOUT_256MS          0x0F
#define RA12_TIMEOUT_512MS          0x10
#define RA12_TIMEOUT_1SEC           0x11
#define RA12_TIMEOUT_2SEC           0x12

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
void RA12_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver);
uint8_t RA12_checkIRQ(uint8_t irqEn);
ra12_deviceRev_t RA12_getRevision(void);
hfalStatus_t RA12_analyze(void);
hfalStatus_t RA12_configuration(ra12_configProtocol_t configProtocol);
void RA12_enableIRQ(uint8_t irqEnable);
void RA12_disableIRQ(uint8_t irqDisable);
void RA12_clearFlagIRQ(uint8_t irqFlag);
uint8_t RA12_getFlagIRQ(uint8_t getFlag);

void RA12_timerStart(void);
void RA12_timerStop(void);
void RA12_timerAutoRestart(uint8_t restartEn);
void RA12_timerConfigure(uint8_t prescale, uint8_t reload);
void RA12_getTimerConfigure(uint8_t *prescale, uint8_t *reload);

void RA12_timerWkupConfig(ra12_wkupTimer_t wkupTimer);
void RA12_timerWkupStart(void);
void RA12_timerWkupStop(void);
hfalStatus_t RA12_timerWkupPrescaler(uint8_t prescale);
hfalStatus_t RA12_timerWkupReload(uint8_t reload);
hfalStatus_t RA12_timerWkupTrigTime(uint8_t trigTime);
uint8_t RA12_timerWkupGetStatus(void);

hfalStatus_t RA12_calInternalADC(void);
void RA12_getADC(uint8_t *adcI, uint8_t *adcQ);
void RA12_wkupCDInit(ra12_wkupCDProperties_t wkupCDProp);
void RA12_CDInit(uint8_t CDTxDelay, uint8_t CDaverage);
void RA12_CDThresholdConfig(ra12_CDThreshold_t CDThreshold);
hfalStatus_t RA12_CDModeSelect(ra12_CDmode_t cdMode);
hfalStatus_t RA12_CDPreCal(ra12_wkupTimer_t wkupTimer);
hfalStatus_t RA12_CDPostCal(ra12_wkupTimer_t wkupTimer,
                            ra12_CDThreshold_t cdThreshold);

void RA12_rfOperate(ra12_rfControl_t rfCtrl);
void RA12_flushFIFO(void);
void RA12_clearState(void);
void RA12_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn);
void RA12_send1PulseBit(uint8_t onePulseBit);
void RA12_clearCryptoBit(void);
void RA12_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit);
void RA12_setCollMaskVal(uint8_t collEn);
uint8_t RA12_readCollPos(void);
hfalStatus_t RA12_checkRFErr(void);
hfalStatus_t RA12_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut);
hfalStatus_t RA12_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut);
hfalStatus_t RA12_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen,
                                    uint16_t timeOut);
hfalStatus_t RA12_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                     uint8_t *dataRx, uint16_t *dataRxLen,
                                     uint8_t txCRCEn, uint8_t rxCRCEn,
                                     uint8_t timeOut);
hfalStatus_t RA12_loadKey(uint8_t *key);
hfalStatus_t RA12_authenticate(uint8_t *dataAuthent);
void RA12_utilDelay(uint32_t ms);


// HF Reader IO functions ----------------------------------------------------//
extern void HFREADER_IO_init(uint8_t irqEnable, uint8_t rstpdEnable);
extern void HFREADER_IO_read1Reg(uint8_t addr, uint8_t *data);
extern void HFREADER_IO_write1Reg(uint8_t addr, uint8_t data);
extern uint8_t HFREADER_IO_readIrq(void);
extern void HFREADER_IO_timStart(uint32_t timeout);
extern void HFREADER_IO_timStop(void);
extern uint8_t HFREADER_IO_timIsExpired(void);
extern void HFREADER_IO_delay(uint32_t ms);

#endif  // #ifndef RA12_H
