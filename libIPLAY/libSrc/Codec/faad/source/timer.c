/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2005 M. Bakker, Nero AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** The "appropriate copyright message" mentioned in section 2c of the GPLv2
** must read: "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Nero AG through Mpeg4AAClicense@nero.com.
**
** $Id: timer.c,v 1.0 2008/04/02 09:41:33 rob Exp $
**/

#include "global612.h"
#include "timer.h"

faad_timer_t const faad_timer_zero = { 0, 0 };

/*
 * NAME:	reduce_timer()
 * DESCRIPTION:	carry timer fraction into seconds
 */
static
void reduce_timer(faad_timer_t *timer)
{
  timer->seconds  += timer->fraction / FAAD_TIMER_RESOLUTION;
  timer->fraction %= FAAD_TIMER_RESOLUTION;
}

/*
 * NAME:	gcd()
 * DESCRIPTION:	compute greatest common denominator
 */
static
unsigned long gcd(unsigned long num1, unsigned long num2)
{
  unsigned long tmp;

  while (num2) {
    tmp  = num2;
    num2 = num1 % num2;
    num1 = tmp;
  }

  return num1;
}

/*
 * NAME:	reduce_rational()
 * DESCRIPTION:	convert rational expression to lowest terms
 */
static
void reduce_rational(unsigned long *numer, unsigned long *denom)
{
  unsigned long factor;

  factor = gcd(*numer, *denom);

  *numer /= factor;
  *denom /= factor;
}

/*
 * NAME:	scale_rational()
 * DESCRIPTION:	solve numer/denom == ?/scale avoiding overflowing
 */
static
unsigned long scale_rational(unsigned long numer, unsigned long denom,
			     unsigned long scale)
{
  reduce_rational(&numer, &denom);
  reduce_rational(&scale, &denom);

  if (denom < scale)
    return numer * (scale / denom) + numer * (scale % denom) / denom;
  if (denom < numer)
    return scale * (numer / denom) + scale * (numer % denom) / denom;

  return numer * scale / denom;
}

/*
 * NAME:	timer->set()
 * DESCRIPTION:	set timer to specific (positive) value
 */
void faad_timer_set(faad_timer_t *timer, unsigned long seconds,
		   unsigned long numer, unsigned long denom)
{
  timer->seconds = seconds;
  if (numer >= denom && denom > 0) {
    timer->seconds += numer / denom;
    numer %= denom;
  }

  switch (denom) {
  case 0:
  case 1:
    timer->fraction = 0;
    break;

  default:
    timer->fraction = numer * (FAAD_TIMER_RESOLUTION / denom);
    break;
  }

  if (timer->fraction >= FAAD_TIMER_RESOLUTION)
    reduce_timer(timer);
}

/*
 * NAME:	timer->add()
 * DESCRIPTION:	add one timer to another
 */
void faad_timer_add(faad_timer_t *timer, faad_timer_t incr)
{
  timer->seconds  += incr.seconds;
  timer->fraction += incr.fraction;

  if (timer->fraction >= FAAD_TIMER_RESOLUTION)
    reduce_timer(timer);
}

/*
 * NAME:	timer->count()
 * DESCRIPTION:	return timer value in selected units
 */
signed long faad_timer_count(faad_timer_t timer, enum faad_units units)
{
  switch (units) {
  case FAAD_UNITS_HOURS:
    return timer.seconds / 60 / 60;

  case FAAD_UNITS_MINUTES:
    return timer.seconds / 60;

  case FAAD_UNITS_SECONDS:
    return timer.seconds;

  case FAAD_UNITS_DECISECONDS:
  case FAAD_UNITS_CENTISECONDS:
  case FAAD_UNITS_MILLISECONDS:
    return timer.seconds * (signed long) units +
      (signed long) scale_rational(timer.fraction, FAAD_TIMER_RESOLUTION,
				   units);
  }

  /* unsupported units */
  return 0;
}
