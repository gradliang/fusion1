/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2005 M. Bakker, Nero AG, http://www.nero.com
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** The "appropriate copyright message" mentioned in section 2c of the GPLv2
** must read: "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Nero AG through Mpeg4AAClicense@nero.com.
**
** $Id: ad_faad.c,v 1.00 2008/02/20 12:33:29 menno Exp $
**/

///@mainpage Faad API Use Guaid
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
#include "common.h"
#include "structs.h"
#include "syntax.h"
#include "decoder.h"
#include "faad.h"
#include "mp4ff.h"
#include "aac_info.h"

static ad_info_t info = {
    "AAC (MPEG2/4 Advanced Audio Coding)",
    "faad",
    "Felix Buenemann",
    "faad2",
    "uses libfaad2"
};

static int init(sh_audio_t * sh);
static int preinit(sh_audio_t * sh);
static void uninit(sh_audio_t * sh);
static int control(sh_audio_t * sh, int cmd, void *arg, ...);
static int _decode_audio(sh_audio_t * sh, unsigned char *buffer, int minlen, int maxlen);

ad_functions_t mpcodecs_ad_faad = {
	&info,
	preinit,
	init,
	uninit,
	control,
	_decode_audio
};

//#define MEASURE_DECODE_TIME
#ifdef MEASURE_DECODE_TIME
    unsigned int time_elapsed, decode_50, total_time, frames, total_frames;
#endif

//#define OUTPUT_PCM_DATA
#ifdef OUTPUT_PCM_DATA
    #include "audio.h"
    static DRIVE *drive;
    static audio_file *aufile;
#endif

typedef struct {
    mp4ff_callback_t *mp4cb;
    mp4ff_t *infile;
    int track;
    int sample_id;
    int num_samples;
    faad_timer_t duration;

    /* for gapless decoding */
    unsigned int use_aac_length;
    unsigned int initial;
    unsigned int framesize;
    unsigned long timescale;
} faad_decoder_t;

typedef struct {
	unsigned int frame;
    faad_timer_t duration;
    int file_offset;
} faad_seekpoint_t;

extern Audio_dec Media_data;
extern NeAACDecFileInfo aacinfo;
extern enum AO_AV_TYPE  AO_AV;
extern void * BitStream_Source;
extern DWORD Audio_FB_Seconds;
static unsigned char first_time, at_eof;
static demux_stream_t ds_data;
static faad_decoder_t decoder;
static faad_seekpoint_t *seek_table;
static int data_blocksize;      /* for SHOUTcast streaming */


#define AAC_TYPE 2
///@defgroup    FAAD    Libfaad API
///This control audio playing

///@ingroup FAAD
///@defgroup INIT   Initialization

///@ingroup PREINIT
///@brief   Init the audio system.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.             
///@return  static int                  Return 0 for fail and 1 for success.
static int preinit(sh_audio_t *sh)
{
	if(AO_AV == AV_TYPE){
		memset(&aacinfo, 0, sizeof(NeAACDecFileInfo));
		Init_BitstreamCache(AAC_TYPE);
	}
	else{
		sh->ds = (demux_stream_t *)&ds_data;
		memset((demux_stream_t *)sh->ds, 0, sizeof(demux_stream_t));
	}

    faad_working_space_init();

#ifdef MEASURE_DECODE_TIME
    decode_50 = total_time = frames = total_frames = 0;
#endif

    first_time = 1;
    at_eof = 0;
    sh->audio_out_minsize = 2048 * 2;

	//Mark below line, because alloc in_buffer memory in Ad_faad - init
	//sh->audio_in_minsize = FAAD_BITSTREAM_CACHE;
	sh->audio_in_minsize = 0;
	
    return 1;
}

static inline int AV_read_data(sh_audio_t *sh)
{
	int len;
	int x;
	int bytes = 0;
	register demux_stream_t * ds = sh->ds;

//	mpDebugPrint("**sh->a_in_buffer_size: %d, sh->a_in_buffer_len: %d", sh->a_in_buffer_size, sh->a_in_buffer_len);
	len = sh->a_in_buffer_size - sh->a_in_buffer_len;	//consume how many bytes

	while (len > 0)
	{
		x = ds->buffer_size - ds->buffer_pos;

//		mpDebugPrint("1. x: %d, ds addr: %x, ds buffer addr: %x, ds info: dsPos: %d, dsSize: %d",
//						x, ds, ds->buffer, ds->buffer_pos, ds->buffer_size);
		if (x == 0)
		{
			if(at_eof) return bytes;

			ds->buffer_pos = 0;

//			mpDebugPrint("2. ds info: dsPos: %d, dsSize: %d", ds->buffer_pos, ds->buffer_size);

			if(Fill_Cache_Buffer2(AAC_TYPE))
				at_eof = 1;

//			mpDebugPrint("3. ds info: dsPos: %d, dsSize: %d", ds->buffer_pos, ds->buffer_size);

			if (!ds->buffer_size){
				return bytes;
			}	
		}
		else
		{
//mpDebugPrint("AAC AV read data else");
			if (x > len)
				x = len;

			if (sh->a_in_buffer){
//mpDebugPrint("AAC AV read data else2, ds Buff addr: %x", ds->buffer);
//mpDebugPrint("AAC AV read data else2, copy size %d", x);
				memcpy(sh->a_in_buffer + sh->a_in_buffer_len + bytes, &ds->buffer[ds->buffer_pos], x);
			}
//mpDebugPrint("AAC AV read data else3");
			bytes += x;
			len -= x;
			ds->buffer_pos += x;
		}
	}

//	mpDebugPrint("AV_read_data: %d", bytes);
	return bytes;
}

static int read_data(sh_audio_t *sh)
{
	int bytes_consumed, x, bytes = 0;
	register demux_stream_t *ds = sh->ds;

	if(AO_AV == AV_TYPE){
		return AV_read_data(sh);
	}
	else if(first_time)
	{
        first_time = 0;
        if ((GetCacheEndPtr() - GetCachePtr()) != 0) {
           ds->buffer_size = MoveReservedBytesandFillinCache(GetCachePtr(), AAC_TYPE);
        }
        ds->buffer_pos = 0;
        ds->buffer = (unsigned char *)GetCacheBuffer2();
        return GetCacheEndPtr();
    } else {
		bytes_consumed = sh->a_in_buffer_size - sh->a_in_buffer_len;

		while (bytes_consumed > 0) {
			x = ds->buffer_size - ds->buffer_pos;

			if (x == 0) {
				if (at_eof)
					return bytes;
				ds->buffer_pos = 0;

				if (Fill_Cache_Buffer2(AAC_TYPE))
					at_eof = 1;

				ds->buffer_size = GetCacheEndPtr();

				if (!ds->buffer_size)
					return bytes;
			} else {
				if (x > bytes_consumed)
					x = bytes_consumed;
				if (sh->a_in_buffer)
					memcpy(sh->a_in_buffer + sh->a_in_buffer_len + bytes, &ds->buffer[ds->buffer_pos], x);
				bytes += x;
				bytes_consumed -= x;
				ds->buffer_pos += x;
			}
		}

		return bytes;
	}
}

#define SEEK_TABLE_LEN	8

static void save_seek_point(sh_audio_t *sh)
{
    NeAACDecHandle this = (NeAACDecHandle) sh->context;
    demux_stream_t *ds = sh->ds;
    int i;

    for (i = (SEEK_TABLE_LEN - 2); i > 0; i--)
        seek_table[i] = seek_table[i - 1];

    seek_table[i].frame = this->frame;
    seek_table[i].duration = decoder.duration;
    seek_table[i].file_offset = MagicPixel_AAC_FAAD_getposition_callback();
    seek_table[i].file_offset -= sh->a_in_buffer_len + (ds->buffer_size - ds->buffer_pos);

    if (faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS) == 0)
        seek_table[SEEK_TABLE_LEN - 1] = seek_table[i];
}

static int search_seek_point(sh_audio_t *sh, int target)
{
    int i;

    for (i = 0; i < SEEK_TABLE_LEN; i++) {
        if (faad_timer_count(seek_table[i].duration, FAAD_UNITS_MILLISECONDS) <= target)
            break;
    }

    return i;
}


///This function is only implemented partial of spec.	C.W 08/11/13
///
///If you want to complete all part of this function,
///Please reference AudioSpecificConfig2 in mp4.c file
void ParsingAudioSpecificInfo(BYTE *infoAddr, DWORD infoLen, DWORD *rtvSamplerate, DWORD *rtvChannel, BYTE *rtvObjType)
{
	WORD bitstream;
	BYTE ObjectType = 0;
	BYTE SamplerateIdx = 0;
	bitstream = *(WORD *)infoAddr;

	ObjectType		= (bitstream & 0xf800) >> 11;
	SamplerateIdx	= (bitstream & 0x0780) >> 7;

	*rtvSamplerate	= get_sample_rate(SamplerateIdx);
	*rtvChannel		= (bitstream & 0x0078) >> 3;
	*rtvObjType		= ObjectType;

//	mpDebugPrint("<Audio Parsing result>infoAddr data: %x, Sample rate: %d, channel: %d, ObjectType: %d",
//					*(DWORD *)infoAddr,	*rtvSamplerate, *rtvChannel, ObjectType);
}

//Init function when Video case - raw data mode
int32_t NeAACDecInit_AVcase(NeAACDecHandle hDecoder, sh_audio_t *sh)
{
	BYTE AudioObjType;

	ParsingAudioSpecificInfo(sh->codecdata, sh->codecdata_len, 
							(DWORD *)&sh->samplerate, (DWORD *)&sh->channels, &AudioObjType);

	//	mpDebugPrint("SH1 Channels: %d, Samplerate: %d, Code address: %x, Code len: %d",
	//					sh->channels, sh->samplerate, sh->codecdata, sh->codecdata_len);

	hDecoder->sf_index = get_sr_index(sh->samplerate);
	hDecoder->object_type = AudioObjType;
	hDecoder->pce_set = 0;

	if(AudioObjType != LC){
		MP_ALERT("Your AAC file is not base type, we may not support it!!");
		return -1;
	}

	//----------------------------   Below part is ported from NeAACDecInit()   --------------------	C.W 081120
#if (defined(PS_DEC) || defined(DRM_PS))
    /* check if we have a mono file */
    if (sh->channels == 1)
    {
        /* upMatrix to 2 channels for implicit signalling of PS */
        sh->channels = 2;
    }
#endif
    hDecoder->channelConfiguration = sh->channels;

#ifdef SBR_DEC
    /* implicit signalling */
    if (sh->samplerate <= 24000 && !(hDecoder->config.dontUpSampleImplicitSBR))
    {
        sh->samplerate *= 2;
        hDecoder->forceUpSampling = 1;
    } else if (sh->samplerate > 24000 && !(hDecoder->config.dontUpSampleImplicitSBR)) {
        hDecoder->downSampledSBR = 1;
	}
#endif

    /* must be done before frameLength is divided by 2 for LD */
#ifdef SSR_DEC
    if (hDecoder->object_type == SSR)
        hDecoder->fb = ssr_filter_bank_init(hDecoder->frameLength/SSR_BANDS);
    else
#endif
        hDecoder->fb = (fb_info *)filter_bank_init(hDecoder->frameLength);

#ifdef LD_DEC
    if (hDecoder->object_type == LD)
        hDecoder->frameLength >>= 1;
#endif

    if (can_decode_ot(hDecoder->object_type) < 0){
		mpDebugPrint("Audio object type is not supported");
        return -1;
    }
	//----------------------------   End of porting from NeAACDecInit()   --------------------

	//mpDebugPrint("End of AAC init>>> Object - %d, sf_index - %d, Channel - %d",
	//				hDecoder->object_type, hDecoder->sf_index, hDecoder->channelConfiguration);

}

///@ingroup INIT
///@brief   The init of driver, opens device, sets sample rate, channels, sample format  parameters.
///@param   struct sh_audio_t *sh       Pointer to a stream header structure to be initialized.
///@return  static int                  Return 0 for fail and 1 for success.
static int init(sh_audio_t *sh)
{
	mpDebugPrint("Faad AAC init");

	extern void * BitStream_Source;

    NeAACDecHandle this;
    NeAACDecConfigurationPtr config;
    unsigned long samplerate = 0;
    unsigned char channels = 0;

	BitStream_Source = (void*)sh;

	//Because alloc a_in_buffer memory here, filling sh related variables is needed.
	sh->a_in_buffer = (BYTE *)InitBitStreamBufferTo();
	sh->a_in_buffer_size = FAAD_BITSTREAM_CACHE;

	if(AO_AV == AV_TYPE){
		sh->a_in_buffer_len = 0;
		memset(sh->a_in_buffer, 0, sh->a_in_buffer_size);
	}

	//mpDebugPrint("[Audio decoder2]sh_Audio in buffer addr: %x", sh->a_in_buffer);

    if ((this = NeAACDecOpen()) == NULL)
        return 0;

    sh->context = this;

    /* set configuration */
    config = NeAACDecGetCurrentConfiguration(this);
    config->outputFormat = FAAD_FMT_16BIT;
    config->downMatrix = 1;     /* Down matrix 5.1 to 2 channels */

    if (!aacinfo.mp4file) {
        /* Set the default object type and samplerate */
        /* This is useful for RAW AAC files */
        config->defObjectType = LC;
        config->defSampleRate = 44100;
        config->useOldADTSFormat = 0;
    }

    NeAACDecSetConfiguration(this, config);

    if (aacinfo.mp4file) {

		unsigned char *buffer = NULL;
		int buffer_size = 0;

		/* initialise the callback structure */
		decoder.mp4cb = (mp4ff_callback_t *)faad_malloc(sizeof(mp4ff_callback_t));

		if (!decoder.mp4cb)     return 0;
		decoder.mp4cb->read = read_callback;
		decoder.mp4cb->seek = seek_callback;
		decoder.mp4cb->user_data = NULL;

		decoder.infile = mp4ff_open_read(decoder.mp4cb);
		if ((decoder.track = get_aac_track(decoder.infile)) < 0) {
			NeAACDecClose(this);
			mp4ff_close(decoder.infile);
			faad_free(decoder.mp4cb);

			return 0;
		}
		mp4ff_get_decoder_config(decoder.infile, decoder.track, &buffer, &buffer_size);

		if (NeAACDecInit2(this, buffer, buffer_size, &samplerate, &channels) < 0) {
			NeAACDecClose(this);
			mp4ff_close(decoder.infile);
			faad_free(decoder.mp4cb);
			faad_free(buffer);

			return 0;
		}
		faad_free(buffer);

		/* for gapless decoding */
		decoder.use_aac_length = 0;
		decoder.initial = 1;
		decoder.framesize = this->frameLength;
		decoder.timescale = mp4ff_time_scale(decoder.infile, decoder.track);
		decoder.sample_id = 0;
		decoder.num_samples = mp4ff_num_samples(decoder.infile, decoder.track);
	}
	else
	{
        int bytes_read;

		if(AO_AV == AO_TYPE)
			sh->a_in_buffer_len += read_data(sh);

		if(AO_AV == AV_TYPE){

			NeAACDecInit_AVcase(this, sh);

			channels = sh->channels;
			samplerate = sh->samplerate;

			bytes_read = 0;
		}
		else if ((bytes_read = NeAACDecInit(this, sh->a_in_buffer, sh->a_in_buffer_len, &samplerate, &channels)) < 0) {
			mpDebugPrint("[AAC]NeAACDecInit fail");
			NeAACDecClose(this);
			return 0;
		}

        if (bytes_read != 0) {
            /* update buffer length */
            sh->a_in_buffer_len -= bytes_read;
            memmove(sh->a_in_buffer, sh->a_in_buffer + bytes_read, sh->a_in_buffer_len);
        }

        if (aacinfo.shoutcast)
            data_blocksize = aacinfo.block_size - aacinfo.skip_bytes - bytes_read;
    }
	
#ifdef OUTPUT_PCM_DATA
    aufile = open_audio_file("output", samplerate, 2, FAAD_FMT_16BIT, OUTPUT_WAV, 0);
#endif
    faad_timer_reset(&decoder.duration);
    seek_table = (faad_seekpoint_t *)faad_malloc(SEEK_TABLE_LEN * sizeof(faad_seekpoint_t));
    memset((char *)seek_table, 0, SEEK_TABLE_LEN * sizeof(faad_seekpoint_t));
    save_seek_point(sh);

    sh->channels = (channels > 2) ? 2 : channels;
    sh->samplerate = samplerate;
    sh->samplesize = 2;

	if(AO_AV == AO_TYPE)
		sh->i_bps = aacinfo.bitrate >> 3;
	else
		sh->i_bps = (128 * 1000) >> 3;		//Playing video caseFrom Soft AAC, using fixed value!!!!!!

	Media_data.fending = 0;
	Media_data.frame = 0;              
	Media_data.ch = sh->channels;
	Media_data.srate = 	sh->samplerate;
	Media_data.bitrate = sh->i_bps;

	mpDebugPrint("Faad AAC init End, Channel: %d, samplerate: %d, Bitrate: %d", sh->channels, sh->samplerate, sh->i_bps);

    return 1;
}

#ifdef OUTPUT_PCM_DATA
audio_file *open_audio_file(char *infile, int samplerate, int channels,
                            int outputFormat, int fileType, long channelMask)
{
    char *file_ext[] = {NULL, "wav", "aif" };

    audio_file *aufile = faad_malloc(sizeof(audio_file));

    aufile->outputFormat = outputFormat;

    aufile->samplerate = samplerate;
    aufile->channels = channels;
    aufile->total_samples = 0;
    aufile->fileType = fileType;
    aufile->channelMask = channelMask;

    switch (outputFormat)
    {
    case FAAD_FMT_16BIT:
        aufile->bits_per_sample = 16;
        break;
    case FAAD_FMT_24BIT:
        aufile->bits_per_sample = 24;
        break;
    case FAAD_FMT_32BIT:
    case FAAD_FMT_FLOAT:
        aufile->bits_per_sample = 32;
        break;
    default:
        if (aufile) faad_free(aufile);
        return NULL;
    }

    aufile->toStdio = 0;
    drive = DriveGet(SD_MMC);

    if (CreateFile(drive, infile, file_ext[outputFormat])) {
        UartOutText("\r\ncreate audio file failed");
        faad_free(aufile);
        return NULL;
    }
    aufile->sndfile = FileOpen(drive);

    if (aufile->sndfile == NULL)
    {
        if (aufile) faad_free(aufile);
        return NULL;
    }

    if (aufile->fileType == OUTPUT_WAV)
    {
        write_wav_header(aufile);
    }

    return aufile;
}

int write_audio_file(audio_file *aufile, void *sample_buffer, int samples, int offset)
{
    char *buf = (char *)sample_buffer;
    switch (aufile->outputFormat)
    {
    case FAAD_FMT_16BIT:
        return write_audio_16bit(aufile, buf + offset*2, samples);
    default:
        return 0;
    }

    return 0;
}

void close_audio_file(audio_file *aufile)
{
    if (aufile->fileType == OUTPUT_WAV)
    {
        Seek(aufile->sndfile, 0);
        write_wav_header(aufile);
    }

    if (aufile->toStdio == 0)
        FileClose(aufile->sndfile);

    if (aufile) faad_free(aufile);
}

static int write_wav_header(audio_file *aufile)
{
    unsigned char header[44];
    unsigned char* p = header;
    unsigned int bytes = (aufile->bits_per_sample + 7) / 8;
    unsigned int data_size = bytes * aufile->total_samples;
    unsigned long word32;

    *p++ = 'R'; *p++ = 'I'; *p++ = 'F'; *p++ = 'F';

    word32 = (data_size + (44 - 8) < MAXWAVESIZE) ?
        (unsigned long)data_size + (44 - 8)  :  (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    *p++ = 'W'; *p++ = 'A'; *p++ = 'V'; *p++ = 'E';

    *p++ = 'f'; *p++ = 'm'; *p++ = 't'; *p++ = ' ';

    *p++ = 0x10; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;

    if (aufile->outputFormat == FAAD_FMT_FLOAT)
    {
        *p++ = 0x03; *p++ = 0x00;
    } else {
        *p++ = 0x01; *p++ = 0x00;
    }

    *p++ = (unsigned char)(aufile->channels >> 0);
    *p++ = (unsigned char)(aufile->channels >> 8);

    word32 = (unsigned long)(aufile->samplerate);
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = aufile->samplerate * bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    word32 = bytes * aufile->channels;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);

    *p++ = (unsigned char)(aufile->bits_per_sample >> 0);
    *p++ = (unsigned char)(aufile->bits_per_sample >> 8);

    *p++ = 'd'; *p++ = 'a'; *p++ = 't'; *p++ = 'a';

    word32 = data_size < MAXWAVESIZE ?
        (unsigned long)data_size : (unsigned long)MAXWAVESIZE;
    *p++ = (unsigned char)(word32 >>  0);
    *p++ = (unsigned char)(word32 >>  8);
    *p++ = (unsigned char)(word32 >> 16);
    *p++ = (unsigned char)(word32 >> 24);

    return FileWrite(aufile->sndfile, header, sizeof(header));
}

static int write_audio_16bit(audio_file *aufile, void *sample_buffer, unsigned int samples)
{
	int ret;
	unsigned int i;
	short *sample_buffer16 = (short*)sample_buffer;
	char *data = faad_malloc(samples*aufile->bits_per_sample*sizeof(char)/8);

	aufile->total_samples += samples;

	for (i = 0; i < samples; i++)
	{
		data[i*2] = (char)(sample_buffer16[i] & 0xFF);
		data[i*2+1] = (char)((sample_buffer16[i] >> 8) & 0xFF);
	}

	ret = FileWrite(aufile->sndfile, data, samples*aufile->bits_per_sample/8);

	if (data) faad_free(data);

	return ret;
}
#endif


static void *decode_aac_frame(sh_audio_t *sh, NeAACDecFrameInfo *info)
{
	NeAACDecHandle this = (NeAACDecHandle) sh->context;
	void *sample_buffer;
	int retry = 0, num_retry = (Media_data.play_time == 0) ? 50 : 400;
	unsigned char *metadata_start;
	int metadata_blocksize;

	while ((sh->a_in_buffer_len += read_data(sh)) > 0) {

		sample_buffer = NeAACDecDecode(this, info, sh->a_in_buffer, sh->a_in_buffer_len);


		/* update buffer length */
		sh->a_in_buffer_len -= info->bytesconsumed;

		if (aacinfo.shoutcast) {
			data_blocksize -= info->bytesconsumed;

			if (data_blocksize < (FAAD_BITSTREAM_CACHE >> 1)) {
				metadata_start = sh->a_in_buffer + info->bytesconsumed + data_blocksize;
				metadata_blocksize = *(metadata_start) << 4;

				if (metadata_blocksize)
					MagicPixel_AAC_FAAD_songinfo_callback(metadata_start + 1, metadata_blocksize, 0);

				metadata_blocksize++;
				sh->a_in_buffer_len -= metadata_blocksize;
				memmove(sh->a_in_buffer, sh->a_in_buffer + info->bytesconsumed, data_blocksize);
				memmove(sh->a_in_buffer + data_blocksize, metadata_start + metadata_blocksize, sh->a_in_buffer_len - data_blocksize);
				data_blocksize += aacinfo.block_size;
			} else {
				memmove(sh->a_in_buffer, sh->a_in_buffer + info->bytesconsumed, sh->a_in_buffer_len);
			}
		} else {
			//mpDebugPrint("Number memmove move: %d, number buff consumed: %d", sh->a_in_buffer_len, info->bytesconsumed);
			memmove(sh->a_in_buffer, sh->a_in_buffer + info->bytesconsumed, sh->a_in_buffer_len);
		}

	    /* calculate frame duration */
	    faad_timer_set(&info->duration, 0, this->frameLength, info->samplerate);

	    if (info->error > 0) {
	        //MP_DEBUG(NeAACDecGetErrorMessage(info->error));
	    }
	    if (!sample_buffer && (retry <= num_retry))
	        retry++;
	    else
	        return sample_buffer;
	}
	return NULL;
}

static void *decode_mp4_frame(sh_audio_t *sh, NeAACDecFrameInfo *info, unsigned int *offset)
{
    NeAACDecHandle this = (NeAACDecHandle) sh->context;
    unsigned char *buffer;
    int buffer_size;
    void *sample_buffer;
    int rc, no_gapless = 0;
    /* for gapless decoding */
    long duration;
    unsigned int sample_count, delay;

    while (decoder.sample_id < decoder.num_samples) {
        duration = mp4ff_get_sample_duration(decoder.infile, decoder.track, decoder.sample_id);
        if (!this->skip_frame) {
            /* get access unit from MP4 file */
            buffer = NULL;
            buffer_size = 0;
            rc = mp4ff_read_sample(decoder.infile, decoder.track, decoder.sample_id, &buffer, &buffer_size);
            if (rc == 0) {
                UartOutText("\r\nreading from MP4 file failed.");
                return NULL;
            }
            sample_buffer = NeAACDecDecode(this, info, buffer, buffer_size);
            if (buffer) faad_free(buffer);
        } else {
            info->channels = sh->channels;
            info->samples = decoder.framesize * info->channels;
            info->samplerate = sh->samplerate;
            info->error = 0;
            this->frame++;
            sample_buffer = sh->a_in_buffer;    /* not used */
        }

        /* for gapless decoding */
        delay = 0;
        if (!no_gapless) {
            if (decoder.sample_id == 0)    duration = 0;
            if (decoder.use_aac_length || (decoder.timescale != info->samplerate)) {
                sample_count = info->samples;
            } else {
                sample_count = (unsigned int)(duration * info->channels);
                if (!decoder.use_aac_length && !decoder.initial &&
                    (decoder.sample_id < (decoder.num_samples >> 1)) && (sample_count != info->samples)) {
                    UartOutText("\r\nMP4 seems to have incorrect frame duration, using values from AAC data.");
                    decoder.use_aac_length = 1;
                    sample_count = info->samples;
                }
            }

            if (decoder.initial && (sample_count < decoder.framesize * info->channels) && (info->samples > sample_count))
                delay = info->samples - sample_count;
        } else {
            sample_count = info->samples;
        }
        if (sample_count > 0)   decoder.initial = 0;

        decoder.sample_id++;

        if (info->error > 0) {
            //MP_DEBUG(NeAACDecGetErrorMessage(info->error));
        } else if (sample_count > 0) {
            /* calculate frame duration */
            faad_timer_set(&info->duration, 0, sample_count / info->channels, info->samplerate);
            info->samples = sample_count;
            *offset = delay;
            return sample_buffer;
        }
    }
    return NULL;
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
static int _decode_audio(sh_audio_t *sh, unsigned char *buf, int minlen, int maxlen)
{
    NeAACDecHandle this = (NeAACDecHandle) sh->context;
    NeAACDecFrameInfo frame_info;
    void *sample_buffer;
    unsigned int len = 0, offset = 0;

    while ((len < minlen) && ((len + 4096) <= maxlen)) {

#ifdef MEASURE_DECODE_TIME
        time_elapsed = GetRelativeMs();
#endif
        if (aacinfo.mp4file) {
            sample_buffer = decode_mp4_frame(sh, &frame_info, &offset);
        } else {
            sample_buffer = decode_aac_frame(sh, &frame_info);
        }
        if (!sample_buffer) {   /* decode failed or EOF */
            Media_data.fending = 1;
            return -1;
        }

        faad_timer_add(&decoder.duration, frame_info.duration);
        Media_data.play_time = faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS);
//        if ((decoder.duration.seconds - seek_table[0].duration.seconds) >= MOVIE_AUDIO_FF_SECONDS)
        if ((decoder.duration.seconds - seek_table[0].duration.seconds) >= Audio_FB_Seconds)
            save_seek_point(sh);

#ifdef MEASURE_DECODE_TIME
        time_elapsed = GetRelativeMs();
        decode_50 += time_elapsed;
        frames++;
        if (frames == 50) {
            total_frames += 50;
            UartOutText("\r\ndecode 50 frames: ");
            UartOutValue(decode_50 , 8);
            UartOutText(" ms, average: ");
            total_time += decode_50;
            UartOutValue(total_time / total_frames , 4);
            UartOutText(" ms per frame");
            frames = decode_50 = 0;
        }
#endif

#ifdef OUTPUT_PCM_DATA
        write_audio_file(aufile, sample_buffer, frame_info.samples, offset);
        if (this->frame > 43 * 5)    /* output 5 seconds PCM data */
            break;
#else
//		mpDebugPrint("[Output] Offset: %d, Samplesize: %d, Samples: %d",
//					offset, sh->samplesize, frame_info.samples);

        memcpy(buf + len, (char *)sample_buffer + (offset << 1), sh->samplesize * frame_info.samples);
        len += sh->samplesize * frame_info.samples;
#endif
        TaskYield();
    }

#ifdef OUTPUT_PCM_DATA
    close_audio_file(aufile);
    UartOutText("\r\ndecoding finished");
#endif

    return (len ? len : -1);
}

static int seek(sh_audio_t *sh, int *target_time)
{
    NeAACDecHandle this = (NeAACDecHandle) sh->context;
    NeAACDecFrameInfo frame_info;
    void *sample_buffer;
    unsigned int offset = 0;

    faad_decode_sram_init();    /* initialize SRAM for decode once */

    this->skip_frame = TRUE;
    if (*target_time >= faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS)) {
        while (faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS) < *target_time) {
            if (aacinfo.mp4file) {
                sample_buffer = decode_mp4_frame(sh, &frame_info, &offset);
            } else {
                sample_buffer = decode_aac_frame(sh, &frame_info);
            }
            if (!sample_buffer)     /* decode failed or EOF */
                break;
            faad_timer_add(&decoder.duration, frame_info.duration);
        }
    } else {
        int ret = search_seek_point(sh, *target_time);
    	if (aacinfo.mp4file) {
    	    decoder.sample_id = seek_table[ret].frame;
            /* for gapless decoding */
    	    if (decoder.sample_id == 0) {
                decoder.use_aac_length = 0;
                decoder.initial = 1;
            }
        } else {
            //faad_uninit_stream_cache();
            Free_BitstreamCache();
            //faad_init_stream_cache();
            Init_BitstreamCache(AAC_TYPE);
            MagicPixel_AAC_FAAD_fileseek_callback(seek_table[ret].file_offset);
            //faad_fill_stream_buffer();
            Fill_Cache_Buffer(AAC_TYPE);
        }
        decoder.duration = seek_table[ret].duration;
        this->frame = seek_table[ret].frame;

        sh->a_in_buffer_len = 0;
        first_time = 1;
        while (faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS) < *target_time) {
            if (aacinfo.mp4file) {
                sample_buffer = decode_mp4_frame(sh, &frame_info, &offset);
            } else {
                sample_buffer = decode_aac_frame(sh, &frame_info);
            }
            if (!sample_buffer)     /* decode failed or EOF */
                break;
            faad_timer_add(&decoder.duration, frame_info.duration);
        }

        if (ret == (SEEK_TABLE_LEN - 1)) {
            memset((char *)seek_table, 0, (SEEK_TABLE_LEN - 1) * sizeof(faad_seekpoint_t));
            seek_table[0] = seek_table[SEEK_TABLE_LEN - 1];
            save_seek_point(sh);
        }
    }
    this->skip_frame = FALSE;

    *target_time = Media_data.play_time = faad_timer_count(decoder.duration, FAAD_UNITS_MILLISECONDS);
    return CONTROL_TRUE;
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
static int control(sh_audio_t *sh, int cmd, void *arg, ...)
{
	switch (cmd)
	{
	case ADCTRL_SEEK_STREAM:
		return seek(sh, arg);
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
	NeAACDecHandle this = (NeAACDecHandle) sh->context;

	NeAACDecClose(this);
	mp4ff_close(decoder.infile);
	faad_free(decoder.mp4cb);
	faad_free(seek_table);
	Free_BitstreamCache();
	faad_working_space_uninit();
}

