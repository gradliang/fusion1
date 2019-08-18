/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_wma.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   08/15/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     08/15/2006    Rebecca Hsieh    first file
****************************************************************
*/
///@mainpage Libmad API Use Guaid
///
///This control audio playing. Mp61X contains wrapper for every codecs, some of them include the
///codec function implementation, some calls functions from other files, some calls optional external libraries.
///
///AVSync model doesn't call them directly, but through the decoder_audio.c and video_decoder.c files, 
///so the AVSync doesn't have to know anything about the codecs.


/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"
#include "avTypeDef.h"
#include "debug.h"

#if WMA_ENABLE
#include "wma_lib.h"
#include "asf_lib.h"
extern enum AO_AV_TYPE AO_AV;
extern Audio_dec  Media_data;
extern int left_channel;
extern int right_channel;

#endif

extern void * BitStream_Source;

static int fending;
static ad_info_t info = {
	"Magic Pixel WMA audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on S250...."
};
static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static void uninit(sh_audio_t * sh);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);


///@defgroup    WMA Decode API
///This control audio playing

///@ingroup Magic Pixel
///@defgroup INIT   Initialization

///@ingroup INIT
///@brief   Init the audio system. Allocate space for audio decoder and 
///         set the minimize size of buffers for both input and output audio data. 
///
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.             
///
///@return  static int                  Return 0 for fail and 1 for success.
static int preinit(sh_audio_t * sh)
{
#if(WMA_ENABLE )
	if (AO_AV==AO_TYPE)
#else
	if (!sh)
#endif
	{
		sh->audio_out_minsize = 2 * 4608;
		sh->audio_in_minsize = 4096;
	}
	else
	{
 		sh->audio_out_minsize = 2 * 4608;
		sh->audio_in_minsize = 4096;
 	}

	return 1;
}

static int init(sh_audio_t * sh)
{
#if WMA_ENABLE
	int ret = 0;
	char *song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, *song_track;
	unsigned int tmpWMA_imgSize, tmpWMA_imageOffset;	//You should implement those two variables what you want
	WORD * Enopt;
	BYTE *extra_data;

	BitStream_Source = (void*)sh;

	if (AO_AV == AO_TYPE)
	{
		ret = MagicPixel_ASF_AO_Init(&Media_data.ch, &Media_data.srate, &Media_data.bitrate, &Media_data.total_time,
											&song_singer, &song_name, &song_album, &song_genre, &song_year, &song_track, 0,
											&tmpWMA_imgSize, &tmpWMA_imageOffset);

		sh->channels   = Media_data.ch;
		sh->samplerate = Media_data.srate;
		sh->i_bps      = Media_data.bitrate ;
		sh->samplesize = 2;
		Media_data.fending = 0;
	}
	else
	{
		memset(&Media_data, 0, sizeof( Audio_dec));
		Media_data.ch = sh->channels =((WAVEFORMATEX*)sh->wf)->nChannels;
		if(sh->channels > 2)
			sh->channels = 2;

		Media_data.srate   = sh->samplerate =((WAVEFORMATEX*)sh->wf)->nSamplesPerSec;
		Media_data.bitrate = sh->i_bps = ((WAVEFORMATEX*)sh->wf)->nAvgBytesPerSec * 8 ;
		sh->samplesize = 2;

		if (sh->wf && sh->wf->cbSize > 0)
		{
			extra_data=(BYTE *) mem_malloc(((WAVEFORMATEX*)sh->wf)->cbSize);
			memcpy(extra_data, (char *) sh->wf + sizeof(WAVEFORMATEX), ((WAVEFORMATEX*)sh->wf)->cbSize);

			Enopt =(WORD *)(extra_data+4);

			ret = MagicPixel_WMA_Init(sh->format, sh->samplerate, sh->channels, sh->i_bps>>3,
					((WAVEFORMATEX*)sh->wf)->nBlockAlign, *Enopt, AV_WMA_bitstream_callback,AV_WMA_newblock_callback); 
			mem_free(extra_data);
		}
		else
		{
			UartOutText("no Enopt value\r\n");
			ret = MagicPixel_WMA_Init(sh->format, sh->samplerate, sh->channels, sh->i_bps>>3,
				((WAVEFORMATEX*)sh->wf)->nBlockAlign, 0, AV_WMA_bitstream_callback, AV_WMA_newblock_callback);
		}
	}

	fending = 0;

	return ret;
#endif

	return -1;
}

///@ingroup Magic Pixel
///@defgroup DECODE   Decode

///@ingroup DECODE
///@brief       Decode the compress data from demux into audio_out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   unsigned char *buf  Pointer to the audio_out buffer 
///@param   int minlen      The max length of decoded audio out data
///@param   int maxlen      The max length of decoded audio out data ??what's diff???
///@return  The length of actual decoded audio data, if success
///         -1, if failed
unsigned char EQ_band_array[16];

static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
	int ret;
	int len = 0;
#if WMA_ENABLE
	if(!Media_data.fending)
		fending = 0;
	if(fending)
		return len ? len : -1;

	while( (len < minlen ) && ( len + 8192 < maxlen) )
	{
		if (AO_AV == AO_TYPE)
		{
			ret = MagicPixel_ASF_AO_Decode(&Media_data.frame_size, Media_data.pcm4_buf2, &Media_data.play_time, 1);
			if(ret)
			{
				mpDebugPrint("ASF error code : %d", ret);
				Media_data.fending = 1;
				fending = 1;
				break;
			}
			if (Media_data.ch >= 2)
            	MagicPixel_audio_channel_process(left_channel,right_channel);
			MagicPixel_ASF_AO_Post_Process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf2, buf);
			MagicPixel_ASF_AO_Post_Decode(Media_data.frame_size);

			//mpDebugPrint("WMA frame_Size: %d", Media_data.frame_size);

			MagicPixel_ASF_AO_EQ_Process(&EQ_band_array[0]);
			AUDIO_EQ_INFO = &EQ_band_array[0];

			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}
		else
		{
			ret= MagicPixel_WMA_Decode (&Media_data.frame_size,0,Media_data.pcm4_buf2);

			if(ret != 0 && ret != 4)//eof
			{ 
				Media_data.fending = 1;
				fending = 1;
				break;
			}
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);
			MagicPixel_WMA_Post_Process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf2, buf);
			MagicPixel_WMA_Post_Decode(&Media_data.frame_size);

			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}

		TaskYield();
	}
#endif
	return len ? len : -1;
}

///@ingroup Magic Pixel
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
#if WMA_ENABLE
	if (AO_AV==AO_TYPE)
#else
	if (!sh)
#endif
	return 0;
}

static int resync(unsigned int second)
{
	return MagicPixel_ASF_AO_resync(second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
#if WMA_ENABLE
	left_channel = 1;
	right_channel = 1;
#endif	
}

ad_functions_t software_wma = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

