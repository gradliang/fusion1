/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_mov.c
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:  MOV format file demux implementation
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/
#include "global612.h"
#if VIDEO_ON

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/

#include "mpTrace.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux_types.h"
#include "demux_packet.h"
#include "demux.h"
#include "stheader.h"

#include "bswap.h"
#include "filter_graph.h"

#include "movfmt.h"
#include "parse_mp4.h"
#include "components.h"
#include "qtpalette.h"


#ifdef HAVE_ZLIB
//#include <zlib.h>abel 2005.05.05
#endif
//add "define HAVE_ZLIB" and include <zlib.h>
//if we want to support zlib mov

//#ifndef _FCNTL_H
//#include <fcntl.h>
//#endif

///
///@ingroup group1_Demux
///@defgroup     MOV MOV File Format
///

extern demuxer_t *new_base_demux();

// MOV/MP4 demuxer
static demuxer_t *demux = 0;

//added by jackyang
static MOV_Content movcontent;

extern DWORD g_dwTotalSeconds;
extern DWORD forward_flag;
extern DWORD backward_flag;
extern float StepSeconds;
int network_mdat=0;
mov_exception exception_type;

int sample_num_entries=0;


//add end

static void mov_build_index(mov_track_t * trak, int timescale)
{
	int i, j, s;
	int last = trak->chunks_size;
	unsigned long long pts = 0;

	// process chunkmap:
	//chunks[j].size: to inform each chunk how many samples it contain ,same as in mjpeg inital  	//Joshua
	i = trak->chunkmap_size;
	while (i > 0)
	{
		--i;
		for (j = trak->chunkmap[i].first; j < last; j++)
		{
			trak->chunks[j].desc = trak->chunkmap[i].sdid;
			trak->chunks[j].size = trak->chunkmap[i].spc;
		}
		last = trak->chunkmap[i].first;
	}

	// calc pts of chunks:
	//chunks[j].sample:  to inform each chunk the first sample it starts  //Joshua
	s = 0;
	for (j = 0; j < trak->chunks_size; j++)
	{
		trak->chunks[j].sample = s;
		s += trak->chunks[j].size;
	}

	// for fixed-sample size (ex: dv and uncompressed)
	if (!trak->samples_size && trak->type != MOV_TRAK_AUDIO)
	{
		trak->samples_size = s;
		trak->samples = (mov_sample_t *) mem_malloc(sizeof(mov_sample_t) * s);
		//CHK_MALLOC(trak->samples, "mov_build_index failed");
		for (i = 0; i < s; i++)
			trak->samples[i].size = trak->samplesize;
		trak->samplesize = 0;
	}

	// constant sampesize
	if (!trak->samples_size)
	{
		//check if constant duration
		if (trak->durmap_size == 1 || (trak->durmap_size == 2 && trak->durmap[1].num == 1))
		{
			trak->duration = trak->durmap[0].dur;
		}
		return;
	}

	// calc pts:
	s = 0;
	
		for (j = 0; j < trak->durmap_size; j++)
		{
		
			for (i = 0; i < trak->durmap[j].num; i++)
			{
			trak->samples[s].pts = pts;
			++s;
			pts += trak->durmap[j].dur;
			}
		
		}
		
	// calc sample offsets
	s = 0;
	for (j = 0; j < trak->chunks_size; j++)
	{
		unsigned int pos = trak->chunks[j].pos;

		for (i = 0; i < trak->chunks[j].size; i++)
		{
			trak->samples[s].pos = pos;
			pos += trak->samples[s].size;
			++s;
			if(s>=sample_num_entries)break;
		}
	}

	// precalc editlist entries
	if (trak->editlist_size > 0)
	{
		int frame = 0;
		int e_pts = 0;

		for (i = 0; i < trak->editlist_size; i++)
		{
			mov_editlist_t *el = &trak->editlist[i];
			int sample = 0;
			int disp = el->pos;

			el->start_frame = frame;
			if (disp < 0)
			{
				// skip!
				el->frames = 0;
				continue;
			}
			// find start sample
			for (; sample < trak->samples_size; sample++)
			{
				if (disp <= trak->samples[sample].pts)
					break;
			}
			sample = 0; // Temporarily set sample=0 for solving H.264 Sony/Sanyo with skip first I-frame issue
			el->start_sample = sample;
			el->pts_offset =
			    ((long long) trak->timescale*(long long) e_pts ) / (long long) timescale -
			    trak->samples[sample].pts;
			e_pts += el->dur;

			disp += ((long long) trak->timescale* (long long) el->dur) / (long long) timescale;

			// find end sample
			for (; sample < trak->samples_size; sample++)
			{
				if (disp <= trak->samples[sample].pts)
					break;
			}
			el->frames = sample - el->start_sample;
			frame += el->frames;
		}
	}

}


//extern int mp4_parse_esds(unsigned char *data, int datalen, esds_t *esds);
//extern void mp4_free_esds(esds_t *esds);
#ifdef AMR_NB_ENABLE

static void lschunks(int level, unsigned int endpos, mov_track_t * trak)
{
	mov_priv_t *priv = demux->priv;

//    MP_DPF("lschunks (level=%d,endpos=%x)\n", level, endpos);
	while (1)
	{
		unsigned int pos;
		unsigned int len;
		unsigned int id;

		//
		pos = stream_tell(demux->stream);
//  MP_DPF("stream_tell==%d\n",pos);
		if (pos >= endpos)
			return;				// END
		len = stream_read_dword(demux->stream);
//  MP_DPF("len==%d\n",len);
		if (len < 8)
			return;				// error
		len -= 8;
		id = stream_read_dword(demux->stream);
		//
		//mp_msg(MSGT_DEMUX,MSGL_DBG2,"lschunks %.4s  %d\n",&id,(int)len);
		//
		if (trak)
		{
			switch (id)
			{
			case MOV_FOURCC('m', 'd', 'a', 't'):
			{
				//mp_msg(MSGT_DEMUX,MSGL_WARN,"Hmm, strange MOV, parsing mdat in lschunks?\n");
				return;
			}
			case MOV_FOURCC('f', 'r', 'e', 'e'):
			case MOV_FOURCC('u', 'd', 't', 'a'):
				/* here not supported :p */
				break;
			case MOV_FOURCC('t', 'k', 'h', 'd'):
			{
				//mp_msg(MSGT_DEMUX,MSGL_V,"MOV: %*sTrack header!\n",level,"");
				// read codec data
				trak->tkdata_len = len;
				trak->tkdata = (unsigned char *) mem_malloc(trak->tkdata_len);
				stream_read(demux->stream, trak->tkdata, trak->tkdata_len);
				/*
				0  1 Version
				1  3 Flags
				4  4 Creation time
				8  4 Modification time
				12 4 Track ID
				16 4 Reserved
				20 4 Duration
				24 8 Reserved
				32 2 Layer
				34 2 Alternate group
				36 2 Volume
				38 2 Reserved
				40 36 Matrix structure
				76 4 Track width
				80 4 Track height
				*/
				mp_msg(MSGT_DEMUX, MSGL_V, "tkhd len=%d ver=%d flags=0x%X id=%d dur=%d lay=%d vol=%d\n", trak->tkdata_len, trak->tkdata[0], trak->tkdata[1], char2int(trak->tkdata, 12),	// id
				       char2int(trak->tkdata, 20),	// duration
				       char2short(trak->tkdata, 32),	// layer
				       char2short(trak->tkdata, 36));	// volume
				break;
			}
			case MOV_FOURCC('m', 'd', 'h', 'd'):
			{
				unsigned int tmp;

				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sMedia header!\n", level, "");
#if 0
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword1: 0x%08X (%d)\n", tmp, tmp);
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword2: 0x%08X (%d)\n", tmp, tmp);
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword3: 0x%08X (%d)\n", tmp, tmp);
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword4: 0x%08X (%d)\n", tmp, tmp);
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword5: 0x%08X (%d)\n", tmp, tmp);
				tmp = stream_read_dword(demuxer->stream);
				MP_DPF("dword6: 0x%08X (%d)\n", tmp, tmp);
#endif
				stream_skip(demux->stream, 12);
				// read timescale
				trak->timescale = stream_read_dword(demux->stream);
				if(trak->timescale==0)
				{
				exception_type.type=1;
				trak->timescale=10000000;
				}
				// read length
				trak->length = stream_read_dword(demux->stream);
				break;
			}
			case MOV_FOURCC('h', 'd', 'l', 'r'):
			{
				unsigned int tmp = stream_read_dword(demux->stream);
				unsigned int type = stream_read_dword_le(demux->stream);
				unsigned int subtype = stream_read_dword_le(demux->stream);
				unsigned int manufact = stream_read_dword_le(demux->stream);
				unsigned int comp_flags = stream_read_dword(demux->stream);
				unsigned int comp_mask = stream_read_dword(demux->stream);
				int len = stream_read_char(demux->stream);
				char *str = (char *) mem_malloc(len + 1);

				stream_read(demux->stream, str, len);
				str[len] = 0;
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sHandler header: %.4s/%.4s (%.4s) %s\n",
				       level, "", &type, &subtype, &manufact, str);
				mem_free(str);
				switch (bswap_32(type))
				{
				case MOV_FOURCC('m', 'h', 'l', 'r'):
					trak->media_handler = bswap_32(subtype);
					break;
				case MOV_FOURCC('d', 'h', 'l', 'r'):
					trak->data_handler = bswap_32(subtype);
					break;
				default:
					mp_msg(MSGT_DEMUX, MSGL_V, "MOV: unknown handler class: 0x%X (%.4s)\n",
					       bswap_32(type), &type);
				}
				break;
			}
			case MOV_FOURCC('v', 'm', 'h', 'd'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sVideo header!\n", level, "");
				trak->type = MOV_TRAK_VIDEO;
				// read video data
				break;
			}
			case MOV_FOURCC('s', 'm', 'h', 'd'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sSound header!\n", level, "");
				trak->type = MOV_TRAK_AUDIO;
				// read audio data
				break;
			}
			case MOV_FOURCC('g', 'm', 'h', 'd'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sGeneric header!\n", level, "");
				trak->type = MOV_TRAK_GENERIC;
				break;
			}
			case MOV_FOURCC('n', 'm', 'h', 'd'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sGeneric header!\n", level, "");
				trak->type = MOV_TRAK_GENERIC;
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'd'):
			{
				int i = stream_read_dword(demux->stream);	// temp!
				int count = stream_read_dword(demux->stream);

				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sDescription list! (cnt:%d)\n", level, "",
				       count);
				for (i = 0; i < count; i++)
				{
					unsigned int pos = stream_tell(demux->stream);
					unsigned int len = stream_read_dword(demux->stream);
					unsigned int fourcc = stream_read_dword_le(demux->stream);

					/* some files created with Broadcast 2000 (e.g. ilacetest.mov)
					   contain raw I420 video but have a yv12 fourcc */
					if (fourcc == mmioFOURCC('y', 'v', '1', '2'))
						fourcc = mmioFOURCC('I', '4', '2', '0');
					if (len < 8)
						break;	// error
					mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*s desc #%d: %.4s  (%d bytes)\n", level,
					       "", i, &fourcc, len - 16);
					if (fourcc != trak->fourcc && i)
						mp_msg(MSGT_DEMUX, MSGL_WARN, MSGTR_MOVvariableFourCC);
//          if(!i)
					{
						trak->fourcc = fourcc;
						// read type specific (audio/video/time/text etc) header
						// NOTE: trak type is not yet known at this point :(((
						trak->stdata_len = len - 8;
						trak->stdata = (unsigned char *) mem_malloc(trak->stdata_len);
						stream_read(demux->stream, trak->stdata, trak->stdata_len);
					}
					if (!stream_seek(demux->stream, pos + len))
						break;
				}
				break;
			}
			case MOV_FOURCC('s', 't', 't', 's'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;
				unsigned int pts = 0;

				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sSample duration table! (%d blocks)\n",
				       level, "", len);
				trak->durmap = (mov_durmap_t *) mem_malloc(sizeof(mov_durmap_t) * len);
				memset(trak->durmap, 0, sizeof(mov_durmap_t) * len);
				trak->durmap_size = len;
				for (i = 0; i < len; i++)
				{
					trak->durmap[i].num = stream_read_dword(demux->stream);
					trak->durmap[i].dur = stream_read_dword(demux->stream);
					pts += trak->durmap[i].num * trak->durmap[i].dur;
				}
				
				if (trak->length != pts)
					mp_msg(MSGT_DEMUX, MSGL_WARN, "Warning! pts=%d  length=%d\n", pts,
					       trak->length);
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'c'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

				mp_msg(MSGT_DEMUX, MSGL_V,
				       "MOV: %*sSample->Chunk mapping table!  (%d blocks) (ver:%d,flags:%ld)\n",
				       level, "", len, ver, flags);
				// read data:
				trak->chunkmap_size = len;
				trak->chunkmap = (mov_chunkmap_t *) mem_malloc(sizeof(mov_chunkmap_t) * len);
				for (i = 0; i < len; i++)
				{
					trak->chunkmap[i].first = stream_read_dword(demux->stream) - 1;
					trak->chunkmap[i].spc = stream_read_dword(demux->stream);
					trak->chunkmap[i].sdid = stream_read_dword(demux->stream);
				}
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'z'):
			{
				int temp = stream_read_dword(demux->stream);
				int ss = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int entries = stream_read_dword(demux->stream);
				int i, len = 0;

				mp_msg(MSGT_DEMUX, MSGL_V,
				       "MOV: %*sSample size table! (entries=%d ss=%d) (ver:%d,flags:%ld)\n",
				       level, "", entries, ss, ver, flags);
				trak->samplesize = ss;
				if (!ss)
				{
					// variable samplesize
					for (i = 0; i < trak->durmap_size; i++)
					{
						len += trak->durmap[i].num;
					}
					if (len > entries)
						trak->samples =	(mov_sample_t *) mem_reallocm(trak->samples,
						                sizeof(mov_sample_t) * len);
					else
						trak->samples =	(mov_sample_t *) mem_reallocm(trak->samples,
						                sizeof(mov_sample_t) * entries);
					trak->samples_size = entries;
					sample_num_entries=entries;
					for (i = 0; i < entries; i++)
						trak->samples[i].size = stream_read_dword(demux->stream);
				}
				break;
			}
			case MOV_FOURCC('s', 't', 'c', 'o'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;

				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sChunk offset table! (%d chunks)\n", level,
				       "", len);
				// extend array if needed:
				if (len > trak->chunks_size)
				{
					trak->chunks =
					    (mov_chunk_t *) mem_reallocm(trak->chunks, sizeof(mov_chunk_t) * len);
					trak->chunks_size = len;
				}
				// read elements:
				for (i = 0; i < len; i++)
					trak->chunks[i].pos = stream_read_dword(demux->stream);
				break;
			}
			case MOV_FOURCC('c', 'o', '6', '4'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;

				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*s64bit chunk offset table! (%d chunks)\n",
				       level, "", len);
				// extend array if needed:
				if (len > trak->chunks_size)
				{
					trak->chunks =
					    (mov_chunk_t *) mem_reallocm(trak->chunks, sizeof(mov_chunk_t) * len);
					trak->chunks_size = len;
				}
				// read elements:
				for (i = 0; i < len; i++)
				{
#ifndef	_LARGEFILE_SOURCE
					if (stream_read_dword(demux->stream) != 0)
						mp_msg(MSGT_DEMUX, MSGL_WARN,
						       "Chunk %d has got 64bit address, but you've MPlayer compiled without LARGEFILE support!\n",
						       i);
					trak->chunks[i].pos = stream_read_dword(demux->stream);
#else
					trak->chunks[i].pos = stream_read_qword(demux->stream);
#endif
				}
				break;
			}
			case MOV_FOURCC('s', 't', 's', 's'):
			{
				int temp = stream_read_dword(demux->stream);
				int entries = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

				mp_msg(MSGT_DEMUX, MSGL_V,
				       "MOV: %*sSyncing samples (keyframes) table! (%d entries) (ver:%d,flags:%ld)\n",
				       level, "", entries, ver, flags);
				trak->keyframes_size = entries;
				trak->keyframes = (unsigned int *) mem_malloc(sizeof(unsigned int) * entries);
				for (i = 0; i < entries; i++)
					trak->keyframes[i] = stream_read_dword(demux->stream) - 1;
//      for (i=0;i<entries;i++) MP_DPF("%3d: %d\n",i,trak->keyframes[i]);
				break;
			}
			case MOV_FOURCC('m', 'd', 'i', 'a'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sMedia stream!\n", level, "");
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('m', 'i', 'n', 'f'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sMedia info!\n", level, "");
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('s', 't', 'b', 'l'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sSample info!\n", level, "");
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('e', 'd', 't', 's'):
			{
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: %*sEdit atom!\n", level, "");
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('e', 'l', 's', 't'):
			{
				int temp = stream_read_dword(demux->stream);
				int entries = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

				mp_msg(MSGT_DEMUX, MSGL_V,
				       "MOV: %*sEdit list table (%d entries) (ver:%d,flags:%ld)\n", level, "",
				       entries, ver, flags);
#if 1
				trak->editlist_size = entries;
				trak->editlist =
				    (mov_editlist_t *) mem_malloc(trak->editlist_size * sizeof(mov_editlist_t));
				for (i = 0; i < entries; i++)
				{
					int dur = stream_read_dword(demux->stream);
					int mt = stream_read_dword(demux->stream);
					int mr = stream_read_dword(demux->stream);	// 16.16fp

					trak->editlist[i].dur = dur;
					trak->editlist[i].pos = mt;
					trak->editlist[i].speed = mr;
					mp_msg(MSGT_DEMUX, MSGL_V,
					       "MOV: %*s  entry#%d: duration: %d  start time: %d  speed: %3.1fx\n",
					       level, "", i, dur, mt, (float) mr / 65536.0f);
				}
#endif
				break;
			}
			case MOV_FOURCC('c', 'o', 'd', 'e'):
			{
				/* XXX: Implement atom 'code' for FLASH support */
			}
			default:
				id = be2me_32(id);
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: unknown chunk: %.4s %d\n", &id, (int) len);
				break;
			}					//switch(id)
		}
		else
		{						/* not in track */
			switch (id)
			{
			case MOV_FOURCC('m', 'v', 'h', 'd'):
			{
				stream_skip(demux->stream, 12);
				priv->timescale = stream_read_dword(demux->stream);
				priv->duration = stream_read_dword(demux->stream);
				if (priv->timescale != 0)
				{
					g_dwTotalSeconds = (DWORD) ((priv->duration + 0.5 * priv->timescale) / priv->timescale);
					exception_type.i1=g_dwTotalSeconds;

				}
				mp_msg(MSGT_DEMUX, MSGL_V,
				       "MOV: %*sMovie header (%d bytes): tscale=%d  dur=%d\n", level, "",
				       (int) len, (int) priv->timescale, (int) priv->duration);
				break;
			}
			case MOV_FOURCC('t', 'r', 'a', 'k'):
			{
//      if(trak) MP_DPF("MOV: Warning! trak in trak?\n");
				if (priv->track_db >= MOV_MAX_TRACKS)
				{
					mp_msg(MSGT_DEMUX, MSGL_WARN, MSGTR_MOVtooManyTrk);
					return;
				}
				if (!priv->track_db)
					mp_msg(MSGT_DEMUX, MSGL_INFO, "--------------\n");
				trak = (mov_track_t *) mem_malloc(sizeof(mov_track_t));
				memset(trak, 0, sizeof(mov_track_t));
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: Track #%d:\n", priv->track_db);
				trak->id = priv->track_db;
				priv->tracks[priv->track_db] = trak;
				lschunks(level + 1, pos + len, trak);
				mov_build_index(trak, priv->timescale);
				switch (trak->type)
				{
				case MOV_TRAK_AUDIO:
				{
#if 0
					struct
					{
						int16_t version;	// 0 or 1 (version 1 is qt3.0+)
						int16_t revision;	// 0
						int32_t vendor_id;	// 0
						int16_t channels;	// 1 or 2  (Mono/Stereo)
						int16_t samplesize;	// 8 or 16 (8Bit/16Bit)
						int16_t compression_id;	// if version 0 then 0
						// if version 1 and vbr then -2 else 0
						int16_t packet_size;	// 0
						uint16_t sample_rate;	// samplerate (Hz)
						// qt3.0+ (version == 1)
						uint32_t samples_per_packet;	// 0 or num uncompressed samples in a packet
						// if 0 below three values are also 0
						uint32_t bytes_per_packet;	// 0 or num compressed bytes for one channel
						uint32_t bytes_per_frame;	// 0 or num compressed bytes for all channels
						// (channels * bytes_per_packet)
						uint32_t bytes_per_sample;	// 0 or size of uncompressed sample
						// if samples_per_packet and bytes_per_packet are constant (CBR)
						// then bytes_per_frame and bytes_per_sample must be 0 (else is VBR)
						// ---
						// optional additional atom-based fields
						// ([int32_t size,int32_t type,some data ],repeat)
					} my_stdata;
#endif
					sh_audio_t *sh = new_sh_audio(demux, priv->track_db);

					sh->format = trak->fourcc;

					switch (sh->format)
					{
					case 0x726D6173:	/* samr */
						/* amr narrowband */
						trak->samplebytes = sh->samplesize = 1;
						trak->nchannels = sh->channels = 1;
						sh->samplerate = 8000;
						break;

					case 0x62776173:	/* sawb */
						/* amr wideband */
						trak->samplebytes = sh->samplesize = 1;
						trak->nchannels = sh->channels = 1;
						sh->samplerate = 16000;
						break;

					default:

// assumptions for below table: short is 16bit, int is 32bit, intfp is 16bit
// XXX: 32bit fixed point numbers (intfp) are only 2 Byte!
// short values are usually one byte leftpadded by zero
//   int values are usually two byte leftpadded by zero
//  stdata[]:
//  8   short   version
//  10  short   revision
//  12  int     vendor_id
//  16  short   channels
//  18  short   samplesize
//  20  short   compression_id
//  22  short   packet_size (==0)
//  24  intfp   sample_rate
//     (26  short)  unknown (==0)
//    ---- qt3.0+ (version>=1)
//  28  int     samples_per_packet
//  32  int     bytes_per_packet
//  36  int     bytes_per_frame
//  40  int     bytes_per_sample
// there may be additional atoms following at 28 (version 0)
// or 44 (version 1), eg. esds atom of .MP4 files
// esds atom:
//      28  int     atom size (bytes of int size, int type and data)
//      32  char[4] atom type (fourc charater code -> esds)
//      36  char[]      atom data (len=size-8)

						trak->samplebytes = sh->samplesize =
						                        char2short(trak->stdata, 18) >> 3;
						trak->nchannels = sh->channels = char2short(trak->stdata, 16);
						/*MP_DPF("MOV: timescale: %d samplerate: %d durmap: %d (%d) -> %d (%d)\n",
						   trak->timescale, char2short(trak->stdata,24), trak->durmap[0].dur,
						   trak->durmap[0].num, trak->timescale/trak->durmap[0].dur,
						   char2short(trak->stdata,24)/trak->durmap[0].dur); */

						sh->samplerate = char2short(trak->stdata, 24);
						if ((sh->samplerate < 7000) && trak->durmap)
						{
							switch (char2short(trak->stdata, 24) / trak->durmap[0].dur)
							{
								// TODO: add more cases.
							case 31:
								sh->samplerate = 32000;
								break;
							case 43:
								sh->samplerate = 44100;
								break;
							case 47:
								sh->samplerate = 48000;
								break;
							default:
								mp_msg(MSGT_DEMUX, MSGL_WARN,
								       "MOV: unable to determine audio samplerate, "
								       "assuming 44.1kHz (got %d)\n",
								       char2short(trak->stdata, 24) / trak->durmap[0].dur);
								sh->samplerate = 44100;
							}
						}
					}
					mp_msg(MSGT_DEMUX, MSGL_INFO, "Audio bits: %d  chans: %d  rate: %d\n",
					       sh->samplesize * 8, sh->channels, sh->samplerate);

					if (trak->stdata_len >= 44 && trak->stdata[9] >= 1)
					{
						mp_msg(MSGT_DEMUX, MSGL_V,
						       "Audio header: samp/pack=%d bytes/pack=%d bytes/frame=%d bytes/samp=%d  \n",
						       char2int(trak->stdata, 28), char2int(trak->stdata,
						                                            32),
						       char2int(trak->stdata, 36), char2int(trak->stdata, 40));
						if (trak->stdata_len >= 44 + 8)
						{
							int len = char2int(trak->stdata, 44);
							int fcc = char2int(trak->stdata, 48);

							// we have extra audio headers!!!
							MP_DPF("Audio extra header: len=%d  fcc=0x%X\n", len, fcc);
							if ((len >= 4) &&
							        (char2int(trak->stdata, 52) >= 12) &&
							        (char2int(trak->stdata, 52 + 4) ==
							         MOV_FOURCC('f', 'r', 'm', 'a'))
							        && (char2int(trak->stdata, 52 + 8) ==
							            MOV_FOURCC('a', 'l', 'a', 'c'))
							        && (len >= 36 + char2int(trak->stdata, 52)))
							{
								sh->codecdata_len =
								    char2int(trak->stdata, 52 + char2int(trak->stdata, 52));
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found alac atom (%d)!\n", sh->codecdata_len);
								sh->codecdata =
								    (unsigned char *) mem_malloc(sh->codecdata_len);
								memcpy(sh->codecdata,
								       &trak->stdata[52 + char2int(trak->stdata, 52)],
								       sh->codecdata_len);
							}
							else
							{
								sh->codecdata_len = len - 8;
								sh->codecdata = trak->stdata + 44 + 8;
							}
						}
					}

					if ((trak->stdata[9] == 0 || trak->stdata[9] == 1)
					        && trak->stdata_len >= 36)
					{	// version 0 with extra atoms
						int adjust = (trak->stdata[9] == 1) ? 48 : 0;
						int atom_len = char2int(trak->stdata, 28 + adjust);

						switch (char2int(trak->stdata, 32 + adjust))
						{	// atom type
#if VIDEO_ENABLE
						case MOV_FOURCC('e', 's', 'd', 's'):
						{
							mp_msg(MSGT_DEMUX, MSGL_INFO,
							       "MOV: Found MPEG4 audio Elementary Stream Descriptor atom (%d)!\n",
							       atom_len);
							if (atom_len > 8)
							{
								esds_t esds;

								if (!mp4_parse_esds
								        (&trak->stdata[36 + adjust], atom_len - 8, &esds))
								{

									sh->i_bps = esds.avgBitrate / 8;

//              MP_DPF("######## audio format = %d ########\n",esds.objectTypeId);
									if (esds.objectTypeId == MP4OTI_MPEG1Audio
									        || esds.objectTypeId == MP4OTI_MPEG2AudioPart3)
										sh->format = 0x55;	// .mp3

									// dump away the codec specific configuration for the AAC decoder
									if (esds.decoderConfigLen)
									{
										if ((esds.decoderConfig[0] >> 3) == 29)
											sh->format = 0x1d61346d;	// request multi-channel mp3 decoder

										sh->codecdata_len = esds.decoderConfigLen;
										sh->codecdata =
										    (unsigned char *) mem_malloc(sh->codecdata_len);
										memcpy(sh->codecdata, esds.decoderConfig, sh->codecdata_len);

									}
								}
								mp4_free_esds(&esds);	// freeup esds mem
#if 0
								{
									FILE *f = fopen("esds.dat", "wb");

									fwrite(&trak->stdata[36], atom_len - 8, 1, f);
									fclose(f);
								}
#endif
							}
						}
						break;
#endif
						case MOV_FOURCC('a', 'l', 'a', 'c'):
						{
							mp_msg(MSGT_DEMUX, MSGL_INFO,
							       "MOV: Found alac atom (%d)!\n", atom_len);
							if (atom_len > 8)
							{
								// copy all the atom (not only payload) for lavc alac decoder
								sh->codecdata_len = atom_len;
								sh->codecdata =
								    (unsigned char *) mem_malloc(sh->codecdata_len);
								memcpy(sh->codecdata, &trak->stdata[28],
								       sh->codecdata_len);
							}
						}
						break;
						default:
							mp_msg(MSGT_DEMUX, MSGL_INFO,
							       "MOV: Found unknown audio atom %c%c%c%c (%d)!\n",
							       trak->stdata[32 + adjust], trak->stdata[33 + adjust],
							       trak->stdata[34 + adjust], trak->stdata[35 + adjust],
							       atom_len);
						}
					}
					mp_msg(MSGT_DEMUX, MSGL_INFO, "Fourcc: %.4s\n", &trak->fourcc);
#if 0
					{
						FILE *f = fopen("stdata.dat", "wb");

						fwrite(trak->stdata, trak->stdata_len, 1, f);
						fclose(f);
					}
					{
						FILE *f = fopen("tkdata.dat", "wb");

						fwrite(trak->tkdata, trak->tkdata_len, 1, f);
						fclose(f);
					}
#endif
					// Emulate WAVEFORMATEX struct:
					sh->wf = (WAVEFORMATEX *) mem_malloc(sizeof(WAVEFORMATEX));
					memset(sh->wf, 0, sizeof(WAVEFORMATEX));
					sh->wf->nChannels = sh->channels;
					sh->wf->wBitsPerSample = (trak->stdata[18] << 8) + trak->stdata[19];
					//Fix the issue that bitspersample information does not match between  sh->wf->wBitsPerSample and trak->samplesize
					if (sh->format == PCM_RAW ||sh->format == PCM_TWOS||sh->format == PCM_SOWT)
					{

						if (sh->wf->wBitsPerSample / 8 != trak->samplesize)
							trak->samplesize = sh->wf->wBitsPerSample / 8;
					}
					if (sh->format == ulaw ||sh->format ==ULAW ||sh->format == 0x7)
					{
						if (sh->channels == 2 && sh->wf->wBitsPerSample / 8 != trak->samplesize)
							trak->samplesize = sh->wf->wBitsPerSample / 8;
					}
					// sh->wf->nSamplesPerSec=trak->timescale;
					sh->wf->nSamplesPerSec =sh->samplerate; // (trak->stdata[24] << 8) + trak->stdata[25];
					if (trak->stdata_len >= 44 && trak->stdata[9] >= 1
					        && char2int(trak->stdata, 28) > 0)
					{
						//Audio header: samp/pack=4096 bytes/pack=743 bytes/frame=1486 bytes/samp=2
						sh->wf->nAvgBytesPerSec =
						    (sh->wf->nChannels * sh->wf->nSamplesPerSec *
						     char2int(trak->stdata, 32) + char2int(trak->stdata,
						                                           28) / 2) /
						    char2int(trak->stdata, 28);
						sh->wf->nBlockAlign = char2int(trak->stdata, 36);
					}
					else
					{
						sh->wf->nAvgBytesPerSec =
						    sh->wf->nChannels * sh->wf->wBitsPerSample *
						    sh->wf->nSamplesPerSec / 8;
						// workaround for ms11 ima4
						if (sh->format == 0x1100736d && trak->stdata_len >= 36)
							sh->wf->nBlockAlign = char2int(trak->stdata, 36);
					}
					// Selection:
//      if(demuxer->audio->id==-1 || demuxer->audio->id==priv->track_db){
//          // (auto)selected audio track:
//          demuxer->audio->id=priv->track_db;
//          demuxer->audio->sh=sh; sh->ds=demuxer->audio;
//      }
					break;
				}
				case MOV_TRAK_VIDEO:
				{
					int i, entry;
					int flag, start, count_flag, end, palette_count, gray;
					int hdr_ptr = 76;	// the byte just after depth
					unsigned char *palette_map;
					sh_video_t *sh = new_sh_video(demux, priv->track_db);
					int depth = trak->stdata[75] | (trak->stdata[74] << 8);

					sh->format = trak->fourcc;

//  stdata[]:
//  8   short   version
//  10  short   revision
//  12  int     vendor_id
//  16  int     temporal_quality
//  20  int     spatial_quality
//  24  short   width
//  26  short   height
//  28  int     h_dpi
//  32  int     v_dpi
//  36  int     0
//  40  short   frames_per_sample
//  42  char[4] compressor_name
//  74  short   depth
//  76  short   color_table_id
// additional atoms may follow,
// eg esds atom from .MP4 files
//      78  int     atom size
//      82  char[4] atom type
//  86  ...     atom data

					{
						ImageDescription *id =
						    (ImageDescription *) mem_malloc(8 + trak->stdata_len);
						trak->desc = id;
						id->idSize = 8 + trak->stdata_len;
//      id->cType=bswap_32(trak->fourcc);
						id->cType = le2me_32(trak->fourcc);
						id->version = char2short(trak->stdata, 8);
						id->revisionLevel = char2short(trak->stdata, 10);
						id->vendor = char2int(trak->stdata, 12);
						id->temporalQuality = char2int(trak->stdata, 16);
						id->spatialQuality = char2int(trak->stdata, 20);
						id->width = char2short(trak->stdata, 24);
						id->height = char2short(trak->stdata, 26);
						id->hRes = char2int(trak->stdata, 28);
						id->vRes = char2int(trak->stdata, 32);
						id->dataSize = char2int(trak->stdata, 36);
						id->frameCount = char2short(trak->stdata, 40);
						memcpy(&id->name, trak->stdata + 42, 32);
						id->depth = char2short(trak->stdata, 74);
						id->clutID = char2short(trak->stdata, 76);
						if (trak->stdata_len > 78)
							memcpy(((char *) &id->clutID) + 2, trak->stdata + 78,
							       trak->stdata_len - 78);
						sh->ImageDesc = id;
#if 0
						{
							FILE *f = fopen("ImageDescription", "wb");

							fwrite(id, id->idSize, 1, f);
							fclose(f);
						}
#endif
					}

					if (trak->stdata_len >= 86)
					{	// extra atoms found
						int pos = 78;
						int atom_len;

						while (pos + 8 <= trak->stdata_len &&
						        (pos + (atom_len = char2int(trak->stdata, pos))) <=
						        trak->stdata_len)
						{
							switch (char2int(trak->stdata, pos + 4))
							{	// switch atom type
							case MOV_FOURCC('g', 'a', 'm', 'a'):
								// intfp with gamma value at which movie was captured
								// can be used to gamma correct movie display
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found unsupported Gamma-Correction movie atom (%d)!\n",
								       atom_len);
								break;
							case MOV_FOURCC('f', 'i', 'e', 'l'):
								// 2 char-values (8bit int) that specify field handling
								// see the Apple's QuickTime Fileformat PDF for more info
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found unsupported Field-Handling movie atom (%d)!\n",
								       atom_len);
								break;
							case MOV_FOURCC('m', 'j', 'q', 't'):
								// Motion-JPEG default quantization table
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found unsupported MJPEG-Quantization movie atom (%d)!\n",
								       atom_len);
								break;
							case MOV_FOURCC('m', 'j', 'h', 't'):
								// Motion-JPEG default huffman table
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found unsupported MJPEG-Huffman movie atom (%d)!\n",
								       atom_len);
								break;
#if VIDEO_ENABLE
							case MOV_FOURCC('e', 's', 'd', 's'):
								// MPEG4 Elementary Stream Descriptor header
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found MPEG4 movie Elementary Stream Descriptor atom (%d)!\n",
								       atom_len);
								// add code here to save esds header of length atom_len-8
								// beginning at stdata[86] to some variable to pass it
								// on to the decoder ::atmos
								if (atom_len > 8)
								{
									esds_t esds;

									if (!mp4_parse_esds
									        (trak->stdata + pos + 8, atom_len - 8, &esds))
									{

										if (esds.objectTypeId == MP4OTI_MPEG2VisualSimple
										        || esds.objectTypeId == MP4OTI_MPEG2VisualMain
										        || esds.objectTypeId == MP4OTI_MPEG2VisualSNR
										        || esds.objectTypeId ==
										        MP4OTI_MPEG2VisualSpatial
										        || esds.objectTypeId == MP4OTI_MPEG2VisualHigh
										        || esds.objectTypeId == MP4OTI_MPEG2Visual422)
											sh->format = mmioFOURCC('m', 'p', 'g', '2');
										else if (esds.objectTypeId == MP4OTI_MPEG1Visual)
											sh->format = mmioFOURCC('m', 'p', 'g', '1');

										// dump away the codec specific configuration for the AAC decoder
										trak->stream_header_len = esds.decoderConfigLen;
										trak->stream_header =
										    (unsigned char *) mem_malloc(trak->
										                                 stream_header_len);
										memcpy(trak->stream_header, esds.decoderConfig,
										       trak->stream_header_len);
									}
									mp4_free_esds(&esds);	// freeup esds mem
								}
								break;
#endif
							case MOV_FOURCC('a', 'v', 'c', 'C'):
								// AVC decoder configuration record
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: AVC decoder configuration record atom (%d)!\n",
								       atom_len);
								if (atom_len > 8)
								{
									int i, poffs, cnt;

									// Parse some parts of avcC, just for fun :)
									// real parsing is done by avc1 decoder
									mp_msg(MSGT_DEMUX, MSGL_V, "MOV: avcC version: %d\n",
									       *(trak->stdata + pos + 8));
									if (*(trak->stdata + pos + 8) != 1)
										mp_msg(MSGT_DEMUX, MSGL_ERR,
										       "MOV: unknown avcC version (%d). Expexct problems.\n",
										       *(trak->stdata + pos + 9));
									mp_msg(MSGT_DEMUX, MSGL_V, "MOV: avcC profile: %d\n",
									       *(trak->stdata + pos + 9));
									mp_msg(MSGT_DEMUX, MSGL_V,
									       "MOV: avcC profile compatibility: %d\n",
									       *(trak->stdata + pos + 10));
									mp_msg(MSGT_DEMUX, MSGL_V, "MOV: avcC level: %d\n",
									       *(trak->stdata + pos + 11));
									mp_msg(MSGT_DEMUX, MSGL_V,
									       "MOV: avcC nal length size: %d\n",
									       ((*(trak->stdata + pos + 12)) & 0x03) + 1);
									mp_msg(MSGT_DEMUX, MSGL_V,
									       "MOV: avcC number of sequence param sets: %d\n",
									       cnt = (*(trak->stdata + pos + 13) & 0x1f));
									poffs = pos + 14;
									for (i = 0; i < cnt; i++)
									{
										mp_msg(MSGT_DEMUX, MSGL_V,
										       "MOV: avcC sps %d have length %d\n", i,
										       BE_16(trak->stdata + poffs));
										poffs += BE_16(trak->stdata + poffs) + 2;
									}
									mp_msg(MSGT_DEMUX, MSGL_V,
									       "MOV: avcC number of picture param sets: %d\n",
									       *(trak->stdata + poffs));
									poffs++;
									for (i = 0; i < cnt; i++)
									{
										mp_msg(MSGT_DEMUX, MSGL_V,
										       "MOV: avcC pps %d have length %d\n", i,
										       BE_16(trak->stdata + poffs));
										poffs += BE_16(trak->stdata + poffs) + 2;
									}
									// Copy avcC for the AVC decoder
									// This data will be put in extradata below, where BITMAPINFOHEADER is created
									trak->stream_header_len = atom_len - 8;
									trak->stream_header =
									    (unsigned char *) mem_malloc(trak->
									                                 stream_header_len);
									memcpy(trak->stream_header, trak->stdata + pos + 8,
									       trak->stream_header_len);
								}
								break;
							case 0:
								break;
							default:
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "MOV: Found unknown movie atom %c%c%c%c (%d)!\n",
								       trak->stdata[pos + 4], trak->stdata[pos + 5],
								       trak->stdata[pos + 6], trak->stdata[pos + 7],
								       atom_len);
							}
							if (atom_len < 8)
								break;
							pos += atom_len;
//         MP_DPF("pos=%d max=%d\n",pos,trak->stdata_len);
						}
					}
					sh->fps = trak->timescale /
					          ((trak->durmap_size >= 1) ? (float) trak->durmap[0].dur : 1);
					sh->frametime = 1.0f / sh->fps;

					sh->disp_w = trak->stdata[25] | (trak->stdata[24] << 8);
					sh->disp_h = trak->stdata[27] | (trak->stdata[26] << 8);
					// if image size is zero, fallback to display size
					if (!sh->disp_w && !sh->disp_h)
					{
						sh->disp_w = trak->tkdata[77] | (trak->tkdata[76] << 8);
						sh->disp_h = trak->tkdata[81] | (trak->tkdata[80] << 8);
					}
					else if (sh->disp_w != (trak->tkdata[77] | (trak->tkdata[76] << 8)))
					{
						// codec and display width differ... use display one for aspect
						sh->aspect = trak->tkdata[77] | (trak->tkdata[76] << 8);
						sh->aspect /= trak->tkdata[81] | (trak->tkdata[80] << 8);
					}

					if (depth > 32 + 8)
						MP_DPF("*** depth = 0x%X\n", depth);

					// palettized?
					gray = 0;
					if (depth > 32)
					{
						depth &= 31;
						gray = 1;
					}	// depth > 32 means grayscale
					if ((depth == 2) || (depth == 4) || (depth == 8))
						palette_count = (1 << depth);
					else
						palette_count = 0;

					// emulate BITMAPINFOHEADER:
					if (palette_count)
					{
						sh->bih =
						    (BITMAPINFOHEADER *) mem_malloc(sizeof(BITMAPINFOHEADER)
						                                    + palette_count * 4);
						memset(sh->bih, 0, sizeof(BITMAPINFOHEADER) + palette_count * 4);
						sh->bih->biSize = 40 + palette_count * 4;
						// fetch the relevant fields
						flag = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						start = BE_32(&trak->stdata[hdr_ptr]);
						hdr_ptr += 4;
						count_flag = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						end = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						palette_map = (unsigned char *) sh->bih + 40;
						mp_msg(MSGT_DEMUX, MSGL_INFO,
						       "Allocated %d entries for palette\n", palette_count);
						mp_msg(MSGT_DEMUX, MSGL_DBG2,
						       "QT palette: start: %x, end: %x, count flag: %d, flags: %x\n",
						       start, end, count_flag, flag);

						/* XXX: problems with sample (statunit6.mov) with flag&0x4 set! - alex */

						// load default palette
						if (flag & 0x08)
						{
							if (gray)
							{
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "Using default QT grayscale palette\n");
								if (palette_count == 16)
									memcpy(palette_map, qt_default_grayscale_palette_16,
									       16 * 4);
								else if (palette_count == 256)
								{
									memcpy(palette_map, qt_default_grayscale_palette_256,
									       256 * 4);
									if (trak->fourcc == mmioFOURCC('c', 'v', 'i', 'd'))
									{
										int i;

										// Hack for grayscale CVID, negative palette
										// If you have samples where this is not required contact me (rxt)
										mp_msg(MSGT_DEMUX, MSGL_INFO,
										       "MOV: greyscale cvid with default palette,"
										       " enabling negative palette hack.\n");
										for (i = 0; i < 256 * 4; i++)
											palette_map[i] = palette_map[i] ^ 0xff;
									}
								}
							}
							else
							{
								mp_msg(MSGT_DEMUX, MSGL_INFO,
								       "Using default QT colour palette\n");
								if (palette_count == 4)
									memcpy(palette_map, qt_default_palette_4, 4 * 4);
								else if (palette_count == 16)
									memcpy(palette_map, qt_default_palette_16, 16 * 4);
								else if (palette_count == 256)
									memcpy(palette_map, qt_default_palette_256, 256 * 4);
							}
						}
						// load palette from file
						else
						{
							mp_msg(MSGT_DEMUX, MSGL_INFO, "Loading palette from file\n");
							for (i = start; i <= end; i++)
							{
								entry = BE_16(&trak->stdata[hdr_ptr]);
								hdr_ptr += 2;
								// apparently, if count_flag is set, entry is same as i
								if (count_flag & 0x8000)
									entry = i;
								// only care about top 8 bits of 16-bit R, G, or B value
								if (entry <= palette_count && entry >= 0)
								{
									palette_map[entry * 4 + 2] = trak->stdata[hdr_ptr + 0];
									palette_map[entry * 4 + 1] = trak->stdata[hdr_ptr + 2];
									palette_map[entry * 4 + 0] = trak->stdata[hdr_ptr + 4];
									mp_msg(MSGT_DEMUX, MSGL_DBG2,
									       "QT palette: added entry: %d of %d (colors: R:%x G:%x B:%x)\n",
									       entry, palette_count,
									       palette_map[entry * 4 + 2],
									       palette_map[entry * 4 + 1],
									       palette_map[entry * 4 + 0]);
								}
								else
									mp_msg(MSGT_DEMUX, MSGL_V,
									       "QT palette: skipped entry (out of count): %d of %d\n",
									       entry, palette_count);
								hdr_ptr += 6;
							}
						}
					}
					else
					{
						if (trak->fourcc == mmioFOURCC('a', 'v', 'c', '1'))
						{
							sh->bih =
							    (BITMAPINFOHEADER *)
							    mem_malloc(sizeof(BITMAPINFOHEADER) +
							               trak->stream_header_len);
							memset(sh->bih, 0,
							       sizeof(BITMAPINFOHEADER) + trak->stream_header_len);
							sh->bih->biSize = 40 + trak->stream_header_len;
							memcpy(((unsigned char *) sh->bih) + 40,
							       trak->stream_header, trak->stream_header_len);
							mem_free(trak->stream_header);
							trak->stream_header_len = 0;
							trak->stream_header = NULL;
						}
						else
						{
							sh->bih =
							    (BITMAPINFOHEADER *) mem_malloc(sizeof(BITMAPINFOHEADER));
							memset(sh->bih, 0, sizeof(BITMAPINFOHEADER));
							sh->bih->biSize = 40;
						}
					}
					sh->bih->biWidth = sh->disp_w;
					sh->bih->biHeight = sh->disp_h;
					sh->bih->biPlanes = 0;
					sh->bih->biBitCount = depth;
					sh->bih->biCompression = trak->fourcc;
					sh->bih->biSizeImage = sh->bih->biWidth * sh->bih->biHeight;

					mp_msg(MSGT_DEMUX, MSGL_INFO, "Image size: %d x %d (%d bpp)\n",
					       sh->disp_w, sh->disp_h, sh->bih->biBitCount);
					if (trak->tkdata_len > 81)
						mp_msg(MSGT_DEMUX, MSGL_INFO, "Display size: %d x %d\n",
						       trak->tkdata[77] | (trak->tkdata[76] << 8),
						       trak->tkdata[81] | (trak->tkdata[80] << 8));
					mp_msg(MSGT_DEMUX, MSGL_INFO, "Fourcc: %.4s  Codec: '%.*s'\n",
					       &trak->fourcc, trak->stdata[42] & 31, trak->stdata + 43);

//      if(demuxer->video->id==-1 || demuxer->video->id==priv->track_db){
//          // (auto)selected video track:
//          demuxer->video->id=priv->track_db;
//          demuxer->video->sh=sh; sh->ds=demuxer->video;
//      }
					break;
				}
				case MOV_TRAK_GENERIC:
					mp_msg(MSGT_DEMUX, MSGL_INFO,
					       "Generic track - not completely understood! (id: %d)\n", trak->id);
					/* XXX: Also this contains the FLASH data */

#if 0
					{
						int pos = stream_tell(demuxer->stream);
						int i;
						int fd;
						char name[20];

						for (i = 0; i < trak->samples_size; i++)
						{
							char buf[trak->samples[i].size];

							stream_seek(demuxer->stream, trak->samples[i].pos);
							snprintf((char *) &name[0], 20, "samp%d", i);
							fd = open((char *) &name[0], O_CREAT | O_WRONLY);
							stream_read(demuxer->stream, &buf[0], trak->samples[i].size);
							write(fd, &buf[0], trak->samples[i].size);
							close(fd);
						}
						for (i = 0; i < trak->chunks_size; i++)
						{
							char buf[trak->length];

							stream_seek(demuxer->stream, trak->chunks[i].pos);
							snprintf((char *) &name[0], 20, "chunk%d", i);
							fd = open((char *) &name[0], O_CREAT | O_WRONLY);
							stream_read(demuxer->stream, &buf[0], trak->length);
							write(fd, &buf[0], trak->length);
							close(fd);
						}
						if (trak->samplesize > 0)
						{
							char *buf;

							buf = malloc(trak->samplesize);
							stream_seek(demuxer->stream, trak->chunks[0].pos);
							snprintf((char *) &name[0], 20, "trak%d", trak->id);
							fd = open((char *) &name[0], O_CREAT | O_WRONLY);
							stream_read(demuxer->stream, buf, trak->samplesize);
							write(fd, buf, trak->samplesize);
							close(fd);
						}
						stream_seek(demuxer->stream, pos);
					}
#endif
					break;
				default:
					mp_msg(MSGT_DEMUX, MSGL_INFO, "Unknown track type found (type: %d)\n",
					       trak->type);
					break;
				}
				mp_msg(MSGT_DEMUX, MSGL_INFO, "--------------\n");
				priv->track_db++;
				trak = NULL;
				break;
			}
#ifndef HAVE_ZLIB
			case MOV_FOURCC('c', 'm', 'o', 'v'):
			{
				mp_msg(MSGT_DEMUX, MSGL_ERR, MSGTR_MOVcomprhdr);
				return;
			}
#else
			case MOV_FOURCC('m', 'o', 'o', 'v'):
			case MOV_FOURCC('c', 'm', 'o', 'v'):
			{
//      mp_msg(MSGT_DEMUX,MSGL_ERR,MSGTR_MOVcomprhdr);
				lschunks(level + 1, pos + len, NULL);
				break;
			}
			case MOV_FOURCC('d', 'c', 'o', 'm'):
			{
//      int temp=stream_read_dword(demuxer->stream);
				unsigned int algo = be2me_32(stream_read_dword(demux->stream));

				mp_msg(MSGT_DEMUX, MSGL_INFO, "Compressed header uses %.4s algo!\n", &algo);
				break;
			}
			case MOV_FOURCC('c', 'm', 'v', 'd'):
			{
//      int temp=stream_read_dword(demuxer->stream);
				unsigned int moov_sz = stream_read_dword(demux->stream);
				unsigned int cmov_sz = len - 4;
				unsigned char *cmov_buf = mem_malloc(cmov_sz);
				unsigned char *moov_buf = mem_malloc(moov_sz + 16);
				int zret;
				z_stream zstrm;
				stream_t *backup;

				mp_msg(MSGT_DEMUX, MSGL_INFO, "Compressed header size: %d / %d\n", cmov_sz,
				       moov_sz);

				stream_read(demux->stream, cmov_buf, cmov_sz);

				zstrm.zalloc = (alloc_func) 0;
				zstrm.zfree = (free_func) 0;
				zstrm.opaque = (voidpf) 0;
				zstrm.next_in = cmov_buf;
				zstrm.avail_in = cmov_sz;
				zstrm.next_out = moov_buf;
				zstrm.avail_out = moov_sz;

				zret = inflateInit(&zstrm);
				if (zret != Z_OK)
				{
					mp_msg(MSGT_DEMUX, MSGL_ERR, "QT cmov: inflateInit err %d\n", zret);
					return;
				}
				zret = inflate(&zstrm, Z_NO_FLUSH);
				if ((zret != Z_OK) && (zret != Z_STREAM_END))
				{
					mp_msg(MSGT_DEMUX, MSGL_ERR, "QT cmov inflate: ERR %d\n", zret);
					return;
				}
#if 0
				else
				{
					FILE *DecOut;

					DecOut = fopen("Out.bin", "w");
					fwrite(moov_buf, 1, moov_sz, DecOut);
					fclose(DecOut);
				}
#endif
				if (moov_sz != zstrm.total_out)
					mp_msg(MSGT_DEMUX, MSGL_WARN,
					       "Warning! moov size differs cmov: %d  zlib: %d\n", moov_sz,
					       zstrm.total_out);
				zret = inflateEnd(&zstrm);

				backup = demux->stream;
				demux->stream = new_memory_stream(moov_buf, moov_sz);
				stream_skip(demux->stream, 8);
				lschunks(level + 1, moov_sz, NULL);	// parse uncompr. 'moov'
				//free_stream(demuxer->stream);
				demux->stream = backup;
				mem_free(cmov_buf);
				mem_free(moov_buf);
				break;
			}
#endif
			case MOV_FOURCC('u', 'd', 't', 'a'):
			{
				unsigned int udta_id;
				unsigned int udta_len;
				unsigned int udta_size = len;

				mp_msg(MSGT_DEMUX, MSGL_DBG2, "mov: user data record found\n");
				mp_msg(MSGT_DEMUX, MSGL_V, "Quicktime Clip Info:\n");

				while ((len > 8) && (udta_size > 8))
				{
					udta_len = stream_read_dword(demux->stream);
					udta_id = stream_read_dword(demux->stream);
					udta_size -= 8;
					mp_msg(MSGT_DEMUX, MSGL_DBG2, "udta_id: %.4s (len: %d)\n", &udta_id,
					       udta_len);
					switch (udta_id)
					{
					case MOV_FOURCC(0xa9, 'c', 'p', 'y'):
					case MOV_FOURCC(0xa9, 'd', 'a', 'y'):
					case MOV_FOURCC(0xa9, 'd', 'i', 'r'):
						/* 0xa9,'e','d','1' - '9' : edit timestamps */
					case MOV_FOURCC(0xa9, 'f', 'm', 't'):
					case MOV_FOURCC(0xa9, 'i', 'n', 'f'):
					case MOV_FOURCC(0xa9, 'p', 'r', 'd'):
					case MOV_FOURCC(0xa9, 'p', 'r', 'f'):
					case MOV_FOURCC(0xa9, 'r', 'e', 'q'):
					case MOV_FOURCC(0xa9, 's', 'r', 'c'):
					case MOV_FOURCC('n', 'a', 'm', 'e'):
					case MOV_FOURCC(0xa9, 'n', 'a', 'm'):
					case MOV_FOURCC(0xa9, 'A', 'R', 'T'):
					case MOV_FOURCC(0xa9, 'c', 'm', 't'):
					case MOV_FOURCC(0xa9, 'a', 'u', 't'):
					case MOV_FOURCC(0xa9, 's', 'w', 'r'):
					{
						unsigned int text_len = stream_read_word(demux->stream);
						char text[text_len + 2 + 1];

						stream_read(demux->stream, (char *) &text, text_len + 2);
						text[text_len + 2] = 0x0;
						switch (udta_id)
						{
						case MOV_FOURCC(0xa9, 'a', 'u', 't'):
							//demux_info_add(demuxer, "author", &text[2]);
							mp_msg(MSGT_DEMUX, MSGL_V, " Author: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'c', 'p', 'y'):
							//demux_info_add(demuxer, "copyright", &text[2]);
							mp_msg(MSGT_DEMUX, MSGL_V, " Copyright: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'i', 'n', 'f'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Info: %s\n", &text[2]);
							break;
						case MOV_FOURCC('n', 'a', 'm', 'e'):
						case MOV_FOURCC(0xa9, 'n', 'a', 'm'):
							//demux_info_add(demuxer, "name", &text[2]);
							mp_msg(MSGT_DEMUX, MSGL_V, " Name: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'A', 'R', 'T'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Artist: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'd', 'i', 'r'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Director: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'c', 'm', 't'):
							//demux_info_add(demuxer, "comments", &text[2]);
							mp_msg(MSGT_DEMUX, MSGL_V, " Comment: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'r', 'e', 'q'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Requirements: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 's', 'w', 'r'):
							//demux_info_add(demuxer, "encoder", &text[2]);
							mp_msg(MSGT_DEMUX, MSGL_V, " Software: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'd', 'a', 'y'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Creation timestamp: %s\n",
							       &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'f', 'm', 't'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Format: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'p', 'r', 'd'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Producer: %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 'p', 'r', 'f'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Performer(s): %s\n", &text[2]);
							break;
						case MOV_FOURCC(0xa9, 's', 'r', 'c'):
							mp_msg(MSGT_DEMUX, MSGL_V, " Source providers: %s\n", &text[2]);
							break;
						}
						udta_size -= 4 + text_len;
						break;
					}
					/* some other shits:    WLOC - window location,
					   LOOP - looping style,
					   SelO - play only selected frames
					   AllF - play all frames
					 */
					case MOV_FOURCC('W', 'L', 'O', 'C'):
					case MOV_FOURCC('L', 'O', 'O', 'P'):
					case MOV_FOURCC('S', 'e', 'l', 'O'):
					case MOV_FOURCC('A', 'l', 'l', 'F'):
					default:
					{
						if (udta_len > udta_size)
							udta_len = udta_size;
						{
							char *dump = (char *) mem_malloc(udta_len - 4);
							stream_read(demux->stream, dump, udta_len - 4 - 4);
							udta_size -= udta_len;
							mem_free(dump);
						}
					}
					}
				}
				break;
			}				/* eof udta */
			default:
				id = be2me_32(id);
				mp_msg(MSGT_DEMUX, MSGL_V, "MOV: unknown chunk: %.4s %d\n", &id, (int) len);
			}					/* endof switch */
		}						/* endof else */

		pos += len + 8;
		if (pos >= endpos)
			break;
		if (!stream_seek(demux->stream, pos))
			break;
	}
}


#else
static void lschunks(int level, unsigned int endpos, mov_track_t * trak)
{
	mov_priv_t *priv = demux->priv;

// MP_DPF("lschunks (level=%d,endpos=%x)\n", level, endpos);
	while (1)
	{
		unsigned int pos;
		unsigned int len;
		unsigned int id;

		//
		pos = stream_tell(demux->stream);
//  MP_DPF("stream_tell==%d\n",pos);
		if (pos >= endpos)
			return;				// END
		len = stream_read_dword(demux->stream);
//  MP_DPF("len==%d\n",len);
		if (len < 8)
			return;				// error
		len -= 8;
		id = stream_read_dword(demux->stream);
		if (trak)
		{
			switch (id)
			{
			case MOV_FOURCC('m', 'd', 'a', 't'):
			{
				return;
			}
			case MOV_FOURCC('f', 'r', 'e', 'e'):
			case MOV_FOURCC('u', 'd', 't', 'a'):
				/* here not supported :p */
				break;
			case MOV_FOURCC('t', 'k', 'h', 'd'):
			{
				// read codec data
				trak->tkdata_len = len;
				trak->tkdata = (unsigned char *) mem_malloc(trak->tkdata_len);
				//CHK_MALLOC(trak->tkdata, "lschunks failed");
				stream_read(demux->stream, (char *) trak->tkdata, trak->tkdata_len);
				/*
				0  1 Version
				1  3 Flags
				4  4 Creation time
				8  4 Modification time
				12 4 Track ID
				16 4 Reserved
				20 4 Duration
				24 8 Reserved
				32 2 Layer
				34 2 Alternate group
				36 2 Volume
				38 2 Reserved
				40 36 Matrix structure
				76 4 Track width
				80 4 Track height
				*/
				break;
			}
			case MOV_FOURCC('m', 'd', 'h', 'd'):
			{
				stream_skip(demux->stream, 12);
				// read timescale
				trak->timescale = stream_read_dword(demux->stream);
				// read length
				trak->length = stream_read_dword(demux->stream);
				break;
			}
			case MOV_FOURCC('h', 'd', 'l', 'r'):
			{
				unsigned int tmp = stream_read_dword(demux->stream);
				unsigned int type = stream_read_dword_le(demux->stream);
				unsigned int subtype = stream_read_dword_le(demux->stream);
				unsigned int manufact = stream_read_dword_le(demux->stream);
				unsigned int comp_flags = stream_read_dword(demux->stream);
				unsigned int comp_mask = stream_read_dword(demux->stream);
				int len = stream_read_char(demux->stream);
				char *str = (char *) mem_malloc(len + 1);

				//CHK_MALLOC(str, "lschunks failed");
				stream_read(demux->stream, str, len);
				str[len] = 0;
				mem_free(str);
				switch (bswap_32(type))
				{
				case MOV_FOURCC('m', 'h', 'l', 'r'):
					trak->media_handler = bswap_32(subtype);
					break;
				case MOV_FOURCC('d', 'h', 'l', 'r'):
					trak->data_handler = bswap_32(subtype);
					break;
				default:
					break;
				}
				break;
			}
			case MOV_FOURCC('v', 'm', 'h', 'd'):
			{
				trak->type = MOV_TRAK_VIDEO;
				// read video data
				break;
			}
			case MOV_FOURCC('s', 'm', 'h', 'd'):
			{
				trak->type = MOV_TRAK_AUDIO;
				// read audio data
				break;
			}
			case MOV_FOURCC('g', 'm', 'h', 'd'):
			{
				trak->type = MOV_TRAK_GENERIC;
				break;
			}
			case MOV_FOURCC('n', 'm', 'h', 'd'):
			{
				trak->type = MOV_TRAK_GENERIC;
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'd'):
			{
				int i = stream_read_dword(demux->stream);	// temp!
				int count = stream_read_dword(demux->stream);

				for (i = 0; i < count; i++)
				{
					unsigned int pos = stream_tell(demux->stream);
					unsigned int len = stream_read_dword(demux->stream);
					unsigned int fourcc = stream_read_dword_le(demux->stream);

					if (len < 8)
						break;	// error
					if (fourcc != trak->fourcc && i)
					{
					}
//          if(!i)
					{
						trak->fourcc = fourcc;
						// read type specific (audio/video/time/text etc) header
						// NOTE: trak type is not yet known at this point :(((
						trak->stdata_len = len - 8;
						trak->stdata = (unsigned char *) mem_malloc(trak->stdata_len);
						//CHK_MALLOC(trak->stdata, "lschunks failed");
						stream_read(demux->stream, (char *) trak->stdata, trak->stdata_len);
					}
					if (!stream_seek(demux->stream, pos + len))
						break;
				}
				break;
			}
			case MOV_FOURCC('s', 't', 't', 's'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;
				unsigned int pts = 0;

				trak->durmap = (mov_durmap_t *) mem_malloc(sizeof(mov_durmap_t) * len);
				//CHK_MALLOC(trak->durmap, "lschunks failed");
				memset(trak->durmap, 0, sizeof(mov_durmap_t) * len);
				trak->durmap_size = len;
				for (i = 0; i < len; i++)
				{
					trak->durmap[i].num = stream_read_dword(demux->stream);
					trak->durmap[i].dur = stream_read_dword(demux->stream);
					pts += trak->durmap[i].num * trak->durmap[i].dur;
				}
				if (trak->length != pts)
				{
				}
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'c'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

				// read data:
				trak->chunkmap_size = len;
				trak->chunkmap = (mov_chunkmap_t *) mem_malloc(sizeof(mov_chunkmap_t) * len);
				//CHK_MALLOC(trak->chunkmap, "lschunks failed");
				for (i = 0; i < len; i++)
				{
					trak->chunkmap[i].first = stream_read_dword(demux->stream) - 1;
					trak->chunkmap[i].spc = stream_read_dword(demux->stream);
					trak->chunkmap[i].sdid = stream_read_dword(demux->stream);
				}
				break;
			}
			case MOV_FOURCC('s', 't', 's', 'z'):
			{
				int temp = stream_read_dword(demux->stream);
				int ss = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int entries = stream_read_dword(demux->stream);
				int i;

				trak->samplesize = ss;
				if (!ss)
				{
					// variable samplesize
					trak->samples =
					    (mov_sample_t *) mem_reallocm(trak->samples,
					                                  sizeof(mov_sample_t) * entries);
					trak->samples_size = entries;
					for (i = 0; i < entries; i++)
						trak->samples[i].size = stream_read_dword(demux->stream);
				}
				break;
			}
			case MOV_FOURCC('s', 't', 'c', 'o'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;

				// extend array if needed:
				if (len > trak->chunks_size)
				{
					trak->chunks =
					    (mov_chunk_t *) mem_reallocm(trak->chunks, sizeof(mov_chunk_t) * len);
					trak->chunks_size = len;
				}
				// read elements:
				for (i = 0; i < len; i++)
					trak->chunks[i].pos = stream_read_dword(demux->stream);
				break;
			}
			case MOV_FOURCC('c', 'o', '6', '4'):
			{
				int temp = stream_read_dword(demux->stream);
				int len = stream_read_dword(demux->stream);
				int i;

				// extend array if needed:
				if (len > trak->chunks_size)
				{
					trak->chunks =
					    (mov_chunk_t *) mem_reallocm(trak->chunks, sizeof(mov_chunk_t) * len);
					trak->chunks_size = len;
				}
				// read elements:
				for (i = 0; i < len; i++)
				{
#ifndef	_LARGEFILE_SOURCE
					if (stream_read_dword(demux->stream) != 0)
					{
					}

					trak->chunks[i].pos = stream_read_dword(demux->stream);
#else
trak->chunks[i].pos = stream_read_qword(demux->stream);
#endif
				}
				break;
			}
			case MOV_FOURCC('s', 't', 's', 's'):
			{
				int temp = stream_read_dword(demux->stream);
				int entries = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

				trak->keyframes_size = entries;
				trak->keyframes = (unsigned int *) mem_malloc(sizeof(unsigned int) * entries);
				//CHK_MALLOC(trak->keyframes, "lschunks failed");
				for (i = 0; i < entries; i++)
					trak->keyframes[i] = stream_read_dword(demux->stream) - 1;
//      for (i=0;i<entries;i++) MP_DPF("%3d: %d\n",i,trak->keyframes[i]);
				break;
			}
			case MOV_FOURCC('m', 'd', 'i', 'a'):
			{
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('m', 'i', 'n', 'f'):
			{
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('s', 't', 'b', 'l'):
			{
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('e', 'd', 't', 's'):
			{
				lschunks(level + 1, pos + len, trak);
				break;
			}
			case MOV_FOURCC('e', 'l', 's', 't'):
			{
				int temp = stream_read_dword(demux->stream);
				int entries = stream_read_dword(demux->stream);
				int ver = (temp << 24);
				int flags = (temp << 16) | (temp << 8) | temp;
				int i;

#if 1
				trak->editlist_size = entries;
				trak->editlist =
				    (mov_editlist_t *) mem_malloc(trak->editlist_size * sizeof(mov_editlist_t));
				//CHK_MALLOC(trak->editlist, "lschunks failed");
				for (i = 0; i < entries; i++)
				{
					int dur = stream_read_dword(demux->stream);
					int mt = stream_read_dword(demux->stream);
					int mr = stream_read_dword(demux->stream);	// 16.16fp

					trak->editlist[i].dur = dur;
					trak->editlist[i].pos = mt;
					trak->editlist[i].speed = mr;
				}
#endif
				break;
			}
			case MOV_FOURCC('c', 'o', 'd', 'e'):
			{
				/* XXX: Implement atom 'code' for FLASH support */
			}
			default:
				id = be2me_32(id);
				break;
			}					//switch(id)
		}
		else
		{						/* not in track */
			switch (id)
			{
			case MOV_FOURCC('m', 'v', 'h', 'd'):
			{
				stream_skip(demux->stream, 12);
				priv->timescale = stream_read_dword(demux->stream);
				priv->duration = stream_read_dword(demux->stream);
				//mpDebugPrint("MOV priv->timescale = %d", priv->timescale);
				if (priv->timescale != 0)
				{
					g_dwTotalSeconds = (DWORD) ((priv->duration + 0.5 * priv->timescale) / priv->timescale);
				}
				break;
			}
			case MOV_FOURCC('t', 'r', 'a', 'k'):
			{
//      if(trak) MP_DPF("MOV: Warning! trak in trak?\n");
				if (priv->track_db >= MOV_MAX_TRACKS)
				{
					return;
				}
				if (!priv->track_db)
				{
				}
				trak = (mov_track_t *) mem_malloc(sizeof(mov_track_t));
				//CHK_MALLOC(trak, "lschunks failed");
				memset(trak, 0, sizeof(mov_track_t));
				trak->id = priv->track_db;
				priv->tracks[priv->track_db] = trak;
				lschunks(level + 1, pos + len, trak);
				mov_build_index(trak, priv->timescale);
				switch (trak->type)
				{
				case MOV_TRAK_AUDIO:
				{

					sh_audio_t *sh = new_sh_audio(demux, priv->track_db);

					sh->format = trak->fourcc;

// assumptions for below table: short is 16bit, int is 32bit, intfp is 16bit
// XXX: 32bit fixed point numbers (intfp) are only 2 Byte!
// short values are usually one byte leftpadded by zero
//   int values are usually two byte leftpadded by zero
//  stdata[]:
//  8   short   version
//  10  short   revision
//  12  int     vendor_id
//  16  short   channels
//  18  short   samplesize
//  20  short   compression_id
//  22  short   packet_size (==0)
//  24  intfp   sample_rate
//     (26  short)  unknown (==0)
//    ---- qt3.0+ (version>=1)
//  28  int     samples_per_packet
//  32  int     bytes_per_packet
//  36  int     bytes_per_frame
//  40  int     bytes_per_sample
// there may be additional atoms following at 28 (version 0)
// or 44 (version 1), eg. esds atom of .MP4 files
// esds atom:
//      28  int     atom size (bytes of int size, int type and data)
//      32  char[4] atom type (fourc charater code -> esds)
//      36  char[]      atom data (len=size-8)

					sh->samplesize = char2short(trak->stdata, 18) / 8;
					sh->channels = char2short(trak->stdata, 16);
					/*MP_DPF("MOV: timescale: %d samplerate: %d durmap: %d (%d) -> %d (%d)\n",
					   trak->timescale, char2short(trak->stdata,24), trak->durmap[0].dur,
					   trak->durmap[0].num, trak->timescale/trak->durmap[0].dur,
					   char2short(trak->stdata,24)/trak->durmap[0].dur); */
					sh->samplerate = char2short(trak->stdata, 24);
					if ((sh->samplerate < 7000) && trak->durmap)
					{
						switch (char2short(trak->stdata, 24) / trak->durmap[0].dur)
						{
							// TODO: add more cases.
						case 31:
							sh->samplerate = 32000;
							break;
						case 43:
							sh->samplerate = 44100;
							break;
						case 47:
							sh->samplerate = 48000;
							break;
						default:
							sh->samplerate = 44100;
						}
					}

					if (trak->stdata_len >= 44 && trak->stdata[9] >= 1)
					{
						if (trak->stdata_len >= 44 + 8)
						{
							int len = char2int(trak->stdata, 44);
							int fcc = char2int(trak->stdata, 48);

							// we have extra audio headers!!!
							//MP_DPF("Audio extra header: len=%d  fcc=0x%X\n",len,fcc);
							sh->codecdata_len = len - 8;
							sh->codecdata = trak->stdata + 44 + 8;
						}
					}

					if ((trak->stdata[9] == 0 || trak->stdata[9] == 1)
					        && trak->stdata_len >= 36)
					{	// version 0 with extra atoms
						int adjust = (trak->stdata[9] == 1) ? 48 : 0;
						int atom_len = char2int(trak->stdata, 28 + adjust);

						switch (char2int(trak->stdata, 32 + adjust))
						{	// atom type
						case MOV_FOURCC('e', 's', 'd', 's'):
						{
							if (atom_len > 8)
							{
								esds_t esds;

								if (!mp4_parse_esds
								        (&trak->stdata[36 + adjust], atom_len - 8, &esds))
								{

									sh->i_bps = esds.avgBitrate / 8;

//              MP_DPF("######## audio format = %d ########\n",esds.objectTypeId);
									if (esds.objectTypeId == 107)
										sh->format = 0x55;	// .mp3

									// dump away the codec specific configuration for the AAC decoder
									if (esds.decoderConfigLen)
									{
										sh->codecdata_len = esds.decoderConfigLen;
										sh->codecdata =
										    (unsigned char *) mem_malloc(sh->
										                                 codecdata_len);
										//CHK_MALLOC(sh->codecdata, "lschunks failed");
										memcpy(sh->codecdata, esds.decoderConfig,
										       sh->codecdata_len);
									}
								}
								mp4_free_esds(&esds);	// freeup esds mem

							}
						}
						break;
						default:
							break;
						}
					}

					// Emulate WAVEFORMATEX struct:
					sh->wf = (WAVEFORMATEX *) mem_malloc(sizeof(WAVEFORMATEX));
					//CHK_MALLOC(sh->wf, "lschunks failed");
					memset(sh->wf, 0, sizeof(WAVEFORMATEX));
					sh->wf->nChannels = (trak->stdata[16] << 8) + trak->stdata[17];
					sh->wf->wBitsPerSample = (trak->stdata[18] << 8) + trak->stdata[19];
					// sh->wf->nSamplesPerSec=trak->timescale;
					sh->wf->nSamplesPerSec = (trak->stdata[24] << 8) + trak->stdata[25];
					if (trak->stdata_len >= 44 && trak->stdata[9] >= 1
					        && char2int(trak->stdata, 28) > 0)
					{
						//Audio header: samp/pack=4096 bytes/pack=743 bytes/frame=1486 bytes/samp=2
						sh->wf->nAvgBytesPerSec =
						    (sh->wf->nChannels * sh->wf->nSamplesPerSec *
						     char2int(trak->stdata, 32) + char2int(trak->stdata,
						                                           28) / 2) /
						    char2int(trak->stdata, 28);
						sh->wf->nBlockAlign = char2int(trak->stdata, 36);
					}
					else
					{
						sh->wf->nAvgBytesPerSec =
						    sh->wf->nChannels * sh->wf->wBitsPerSample *
						    sh->wf->nSamplesPerSec / 8;
						// workaround for ms11 ima4
						if (sh->format == 0x1100736d && trak->stdata_len >= 36)
							sh->wf->nBlockAlign = char2int(trak->stdata, 36);
					}
					// Selection:
//      if(demux->audio->id==-1 || demux->audio->id==priv->track_db){
//          // (auto)selected audio track:
//          demux->audio->id=priv->track_db;
//          demux->audio->sh=sh; sh->ds=demux->audio;
//      }
					break;
				}
				case MOV_TRAK_VIDEO:
				{
					int i, entry;
					int flag, start, count_flag, end, palette_count, gray;
					int hdr_ptr = 76;	// the byte just after depth
					unsigned char *palette_map;
					sh_video_t *sh = new_sh_video(demux, priv->track_db);
					int depth = trak->stdata[75] | (trak->stdata[74] << 8);

					sh->format = trak->fourcc;

//  stdata[]:
//  8   short   version
//  10  short   revision
//  12  int     vendor_id
//  16  int     temporal_quality
//  20  int     spatial_quality
//  24  short   width
//  26  short   height
//  28  int     h_dpi
//  32  int     v_dpi
//  36  int     0
//  40  short   frames_per_sample
//  42  char[4] compressor_name
//  74  short   depth
//  76  short   color_table_id
// additional atoms may follow,
// eg esds atom from .MP4 files
//      78  int     atom size
//      82  char[4] atom type
//  86  ...     atom data

					{
						ImageDescription *id =
						    (ImageDescription *) mem_malloc(8 + trak->stdata_len);
						//CHK_MALLOC(id, "lschunks failed");
						trak->desc = id;
						id->idSize = 8 + trak->stdata_len;
//      id->cType=bswap_32(trak->fourcc);
						id->cType = le2me_32(trak->fourcc);
						id->version = char2short(trak->stdata, 8);
						id->revisionLevel = char2short(trak->stdata, 10);
						id->vendor = char2int(trak->stdata, 12);
						id->temporalQuality = char2int(trak->stdata, 16);
						id->spatialQuality = char2int(trak->stdata, 20);
						id->width = char2short(trak->stdata, 24);
						id->height = char2short(trak->stdata, 26);
						id->hRes = char2int(trak->stdata, 28);
						id->vRes = char2int(trak->stdata, 32);
						id->dataSize = char2int(trak->stdata, 36);
						id->frameCount = char2short(trak->stdata, 40);
						memcpy(&id->name, trak->stdata + 42, 32);
						id->depth = char2short(trak->stdata, 74);
						id->clutID = char2short(trak->stdata, 76);
						if (trak->stdata_len > 78)
							memcpy(((char *) &id->clutID) + 2, trak->stdata + 78,
							       trak->stdata_len - 78);
						sh->ImageDesc = id;

					}

					if (trak->stdata_len >= 86)
					{	// extra atoms found
						int pos = 78;
						int atom_len;

						while (pos + 8 <= trak->stdata_len &&
						        (pos + (atom_len = char2int(trak->stdata, pos))) <=
						        trak->stdata_len)
						{
							switch (char2int(trak->stdata, pos + 4))
							{	// switch atom type
							case MOV_FOURCC('g', 'a', 'm', 'a'):
								// intfp with gamma value at which movie was captured
								// can be used to gamma correct movie display
								break;
							case MOV_FOURCC('f', 'i', 'e', 'l'):
								// 2 char-values (8bit int) that specify field handling
								// see the Apple's QuickTime Fileformat PDF for more info
								break;
							case MOV_FOURCC('m', 'j', 'q', 't'):
								// Motion-JPEG default quantization table
								break;
							case MOV_FOURCC('m', 'j', 'h', 't'):
								// Motion-JPEG default huffman table
								break;
							case MOV_FOURCC('e', 's', 'd', 's'):
								// MPEG4 Elementary Stream Descriptor header
								// add code here to save esds header of length atom_len-8
								// beginning at stdata[86] to some variable to pass it
								// on to the decoder ::atmos
								if (atom_len > 8)
								{
									esds_t esds;

									if (!mp4_parse_esds
									        (trak->stdata + pos + 8, atom_len - 8, &esds))
									{

										// dump away the codec specific configuration for the AAC decoder
										trak->stream_header_len = esds.decoderConfigLen;
										trak->stream_header =
										    (unsigned char *) mem_malloc(trak->
										                                 stream_header_len);
										//CHK_MALLOC(trak->stream_header, "lschunks failed");
										memcpy(trak->stream_header, esds.decoderConfig,
										       trak->stream_header_len);
									}
									mp4_free_esds(&esds);	// freeup esds mem
								}
								break;
							case 0:
								break;
							default:
								break;
							}
							if (atom_len < 8)
								break;
							pos += atom_len;
//         MP_DPF("pos=%d max=%d\n",pos,trak->stdata_len);
						}
					}
					sh->fps = trak->timescale /
					          ((trak->durmap_size >= 1) ? (float) trak->durmap[0].dur : 1);
					sh->frametime = 1.0f / sh->fps;

					sh->disp_w = trak->stdata[25] | (trak->stdata[24] << 8);
					sh->disp_h = trak->stdata[27] | (trak->stdata[26] << 8);
					// if image size is zero, fallback to display size
					if (!sh->disp_w && !sh->disp_h)
					{
						sh->disp_w = trak->tkdata[77] | (trak->tkdata[76] << 8);
						sh->disp_h = trak->tkdata[81] | (trak->tkdata[80] << 8);
					}
					else if (sh->disp_w != (trak->tkdata[77] | (trak->tkdata[76] << 8)))
					{
						// codec and display width differ... use display one for aspect
						sh->aspect = trak->tkdata[77] | (trak->tkdata[76] << 8);
						sh->aspect /= trak->tkdata[81] | (trak->tkdata[80] << 8);
					}

					//if(depth>32+8)    MP_DPF("*** depth = 0x%X\n",depth);

					// palettized?
					gray = 0;
					if (depth > 32)
					{
						depth &= 31;
						gray = 1;
					}	// depth > 32 means grayscale
					if ((depth == 2) || (depth == 4) || (depth == 8))
						palette_count = (1 << depth);
					else
						palette_count = 0;

					// emulate BITMAPINFOHEADER:
					if (palette_count)
					{
						sh->bih =
						    (BITMAPINFOHEADER *) mem_malloc(sizeof(BITMAPINFOHEADER)
						                                    + palette_count * 4);
						//CHK_MALLOC(sh->bih, "lschunks failed");
						memset(sh->bih, 0, sizeof(BITMAPINFOHEADER) + palette_count * 4);
						sh->bih->biSize = 40 + palette_count * 4;
						// fetch the relevant fields
						flag = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						start = BE_32(&trak->stdata[hdr_ptr]);
						hdr_ptr += 4;
						count_flag = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						end = BE_16(&trak->stdata[hdr_ptr]);
						hdr_ptr += 2;
						palette_map = (unsigned char *) sh->bih + 40;

						/* XXX: problems with sample (statunit6.mov) with flag&0x4 set! - alex */

						// load default palette
						if (flag & 0x08)
						{
							if (gray)
							{
								if (palette_count == 16)
									memcpy(palette_map, qt_default_grayscale_palette_16,
									       16 * 4);
								else if (palette_count == 256)
									memcpy(palette_map, qt_default_grayscale_palette_256,
									       256 * 4);
							}
							else
							{
								if (palette_count == 4)
									memcpy(palette_map, qt_default_palette_4, 4 * 4);
								else if (palette_count == 16)
									memcpy(palette_map, qt_default_palette_16, 16 * 4);
								else if (palette_count == 256)
									memcpy(palette_map, qt_default_palette_256, 256 * 4);
							}
						}
						// load palette from file
						else
						{
							for (i = start; i <= end; i++)
							{
								entry = BE_16(&trak->stdata[hdr_ptr]);
								hdr_ptr += 2;
								// apparently, if count_flag is set, entry is same as i
								if (count_flag & 0x8000)
									entry = i;
								// only care about top 8 bits of 16-bit R, G, or B value
								if (entry <= palette_count && entry >= 0)
								{
									palette_map[entry * 4 + 2] = trak->stdata[hdr_ptr + 0];
									palette_map[entry * 4 + 1] = trak->stdata[hdr_ptr + 2];
									palette_map[entry * 4 + 0] = trak->stdata[hdr_ptr + 4];
								}
								else
									hdr_ptr += 6;
							}
						}
					}
					else
					{
						sh->bih = (BITMAPINFOHEADER *) mem_malloc(sizeof(BITMAPINFOHEADER));
						//CHK_MALLOC(sh->bih, "lschunks failed");
						memset(sh->bih, 0, sizeof(BITMAPINFOHEADER));
						sh->bih->biSize = 40;
					}
					sh->bih->biWidth = sh->disp_w;
					sh->bih->biHeight = sh->disp_h;
					sh->bih->biPlanes = 0;
					sh->bih->biBitCount = depth;
					sh->bih->biCompression = trak->fourcc;
					sh->bih->biSizeImage = sh->bih->biWidth * sh->bih->biHeight;

					if (trak->tkdata_len > 81)
					{
					}
					break;
				}
				case MOV_TRAK_GENERIC:
					break;
				default:
					break;
				}
				priv->track_db++;
				trak = NULL;
				break;
			}
#ifndef HAVE_ZLIB
			case MOV_FOURCC('c', 'm', 'o', 'v'):
			{
				return;
			}
#else
case MOV_FOURCC('m', 'o', 'o', 'v'):
case MOV_FOURCC('c', 'm', 'o', 'v'):
{
lschunks(level + 1, pos + len, NULL);
break;
}
case MOV_FOURCC('d', 'c', 'o', 'm'):
{
//      int temp=stream_read_dword(demux->stream);
unsigned int algo = be2me_32(stream_read_dword(demux->stream));

break;
}
case MOV_FOURCC('c', 'm', 'v', 'd'):
{
//      int temp=stream_read_dword(demux->stream);
unsigned int moov_sz = stream_read_dword(demux->stream);
unsigned int cmov_sz = len - 4;
unsigned char *cmov_buf = mem_malloc(cmov_sz);
unsigned char *moov_buf = mem_malloc(moov_sz + 16);
int zret;
z_stream zstrm;
stream_t *backup;

//CHK_MALLOC(cmov_buf, "lschunks failed");
//CHK_MALLOC(moov_buf, "lschunks failed");

stream_read(demux->stream, cmov_buf, cmov_sz);

zstrm.zalloc = (alloc_func) 0;
zstrm.zfree = (free_func) 0;
zstrm.opaque = (voidpf) 0;
zstrm.next_in = cmov_buf;
zstrm.avail_in = cmov_sz;
zstrm.next_out = moov_buf;
zstrm.avail_out = moov_sz;

zret = inflateInit(&zstrm);
if (zret != Z_OK)
{
	return;
}
zret = inflate(&zstrm, Z_NO_FLUSH);
if ((zret != Z_OK) && (zret != Z_STREAM_END))
{
	return;
}

if (moov_sz != zstrm.total_out)
{
}
zret = inflateEnd(&zstrm);

backup = demux->stream;
demux->stream = new_memory_stream(moov_buf, moov_sz);
stream_skip(demux->stream, 8);
lschunks(level + 1, moov_sz, NULL);	// parse uncompr. 'moov'
//free_stream(demux->stream);
demux->stream = backup;
mem_free(cmov_buf);
mem_free(moov_buf);
break;
}
#endif
			case MOV_FOURCC('u', 'd', 't', 'a'):
			{
				unsigned int udta_id;
				unsigned int udta_len;
				unsigned int udta_size = len;

				while ((len > 8) && (udta_size > 8))
				{
					udta_len = stream_read_dword(demux->stream);
					udta_id = stream_read_dword(demux->stream);
					udta_size -= 8;
					switch (udta_id)
					{
					case MOV_FOURCC(0xa9, 'c', 'p', 'y'):
					case MOV_FOURCC(0xa9, 'd', 'a', 'y'):
					case MOV_FOURCC(0xa9, 'd', 'i', 'r'):
						/* 0xa9,'e','d','1' - '9' : edit timestamps */
					case MOV_FOURCC(0xa9, 'f', 'm', 't'):
					case MOV_FOURCC(0xa9, 'i', 'n', 'f'):
					case MOV_FOURCC(0xa9, 'p', 'r', 'd'):
					case MOV_FOURCC(0xa9, 'p', 'r', 'f'):
					case MOV_FOURCC(0xa9, 'r', 'e', 'q'):
					case MOV_FOURCC(0xa9, 's', 'r', 'c'):
					case MOV_FOURCC('n', 'a', 'm', 'e'):
					case MOV_FOURCC(0xa9, 'n', 'a', 'm'):
					case MOV_FOURCC(0xa9, 'A', 'R', 'T'):
					case MOV_FOURCC(0xa9, 'c', 'm', 't'):
					case MOV_FOURCC(0xa9, 'a', 'u', 't'):
					case MOV_FOURCC(0xa9, 's', 'w', 'r'):
					{
						unsigned int text_len = stream_read_word(demux->stream);

						//char text[text_len+2+1];
						char *text = (char *) mem_malloc(text_len + 2 + 1);

						//CHK_MALLOC(text, "lschunks failed");
						stream_read(demux->stream, text, text_len + 2);
						text[text_len + 2] = 0x0;
						switch (udta_id)
						{
						case MOV_FOURCC(0xa9, 'a', 'u', 't'):
							/*get author jackyang 20050607 */
							if (MAX_AUTHOR_LEN > text_len)
							{
								memcpy(movcontent.szAuthor, &text[2], text_len);
							}
							/*add end jackyang 20050607 */
							break;
						case MOV_FOURCC(0xa9, 'c', 'p', 'y'):
							/*get copyright, jackyang 20050607 */
							if (MAX_COPYRIGHT_LEN > text_len)
							{
								memcpy(movcontent.szCopyright, &text[2], text_len);
							}
							/*add end jackyang 20050607 */
							break;
						case MOV_FOURCC(0xa9, 'i', 'n', 'f'):
							break;
						case MOV_FOURCC('n', 'a', 'm', 'e'):
						case MOV_FOURCC(0xa9, 'n', 'a', 'm'):
							/*get file name, jackyang 20050607 */
							if (MAX_TITLE_LEN > text_len)
							{
								memcpy(movcontent.szName, &text[2], text_len);
							}
							/*add end jackyang 20050607 */
							break;
						case MOV_FOURCC(0xa9, 'A', 'R', 'T'):
							break;
						case MOV_FOURCC(0xa9, 'd', 'i', 'r'):
							break;
						case MOV_FOURCC(0xa9, 'c', 'm', 't'):
							/*get comment, jackyang 20050607 */
							if (MAX_DESCRIP_LEN > text_len);
							{
								memcpy(movcontent.szComment, &text[2], text_len);
							}
							/*add end jackyang 20050607 */
							break;
						case MOV_FOURCC(0xa9, 'r', 'e', 'q'):
							break;
						case MOV_FOURCC(0xa9, 's', 'w', 'r'):
							break;
						case MOV_FOURCC(0xa9, 'd', 'a', 'y'):
							break;
						case MOV_FOURCC(0xa9, 'f', 'm', 't'):
							break;
						case MOV_FOURCC(0xa9, 'p', 'r', 'd'):
							break;
						case MOV_FOURCC(0xa9, 'p', 'r', 'f'):
							break;
						case MOV_FOURCC(0xa9, 's', 'r', 'c'):
							break;
						}
						udta_size -= 4 + text_len;

						mem_free(text);
						break;
					}
					/* some other shits:    WLOC - window location,
					   LOOP - looping style,
					   SelO - play only selected frames
					   AllF - play all frames
					 */
					case MOV_FOURCC('W', 'L', 'O', 'C'):
					case MOV_FOURCC('L', 'O', 'O', 'P'):
					case MOV_FOURCC('S', 'e', 'l', 'O'):
					case MOV_FOURCC('A', 'l', 'l', 'F'):
					default:
					{
						if (udta_len > udta_size)
							udta_len = udta_size;
						{
							//char dump[udta_len-4];
							char *dump = (char *) mem_malloc(udta_len - 4);

							//CHK_MALLOC(dump, "lschunks failed");
							stream_read(demux->stream, dump, udta_len - 4 - 4);
							udta_size -= udta_len;
							mem_free(dump);
						}
					}
					}
				}
				break;
			}				/* eof udta */
			default:
				id = be2me_32(id);
			}					/* endof switch */
		}						/* endof else */

		pos += len + 8;
		if (pos >= endpos)
			break;
		if (!stream_seek(demux->stream, pos))
			break;
	}
}
#endif

static float mov_seek_track(mov_track_t * trak, float pts, int flags)
{

//    MP_DPF("MOV track seek called  %5.3f  \n",pts);
	//sh_video_t *sh_video=demux->video->sh;

	if (flags & 2)
		pts *= trak->length;
	else
		pts *= (float) trak->timescale;

	if (trak->samplesize)
	{
		// int sample=pts/trak->duration;
		int sample = (pts / (float) trak->duration) + 0.5;

//    MP_DPF("MOV track seek - chunk: %d  (pts: %5.3f  dur=%d)  \n",sample,pts,trak->duration);
		if (!(flags & 1))
			sample += trak->chunks[trak->pos].sample;	// relative
		trak->pos = 0;
		while (trak->pos < trak->chunks_size && trak->chunks[trak->pos].sample < sample)
			++trak->pos;
		pts = (float) (trak->chunks[trak->pos].sample * trak->duration) / (float) trak->timescale;

		//added for progress bar & seek
		////MP_DEBUG1("MOV:SEEK1:sh_video->num_frames=", sh_video->num_frames, 8);
		//sh_video->num_frames=trak->chunks[trak->pos].sample;
	}
	else
	{
		unsigned long long ipts;
		float ori_pts = 0;

		if (!(flags & 1))
			pts += trak->samples[trak->pos].pts;
		if (pts < 0)
			pts = 0;

		ori_pts = trak->samples[trak->pos].pts;
		if (ori_pts < 0)
			ori_pts = 0;

		//ipts=pts;

		ipts = pts + 0.5;
		//MP_DEBUG1("!!!!!ipts=", ipts, 8);

		//MP_DPF("MOV track seek - sample: %d  \n",ipts);
		for (trak->pos = 0; trak->pos < trak->samples_size; ++trak->pos)
		{
			if (trak->samples[trak->pos].pts >= ipts)
				break;			// found it!
		}
		if (trak->keyframes_size)
		{
			// find nearest keyframe
			int i;

			for (i = 0; i < trak->keyframes_size; i++)
			{
				if (trak->keyframes[i] >= trak->pos)
					break;
			}

			if (ori_pts < trak->samples[trak->keyframes[i]].pts)
			{
				if (ori_pts < trak->samples[trak->keyframes[i-1]].pts)
				{
					if (forward_flag)
					{
						if (trak->samples[trak->keyframes[i]].pts - trak->samples[trak->pos].pts
						        > trak->samples[trak->pos].pts - trak->samples[trak->keyframes[i-1]].pts)
							trak->pos = trak->keyframes[i-1];
						else
							trak->pos = trak->keyframes[i];
					}
					else if (backward_flag && i >= 1)
						trak->pos = trak->keyframes[i-1];
					else
						trak->pos = trak->keyframes[i];
				}
				else
				{
					if (forward_flag)
						trak->pos = trak->keyframes[i];
					else if (backward_flag && i >= 1)
						trak->pos = trak->keyframes[i-1];
					else
						trak->pos = trak->keyframes[i];
				}
			}
			else
			{
				if (forward_flag)
					trak->pos = trak->samples_size;
				else if (backward_flag && i >= 1)
					trak->pos = trak->keyframes[i-1];
				else
					trak->pos = trak->keyframes[i];

			}
//  MP_DPF("nearest keyframe: %d  \n",trak->pos);

		}

		//MP_DEBUG1("!!!!!trak->pos=", trak->pos, 8);

		pts = (float) trak->samples[trak->pos].pts / (float) trak->timescale;


		//added for progress bar & seek
		////MP_DEBUG1("MOV:SEEK2:sh_video->num_frames=", sh_video->num_frames, 8);
		//sh_video->num_frames=trak->pos;
	}

//    MP_DPF("MOV track seek done:  %5.3f  \n",pts);

	return pts;
}

static int mov_check_file()
{

	int local_dev = 1;
	int flags = 0;
	int no = 0;
	BOOL moov_parsed = FALSE;
	mov_priv_t *priv = (mov_priv_t *) mem_malloc(sizeof(mov_priv_t));

	memset(priv, 0, sizeof(mov_priv_t));

	while (1)
	{
		int i;
		int skipped = 8;
		unsigned int len = stream_read_dword(demux->stream);
		unsigned int id = stream_read_dword(demux->stream);

		if (stream_eof(demux->stream))
			break;				// EOF
		if (len == 1)			/* real size is 64bits - cjb */
		{
#ifndef _LARGEFILE_SOURCE
			if (stream_read_dword(demux->stream) != 0)
				len = stream_read_dword(demux->stream);
#else
			len = stream_read_qword(demux->stream);
#endif
			skipped += 8;
		}
		else if (len < 8)
			break;				// invalid chunk

		switch (id)   				//check 3 tag only,ftyp,moov,mdat
		{
		case MOV_FOURCC('f', 't', 'y', 'p'):
		{
			unsigned int tmp;

			// File Type Box (ftyp):
			// char[4]  major_brand      (eg. 'isom')
			// int      minor_version    (eg. 0x00000000)
			// char[4]  compatible_brands[]  (eg. 'mp41')
			// compatible_brands list spans to the end of box
			tmp = stream_read_dword(demux->stream);
			switch (tmp)
			{
			case MOV_FOURCC('i', 's', 'o', 'm'):
				break;
			default:
				tmp = be2me_32(tmp);
			}
//Jashua,Brenda debug
			stream_read_dword(demux->stream);
			skipped += 8;
			// List all compatible brands
			for (i = 0; i < ((len - 16) / 4); i++)
			{
				tmp = be2me_32(stream_read_dword(demux->stream));
				skipped += 4;
			}
		}
		break;
		case MOV_FOURCC('m', 'o', 'o', 'v'):

//  case MOV_FOURCC('c','m','o','v'):
			moov_parsed = TRUE;
			priv->moov_start = (unsigned int) stream_tell(demux->stream);
			priv->moov_end = (unsigned int) priv->moov_start + len - skipped;
			skipped += 8;
			i = stream_read_dword(demux->stream) - 8;
			if (stream_read_dword(demux->stream) == MOV_FOURCC('r', 'm', 'r', 'a'))
			{
				//int ref=0;
				skipped += i;
				//set demux type to playlist ...
				//demux->type=DEMUXER_TYPE_PLAYLIST;
				while (i > 0)
				{
					int len = stream_read_dword(demux->stream) - 8;
					int fcc = stream_read_dword(demux->stream);

					if (len < 0)
						break;	// EOF!?
					i -= 8;
//        MP_DPF("i=%d  len=%d\n",i,len);
					switch (fcc)
					{
					case MOV_FOURCC('r', 'm', 'd', 'a'):
						continue;
					case MOV_FOURCC('r', 'd', 'r', 'f'):
					{
						int tmp = stream_read_dword(demux->stream);
						int type = stream_read_dword_le(demux->stream);
						int slen = stream_read_dword(demux->stream);

						//char* s=malloc(slen+1);
						//stream_read(demux->stream,s,slen);

						//FIXME: also store type & data_rate ?
						ds_read_packet(demux->video, demux->stream, slen, 0, stream_tell(demux->stream), 0	// no flags
						              );
						flags |= 4;
						len -= 12 + slen;
						i -= 12 + slen;
						break;
					}
					case MOV_FOURCC('r', 'm', 'd', 'r'):
					{
						int flags = stream_read_dword(demux->stream);
						int rate = stream_read_dword(demux->stream);

						len -= 8;
						i -= 8;
						break;
					}
					case MOV_FOURCC('r', 'm', 'q', 'u'):
					{
						int q = stream_read_dword(demux->stream);

						len -= 4;
						i -= 4;
						break;
					}
					}
					i -= len;
					stream_skip(demux->stream, len);
				}
			}
			flags |= 1;
			break;
		case MOV_FOURCC('w', 'i', 'd', 'e'):
			if (flags & 2)
				break;
		case MOV_FOURCC('m', 'd', 'a', 't'):
			network_mdat++;
			priv->mdat_start = stream_tell(demux->stream);
			priv->mdat_end = priv->mdat_start + len - skipped;
			flags |= 2;
			if (flags == 3)
			{
				// if we're over the headers, then we can stop parsing here!
				demux->priv = priv;
//      return 1;
			}
			break;
		case MOV_FOURCC('f', 'r', 'e', 'e'):
		case MOV_FOURCC('s', 'k', 'i', 'p'):
		case MOV_FOURCC('j', 'u', 'n', 'k'):
			/* unused, if you edit a mov, you can use space provided by free atoms (redefining it) */
			break;
		case MOV_FOURCC('p', 'n', 'o', 't'):
		case MOV_FOURCC('P', 'I', 'C', 'T'):
			/* dunno what, but we shoudl ignore it */
			break;
		default:
			if (no == 0)
			{
				mem_free(priv);
				return 0;
			}					// first chunk is bad!
			id = be2me_32(id);
		}
skip_chunk:
		if (DEV_USB_WIFI_DEVICE == demux->stream->fd->Drv->DevID||DEV_CF_ETHERNET_DEVICE == demux->stream->fd->Drv->DevID)
		{
			local_dev = 0;
		}

		if (local_dev || !moov_parsed)
		{
			if (!stream_skip(demux->stream, len - skipped))
				break;
		}
		else
		{
			demux->priv = priv;
			return 1;
		}
		++no;
	}

	if (flags == 3)
	{
		demux->priv = priv;
		return 1;
	}
	mem_free(priv);

	if ((flags == 5) || (flags == 7))	// reference & header sent
		return 1;

	if (flags == 1)
	{
	}
	else if (flags == 2)
	{
	}

	return 0;
}

int fllll=0;
// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet and add a packet to demux_stream_t  ds
//	       packet can be either chunk or single sample depend on if variant size
static int _fill_buffer(demux_stream_t * ds)
{
	mov_priv_t *priv = demux->priv;
	mov_track_t *trak = NULL;
	float pts;
	int x,x1;
	int last_x;
	unsigned int pos;
	sh_video_t *sh_video = (sh_video_t *) filter_graph.demux->sh_video;
	sh_audio_t *sh_audio = (sh_audio_t *) filter_graph.demux->sh_audio;
	unsigned int four_cc = sh_video->format;
	if (ds->id < 0 || ds->id >= priv->track_db)
		return 0;
	trak = priv->tracks[ds->id];
	if (trak->samplesize)
	{
		// read chunk:
		if (trak->pos >= trak->chunks_size)
			return 0;			// EOF
//		stream_seek(demux->stream, trak->chunks[trak->pos].pos);

		if ((trak->type == MOV_TRAK_AUDIO) && (demux->stream_a != NULL))
		{
			stream_seek(demux->stream_a, trak->chunks[trak->pos].pos);

		}
		else
		{
			stream_seek(demux->stream, trak->chunks[trak->pos].pos);
		}
		pts = (float) (trak->chunks[trak->pos].sample * trak->duration) / (float) trak->timescale;
		if (trak->samplesize != 1)
			//if (trak->samplebytes != 1)
		{		
			x = trak->chunks[trak->pos].size * trak->samplesize;
		}
		else
		{
			x = trak->chunks[trak->pos].size;
		}
//    MP_DPF("X = %d\n", x);
		/* the following stuff is audio related */
		if (trak->type == MOV_TRAK_AUDIO)
		{
			if (trak->stdata_len >= 44 && trak->stdata[9] >= 1 && char2int(trak->stdata, 28) > 0)
			{
				// stsd version 1 - we have audio compression ratio info:
				x /= char2int(trak->stdata, 28);	// samples/packet
				//  x*=char2int(trak->stdata,32); // bytes/packet

				x *= char2int(trak->stdata, 36);	// bytes/frame
			}
			else
			{
				if (ds->ss_div && ds->ss_mul)
				{
					// workaround for buggy files like 7up-high-traffic-areas.mov,
					// with missing stsd v1 header containing compression rate
					x /= ds->ss_div;
					x *= ds->ss_mul;	// compression ratio fix  ! HACK !
				}

				//0818 for h.263+uLaw files	else
				//0818 for h.263+uLaw files	{
				//x*=(trak->stdata[16]<<8)+trak->stdata[17]; //channels
				//x*=(trak->stdata[18]<<8)+trak->stdata[19]; //bits/sample
				//x/=8;  // bits/sample
				//0818 for h.263+uLaw files		x *= trak->nchannels;
				//0818 for h.263+uLaw files		x *= trak->samplebytes;
				//0818 for h.263+uLaw files	}
			}
			//Fix the issue that chunk length does not match.2009/12/29 by xianwen

			if (sh_audio->format == PCM_TWOS||sh_audio->format == PCM_SOWT)
			{

				if (x !=( trak->chunks[trak->pos].size * trak->samplesize*trak->nchannels))
				{
					x = trak->chunks[trak->pos].size * trak->samplesize*trak->nchannels;
				}

			}
			
			if (sh_audio->format == ulaw ||sh_audio->format ==ULAW ||sh_audio->format == 0x7)
			{
				x /= trak->samplesize;
			}
			
		}						/* MOV_TRAK_AUDIO */

		pos = trak->chunks[trak->pos].pos;

	}
	else
	{
		int frame = trak->pos;
		if (frame >= trak->samples_size)
			return 0;		// EOF

		// editlist support:
		if (trak->type == MOV_TRAK_VIDEO && trak->editlist_size >= 1)
		{
			// find the right editlist entry:
			if (frame < trak->editlist[trak->editlist_pos].start_frame)
				trak->editlist_pos = 0;
			while (trak->editlist_pos < trak->editlist_size - 1 &&
			        frame >= trak->editlist[trak->editlist_pos + 1].start_frame)
				++trak->editlist_pos;
			if (frame >= trak->editlist[trak->editlist_pos].start_frame +
			        trak->editlist[trak->editlist_pos].frames)
				return 0;		// EOF
			// calc real frame index:
			frame -= trak->editlist[trak->editlist_pos].start_frame;
			frame += trak->editlist[trak->editlist_pos].start_sample;
			// calc pts:
			pts = (float) (trak->samples[frame].pts +
			               trak->editlist[trak->editlist_pos].pts_offset) / (float) trak->timescale;
		}
		else
		{
			pts = (float) trak->samples[frame].pts / (float) trak->timescale;
			//mpDebugPrint("pts=%d__samples[%d].pts=%d__timescale=%d",FLOAT1000(pts),frame,trak->samples[frame].pts,trak->timescale);

		}

		// read sample:
		if ((trak->type == MOV_TRAK_AUDIO) && (demux->stream_a != NULL))
			stream_seek(demux->stream_a, trak->samples[frame].pos);
		else
			stream_seek(demux->stream, trak->samples[frame].pos);
		x = trak->samples[frame].size;
		pos = trak->samples[frame].pos;

		last_x = x;

		if (x == 0)
		{
			if ((JPEG == four_cc) ||
			        (jpeg == four_cc) ||
			        (MJPG == four_cc) ||
			        (mjpg == four_cc)
			   )
			{
				//mpDebugPrint("trak->samples_size = %d", trak->samples_size);
				// for sample size =0 of olympus fe 170.mov
				if (ds != demux->video)
				{
					return 0;
				}
				else
				{
					int temp_frame_1 = frame;
					int temp_frame_2 = frame;
					static int last_frame = 0;
					mov_sample_t* p_samples;
					do
					{
						temp_frame_1--;
						p_samples = &trak->samples[temp_frame_1];
					}
					while (0 == p_samples->size);

					do
					{
						temp_frame_2++;
						p_samples = &trak->samples[temp_frame_2];
					}
					while (0 == p_samples->size);

					if ((((temp_frame_2-frame) >= (frame-temp_frame_1)) && (temp_frame_1 >= last_frame)) || (temp_frame_2 >= trak->samples_size))
					{
						last_frame = temp_frame_1;
					}
					else
					{
						last_frame = temp_frame_2;
					}
					p_samples = &trak->samples[last_frame];
					//mpDebugPrint("last_frame = %d", last_frame);
					stream_seek(demux->stream, p_samples->pos);
					x = p_samples->size;
					pos = p_samples->pos;
				}
			}
		}
		//mpDebugPrint("trak->samples[frame].size = %d", trak->samples[frame].size);
	}
	
	if (trak->pos == 0 && trak->stream_header_len > 0)
	{
		
		// we have to append the stream header...
		demux_packet_t *dp = (demux_packet_t *) new_demux_packet(x + trak->stream_header_len);

		memcpy(dp->buffer, trak->stream_header, trak->stream_header_len);
		stream_read(demux->stream, (char *) (dp->buffer + trak->stream_header_len), x);
		mem_free(trak->stream_header);
		trak->stream_header = NULL;
		trak->stream_header_len = 0;
		dp->pts = pts;
		dp->flags = 0;
		dp->pos = pos;			// FIXME?
		ds_add_packet(ds, dp);
	}
	else
	{

		if ((trak->type == MOV_TRAK_AUDIO) && (demux->stream_a != NULL))
		{
			if (ds_read_packet(ds, demux->stream_a, x, pts, pos, 0))
				return 0;
		}
		else
		{
			if (ds_read_packet(ds, demux->stream, x, pts, pos, 0))
				return 0;
		}
	}


	if ((JPEG == four_cc) ||
	        (jpeg == four_cc) ||
	        (MJPG == four_cc) ||
	        (mjpg == four_cc)
	   )
	{
		if ((0 == last_x) && (MOV_TRAK_VIDEO == trak->type))
		{
			stream_seek(demux->stream, trak->samples[trak->pos].pos);
		}
	}

	++trak->pos;
	//return 1;
	return ds ? 1 : 0;

}

static int _seek(float rel_seek_secs, int flags)
{
	float pts = rel_seek_secs;

	mov_priv_t *priv = demux->priv;
	demux_stream_t *ds;
	sh_video_t *sh_video;

//    MP_DPF("MOV seek called  %5.3f  flag=%d  \n",pts,flags);

	ds = demux->video;
	sh_video = demux->video->sh;

	{
		mov_track_t *trak = priv->tracks[ds->id];

		////MP_DEBUG1("!!!!!MOV:before SEEK:track->pos", trak->pos, 8);

	}

	if (ds && ds->id >= 0 && ds->id < priv->track_db)
	{
		mov_track_t *trak = priv->tracks[ds->id];

		//if(flags&2) pts*=(float)trak->length/(float)trak->timescale;
		//if(!(flags&1)) pts+=ds->pts;
		pts = ds->pts = mov_seek_track(trak, pts, flags);
		flags = 1;				// absolute seconds

		//added for progress bar & seek
		//MP_DEBUG1("!!!!!MOV:SEEK:sh_video->num_frames=", sh_video->num_frames, 8);
		if (trak->samplesize)
		{
			sh_video->num_frames = trak->chunks[trak->pos].sample;
			//MP_DEBUG1("!!!!!MOV:SEEK1:sh_video->num_frames=", sh_video->num_frames, 8);
		}
		else
		{
			sh_video->num_frames = trak->pos;
			//MP_DEBUG1("!!!!!MOV:SEEK2:sh_video->num_frames=", sh_video->num_frames, 8);
		}
	}
	ds = demux->audio;
	if (ds && ds->id >= 0 && ds->id < priv->track_db)
	{
		mov_track_t *trak = priv->tracks[ds->id];

		//if(flags&2) pts*=(float)trak->length/(float)trak->timescale;
		//if(!(flags&1)) pts+=ds->pts;
		ds->pts = mov_seek_track(trak, pts, flags);
	}

	{
		mov_track_t *trak = priv->tracks[demux->video->id];

		////MP_DEBUG1("!!!!!MOV:after SEEK:track->pos", trak->pos, 8);

	}
	return 1;
}

//static int _control(int cmd, void *arg){
//}

static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
	
	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_MOV))
	{network_mdat=0;
	 
		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		if (mov_check_file())
		{
			if (DEV_USB_WIFI_DEVICE == demux->stream->fd->Drv->DevID||DEV_CF_ETHERNET_DEVICE == demux->stream->fd->Drv->DevID)
		   {
			if(network_mdat!=0)return 0;
		   }
	
			return 1;
		}
		else
			return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
	}
	//MP_DEBUG("not mov\r\n");
	return 0;
}

static int _open()
{
	mov_priv_t *priv = demux->priv;
	int t_no;
	int best_a_id = -1, best_a_len = 0;
	int best_v_id = -1, best_v_len = 0;
	int video_stream_flag = 0;
	exception_type.type=0;
	// Parse header:
	//WaitMs(2);// For some reason,  gregxu & joshua & mengwang & brendali
	stream_reset(demux->stream);
	if (!stream_seek(demux->stream, priv->moov_start))
	{
		mpDebugPrint("Demux mov open - stream seeks start position fail...");
		return 0;
	}

	// WaitMs(2);// For some reason
	lschunks(0, priv->moov_end, NULL);
	// just in case we have hit eof while parsing...

	demux->stream->eof = 0;
	// find the best (longest) streams:

	for (t_no = 0; t_no < priv->track_db; t_no++)
	{
		mov_track_t *trak = priv->tracks[t_no];
		int len = (trak->samplesize) ? trak->chunks_size : trak->samples_size;
		
		if (demux->a_streams[t_no])
		{						// need audio
			if (len > best_a_len)
			{
				best_a_len = len;
				best_a_id = t_no;
			}
			if (priv->track_db >= 3)
			{
				best_a_len = len;
				best_a_id = t_no;				
			}
		}
		if (demux->v_streams[t_no])
		{						// need video
			if (len > best_v_len)
			{
				best_v_len = len;
				best_v_id = t_no;
			}
		}
	}
	if (demux->audio->id == -1 && best_a_id >= 0)
		demux->audio->id = best_a_id;
	if (demux->video->id == -1 && best_v_id >= 0)
		demux->video->id = best_v_id;

	// setup sh pointers:
	if (demux->audio->id >= 0)
	{
		sh_audio_t *sh = demux->a_streams[demux->audio->id];
		mpDebugPrint("demux->audio->id >= 0");
		if (sh)
		{
			demux->audio->sh = sh;
			sh->ds = demux->audio;
		}
		else
		{
			demux->audio->id = -2;
		}
	}
	if (demux->video->id >= 0)
	{
		sh_video_t *sh = demux->v_streams[demux->video->id];
		mpDebugPrint("demux->video->id >= 0");
		if (sh)
		{
			demux->video->sh = sh;
			sh->ds = demux->video;
			video_stream_flag = 1;
		}
		else
		{
			demux->video->id = -2;
		}
		video_stream_flag = 1;
	}
	
	if (video_stream_flag == 0)
		return 0;
	
	return 1;
}

/*!
@ingroup	MOV
@brief		Free all mov demuxer memory
@param		none
@note		Disabled by Shawn for some reasons. Try to enable it and see if it works
*/
static int _close()
{
	int i;
	mov_priv_t *priv = demux->priv;

	if (!priv)
		return 1;

	for (i = 0; i <= (MOV_MAX_TRACKS - 1); i++)
	{
		if (priv->tracks[i])
		{
			mov_track_t *cur_track = priv->tracks[i];

			if (cur_track->tkdata)
				mem_free(cur_track->tkdata);
			if (cur_track->stdata)
				mem_free(cur_track->stdata);
			if (cur_track->stream_header)
				mem_free(cur_track->stream_header);
			if (cur_track->samples)
				mem_free(cur_track->samples);
			if (cur_track->chunks)
				mem_free(cur_track->chunks);
			if (cur_track->chunkmap)
				mem_free(cur_track->chunkmap);
			if (cur_track->durmap)
				mem_free(cur_track->durmap);
			if (cur_track->keyframes)
				mem_free(cur_track->keyframes);
			if (cur_track->editlist)
				mem_free(cur_track->editlist);
			if (cur_track->desc)
				mem_free(cur_track->desc);

			mem_free(priv->tracks[i]);
		}
	}

	mem_free(priv);
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

	mov_priv_t *priv = demux->priv;
	demux_stream_t *ds;

	//work around for seek's 2 frame/jump.
	rel_seek_samples--;

	ds = demux->video;
	if (ds && ds->id >= 0 && ds->id < priv->track_db)
	{
		mov_track_t *trak = priv->tracks[ds->id];

		////map seek samples to pts

		switch (flags)
		{
		case 2:
		{
			int no;

			if (trak->samplesize)
			{
				if ((trak->pos + rel_seek_samples) < 0)
					no = 0;
				else if ((trak->pos + rel_seek_samples) > (trak->chunks_size - 1))
					no = trak->chunks_size - 1;
				//else no=trak->chunks_size-1;
				else
					no = trak->pos + rel_seek_samples;

				pts =
				    (float) trak->chunks[no].sample * trak->duration / (float) trak->timescale;
				//(trak->pos+rel_seek_samples)*trak->duration/(float)trak->timescale;
			}
			else
			{
				if ((trak->pos + rel_seek_samples) < 0)
					no = 0;
				else if ((trak->pos + rel_seek_samples) > (trak->samples_size - 1))
					no = trak->samples_size - 1;
				else
					no = trak->pos + rel_seek_samples;

				////MP_DEBUG1("!!!!!rel_seek_samples=",rel_seek_samples, 8);
				////MP_DEBUG1("!!!!!trak->pos=", trak->pos, 8);
				////MP_DEBUG1("!!!!!no=",no , 8);
				////MP_DEBUG1("!!!!!trak->samples[no].pts=", trak->samples[no].pts, 8);

				pts = (float) trak->samples[no].pts / (float) trak->timescale;
			}
			break;
		}
		case 0:
		{
			if (trak->samplesize)
				pts =
				    (float) (trak->chunks[0].sample * trak->duration) / (float) trak->timescale;
			//(trak->pos+rel_seek_samples)*trak->duration/(float)trak->timescale;
			else
			{
				//  //MP_DEBUG1("!!!!!trak->pos=", trak->pos, 8);
				//  //MP_DEBUG1("!!!!!trak->samples[0].pts=", trak->samples[0].pts, 8);

				pts = (float) trak->samples[0].pts / (float) trak->timescale;
			}
			break;
		}
		case 1:
		{
			if (trak->samplesize)
				pts =
				    (float) (trak->chunks[trak->chunks_size - 1].sample * trak->duration) /
				    (float) trak->timescale;
			//pts=trak->samples[trak->chunks[trak->chunks_size-1].sample].pts/(float)trak->timescale;
			//(trak->pos+rel_seek_samples)*trak->duration/(float)trak->timescale;
			else
			{
				// //MP_DEBUG1("!!!!!get_pts_by_relframe:TOEND: trak->samples_size-1=", trak->samples_size-1, 8);
				////MP_DEBUG1("!!!!!trak->pos=", trak->pos, 8);
				// //MP_DEBUG1("!!!!!trak->samples[0].pts=", trak->samples[0].pts, 8);

				pts =
				    (float) trak->samples[trak->samples_size - 1].pts / (float) trak->timescale;
			}
			break;
		}
		default:
			break;
		}
	}

	return pts;
	//MovieSeek(pts,1);

}

//command=0: get total time/file length
//command=1: get current time/file point
//command=2: get percentage
//command=3: get mov information
//return=0:  not fill the buffer
//return=1:  fill the buffer
static int _get_movie_info(BYTE * buffer, int command)
{
	register DWORD second, minute, hour;
	BYTE *string;
	BYTE digit;

	demux_stream_t *d_video;
	sh_video_t *sh_video;
	mov_priv_t *priv;

	priv = demux->priv;
	d_video = demux->video;
	sh_video = d_video->sh;

	if (d_video && d_video->id >= 0 && d_video->id < priv->track_db)
	{
		mov_track_t *trak = priv->tracks[d_video->id];

		switch (command)
		{
		case 0:
		{
			//second = ((float)priv->numberofframes)/ sh_video->fps;
			if (trak->samplesize)
			{
				//MP_DEBUG1("!!!!!trak->chunks[trak->chunks_size-1].sample=", trak->chunks[trak->chunks_size-1].sample, 8);
				second = (trak->chunks[trak->chunks_size - 1].sample) / sh_video->fps;
			}
			else
			{
				//MP_DEBUG1("!!!!! trak->samples_size-1=", trak->samples_size-1, 8);
				second = (trak->samples_size - 1) / sh_video->fps;
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

			//MP_DEBUG1("!!!!!MOV:  _get_movie_info:sh_video->num_frames=", sh_video->num_frames, 8);
			//MP_DEBUG1("!!!!!trak->pos=", trak->pos, 8);

			second = ((float) sh_video->num_frames - 1) / sh_video->fps;
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

			second =
			    ((float) sh_video->num_frames - 1) / sh_video->fps +
			    1.0 / (2.0 * sh_video->fps);
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
			
			if (trak->timescale)
				second = (float)trak->length / trak->timescale;

			if(exception_type.type==1)
			{
				second=exception_type.i1;
			}

			/* Meng 2005.6.7 Updated starts */
			PMedia_Info pMOVInfo = (PMedia_Info) buffer;
			sh_audio_t *sh_audio = (sh_audio_t *) demux->sh_audio;

			if (sh_audio)
			{
				GetAudioCodecCategory(pMOVInfo->cAudioCodec);

				pMOVInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_AUDIO;

				if ((pMOVInfo->dwSampleRate = sh_audio->samplerate) > 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_SampleRate_USEFUL;

				if ((pMOVInfo->dwSampleSize = sh_audio->samplesize) > 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_SampleSize_USEFUL;

				if ((pMOVInfo->dwBitrate = sh_audio->i_bps) > 0)
				{
					pMOVInfo->dwFlags |= (DWORD) MOVIE_Bitrate_USEFUL;

					if ((pMOVInfo->dwTotalTime = demux->stream->file_len / sh_audio->i_bps) >= 0)
						pMOVInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
				}
				else
					pMOVInfo->dwTotalTime = 0;
			}

			if (sh_video)
			{
				GetVideoCodecCategory(pMOVInfo->cVidoeCodec);

				pMOVInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_VIDEO;

				if ((pMOVInfo->dwFrameRate = sh_video->fps) > 0)
				{
					//UartOutValue(pMOVInfo->dwFrameRate,8);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_FrameRate_USEFUL;
				}
				if ((pMOVInfo->dwImageHeight = sh_video->disp_h) > 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_ImageHeight_USEFUL;

				if ((pMOVInfo->dwImageWidth = sh_video->disp_w) > 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_ImageWidth_USEFUL;

				if ((pMOVInfo->dwTotalFrame = second * sh_video->fps) > 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_TotalFrame_USEFUL;

				if ((pMOVInfo->dwTotalTime = second) >= 0)
					pMOVInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;


				if (strlen(movcontent.szAuthor) > 0)
				{
					memcpy(pMOVInfo->contentinfo.szAuthor, movcontent.szAuthor, MAX_AUTHOR_LEN);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
				}
				if (strlen(movcontent.szCopyright) > 0)
				{
					memcpy(pMOVInfo->contentinfo.szCopyright, movcontent.szCopyright,
					       MAX_COPYRIGHT_LEN);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_Copyright_USEFUL;
				}
				if (strlen(movcontent.szComment) > 0)
				{
					memcpy(pMOVInfo->contentinfo.szDescription, movcontent.szComment,
					       MAX_DESCRIP_LEN);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_Description_USEFUL;
				}
				if (strlen(movcontent.szRating) > 0)
				{
					memcpy(pMOVInfo->contentinfo.szRating, movcontent.szRating, MAX_RATING_LEN);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_Rating_USEFUL;
				}
				if (strlen(movcontent.szName) > 0)
				{
					memcpy(pMOVInfo->contentinfo.szTitle, movcontent.szName, MAX_TITLE_LEN);
					pMOVInfo->dwFlags |= (DWORD) MOVIE_Title_USEFUL;
				}
			}

			return 1;
			/* Meng 2005.6.7 Updated ends */
		}
		//add end jackyang 20050607
		default:
			break;
		}

	}
	return 0;
}

#if 0
static int _seek_video(float rel_seek_secs, int flags)
{
	float pts = rel_seek_secs;

	pts = 0.034;
	//flags=0;

	mov_priv_t *priv = demux->priv;
	demux_stream_t *ds;
	sh_video_t *sh_video;

	//DpWord(pts);
	ds = demux->video;
	sh_video = demux->video->sh;

	{
		mov_track_t *trak = priv->tracks[ds->id];

	}

	if (ds && ds->id >= 0 && ds->id < priv->track_db)
	{
		mov_track_t *trak = priv->tracks[ds->id];

		pts = ds->pts = mov_seek_track(trak, pts, flags);
		flags = 1;				// absolute seconds

		if (trak->samplesize)
		{
			sh_video->num_frames = trak->chunks[trak->pos].sample;
		}
		else
		{
			sh_video->num_frames = trak->pos;
		}
	}

	return 1;
}
#endif

static int _control(int cmd, void *arg)
{
	switch (cmd)
	{
	case DEMUXER_CTRL_IF_PTS_FROM_STREAM:
		*((int*)arg) = 0;
		return DEMUXER_CTRL_OK;

	default:
		return DEMUXER_CTRL_NOTIMPL;
	}
}


demuxer_t *new_mov_demux()
{
	demux = new_base_demux();
	// Assign all of the function pointers and type specific data
	demux->check_type = _check_type;
	demux->open = _open;
	demux->fill_buffer = _fill_buffer;
	demux->seek = _seek;
	demux->get_pts_by_relframe = _get_pts_by_relframe;
	demux->control = _control;
	demux->close = _close;
	demux->get_movie_info = _get_movie_info;
//	demux->seek_video = _seek_video;
	demux->type = DEMUXER_TYPE_MOV;
	demux->file_format = DEMUXER_TYPE_MOV;
	return demux;
}
#endif
