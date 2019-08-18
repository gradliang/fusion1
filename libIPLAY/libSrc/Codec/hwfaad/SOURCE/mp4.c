/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mp4.c
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

#include "bits.h"
#include "mp4.h"
#include "syntax.h"

/* Table 1.6.1 */
int8_t FAADAPI AudioSpecificConfig(uint8_t * pBuffer,
								   uint32_t buffer_size, mp4AudioSpecificConfig * mp4ASC)
{
	return AudioSpecificConfig2_faad(pBuffer, buffer_size, mp4ASC, NULL);
}

int8_t FAADAPI AudioSpecificConfig2_faad(uint8_t * pBuffer,
									uint32_t buffer_size,
									mp4AudioSpecificConfig * mp4ASC, program_config * pce)
{
	bitfile ld;
	int8_t result = 0;

	if (pBuffer == NULL)
		return -7;
	if (mp4ASC == NULL)
		return -8;

	memset(mp4ASC, 0, sizeof(mp4AudioSpecificConfig));

	hw_faad_initbits(&ld, pBuffer, buffer_size);
	hw_faad_byte_align(&ld);

	mp4ASC->objectTypeIndex = (uint8_t) faad_getbits(&ld, 5
												  DEBUGVAR(1, 1,
														   "parse_audio_decoder_specific_info(): ObjectTypeIndex"));

	mp4ASC->samplingFrequencyIndex = (uint8_t) faad_getbits(&ld, 4
														 DEBUGVAR(1, 2,
																  "parse_audio_decoder_specific_info(): SamplingFrequencyIndex"));

	mp4ASC->channelsConfiguration = (uint8_t) faad_getbits(&ld, 4
														DEBUGVAR(1, 3,
																 "parse_audio_decoder_specific_info(): ChannelsConfiguration"));

	mp4ASC->samplingFrequency = get_sample_rate_faad(mp4ASC->samplingFrequencyIndex);

	if (mp4ASC->objectTypeIndex != LC)
	{
		hw_faad_endbits(&ld);
		return -1;
	}

	if (mp4ASC->samplingFrequency == 0)
	{
		hw_faad_endbits(&ld);
		return -2;
	}

	if (mp4ASC->channelsConfiguration > 2)
	{
		hw_faad_endbits(&ld);
		return -3;
	}

	/* get GASpecificConfig_faad */
	result = GASpecificConfig_faad(&ld, mp4ASC, pce);

	hw_faad_endbits(&ld);

	return result;
}
