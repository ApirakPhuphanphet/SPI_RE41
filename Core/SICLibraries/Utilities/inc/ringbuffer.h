//! \file  ringbuffer.h
//! \brief Ring buffer header file

//----------------------------------------------------------------------------//
// COMPANY NAME: SILICON CRAFT TECHNOLOGY CO.,LTD.
//
// INTRODUCTION:
//
// AUTHOR: APIRAK RATSAMEESAWANG
// CONTACT: apirak@sic.co.th
//
// REFERENCE DOCUMENT:
// 1) ...
//
// NOTE:
// 1) ...
//
// 2) ...
//
//----------------------------------------------------------------------------//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

//----------- (1) INCLUDES ---------------------------------------------------//
#include <stdint.h>

//----------- (2) EXPORTED TYPES ---------------------------------------------//
typedef struct
{
    uint8_t  *storage;
    uint8_t  *end;
    uint16_t size;
    volatile uint8_t  *read;
    volatile uint8_t  *write;
    volatile uint16_t available;
} ringBuffer_t;

//----------- (3) EXPORTED CONSTANTS -----------------------------------------//
// N/A

//----------- (4) EXPORTED MACRO ---------------------------------------------//
// N/A

//----------- (5) EXPORTED FUNCTIONS -----------------------------------------//
void RINGBUFFER_init(ringBuffer_t *ring, uint8_t* storage, uint16_t size);
uint16_t RINGBUFFER_available(ringBuffer_t *ring);
uint16_t RINGBUFFER_free(ringBuffer_t *ring);
void RINGBUFFER_clear(ringBuffer_t *ring);
uint8_t RINGBUFFER_readByte(ringBuffer_t *ring);
void RINGBUFFER_read(ringBuffer_t *ring, uint8_t *buffer, uint16_t size);
void RINGBUFFER_writeByte(ringBuffer_t *ring, uint8_t data);
void RINGBUFFER_write(ringBuffer_t *ring, const uint8_t *buffer, uint16_t size);

#endif // #ifndef RINGBUFFER_H
