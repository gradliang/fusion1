/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      fixed.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   MPEG audio decoder library
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li   first file
****************************************************************
*/

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "global.h"

# include "fixed.h"

/*
 * NAME:	fixed->abs()
 * DESCRIPTION:	return absolute value of a fixed-point number
 */
mad_fixed_t mad_f_abs(mad_fixed_t x)
{
	return x < 0 ? -x : x;
}

/*
 * NAME:	fixed->div()
 * DESCRIPTION:	perform division using fixed-point math
 */
mad_fixed_t mad_f_div(mad_fixed_t x, mad_fixed_t y)
{
	mad_fixed_t q, r;
	unsigned int bits;

	q = mad_f_abs(x / y);

  if (x < 0) {
		x = -x;
		y = -y;
	}

	r = x % y;

  if (y < 0) {
		x = -x;
		y = -y;
	}

	if (q > mad_f_intpart(MAD_F_MAX) &&
		!(q == -mad_f_intpart(MAD_F_MIN) && r == 0 && (x < 0) != (y < 0)))
		return 0;

  for (bits = MAD_F_FRACBITS; bits && r; --bits) {
		q <<= 1, r <<= 1;
		if (r >= y)
			r -= y, ++q;
	}

	/* round */
	if (2 * r >= y)
		++q;

	/* fix sign */
	if ((x < 0) != (y < 0))
		q = -q;

	return q << bits;
}
