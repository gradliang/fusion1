/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_avi.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: AVI format file demux implementation 
*        
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/

#include "global612.h"
#if VIDEO_ON

#include "demux_types.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux_packet.h"
#include "demux.h"
#include "stheader.h"
#include "filter_graph.h"
#include "avi/avifmt.h"
#include "avi/aviheader.h"

///
///@ingroup group1_Demux
///@defgroup    BOX AVI AVI File Format
///

extern demuxer_t *new_base_demux();


// Avi demuxer
static demuxer_t *demux = 0;

extern AVI_Content avicontent;

#define _AVI        1			// interleaved
#define _AVI_NI     2			// non-interleaved
#define _AVI_NI_NI  3			// non-interleaved, no index

int current_avi_format = _AVI;

// PTS:  0=interleaved  1=BPS-based
int pts_from_bps = 1;

// Select ds from ID
demux_stream_t *demux_avi_select_stream(demuxer_t * demux, unsigned int id)
{
	int stream_id = avi_stream_id(id);

	//MP_DPF("%s demux_avi_select_stream(%d)  {a:%d/v:%d}\n",__FILE__,stream_id,
	//     demux->audio->id,demux->video->id);

	if (demux->video->id == -1)
	{
		if (demux->v_streams[stream_id])
			demux->video->id = stream_id;
	}
	else if ((stream_id != demux->video->id) && (demux->video_3D->id == -1))
	{
		if (demux->v_streams[stream_id])
			demux->video_3D->id = stream_id;
	}

	if (demux->audio->id == -1)
		if (demux->a_streams[stream_id])
			demux->audio->id = stream_id;

	if (stream_id == demux->audio->id)
	{
		if (!demux->audio->sh)
		{
			sh_audio_t *sh;
			avi_priv_t *priv = demux->priv;

			sh = demux->audio->sh = demux->a_streams[stream_id];
			if (sh->wf)
			{
				priv->audio_block_size = sh->wf->nBlockAlign;
				if (!priv->audio_block_size)
				{
					// for PCM audio we can calculate the blocksize:
					if (sh->format == 1)
						priv->audio_block_size = sh->wf->nChannels * (sh->wf->wBitsPerSample >> 3);
					else
						priv->audio_block_size = 1;	// hope the best...
				}
				else
				{
					// workaround old mencoder's bug:
					if (sh->audio.dwSampleSize == 1 && sh->audio.dwScale == 1 &&
						(sh->wf->nBlockAlign == 1152 || sh->wf->nBlockAlign == 576))
					{
						priv->audio_block_size = 1;
					}
				}
			}
			else
			{
				priv->audio_block_size = sh->audio.dwSampleSize;
			}
//  MP_DPF("&&&&& setting blocksize to %d &&&&&\n",priv->audio_block_size);
		}
		return demux->audio;
	}
	if (stream_id == demux->video->id)
	{
		if (!demux->video->sh)
		{
			demux->video->sh = demux->v_streams[stream_id];
		}
		return demux->video;
	}
	if (stream_id == demux->video_3D->id)
	{
		if (!demux->video_3D->sh)
		{
			demux->video_3D->sh = demux->v_streams[stream_id];
		}
		return demux->video_3D;
	}

	if (id != mmioFOURCC('J', 'U', 'N', 'K'))
	{
		// unknown
		//abort();
	}
	return NULL;
}

static int valid_fourcc(unsigned int id)
{
	unsigned char *fcc = (unsigned char *) (&id);

//#define FCC_CHR_CHECK(x) (x<48 || x>=96)
#define FCC_CHR_CHECK(x) (x<48 || x>122)
	if (FCC_CHR_CHECK(fcc[0]))
		return 0;
	if (FCC_CHR_CHECK(fcc[1]))
		return 0;
	if (FCC_CHR_CHECK(fcc[2]))
		return 0;
	if (FCC_CHR_CHECK(fcc[3]))
		return 0;
	return 1;
#undef FCC_CHR_CHECK
}

static int choose_chunk_len(unsigned int len1, unsigned int len2)
{
	// len1 has a bit more priority than len2. len1!=len2
	// Note: this is a first-idea-logic, may be wrong. comments welcomed.

	// prefer small frames rather than 0
	if (!len1)
		return (len2 > 0x80000) ? len1 : len2;
	if (!len2)
		return (len1 > 0x100000) ? len2 : len1;

	// choose the smaller value:
	return (len1 < len2) ? len1 : len2;
}

static int
demux_avi_read_packet(demuxer_t * demux, demux_stream_t * ds, unsigned int id, unsigned int len,
					  int idxpos, int flags)
{
	avi_priv_t *priv = demux->priv;
	int skip;
	float pts = 0;

	if (ds == demux->audio)
	{
		if (priv->pts_corrected == 0)
		{
//          MP_DPF("\rYYY-A  A: %5.3f  V: %5.3f  \n",priv->avi_audio_pts,priv->avi_video_pts);
			if (priv->pts_has_video)
			{
				// we have video pts now
				float delay = 0;

				if (((sh_audio_t *) (ds->sh))->wf->nAvgBytesPerSec)
					delay =
						(float) priv->pts_corr_bytes /
						((sh_audio_t *) (ds->sh))->wf->nAvgBytesPerSec;
				//priv->pts_correction=-priv->avi_audio_pts+delay;
				priv->pts_correction = delay - priv->avi_audio_pts;
				priv->avi_audio_pts += priv->pts_correction;
				priv->pts_corrected = 1;
			}
			else
				priv->pts_corr_bytes += len;
		}

		if (pts_from_bps)
		{
			pts = priv->audio_block_no *
				(float) ((sh_audio_t *) demux->audio->sh)->audio.dwScale /
				(float) ((sh_audio_t *) demux->audio->sh)->audio.dwRate;
		}
		else
			pts = priv->avi_audio_pts;	//+priv->pts_correction;

		priv->avi_audio_pts = 0;
		// update blockcount:
		priv->audio_block_no += priv->audio_block_size ?
			((len + priv->audio_block_size - 1) / priv->audio_block_size) : 1;
//      MP_DPF("\raudio_block_no=%d      \n",priv->audio_block_no);
	}
	else if ((ds == demux->video) || (ds == demux->video_3D))
	{
		// video
		if (priv->skip_video_frames > 0)
		{
			// drop frame (seeking)
			--priv->skip_video_frames;
			ds = NULL;
		}

		pts = priv->avi_video_pts = priv->video_pack_no *
			(float) ((sh_video_t *) demux->video->sh)->video.dwScale /
			(float) ((sh_video_t *) demux->video->sh)->video.dwRate;
//          MP_DPF("\rYYY-V  A: %5.3f  V: %5.3f  \n",priv->avi_audio_pts,priv->avi_video_pts);

		priv->avi_audio_pts = priv->avi_video_pts + priv->pts_correction;
		priv->pts_has_video = 1;

		if (ds)
			++priv->video_pack_no;

		//MP_DPF("read  pack_no: %d  pts %5.3f  \n",demux->video->pack_no+demux->video->packs,pts);
	}

	skip = (len + 1) & (~1);	// total bytes in this chunk

	if (ds)
	{
		if (ds == demux->video)    // for CASIO
		{
			sh_video_t * const sh_video = (sh_video_t *)demux->video->sh;
			if ((priv->video_pack_no == 1) && (sh_video->bih->biSize > sizeof(BITMAPINFOHEADER))) 
			{
				const unsigned char * const stream_header = (unsigned char *)sh_video->bih + sizeof(BITMAPINFOHEADER);
				const int stream_header_len = sh_video->bih->biSize - sizeof(BITMAPINFOHEADER);
				demux_packet_t * const dp = (demux_packet_t *) new_demux_packet(len + stream_header_len);

				memcpy(dp->buffer, stream_header, stream_header_len);
				unsigned char * const frame= (unsigned char *) (dp->buffer + stream_header_len);
				stream_read(demux->stream, (char *)frame, len);
				dp->pts = pts;
				dp->flags = flags;
				dp->pos = idxpos;
				ds_add_packet(ds, dp);
			}
			else
			{
				ds_read_packet(ds, demux->stream, len, pts, idxpos, flags);
			}
		}
		else if (ds == demux->video_3D) 
		{
			ds_read_packet(ds, demux->stream, len, pts, idxpos, flags);
		}
		else
//			ds_read_packet(ds, demux->stream, len, pts, idxpos, flags);
		{
			if (((current_avi_format == _AVI_NI) || (current_avi_format == _AVI_NI_NI))&& (demux->stream_a != NULL))
				ds_read_packet(ds, demux->stream_a, len, pts, idxpos, flags);
			else
				ds_read_packet(ds, demux->stream, len, pts, idxpos, flags);
		}
		skip -= len;
	}

	if (skip)
	{
//		stream_skip(demux->stream, skip);
		if ((ds == demux->audio) && ((current_avi_format == _AVI_NI)|| (current_avi_format == _AVI_NI_NI)) && (demux->stream_a != NULL))
			stream_skip(demux->stream_a, skip);
		else
			stream_skip(demux->stream, skip);
	}

	return ds ? 1 : 0;
}



// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
static int _fill_buffer_ni(demux_stream_t * ds)
{
    //mpDebugPrint("_fill_buffer_ni");
	avi_priv_t *priv = demux->priv;
	unsigned int id = 0;
	unsigned int len;
	stream_t *stream;

//int max_packs=128;
	int ret = 0;

	do
	{
		int flags = 1;
		AVIINDEXENTRY *idx = NULL;
		int idx_pos = 0;

		demux->filepos = stream_tell(demux->stream);

		if (ds == demux->video)
			idx_pos = priv->idx_pos_v++;
		else if (ds == demux->video_3D)
			idx_pos = priv->idx_pos_v_3D++;
		else if (ds == demux->audio)
			idx_pos = priv->idx_pos_a++;
		else
			idx_pos = priv->idx_pos++;
		if (priv->idx_size > 0 && idx_pos < priv->idx_size)
		{
			unsigned int pos;

			idx = &((AVIINDEXENTRY *) priv->idx)[idx_pos];

			//if (idx->dwFlags & AVIIF_LIST)
			//{
			//	// LIST
			//	continue;
			//}
			if (ds && demux_avi_select_stream(demux, idx->ckid) != ds)
			{
				continue;		// skip this chunk
			}

			pos = priv->idx_offset + (unsigned long) idx->dwChunkOffset;			
			if ((pos < demux->movi_start || pos >= demux->movi_end)				
				&& (demux->movi_end > demux->movi_start))
			{
				continue;
			}

            // modified for conitune playing video when audio stream is NULL
			//if ((ds == demux->audio) && (demux->stream_a != NULL))
			if (ds == demux->audio)
			{
                if (demux->stream_a != NULL)
                {
				    stream = demux->stream_a;
                }
				else
				{
				    stream = demux->stream;//continue;
				}	
			}	
			// end of modified
			else
				stream = demux->stream;

			stream_seek(stream, pos);

			id = stream_read_dword_le(stream);

			if (stream_eof(stream))
				return 0;

			if (id != idx->ckid)
			{
				#if 0
				if (valid_fourcc(idx->ckid))
					id = idx->ckid;	// use index if valid
				else if (!valid_fourcc(id))
				#endif
					continue;	// drop chunk if both id and idx bad
			}
			len = stream_read_dword_le(stream);
			if (0 == len)				
			{
			   sh_video_t *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	           unsigned int four_cc = sh_video->format;
               if ((JPEG == four_cc) ||					
                   (jpeg == four_cc) ||	
  	               (MJPG == four_cc) ||	
	               (mjpg == four_cc) 
	              )
	           {
	               // for many video chunk length =0 of V1.0-30FPS-44100Hz-MULAW.avi 
			       if (ds != demux->video)
			       {
			           continue;
			       }
			       else
			       {
			           AVIINDEXENTRY *temp_idx;
			           int temp_idx_pos1 = idx_pos;
					   int temp_idx_pos2 = idx_pos;
				       static int last_idx_pos = 0;

					   do
			           {
			               temp_idx_pos1--;
			               temp_idx = &((AVIINDEXENTRY *) priv->idx)[temp_idx_pos1];				           
			           } while (((temp_idx->ckid & 0xFFFF0000) == 0x62770000) || (0 == temp_idx->dwChunkLength));

					   do
			           {
			               temp_idx_pos2++;
			               temp_idx = &((AVIINDEXENTRY *) priv->idx)[temp_idx_pos2];				           
			           } while (((temp_idx->ckid & 0xFFFF0000) == 0x62770000) || (0 == temp_idx->dwChunkLength));

					   if ((((temp_idx_pos2 - idx_pos) >= (idx_pos - temp_idx_pos1)) && (temp_idx_pos1 >= last_idx_pos)) || (temp_idx_pos2 >= priv->idx_size))
					   {
					       last_idx_pos = temp_idx_pos1;						   
					   }
					   else
					   {
					       last_idx_pos = temp_idx_pos2;
					   }
					   temp_idx = &((AVIINDEXENTRY *) priv->idx)[last_idx_pos];					   				   
					   
			           unsigned int temp_pos = priv->idx_offset + (unsigned long) temp_idx->dwChunkOffset;
			           stream_seek(stream, temp_pos);
				       id = stream_read_dword_le(stream);
				       len = stream_read_dword_le(stream);
					   //mpDebugPrint("last_idx_pos = %d, id = 0x%d, len = 0x%x, offset = 0x%x, pos = 0x%x", last_idx_pos, id, len, temp_idx->dwChunkOffset, temp_pos);
			           demux_avi_read_packet(demux, ds, id, len, last_idx_pos, flags);
				       stream_seek(stream, pos);
				       stream_read_dword_le(stream);
			           stream_read_dword_le(stream);
				       demux->filepos = stream_tell(demux->stream);
			           return 1;
			       	}	   
			   }
			   else // not MJPEG			   	
			   {
			       //mpDebugPrint("_fill_buffer_ni len=0");
			       //continue; //Mark the line, it will cause avsync fail. 
			       			   //If it meets that can not play some video files, try to unmark it 2011/07/21 XW noted.
			   }
			}  
			if ((len != idx->dwChunkLength) && ((len + 1) != idx->dwChunkLength))
			{
				if (len > 0x200000 && idx->dwChunkLength > 0x200000)
					continue;	// both values bad :(
				len = choose_chunk_len(idx->dwChunkLength, len);
			}
			if (!(idx->dwFlags & AVIIF_KEYFRAME))
				flags = 0;
		}
		else
			return 0;
		ret = demux_avi_read_packet(demux, ds, id, len, idx_pos, flags);
	} while (ret != 1);
	return 1;
}

// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
static int _fill_buffer_nini(demux_stream_t * ds)
{
    //mpDebugPrint("_fill_buffer_nini");
	avi_priv_t *priv = demux->priv;
	unsigned int id = 0;
	unsigned int len;
	int ret = 0;
	unsigned int *fpos = NULL;
	stream_t *stream;

	if (ds == demux->video)
		fpos = &priv->idx_pos_v;
	else if (ds == demux->video_3D)
		fpos = &priv->idx_pos_v_3D;
	else if (ds == demux->audio)
		fpos = &priv->idx_pos_a;
	else
		return 0;

	if ((ds == demux->audio) && (demux->stream_a != NULL))
		stream = demux->stream_a;
	else
		stream = demux->stream;

	stream_seek(stream, fpos[0]);
	do
	{

		demux->filepos = stream_tell(stream);
		if (demux->filepos >= demux->movi_end && (demux->movi_end > demux->movi_start))
		{
			//demux->stream->eof=1;
			ds->eof = 1;
			return 0;
		}

		id = stream_read_dword_le(stream);
		len = stream_read_dword_le(stream);

        if (0 == len)				
		{			   
			return 0;
	    }
		
		if (stream_eof(stream))
			return 0;

		if (id == mmioFOURCC('L', 'I', 'S', 'T'))
		{
			id = stream_read_dword_le(stream);	// list type
			continue;
		}

		if (id == mmioFOURCC('R', 'I', 'F', 'F'))
		{
			id = stream_read_dword_le(stream);	// "AVIX"
			continue;
		}

		if (ds == demux_avi_select_stream(demux, id))
		{
			// read it!
			ret = demux_avi_read_packet(demux, ds, id, len, priv->idx_pos - 1, 0);
		}
		else
		{
			// skip it!
			int skip = (len + 1) & (~1);	// total bytes in this chunk

			stream_skip(stream, skip);
		}

	}
	while (ret != 1);
	fpos[0] = stream_tell(stream);
	return 1;
}


// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
int _fill_buffer(demux_stream_t * dst)
{
    //mpDebugPrint("_fill_buffer");
	avi_priv_t *priv;
	unsigned int id = 0;
	unsigned int len;
	int ret = 0;
	demux_stream_t *ds;
	if (current_avi_format == _AVI_NI)
		return _fill_buffer_ni(dst);
	else if (current_avi_format == _AVI_NI_NI)
		return _fill_buffer_nini(dst);

	priv = demux->priv;
	//id=0;
	//ret=0;
//int max_packs=128;

	do
	{
		int flags = 1;
		AVIINDEXENTRY *idx = NULL;

		if (priv->idx_size > 0 && priv->idx_pos < priv->idx_size)
		{
			unsigned int pos;

			idx = &((AVIINDEXENTRY *) priv->idx)[priv->idx_pos++];

			if (idx->dwFlags & AVIIF_LIST)
			{
				// LIST
				continue;
			}

			if (!demux_avi_select_stream(demux, idx->ckid))
			{
				continue;		// skip this chunk
			}

			pos = priv->idx_offset + (unsigned long) idx->dwChunkOffset;
			// if((pos<demux->movi_start || pos>=demux->movi_end) && (demux->movi_end>demux->movi_start) && (demux->stream->type!=STREAMTYPE_STREAM)){
			if ((pos < demux->movi_start || pos >= demux->movi_end)
				&& (demux->movi_end > demux->movi_start))
			{
				continue;
			}

			stream_seek(demux->stream, pos);
			demux->filepos = stream_tell(demux->stream);
			id = stream_read_dword_le(demux->stream);
			if (stream_eof(demux->stream))
				return 0;		// EOF!

			if (id != idx->ckid)
			{
			#if 0
				if (valid_fourcc(idx->ckid))
					id = idx->ckid;	// use index if valid
				else if (!valid_fourcc(id))
			#endif		
					continue;	// drop chunk if both id and idx bad
			}
			len = stream_read_dword_le(demux->stream);
			if (0 == len)				
			{
			   continue;
			}
			
			if ((len != idx->dwChunkLength) && ((len + 1) != idx->dwChunkLength))
			{
				if (len > 0x200000 && idx->dwChunkLength > 0x200000)
					continue;	// both values bad :(
				len = choose_chunk_len(idx->dwChunkLength, len);
			}
			if (!(idx->dwFlags & AVIIF_KEYFRAME))
				flags = 0;
		}
		else
		{
			demux->filepos = stream_tell(demux->stream);
			if (demux->filepos >= demux->movi_end && demux->movi_end > demux->movi_start)
			{
				demux->stream->eof = 1;
				return 0;
			}
			id = stream_read_dword_le(demux->stream);
			len = stream_read_dword_le(demux->stream);
			if (stream_eof(demux->stream))
				return 0;		// EOF!
			if (id == mmioFOURCC('L', 'I', 'S', 'T') || id == mmioFOURCC('R', 'I', 'F', 'F'))
			{
				id = stream_read_dword_le(demux->stream);	// list or RIFF type
				continue;
			}
		}

		ds = demux_avi_select_stream(demux, id);
		if (ds)
			if (ds->packs + 1 >= MAX_PACKS || ds->bytes + len >= MAX_PACK_BYTES)
			{
				// this packet will cause a buffer overflow, switch to -ni mode!!!
				if (priv->idx_size > 0)
				{
					// has index
					// demux->type=DEMUXER_TYPE_AVI_NI;
					current_avi_format = _AVI_NI;
					--priv->idx_pos;	// hack
				}
				else
				{
					// no index
					// demux->type=DEMUXER_TYPE_AVI_NINI;
					current_avi_format = _AVI_NI_NI;
					priv->idx_pos = demux->filepos;	// hack
				}
				priv->idx_pos_v = priv->idx_pos_v_3D =priv->idx_pos_a = priv->idx_pos;
				// quit now, we can't even (no enough buffer memory) read this packet :(
				return -1;
			}

		ret = demux_avi_read_packet(demux, ds, id, len, priv->idx_pos - 1, flags);
	} while (ret != 1);
	return 1;
}



// AVI demuxer parameters:
int index_mode = -1;			// -1=untouched  0=don't use index  1=use (geneate) index
char *index_file_save = NULL, *index_file_load = NULL;
int force_ni = 1;				// force non-interleaved AVI parsing

int read_avi_header(demuxer_t * demuxer, int index_mode);

static demuxer_t *demux_open_avi()
{
	demux_stream_t *d_audio = demux->audio;
	demux_stream_t *d_video = demux->video;
	demux_stream_t *d_video_3D = demux->video_3D;
	sh_audio_t *sh_audio = NULL;
	sh_video_t *sh_video = NULL;
	sh_video_t *sh_video_3D = NULL;
	avi_priv_t *priv = (avi_priv_t *) mem_malloc(sizeof(avi_priv_t));

	memset(priv, 0, sizeof(avi_priv_t));
	//CHK_MALLOC ( priv, "demux_open_avi failed" );

	// priv struct:
	priv->avi_audio_pts = priv->avi_video_pts = 0.0f;
	priv->pts_correction = 0.0f;
	priv->skip_video_frames = 0;
	priv->pts_corr_bytes = 0;
	priv->pts_has_video = priv->pts_corrected = 0;
	priv->video_pack_no = 0;
	priv->audio_block_no = 0;
	priv->audio_block_size = 0;
	priv->isodml = 0;
	priv->suidx_size = 0;
	priv->suidx = NULL;
	demux->priv = (void *) priv;

	//---- AVI header:
	// read_avi_header(demuxer,(demuxer->stream->type!=STREAMTYPE_STREAM)?index_mode:-2);
	if(read_avi_header(demux, index_mode)!=0)return -1;

	//if (demux->audio->id >= 0 && !demux->a_streams[demux->audio->id])
	if (demux->audio->id < 0 && !demux->a_streams[demux->audio->id])
	{
		demux->audio->id = -2;	// disabled
	}
	if (demux->video->id >= 0 && !demux->v_streams[demux->video->id])
	{
		demux->video->id = -1;	// autodetect
	}
	if (demux->video_3D->id >= 0 && !demux->v_streams[demux->video_3D->id])
	{
		demux->video_3D->id = -1;	// autodetect
	}

	stream_reset(demux->stream);
	stream_seek(demux->stream, demux->movi_start);
	priv->idx_pos = 0;
	priv->idx_pos_a = 0;
	priv->idx_pos_v = 0;
	priv->idx_pos_v_3D = 0;
	if (priv->idx_size > 1)
	{
		// decide index format:
#if 1
		if ((unsigned long) ((AVIINDEXENTRY *) priv->idx)[0].dwChunkOffset <
			demux->movi_start
			|| (unsigned long) ((AVIINDEXENTRY *) priv->idx)[1].dwChunkOffset < demux->movi_start)
			priv->idx_offset = demux->movi_start - 4;
		else
			priv->idx_offset = 0;
#else
		if ((unsigned long) ((AVIINDEXENTRY *) priv->idx)[0].dwChunkOffset < demux->movi_start)
			priv->idx_offset = demux->movi_start - 4;
		else
			priv->idx_offset = 0;
#endif
	}
//  demux->endpos=avi_header.movi_end;
    if (priv->idx_size > 0)
	{
		// check that file is non-interleaved:
		int i;
		unsigned int a_pos = 0;
		unsigned int v_pos = 0;
		unsigned int v_pos_3D = 0;

		for (i = 0; i < priv->idx_size; i++)
		{
			AVIINDEXENTRY *idx = &((AVIINDEXENTRY *) priv->idx)[i];
			demux_stream_t *ds = demux_avi_select_stream(demux, idx->ckid);
			unsigned int pos = priv->idx_offset + (unsigned long) idx->dwChunkOffset;

			if (a_pos == 0 && ds == demux->audio)
			{
				a_pos = pos;
				if (v_pos != 0)
					break;
			}
			if (v_pos == 0 && ds == demux->video)
			{
				v_pos = pos;
				if (a_pos != 0)
					break;
			}
			if (v_pos_3D == 0 && ds == demux->video_3D)
			{
				v_pos_3D = pos;
				if (a_pos != 0)
					break;
			}
		}

		if (v_pos == 0)
		{
			return NULL;
		}

		if (a_pos == 0)
		{
			sh_audio = NULL;
		}
		else
		{
			//if (force_ni || abs(a_pos - v_pos) > 0x100000)
			if (sh_audio->format == 0x1)
			{
				if (abs(a_pos - v_pos) > 0x1000)	
				{					// distance > 4kB
					current_avi_format = _AVI_NI;	// HACK!!!!
					// demux->type=DEMUXER_TYPE_AVI_NI; // HACK!!!!
					pts_from_bps = 1;	// force BPS sync!
				}
			}
			else
			{
				if (abs(a_pos - v_pos) > 0x100)	
				{					// distance > 256B
					current_avi_format = _AVI_NI;	// HACK!!!!
					// demux->type=DEMUXER_TYPE_AVI_NI; // HACK!!!!
					pts_from_bps = 1;	// force BPS sync!
				}				
			}
		}

	}
	else
	{
		// no index
		if (force_ni)
		{
			// demux->type=DEMUXER_TYPE_AVI_NINI; // HACK!!!!
			current_avi_format = _AVI_NI_NI;	// HACK!!!!
			priv->idx_pos_a = priv->idx_pos_v = demux->movi_start;
			pts_from_bps = 1;	// force BPS sync!
		}

		demux->seekable = 0;
	}

	if (!ds_fill_buffer(d_video))
	{
		return NULL;
	}

//MP_DEBUG("demux_open_avi(): ds_fill_buffer done\r\n");
//DebugBreak();

	sh_video = d_video->sh;
	sh_video->ds = d_video;

  if(demux->sh_video_3D)  // if this is a 3D video file
  {
	  if (!ds_fill_buffer(d_video_3D))
	  {
		  return NULL;
	  }
	    
	  sh_video_3D = d_video_3D->sh;
	  sh_video_3D->ds = d_video_3D;
	}

#if (PURE_VIDEO == 0)
	if (d_audio->id != -2)
	{
        int ok;
        if ((g_psSystemConfig->sVideoPlayer.dwPlayerStatus != MOVIE_STATUS_PREVIEW) ||
			(current_avi_format == _AVI_NI_NI)
		   )	
        {
		    mpDebugPrint("ds_fill_buffer(d_audio)");
			ok = ds_fill_buffer(d_audio);
        }
		else
		{
		    ok = 1;
		}
		if (!priv->audio_streams || !ok)
		{
			sh_audio = NULL;
		}
		else
		{
			sh_audio = d_audio->sh;
			sh_audio->ds = d_audio;
			sh_audio->format = sh_audio->wf->wFormatTag;
			demux->sh_audio = sh_audio;
		}
	}
#endif

	// calc. FPS:
	sh_video->fps = (float) sh_video->video.dwRate / (float) sh_video->video.dwScale;
	sh_video->frametime = (float) sh_video->video.dwScale / (float) sh_video->video.dwRate;

	// calculating audio/video bitrate:
	if (priv->idx_size > 0)
	{
		// we have index, let's count 'em!
		size_t vsize = 0;
		size_t asize = 0;
		size_t vsamples = 0;
		size_t asamples = 0;
		int i;

		for (i = 0; i < priv->idx_size; i++)
		{
			int id = avi_stream_id(((AVIINDEXENTRY *) priv->idx)[i].ckid);
			int len = ((AVIINDEXENTRY *) priv->idx)[i].dwChunkLength;

			if (sh_video->ds->id == id)
			{
				vsize += len;
				++vsamples;
			}
			else if (sh_audio && sh_audio->ds->id == id)
			{
				asize += len;
				asamples += (len + priv->audio_block_size - 1) / priv->audio_block_size;
			}
		}
		priv->numberofframes = vsamples;
		sh_video->i_bps =
			((float) vsize / (float) vsamples) * (float) sh_video->video.dwRate /
			(float) sh_video->video.dwScale;
		if (sh_audio)
			sh_audio->i_bps =
				((float) asize / (float) asamples) * (float) sh_audio->audio.dwRate /
				(float) sh_audio->audio.dwScale;
	}
	else
	{
		// guessing, results may be inaccurate:
		size_t vsize;
		size_t asize = 0;

		if ((priv->numberofframes = sh_video->video.dwLength) <= 1)
			// bad video header, try to get number of frames from audio
			if (sh_audio && sh_audio->wf->nAvgBytesPerSec)
				priv->numberofframes =
					sh_video->fps * sh_audio->audio.dwLength / sh_audio->audio.dwRate *
					sh_audio->audio.dwScale;
		if (priv->numberofframes <= 1)
		{
			priv->numberofframes = 0;
		}

		if (sh_audio)
		{
			if (sh_audio->wf->nAvgBytesPerSec && sh_audio->audio.dwSampleSize != 1)
			{
				asize =
					(float) sh_audio->wf->nAvgBytesPerSec * sh_audio->audio.dwLength *
					sh_audio->audio.dwScale / sh_audio->audio.dwRate;
				sh_audio->i_bps = sh_audio->wf->nAvgBytesPerSec;
			}
			else
			{
				asize = sh_audio->audio.dwLength;
				sh_audio->i_bps = (float) asize / (sh_video->frametime * priv->numberofframes);
			}
		}
		vsize = demux->movi_end - demux->movi_start - asize - 8 * priv->numberofframes;
		sh_video->i_bps = (float) vsize / (sh_video->frametime * priv->numberofframes);
	}

		    mpDebugPrint("demux_open_avi(end) %p",demux);
	return demux;
}

//extern float initial_pts_delay;
extern void resync_audio_stream(sh_audio_t * sh_audio);
static int _seek(float rel_seek_secs, int flags)
{
    //if (current_avi_format == _AVI_NI_NI)
    //if (rel_seek_secs != 0)
	//	__asm("break 100");
	
	avi_priv_t *priv = demux->priv;
	demux_stream_t *d_audio = demux->audio;
	demux_stream_t *d_video = demux->video;
	demux_stream_t *d_video_3D = demux->video_3D;
	sh_audio_t *sh_audio = d_audio->sh;
	sh_video_t *sh_video = d_video->sh;
	sh_video_t *sh_video_3D = d_video_3D->sh;
	float skip_audio_secs = 0;

	
	//================= seek in AVI ==========================
    unsigned int newpos_v,newpos_a;
	int rel_seek_frames = rel_seek_secs * sh_video->fps;
	unsigned int video_chunk_pos = d_video->pos;
	int i;
	int frame_rate,four_cc;

    if (_AVI_NI_NI == current_avi_format)
	{
	    newpos_v = (flags & 1) ? demux->movi_start : demux->filepos;
		
	    if (!sh_video->i_bps)	
			newpos_v += 2324 * 75 * rel_seek_secs;	// 174.3 kbyte/sec
	    else					
			newpos_v += (unsigned int) (rel_seek_secs * sh_video->i_bps);

		if (newpos_v < demux->movi_start)
		    newpos_v = demux->movi_start;
		else if (newpos_v > demux->movi_end)	
			newpos_v = demux->movi_end;
    }	
	else
	{
	    if (flags & 1)
	    {
		    // seek absolute
		    video_chunk_pos = 0;
	    }

	    if (flags & 2)
	    {
		    rel_seek_frames = rel_seek_secs * priv->numberofframes;
	    }	
	}
	
	priv->skip_video_frames = 0;
	priv->avi_audio_pts = 0;
	four_cc = sh_video->format;
// ------------ STEP 1: find nearest video keyframe chunk ------------
	// find nearest video keyframe chunk pos:
	if (rel_seek_frames > 0)
	{
		// seek forward
		if (_AVI_NI_NI == current_avi_format)
		{
		    #if 1
			stream_t *stream;
			stream = demux->stream;
			#ifdef _LARGEFILE_SOURCE
	        newpos_v &= ~((long long) STREAM_BUFFER_SIZE - 1);	// sector boundary
            #else
	        newpos_v &= ~(STREAM_BUFFER_SIZE - 1);	// sector boundary
            #endif
	        //MP_DEBUG1("seek video 0x%x",newpos_v);
	        stream->eof = 0;
	        stream_seek(stream, newpos_v);
	        demux->filepos = stream_tell(stream);
			ds_fill_buffer(d_video);
			
			#else
		    unsigned int  *fpos = NULL;
	        stream_t *stream;

		    fpos = &priv->idx_pos_v;
  		    stream = demux->stream;		       
   	        stream_seek(stream, fpos[0]);
            
			do
	        {
		       demux->filepos = stream_tell(stream);
		       if (demux->filepos >= demux->movi_end && (demux->movi_end > demux->movi_start))
		       {
			      demux->video->eof = 1;
			      return 0;
		       }

		       int id = stream_read_dword_le(stream);
		       unsigned int len = stream_read_dword_le(stream);

               if (0 == len)							   
			      continue;	   

			   if (avi_stream_id(id) == d_video->id)
			   { 
			       // video frame
			       d_video->pack_no++;
				   if (four_cc == MJPG || four_cc == mjpg || four_cc == JPEG || four_cc == jpeg || four_cc == dmb1)
				   {
					   if ((--rel_seek_frames) < 0)
						   break;
				   }
				   else
				   {
					   if ((--rel_seek_frames) < 0 && 
						   ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].dwFlags & AVIIF_KEYFRAME)
						   break;
				   }	
			   }

		       // skip it!
			   int skip = (len + 1) & (~1);	// total bytes in this chunk
			   stream_skip(stream, skip);
			
		       if (stream_eof(stream))
			   	   return 0;
			}  
			while (demux->video->eof != 1);  
			#endif
		}
		else
		{
		    while (video_chunk_pos < priv->idx_size - 1)
		    {
			    int id = ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].ckid;

			    if (avi_stream_id(id) == d_video->id)
			    {					// video frame
				    if (four_cc == MJPG || four_cc == mjpg || four_cc == JPEG || four_cc == jpeg || four_cc == dmb1)
				    {
					    if ((--rel_seek_frames) < 0)
						    break;
				    }
				    else
				    {
					    if ((--rel_seek_frames) < 0 && 
							((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].dwFlags & AVIIF_KEYFRAME)
						break;
				    }	
			    }
			    ++video_chunk_pos;
		    }
		}	
	}
	else
	{
		// seek backward
		if (_AVI_NI_NI == current_avi_format)
		{
		    stream_t *stream;
			stream = demux->stream;
			#ifdef _LARGEFILE_SOURCE
	        newpos_v &= ~((long long) STREAM_BUFFER_SIZE - 1);	// sector boundary
            #else
	        newpos_v &= ~(STREAM_BUFFER_SIZE - 1);	// sector boundary
            #endif
	        //MP_DEBUG1("seek video 0x%x",newpos_v);
	        stream->eof = 0;
	        stream_seek(stream, newpos_v);
	        demux->filepos = stream_tell(stream);
			ds_fill_buffer(d_video);
		}
		else
		{
		    while (video_chunk_pos > 0)
		    {
			    int id = ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].ckid;

			    if (avi_stream_id(id) == d_video->id)
			    {					
			       // video frame				
				   if (four_cc == MJPG || four_cc == mjpg || four_cc == JPEG || four_cc == jpeg || four_cc == dmb1)
				   {
				       if ((++rel_seek_frames) > 0)
						  break;
				   }
				   else
				   {
					   if ((++rel_seek_frames) > 0 && 
					   	    ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].dwFlags & AVIIF_KEYFRAME)
						   break;
				   }	
			    }
			    --video_chunk_pos;
		    }	
		}		
	}

    if (_AVI_NI_NI == current_avi_format)
    {
        // re-calc video pts:       
		d_video->pack_no +=  (int)(rel_seek_secs*(float)sh_video->fps);
		if (d_video->pack_no < 0)
			d_video->pack_no = 0;
		priv->video_pack_no = sh_video->num_frames = sh_video->num_frames_decoded = d_video->pack_no;
	    priv->avi_video_pts =
		d_video->pack_no * (float) sh_video->video.dwScale / (float) sh_video->video.dwRate+rel_seek_secs;
		if (priv->avi_video_pts <0)
			priv->avi_video_pts = 0;
		
    }
	else
	{
	    priv->idx_pos_a = priv->idx_pos_v = priv->idx_pos_v_3D = priv->idx_pos = video_chunk_pos;
	
	    // re-calc video pts:
	    d_video->pack_no = 0;
	    for (i = 0; i < video_chunk_pos; i++)
	    {
		    int id = ((AVIINDEXENTRY *) priv->idx)[i].ckid;

		    if (avi_stream_id(id) == d_video->id)
			    ++d_video->pack_no;
	    }
	    priv->video_pack_no = sh_video->num_frames = sh_video->num_frames_decoded = d_video->pack_no;
	    priv->avi_video_pts =
		d_video->pack_no * (float) sh_video->video.dwScale / (float) sh_video->video.dwRate;
	    d_video->pos = video_chunk_pos;
	}	

// ------------ STEP 2: seek audio, find the right chunk & pos ------------

	d_audio->pack_no = 0;
//      d_audio->block_no=0;
	priv->audio_block_no = 0;
	d_audio->dpos = 0;

	if (sh_audio)
	{
		int i;
		int len = 0;
		int skip_audio_bytes = 0;
		int curr_audio_pos = -1;
		int audio_chunk_pos = -1;
		int chunk_max = (current_avi_format == _AVI) ? video_chunk_pos : priv->idx_size;

		if (sh_audio->audio.dwSampleSize)
		{
			// constant rate audio stream
#if 0
			int align;

			curr_audio_pos = (priv->avi_video_pts) * sh_audio->wf->nAvgBytesPerSec;
			if (curr_audio_pos < 0)
				curr_audio_pos = 0;
			align = sh_audio->audio.dwSampleSize;
			if (sh_audio->wf->nBlockAlign > align)
				align = sh_audio->wf->nBlockAlign;
			curr_audio_pos /= align;
			curr_audio_pos *= align;
#else
			curr_audio_pos =
				(int)((priv->avi_video_pts) * (float) sh_audio->audio.dwRate /
				(float) sh_audio->audio.dwScale);
				if(curr_audio_pos >=sh_audio->audio.dwStart)
			     curr_audio_pos -= sh_audio->audio.dwStart;
			curr_audio_pos *= sh_audio->audio.dwSampleSize;
#endif
			if (_AVI_NI_NI == current_avi_format && g_psSystemConfig->sVideoPlayer.dwPlayerStatus != MOVIE_STATUS_PREVIEW)
			{
				stream_t *stream_a;
				stream_a = demux->stream_a;
				newpos_a = (int)((priv->avi_video_pts) * (float)sh_audio->i_bps)>>3;			
	        	stream_a->eof = 0; 
				demux->filepos = demux->movi_start; 
				priv->idx_pos_a = demux->movi_start;
				stream_seek(stream_a,newpos_a);
				demux->filepos = stream_tell(stream_a);
				ds_fill_buffer(d_audio);
				demux->filepos = stream_tell(demux->stream);
			}
			else
			{
			// find audio chunk pos:
			for (i = 0; i < chunk_max; i++)
			{
				int id = ((AVIINDEXENTRY *) priv->idx)[i].ckid;

				if (avi_stream_id(id) == d_audio->id)
				{
					len = ((AVIINDEXENTRY *) priv->idx)[i].dwChunkLength;
					if (d_audio->dpos <= curr_audio_pos && curr_audio_pos < (d_audio->dpos + len))
					{
						break;
					}
					++d_audio->pack_no;
					priv->audio_block_no += priv->audio_block_size ?
						((len + priv->audio_block_size - 1) / priv->audio_block_size) : 1;
					d_audio->dpos += len;
				}
			}
			}
			audio_chunk_pos = i;
			skip_audio_bytes = curr_audio_pos - d_audio->dpos;
		}
		else
		{
			// VBR audio
			int chunks =
				(priv->avi_video_pts) * (float) sh_audio->audio.dwRate /
				(float) sh_audio->audio.dwScale;
			audio_chunk_pos = 0;

			// find audio chunk pos:
			for (i = 0; i < priv->idx_size && chunks > 0; i++)
			{
				int id = ((AVIINDEXENTRY *) priv->idx)[i].ckid;

				if (avi_stream_id(id) == d_audio->id)
				{
					len = ((AVIINDEXENTRY *) priv->idx)[i].dwChunkLength;
					if (i > chunk_max)
					{
						skip_audio_bytes += len;
					}
					else
					{
						++d_audio->pack_no;
						priv->audio_block_no += priv->audio_block_size ?
							((len + priv->audio_block_size - 1) / priv->audio_block_size) : 1;
						d_audio->dpos += len;
						audio_chunk_pos = i;
					}
//      --chunks;
					if (priv->audio_block_size)
						chunks -= (len + priv->audio_block_size - 1) / priv->audio_block_size;
				}
			}
			//if(audio_chunk_pos>chunk_max) audio_chunk_pos=chunk_max;
		}

		// Now we have:
		//      audio_chunk_pos = chunk no in index table (it's <=chunk_max)
		//      skip_audio_bytes = bytes to be skipped after chunk seek
		//      d-audio->pack_no = chunk_no in stream at audio_chunk_pos
		//      d_audio->dpos = bytepos in stream at audio_chunk_pos
		// let's seek!

		// update stream position:
		d_audio->pos = audio_chunk_pos;
//          d_audio->dpos=apos;
//    d_audio->pts=initial_pts_delay+(float)apos/(float)sh_audio->wf->nAvgBytesPerSec;
		if (current_avi_format == _AVI)
		{
			// interleaved stream:
			if (audio_chunk_pos < video_chunk_pos)
			{
				// calc priv->skip_video_frames & adjust video pts counter:
				for (i = audio_chunk_pos; i < video_chunk_pos; i++)
				{
					int id = ((AVIINDEXENTRY *) priv->idx)[i].ckid;

					if (avi_stream_id(id) == d_video->id)
						++priv->skip_video_frames;
				}
				// requires for correct audio pts calculation (demux):
				priv->avi_video_pts -=
					priv->skip_video_frames * (float) sh_video->video.dwScale /
					(float) sh_video->video.dwRate;
				priv->avi_audio_pts = priv->avi_video_pts;
				// set index position:
				priv->idx_pos_a = priv->idx_pos_v = priv->idx_pos = audio_chunk_pos;
			}
		}
		else
		{
			if (_AVI_NI_NI != current_avi_format)
			{
			
			// non-interleaved stream:
			priv->idx_pos_a = audio_chunk_pos;
			priv->idx_pos_v = video_chunk_pos;
			priv->idx_pos_v_3D = video_chunk_pos;
			priv->idx_pos = (audio_chunk_pos < video_chunk_pos) ? audio_chunk_pos : video_chunk_pos;
			}
		}

    if(skip_audio_bytes)
    {
            demux_read_data(d_audio,NULL,skip_audio_bytes);
    }
#if 0		
		if (skip_audio_bytes)
		{
			/* BUG: A/V sync workaround */
			priv->idx_pos_a--;
			len = ((AVIINDEXENTRY *) priv->idx)[priv->idx_pos_a].dwChunkLength;
			d_audio->dpos -= len;

			ds_read_data(d_audio, NULL, skip_audio_bytes);
			//d_audio->pts=0; // PTS is outdated because of the raw data skipping
		}
#endif
#if 0
		char strbuf[80];

		snprintf(strbuf, 80, "pack_no:%d, dwScale:%d, dwRate:%d", d_video->pack_no,
				 sh_video->video.dwScale, sh_video->video.dwRate);
		UartOutText(strbuf);
		snprintf(strbuf, 80, "(apos:%d,%dbytes,%dms)\r\n", curr_audio_pos, skip_audio_bytes,
				 skip_audio_bytes * sh_audio->audio.dwScale * 1000 / (sh_audio->audio.dwRate *
																	  sh_audio->audio.
																	  dwSampleSize));
		UartOutText(strbuf);
#endif

		resync_audio_stream(sh_audio);

//          sh_audio->timer=-skip_audio_secs;
	}
	d_video->pts = priv->avi_video_pts;	// OSD
	return 1;
}


static int _close()
{
	avi_priv_t *priv = demux->priv;

	if (!priv)
		return 1;

	if (priv->idx_size > 0)
		mem_free(priv->idx);
	mem_free(priv);
	return 1;
}


static int _control(int cmd, void *arg)
{
	avi_priv_t *priv = demux->priv;

/*    demux_stream_t *d_audio=demux->audio;*/
	demux_stream_t *d_video = demux->video;

/*    sh_audio_t *sh_audio=d_audio->sh;*/
	sh_video_t *sh_video = d_video->sh;

	switch (cmd)
	{
	case DEMUXER_CTRL_GET_TIME_LENGTH:
		if (!priv->numberofframes)
			return DEMUXER_CTRL_DONTKNOW;
		*((unsigned long *) arg) = priv->numberofframes / sh_video->fps;
		if (sh_video->video.dwLength <= 1)
			return DEMUXER_CTRL_GUESS;
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_GET_PERCENT_POS:
		if (!priv->numberofframes)
		{
			if (demux->movi_end == demux->movi_start)
				return DEMUXER_CTRL_DONTKNOW;
			*((int *) arg) =
				(int) ((demux->filepos -
						demux->movi_start) / ((demux->movi_end - demux->movi_start) / 100));
			return DEMUXER_CTRL_OK;
		}
		*((int *) arg) = (int) (priv->video_pack_no * 100 / priv->numberofframes);
		if (sh_video->video.dwLength <= 1)
			return DEMUXER_CTRL_GUESS;
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_IF_PTS_FROM_STREAM:
		*((int*)arg) = 0;
		return DEMUXER_CTRL_OK;

	default:
		return DEMUXER_CTRL_NOTIMPL;
	}
}


static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
	int id;

	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_AVI))
	{

		//MP_DEBUG ( " avi check_type\r\n" );
//DEBUG_BREAK();

		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		//---- RIFF header:
		id = stream_read_dword_le(demux->stream);	// "RIFF"
		if (id == mmioFOURCC('R', 'I', 'F', 'F'))
		{
			stream_read_dword_le(demux->stream);	//filesize
			id = stream_read_dword_le(demux->stream);	// "AVI "
			if (id == formtypeAVI)
			{
				//MP_DEBUG ( "formtypeAVI\r\n" );
				return 1;
			}
//            else if ( id == formtypeCDXA )
//            {
//                //MP_DEBUG ( "formtypeCDXA\r\n" );
//                return 1;
//            }
		}
		//MP_DEBUG ( "not avi: unknown   \r\n" );
		return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
	}

	//MP_DEBUG ( "not avi:  \r\n" );
	return 0;
}

static int _open()
{
	current_avi_format = _AVI;
	if (NULL == demux_open_avi())
		return 0; 
	return 1;
}

//function GetPTSByRelFrame
//flags=0 seek to begin
//flags=1 seek to end
//flags=2 seek to "+/-" rel_seek_samples
static float _get_pts_by_relframe(int rel_seek_samples, int flags)
{
	////////////////////
	float pts;
	demux_stream_t *d_video;
	sh_video_t *sh_video;
	avi_priv_t *priv;


	priv = filter_graph.demux->priv;
	d_video = filter_graph.demux->video;
	sh_video = d_video->sh;

	////map seek samples to pts   


	switch (flags)
	{
	case 2:
		{

			// find nearest video keyframe chunk pos:
#if 0
			if (rel_seek_samples < 0)
			{
				// seek backward
				int video_chunk_pos = d_video->pos;

				rel_seek_samples++;

				while (video_chunk_pos > 0)
				{
					int id = ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].ckid;

					if (avi_stream_id(id) == d_video->id)
					{			// video frame
						if ((++rel_seek_samples) > 0
							&& ((AVIINDEXENTRY *) priv->idx)[video_chunk_pos].
							dwFlags & AVIIF_KEYFRAME)
							break;
					}
					--video_chunk_pos;
				}

				pts = (float) video_chunk_pos / sh_video->fps;
			}
			else
				pts = (sh_video->num_frames + rel_seek_samples) / sh_video->fps;
#else
			pts = (sh_video->num_frames + rel_seek_samples) / sh_video->fps;
#endif

			//MP_DEBUG1 ( "((((((((((((((((((_get_pts_by_relframe:sh_video->num_frames=",
			//      sh_video->num_frames, 8 );
			////MP_DEBUG1("_get_pts_by_relframe:sh_video->fps=",sh_video->fps,8);
			////MP_DEBUG1("_get_pts_by_relframe:pts*1000=",pts*1000,8); 
			break;
		}
	case 0:
		{
			pts = 0;
			////MP_DEBUG1("_get_pts_by_relframe:sh_video->num_frames=", sh_video->num_frames, 8);
			////MP_DEBUG1("_get_pts_by_relframe:sh_video->fps=",sh_video->fps,16);
			////MP_DEBUG1("_get_pts_by_relframe:pts=",pts,8);
			break;
		}
	case 1:
		{
			// pts=(priv->numberofframes)/sh_video->fps;

			// find lastest video keyframe chunk pos:

			int video_chunk_pos = 0;
			int i = 0;

			while (1)
			{
				int id = ((AVIINDEXENTRY *) priv->idx)[i].ckid;

				if (avi_stream_id(id) == d_video->id)
				{				// video frame
					if (((AVIINDEXENTRY *) priv->idx)[i].dwFlags & AVIIF_KEYFRAME)
						video_chunk_pos = i;
					if ((i >= priv->numberofframes))
						break;
				}
				i++;
			}

			pts = (float) video_chunk_pos / sh_video->fps;

			// //MP_DEBUG1("_get_pts_by_relframe:sh_video->num_frames=", sh_video->num_frames, 8);
			// //MP_DEBUG1("_get_pts_by_relframe:sh_video->fps=",sh_video->fps,16);
			////MP_DEBUG1("_get_pts_by_relframe:pts=",pts,8);
			break;
		}
	default:
		break;
	}


	return pts;
}

//command=0: get total time/file length
//command=1: get current time/file point
//command=2: get percentage
//command=3: get avi information
//return=0:  not fill the buffer
//return=1:  fill the buffer
static int _get_movie_info(BYTE * buffer, int command)
{
	register DWORD second, minute, hour;
	BYTE *string;
	BYTE digit;

	demux_stream_t *d_video;
	sh_video_t *sh_video;
	avi_priv_t *priv;

	priv = filter_graph.demux->priv;
	d_video = filter_graph.demux->video;
	sh_video = d_video->sh;


	switch (command)
	{
	case 0:
		{
			if (sh_video->fps)
			{
				second = ((float) priv->numberofframes) / sh_video->fps;
			}
			else
			{
				second = 0;
			}
			minute = second / 60;
			second = second - minute * 60;
			hour = minute / 60;
			minute = minute - hour * 60;

			// convert time value to string
			string = buffer;
			hour = Bin2Bcd(hour);
			string = HexString(string, hour, 2);
			*string = ':';
			string++;
			minute = Bin2Bcd(minute);
			string = HexString(string, minute, 2);
			*string = ':';
			string++;
			second = Bin2Bcd(second);
			string = HexString(string, second, 2);
			*string = 0;

			return 1;
			//break;
		}

	case 1:
		{
			// //MP_DEBUG1("_get_movie_info:priv->numberofframes=", priv->numberofframes, 8);
			// //MP_DEBUG1("_get_movie_info:sh_video->num_frames=", sh_video->num_frames, 8); 
			//  //MP_DEBUG1("_get_movie_info:sh_video->fps=", sh_video->fps, 8); 
			if (sh_video->fps)
			{
				second = ((float) sh_video->num_frames) / sh_video->fps;
			}
			else
			{
				second = 0;
			}
			digit = sh_video->num_frames - second * sh_video->fps + 0.5;

			//if(!digit)
			//{
			minute = second / 60;
			second = second - minute * 60;
			hour = minute / 60;
			minute = minute - hour * 60;

			// convert time value to string
			string = buffer;
			hour = Bin2Bcd(hour);
			string = HexString(string, hour, 2);
			*string = ':';
			string++;
			minute = Bin2Bcd(minute);
			string = HexString(string, minute, 2);
			*string = ':';
			string++;
			second = Bin2Bcd(second);
			string = HexString(string, second, 2);
			*string = 0;

			return 1;
			//}
			//return 0;
			//break;
		}
	case 2:
		{
			////MP_DEBUG1("_get_movie_info:priv->numberofframes=", priv->numberofframes, 8);
			// //MP_DEBUG1("_get_movie_info:sh_video->num_frames=", sh_video->num_frames, 8); 
			// //MP_DEBUG1("_get_movie_info:sh_video->fps=", sh_video->fps, 8); 
			if (sh_video->fps)
			{
				second = ((float) sh_video->num_frames) / sh_video->fps;
			}
			else
			{
				second = 0;
			}

			digit = sh_video->num_frames - second * sh_video->fps + 0.5;


			//digit ++;
			string = buffer;
			digit = Bin2Bcd(digit);
			string = HexString(string, digit, 2);
			*string = '/';
			string++;
			digit = Bin2Bcd((DWORD) (sh_video->fps + 0.5));
			string = HexString(string, digit, 2);
			*string = 0;

			return 1;
			//break;
		}
	case 3:
		{
			PMedia_Info pAVIInfo = (PMedia_Info) buffer;
			sh_video_t *sh_video = (sh_video_t *) demux->sh_video;
			sh_audio_t *sh_audio = (sh_audio_t *) demux->sh_audio;
			avi_priv_t *priv = (avi_priv_t *) demux->priv;

			if (sh_audio)
			{                
		        GetAudioCodecCategory(pAVIInfo->cAudioCodec);
				pAVIInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_AUDIO;

				if ((pAVIInfo->dwSampleRate = sh_audio->samplerate) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_SampleRate_USEFUL;

				if ((pAVIInfo->dwSampleSize = sh_audio->samplesize) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_SampleSize_USEFUL;

				if ((pAVIInfo->dwBitrate = sh_audio->i_bps) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_Bitrate_USEFUL;

				if (sh_audio->i_bps > 0)
				{
					int length = demux->stream->file_len;
					int bps = sh_audio->i_bps;
					int total_time = length / bps + 0.5;

					pAVIInfo->dwTotalTime = total_time;
					if (pAVIInfo->dwTotalTime >= 0)
					{
						pAVIInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
					}
				}
				else
					pAVIInfo->dwTotalTime = 0;
			}

			if (sh_video)
			{
                GetVideoCodecCategory(pAVIInfo->cVidoeCodec);
				pAVIInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_VIDEO;

				if ((pAVIInfo->dwFrameRate = sh_video->fps) > 0)
				{
					pAVIInfo->dwFlags |= (DWORD) MOVIE_FrameRate_USEFUL;
				}
				if ((pAVIInfo->dwImageHeight = sh_video->disp_h) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_ImageHeight_USEFUL;

				if ((pAVIInfo->dwImageWidth = sh_video->disp_w) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_ImageWidth_USEFUL;

				if ((pAVIInfo->dwTotalFrame = priv->numberofframes) > 0)
					pAVIInfo->dwFlags |= (DWORD) MOVIE_TotalFrame_USEFUL;

				if (sh_video->fps)
				{
					pAVIInfo->dwTotalTime = ((float) priv->numberofframes) / sh_video->fps + 0.5;
					if (pAVIInfo->dwTotalTime >= 0)
						pAVIInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
				}
				else
					pAVIInfo->dwTotalTime = 0;



				if (strlen(avicontent.szAuthor) > 0)
				{
					memcpy(pAVIInfo->contentinfo.szAuthor, avicontent.szAuthor, MAX_AUTHOR_LEN);
					pAVIInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
				}
				if (strlen(avicontent.szCopyright) > 0)
				{
					memcpy(pAVIInfo->contentinfo.szCopyright, avicontent.szCopyright,
						   MAX_COPYRIGHT_LEN);
					pAVIInfo->dwFlags |= (DWORD) MOVIE_Copyright_USEFUL;
				}
				if (strlen(avicontent.szDescription) > 0)
				{
					memcpy(pAVIInfo->contentinfo.szDescription, avicontent.szDescription,
						   MAX_DESCRIP_LEN);
					pAVIInfo->dwFlags |= (DWORD) MOVIE_Description_USEFUL;
				}
				if (strlen(avicontent.szRating) > 0)
				{
					memcpy(pAVIInfo->contentinfo.szRating, avicontent.szRating, MAX_RATING_LEN);
					pAVIInfo->dwFlags |= (DWORD) MOVIE_Rating_USEFUL;
				}
				if (strlen(avicontent.szTitle) > 0)
				{
					memcpy(pAVIInfo->contentinfo.szTitle, avicontent.szTitle, MAX_TITLE_LEN);
					pAVIInfo->dwFlags |= (DWORD) MOVIE_Title_USEFUL;
				}

			}

			return 1;
		}
	default:
		break;
	}
	return 0;
}

demuxer_t *new_avi_demux()
{
	demux = new_base_demux();
	// Assign all of the function pointers and type specific data
	demux->check_type = _check_type;
	demux->open = _open;
	demux->fill_buffer = _fill_buffer;
	demux->seek = _seek;
	demux->control = _control;
	demux->close = _close;
	demux->get_pts_by_relframe = _get_pts_by_relframe;
	demux->get_movie_info = _get_movie_info;
	demux->type = DEMUXER_TYPE_AVI;
	demux->file_format = DEMUXER_TYPE_AVI;
	return demux;
}
#endif
