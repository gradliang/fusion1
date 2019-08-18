/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_mp3.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   09/20/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     09/20/2006    Rebecca Hsieh    first file
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

extern Audio_dec  Media_data;
extern int left_channel;
extern int right_channel;

#if AC3_AUDIO_ENABLE
#include "ac3_lib.h"
extern enum AO_AV_TYPE AO_AV;
static int fending;
extern enum FW_BW_TYPE FW_BW;
extern void * BitStream_Source;
#endif

static ad_info_t info = {
	"Magic Pixel mpeg audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on E360...."
};
static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static void uninit(sh_audio_t * sh);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

///@defgroup    ac3 Decode API
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
#if(AC3_AUDIO_ENABLE)
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
#if(AC3_AUDIO_ENABLE)
	unsigned char *buf = sh->a_buffer;	
	int ret = 0;
	char * song_singer, *song_name,*song_album,*song_comment,*song_genre,*song_year,*song_track;

	BitStream_Source=(void*)sh;

#if AC3_AUDIO_ENABLE
	fending = 0;
	if (AO_AV == AO_TYPE)
#else
	if (!sh)
#endif
    {
		Media_data.sdram_buf = (int *)mem_malloc(300 * 1024);
		sdbuf_left = 0;
		Media_data.sdram_buf += 0x20000000;
		ret = MagicPixel_ac3_init(&Media_data.ch, &Media_data.srate, &Media_data.frame_size, &Media_data.bitrate, &Media_data.total_time, Media_data.file_size);

		sh->channels = Media_data.ch;
		sh->samplerate = Media_data.srate;
		sh->i_bps = Media_data.bitrate ;
		sh->samplesize = 2;
    }
	else
	{
		memset(&Media_data, 0, sizeof( Audio_dec));
		if (sh->wf)
		{
			sh->channels = sh->wf->nChannels;
			if (sh->channels > 2)
				sh->channels = 2;
			sh->samplerate = sh->wf->nSamplesPerSec;
			sh->i_bps = sh->wf->nAvgBytesPerSec * 8;
			sh->samplesize = 2;
		}
		else
		{
			ret = MagicPixel_ac3_init(&Media_data.ch, &Media_data.srate, &Media_data.frame_size, &Media_data.bitrate, &Media_data.total_time, Media_data.file_size);
			sh->channels = Media_data.ch;
			sh->samplerate = Media_data.srate;
			sh->i_bps = Media_data.bitrate ;
			sh->samplesize = 2;
			
		}
		ret = 0;	// Set initialization success at AV case
	}

	return ret;
#else
	return -1;
#endif
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


static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
#if(AC3_AUDIO_ENABLE)
	int ret;
	unsigned char *EQ_band;
    char * song_singer, *song_name,*song_album,*song_comment,*song_genre,*song_year,*song_track;
    int len = 0;
	
    if (fending) return len ? len : -1;
        
    while (len < minlen )  // && len + 4608 <= maxlen)
	{
#if(AC3_AUDIO_ENABLE)
		if (AO_AV==AV_TYPE)
#else
		if (!sh)
#endif
		{
			if(Media_data.frame==0)
			{
				mClrIntsram0Cks ();
				mClrIntsram1Cks ();
				mSetIntsram0Cks (INTSRAM2CKS_CPU_CLK);
				mSetIntsram1Cks (INTSRAM2CKS_CPU_CLK);
				MP_DEBUG4("Media_data.ch=%d,Media_data.srate=%d,Media_data.frame_size=%d,Media_data.bitrate=%d",Media_data.ch,Media_data.srate,Media_data.frame_size,Media_data.bitrate);
				ret = MagicPixel_ac3_init(&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,Media_data.file_size);
				MP_DEBUG1("ret=0x%x",ret);
			}
			ret= MagicPixel_ac3_decode(&Media_data.frame_size,Media_data.pcm4_buf,&Media_data.ch,&Media_data.play_time,MagicPixel_ac3_getposition_callback()+1);

			if(ret)//eof
			{
				MP_DEBUG("ret1=0x%x",ret);
				fending=1;
				break;
			}

			Media_data.frame++;
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);
			MagicPixel_ac3_post_process(0,0,Media_data.frame_size,Media_data.ch,Media_data.pcm4_buf, (short*)buf);

			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		} 
		else
		{
			ret= MagicPixel_ac3_decode(&Media_data.frame_size,Media_data.pcm4_buf,&Media_data.ch,&Media_data.play_time,Media_data.file_size);
			if(ret)//eof
			{ 
				fending=1;
				break;
			}

			Media_data.frame++;
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);
			MagicPixel_ac3_post_process(0,0,Media_data.frame_size,Media_data.ch,Media_data.pcm4_buf, (short*)buf);
			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}

		TaskYield();
   }
     
	return len ? len : -1;
#else
	return 0;
#endif
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
#if AC3_AUDIO_ENABLE
	if (AO_AV==AO_TYPE)
#else
	if (!sh)
#endif
		return 0;
	else
	{	// various optional functions you MAY implement:
		switch (cmd)
		{
		case ADCTRL_RESYNC_STREAM:
			return CONTROL_TRUE;
		case ADCTRL_SKIP_FRAME:
			return CONTROL_TRUE;
		}

		return CONTROL_UNKNOWN;
	}
}

static int resync(unsigned int second)
{
	int swRet = MagicPixel_ac3_resync(Media_data.file_size, second);
	MagicPixel_ac3_resync_post_function();

	return swRet;
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	extern Audio_dec Media_data;

	mpDebugPrint("AC3 uninit");
	//Because we use AC3 buffer with strange way, we shoud free it here
	//This strange way should be corrected
	mpDebugPrint("free ac3 memory");
	if(Media_data.sdram_buf){
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}
	if(Media_data.sdram_buf2){
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
}

ad_functions_t software_ac3 = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

