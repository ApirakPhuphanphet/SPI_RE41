//! \file  ringbuffer.c
//! \brief Ring buffer source file

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

//----------- I M P O R T A N T   N O T E ------------------------------------//
//
//----------------------------------------------------------------------------//

//----------- (1) INCLUDES ---------------------------------------------------//
#include "ringbuffer.h"

//----------- (2) PRIVATE TYPEDEF --------------------------------------------//
// N/A

//----------- (3) PRIVATE DEFINE ---------------------------------------------//
// N/A

//----------- (4) PRIVATE MACRO ----------------------------------------------//
// N/A

//----------- (5) PRIVATE VARIABLES ------------------------------------------//
// N/A

//----------- (6) PRIVATE FUNCTION PROTOTYPES --------------------------------//
// N/A

//----------- (7) PRIVATE FUNCTIONS ------------------------------------------//

void RINGBUFFER_init(ringBuffer_t *ring, uint8_t *storage, uint16_t size)
{
    ring->storage   = storage;
    ring->size      = size;
    ring->end       = ring->storage + ring->size;
    ring->read      = ring->storage;
    ring->write     = ring->storage;
    ring->available = 0;
}

uint16_t RINGBUFFER_available(ringBuffer_t *ring)
{
    return ring->available;
}

uint16_t RINGBUFFER_free(ringBuffer_t *ring)
{
    return ring->size - ring->available;
}

void RINGBUFFER_clear(ringBuffer_t *ring)
{
    ring->read      = ring->storage;
    ring->write     = ring->storage;
    ring->available = 0;
}

uint8_t RINGBUFFER_readByte(ringBuffer_t *ring)
{
    uint8_t ret = 0;

    if(ring->available == 0)
    {
        return 0;
    }
    ret = *ring->read++;
    ring->available--;
    if(ring->read >= ring->end)
    {
        ring->read = ring->storage;
    }

    return ret;
}

void RINGBUFFER_read(ringBuffer_t *ring, uint8_t *buffer, uint16_t size)
{
    uint16_t i;

    for(i = 0; i < size; i++)
    {
        buffer[i] = RINGBUFFER_readByte(ring);
    }
}

void RINGBUFFER_writeByte(ringBuffer_t *ring, uint8_t data)
{
    if(ring->available >= ring->size)
    {
        RINGBUFFER_readByte(ring);
    }

    *ring->write = data;
    ring->write++;
    ring->available++;
    if(ring->write >= ring->end)
    {
        ring->write = ring->storage;
    }
}

void RINGBUFFER_write(ringBuffer_t *ring, const uint8_t *buffer, uint16_t size)
{
    uint16_t i;

    for(i = 0; i < size; i++)
    {
        RINGBUFFER_writeByte(ring, buffer[i]);
    }
}

//----------- (8) END OF FILE ------------------------------------------------//
