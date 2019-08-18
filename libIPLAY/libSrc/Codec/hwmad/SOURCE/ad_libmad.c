/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      ad_libmad.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created:   03/30/2005
*
* Description: SAMPLE audio decoder ,use this for creating new codec
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
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
#define OUTPUT_PCM  0

/*
// Include section 
*/
#include "global612.h"
#include "mptrace.h"
#include "avTypeDef.h"
#include "mpc_info.h"
#include "config.h"
//#include "ad_internal.h"
#include "debug.h"

//extern id3_tag_t ID3TAG;


static ad_info_t info = {
	"libmad mpeg audio decoder",
	"libmad",
	"A'rpi",
	"libmad...",
	"based on Xine's libmad/xine_decoder.c"
};


static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static void uninit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

ad_functions_t hw_mpcodecs_ad_libmad = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio
};

#include <mad.h>
extern int n_frames;
int tot_frames = 0;
typedef struct mad_decoder_s
{
	//struct mad_synth  synth; 
	struct mad_stream stream;
	struct mad_frame frame;

	int have_frame;

	int output_sampling_rate;
	int output_open;
	int output_mode;

} mad_decoder_t;

//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
#include "mjpeg_define.h"
extern BYTE MJ;
static DWORD bwordalign;
static BYTE fending;
static BYTE *BS = NULL;
extern void *BitStream_Source;
extern int BT_len;
#endif

#if OUTPUT_PCM
static DRIVE *sDrv;
static STREAM *shandle;
static int first_time = 1;
#endif

///@defgroup    LIBMAD  Libmad API
///This control audio playing

///@ingroup LIBMAD
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

	volatile mad_decoder_t *this = (mad_decoder_t *) mem_malloc(sizeof(mad_decoder_t));

	if (this == NULL)
	{
		MP_ALERT("Ad_libmad Preinit malloc failed!");
		return 0;
	}
	memset(( mad_decoder_t *)this, 0, sizeof(mad_decoder_t));
	sh->context = (mad_decoder_t *) this;

	mad_stream_init((struct mad_stream *)&this->stream);
	mad_frame_init((struct mad_frame *)&this->frame);

	sh->audio_out_minsize = 2 * 4608;
	sh->audio_in_minsize = 4096;

	return 1;
}

//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
static int MJ_read_data(register unsigned char *mem, register int len)
{
	int numByteRead;
	DWORD dwReadSize;
	SWORD ret;

	if (!len) return 0;
	numByteRead=0;
	if(!BT_len)
	BS=(BYTE *)BitStream_Source;
	while(numByteRead < len)
	{ 
		if (BT_len == 0)
		{
			ret = MjpegPlayerAudioFillBlock (MJPEG_AVI, &dwReadSize,&bwordalign);
			if(ret != MSG_NO_ERR)
			      break;
			BS = (BYTE *)(BitStream_Source + bwordalign);
			BT_len = dwReadSize-bwordalign;
		}
		else
			dwReadSize = BT_len; 

		if(numByteRead + BT_len > len)
		{
			memcpy(mem + numByteRead, BS, len - numByteRead);
			dwReadSize -= len-numByteRead;
			BT_len -= len - numByteRead;
			BS += len - numByteRead;
			numByteRead += len - numByteRead;
		}
		else
		{
			memcpy(mem + numByteRead, BS, BT_len);
			numByteRead += BT_len;
			BT_len = 0;
			BS=BitStream_Source;
		}

	}

	return numByteRead;
}
#endif
static int read_frame(sh_audio_t * sh, unsigned char *buf)
{
	volatile mad_decoder_t *this = (mad_decoder_t *) sh->context;
	int decode_ok = 0;
	int len,num_bytes,ret;

//#if MJPEG_ENABLE
#if MJPEG_ENABLE && MJPEG_TOGGLE
	if (MJ) 
		len = MJ_read_data(&sh->a_in_buffer[sh->a_in_buffer_len],
				sh->a_in_buffer_size - sh->a_in_buffer_len);
	else
	len = demux_read_data(sh->ds, &sh->a_in_buffer[sh->a_in_buffer_len],
				sh->a_in_buffer_size - sh->a_in_buffer_len);
    while(len> 0)
#else
	while ((len =
			demux_read_data(sh->ds, &sh->a_in_buffer[sh->a_in_buffer_len],
							sh->a_in_buffer_size - sh->a_in_buffer_len)) > 0)
#endif
	{
		sh->a_in_buffer_len += len;
		while (!decode_ok)
		{
			mad_stream_buffer((struct mad_stream *)&this->stream, sh->a_in_buffer, sh->a_in_buffer_len);

			ret = mad_frame_decode((struct mad_frame *)&this->frame, (struct mad_stream *)&this->stream, buf);
			if (this->stream.next_frame)
			{
				num_bytes =
					(char *) sh->a_in_buffer + sh->a_in_buffer_len -
					(char *) this->stream.next_frame;
				memmove(sh->a_in_buffer, this->stream.next_frame, num_bytes);
				sh->a_in_buffer_len = num_bytes;
			}

			if (ret == 0)
				return 1;		// OK!!!

			// error! try to resync!
			if (this->stream.error != MAD_ERROR_NONE)
			{

			}
			if (this->stream.error == MAD_ERROR_BUFLEN)
				break;
		}
	}

	return 0;
}

///@ingroup INIT
///@brief   The init of driver, opens device, sets sample rate, channels, sample format  parameters.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.
///@return  static int                  Return 0 for fail and 1 for success.
static int init(sh_audio_t * sh)
{
	volatile mad_decoder_t *this = (mad_decoder_t *) sh->context;
	unsigned char *buf = sh->a_buffer;	//no use

	MPAInit(0);

//#if MJPEG_ENABLE
#if (MJPEG_ENABLE && MJPEG_TOGGLE)
	if(!MJ)
#endif
	{
		this->have_frame = read_frame(sh, buf);
		if (!this->have_frame)
		return 0;				// failed to sync...

		sh->channels = (this->frame.header.mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;
		sh->samplerate = this->frame.header.samplerate;
		sh->i_bps = this->frame.header.bitrate  >> 3;
		sh->samplesize = 2;
	}

	return 1;
}

extern int Layer3_Inited;
extern int Layer1_Inited;
extern int Layer2_Inited;

///@ingroup LIBMAD
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
	volatile mad_decoder_t *this = (mad_decoder_t *) sh->context;
	int len = 0;

	struct mad_frame *frame = (struct mad_frame *)&this->frame;
	  
#if OUTPUT_PCM
	unsigned char *tmp_buf;
	int ret;
	if (first_time)
	{
	    first_time=0;
	    sDrv = DriveGet(SD_MMC);
	    ret = CreateFile(sDrv, "test", "pcm");
		if (ret) UartOutText("create file fail\r\n");
		shandle = FileOpen(sDrv);
	}
	tmp_buf=buf;
#endif  

	while (len < minlen && len + 4608 <= maxlen)
	{
		unsigned int nchannels, nsamples;
		uint16_t *output = (uint16_t *) buf;
		int sample;
		int x;
		int inited = Layer1_Inited | Layer2_Inited | Layer3_Inited;

              
		if (!this->have_frame)
			this->have_frame = read_frame(sh, buf);

		TaskYield();
		if (!this->have_frame)
		{
			len = 0;
			break;				// failed to sync... or EOF
		}
		this->have_frame = 0;

		nsamples = 32 * MAD_NSBSAMPLES(&frame->header);
		if (frame->options & MAD_OPTION_HALFSAMPLERATE)
		{
			nsamples >>= 1;
		}
		if (inited == 1)
		{
			nchannels = MAD_NCHANNELS(&frame->header);
#ifdef LAYERII_FORCE_MONO 
			if (frame->header.layer == MAD_LAYER_II)
			{
				nchannels = 1;
			}
#endif
             
			len += 2 * nchannels * nsamples;
			buf += 2 * nchannels * nsamples;
		}
	}
    #if OUTPUT_PCM
    if(len>0)
    {
        ret=FileWrite(shandle,tmp_buf, len);
        if (ret!=len) MP_DEBUG2("actually write %d ,not %d",ret,len);
    }
    else
        FileClose(shandle);

    #endif
	return len ? len : -1;
}

///@ingroup LIBMAD
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
	volatile mad_decoder_t *this = (mad_decoder_t *) sh->context;
	unsigned char *buf = sh->a_out_buffer;	//no use

	// various optional functions you MAY implement:
	switch (cmd)
	{
	case ADCTRL_RESYNC_STREAM:
		this->have_frame = 0;
		mad_stream_init((struct mad_stream *)&this->stream);
		mad_frame_init((struct mad_frame *)&this->frame);
		return CONTROL_TRUE;

	case ADCTRL_SKIP_FRAME:
		this->have_frame = read_frame(sh, buf);
		return CONTROL_TRUE;
	}
	return CONTROL_UNKNOWN;
}

///@ingroup LIBMAD
///@defgroup UINIT   Uninitialization

///@ingroup UINIT
///@brief   Uninit the whole system , this is on the same "level" as preinit.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be uninitialized.
///@return  NULL    
static void uninit(sh_audio_t * sh)
{
	volatile mad_decoder_t *this = (mad_decoder_t *) sh->context;

	mad_frame_finish((struct mad_frame *)&this->frame);
	mad_stream_finish((struct mad_stream *)&this->stream);
	mem_free(sh->context);
}

