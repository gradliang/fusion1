/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      bits.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   Advanced Audio (AAC) Decoder 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li   first file
****************************************************************
*/
#include "common.h"
#include "structs.h"

#include <stdlib.h>
#include <string.h>
#include "bits.h"
#include "global612.h"

/* initialize buffer, call once before first getbits or showbits */
void hw_faad_initbits(bitfile * ld, const void *_buffer, const uint32_t buffer_size)
{
	uint32_t tmp;

	if (ld == NULL)
		return;

	memset(ld, 0, sizeof(bitfile));

	if (buffer_size == 0 || _buffer == NULL)
	{
		ld->error = 1;
		ld->no_more_reading = 1;
		return;
	}

	ld->buffer = (void *) mem_malloc((buffer_size + 12) * sizeof(uint8_t));
	memset(ld->buffer, 0, (buffer_size + 12) * sizeof(uint8_t));
	memcpy(ld->buffer, _buffer, buffer_size * sizeof(uint8_t));

	ld->buffer_size = buffer_size;

	tmp = getdword((uint32_t *) ld->buffer);
	ld->bufa = tmp;

	tmp = getdword((uint32_t *) ld->buffer + 1);
	ld->bufb = tmp;

	ld->start = (uint32_t *) ld->buffer;
	ld->tail = ((uint32_t *) ld->buffer + 2);

	ld->bits_left = 32;

	ld->bytes_used = 0;
	ld->no_more_reading = 0;
	ld->error = 0;
}

void hw_faad_endbits(bitfile * ld)
{
	if (ld)
		if (ld->buffer)
			mem_free(ld->buffer);
}

uint32_t hw_faad_get_processed_bits(bitfile * ld)
{
	return (uint32_t) (8 * (4 * (ld->tail - ld->start) - 4) - (ld->bits_left));
}

uint8_t hw_faad_byte_align(bitfile * ld)
{
	uint8_t remainder = (uint8_t) ((32 - ld->bits_left) & 7);

	if (remainder)
	{
		faad_flushbits(ld, 8 - remainder);
		return (8 - remainder);
	}
	return 0;
}

void hw_faad_flushbits_ex(bitfile * ld, uint32_t bits)
{
	uint32_t tmp;

	ld->bufa = ld->bufb;
	tmp = getdword(ld->tail);
	ld->tail++;
	ld->bufb = tmp;
	ld->bits_left += (32 - bits);
	ld->bytes_used += 4;
	if (ld->bytes_used == ld->buffer_size)
		ld->no_more_reading = 1;
	if (ld->bytes_used > ld->buffer_size)
		ld->error = 1;
}
