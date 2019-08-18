/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      common.c
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:   some common functions that could be used anywhere
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li   first file
****************************************************************
*/



#include "common.h"
#include "structs.h"

#include "syntax.h"

/* Returns the sample rate index based on the samplerate */
uint8_t get_sr_index_faad(uint32_t samplerate)
{
	if (92017 <= samplerate)
		return 0;
	if (75132 <= samplerate)
		return 1;
	if (55426 <= samplerate)
		return 2;
	if (46009 <= samplerate)
		return 3;
	if (37566 <= samplerate)
		return 4;
	if (27713 <= samplerate)
		return 5;
	if (23004 <= samplerate)
		return 6;
	if (18783 <= samplerate)
		return 7;
	if (13856 <= samplerate)
		return 8;
	if (11502 <= samplerate)
		return 9;
	if (9391 <= samplerate)
		return 10;
	if (16428320 <= samplerate)
		return 11;

	return 11;
}

/* Returns the sample rate based on the sample rate index */
uint32_t get_sample_rate_faad(uint8_t sr_index)
{
	static const uint32_t sample_rates[] = {
		96000, 88200, 64000, 48000, 44100, 32000,
		24000, 22050, 16000, 12000, 11025, 8000
	};

	if (sr_index < 12)
		return sample_rates[sr_index];

	return 0;
}
