/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_ram.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   10/23/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     10/23/2006    Rebecca Hsieh    first file
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

#if RAM_AUDIO_ENABLE
#include "ram_lib.h"
extern int left_channel;
extern int right_channel;

static int fending;


extern enum AO_AV_TYPE AO_AV;

static ad_info_t info = {
	"Magic Pixel RAM audio decoder",
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

///@defgroup   RAM Decode API
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
	if (AO_AV==AO_TYPE)
	{
		sh->audio_out_minsize = 2 * 4608;
		sh->audio_in_minsize = 4096;
	}

	return 1;
}
static int flag_RM;

static int init(sh_audio_t * sh)
{
#if RAM_AUDIO_ENABLE
	unsigned char *buf = sh->a_buffer;	
	int ret;
	char * song_singer, *song_name,*song_album,*song_comment,*song_genre,*song_year,*song_track;
	flag_RM = 0;
	if (AO_AV == AO_TYPE)
	{
		Media_data.sdram_buf = (BYTE *) mem_malloc(SDRAM_SIZE_RAM);

		ret = MagicPixel_ram_init(&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,
										&Media_data.total_time,&song_singer,&song_name, Media_data.sdram_buf, SDRAM_SIZE_RAM);

		sh->channels = Media_data.ch;
		sh->samplerate = Media_data.srate;
		sh->i_bps = Media_data.bitrate ;
		sh->samplesize = 2;
		Media_data.fending = 0;
		Media_data.frame = 0;
		fending = 0;
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
#if RAM_AUDIO_ENABLE
	int ret;
	int len = 0;

	if (fending==1) return len ? len : -1;
//mpDebugPrint("\nRAM audio decoder min: %d, max: %d", minlen, maxlen);
	//while( (len < minlen )&& ( len + 8192 < maxlen))
	while( (len < minlen ))
	{
		if (AO_AV == AO_TYPE)
		{
//UartOutText("Start ram decoding ...");
			ret = MagicPixel_ram_decode(&Media_data.frame_size, Media_data.pcm4_buf, &Media_data.play_time, 0, NULL);

//mpDebugPrint("K frameSize: %d, channel: %d, pcmBuf: %x, playTime: %d, total len: %d",
				//Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, Media_data.play_time, len);
//__asm("break 100");
			Media_data.frame++;
			if(ret) //eof
			{
//__asm("break 100");			
//mpDebugPrint("RAM decode fail case...");
				//if (flag_RM == 10)
					fending = 1;
				//flag_RM++;
				//len = 1;
				break;
			}	

			sh->ds->eof = 0;
//UartOutText("	post..");
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel, right_channel);
			MagicPixel_ram_post_process(0, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, buf);
	
//UartOutText("	..End\n");
			len += 2 * Media_data.ch * Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch * Media_data.frame_size;
		}
		//break;
		TaskYield();
	}
	//mpDebugPrint("len %d",len);

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
	return 0;
}

static int resync(unsigned int second)
{
	Media_data.fending = 0;
	fending = 0;
	return MagicPixel_ram_resync(&second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	mpDebugPrint("free ram memory");

	if(Media_data.sdram_buf){
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}
	if(Media_data.sdram_buf2){
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
}

ad_functions_t software_ram = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

#endif

