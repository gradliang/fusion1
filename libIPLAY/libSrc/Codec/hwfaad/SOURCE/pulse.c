/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      pulse.c
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

#include "syntax.h"
#include "pulse.h"

uint8_t pulse_decode_2(ic_stream * ics, int16_t * spec_data, uint16_t framelen)
{
	uint8_t i;
	uint16_t k;
	pulse_info *pul = &(ics->pul);

	k = ics->swb_offset[pul->pulse_start_sfb];

	for (i = 0; i <= pul->number_pulse; i++)
	{
		k += pul->pulse_offset[i];

		if (k >= framelen)
			return 15;			/* should not be possible */

		if (spec_data[k] > 0)
			spec_data[k] += pul->pulse_amp[i];
		else
			spec_data[k] -= pul->pulse_amp[i];
	}

	return 0;
}
