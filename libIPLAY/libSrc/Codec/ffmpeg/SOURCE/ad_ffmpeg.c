/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define OUTPUT_PCM  0
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "avTypeDef.h"
#include "mpc_info.h"
#include "bswap.h"
#include "avcodec.h"
 

static ad_info_t info = {
	"FFmpeg/libavcodec audio decoders",
	"ffmpeg",
	"Nick Kurshev",
	"ffmpeg.sf.net",
	""
};

static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static void uninit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static int decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);


ad_functions_t mpcodecs_ad_ffmpeg = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio
};

static int buflen;
static unsigned char *bs_buffer;
static int bufptr;
static int fending;
#define nframe 10 //how many frames in one decode

#if OUTPUT_PCM
static int enter_first = 0;
static DRIVE *sDrv;
static STREAM *shandle;
static int ret;
static unsigned char *tmp_buf;
static int buffer_num;
#endif

extern int avcodec_inited;

static int preinit(sh_audio_t * sh)
{
	sh->audio_out_minsize = 4 * 1600;
	sh->a_buffer = mem_malloc(sh->audio_out_minsize);	//Alloc memmory buffer at interface layer	(New style) C.W 08/12/31

	if(!sh->a_buffer)
		return 0;
	else
		return 1;
}

extern AVCodec amr_nb_decoder;
static int init(sh_audio_t * sh_audio)
{
	int x;
	AVCodecContext *lavc_context;
	AVCodec *lavc_codec;
	
	fending = 0;
	bufptr = 0;
	bs_buffer = NULL;
	buflen = 0;

	//lavc_codec = (AVCodec *)avcodec_find_decoder_by_name(sh_audio->codec->dll); //fixed to AMR_NB
	lavc_codec = (AVCodec *) & amr_nb_decoder;

	if (!lavc_codec)
	{							//impossible
		return -1;
	}

	lavc_context = avcodec_alloc_context();
	sh_audio->context = lavc_context;

	if (sh_audio->wf)
	{
		lavc_context->channels = sh_audio->wf->nChannels;
		lavc_context->sample_rate = sh_audio->wf->nSamplesPerSec;
		lavc_context->bit_rate = sh_audio->wf->nAvgBytesPerSec * 8;
		lavc_context->block_align = sh_audio->wf->nBlockAlign;
		lavc_context->bits_per_sample = sh_audio->wf->wBitsPerSample;
	}
	lavc_context->codec_tag = sh_audio->format;	//FOURCC
	lavc_context->codec_id = lavc_codec->id;	// not sure if required, imho not --A'rpi

	/* alloc extra data */
	if (sh_audio->wf && sh_audio->wf->cbSize > 0)
	{
		lavc_context->extradata = av_malloc(sh_audio->wf->cbSize);
		lavc_context->extradata_size = sh_audio->wf->cbSize;
		memcpy(lavc_context->extradata, (char *) sh_audio->wf + sizeof(WAVEFORMATEX),
			   lavc_context->extradata_size);
	}

	// for QDM2
	if (sh_audio->codecdata_len && sh_audio->codecdata && !lavc_context->extradata)
	{
		lavc_context->extradata = av_malloc(sh_audio->codecdata_len);
		lavc_context->extradata_size = sh_audio->codecdata_len;
		memcpy(lavc_context->extradata, (char *) sh_audio->codecdata, lavc_context->extradata_size);
	}

	/* open it */
	if (avcodec_open(lavc_context, lavc_codec) < 0)
	{
		return -1;
	}

//   MP_DPF("\nFOURCC: 0x%X\n",sh_audio->format);
	if (sh_audio->format == 0x3343414D)
	{
		// MACE 3:1
		sh_audio->ds->ss_div = 2 * 3;	// 1 samples/packet
		sh_audio->ds->ss_mul = 2 * sh_audio->wf->nChannels;	// 1 byte*ch/packet
	}
	else if (sh_audio->format == 0x3643414D)
	{
		// MACE 6:1
		sh_audio->ds->ss_div = 2 * 6;	// 1 samples/packet
		sh_audio->ds->ss_mul = 2 * sh_audio->wf->nChannels;	// 1 byte*ch/packet
	}

	// Decode at least 1 byte:  (to get header filled)
	x = decode_audio(sh_audio, sh_audio->a_buffer, 1, sh_audio->a_buffer_size);
	if (x > 0)
		sh_audio->a_buffer_len = x;

#if 1
	sh_audio->channels = lavc_context->channels;
	sh_audio->samplerate = ( lavc_context->sample_rate< 8000 ? 8000 : lavc_context->sample_rate);
	sh_audio->i_bps = lavc_context->bit_rate >> 3;
#else
	sh_audio->channels = sh_audio->wf->nChannels;
	sh_audio->samplerate = sh_audio->wf->nSamplesPerSec;
	sh_audio->i_bps = sh_audio->wf->nAvgBytesPerSec;
#endif

	sh_audio->samplesize = 2;
#if OUTPUT_PCM
	UartOutValue(sh_audio->channels, 8);
	UartOutText("<-ch\r\n");
	UartOutValue(sh_audio->samplerate, 8);
	UartOutText("<-srate\r\n");
	UartOutValue(sh_audio->i_bps, 8);
	UartOutText("<-sh_audio->i_bps\r\n");
	buffer_num = 0;
	enter_first = 0;
#endif

	return 0;
}

static void uninit(sh_audio_t * sh)
{
	//mpDebugPrint("AMR uninit");
	AVCodecContext *lavc_context = sh->context;

	if (avcodec_close(lavc_context) < 0)
		av_freep(&lavc_context->extradata);
	av_freep(&lavc_context);


	mem_free(sh->a_buffer);
	sh->a_buffer = NULL;

#if OUTPUT_PCM
	if (!shandle)
		FileClose(shandle);
#endif
}

static int control(sh_audio_t * sh, int cmd, void *arg, ...)
{
	AVCodecContext *lavc_context = sh->context;

	switch (cmd)
	{
	case ADCTRL_RESYNC_STREAM:
		avcodec_flush_buffers(lavc_context);
		return CONTROL_TRUE;
	}
	return CONTROL_UNKNOWN;
}


static int decode_audio(sh_audio_t * sh_audio, unsigned char *buf, int minlen, int maxlen)
{
	unsigned char *start = NULL;
	int  y,n,len2, len = 0;

    #if OUTPUT_PCM
	tmp_buf = buf;
	if(!enter_first)
	{
		UartOutText("create file\r\n");
		enter_first = 1;
		sDrv = DriveGet(SD_MMC);
		ret = CreateFile(sDrv, "amr", "pcm");
		if(ret)
			UartOutText("create file fail\r\n");
		shandle = FileOpen(sDrv);
		if(!shandle)
			UartOutText("open file fail\r\n");
	}
    #endif

	if (fending == 1) return len ? len : -1;
       
	while (len < minlen)
	{
		 len2 = 0;
              
		/******Rebecca add********/
		if (buflen == 0)
		{
			bs_buffer=NULL;
			buflen = ds_get_packet(sh_audio->ds, & bs_buffer);

			if (buflen <= 0)
			{
				fending = 1;
				break;
			}

			bufptr = 0;
		}

		start = bs_buffer + bufptr;
		n = CountNFrameSize(start, nframe);//the bistream len of nframe is n, every one frame is 320 bytes

		if (n>buflen)
			n=buflen; 
                            
		y = avcodec_decode_audio(sh_audio->context, (int16_t *) buf, &len2, start, n);
		buflen -= n;
		bufptr += n;
		      	
		if (y < 0)
		{
			UartOutValue(y, 5);
			UartOutText("<--y,Decode error in AMR\r\n");
		}
		if (y < n)
			sh_audio->ds->buffer_pos += y - n;	// put back data (HACK!)

		if (len2 > 0)
		{
			len += len2;
			buf += len2;
		}
		TaskYield();

		/******Rebecca add end****/
		/*int x = ds_get_packet(sh_audio->ds, &start);

		if (x <= 0)
			break;				// error
		y = avcodec_decode_audio(sh_audio->context, (int16_t *) buf, &len2, start, x);
		if (y < 0)
		{
			break;
		}
		if (y < x)
			sh_audio->ds->buffer_pos += y - x;	// put back data (HACK!)
		if (len2 > 0)
		{
			if (len < 0)
				len = len2;
			else
				len += len2;
			buf += len2;
		}*/
	}

#if OUTPUT_PCM
	if(len){
		UartOutText("write file \r\n");

		if (buffer_num < 200)
			ret = FileWrite(shandle, tmp_buf, len);
		if(!ret)
			UartOutText("write file fail\r\n");
		buffer_num++;
	}
	else
	{
		if(fending)
			FileClose(shandle);
	}
#endif

	return len ? len : -1;
}

