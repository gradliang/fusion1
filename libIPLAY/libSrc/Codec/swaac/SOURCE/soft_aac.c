/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_aac.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   08/03/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     08/03/2006    Rebecca Hsieh    first file
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

#if( AAC_SW_AUDIO)
#include "aac_lib.h"
extern enum AO_AV_TYPE AO_AV;
extern int left_channel;
extern int right_channel;
extern void * BitStream_Source;
int BS_len;
#endif

static ad_info_t info = {
	"Magic Pixel aac audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on E360...."
};

static int fending;
static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static void uninit(sh_audio_t * sh);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

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
#if( AAC_SW_AUDIO)
	sh->audio_out_minsize = 2048 * 2;
	sh->audio_in_minsize = 768 * 2;	//FAAD_MIN_STREAMSIZE(6144 bits/channel)*MAX_CHANNELS
#endif

	return 1;
}

static int init(sh_audio_t * sh)
{
//	mpDebugPrint("(Soft aac init)Channels: %d, Sample rate: %d, Media bit rate: %d", sh->channels, sh->samplerate, Media_data.bitrate);

	int ret = 0;
	char *song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, *song_track;
	int total_frame, header_type;

#if AAC_SW_AUDIO
	if (AO_AV == AO_TYPE)
    {
		Media_data.sdram_buf = (int *) mem_malloc(SDRAM_SIZE_AAC);
		Media_data.sdram_buf = (int *)((DWORD)Media_data.sdram_buf | 0x20000000);
		ret = MagicPixel_mp4_init(Media_data.file_size, &Media_data.srate, &Media_data.ch, &Media_data.bitrate, &Media_data.total_time,
										&total_frame, &header_type, Media_data.sdram_buf, SDRAM_SIZE_AAC,
										&song_singer, &song_name, &song_comment, &song_album, &song_genre,
										&song_year, song_track, 0);  //0 is for decode

		Media_data.frame = 0;
		Media_data.fending = 0;
		sh->channels = Media_data.ch;
		sh->samplerate = Media_data.srate;
		sh->i_bps = Media_data.bitrate;
    }
    else
    {
		if (!sh->codecdata_len)
		{
			UartOutText("AV AAC init failed,sh->codecdata_len=0;\r\n");
			//if this case happened, need to add and re-write the following.
			//sh->a_in_buffer_len = demux_read_data(sh->ds, sh->a_in_buffer, sh->a_in_buffer_size);
			//faac_init = faacDecInit(this, sh->a_in_buffer,
			//					sh->a_in_buffer_len, &faac_samplerate, &faac_channels);
			//sh->a_in_buffer_len -= (faac_init > 0) ? faac_init : 0;	// how many bytes init consumed
		}
		else
		{	// We have ES DS in codecdata
			Media_data.fending = 0;
			ret = MagicPixel_avAAC_init(sh->codecdata, sh->codecdata_len, &Media_data.srate, &Media_data.ch);
			Media_data.frame = 0;
			sh->channels = Media_data.ch;
			sh->samplerate = Media_data.srate;
			sh->i_bps = Media_data.bitrate = (128 * 1000) >> 3;

			//mpDebugPrint("(Soft aac init)Channels: %d, Sample rate: %d, BitPerSec: %d", sh->channels, sh->samplerate, sh->i_bps);
		}
    }
	mpDebugPrint("Media_data.total_time %d",Media_data.total_time);
    fending = 0;
#endif
	return ret;
}

///@ingroup		Magic Pixel
///@defgroup	DECODE	Decode

///@ingroup		DECODE
///@brief		Decode the compress data from demux into audio_out buffer at a certain length
///@param		sh_audio_t *sh  Pointer to the stream header structure used
///@param		unsigned char *buf  Pointer to the audio_out buffer 
///@param		int minlen      The max length of decoded audio out data
///@param		int maxlen      The max length of decoded audio out data ??what's diff???
///@return		The length of actual decoded audio data, if success
///				-1, if failed

static int decode_audio(sh_audio_t * sh, unsigned char *buf, int minlen, int maxlen)
{
	int ret, eq_enable;
	unsigned char *EQ_band;
	unsigned char * bufptr;
	int buflen;
    int len = 0,value1, value2,i;
#if( AAC_SW_AUDIO)
	if (fending) return len ? len : -1;
#endif

	while (len < minlen )  // && len + 4608 <= maxlen)
	{
#if(AAC_SW_AUDIO)
		if (AO_AV == AV_TYPE)
#else
		if (!sh)
#endif
		{
#if AAC_SW_AUDIO
			bufptr = (unsigned char *)NULL;
			buflen = ds_get_packet(sh->ds, &bufptr);
			BitStream_Source = (void *)bufptr;
			BS_len = buflen;

			if (buflen <= 0)
				break;
			value1 = 0;
			value2 = 0;
			//Media_data.frame = 0;
			for(i=0;i<buflen/4;i++)
				value1 += *(bufptr+buflen-1-i);
			value2 = *(bufptr+ 0) + *(bufptr+ 1) + *(bufptr+ 2) + *(bufptr+ 3);
			if (value1 != 0 && value2 !=0)
			{
				MagicPixel_AAC_fill_frame(buflen);	//What is this line doing?
				ret = MagicPixel_aac_decode(eq_enable, Media_data.pcm4_buf, &EQ_band, &Media_data.ch);
				if(ret)//eof
				{
					if ((ret == 0x200000)||(ret == 0x30000))
						fending = 1;
					break;	
				}
				Media_data.frame++;
				if (Media_data.ch >= 2)
					MagicPixel_audio_channel_process(left_channel,right_channel);
				MagicPixel_AAC_post_process(Media_data.ch - 1, 0, 1024, Media_data.ch, Media_data.pcm4_buf, buf);           

				len += 2 * (Media_data.ch << 10);// short * channel * frame_size
				buf += 2 * (Media_data.ch << 10);
			}
			else
			{
				MP_ALERT("The Audio chunk has garbages");
			}
#endif	
       } 
       else
       {
#if( AAC_SW_AUDIO)
			//Media_data.frame=0;
			ret = MagicPixel_mp4_decode(EQUALIZER_EN, Media_data.pcm4_buf, &Media_data.play_time, &AUDIO_EQ_INFO, &Media_data.ch, &Media_data.frame_size, &Media_data.fending);

			////////////////////////sample rate needs to double for heaac
			//if((Media_data.ch & 0x10) == 0x10)
			//{
			//	Media_data.ch &= 0x0f;
			//	if(Media_data.frame == 0)
			//	{
			//		Media_data.srate *= 2;
			//		mpDebugPrint("srate:%d",Media_data.srate);
			//		mpDebugPrint("ch:%d",Media_data.ch);
			//		MX6xx_AudioConfig((WORD) Media_data.srate, Media_data.ch, 16);
			//	}
			//}
			////////////////////////
			
			if(Media_data.frame_size == 0)
			   continue;
			
			if(ret && Media_data.fending == 0x1)//eof
			{
			   fending=1;
			   break;
			}
			if (Media_data.ch >= 2)
				MagicPixel_audio_channel_process(left_channel,right_channel);
                  
			MagicPixel_AAC_post_process(Media_data.ch-1, 0, Media_data.frame_size, Media_data.ch, Media_data.pcm4_buf, buf);
			Media_data.frame++;
			len += 2 * Media_data.ch *Media_data.frame_size;// short * channel * frame_size
			buf += 2 * Media_data.ch *Media_data.frame_size;
			if (Media_data.fending == 0x1)
			{
				fending = 1;
				break;
			}
#endif
       }

       TaskYield();
    }

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
#if( AAC_SW_AUDIO)
	if (AO_AV == AO_TYPE)
#else
	if (!sh)
#endif
		return 0;
	else
	{
		return CONTROL_UNKNOWN;
	}
}

static int resync(unsigned int second)
{
	return MagicPixel_MP4_resync(second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{

	mpDebugPrint("free aac memory");
   if(Media_data.sdram_buf){
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}
	if(Media_data.sdram_buf2){
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
}

ad_functions_t software_aac = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};

