//! \file:  re41.h
//! \brief: This file provides basic code for using RE41 reader chip.

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
// UPDATE: 2020-11-23
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RE41-R1.3-20190405
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef RE41_H
#define RE41_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// re41_configParam_t is a default register structure on RE41 reader to be set
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

    uint8_t  GainST3;           // address: 0x3F
} re41_configParam_t;

// Standard RF protocols communication support by RE41 reader are defined here
typedef enum
{
    ISO14443A_106KBPS = 0x00u,
    ISO14443B_106KBPS,
    ISO15693_1OUT4_1SUB_HI,
    FELICA_212KBPS
} re41_configProtocol_t;

// The supported device revision
typedef enum
{
    RE41_CODE1       = 0x7F,
    RE41_CODE2       = 0xBF,
    RE41_UNKNOWN
} re41_deviceRev_t;


// RF control group definition
typedef enum
{
    RFON = 0x00u,
    RFOFF,
    TX1ON,
    TX2ON
} re41_rfControl_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// RE41 register's address definition
#define RE41_CMDREG                 0x01
#define RE41_FIFODATA               0x02
#define RE41_PRIMSTATUS             0x03
#define RE41_FIFOLEN                0x04
#define RE41_SECONDSTATUS           0x05
#define RE41_IRQEN                  0x06
#define RE41_IRQFLAG                0x07

#define RE41_CTRLREG                0x09
#define RE41_ERRFLAG                0x0A
#define RE41_COLLPOS                0x0B
#define RE41_TIMVALREG              0x0C
#define RE41_CRCRESULTLSB           0x0D
#define RE41_CRCRESULTMSB           0x0E
#define RE41_BITFRAMING             0x0F

#define RE41_TXCTRL                 0x11
#define RE41_TXCFGPWR               0x12
#define RE41_TXCFGMOD               0x13
#define RE41_CODERCTRL              0x14
#define RE41_MODWIDTH               0x15
#define RE41_MODWIDTHSOF            0x16
#define RE41_TYPEBFRAMING           0x17

#define RE41_RXCTRL1                0x19
#define RE41_DECODERCTRL            0x1A
#define RE41_BITPHASE               0x1B
#define RE41_RXTHRESHOLD            0x1C
#define RE41_BPSKDEMOD              0x1D
#define RE41_RXCTRL2                0x1E
#define RE41_RXCTRL3                0x1F

#define RE41_RXWAIT                 0x21
#define RE41_CRCSETTING             0x22
#define RE41_CRCPRESETLSB           0x23
#define RE41_CRCPRESETMSB           0x24

#define RE41_FIFOLEVEL              0x29
#define RE41_TIMERCLOCK             0x2A
#define RE41_TIMERCONTROL           0x2B
#define RE41_TIMERRELOAD            0x2C
#define RE41_MANFILTERCTRL          0x2E
#define RE41_FILTERADJUST           0x2F

#define RE41_IOCONFIG               0x31
#define RE41_SIGNAL_INDICATOR       0x37
#define RE41_DEVICETYPE             0x38

#define RE41_TEST                   0x3A
#define RE41_TXDISABLE              0x3B
#define RE41_GAIN_ST3               0x3F

// RE41 command for writing to address 0x01
#define IDLE_CMD                    0x00
#define WREEPROM_CMD                0x01
#define RDEEPROM_CMD                0x03
#define LOADCFG_EEPROM_CMD          0x07
#define LOADKEY_EEPROM_CMD          0x0B
#define TUNEFILTER_CMD              0x10
#define CALCRC_CMD                  0x12
#define LOADKEY_FIFO_CMD            0x19
#define RX_CMD                      0x16
#define TX_CMD                      0x1A
#define AUTHENT_CMD                 0x1C
#define TRANSCEIVE_CMD              0x1E

// RE41 secondary status (ADDR 0x05)
#define TRUNNING_STATUS             0x80
#define E2READY_STATUS              0x40
#define CRCREADY_STATUS             0x20
#define EMD_DET_STATUS              0x10
#define SUBC_DET_STATUS             0x08
#define RXLASTBIT_STATUS            0x07

// mask bit of interrupt reg (ADDR 0x06, 0x07)
#define SETIRQ_FLAG                 0x80
#define SETIRQ_EN                   0x80
#define ALL_IRQ                     0x3F
#define TIMER_IRQ                   0x20
#define TX_IRQ                      0x10
#define RX_IRQ                      0x08
#define IDLE_IRQ                    0x04
#define HIALERT_IRQ                 0x02
#define LOALERT_IRQ                 0x01

// mask bit of system control (ADDR 0x09)
#define STANDBY_MODE                0x20
#define PWR_DOWN_MODE               0x10
#define CYRPTO1_MOn                 0x08
#define CLEAR_ALL_MODE              0xC7
#define CHECK_ALL_MODE              0x38
#define FLUSHFIFO_BIT               0x01
#define TSTART_BIT                  0x02
#define TSTOP_BIT                   0x04

// mask bit of error register (ADDR 0x0A)
#define EEPROM_ERR_MASK             0x80
#define KEY_ERR_MASK                0x40
#define ACCESS_ERR_MASK             0x20
#define FIFO_OVERFLOW_MASK          0x10
#define CRC_ERR_MASK                0x08
#define FRAMING_ERR_MASK            0x04
#define PARITY_ERR_MASK             0x02
#define COLL_ERR_MASK               0x01

// mask bit of TxControl reg (ADDR 0x11)
#define TX1INV_MASK                 0x80
#define MODSRC_MASK                 0x60
#define FORCE100ASK_MASK            0x10
#define TX2INV_MASK                 0x08
#define TX2CW_MASK                  0x04
#define TX2RFEn_MASK                0x02
#define TX1RFEn_MASK                0x01

// set or clear one pulse bit (ADDR 0x14)
#define SET1PULSE_BIT               0x80
#define CLR1PULSE_BIT               0x7F

// set or clear RxMultiple bit (ADDR 0x1A)
#define SET_RXMULTIPLE              0x80
#define CLR_RXMULTIPLE              0x7F

// set or clear timer auto restart bit (ADDR 0x2A)
#define SET_TAUTORESTART            0x20
#define CLR_TAUTORESTART            0xDF

// mask bit of RE41 TIMER control (ADDR 0x2B)
#define RE41_TSTOP_RXEND            0x08
#define RE41_TSTOP_RXBEGIN          0x04
#define RE41_TSTART_TXEND           0x02
#define RE41_TSTART_TXBEGIN         0x01

// RE41 program timer for transparent command
#define RE41_TIMEOUT_DEFAULT        0x00
#define RE41_TIMEOUT_1MS            0x07
#define RE41_TIMEOUT_2MS            0x08
#define RE41_TIMEOUT_4MS            0x09
#define RE41_TIMEOUT_8MS            0x0A
#define RE41_TIMEOUT_16MS           0x0B
#define RE41_TIMEOUT_32MS           0x0C
#define RE41_TIMEOUT_64MS           0x0D
#define RE41_TIMEOUT_128MS          0x0E
#define RE41_TIMEOUT_256MS          0x0F
#define RE41_TIMEOUT_512MS          0x10
#define RE41_TIMEOUT_1SEC           0x11
#define RE41_TIMEOUT_2SEC           0x12

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
// general purpose function on RE41 reader
void RE41_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver);
uint8_t RE41_checkIRQ(uint8_t irqEn);
re41_deviceRev_t RE41_getRevision(void);
hfalStatus_t RE41_analyze(void);
hfalStatus_t RE41_configuration(re41_configProtocol_t configProtocol);
void RE41_enableIRQ(uint8_t irqEnable);
void RE41_disableIRQ(uint8_t irqDisable);
void RE41_clearFlagIRQ(uint8_t irqFlag);
uint8_t RE41_getFlagIRQ(uint8_t getFlag);
void RE41_timerStart(void);
void RE41_timerStop(void);
void RE41_timerAutoRestart(uint8_t restartEn);
void RE41_timerConfigure(uint8_t prescale, uint8_t reload);
void RE41_getTimerConfigure(uint8_t *prescale, uint8_t *reload);

// HFREADER driver standard mapping
void RE41_rfOperate(re41_rfControl_t rfCtrl);
void RE41_flushFIFO(void);
void RE41_clearState(void);
void RE41_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn);
void RE41_send1PulseBit(uint8_t onePulseBit);
void RE41_clearCryptoBit(void);
void RE41_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit);
void RE41_setCollMaskVal(uint8_t collEn);
uint8_t RE41_readCollPos(void);
hfalStatus_t RE41_checkRFErr(void);
hfalStatus_t RE41_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen, uint16_t timeOut);
hfalStatus_t RE41_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen, uint16_t timeOut);
hfalStatus_t RE41_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen,
                                    uint16_t timeOut);
hfalStatus_t RE41_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                     uint8_t *dataRx, uint16_t *dataRxLen,
                                     uint8_t txCRCEn, uint8_t rxCRCEn,
                                     uint8_t timeOut);
hfalStatus_t RE41_loadKey(uint8_t *key);
hfalStatus_t RE41_authenticate(uint8_t *dataAuthent);
void RE41_utilDelay(uint32_t ms);


// HF Reader IO functions ----------------------------------------------------//
extern void HFREADER_IO_init(uint8_t irqEnable, uint8_t rstpdEnable);
extern void HFREADER_IO_read1Reg(uint8_t addr, uint8_t *data);
extern void HFREADER_IO_write1Reg(uint8_t addr, uint8_t data);
extern uint8_t HFREADER_IO_readIrq(void);
extern void HFREADER_IO_timStart(uint32_t timeout);
extern void HFREADER_IO_timStop(void);
extern uint8_t HFREADER_IO_timIsExpired(void);
extern void HFREADER_IO_delay(uint32_t ms);

#endif  // #ifndef RE41_H
