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
*                MP S250 division
*
* Created:   07/18/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     07/18/2006    Rebecca Hsieh    first file
****************************************************************
*/
///@mainpage Libmad API Use Guaid
///
///This control audio playing. Mp6xx contains wrapper for every codecs, some of them include the
///codec function implementation, some calls functions from other files, some calls optional external libraries.
///
///AVSync model doesn't call them directly, but through the decoder_audio.c and video_decoder.c files, 
///so the AVSync doesn't have to know anything about the codecs.


/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define OUTPUT_PCM  0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"
#include "avTypeDef.h"
#include "debug.h"

#if MP3_SW_AUDIO
#include "mp3_lib.h"
extern enum AO_AV_TYPE AO_AV;
extern Audio_dec  Media_data;
extern int left_channel;
extern int right_channel;
static int fending;
extern DWORD audiostream_finish;
int MP3_for_FLV;
#endif

extern void * BitStream_Source;

#if OUTPUT_PCM
static int enter_first = 0;
static DRIVE *sDrv;
static STREAM *shandle;
static int ret;
static unsigned char *tmp_buf;
static int buffer_num;
#endif
typedef struct MP3_FIXBUG_VAR
{
	BYTE mp3layer;
	BYTE ERROR_CONTROL;
	short decode_fail_count;
}MP3_FIXBUG_VAR;

MP3_FIXBUG_VAR mp3_bug_var;

#define INIT_COUNT 4
#define FAIL_DEC_MAX 16
static ad_info_t info = {
	"Magic Pixel mpeg audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on S250...."
};

///@defgroup    MP3 Decode API
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
#if MP3_SW_AUDIO
	sh->audio_out_minsize = 2 * 4608;
	sh->audio_in_minsize = 4096;
#endif
     
	return 1;
}

static int init(sh_audio_t * sh)
{
	MP_DEBUG("Soft_mp3_init()");
#if MP3_SW_AUDIO
	unsigned char *buf = sh->a_buffer;	
	int ret = 0, mpeg2, layer;
	char *song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, *song_track;

	BitStream_Source = (void*)sh;
    fending = 0;
	mp3_bug_var.decode_fail_count = 0;//For AV type
    if (AO_AV == AO_TYPE)
    {
		Media_data.frame = 0;
		Media_data.fending = 0;
		ret = MagicPixel_mp3_init(Media_data.file_size, 0, &Media_data.ch, &Media_data.srate, &Media_data.frame_size, &Media_data.bitrate, &Media_data.total_time,
									&song_singer, &song_name, &song_comment, &song_album, &song_genre, &song_year, &song_track, &mpeg2, &layer);

		mp3_bug_var.mp3layer = layer;
		sh->channels   = Media_data.ch;
		sh->samplerate = Media_data.srate;
		sh->i_bps      = Media_data.bitrate ;
		sh->samplesize = 2;
		mp3_bug_var.ERROR_CONTROL = 1;
    }
    else
    {
		memset(&Media_data, 0, sizeof( Audio_dec));
		if (sh->wf)
		{
			sh->channels	= sh->wf->nChannels;
			sh->samplerate	= sh->wf->nSamplesPerSec;
			sh->i_bps		= sh->wf->nAvgBytesPerSec * 8 ;
		}

		ret = MagicPixel_mp3_init(Media_data.file_size, 0, & Media_data.ch,& Media_data.srate,& Media_data.frame_size,& Media_data.bitrate,& Media_data.total_time,
														&song_singer,&song_name,&song_comment,&song_album,&song_genre,&song_year,&song_track,&mpeg2,&layer);		

		if ( !sh->channels || ! sh->samplerate || !sh->i_bps)
		{
			
			sh->channels	= Media_data.ch;
			sh->samplerate	= Media_data.srate;
			sh->i_bps		= Media_data.bitrate;
			//mpDebugPrint("Channel: %d, Sample rate: %d, Bitrate: %d", sh->channels, sh->samplerate, sh->i_bps);
		}	
		Media_data.fending = 0;
		Media_data.frame = 0;
		sh->samplesize = 2;
		mp3_bug_var.ERROR_CONTROL = 0;
	}
	
#if OUTPUT_PCM
	UartOutValue(sh->channels, 8);
	UartOutText("<-ch\r\n");
	UartOutValue(sh->samplerate, 8);
	UartOutText("<-srate\r\n");
	UartOutValue(sh->i_bps, 8);
	UartOutText("<-sh->i_bps\r\n");
	buffer_num = 0;
#endif

	return ret;
#else
	return -1;
#endif
}

///@ingroup Magic Pixel
///@defgroup DECODE   Decode

///@ingroup DECODE
///@brief   Decode the compress data from demux into audio_out buffer at a certain length
///@param   sh_audio_t *sh  Pointer to the stream header structure used
///@param   unsigned char *buf  Pointer to the audio_out buffer 
///@param   int minlen      The max length of decoded audio out data
///@param   int maxlen      The max length of decoded audio out data ??what's diff???
///@return  The length of actual decoded audio data, if success
///         -1, if failed


static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
#if MP3_SW_AUDIO
	int ret,mpeg2,layer;
	int ch,srate,frame_size,total_time,bitrate;
    char *song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, song_track,wavHeader[44];
	int len = 0;
	static int init_count=0;
#if OUTPUT_PCM
	tmp_buf = buf;
	if(!enter_first)
	{
		enter_first = 1;
		sDrv = DriveGet(SD_MMC);
		ret = CreateFile(sDrv, "mp3", "pcm");
		if (ret) UartOutText("create file fail\r\n");
		shandle = FileOpen(sDrv);
		if(!shandle) UartOutText("open file fail\r\n");
	}
#endif
//	if (audiostream_finish == 0)
//	{
//		Media_data.fending = 0;
//		fending = 0;
//	}
	if(Media_data.fending == 0)
		fending = 0;
#if MP3_SW_AUDIO
    if (fending) return len ? len : -1;
#endif
        
	while (len < minlen )  // && len + 4608 <= maxlen)
	{
#if MP3_SW_AUDIO
		if (AO_AV == AV_TYPE)
#else
		if (!sh)
#endif
		{
			//Fix the issue that aMagicPixel_mp3_initvsync fails in playing "*.flv" .Mark by XW 2010/01/20
			//Just temporarily call init function twice,we will solve it ASAP.Note by XianWen 2010/01/25.
//			if(Media_data.frame == 0 && MP3_for_FLV == 0)
//			{
//				MagicPixel_mp3_init(Media_data.file_size,0,& Media_data.ch,& Media_data.srate,& Media_data.frame_size,& Media_data.bitrate,& Media_data.total_time,
//									&song_singer,&song_name,&song_comment,&song_album,&song_genre,&song_year,&song_track,&mpeg2,&layer);
//			}

			ret = MagicPixel_mp3_decode(mp3_bug_var.ERROR_CONTROL, 0, Media_data.pcm4_buf, &Media_data.play_time, &AUDIO_EQ_INFO, &Media_data.ch);
			
			if(ret)//eof
			{
				mpDebugPrint("ret %d\n",ret);
				//Because ret may not be 0xe in the end of audio stream,we add a  Media_data.fending condtion
				if(ret == 0xe || Media_data.fending == 1)
				{
					audiostream_finish = 1;
					fending = 1;
					init_count = 0;
				}
				if (ret != 0xe)
					mp3_bug_var.decode_fail_count++;

				if (mp3_bug_var.decode_fail_count == 1)
				{
					ret = MagicPixel_mp3_init(Media_data.file_size, 0, &ch,&srate,&frame_size,&bitrate,&total_time,
														NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
					init_count ++;
					if (init_count == INIT_COUNT)
						mp3_bug_var.ERROR_CONTROL = 1;
				}
				
				if (mp3_bug_var.decode_fail_count >= FAIL_DEC_MAX || ret == FAIL)
				{
					Media_data.fending = 1;
					fending = 1;
					init_count = 0;
					MP_ALERT("MP3 stream may be multi-layer, MPX mp3 decoder does not support it");
				}
				break;
			}
			else
				mp3_bug_var.decode_fail_count = 0;

			Media_data.frame++;
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);

			MagicPixel_mp3_post_process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, (short*)buf);           
			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}
		else
		{
			ret = MagicPixel_mp3_decode(mp3_bug_var.ERROR_CONTROL, EQUALIZER_EN, Media_data.pcm4_buf, &Media_data.play_time, &AUDIO_EQ_INFO, &Media_data.ch);
			if(ret)//eof
			{
				if (mp3_bug_var.mp3layer == 2 ||(ret != 0 && mp3_bug_var.mp3layer == 3))
					Media_data.fending = 1;
				fending = 1;
				break;
			}
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);
			MagicPixel_mp3_post_process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, (short*)buf);
			Media_data.frame++;
			len += 2 * Media_data.ch * Media_data.frame_size;	// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}
		if (g_bAniFlag & ANI_SLIDE)
		{
			if(Media_data.frame % 3 == 0)	// Reduce task yield times
				TaskYield();
		}
		else
		{
			TaskYield();
		}
	}

#if OUTPUT_PCM
	if(len){
		if (buffer_num < 200)
			ret=FileWrite(shandle, tmp_buf, len);
		else
		{
			if (shandle)
			{
				FileClose(shandle);
				UartOutText("file close\r\n");
				shandle = NULL;
			}
		}
		if(!ret) UartOutText("write file fail\r\n");
			buffer_num++;
	}
	else
	{
		if(fending)
		{
			if (shandle)
			{
				mpDebugPrint("file close");
				FileClose(shandle);
			}
		}
	}
#endif
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
#if MP3_SW_AUDIO
	if (AO_AV == AO_TYPE)
#else
	if (!sh)
#endif
		return 0;
	else
	{
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
	return MagicPixel_mp3_resync(0, second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	extern DWORD audio_device_pts;
	audio_device_pts = 0;

#if OUTPUT_PCM
	if (!shandle)
		FileClose(shandle);
#endif
}

ad_functions_t software_mp3 = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

