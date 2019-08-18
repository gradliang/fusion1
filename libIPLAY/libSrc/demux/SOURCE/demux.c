/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:
*
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "stheader.h"
#include "filetype.h"  
#if (AUDIO_ON && WAV_ENABLE)
extern BYTE buflen_softaviwave;
#endif
extern FILE_TYPE_T g_FileType;

///
///@mainpage Demux User Guide
///The Demultiplexer ("demux") will examine the file headers, and parse the
///multimedia stream that are delivered in pull-mode (file playback) into separate
///video and audio streams.
///
///The responsibilities of demux:
///
///1.   Examine the file header, and figure out its format.
///
///2.   Get the audio and video streams parameters.
///
///3.   Parse the file into separate video frames and audio samples.
///
///4.   Send video frames and audio samples to correct decoder.
///
///NOTE:
///Currently supported file format:AVI, ASF, MOV, MPEG, MP3, AAC
///

///
///@defgroup group1_Demux Demux
///


///
///@ingroup group1_Demux
///@brief   This static function will check whether the stream's format match the demux.
///         This is just the default implementation which does nothing.
///         Each file format has its own implementation!!!!
///@param   stream_t* s     The stream for checking
///@param   DEMUX_T t   The demux chosed for this stream.
///
///@return  0 for fail, 1 for success.
///
static int _check_type(stream_t * s, DEMUX_T t)
{
	return 0;
}

///
///@ingroup group1_Demux
///@brief   This static function will open the demux.
///         This is just the default implementation which does nothing.
///         Each file format has its own implementation!!!!
///@param   No
///
///@return  0 for fail, 1 for success.
///
static int _open()
{
	return 0;
}

///
///@ingroup group1_Demux
///@brief   This static function will read data from stream.
///         This is just the default implementation which does nothing.
///         Each file format has its own implementation!!!!
///@param   demux_stream_t * ds
///
///@return  0 for fail, 1 for success.
///
static int _fill_buffer(demux_stream_t * ds)
{
	return 0;
}

///
///@ingroup group1_Demux
///@brief   This static function will seek in the stream according the input parameters.
///         This is just the default implementation which does nothing.
///         Each file format has its own implementation!!!!
///@param   demux_stream_t * ds
///
///@return  0 for fail, 1 for success.
///
static int _seek(float relative_seek_seconds, int flags)
{
	return 0;
}

static int _control(int cmd, void *arg)
{
	return DEMUXER_CTRL_NOTIMPL;
}

static int _close()
{
	return 1;
}


static float _get_pts_by_relframe(int rel_seek_samples, int flags)
{
	return 0;
}

static int _get_movie_info(BYTE * buffer, int command)
{
}

demuxer_t *new_base_demux()
{
	register demuxer_t *d = (demuxer_t *) mem_malloc(sizeof(demuxer_t));

	memset(d, 0, sizeof(demuxer_t));
	d->seekable = 1;
	d->audio = new_demuxer_stream(-1, d);
	d->video = new_demuxer_stream(-1, d);
	d->video_3D = new_demuxer_stream(-1, d);
	d->check_type = _check_type;
	d->open = _open;
	d->fill_buffer = _fill_buffer;
	d->seek = _seek;
	d->control = _control;
	d->close = _close;
	d->get_pts_by_relframe = _get_pts_by_relframe;
	d->get_movie_info = _get_movie_info;
	// stream_reset(stream);
	// stream_seek(stream,stream->start_pos);
	return d;
}

#if PURE_VIDEO
sh_audio_t new_sh_audio_wrapper;
#endif

sh_audio_t *new_sh_audio(demuxer_t * const demuxer, const int id)
{
#if (PURE_VIDEO == 0)
	if (id > MAX_A_STREAMS - 1 || id < 0)	return NULL;
	if (demuxer->a_streams[id])
	{

	}
	else
	{
		sh_audio_t * const sh = (sh_audio_t *)mem_malloc(sizeof(sh_audio_t));
		memset(sh, 0, sizeof(sh_audio_t));
		demuxer->sh_audio = demuxer->a_streams[id] = (void *) sh;
		//sh->aid = aid;
        sh->ds = demuxer->audio;
	}
	return demuxer->a_streams[id];
#else
	memset((BYTE *) &new_sh_audio_wrapper, 0, sizeof(sh_audio_t));

	/*	Testing code (Do not remove this section please.. C.W 090212)
	if (id > MAX_A_STREAMS - 1 || id < 0)
	{
		return NULL;
	}
	if (demuxer->a_streams[id]){}
	else
	{
		demuxer->a_streams[id] = &new_sh_audio_wrapper;
		memset((BYTE *) &new_sh_audio_wrapper, 0, sizeof(sh_audio_t));
		demuxer->sh_audio = demuxer->a_streams[id];
	}
	*/

	return (void *) &new_sh_audio_wrapper;
#endif
}

static void free_sh_audio(sh_audio_t * const sh)
{
	if (!sh) return;

	if (sh->wf)
	{
		mem_free(sh->wf);
		sh->wf = NULL;
	}

	if (sh->codecdata)
	{
		mem_free(sh->codecdata);
		sh->codecdata = NULL;
	}

	//FIXME: need free a_in_buffer/a_buffer/a_out_buffer or not?

	mem_free(sh);
}

static sh_video_t* new_sh_video_vid(demuxer_t * const demuxer, const int id, const int vid, const int if3D)
{
	if (id > MAX_V_STREAMS - 1 || id < 0)
		return NULL;
	
	if (!demuxer->v_streams[id])
	{
		sh_video_t * const sh = (sh_video_t *)mem_malloc(sizeof(sh_video_t));
		memset(sh, 0, sizeof(sh_video_t));
		demuxer->v_streams[id] = (void *) sh;
		if3D?demuxer->sh_video_3D:demuxer->sh_video = (void *) sh;
		
		sh->vid = vid;
        sh->ds = demuxer->video;
	}
	return demuxer->v_streams[id];
}

sh_video_t *new_sh_video(demuxer_t * const demuxer, const int id)
{
	return new_sh_video_vid(demuxer, id, id, 0);
}

sh_video_t *new_sh_video_3D(demuxer_t * const demuxer, const int id)
{
	return new_sh_video_vid(demuxer, id, id, 1);
}

static void free_sh_video(sh_video_t * const sh)
{
	if (sh->bih)
		mem_free(sh->bih);
	
	
	if (FILE_TYPE_MOV == g_FileType)
	{
	    sh->ImageDesc = NULL;
	}
	else
	{
	   if (sh->ImageDesc) 
	       mem_free(sh->ImageDesc);
	}   
	
	mem_free(sh);
	
}

///
///@ingroup group1_Demux
///@brief   This function will free all the memory space occupied by demuxer's fields, and reset these fields to NULL.
///@param   demuxer_t *demuxer  The demuxer for current file format
///@return  None
///
void free_demuxer(demuxer_t * const demuxer)
{
	int i;

	if (!demuxer)
		return;
	demuxer->close();//Free all format specific memory including priv

	for (i = 0; i < MAX_A_STREAMS; i++)//maybe only num_a_streams needed
	{
		if (demuxer->a_streams[i])
		{
			free_sh_audio(demuxer->a_streams[i]);
			demuxer->a_streams[i] = 0;
		}
	}
	for (i = 0; i < MAX_V_STREAMS; i++)
	{
		if (demuxer->v_streams[i])
		{
			free_sh_video(demuxer->v_streams[i]);
			demuxer->v_streams[i] = 0;
		}
	}
	demuxer->sh_audio = 0;
	demuxer->sh_video = 0;

	free_demuxer_stream(demuxer->audio);
	demuxer->audio = 0;
	free_demuxer_stream(demuxer->video);
	demuxer->video = 0;
	free_demuxer_stream(demuxer->video_3D);
	demuxer->video_3D = 0;
	if (demuxer->info)
	{
		for (i = 0; demuxer->info[i] != NULL; i++)
		{
			mem_free(demuxer->info[i]);
			demuxer->info[i] = 0;
		}
		mem_free(demuxer->info);
		demuxer->info = 0;
	}

	mem_free(demuxer);
}

void demux_flush(demuxer_t *demuxer)
{
	ds_free_packs(demuxer->video);
	ds_free_packs(demuxer->video_3D);
	ds_free_packs(demuxer->audio);
	ds_free_packs(demuxer->sub);
}


///
///@ingroup group1_Demux
///@brief   This function will clear audio&video stream buffer and call demuxer's seek function for seeking.
///
///@param   demuxer_t *demuxer  The demuxer for current file format
///@param   float rel_seek_secs     The PTS for seeking
///@param   int flags               Seeking type
///         flags=3, absolute seek by percentage
///         flags=2, relatvie seek by percentage
///         flags=1, Absolute seek to a specific timestamp in seconds
///
///@return  0 for fail, 1 for success.
///
int demux_seek(register demuxer_t * const demuxer, const float rel_seek_secs, const int flags)
{
	if (demuxer)
	{
		demux_stream_t *d_audio = demuxer->audio;
		demux_stream_t *d_video = demuxer->video;
		demux_stream_t *d_video_3D = demuxer->video_3D;
		sh_audio_t *sh_audio = d_audio->sh;
		sh_video_t *sh_video = d_video->sh;
		sh_video_t *sh_video_3D = d_video_3D->sh;

		if (!demuxer->seekable)
			return 0;

		// clear demux buffers:
		if (sh_audio)
		{
			if (sh_audio->format == 0x1 || sh_audio->format ==0x74776f73 || sh_audio->format ==0x736f7774)
			{
				#if (AUDIO_ON && WAV_ENABLE)
				buflen_softaviwave = 1;
				#endif
			}
			ds_free_packs(d_audio);
			sh_audio->a_buffer_len = 0;
		}
		ds_free_packs(d_video);
		ds_free_packs(d_video_3D);

		demuxer->stream->eof = 0;	// clear eof flag
		demuxer->video->eof = 0;
		demuxer->video_3D->eof = 0;
		demuxer->audio->eof = 0;

		if (sh_audio)
			sh_audio->delay = 0;
		if (sh_video)
			sh_video->timer = 0;
		if (sh_video_3D)
			sh_video_3D->timer = 0;
		//reset_av_player_controller();
		if(demuxer->seek != NULL)
		{
			demuxer->seek(rel_seek_secs, flags);
			av_resync();
		}	
		demuxer->seeked = 1;
	}
	return 1;
}

int demux_control(demuxer_t * demuxer, int cmd, void *arg)
{
	if (demuxer && demuxer->control)
	{
		return demuxer->control(cmd, arg);
	}
	return 0;
}

unsigned long demuxer_get_time_length(demuxer_t * demuxer)
{
	unsigned long get_time_ans;

	if (demux_control(demuxer, DEMUXER_CTRL_GET_TIME_LENGTH, (void *) &get_time_ans) <= 0)
	{
		get_time_ans = 0;
	}
	return get_time_ans;
}

int demuxer_get_percent_pos(demuxer_t * demuxer)
{
	int ans;

	if (demux_control(demuxer, DEMUXER_CTRL_GET_PERCENT_POS, &ans) <= 0)
	{
		ans = 0;
	}
	if (ans > 100 || ans < 0)
		ans = 0;
	return ans;
}


int demux_read_data(register demux_stream_t * ds, register unsigned char *mem, register int len)
{
	int x;
	int bytes = 0;

	while (len > 0)
	{
		x = ds->buffer_size - ds->buffer_pos;
		if (x == 0)
		{
			if (!ds_fill_buffer(ds))
				return bytes;
		}
		else
		{
			if (x > len)
				x = len;
			if (mem)
			{
			    #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
                    mmcp_memcpy_polling(mem + bytes, &ds->buffer[ds->buffer_pos], x);
                #else	
				    memcpy(mem + bytes, &ds->buffer[ds->buffer_pos], x);
			    #endif
			}	
			bytes += x;
			len -= x;
			ds->buffer_pos += x;
		}
	}
	return bytes;
}

/**
 * \ingroup group1_Demux
 * \brief read data until the given 3-byte pattern is encountered, up to maxlen
 * \param mem memory to read data into, may be NULL to discard data
 * \param maxlen maximum number of bytes to read
 * \param read number of bytes actually read
 * \param pattern pattern to search for (lowest 8 bits are ignored)
 * \return whether pattern was found
 */
int demux_pattern_3(demux_stream_t *ds, unsigned char *mem, int maxlen,
                    int *read, uint32_t pattern)
{
	register uint32_t head = 0xffffff00;
	register uint32_t pat = pattern & 0xffffff00;
	int total_len = 0;
	do
	{
		register unsigned char *ds_buf = &ds->buffer[ds->buffer_size];
		int len = ds->buffer_size - ds->buffer_pos;
		register long pos = -len;
		if (pos >= 0)   // buffer is empty
		{
			ds_fill_buffer(ds);
			continue;
		}
		do
		{
			head |= ds_buf[pos];
			head <<= 8;
		}
		while (++pos && head != pat);
		len += pos;
		if (total_len + len > maxlen)
			len = maxlen - total_len;
		len = demux_read_data(ds, mem ? &mem[total_len] : NULL, len);
		total_len += len;
	}
	while ((head != pat || total_len < 3) && total_len < maxlen && !ds->eof);
	if (read)
		*read = total_len;
	return total_len >= 3 && head == pat;
}
#endif
