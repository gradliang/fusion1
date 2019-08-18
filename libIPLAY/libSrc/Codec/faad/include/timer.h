/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
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
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: timer.h,v 1.0 2008/04/02 08:20:11 menno Exp $
**/

#ifndef __TIMER_H__
#define __TIMER_H__

typedef struct {
  signed long seconds;		/* whole seconds */
  unsigned long fraction;	/* 1/FAAD_TIMER_RESOLUTION seconds */
} faad_timer_t;

extern faad_timer_t const faad_timer_zero;

#define FAAD_TIMER_RESOLUTION	352800000UL

enum faad_units {
  FAAD_UNITS_HOURS		=    -2,
  FAAD_UNITS_MINUTES	=    -1,
  FAAD_UNITS_SECONDS	=     0,

  /* metric units */

  FAAD_UNITS_DECISECONDS  =    10,
  FAAD_UNITS_CENTISECONDS =   100,
  FAAD_UNITS_MILLISECONDS =  1000,

  /* audio sample units */

  FAAD_UNITS_8000_HZ	 =  8000,
  FAAD_UNITS_11025_HZ	 = 11025,
  FAAD_UNITS_12000_HZ	 = 12000,

  FAAD_UNITS_16000_HZ	 = 16000,
  FAAD_UNITS_22050_HZ	 = 22050,
  FAAD_UNITS_24000_HZ	 = 24000,

  FAAD_UNITS_32000_HZ	 = 32000,
  FAAD_UNITS_44100_HZ	 = 44100,
  FAAD_UNITS_48000_HZ	 = 48000,
};

#define faad_timer_reset(timer)	((void) (*(timer) = faad_timer_zero))

void faad_timer_set(faad_timer_t *, unsigned long, unsigned long, unsigned long);
void faad_timer_add(faad_timer_t *, faad_timer_t);

signed long faad_timer_count(faad_timer_t, enum faad_units);

#endif
