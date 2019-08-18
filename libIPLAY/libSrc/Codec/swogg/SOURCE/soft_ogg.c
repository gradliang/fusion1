/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2006, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      soft_ogg.c
*
* Programmer:    Rebecca Hsieh
*                MP E320 division
*
* Created:   08/29/2006
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     08/29/2006    Rebecca Hsieh    first file
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

#if OGG_ENABLE
#include "ogg.h"
extern enum AO_AV_TYPE AO_AV;
extern int left_channel;
extern int right_channel;
static int fending;


static ad_info_t info = {
	"Magic Pixel ogg audio decoder",
	"MP",
	"A'rpi",
	"MP...",
	"based on E340...."
};
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
      #if OGG_ENABLE
      if (AO_AV==AO_TYPE)
      #else
      if (!sh)
      #endif
      {
	      sh->audio_out_minsize = 2048 * 2;
	      sh->audio_in_minsize = 768 * 2;	//FAAD_MIN_STREAMSIZE(6144 bits/channel)*MAX_CHANNELS
      }
      return 1;
}

static int init(sh_audio_t * sh)
{
#if OGG_ENABLE
	unsigned char *buf = sh->a_buffer;	
	int ret;
	char *song_singer, *song_name, *song_album, *song_comment, *song_genre, *song_year, *song_track;

    if (AO_AV == AO_TYPE)
    {
		Media_data.sdram_buf2 = (BYTE *)mem_malloc(SDRAM_SIZE_OGG);

		MP_DEBUG1("sdram_buf2=0x%x", Media_data.sdram_buf2);
		memset(Media_data.sdram_buf2,0,SDRAM_SIZE_OGG);
		ret = MagicPixel_OGG_init(Media_data.file_size,&Media_data.ch,&Media_data.srate,&Media_data.frame_size,&Media_data.bitrate,&Media_data.total_time,
										&song_singer,&song_name, Media_data.sdram_buf2, SDRAM_SIZE_OGG);
	    fending = 0;
        Media_data.frame_num = 0;
        Media_data.fending   = 0;
    	sh->channels   = Media_data.ch;
	    sh->samplerate = Media_data.srate;
	    sh->i_bps      = Media_data.bitrate ;
    }
#endif

    return ret;
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
	int ret;
	unsigned char *EQ_band;
	unsigned char * bufptr;
	int buflen;
    int len = 0;
	
	#if OGG_ENABLE
    if (fending) return len ? len : -1;
    #endif
        
    while( (len < minlen )   &&( len + 8192 <= maxlen))
	{
           #if OGG_ENABLE
           if (AO_AV==AO_TYPE)
           #else
           if (!sh)
           #endif
           {
           	    #if OGG_ENABLE
            	Media_data.frame_num=0;
           	    ret= MagicPixel_OGG_decode(&Media_data.frame_size,Media_data.pcm4_buf,&Media_data.play_time);   
              	if(ret)//eof
            	{
            	       fending=1;
            	       break;
            	}
				if (Media_data.ch >= 2)
					MagicPixel_audio_channel_process(left_channel,right_channel);
         	    MagicPixel_OGG_post_process(0,0,Media_data.frame_size,Media_data.ch,Media_data.pcm4_buf, buf);
        	    Media_data.frame_num++;
         	    len+=2 *Media_data.ch *Media_data.frame_size;// short * channel * frame_size
        	    buf+=2* Media_data.ch *Media_data.frame_size;
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
       #if OGG_ENABLE
       if (AO_AV==AO_TYPE)
       #else
       if (!sh)
       #endif
	      return 0;
}

static int resync(unsigned int second)
{
	return MagicPixel_OGG_resync(1, second);
}

///@ingroup Magic Pixel
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	mpDebugPrint("free ogg memory");
	if(Media_data.sdram_buf){
		mem_free(Media_data.sdram_buf);
		Media_data.sdram_buf = NULL;
	}
	if(Media_data.sdram_buf2){
		mem_free(Media_data.sdram_buf2);
		Media_data.sdram_buf2 = NULL;
	}
}

ad_functions_t software_ogg = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio,
	resync
};
#endif
