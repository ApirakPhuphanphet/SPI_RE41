//! \file:  re31.h
//! \brief: This file provides basic code for using RE31 reader chip.

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
// UPDATE: 2021-03-22
//
// REFERENCE DOCUMENT:
// 1) PD-FM-51-DTS-RE31-R1.3-20190405
//
// NOTE: -
//
//----------------------------------------------------------------------------//

#ifndef RE31_H
#define RE31_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include "hfreader.h"

//----------- (2) EXPORTED TYPES ---------------------------------------------//
// re31_configParam_t is a default register structure on RE31 reader to be set
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
} re31_configParam_t;

// Standard RF protocols communication support by RE31 reader are defined here
typedef enum
{
    ISO14443A_106KBPS = 0x00u,
    ISO14443B_106KBPS,
    ISO15693_1OUT4_1SUB_HI,
} re31_configProtocol_t;

// The supported device revision
typedef enum
{
    RE31_CODE    = 0x3B,
    RE31_UNKNOWN
} re31_deviceRev_t;

// RF control group definition
typedef enum
{
    RFON = 0x00u,
    RFOFF,
    TX1ON,
    TX2ON
} re31_rfControl_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// RE31 register's address definition
#define RE31_CMDREG                 0x01
#define RE31_FIFODATA               0x02
#define RE31_PRIMSTATUS             0x03
#define RE31_FIFOLEN                0x04
#define RE31_SECONDSTATUS           0x05
#define RE31_IRQEN                  0x06
#define RE31_IRQFLAG                0x07

#define RE31_CTRLREG                0x09
#define RE31_ERRFLAG                0x0A
#define RE31_COLLPOS                0x0B
#define RE31_TIMVALREG              0x0C
#define RE31_CRCRESULTLSB           0x0D
#define RE31_CRCRESULTMSB           0x0E
#define RE31_BITFRAMING             0x0F

#define RE31_TXCTRL                 0x11
#define RE31_TXCFGPWR               0x12
#define RE31_TXCFGMOD               0x13
#define RE31_CODERCTRL              0x14
#define RE31_MODWIDTH               0x15
#define RE31_MODWIDTHSOF            0x16
#define RE31_TYPEBFRAMING           0x17

#define RE31_RXCTRL1                0x19
#define RE31_DECODERCTRL            0x1A
#define RE31_BITPHASE               0x1B
#define RE31_RXTHRESHOLD            0x1C
#define RE31_BPSKDEMOD              0x1D
#define RE31_RXCTRL2                0x1E
#define RE31_RXCTRL3                0x1F

#define RE31_RXWAIT                 0x21
#define RE31_CRCSETTING             0x22
#define RE31_CRCPRESETMSB           0x23
#define RE31_CRCPRESETLSB           0x24

#define RE31_FIFOLEVEL              0x29
#define RE31_TIMERCLOCK             0x2A
#define RE31_TIMERCONTROL           0x2B
#define RE31_TIMERRELOAD            0x2C
#define RE31_MANFILTERCTRL          0x2E
#define RE31_FILTERADJUST           0x2F

#define RE31_IOCONFIG               0x31
#define RE31_SIGNAL_INDICATOR       0x37
#define RE31_DEVICETYPE             0x38

#define RE31_TEST                   0x3A
#define RE31_TXDISABLE              0x3B
#define RE31_GAIN_ST3               0x3F

// RE31 command for writing to address 0x01
#define IDLE_CMD                    0x00
#define WREEPROM_CMD                0x01
#define RDEEPROM_CMD                0x03
#define LOADCFG_EEPROM_CMD          0x07
#define LOADKEY_EEPROM_CMD          0x0B
#define TUNEFILTER_CMD              0x10
#define CALCRC_CMD                  0x12
#define RX_CMD                      0x16
#define LOADKEY_FIFO_CMD            0x19
#define TX_CMD                      0x1A
#define AUTHENT_CMD                 0x1C
#define TRANSCEIVE_CMD              0x1E

// RE31 secondary status (ADDR 0x05)
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

// mask bit of RE31 TIMER control (ADDR 0x2B)
#define RE31_TSTOP_RXEND            0x08
#define RE31_TSTOP_RXBEGIN          0x04
#define RE31_TSTART_TXEND           0x02
#define RE31_TSTART_TXBEGIN         0x01

// RE31 program timer for transparent command
#define RE31_TIMEOUT_DEFAULT        0x00
#define RE31_TIMEOUT_1MS            0x07
#define RE31_TIMEOUT_2MS            0x08
#define RE31_TIMEOUT_4MS            0x09
#define RE31_TIMEOUT_8MS            0x0A
#define RE31_TIMEOUT_16MS           0x0B
#define RE31_TIMEOUT_32MS           0x0C
#define RE31_TIMEOUT_64MS           0x0D
#define RE31_TIMEOUT_128MS          0x0E
#define RE31_TIMEOUT_256MS          0x0F
#define RE31_TIMEOUT_512MS          0x10
#define RE31_TIMEOUT_1SEC           0x11
#define RE31_TIMEOUT_2SEC           0x12

//----------- (5) EXPORTED VARIABLES -----------------------------------------//
// N/A

//----------- (6) EXPORTED FUNCTIONS -----------------------------------------//
// general purpose function on RE31 reader
void RE31_init(uint8_t irqEn, uint8_t rstpdEn, HFREADER_DrvTypeDef **hfDriver);
uint8_t RE31_checkIRQ(uint8_t irqEn);
re31_deviceRev_t RE31_getRevision(void);
hfalStatus_t RE31_analyze(void);
hfalStatus_t RE31_configuration(re31_configProtocol_t configProtocol);
void RE31_enableIRQ(uint8_t irqEnable);
void RE31_disableIRQ(uint8_t irqDisable);
void RE31_clearFlagIRQ(uint8_t irqFlag);
uint8_t RE31_getFlagIRQ(uint8_t getFlag);
void RE31_timerStart(void);
void RE31_timerStop(void);
void RE31_timerAutoRestart(uint8_t restartEn);
void RE31_timerConfigure(uint8_t prescale, uint8_t reload);
void RE31_getTimerConfigure(uint8_t *prescale, uint8_t *reload);

// HFREADER driver standard mapping
void RE31_rfOperate(re31_rfControl_t rfCtrl);
void RE31_flushFIFO(void);
void RE31_clearState(void);
void RE31_crcSetting(uint8_t txCRCEn, uint8_t rxCRCEn);
void RE31_send1PulseBit(uint8_t onePulseBit);
void RE31_clearCryptoBit(void);
void RE31_setBitFraming(uint8_t RxAlign, uint8_t TxLastBit);
void RE31_setCollMaskVal(uint8_t collEn);
uint8_t RE31_readCollPos(void);
hfalStatus_t RE31_checkRFErr(void);
hfalStatus_t RE31_transmitCommand(uint8_t *dataTx, uint16_t  dataTxLen);
hfalStatus_t RE31_receiveCommand(uint8_t *dataRx, uint16_t *dataRxLen);
hfalStatus_t RE31_transceiveCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                    uint8_t *dataRx, uint16_t *dataRxLen);
hfalStatus_t RE31_transparentCommand(uint8_t *dataTx, uint16_t  dataTxLen,
                                     uint8_t *dataRx, uint16_t *dataRxLen,
                                     uint8_t txCRCEn, uint8_t rxCRCEn,
                                     uint8_t timeOut);
hfalStatus_t RE31_loadKey(uint8_t *key);
hfalStatus_t RE31_authenticate(uint8_t *dataAuthent);
void RE31_utilDelay(uint32_t ms);


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
