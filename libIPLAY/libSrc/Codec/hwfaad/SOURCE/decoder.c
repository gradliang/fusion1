/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      decoder.c
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
#include "global612.h"
#include "mpTrace.h"

#include "common.h"
#include "structs.h"

#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "decoder.h"
#include "mp4.h"
#include "syntax.h"

#include "codec.h"

faacDecHandle FAADAPI faacDecOpen()
{
	uint8_t i;
	faacDecHandle hDecoder = NULL;

	if ((hDecoder = (faacDecHandle) mem_malloc(sizeof(faacDecStruct))) == NULL)
		return NULL;

	memset(hDecoder, 0, sizeof(faacDecStruct));

	hDecoder->config.outputFormat = FAAD_FMT_16BIT;
	hDecoder->config.defObjectType = LC;
	hDecoder->config.defSampleRate = 44100;	/* Default: 44.1kHz */
	hDecoder->adts_header_present = 0;
	hDecoder->adif_header_present = 0;

	hDecoder->frame = 0;
	hDecoder->sample_buffer = NULL;

	for (i = 0; i < MAX_CHANNELS; i++)
		hDecoder->window_shape_prev[i] = 0;

	return hDecoder;
}

int32_t FAADAPI faacDecInit(faacDecHandle hDecoder, uint8_t * buffer,
							uint32_t buffer_size, uint32_t * samplerate, uint8_t * channels)
{
	uint32_t bits = 0;
	bitfile ld;
	adif_header adif;
	adts_header adts;

	if ((hDecoder == NULL) || (samplerate == NULL) || (channels == NULL))
		return -1;

	hDecoder->sf_index = get_sr_index_faad(hDecoder->config.defSampleRate);
	hDecoder->object_type = hDecoder->config.defObjectType;
	*samplerate = get_sample_rate_faad(hDecoder->sf_index);
	*channels = 1;

	if (buffer != NULL)
	{
		hw_faad_initbits(&ld, buffer, buffer_size);

		/* Check if an ADIF header is present */
		if ((buffer[0] == 'A') && (buffer[1] == 'D') && (buffer[2] == 'I') && (buffer[3] == 'F'))
		{
			hDecoder->adif_header_present = 1;

			hw_get_adif_header(&adif, &ld);
			hw_faad_byte_align(&ld);

			hDecoder->sf_index = adif.pce[0].sf_index;
			hDecoder->object_type = adif.pce[0].object_type + 1;

			*samplerate = get_sample_rate_faad(hDecoder->sf_index);
			*channels = adif.pce[0].channels;

			memcpy(&(hDecoder->pce), &(adif.pce[0]), sizeof(program_config));
			bits = bit2byte(hw_faad_get_processed_bits(&ld));

			/* Check if an ADTS header is present */
		}
		else if (faad_showbits(&ld, 12) == 0xfff)
		{
			hDecoder->adts_header_present = 1;

			adts.old_format = hDecoder->config.useOldADTSFormat;
			hw_adts_frame(&adts, &ld);

			hDecoder->sf_index = adts.sf_index;
			hDecoder->object_type = adts.profile + 1;

			*samplerate = get_sample_rate_faad(hDecoder->sf_index);
			*channels = (adts.channel_configuration > 6) ? 2 : adts.channel_configuration;
		}

		if (ld.error)
		{
			hw_faad_endbits(&ld);
			return -1;
		}
		hw_faad_endbits(&ld);
	}

	/* allocate the buffer for the final samples */
	if (hDecoder->sample_buffer == NULL)
	{
		hDecoder->sample_buffer = (void *) mem_malloc(1024 ** channels * sizeof(int16_t));
		if (hDecoder->sample_buffer == NULL)
		{
			//CHK_MALLOC(hDecoder->sample_buffer, "FAAD decoder faacDecDecode malloc  failed!");
			return -1;
		}
	}

	if (hDecoder->object_type != LC)
		return -1;

	return bits;
}

/* Init the library using a DecoderSpecificInfo */
int8_t FAADAPI faacDecInit2(faacDecHandle hDecoder, uint8_t * pBuffer,
							uint32_t SizeOfDecoderSpecificInfo,
							uint32_t * samplerate, uint8_t * channels)
{
	int8_t rc;
	mp4AudioSpecificConfig mp4ASC;

	if ((hDecoder == NULL)
		|| (pBuffer == NULL)
		|| (SizeOfDecoderSpecificInfo < 2) || (samplerate == NULL) || (channels == NULL))
	{
		return -1;
	}

	hDecoder->adif_header_present = 0;
	hDecoder->adts_header_present = 0;

	/* decode the audio specific config */
	rc = AudioSpecificConfig2_faad(pBuffer, SizeOfDecoderSpecificInfo, &mp4ASC, &(hDecoder->pce));

	/* copy the relevant info to the decoder handle */
	*samplerate = mp4ASC.samplingFrequency;
	if (mp4ASC.channelsConfiguration)
	{
		*channels = mp4ASC.channelsConfiguration;
	}
	else
	{
		*channels = hDecoder->pce.channels;
	}
	hDecoder->sf_index = mp4ASC.samplingFrequencyIndex;
	hDecoder->object_type = mp4ASC.objectTypeIndex;

	if (rc != 0)
	{
		return rc;
	}
	if (mp4ASC.frameLengthFlag)
		return -1;

	/* allocate the buffer for the final samples */
	if (hDecoder->sample_buffer == NULL)
	{
		hDecoder->sample_buffer = (void *) mem_malloc(1024 ** channels * sizeof(int16_t));
		if (hDecoder->sample_buffer == NULL)
		{
			//CHK_MALLOC(hDecoder->sample_buffer, "FAAD decoder faacDecDecode malloc  failed!");
			return -1;
		}
	}

	return 0;
}

void FAADAPI faacDecClose(faacDecHandle hDecoder)
{
	uint8_t i;

	if (hDecoder == NULL)
		return;

	if (hDecoder->sample_buffer)
		mem_free(hDecoder->sample_buffer);

	if (hDecoder)
		mem_free(hDecoder);
}
extern int AACDmaOut[2][512];
extern int n_frames;
void *FAADAPI faacDecDecode(faacDecHandle hDecoder,
							faacDecFrameInfo * hInfo, uint8_t * buffer, uint32_t buffer_size)
{
	adts_header adts;
	uint8_t channels = 0;
	bitfile ld;
	uint32_t bitsconsumed;

	/* local copy of globals */
	uint8_t sf_index;
	program_config *pce;

	/* safety checks */
	if ((hDecoder == NULL) || (hInfo == NULL) || (buffer == NULL))
	{
		return NULL;
	}

	sf_index = hDecoder->sf_index;
	pce = &hDecoder->pce;

	/* initialize the bitstream */
	hw_faad_initbits(&ld, buffer, buffer_size);

	if (hDecoder->adts_header_present)
	{
		adts.old_format = hDecoder->config.useOldADTSFormat;
		if ((hInfo->error = hw_adts_frame(&adts, &ld)) > 0)
			goto error;

		/* MPEG2 does byte_alignment() here,
		 * but ADTS header is always multiple of 8 bits in MPEG2
		 * so not needed to actually do it.
		 */
	}

	/* decode the complete bitstream */
	hw_raw_data_block(hDecoder, hInfo, &ld, pce);

	channels = hDecoder->fr_channels;

	if (hInfo->error > 0)
		goto error;

	/* no more bit reading after this */
	bitsconsumed = hw_faad_get_processed_bits(&ld);
	hInfo->bytesconsumed = bit2byte(bitsconsumed);
	if (ld.error)
	{
		hInfo->error = 14;
		goto error;
	}
	hw_faad_endbits(&ld);

	/* number of samples in this frame */
	hInfo->samples = 1024 * channels;

	/* check if frame has channel elements */
	if (channels == 0)
	{
		hDecoder->frame++;
		return NULL;
	}

	hDecoder->frame++;
	if (hDecoder->frame <= 1)
		hInfo->samples = 0;

	/* cleanup */
	return hDecoder->sample_buffer;

  error:
	/* cleanup */
	return NULL;
}
