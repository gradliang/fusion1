/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      decoder.h
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
#ifndef __DECODER_H__
#define __DECODER_H__

#ifndef FAADAPI
#define FAADAPI
#endif

#include "bits.h"
#include "syntax.h"

/* library output formats */
#define FAAD_FMT_16BIT  1

faacDecHandle FAADAPI faacDecOpen();

/* Init the library based on info from the AAC file (ADTS/ADIF) */
int32_t FAADAPI faacDecInit(faacDecHandle hDecoder,
                            BYTE *buffer,
                            uint32_t buffer_size,
                            uint32_t *samplerate,
                            BYTE *channels);

/* Init the library using a DecoderSpecificInfo */
int8_t FAADAPI faacDecInit2(faacDecHandle hDecoder, BYTE *pBuffer,
                         uint32_t SizeOfDecoderSpecificInfo,
                         uint32_t *samplerate, BYTE *channels);

void FAADAPI faacDecClose(faacDecHandle hDecoder);

void* FAADAPI faacDecDecode(faacDecHandle hDecoder,
                            faacDecFrameInfo *hInfo,
                            BYTE *buffer,
                            uint32_t buffer_size);

#endif
