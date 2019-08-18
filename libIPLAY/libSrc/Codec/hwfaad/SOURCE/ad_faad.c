/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      ad_faad.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
///@mainpage Faad API Use Guaid
///
///This control audio playing. Mp61X contains wrapper for every codecs, some of them include the
///codec function implementation, some calls functions from other files, some calls optional external libraries.
///
///AVSync model doesn't call them directly, but through the decoder_audio.c and video_decoder.c files, 
///so the AVSync doesn't have to know anything about the codecs.


#include "global612.h"
//#include "ad_internal.h"
#include "mpc_info.h"
#include "avTypeDef.h"
#include "faad.h"


static ad_info_t info = {
	"AAC (MPEG2/4 Advanced Audio Coding)",
	"faad",
	"Felix Buenemann",
	"faad2",
	"uses libfaad2"
};

//LIBAD_EXTERN(faad)

static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static void uninit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

ad_functions_t hw_mpcodecs_ad_faad = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio
};

#define FAAD_BUFFLEN (FAAD_MIN_STREAMSIZE*MAX_CHANNELS)


///@defgroup    FAAD    FAAD API
///This control audio playing

///@ingroup FAAD
///@defgroup INIT   Initialization

///@ingroup INIT
///@brief   Init the audio system.
///
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.             
///
///@return  static int                  Return 0 for fail and 1 for success.
static int preinit(sh_audio_t * sh)
{
	sh->audio_out_minsize = 2048 * MAX_CHANNELS;
	sh->audio_in_minsize = FAAD_BUFFLEN;
	return 1;
}

///@ingroup INIT
///@brief   The init of driver, opens device, sets sample rate, channels, sample format  parameters.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.
///@return  static int                  Return 0 for fail and 1 for success.
static int init(sh_audio_t * sh)
{
	unsigned long faac_samplerate;
	unsigned char faac_channels;
	int faac_init;
	faacDecHandle this = faacDecOpen();

	sh->context = this;

//	MPAInit(1);		//---> This will due to some problem, but we do not use hardware AAC now	C.W 081001
					//---> If we want to fix this problem, we may need to revise makefile about hwmp3 part

	// If we don't get the ES descriptor, try manual config
	if (!sh->codecdata_len)
	{
		sh->a_in_buffer_len = demux_read_data(sh->ds, sh->a_in_buffer, sh->a_in_buffer_size);
		faac_init = faacDecInit(this, sh->a_in_buffer,
								sh->a_in_buffer_len, &faac_samplerate, &faac_channels);
		sh->a_in_buffer_len -= (faac_init > 0) ? faac_init : 0;	// how many bytes init consumed
	}
	else
	{							// We have ES DS in codecdata
		faac_init = faacDecInit2(this, sh->codecdata,
								 sh->codecdata_len, &faac_samplerate, &faac_channels);
	}
	if (faac_init < 0)
	{
		faacDecClose(this);
		return 0;
	}
	else
	{
		sh->channels = faac_channels;
		sh->samplerate = faac_samplerate;
		if (!sh->i_bps)
		{
			sh->i_bps = 128 * 1000 / 8;
		}
	}
	return 1;
}

///@ingroup FAAD
///@defgroup DECODE   Decode

///@ingroup DECODE
///@brief       Decode the compress data from demux into audio_out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   unsigned char *buf  Pointer to the audio_out buffer 
///@param   int minlen      The max length of decoded audio out data
///@param   int maxlen      The max length of decoded audio out data ??what's diff???
///@return  The length of actual decoded audio data, if success
///         -1, if failed

DWORD DsFillBuf_T = 0;
static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
	int j = 0, len = 0;
	void *faac_sample_buffer;
	faacDecStruct *this = (faacDecStruct *) sh->context;

//Fix.2005.06.06 Brenda, ADTS Playback
	while (len < minlen && sh->ds->eof != 1)
	{
		/* update buffer for raw aac streams: */
		if (!sh->codecdata_len)
			if (sh->a_in_buffer_len < sh->a_in_buffer_size)
			{
				sh->a_in_buffer_len +=
					demux_read_data(sh->ds, &sh->a_in_buffer[sh->a_in_buffer_len],
									sh->a_in_buffer_size - sh->a_in_buffer_len);
			}

		if (!sh->codecdata_len)
		{
			do
			{
				faac_sample_buffer =
					faacDecDecode(this, &this->faac_finfo, sh->a_in_buffer + j,
								  sh->a_in_buffer_len);

				/* update buffer index after faacDecDecode */
				if (this->faac_finfo.bytesconsumed >= sh->a_in_buffer_len)
				{
					sh->a_in_buffer_len = 0;
				}
				else
				{
					sh->a_in_buffer_len -= this->faac_finfo.bytesconsumed;
					memcpy(sh->a_in_buffer, &sh->a_in_buffer[this->faac_finfo.bytesconsumed],
						   sh->a_in_buffer_len);
				}

				if (this->faac_finfo.error > 0)
				{
					j++;
				}
				else
					break;
			}
			while (j < FAAD_BUFFLEN);
		}
		else
		{
			unsigned char *bufptr = NULL;
			DWORD Dtick, Dtpc, Dtmv;

			get_cur_timeL(&Dtick, &Dtpc, &Dtmv);
			int buflen = ds_get_packet(sh->ds, &bufptr);
			TaskYield();

			DsFillBuf_T += get_elapsed_timeL(Dtick, Dtpc, Dtmv);
			if (buflen <= 0)
				break;
			faac_sample_buffer = faacDecDecode(this, &this->faac_finfo, bufptr, buflen);
			TaskYield();
		}

		if (this->faac_finfo.error <= 0 && this->faac_finfo.samples != 0)
		{
			memcpy(buf + len, faac_sample_buffer, sh->samplesize * this->faac_finfo.samples);
			len += sh->samplesize * this->faac_finfo.samples;
		}
		TaskYield();
	}

	return len?len:-1;  //05.23.2006 Rebecca
}


///@ingroup FAAD
///@defgroup CON   Control

///@ingroup CON
///@brief           Decode the compress data from demux into play out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   int cmd         Type of command
///@param   void *arg           Pointer to argument, not yet used
///@return  CONTROL_OK for success
///     CONTROL_UNKNOW for fail
///@remark  This is for reading/setting special parameters and can be used for keyboard input for example.  
///         control codes are found in audio_decoder.h.
static int control(sh_audio_t * sh, int cmd, void *arg, ...)
{
	switch (cmd)
	{
#if 0
	case ADCTRL_RESYNC_STREAM:
		return CONTROL_TRUE;
	case ADCTRL_SKIP_FRAME:
		return CONTROL_TRUE;
#endif
	}
	return CONTROL_UNKNOWN;
}


///@ingroup FAAD
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system, this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	faacDecHandle this = (faacDecHandle) sh->context;

	faacDecClose(this);
}
