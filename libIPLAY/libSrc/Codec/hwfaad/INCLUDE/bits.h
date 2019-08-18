/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      bits.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li    first file
****************************************************************
*/
#ifndef __BITS_H__
#define __BITS_H__

#define DEBUGDEC
#define DEBUGVAR(A,B,C)

#define BYTE_NUMBIT 8
#define bit2byte(a) ((a+7)/BYTE_NUMBIT)

typedef struct _bitfile
{
    /* bit input */
    uint32_t bufa;
    uint32_t bufb;
    uint32_t bits_left;
    uint32_t buffer_size; /* size of the buffer in bytes */
    uint32_t bytes_used;
    BYTE no_more_reading;
    BYTE error;
    uint32_t *tail;
    uint32_t *start;
    void *buffer;
} bitfile;


#define BSWAP(a) \
    ((a) = ( ((a)&0xff)<<24) | (((a)&0xff00)<<8) | (((a)>>8)&0xff00) | (((a)>>24)&0xff))

static uint32_t bitmask[] = {
    0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF,
    0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
    0x1FFFF, 0x3FFFF, 0x7FFFF, 0xFFFFF, 0x1FFFFF, 0x3FFFFF,
    0x7FFFFF, 0xFFFFFF, 0x1FFFFFF, 0x3FFFFFF, 0x7FFFFFF,
    0xFFFFFFF, 0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF
};

void hw_faad_initbits(bitfile *ld, const void *buffer, const uint32_t buffer_size);
void hw_faad_endbits(bitfile *ld);
BYTE hw_faad_byte_align(bitfile *ld);
uint32_t hw_faad_get_processed_bits(bitfile *ld);
void hw_faad_flushbits_ex(bitfile *ld, uint32_t bits);

/* circumvent memory alignment errors on ARM */
static INLINE uint32_t getdword(void *mem)
{
#if 1
    uint32_t tmp;
    tmp = *(uint32_t*)mem;
#ifndef ARCH_IS_BIG_ENDIAN
    BSWAP(tmp);
#endif
    return tmp;
#else
    	uint32_t tmp;
	__asm__ __volatile__(
		"lw %1, %0"
		:"=a" (tmp)
		:""

	);
#endif
}

static INLINE uint32_t faad_showbits(bitfile *ld, uint32_t bits)
{
    if (bits <= ld->bits_left)
    {
        return (ld->bufa >> (ld->bits_left - bits)) & bitmask[bits];
    }

    bits -= ld->bits_left;
    return ((ld->bufa & bitmask[ld->bits_left]) << bits) | (ld->bufb >> (32 - bits));
}

static INLINE void faad_flushbits(bitfile *ld, uint32_t bits)
{
    /* do nothing if error */
    if (ld->error != 0)
        return;

    if (bits < ld->bits_left)
    {
        ld->bits_left -= bits;
    } else {
        hw_faad_flushbits_ex(ld, bits);
    }
}

/* return next n bits (right adjusted) */
static INLINE uint32_t faad_getbits(bitfile *ld, uint32_t n DEBUGDEC)
{
    uint32_t ret;

    if (ld->no_more_reading || n == 0)
        return 0;

    ret = faad_showbits(ld, n);
    faad_flushbits(ld, n);
    return ret;
}

static INLINE BYTE faad_get1bit(bitfile *ld DEBUGDEC)
{
    BYTE r;

    if (ld->bits_left == 0)
        return (BYTE)faad_getbits(ld, 1 DEBUGVAR(print,var,dbg));

    ld->bits_left--;
    r = (BYTE)((ld->bufa >> ld->bits_left) & 1);

    return r;
}

#endif
