/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_stream.c
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
#define MP_PERFORMANCE_TRACE 0

#include "mpTrace.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_packet.h"
#include "demux_stream.h"
#include "demux.h"



///
///@ingroup group1_Demux
///@defgroup        group_Dstream   Demux Stream
///

///
///@ingroup group_Dstream
///@brief   This function will allocate memory for structure demux_stream_t and init it.
///
///@param   int id  The stream id
///@param   demuxer_t *demuxer  The demuxer for current file format
///
///@return  demux_stream_t*
///
demux_stream_t *new_demuxer_stream(int id, demuxer_t * demuxer)
{
	demux_stream_t *ds = (demux_stream_t *) mem_malloc(sizeof(demux_stream_t));

	ds->id = id;
	ds->buffer_pos = ds->buffer_size = 0;
	ds->buffer = NULL;
	ds->pts = 0;
	ds->pts_bytes = 0;
	ds->eof = 0;
	ds->pos = 0;
	ds->dpos = 0;
	ds->pack_no = 0;
//---------------
	ds->packs = 0;
	ds->bytes = 0;
	ds->first = ds->last = ds->current = NULL;
	ds->demuxer = (demuxer_t *) demuxer;
//----------------
	ds->asf_seq = -1;
	ds->asf_packet = NULL;
//----------------
	ds->ss_mul = ds->ss_div = 0;
//----------------
	ds->sh = NULL;

	return ds;
}

///
///@ingroup group_Dstream
///@brief   This function will allocate memory for structure demux_stream_t and init it.
///
///@param   demux_stream_t *ds  This structure is used to manage the demux stream. 
///
///@return  No
///
void free_demuxer_stream(demux_stream_t * ds)
{
	if (ds)
	{
		ds_free_packs(ds);
		mem_free(ds);
	}
}

/*put packet "dp"  to the "last packet"(if last is not NULL) of demux_stream_t   ds 
/ otherwise preemp to first packet 
*/
void ds_add_packet(register demux_stream_t * ds, register demux_packet_t * dp)
{
//    demux_packet_t* dp=new_demux_packet(len);
//    stream_read(stream,dp->buffer,len);
//    dp->pts=pts; //(float)pts/90000.0f;
//    dp->pos=pos;


//#ifdef PURE_VIDEO
#if (PURE_VIDEO == 1)
	if (ds == ds->demuxer->audio)
	{
		free_demux_packet(dp);
		return;
	}
#endif


	// append packet to DS stream:
	++ds->packs;

	ds->bytes += dp->len;
	if (ds->last)
	{
		// next packet in stream
		ds->last->next = dp;
		ds->last = dp;
	}
	else
	{
		// first packet in stream
		ds->first = ds->last = dp;
	}

	// Deming added
#ifdef _TESTING_
	log_write((ds == ds->demuxer->audio) ? 1 : 0, dp->buffer, dp->len);
#endif
}

/*
This function will add read "len" from stream,
@para  stream: stream buffer, contains source file
@para  pos: 	offset in the file
@para  len: 	read len, define in mov_sample_t* samples stsz atom
@local  dp:	new a packet to store preemp  in demux_stream_t * ds
*/  
int ds_read_packet(demux_stream_t * ds, stream_t * stream, int len, float pts, unsigned int pos,
					int flags)
{
	const demuxer_t * const demux = ds->demuxer;
	demux_packet_t *const dp = (demux_packet_t *) new_demux_packet(len);
	int len_temp;
	if (dp)
	{
		len_temp =len;
		len = stream_read(stream, (char *) dp->buffer, len);
//		MP_PERFORMANCE_TRC("ds_2\t(t:%d)",ELAPSE_TIME);
		if (len != 0)
		{
			resize_demux_packet(dp, len);
			dp->pts = pts;
			dp->pos = pos;
			dp->flags = flags;
#ifdef _TESTING_
		//log_write((ds == ds->demuxer->audio) ? 1 : 0,dp->buffer,len);
#endif

		// append packet to DS stream:
			ds_add_packet(ds, dp);
			return 0;
		}
		else
		{
			free_demux_packet(dp);
			return 1;
		}

	}
	else
	{
		SystemSetErrEvent(ERR_VIDEO_STREAM);
	}
	MP_PERFORMANCE_TRC("ds_end(%s)\tpacks(%d,%d)(t:%d)",ds == ds->demuxer->audio ? "A" : "V",demux->audio->packs,demux->video->packs,ELAPSE_TIME);
}

// ds_read_packet2:temporary solution for passing floating pts parameter error bug
void ds_read_packet2(demux_stream_t * ds, stream_t * stream, int len, DWORD pts, unsigned int  pos,
					int flags)
{
	demux_packet_t *dp = (demux_packet_t *) new_demux_packet(len);

	if (dp)
	{
		len = stream_read(stream, (char *) dp->buffer, len);
		resize_demux_packet(dp, len);
		dp->pts = (float)pts/1000.0f;			//(float)pts/90000.0f;
		dp->pos = pos;
		dp->flags = flags;

#ifdef _TESTING_
		// log_write((ds == ds->demuxer->audio) ? 1 : 0,dp->buffer,len);
#endif

		// append packet to DS stream:
		ds_add_packet(ds, dp);
	}
	else
	{
		SystemSetErrEvent(ERR_VIDEO_STREAM);
	}
}
// ds_read_packet_pureh264:this function only allow go forward

void ds_read_packet_pureh264(demux_stream_t * ds,BYTE * tBuf1, int len, DWORD pts, unsigned int  pos,int flags)
{
	demux_packet_t *dp = (demux_packet_t *) new_demux_packet(len);

	if (dp)
	{               
		
		memcpy((char *) dp->buffer, tBuf1, len);
		dp->pts = (float)pts/1000.0f;			//(float)pts/90000.0f;
		dp->pos = pos;
		dp->flags = flags;
		ds_add_packet(ds, dp);
	}
	else
	{
		SystemSetErrEvent(ERR_VIDEO_STREAM);
	}
}

static void inline error_pack(demux_stream_t * const ds)
{
	MP_DEBUG("error_pack: Block %s", ds == ds->demuxer->audio ? "Audio" : "Video");
	ds->buffer_pos = ds->buffer_size = 0;
	ds->buffer = NULL;
	ds->current = NULL;
	ds->eof = 0;
}

///
///@brief
///@retval  0 EOF
///@retval  1 succesfull
///
int ds_fill_buffer(register demux_stream_t * ds)
{
	register demuxer_t * const demux = ds->demuxer;

	if (ds->current)
	{
		free_demux_packet(ds->current);
		ds->current = NULL;
	}
	
	if (ds == demux->audio)		{MP_DEBUGN("ds_fill_buffer(A)");}
	else if (ds == demux->video)	{MP_DEBUGN("ds_fill_buffer(V)");}
	else if (ds == demux->sub)	{MP_DEBUGN("ds_fill_buffer(S) called");}
	else						{MP_DEBUGN("ds_fill_buffer(unknown 0x%X) called",(unsigned int) ds);}
	MP_DEBUGN(",packs(%d,%d)",demux->audio->packs,demux->video->packs);
	#if LOCAL_DEBUG_ENABLE
		#if MP_PERFORMANCE_TRACE
			MP_PERFORMANCE_TRC("(t:%d)",ELAPSE_TIME);
		#else
			MP_DEBUGN("\r\n");
		#endif
	#endif
	while (1)
	{
		if (ds->packs)
		{
			register demux_packet_t *p = ds->first;
			if(p == NULL || p->buffer == NULL)
			{
			    if (ds == demux->audio)
				    MP_ALERT("audio [ds_fill_buffer] Can not get buffer --- System Hanged!!");
				else if (ds == demux->video)
				    MP_ALERT("video [ds_fill_buffer] Can not get buffer --- System Hanged!!");
				else if (ds == demux->sub)
				    MP_ALERT("subtitle [ds_fill_buffer] Can not get buffer --- System Hanged!!");
				else 
				    MP_ALERT("unknown [ds_fill_buffer] Can not get buffer --- System Hanged!!");
				mpDebugPrint("p = 0x%x, p->buffer = %d", p, p->buffer);
				mpDebugPrint("TaskGetId() = %d", TaskGetId()); 
				mem_Zones(1);
				TimerDelay(2000);
  				__asm("break 100");
			}
			int p_len = p->len;
			// copy useful data:
			ds->buffer = p->buffer;
			ds->buffer_pos = 0;
			ds->buffer_size = p_len;
			ds->pos = p->pos;
			ds->dpos += p_len;	// !!!
			++ds->pack_no;
			if (p->pts)
			{
				ds->pts = p->pts;
				ds->pts_bytes = 0;
			}
			ds->pts_bytes += p_len;	// !!!
			ds->flags = p->flags;
			// unlink packet:
			ds->bytes -= p_len;
			ds->current = p;
			ds->first = p->next;
			if (!ds->first)	ds->last = NULL;
			--ds->packs;

			MP_PERFORMANCE_TRC("ds_fill_buffer(%s)return,packs(%d,%d) (t:%d)", ds == ds->demuxer->audio ? "A" : "V",demux->audio->packs,demux->video->packs,ELAPSE_TIME);
			return 1;			//ds->buffer_size;
		}

		//mpDebugPrint("audio packs=%d, bytes=0x%x", demux->audio->packs, demux->audio->bytes);
		//mpDebugPrint("video packs=%d, bytes=0x%x", demux->video->packs, demux->video->bytes);
		if (demux->audio->packs >= MAX_PACKS || demux->audio->bytes >= MAX_PACK_BYTES)
		{
			//audio underflow, no enough memory, low speed performance,...etc
			MP_ALERT("Too many audio packets in the buffer: (%d in %d bytes)",demux->audio->packs, demux->audio->bytes);
			//MP_ALERT("Maybe you are playing a non-interleaved stream/file or the codec failed?\r\nFor AVI files, try to force non-interleaved mode with the -ni option.");
			error_pack(ds);
			return 0;
		}
	 	if (demux->video->packs >= MAX_PACKS || demux->video->bytes >= MAX_PACK_BYTES)
		{
			//audio underflow, no enough memory, low speed performance,...etc
			MP_ALERT("Too many video packets in the buffer: (%d in %d bytes)",demux->video->packs, demux->video->bytes);
			//MP_ALERT("Maybe you are playing a non-interleaved stream/file or the codec failed?\r\nFor AVI files, try to force non-interleaved mode with the -ni option.");
			error_pack(ds);
			return 0;
		}
		
		if (!ds->demuxer->fill_buffer(ds))//depend on file format if  mp4 fill bufer deined in demux_mov.c
		{
			//MP_ALERT("ds_fill_buffer()->demux_fill_buffer() failed");
			break;	// EOF
		}
	}

	#if (MOVIE_EVENT_DEBUG & MOVIE_EVENT_DEBUG_FUNC)
		mpDebugPrint("%s: EOF reached (stream: %s)", __FUNCTION__, ds == demux->audio ? "audio" : "video");
	#endif
	error_pack(ds);
	ds->eof = 1;

	return 0;
}

int ds_read_data(demux_stream_t * ds, unsigned char *mem, int len)
{
	int x;
	int bytes = 0;

	while (len > 0)
	{
		x = ds->buffer_size - ds->buffer_pos;
		if (x == 0)
		{
			if (!ds_fill_buffer(ds))
			{
				return bytes;
			}
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

void ds_free_packs(register demux_stream_t * ds)
{
	if (!ds)	return;
	demux_packet_t *dp = ds->first;

	while (dp)
	{
		demux_packet_t *dn = dp->next;

		free_demux_packet(dp);
		dp = dn;
	}
	if (ds->asf_packet)
	{
		// free unfinished .asf fragments:
		mem_free(ds->asf_packet->buffer);
		mem_free(ds->asf_packet);
		ds->asf_packet = NULL;
	}
	ds->first = ds->last = NULL;
	ds->packs = 0;				// !!!!!
	ds->bytes = 0;
	if (ds->current)
	{
		free_demux_packet(ds->current);
		ds->current = NULL;
	}
	ds->buffer = NULL;
	ds->buffer_pos = ds->buffer_size;
	ds->pts = 0;
	ds->pts_bytes = 0;
}

int ds_get_packet(register demux_stream_t * ds, unsigned char **start)
{
	while (1)
	{
		int len;

		if (ds->buffer_pos >= ds->buffer_size)		//last read ok to the end
		{
			if (!ds_fill_buffer(ds))
			{
				// EOF
				*start = NULL;
				return -1;
			}
		}
		len = ds->buffer_size - ds->buffer_pos;
		*start = &ds->buffer[ds->buffer_pos];
		ds->buffer_pos += len;

		return len;
	}
}


float ds_get_next_pts(demux_stream_t * ds)
{
	demuxer_t *demux = ds->demuxer;

	while (!ds->first)
	{
		if (demux->audio->packs >= MAX_PACKS || demux->audio->bytes >= MAX_PACK_BYTES)
		{
			return -1;
		}
		if (demux->video->packs >= MAX_PACKS || demux->video->bytes >= MAX_PACK_BYTES)
		{
			return -1;
		}
		// if(!demuxer_p(ds)->fill_buffer())
		if (!ds->demuxer->fill_buffer(ds))
			return -1;
	}
	return ds->first->pts;
}
#endif
