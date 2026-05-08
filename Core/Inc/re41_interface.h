// header file of re41_interface.c
#include <hfreader.h>
#ifndef __RE41_INTERFACE_H
#define __RE41_INTERFACE_H

uint8_t read_type_A(uint8_t addr);
uint8_t write_type_A(uint8_t pageNo, uint8_t *data, uint16_t dataLen);
uint8_t dump_mem(void);
uint8_t re41_cli_rspPrintout(hfalStatus_t sts, uint8_t *rsp, uint16_t rspLen, uint8_t opt);

typedef enum
{
    CMD_WRITE = 0x00,
    CMD_READ = 0x01,
    CMD_RESET = 0x02,
    CMD_DUMP_MEM = 0x03
} CommandTypeDef;
#endif /* __RE41_INTERFACE_H */