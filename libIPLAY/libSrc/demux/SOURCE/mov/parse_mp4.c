/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      parse_mp4.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:  
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
#include "global612.h"
#if VIDEO_ON

#include "UtilTypeDef.h"
#include "parse_mp4.h"
#include "app_interface.h"
#include "stream.h"


#define MP4_DL MSGL_V
#define freereturn(a,b) mem_free(a); return b

int mp4_read_descr_len(stream_t * s)
{
	BYTE b;
	BYTE numBytes = 0;
	uint32_t length = 0;

	do
	{
		b = stream_read_char(s);
		numBytes++;
		length = (length << 7) | (b & 0x7F);
	}
	while ((b & 0x80) && numBytes < 4);

	//MP_DPF("MP4 read desc len: %d\n", length);
	return length;
}

/* parse the data part of MP4 esds atoms */
int mp4_parse_esds(unsigned char *data, int datalen, esds_t * esds)
{
	/* create memory stream from data */
	stream_t *s = (stream_t *) new_memory_stream(data, datalen);
	BYTE len;

#ifdef MP4_DUMPATOM
	{
		int i;

		MP_DPF("ESDS Dump (%dbyte):\n", datalen);
		for (i = 0; i < datalen; i++)
			MP_DPF("%02X ", data[i]);
		MP_DPF("\nESDS Dumped\n");
	}
#endif
	memset(esds, 0, sizeof(esds_t));

	esds->version = stream_read_char(s);
	esds->flags = stream_read_int24(s);

	/* get and verify ES_DescrTag */
	if (stream_read_char(s) == MP4ESDescrTag)
	{
		/* read length */
		len = mp4_read_descr_len(s);

		esds->ESId = stream_read_word(s);
		esds->streamPriority = stream_read_char(s);

		if (len < (5 + 15))
		{
			freereturn(s, 1);
		}
	}
	else
	{
		esds->ESId = stream_read_word(s);
	}

	/* get and verify DecoderConfigDescrTab */
	if (stream_read_char(s) != MP4DecConfigDescrTag)
	{
		freereturn(s, 1);
	}

	/* read length */
	len = mp4_read_descr_len(s);

	esds->objectTypeId = stream_read_char(s);
	esds->streamType = stream_read_char(s);
	esds->bufferSizeDB = stream_read_int24(s);
	esds->maxBitrate = stream_read_dword(s);
	esds->avgBitrate = stream_read_dword(s);

	esds->decoderConfigLen = 0;

	if (len < 15)
	{
		freereturn(s, 0);
	}

	/* get and verify DecSpecificInfoTag */
	if (stream_read_char(s) != MP4DecSpecificDescrTag)
	{
		freereturn(s, 0);
	}

	/* read length */
	esds->decoderConfigLen = len = mp4_read_descr_len(s);

	esds->decoderConfig = (BYTE *) mem_malloc(esds->decoderConfigLen);
	if (esds->decoderConfig)
	{
		stream_read(s, (char *) esds->decoderConfig, esds->decoderConfigLen);
	}
	else
	{
		esds->decoderConfigLen = 0;
		//CHK_MALLOC(esds->decoderConfig, "mp4_parse_esds failed");
	}

	/* get and verify SLConfigDescrTag */
	if (stream_read_char(s) != MP4SLConfigDescrTag)
	{
		freereturn(s, 0);
	}

	/* Note: SLConfig is usually constant value 2, size 1Byte */
	esds->SLConfigLen = len = mp4_read_descr_len(s);
	esds->SLConfig = (BYTE *) mem_malloc(esds->SLConfigLen);
	if (esds->SLConfig)
	{
		stream_read(s, (char *) esds->SLConfig, esds->SLConfigLen);
	}
	else
	{
		esds->SLConfigLen = 0;
		//CHK_MALLOC(esds->SLConfig, "mp4_parse_esds failed");
	}
	/* will skip the remainder of the atom */
	freereturn(s, 0);

}

/* cleanup all mem occupied by mp4_parse_esds */
void mp4_free_esds(esds_t * esds)
{
	if (esds->decoderConfigLen)
		mem_free(esds->decoderConfig);
	if (esds->SLConfigLen)
		mem_free(esds->SLConfig);
}

#undef freereturn
#undef MP4_DL

#endif
