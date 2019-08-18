/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      bit.h
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
# ifndef LIBMAD_BIT_H
# define LIBMAD_BIT_H
# ifdef HAVE_LIMITS_H
#  include <limits.h>
# else
#  define CHAR_BIT  8
#define CHAR_BIT_SHIFT 3
#define CHAR_BIT_MODE  7
# endif

struct mad_bitptr {
  unsigned char const *byte;
  unsigned short cache;
  unsigned short left;
};
#if 1
static inline void mad_bit_init(struct mad_bitptr *, unsigned char const *) __attribute__((always_inline));

#define mad_bit_finish(bitptr)		/* nothing */

static inline unsigned int mad_bit_length(struct mad_bitptr const *,
			    struct mad_bitptr const *) __attribute__((always_inline));

#define mad_bit_bitsleft(bitptr)  ((bitptr)->left)
static inline unsigned char const *mad_bit_nextbyte(struct mad_bitptr const *) __attribute__((always_inline));

static inline void mad_bit_skip(struct mad_bitptr *, unsigned int) __attribute__((always_inline));
#if 0
#define MAD_BIT_READ_INLINE 1
static inline unsigned long mad_bit_read(struct mad_bitptr *, unsigned int) __attribute__((always_inline));
#else
#define MAD_BIT_READ_INLINE 0
unsigned long mad_bit_read(struct mad_bitptr *, unsigned int);
#endif
//void mad_bit_write(struct mad_bitptr *, unsigned int, unsigned long);

unsigned short mad_bit_crc2(struct mad_bitptr, unsigned int, unsigned short);

/*
 * NAME:	bit->init()
 * DESCRIPTION:	initialize bit pointer struct
 */
static inline void mad_bit_init(struct mad_bitptr *bitptr, unsigned char const *byte)
{
  bitptr->byte  = byte;
  bitptr->cache = 0;
  bitptr->left  = CHAR_BIT;
}

/*
 * NAME:	bit->length()
 * DESCRIPTION:	return number of bits between start and end points
 */
static inline unsigned int mad_bit_length(struct mad_bitptr const *begin,
			    struct mad_bitptr const *end)
{
  return begin->left +
    CHAR_BIT * (end->byte - (begin->byte + 1)) + (CHAR_BIT - end->left);
}

/*
 * NAME:	bit->nextbyte()
 * DESCRIPTION:	return pointer to next unprocessed byte
 */
static inline unsigned char const *mad_bit_nextbyte(struct mad_bitptr const *bitptr)
{
  return bitptr->left == CHAR_BIT ? bitptr->byte : bitptr->byte + 1;
}

/*
 * NAME:	bit->skip()
 * DESCRIPTION:	advance bit pointer
 */
static inline void mad_bit_skip(struct mad_bitptr *bitptr, unsigned int len)
{
  bitptr->byte += len >> CHAR_BIT_SHIFT;
  len &= CHAR_BIT_MODE;
  if (bitptr->left > len)
	  bitptr->left -= len;
  else {
  	bitptr->byte++;
	bitptr->left += CHAR_BIT - len;
  }	

  bitptr->cache = *bitptr->byte;
}
#if MAD_BIT_READ_INLINE
/*
 * NAME:	bit->read()
 * DESCRIPTION:	read an arbitrary number of bits and return their UIMSBF value
 */
static inline unsigned long mad_bit_read(struct mad_bitptr *bitptr, unsigned int len)
{
  register unsigned long value;

  if (bitptr->left == CHAR_BIT)
    bitptr->cache = *bitptr->byte;

  if (len < bitptr->left) {
    value = (bitptr->cache & ((1 << bitptr->left) - 1)) >>
      (bitptr->left - len);
    bitptr->left -= len;

    return value;
  }

  /* remaining bits in current byte */

  value = bitptr->cache & ((1 << bitptr->left) - 1);
  len  -= bitptr->left;

  bitptr->byte++;
  bitptr->left = CHAR_BIT;

  /* more bytes */

  while (len >= CHAR_BIT) {
    value = (value << CHAR_BIT) | *bitptr->byte++;
    len  -= CHAR_BIT;
  }

  if (len > 0) {
    bitptr->cache = *bitptr->byte;

    value = (value << len) | (bitptr->cache >> (CHAR_BIT - len));
    bitptr->left -= len;
  }

  return value;
}
#endif
#endif
# endif
