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

/*
// Include section 
*/
#include "global612.h"
//#include "mptrace.h"
#include "avTypeDef.h"
#include "mpc_info.h"
#include "config.h"
#include "debug.h"
#include "mad.h"
#include "dma_cache.h"
#include "mp3_hdr.h"

extern DWORD Audio_FB_Seconds;
extern Audio_dec  Media_data;
extern struct mad_file_info mp3info;
static int data_blocksize;		/* for SHOUTcast streaming */

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

ad_functions_t mpcodecs_ad_libmad = {
	&info,
	preinit,
	init,
	uninit,
	control,
	decode_audio
};

int n_frames = 0;
int tot_frames = 0;

//#if (MJPEG_ENABLE && MP3_MAD_AUDIO)
#if (MJPEG_ENABLE && MJPEG_TOGGLE && MP3_MAD_AUDIO)
#include "mjpeg_define.h"
extern BYTE MJ;
static DWORD bwordalign;
static BYTE fending;
static BYTE  * BS=NULL;
extern void * BitStream_Source;
extern int BT_len;
#endif


typedef struct {
	struct mad_synth synth; 
	struct mad_stream stream;
	struct mad_frame frame;
	mad_timer_t duration;
	int have_frame;

	int output_sampling_rate;
	int output_open;
	int output_mode;
} mad_decoder_t;

typedef struct {
	mad_timer_t duration;
	int file_offset;
} mad_memory_t;

#define NUM_SEEK_POINT	8
mad_memory_t *seek_point;
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

static BYTE first_time;
static BYTE fending;

extern enum AO_AV_TYPE  AO_AV;

static int preinit(sh_audio_t * sh)
{
	mpDebugPrint("Mad preinit");
	
	volatile mad_decoder_t *this = (mad_decoder_t *) libmad_malloc(sizeof(mad_decoder_t));
 
	if (this == NULL)
	{
		MP_ALERT("Ad_libmad Preinit malloc failed!");
		return 0;
	}

	decode_sram_init();

	memset(( mad_decoder_t *)this, 0, sizeof(mad_decoder_t));
	sh->context = (mad_decoder_t *) this;

	if(AO_AV == AV_TYPE){
		memset(&mp3info, 0, sizeof(struct mad_file_info));
		Init_BitstreamCache(MP3_TYPE);
	}
	else{
		sh->ds = (demux_stream_t *)libmad_malloc(sizeof(demux_stream_t));
	    memset((demux_stream_t *)sh->ds, 0, sizeof(demux_stream_t));
	}

    mad_timer_reset(&this->duration);
	seek_point = (mad_memory_t *)libmad_malloc(NUM_SEEK_POINT * sizeof(mad_memory_t));
    memset((char *)seek_point, 0, NUM_SEEK_POINT * sizeof(mad_memory_t));
    first_time=1;
    fending=0;
    
	mad_synth_init((struct mad_synth *)&this->synth);
	mad_stream_init((struct mad_stream *)&this->stream);
	mad_frame_init((struct mad_frame *)&this->frame);

	sh->audio_out_minsize = 2 * 4608;
	sh->audio_in_minsize = 4096;
    MP_DEBUG("libmad preinit ok");
	return 1;
}

static size_t search_0xff(unsigned char *buf, int search_size, int *find_pos)
{
    int i;
    search_size -= 1;
    for (i = 0; i < search_size; i++)
    {
        if (buf[i] == 0xFF)
        {            
            if ((buf[i + 1] & 0xE0) == 0xE0)
            {
                *find_pos = i;
                return 1;
            }
        }
    }
    return 0;
}

static inline int AV_read_data(sh_audio_t *sh)
{
	int len;
	int x;
	int bytes = 0;
	register demux_stream_t * ds = sh->ds;

	//mpDebugPrint("**sh->a_in_buffer_size: %d,sh->a_in_buffer_len: %d", sh->a_in_buffer_size, sh->a_in_buffer_len);
    len = sh->a_in_buffer_size - sh->a_in_buffer_len;	//consume how many bytes

    MP_DEBUG2("sh->a_in_buffer_len=%d, len=%d", sh->a_in_buffer_len, len);

    while (len > 0)
    {
		x = ds->buffer_size - ds->buffer_pos;

		//mpDebugPrint("1. x: %d, ds info: dsPos: %d, dsSize: %d", x, ds->buffer_pos, ds->buffer_size);
		if (x == 0)
		{
			if(fending) return bytes;

			ds->buffer_pos = 0;

			//mpDebugPrint("2. ds info: dsPos: %d, dsSize: %d", ds->buffer_pos, ds->buffer_size);
			if(Fill_Cache_Buffer2(MP3_TYPE))
				fending = 1;

			//mpDebugPrint("3. ds info: dsPos: %d, dsSize: %d", ds->buffer_pos, ds->buffer_size);
			MP_DEBUG("read 1");

			if (!ds->buffer_size){
				return bytes;
			}	
		}
		else
		{
			if (x > len)
				x = len;

			if (sh->a_in_buffer)
				memcpy(sh->a_in_buffer + sh->a_in_buffer_len + bytes, &ds->buffer[ds->buffer_pos], x);

			bytes += x;
			len -= x;
			ds->buffer_pos += x;
		}
	}

	//mpDebugPrint("AV_read_data: %d", bytes);

	return bytes;
}

/*
      there are two  4K buffer for simple bitstream buffer
*/
static int read_data(sh_audio_t *sh)
{
	int ret,len;
	int x;
	int bytes = 0;
	register demux_stream_t * ds = sh->ds;

	if(AO_AV == AV_TYPE)
	{
		return AV_read_data(sh);
	}
	else if(first_time)
    {
        first_time = 0;
        if( (ret=GetCacheEndPtr() - GetCachePtr()) != 0)
               ds->buffer_size=MoveReservedBytesandFillinCache(GetCachePtr(),MP3_TYPE);
        
        ds->buffer_pos=0;//for Cache2_ptr;
        ds->buffer=(unsigned char *)GetCacheBuffer2();//cachebuffer2

        return (int)GetCacheEndPtr();
    }
    else
    {
        bytes = 0;

        len = sh->a_in_buffer_size - sh->a_in_buffer_len;//consume how many bytes
        MP_DEBUG2("sh->a_in_buffer_len=%d,len=%d",sh->a_in_buffer_len,len);
        while (len > 0)
        {
			x = ds->buffer_size - ds->buffer_pos;
			if (x == 0)
			{
				if(fending) return bytes;
					ds->buffer_pos=0;

               if(Fill_Cache_Buffer2(MP3_TYPE))
					fending=1;

				ds->buffer_size = GetCacheEndPtr();
				MP_DEBUG("read 1");

				if (!ds->buffer_size)
					return bytes;
			}
			else
			{
				if (x > len)
					x = len;

				if (sh->a_in_buffer)
					memcpy(sh->a_in_buffer + sh->a_in_buffer_len + bytes, &ds->buffer[ds->buffer_pos], x);

				bytes += x;
				len -= x;
				ds->buffer_pos += x;
			}
		}

		return bytes;
	}
}

//#if (MJPEG_ENABLE  && MP3_MAD_AUDIO)
#if (MJPEG_ENABLE && MJPEG_TOGGLE && MP3_MAD_AUDIO)
static int MJ_read_data(register unsigned char *mem, register int len)
{
	int numByteRead;
	DWORD dwReadSize;
	SWORD ret;

	MP_TRACE_LINE();

	if (!len) return 0;

	MP_TRACE_LINE();
	numByteRead = 0;
	MP_TRACE_LINE();

	if(!BT_len)
		BS = (BYTE *)BitStream_Source;

	MP_TRACE_LINE();

	while(numByteRead < len)
	{
		if (BT_len == 0)
		{
			MP_TRACE_LINE();
			ret = MjpegPlayerAudioFillBlock(MJPEG_AVI, &dwReadSize, &bwordalign);

			if(ret != MSG_NO_ERR)
				break;
			MP_TRACE_LINE();
			BS = (BYTE *)(BitStream_Source + bwordalign);
			BT_len = dwReadSize - bwordalign;
		}
		else
			dwReadSize = BT_len; 


		MP_TRACE_LINE();                
		if(numByteRead + BT_len > len)
		{
			MP_TRACE_LINE();
			memcpy(mem + numByteRead, BS, len - numByteRead);
			dwReadSize -= len-numByteRead;
			BT_len -= len - numByteRead;
			BS += len - numByteRead;
			numByteRead += len - numByteRead;
		}
		else
		{
			MP_TRACE_LINE();
			memcpy(mem + numByteRead, BS, BT_len);
			numByteRead += BT_len;
			BT_len = 0;
			BS = BitStream_Source;
		}
	}

	return numByteRead;
}

static int Get_Data(sh_audio_t *sh)
{
	int len;
	if  (MJ) 
	{
		len = MJ_read_data(&sh->a_in_buffer[sh->a_in_buffer_len],
				sh->a_in_buffer_size - sh->a_in_buffer_len);
		sh->a_in_buffer_len += len;
	}     
	else
	{
		MP_TRACE_LINE();
		sh->a_in_buffer_len += read_data(sh);
		len = sh->a_in_buffer_len;
	}

	return len;
}
#endif

static int decode_frame(sh_audio_t *sh)
{
  mad_decoder_t *this = (mad_decoder_t *) sh->context;
  int len;
  int start = 0;
  int search_time = 0;
  int check_error = 0, num_check_error = (Media_data.play_time == 0) ? 10000 : 40000;
  unsigned char *metadata_start;
  int metadata_blocksize, bytesconsumed;

	MP_DEBUG2("a_in_buffer_size=%d,a_in_buffer_len=%d",sh->a_in_buffer_size,sh->a_in_buffer_len);

//#if (MJPEG_ENABLE  && MP3_MAD_AUDIO)
#if (MJPEG_ENABLE && MJPEG_TOGGLE && MP3_MAD_AUDIO)
	while ((len = Get_Data(sh)) > 0)
#else
	while((sh->a_in_buffer_len += read_data(sh))>0)
#endif
	{
		if ((unsigned char)sh->a_in_buffer[0] != 0xff)
		{
			search_time++;

			if  (search_0xff(sh->a_in_buffer, sh->a_in_buffer_len, &start) == 0)
			{
				start += sh->a_in_buffer_len;
				sh->a_in_buffer_len = 0;
				continue;
			}

			MP_DEBUG("search_0xff == %d, search time %d", start, search_time);
		}

		if (start > 4)
			MagicPixel_MP3_MAD_songinfo_callback(sh->a_in_buffer, start, 0);

		while(1){
			int ret;
			mad_stream_buffer (&this->stream, sh->a_in_buffer, sh->a_in_buffer_len);
			//    time_elapsed = GetRelativeMs();
			ret = mad_frame_decode (&this->frame, &this->stream);
			//    time_elapsed = GetRelativeMs();
			//    decode_50 += time_elapsed;
			MP_DEBUG1("ret=%d",ret);

			if (this->stream.next_frame) {
				int num_bytes =
				(char*)sh->a_in_buffer + sh->a_in_buffer_len - (char*)this->stream.next_frame;

				if (mp3info.shoutcast) {
					bytesconsumed = sh->a_in_buffer_len - num_bytes;
					data_blocksize -= bytesconsumed;

					if (data_blocksize < (MP3_BITSTREAM_CACHE >> 1)) {
						metadata_start = sh->a_in_buffer + bytesconsumed + data_blocksize;
						metadata_blocksize = *(metadata_start) << 4;

						if (metadata_blocksize)
							MagicPixel_MP3_MAD_songinfo_callback(metadata_start + 1, metadata_blocksize, 0);

						metadata_blocksize++;
						num_bytes -= metadata_blocksize;
						memmove(sh->a_in_buffer, sh->a_in_buffer + bytesconsumed, data_blocksize);
						memmove(sh->a_in_buffer + data_blocksize, metadata_start + metadata_blocksize, num_bytes - data_blocksize);
						data_blocksize += mp3info.block_size;
					} else {
						memmove(sh->a_in_buffer, this->stream.next_frame, num_bytes);
					}
				} else {
					memmove(sh->a_in_buffer, this->stream.next_frame, num_bytes);
				}

				mp_msg(MSGT_DECAUDIO,MSGL_DBG2, "libmad: %d bytes processed\n", sh->a_in_buffer_len - num_bytes);
				sh->a_in_buffer_len = num_bytes;
			}

			if (ret == 0) 	return 1; // OK!!!

			// error! try to resync!
			if(this->stream.error == MAD_ERROR_BUFLEN)
			{
				if(Media_data.fending == 1) return 0;
				else  break;
			}
		
			if (ret == -1) {
				if (check_error <= num_check_error)
				{
					check_error++;
					if(Media_data.play_time == 0)  mpDebugPrint("sync for Next Word....");
					if((check_error % 500) == 0){
						mpDebugPrint("sync for Next Word....");
					}
				}	
				else {
					Media_data.fending = 1;
					return 0;
				}
			}
		}
	}

	mp_msg(MSGT_DECAUDIO,MSGL_INFO,"Cannot sync MAD frame\n");

	return 0;
}

static int skip_frame(sh_audio_t *sh)
{
	mad_decoder_t *this = (mad_decoder_t *) sh->context;
	int start = 0, ret, num_bytes;

	while((sh->a_in_buffer_len += read_data(sh)) > 0) {
		if ((unsigned char)sh->a_in_buffer[0] != 0xff) {
			if (search_0xff(sh->a_in_buffer, sh->a_in_buffer_len, &start) == 0) {
				sh->a_in_buffer_len = 0;
				continue;
			}
		}

		while (1) {
			mad_stream_buffer(&this->stream, sh->a_in_buffer + start, sh->a_in_buffer_len - start);
			start = 0;
			ret = mad_frame_skip(&this->frame, &this->stream);
			if (this->stream.next_frame) {
				num_bytes = (char *)sh->a_in_buffer + sh->a_in_buffer_len - (char *)this->stream.next_frame;
				memmove(sh->a_in_buffer, this->stream.next_frame, num_bytes);
				sh->a_in_buffer_len = num_bytes;
			}
			if (ret == 0) return 1;	// OK!!!
			// error! try to resync!
			if(this->stream.error == MAD_ERROR_BUFLEN) break;
		}
	}
	return 0;
}

static void save_point(sh_audio_t *sh)
{
	mad_decoder_t *this = (mad_decoder_t *) sh->context;
	demux_stream_t *ds = sh->ds;
	int i;

	for (i = (NUM_SEEK_POINT - 2); i > 0; i--)
		seek_point[i] = seek_point[i - 1];

	seek_point[i].duration = this->duration;
	seek_point[i].file_offset = MagicPixel_MP3_MAD_getposition_callback();
	seek_point[i].file_offset -= sh->a_in_buffer_len + (ds->buffer_size - ds->buffer_pos);

	if (mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS) == 0)
		seek_point[NUM_SEEK_POINT - 1] = seek_point[i];
}

static int search_point(sh_audio_t *sh, int target)
{
	int i;

	for (i = 0; i < NUM_SEEK_POINT; i++) {
		if (mad_timer_count(seek_point[i].duration, MAD_UNITS_MILLISECONDS) <= target)
			break;
	}
	return i;
}

///@ingroup INIT
///@brief   The init of driver, opens device, sets sample rate, channels, sample format  parameters.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.
///@return  static int                  Return 0 for fail and 1 for success.

static int init(sh_audio_t *sh)
{
	mpDebugPrint("Mad init");
//#if (MJPEG_ENABLE && MP3_MAD_AUDIO)
#if (MJPEG_ENABLE && MJPEG_TOGGLE && MP3_MAD_AUDIO)
	if(MJ)
		return 1;
#endif    

	mad_decoder_t *this = (mad_decoder_t *) sh->context;
	extern void * BitStream_Source;

	BitStream_Source = (void*)sh;
	sh->a_in_buffer = (BYTE *)InitBitStreamBufferTo();//rebecca add

	if(AO_AV == AV_TYPE){
		memset(&Media_data, 0, sizeof( Audio_dec));
//		sh->channels	= ((WAVEFORMATEX*)sh->wf)->nChannels;
//		sh->samplerate	= ((WAVEFORMATEX*)sh->wf)->nSamplesPerSec;
//		sh->i_bps		= ((WAVEFORMATEX*)sh->wf)->nAvgBytesPerSec * 8 ;


		//mpDebugPrint("Mad init ---- Demux audio address: %x", sh->ds->demuxer->audio);
		//mpDebugPrint("Mad init ---- sh_audio address: %x",    sh);
		//mpDebugPrint("Channel: %d, Sample rate: %d, Bitrate: %d", sh->channels, sh->samplerate, sh->i_bps);

		Media_data.frame = 0;
//		sh->samplesize = 2;

//		this->have_frame = decode_frame(sh);

//		return 1;
	}

	if (mp3info.shoutcast)
		data_blocksize = mp3info.block_size - mp3info.skip_bytes;

	this->have_frame = decode_frame(sh);

	if(!this->have_frame){
		mpDebugPrint("Init fail to sync");
		return 0; // failed to sync...
	}

	save_point(sh);

	sh->channels	= (this->frame.header.mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;
	sh->samplerate	= this->frame.header.samplerate;
	sh->i_bps		= this->frame.header.bitrate >> 3;
	sh->samplesize	= 2;

	//mpDebugPrint("libmad init ok - ch %d, samplerate %d, i_bps %d", sh->channels, sh->samplerate, sh->i_bps);

	return 1;
}


/* utility to scale and round samples to 16 bits */
static inline signed int scale(mad_fixed_t sample) {
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static int decode_audio(sh_audio_t *sh,unsigned char *buf,int minlen,int maxlen)
{
	//mpDebugPrint("Mad Decode audio, sh: %x", sh);

	mad_decoder_t *this = (mad_decoder_t *) sh->context;
	int len=0;

	while(len<minlen && len+4608<=maxlen){
		if(!this->have_frame) this->have_frame=decode_frame(sh);
		if(!this->have_frame) break; // failed to sync... or EOF
    
		this->have_frame = 0;

		//    time_elapsed = GetRelativeMs();
		synth_sram_init();
		mad_synth_frame (&this->synth, &this->frame);
		decode_sram_init();
		//    time_elapsed = GetRelativeMs();
		//    synth_50 += time_elapsed;
		//    frame_count++;
		/*    if (frame_count == 50) {
		total_frame += 50;
		UartOutText("\r\ndecode 50 frames: ");
		UartOutValue(decode_50, 8);
		UartOutText(" ms, average: ");
		total_decode_time += decode_50;
		UartOutValue(total_decode_time / total_frame, 4);
		UartOutText(" ms per frame");
		UartOutText("\r\nsynth 50 frames: ");
		UartOutValue(synth_50, 8);
		UartOutText(" ms, average: ");
		total_synth_time += synth_50;
		UartOutValue(total_synth_time / total_frame, 4);
		UartOutText(" ms per frame\r\n");
		frame_count = decode_50 = synth_50 = 0;
		}*/
		{
			unsigned int         nchannels, nsamples;
			mad_fixed_t const   *left_ch, *right_ch;
			struct mad_pcm      *pcm = &this->synth.pcm;
			uint16_t            *output = (uint16_t*) buf;

			nchannels = pcm->channels;
			nsamples  = pcm->length;
			left_ch   = pcm->samples[0];
			right_ch  = pcm->samples[1];
	   
#if 0	// #ifdef LAYERII_FORCE_MONO 
			if (this->frame.header.layer == MAD_LAYER_II)
			{
				nchannels = 1;
			}
#endif

			len += 2 * nchannels * nsamples;
			buf += 2 * nchannels * nsamples;

			while (nsamples--) {
				/* output sample(s) in 16-bit signed little-endian PCM */

				*output++ = scale(*left_ch++);

				if (nchannels == 2) 
				*output++ = scale(*right_ch++);
			}
		}
//#if (MJPEG_ENABLE && MP3_MAD_AUDIO)
#if (MJPEG_ENABLE && MJPEG_TOGGLE && MP3_MAD_AUDIO)
		if(!MJ)
#endif
		{
			mad_timer_add(&this->duration, this->frame.header.duration);
			Media_data.play_time = mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS);

//			if ((this->duration.seconds - seek_point[0].duration.seconds) >= MOVIE_AUDIO_FF_SECONDS)
			if ((this->duration.seconds - seek_point[0].duration.seconds) >= Audio_FB_Seconds)
				save_point(sh);
		}
	}

	MP_DEBUG("libmad decode_audio %d", len);

	return len?len:-1;
}

static int seek(sh_audio_t *sh, int *target_time)
{
	mad_decoder_t *this = (mad_decoder_t *) sh->context;

	if (*target_time >= mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS)) {
		while (mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS) < *target_time) {
			if (!this->have_frame)	this->have_frame = skip_frame(sh);
			if (!this->have_frame)	break;	// failed to sync... or EOF
			this->have_frame = 0;
			mad_timer_add(&this->duration, this->frame.header.duration);
		}
	} else {
#if 1	// seek from memorized position
        ClearBitstreamCache();
        SetCachePtr(0);
        SetCacheEndPtr(0);
		
		int ret = search_point(sh, *target_time);
		MagicPixel_MP3_MAD_fileseek_callback(seek_point[ret].file_offset);
		Fill_Cache_Buffer(MP3_TYPE);
		this->duration = seek_point[ret].duration;

		sh->a_in_buffer_len = 0;
		fending = 0;
		first_time = 1;
		while (mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS) < *target_time) {
			if(!this->have_frame) this->have_frame = skip_frame(sh);
			if(!this->have_frame) break;	// failed to sync... or EOF
			this->have_frame = 0;
			mad_timer_add(&this->duration, this->frame.header.duration);
		}

		if (ret == (NUM_SEEK_POINT - 1)) {
			memset((char *)seek_point, 0, (NUM_SEEK_POINT - 1) * sizeof(mad_memory_t));
			seek_point[0] = seek_point[NUM_SEEK_POINT - 1];
			save_point(sh);
		}
#else	// seek from the beginning of the file
		uint8_t hdr[4];

		Init_BitstreamCache();
		MagicPixel_MP3_MAD_fileseek_callback(0);
		Fill_Cache_Buffer();
		mad_read_buffer(hdr, 4);
		if (hdr[0] == 'I' && hdr[1] == 'D' && hdr[2] == '3' && (hdr[3] >= 2)) {
			mad_skip_buffer(2);
			mad_read_buffer(hdr, 4);
			mad_skip_buffer(GetID3Length(hdr));
		} else {
			mad_seek_buffer(0);
		}

		sh->a_in_buffer_len = 0;
		fending = 0;
		first_time = 1;
		mad_timer_reset(&this->duration);
		while (mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS) < *target_time) {
			if(!this->have_frame) this->have_frame = skip_frame(sh);
			if(!this->have_frame) break;	// failed to sync... or EOF
			this->have_frame = 0;
			mad_timer_add(&this->duration, this->frame.header.duration);
		}
#endif
	}

	*target_time = Media_data.play_time = mad_timer_count(this->duration, MAD_UNITS_MILLISECONDS);

	return CONTROL_TRUE;
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
	mad_decoder_t *this = (mad_decoder_t *) sh->context;

	// various optional functions you MAY implement:
	switch (cmd) {
	case ADCTRL_SEEK_STREAM:
		this->have_frame = 0;
		return seek(sh, arg);
	case ADCTRL_RESYNC_STREAM:
		this->have_frame = 0;
		mad_synth_init(&this->synth);
		mad_stream_init((struct mad_stream *)&this->stream);
		mad_frame_init((struct mad_frame *)&this->frame);
		return CONTROL_TRUE;
	case ADCTRL_SKIP_FRAME:
		this->have_frame = decode_frame(sh);
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
static void uninit(sh_audio_t *sh)
{
//mpDebugPrint("Mad Uninit");

	mad_decoder_t *this = (mad_decoder_t *) sh->context;

	mad_synth_finish (&this->synth);
#if 1
	if (this->stream.main_data)
	{
		libmad_free(this->stream.main_data);
		this->stream.main_data = 0;
	}
#else
	mad_stream_finish(&this->stream);  
#endif  

	mad_frame_finish (&this->frame);
	libmad_free(sh->context);
	libmad_free(seek_point);
	libmad_free(sh->ds);
	Free_BitstreamCache();
}

