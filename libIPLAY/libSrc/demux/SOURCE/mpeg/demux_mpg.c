/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_mpg.c
*
* Programmer:    Deming Li
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: MPG format file demux implementation
*
* Change History (most recent first):
*     <1>     03/30/2005    Deming Li    first file
****************************************************************
*/

#include "global612.h"
#if VIDEO_ENABLE

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
#define MP_PERFORMANCE_TRACE 0


#include "mpTrace.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "parse_es.h"
#include "mpeg_hdr.h"
#include "stheader.h"
#include "video_decoder.h"
#include "mpeg/demux_mpg.h"



#define	MPG_AUDIO	!PURE_VIDEO
#define	FORCE_GENERATE_VPTS	0	//generate video pts all the time, not use pts in mpg file
#define	SEPERATE_AUDIO_STREAM_PS 1


///@ingroup group1_Demux
///@defgroup     MPG  MPG File Format
///

//#define IS_AUDIO_PACKET(id)  ((id >= MPEG_AUDIO_PACKET_MIN_SID && id <= MPEG_AUDIO_PACKET_MAX_SID) || id == MPEG_SYS_PRIVATE_STREAM_1)
#define IS_AUDIO_PACKET(id)  (id >= MPEG_AUDIO_PACKET_MIN_SID && id <= MPEG_AUDIO_PACKET_MAX_SID)
#define IS_VIDEO_PACKET(id)  (id >= MPEG_VIDEO_PACKET_MIN_SID && id <= MPEG_VIDEO_PACKET_MAX_SID)

static demuxer_t *demux = NULL;
#define	STREAM_NAME(ds, demuxer)	((ds == demuxer->audio ? "A" : (ds==demuxer->video?"V":"O")))

static int parse_psm(demuxer_t * const demuxer, const int len)
{
	#define UNKNOWN         0
	#define VIDEO_MPEG1     0x10000001
	#define VIDEO_MPEG2     0x10000002
	#define VIDEO_MPEG4     0x10000004
	#define VIDEO_H264      0x10000005
	#define AUDIO_MP2       0x50
	#define AUDIO_A52       0x2000
	#define AUDIO_LPCM_BE   0x10001
	#define AUDIO_AAC       mmioFOURCC('M', 'P', '4', 'A')
	
	unsigned char c, id, type;
	unsigned int plen, prog_len, es_map_len;
	mpg_demuxer_t * const priv = (mpg_demuxer_t *) demuxer->priv;

	if (! len || len > 1018)	return 0;

	c = stream_read_char(demuxer->stream);
	if (! (c & 0x80))
	{
		stream_skip(demuxer->stream, len - 1);  //not yet valid, discard
		return 0;
	}
	priv->es_map = (unsigned int*)mem_malloc(0x40);
	stream_skip(demuxer->stream, 1);
	prog_len = stream_read_word(demuxer->stream);		//length of program descriptors
	stream_skip(demuxer->stream, prog_len);			//.. that we ignore
	es_map_len = stream_read_word(demuxer->stream);		//length of elementary streams map
	es_map_len = min(es_map_len, len - prog_len - 8);	//sanity check
	while (es_map_len > 0)
	{
		type = stream_read_char(demuxer->stream);
		id = stream_read_char(demuxer->stream);
		if (id >= 0xB0 && id <= 0xEF && priv)
		{
			int idoffset = id - 0xB0;
			switch (type)
			{
			case 0x1:
				priv->es_map[idoffset] = VIDEO_MPEG1;
				break;
			case 0x2:
				priv->es_map[idoffset] = VIDEO_MPEG2;
				break;
			case 0x3:
			case 0x4:
				priv->es_map[idoffset] = AUDIO_MP2;
				break;
			case 0x0f:
			case 0x11:
				priv->es_map[idoffset] = AUDIO_AAC;
				break;
			case 0x10:
				priv->es_map[idoffset] = VIDEO_MPEG4;
				break;
			case 0x1b:
				priv->es_map[idoffset] = VIDEO_H264;
				break;
			case 0x81:
				priv->es_map[idoffset] = AUDIO_A52;
				break;
			}
//     mp_dbg(MSGT_DEMUX,MSGL_V, "PSM ES, id=0x%x, type=%x, stype: %x\n", id, type, priv->es_map[idoffset]);
		}
		plen = stream_read_word(demuxer->stream);		//length of elementary stream descriptors
		plen = min(plen, es_map_len);			//sanity check
		stream_skip(demuxer->stream, plen);			//skip descriptors for now
		es_map_len -= 4 + plen;
	}
	stream_skip(demuxer->stream, 4);			//skip crc32
	return 1;
}

static unsigned int read_mpeg_timestamp(stream_t * const s, const int c)
{
	int d, e;
	unsigned int pts;

	d = stream_read_word(s);
	e = stream_read_word(s);
	if (((c & 1) != 1) || ((d & 1) != 1) || ((e & 1) != 1))
	{
		return 0;				// invalid pts
	}
	pts = ((uint64_t)((c >> 1) & 7) << 30) | ((d >> 1) << 15) | (e >> 1);
	return pts;
}

/*!
@return process this audio stream or not
@retval 1 process this audio stream
@retval 0 skip this audio stream
*/
static int new_audio_stream(demuxer_t * const demuxer, const int aid)
{
	mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
	if (!demuxer->a_streams[aid] && !mpg_d->num_a_streams)
	{
		mpDebugPrint("new_audio_stream 0x%x", aid);
		new_sh_audio(demuxer,aid);
		sh_audio_t* const sh_a = (sh_audio_t*)demuxer->a_streams[aid];
		switch (aid & 0xE0)  // 1110 0000 b  (high 3 bit: type  low 5: id)
		{
		case 0x00:
			sh_a->format=0x50;
			break; // mpeg
		case 0xA0:
			sh_a->format=0x10001;
			break;  // dvd pcm
		case 0x80:
			if ((aid & 0xF8) == 0x88) sh_a->format=0x2001;//dts
			else sh_a->format=0x2000;
			break; // ac3
		}
		//evo files
		if ((aid & 0xC0) == 0xC0) sh_a->format=0x2000;
		else if (aid >= 0x98 && aid <= 0x9f) sh_a->format=0x2001;
		if (mpg_d) mpg_d->a_stream_ids[mpg_d->num_a_streams++] = aid;
	}
	if (demuxer->audio->id==-1) demuxer->audio->id=aid;
	if (demuxer->a_streams[aid])	return 1;
	else						return 0;
}


/*!
@retval	1	success
@retval	0	psm
@retval	-1	invalid packet or padding or private2
@retval	-5	skip packet
*/
static int read_packet(const int id, const demux_stream_t * const dst)
{

	#define MAX_PS_PACKETSIZE (224*1024)
	int d;
	int len;
	unsigned char c = 0;
	unsigned int pts = 0;
	unsigned int dts = 0;
	demux_stream_t *ds = NULL;
	demuxer_t * const demuxer = dst->demuxer;
	stream_t *stream = demuxer->stream;
	mpg_demuxer_t * const mpg_d = (mpg_demuxer_t*)demuxer->priv;
	sh_video_t * const sh_video = demuxer->video->sh;
	int pes_ext2_subid = -1;
	mpg_demuxer_t * const priv = mpg_d;

#if SEPERATE_AUDIO_STREAM_PS
	if (dst == demuxer->audio && demuxer->stream_a)	stream = demuxer->stream_a;
#endif
	
	if ((id<MPEG_SYS_MAP_STREAM_ID || id>=0x1F0) && id != 0x1FD)
		return -1;

	len = stream_read_word(stream);
	MP_DEBUG("%s(0x%x, %s) %d from 0x%x", __FUNCTION__, id, STREAM_NAME(dst, demuxer), len, stream_tell(stream));

	if (len == 0 || len > MAX_PS_PACKETSIZE)
	{
		MP_ALERT("Return -2.  Invalid PS packet(%s) len: =%d, pos=0x%x ,%s:%d", STREAM_NAME(dst, demuxer), len, stream_tell(stream),__FILE__,__LINE__);
		return -2;				// invalid packet !!!!!!
	}

	if (id==MPEG_SYS_PADDING_STREAM || id==MPEG_SYS_PRIVATE_STREAM_2)
	{
	//	mpDebugPrint("stream_skip len=%d while id=0x%x", len, id);
		stream_skip(stream, len);
		return -1;
	}

	
	if (id==MPEG_SYS_MAP_STREAM_ID)
	{
		parse_psm(demuxer, len);
		return 0;
	}

	while (len > 0)
	{							// Skip stuFFing bytes
		c = stream_read_char(stream);
		--len;
		if (c != 0xFF)	break;
	}
	
	if ((c >> 6) == 1)
	{							// Read (skip) STD scale & size value
		//MP_DEBUG("  STD_scale=%d",(c>>5)&1);
		d = ((c & 0x1F) << 8) | stream_read_char(stream);
		len -= 2;
		//MP_DEBUG("  STD_size=%d",d);
		c = stream_read_char(stream);
	}

	// Read MPEG-1 packet timestamps:
	if ((c >> 4) == 2)
	{
		pts = read_mpeg_timestamp(stream, c);
		len -= 4;
	}
	else if ((c >> 4) == 3)
	{
		pts = read_mpeg_timestamp(stream, c);
		c = stream_read_char(stream);
		if ((c >> 4) != 1)	pts = 0;//MP_DPF("{ERROR4}");
		dts = read_mpeg_timestamp(stream, c);
		len -= 4 + 1 + 4;
	}
	else if ((c >> 6) == 2)
	{							// MPEG2 PES packet
		
		int pts_flags;
		int hdrlen;
		int parse_ext2;

		// System-2 (.VOB) stream:
		if ((c >> 4) & 3)
		{
		}
		c = stream_read_char(stream);
		pts_flags = c >> 6;
		parse_ext2 = (id == 0x1FD) && ((c & 0x3F) == 1);
		c = stream_read_char(stream);
		hdrlen = c;
		len -= 2;

		if (hdrlen > len)	return -1;
		if (pts_flags == 2 && hdrlen >= 5)
		{
			c = stream_read_char(stream);
			pts = read_mpeg_timestamp(stream, c);
			len -= 5;
			hdrlen -= 5;
		}
		else if (pts_flags == 3 && hdrlen >= 10)
		{
			c = stream_read_char(stream);
			pts = read_mpeg_timestamp(stream, c);
			c = stream_read_char(stream);
			dts = read_mpeg_timestamp(stream, c);
			len -= 10;
			hdrlen -= 10;
		}
		len -= hdrlen;

		if (parse_ext2 && hdrlen>=3)
		{
			c=stream_read_char(stream);
			hdrlen--;

			if ((c & 0x0F) != 0x0F)
			{
				MP_DEBUG("demux_mpg: pes_extension_flag2 not set, discarding pes packet");
				return -1;
			}
			if (c & 0x80)  //pes_private_data_flag
			{
				if (hdrlen<16)
				{
					MP_DEBUG("demux_mpg: not enough pes_private_data bytes: %d < 16, discarding pes packet", hdrlen);
					return -1;
				}
				stream_skip(stream, 16);
				hdrlen-=16;
			}
			if (c & 0x40)  //pack_header_field_flag
			{
				int l = stream_read_char(stream);
				if (l < 0) //couldn't read from the stream?
					return -1;
				hdrlen--;
				if (l < 0 || hdrlen < l)
				{
					MP_DEBUG("demux_mpg: not enough pack_header bytes: hdrlen: %d < skip: %d, discarding pes packet\n",
					       hdrlen, l);
					return -1;
				}
				stream_skip(stream, l);
				hdrlen-=l;
			}
			if (c & 0x20)  //program_packet_sequence_counter_flag
			{
				if (hdrlen < 2)
				{
					MP_DEBUG("demux_mpg: not enough program_packet bytes: hdrlen: %d, discarding pes packet\n", hdrlen);
					return -1;
				}
				stream_skip(stream, 2);
				hdrlen-=2;
			}
			if (c & 0x10)
			{
				//STD
				stream_skip(stream, 2);
				hdrlen-=2;
			}
			c=stream_read_char(stream); //pes_extension2 flag
			hdrlen--;
			if (c!=0x81)
			{
				MP_DEBUG("demux_mpg: unknown pes_extension2 format, len is > 1  \n");
				return -1;
			}
			c=stream_read_char(stream); //pes_extension2 payload === substream id
			hdrlen--;
			if (c<0x55 || c>0x5F)
			{
				MP_DEBUG("demux_mpg: unknown vc1 substream_id: 0x%x  \n", c);
				return -1;
			}
			pes_ext2_subid=c;
		}
		
		if (hdrlen > 0)	stream_skip(stream, hdrlen);	// skip header bytes

		if (id==0x1FD && pes_ext2_subid!=-1)
		{
			//==== EVO VC1 STREAMS ===//
			if (!demuxer->v_streams[pes_ext2_subid]) new_sh_video(demuxer,pes_ext2_subid);
			if (demuxer->video->id==-1) demuxer->video->id=pes_ext2_subid;
			if (demuxer->video->id==pes_ext2_subid)
			{
				ds=demuxer->video;
				if (!ds->sh) ds->sh=demuxer->v_streams[pes_ext2_subid];
				if (priv && ds->sh)
				{
					sh_video_t *sh = (sh_video_t *)ds->sh;
					sh->format = mmioFOURCC('W', 'V', 'C', '1');
				}
			}
		}
		
		//============== DVD Audio sub-stream ======================
		if (id==MPEG_SYS_PRIVATE_STREAM_1)
		{
			int aid, rawa52 = 0;
			off_t tmppos;
			unsigned int tmp;

			tmppos = stream_tell(stream);
			tmp = stream_read_word(stream);
			stream_seek(stream, tmppos);
			/// vdr stores A52 without the 4 header bytes, so we have to check this condition first
			if (tmp == 0x0B77)
			{
				aid = 128;
				rawa52 = 1;
			}
			else
			{
				aid=stream_read_char(stream);
				--len;
				if (len<3) return -1; // invalid audio packet
			}

			// AID:
			// 0x20..0x3F  subtitle
			// 0x80..0x87 and 0xC0..0xCF  AC3 audio
			// 0x88..0x8F and 0x98..0x9F  DTS audio
			// 0xA0..0xBF  PCM audio

			if ((aid & 0xE0) == 0x20)
			{
				//temporary skip subtitle
				stream_skip(stream, len);
				return -1;
				
				// subtitle:
/*				aid&=0x1F;

				if (!demuxer->s_streams[aid])
				{
					sh_sub_t *sh = new_sh_sub(demuxer, aid);
					if (sh) sh->type = 'v';
					mp_msg(MSGT_DEMUX,MSGL_V,"==> Found subtitle: %d\n",aid);
				}

				if (demuxer->sub->id > -1)
					demuxer->sub->id &= 0x1F;
				if (!dvdsub_lang && demuxer->sub->id == -1)
					demuxer->sub->id = aid;
				if (demuxer->sub->id==aid)
				{
					ds=demuxer->sub;
				}
*/			}
			else if ((aid >= 0x80 && aid <= 0x8F) || (aid >= 0x98 && aid <= 0xAF) || (aid >= 0xC0 && aid <= 0xCF))
			{
				//        aid=128+(aid&0x7F);
				// aid=0x80..0xBF
				#if !MPG_AUDIO
					stream_skip(stream, len);
					return 1;
				#endif
				if (!new_audio_stream(demuxer, aid))
				{
					stream_skip(stream, len);
					return -1;
				}
				if (demuxer->audio->id==aid)
				{
					int type;
					ds=demuxer->audio;
					if (!ds->sh)
					{
						ds->sh=demuxer->a_streams[aid];
						((sh_audio_t *) demuxer->sh_audio)->ds = ds;
					}
					// READ Packet: Skip additional audio header data:
					if (!rawa52)
					{
						c=stream_read_char(stream);//num of frames
						type=stream_read_char(stream);//startpos hi
						type=(type<<8)|stream_read_char(stream);//startpos lo
        				MP_DEBUG("DVD audio [%02X][%04X]",c,type);
						len-=3;
					}
					if ((aid&0xE0)==0xA0 && len>=3)
					{
						unsigned char* hdr;
						// save audio header as codecdata!
						if (!((sh_audio_t*)(ds->sh))->codecdata_len)
						{
							((sh_audio_t*)(ds->sh))->codecdata=(unsigned char*)mem_malloc(3);
							((sh_audio_t*)(ds->sh))->codecdata_len=3;
						}
						hdr=((sh_audio_t*)(ds->sh))->codecdata;
						// read LPCM header:
						// emphasis[1], mute[1], rvd[1], frame number[5]:
						hdr[0]=stream_read_char(stream);
					//          printf(" [%01X:%02d]",c>>5,c&31);
						// quantization[2],freq[2],rvd[1],channels[3]
						hdr[1]=stream_read_char(stream);
					//          printf("[%01X:%01X] ",c>>4,c&15);
						// dynamic range control (0x80=off):
						hdr[2]=stream_read_char(stream);
					//          printf("[%02X] ",c);
						len-=3;
						if (len<=0) MP_ALERT("End of packet while searching for PCM header");
					}
					//        printf("  \n");
				} //  if(demux->audio->id==aid)

			}
			else MP_DEBUG("Unknown 0x1BD substream: 0x%02X  ",aid);
		} //if(id==0x1BD)

		
	}
	else
	{
		if (c != 0x0f)	return -1;	// invalid packet !!!!!!
	}

	if (len <= 0 || len > MAX_PS_PACKETSIZE)	return -1;// invalid packet !!!!!!

#if SEPERATE_AUDIO_STREAM_PS	
	if (demuxer->stream_a && (((IS_AUDIO_PACKET(id)|| id==MPEG_SYS_PRIVATE_STREAM_1) && dst!=demuxer->audio) || (IS_VIDEO_PACKET(id) && dst!=demuxer->video)))
	{
		MP_DEBUG("skip !%s, id=%x len=%d from 0x%x", STREAM_NAME(dst, demuxer), id, len, stream_tell(stream));
		stream_skip(stream, len);
		return -5;
	}
#endif

	if (IS_AUDIO_PACKET(id) /* && dst==demuxer->audio */)
	{
#if !MPG_AUDIO
		stream_skip(stream, len);
		MP_DEBUG("skip audio 0x%x", len);
		return 1;
#endif
		const int aid = id - MPEG_AUDIO_PACKET_MIN_SID;
		if (!demuxer->a_streams[aid])
		{
			mpDebugPrint("audio stream 0x%x found",id);
			new_audio_stream(demuxer, aid);
		}
		if (demuxer->audio->id == -1)	demuxer->audio->id = aid;
		if (demuxer->audio->id == aid)
		{
			//MP_PERFORMANCE_TRC("audio packet(t:%d)",ELAPSE_TIME);
			MP_DEBUG("audio packet");
			ds = demuxer->audio;
			if (!ds->sh)
			{
				ds->sh = demuxer->a_streams[aid];
				((sh_audio_t *) demuxer->sh_audio)->ds = ds;
			}
		}
	}
	else if (IS_VIDEO_PACKET(id))
	{
		const int vid = id - MPEG_VIDEO_PACKET_MIN_SID;
		if (!demuxer->v_streams[vid])
		{
			mpDebugPrint("new video stream 0x%x",id);
			new_sh_video(demuxer, vid);
		}
		if (demuxer->video->id == -1)	demuxer->video->id = vid;
		if (demuxer->video->id == vid)
		{
			ds = demuxer->video;
			if (!ds->sh)
			{
				ds->sh = demuxer->v_streams[vid];
				((sh_video_t *) demuxer->sh_video)->ds = ds;
			}
		}
	}

	if (ds)
	{
		float fpts=(float)pts/90000.0f;
		if (mpg_d)
		{
			if (!pts && mpg_d->find_pts)	return -1;
			if (pts)
			{
				mpg_d->last_pts = fpts;
				if (mpg_d->find_first_apts)
				{
					mpg_d->first_apts = mpg_d->last_pts;
					mpg_d->find_first_apts = 0;
				}
			}
		}
		//extrapolation pts for video if needed
		if (IS_VIDEO_PACKET(id) && dst == demuxer->video)
		{
			mpg_d->has_vpacket_header = 1;
			mpg_d->nextvframe_vpts = fpts;
#if !FORCE_GENERATE_VPTS
			if (pts)
			{
				mpg_d->no_pts_after_vframes = 0;
				if (fpts!=mpg_d->last_vpts)
				{
					mpg_d->last2_vpts = mpg_d->last_vpts;
					mpg_d->last_vpts = fpts;
				}
				mpg_d->no_vpts = 0;
			}
			else if (mpg_d->seeking && mpg_d->no_vpts)
			{
				fpts = mpg_d->last_vpts;
				mpg_d->no_pts_after_vframes = 0;
			}
			else if (mpg_d->no_pts_after_vframes > (unsigned int)(sh_video->fps*0.7))
			{
#endif
				mpg_d->no_vpts = 1;
				fpts = mpg_d->last_vpts + sh_video->frametime*mpg_d->no_pts_after_vframes;
//				mpDebugPrint("extrapolation1 fpts=%d", FLOAT1000(fpts));
#if !FORCE_GENERATE_VPTS
			}
			else
			{
				fpts = mpg_d->last_vpts + sh_video->frametime*mpg_d->no_pts_after_vframes;
				//mpDebugPrint("extrapolation2 when no pts fpts=%d, while last_vpts=%d, no_pts_after_vframes=%d", FLOAT1000(fpts), FLOAT1000(mpg_d->last_vpts), mpg_d->no_pts_after_vframes);
			}
#endif
		}
		
		//if (IS_VIDEO_PACKET(id))	
			MP_DEBUG("ds_read_packet(%s), fpts=%d, last_vpts=%d, last2_vpts=%d, current_vpts=%d", STREAM_NAME(dst, demuxer), FLOAT1000(fpts), FLOAT1000(mpg_d->last_vpts), FLOAT1000(mpg_d->last2_vpts), FLOAT1000(mpg_d->nextvframe_vpts));
		ds_read_packet(ds, stream, len, fpts, demuxer->filepos, 0);
		return 1;
	}
	MP_DEBUG("DEMUX_MPG(%s): Skipping %d data bytes from packet %04X at pos 0x%x", STREAM_NAME(dst, demuxer),len,id, stream_tell(stream));
//	if (len <= 2356)	//liwu: what is this number for?
		stream_skip(stream, len);
	return 0;
}

static int _mpeg_es_fill_buffer(demuxer_t * const demuxer)
{
	mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
	const demux_stream_t * const d_video = demuxer->video;
	const sh_video_t * const sh_video = d_video->sh;
	const float fpts = 0.033*mpg_d->no_pts_after_vframes;
	MP_DEBUG("fpts=%d, no_pts_after_vframes=%d", FLOAT1000(fpts), mpg_d->no_pts_after_vframes); 
	// Elementary video stream
	if (demuxer->stream->eof)	return 0;
	demuxer->filepos = stream_tell(demuxer->stream);
	ds_read_packet(demuxer->video, demuxer->stream, STREAM_BUFFER_SIZE, fpts, demuxer->filepos, 0);
	return 1;
}

/*! \ingroup	MPG
 *  \brief		Free all mpg demuxer memory
 *	\param		none
 *	\return		EOF or not;
 *	\retval		1 Not EOF
 *	\retval		0 EOF
 */
static int _fill_buffer(demux_stream_t * dst)
{
	demuxer_t * const demuxer = dst->demuxer;
	if (demuxer->type == DEMUXER_TYPE_MPEG_ES)	return _mpeg_es_fill_buffer(demuxer);
	unsigned int head = 0;
	int skipped = 0;
	int max_packs = 2048;		// 512kbyte
	int ret = 0;
	mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
	
	stream_t *stream = demuxer->stream;
#if SEPERATE_AUDIO_STREAM_PS
	if (dst==demuxer->audio && demuxer->stream_a)	stream = demuxer->stream_a;//not preview
#endif

	//MP_PERFORMANCE_TRC("mpg_fill_buffer(%s)pos:0x%x(t:%d)",STREAM_NAME(dst, demuxer), stream_tell(stream),ELAPSE_TIME);
	MP_DEBUG("mpg_fill_buffer(%s)pos:0x%x",STREAM_NAME(dst, demuxer), stream_tell(stream));

	// For MPEG_PS, do the following.
	do
	{
		demuxer->filepos = stream_tell(stream);
		demuxer->movi_end = stream->end_pos;
		head = stream_read_dword(stream);

		if ((head & 0xFFFFFF00) != 0x100)
		{
			// sync...
			demuxer->filepos -= skipped;
			while (1)
			{
				int c = stream_read_char(stream);
				if (c < 0)	break;		//EOF
				head <<= 8;
				if (head != 0x100)
				{
					head |= c;
					++skipped;
					continue;
				}
				head |= c;
				break;
			}
			demuxer->filepos += skipped;
		}

		if (stream_eof(stream))	break;
		
		// sure: head=0x000001XX
		ret = read_packet(head, dst);
		if (!ret && --max_packs==0)
		{
			MP_ALERT("demux_mpg: File doesn't contain the selected audio or video stream.");
			stream->eof=1;
			return 0;
		}
	}
	while (ret != 1);
	return stream->eof ? 0 : 1;
}

extern BOOL boVideoPreview;
static int _seek(float rel_seek_secs, int flags)//flags always 0, except start
{
	//#undef MP_DEBUG
	//#define MP_DEBUG mpDebugPrint
	MP_DEBUG("mpg_seek(%d,%d) 1",FLOAT1000(rel_seek_secs),flags);
	demuxer_t * const demuxer = demux;
	demux_stream_t * const d_audio = demuxer->audio;
	demux_stream_t * const d_video = demuxer->video;
	sh_video_t * const sh_video = d_video->sh;
	mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
	stream_t * const stream = demuxer->stream;
	stream_t * const stream_a = demuxer->stream_a;
	int i;
	unsigned char picture_coding_type;
	#define CODING_TYPE (picture_coding_type==1?"I":(picture_coding_type==2?"P":(picture_coding_type==3?"B":"O")))
	mpg_d->seeking = 1;

#if MPG_AUDIO
	sh_audio_t * const sh_audio = d_audio->sh;
	if (!sh_audio)	MP_ALERT("sh_audio=NULL");
#else	
	sh_audio_t * sh_audio = NULL;
	mpDebugPrint("BLOCK MPG_AUDIO");
#endif
	
	//================= seek in MPEG ==========================
	//calculate the pts to seek to
	off_t newpos_v = (flags & 1) ? demuxer->movi_start : demuxer->filepos;
	off_t newpos_a = (flags & 1) ? demuxer->movi_start : demuxer->filepos;

	if (flags & 2)
	{
		// float seek 0..1
		newpos_v += rel_seek_secs*(demuxer->movi_end - demuxer->movi_start);
	}
	else
	{
		// time seek (secs)
		if (!sh_video->i_bps)	newpos_v += rel_seek_secs*2324 * 75 ;	// 174.3 kbyte/sec
		else					newpos_v += (off_t) (rel_seek_secs * sh_video->i_bps);

		#if MPG_AUDIO
		if (sh_audio->i_bps)		newpos_a += (off_t) (rel_seek_secs * (sh_audio->i_bps+sh_video->i_bps));
		else					MP_DEBUG("NO sh_audio->i_bps");

		#endif
	}


	if (newpos_v < (signed)demuxer->movi_start)
	{
		//demuxer->movi_start = 0;
		//if (newpos_v < demuxer->movi_start)	
		newpos_v = demuxer->movi_start;
	}
	if (newpos_a < (signed)demuxer->movi_start)
	{
		//demuxer->movi_start = 0;
		//if (newpos_a < (signed)demuxer->movi_start)
		newpos_a = demuxer->movi_start;
	}

	if (newpos_v > (signed)demuxer->movi_end)	newpos_v = demuxer->movi_end;
	if (newpos_a > (signed)demuxer->movi_end)	newpos_a = demuxer->movi_end;

	newpos_a = newpos_v;

#ifdef _LARGEFILE_SOURCE
	newpos_v &= ~((long long) STREAM_BUFFER_SIZE - 1);	// sector boundary
#else
	newpos_v &= ~(STREAM_BUFFER_SIZE - 1);	// sector boundary
	newpos_a &= ~(STREAM_BUFFER_SIZE - 1);	// sector boundary
#endif

	MP_DEBUG("seek video 0x%x",newpos_v);
	stream->eof = 0;
	stream_seek(stream, newpos_v);
#if SEPERATE_AUDIO_STREAM_PS
	if (stream_a)
	{
		MP_DEBUG("seek audio 0x%x",newpos_a);
		stream_a->eof = 0;
		stream_seek(stream_a, newpos_a);
	}
#endif
	demuxer->filepos = stream_tell(stream);
	// re-sync video:
	videobuf_code_len = 0;		// reset ES stream buffer

#if FORCE_GENERATE_VPTS
	mpg_d->no_vpts = 1;
#endif


	if (mpg_d->no_vpts)	mpg_d->no_pts_after_vframes += rel_seek_secs*sh_video->fps;
	else				mpg_d->find_pts = 1;
	if (sh_audio && stream_a)
	{
		MP_DEBUG("fill audio buffer in mpg_seek");
		ds_fill_buffer(d_audio);
	}
	MP_DEBUG("fill video buffer in mpg_seek");
	ds_fill_buffer(d_video);
	float a_pts;

	MP_DEBUG("apts:%d, vpts:%d", FLOAT1000(d_audio->pts),FLOAT1000(d_video->pts)); 
	mpg_d->find_pts = 0;

	while (1)
	{
#if FORCE_GENERATE_VPTS
		if (!boVideoPreview && sh_audio && stream_a && !d_audio->eof)
		{
			if (stream_tell(stream) > stream_tell(stream_a))
			{
				skip_audio_frame(sh_audio);  // sync audio
				continue;
			}
		}
#else
		if (!boVideoPreview && sh_audio && !d_audio->eof && !mpg_d->no_vpts && d_video->pts && d_audio->pts)
		{
			a_pts = d_audio->pts + (float)(ds_tell_pts(d_audio) - sh_audio->a_in_buffer_len) / (float) sh_audio->i_bps;
			if (d_video->pts>a_pts)
			{
				skip_audio_frame(sh_audio);  // sync audio
				continue;
			}
			
		}
#endif
		if (!sh_video) break;
		i = sync_video_packet(d_video);
		
		if (i==MPEG_DATA_PICTURE_START_CODE)
			picture_coding_type = (d_video->buffer[d_video->buffer_pos+1] >> 3) & 3;

		if (i == MPEG_DATA_SEQUENCE_START_CODE || i == MPEG_DATA_GOP_START_CODE ||
			(i==MPEG_DATA_PICTURE_START_CODE && picture_coding_type == 1))//find next I, additional to next sequence header or gop header. Since mpeg-2 gop header is optional, and sequence header is not necesarry, either
			break;// found sequence header or GOP header! Or I frame
			
		if (!i || !skip_video_packet(d_video))
		{
			MP_ALERT("can't sync next video packet");
			break;
		}
	}

	mpg_d->seeking = 0;
	MP_DEBUG("mpg_seek(%d,) completed while i=0x%x v_pos=0x%x, a_pos=0x%x",FLOAT1000(rel_seek_secs),i,stream_tell(stream),stream_tell(stream_a));
	MP_DEBUG("\tapts:%d, vpts:%d", FLOAT1000(d_audio->pts),FLOAT1000(d_video->pts)); 
	return 1;
	//#undef MP_DEBUG
	//#define	MP_DEBUG
}

static int _control(int cmd, void *arg)
{
	MP_DEBUG("mpg_control(%d)",cmd);
	demuxer_t * const demuxer = demux;
	mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;

	switch (cmd)
	{
/*	case DEMUXER_CTRL_GET_TIME_LENGTH:
		if (stream_control(demuxer->stream, STREAM_CTRL_GET_TIME_LENGTH, arg) != STREAM_UNSUPPORTED)
		{
			mp_msg(MSGT_DEMUXER,MSGL_DBG2,"\r\nDEMUX_MPG_CTRL, (%.3lf)\r\n", *((double*)arg));
			return DEMUXER_CTRL_GUESS;
		}
		if (mpg_d && mpg_d->has_valid_timestamps)
		{
			*((double *)arg)=(double)mpg_d->first_to_final_pts_len;
			return DEMUXER_CTRL_OK;
		}
		return DEMUXER_CTRL_DONTKNOW;

	case DEMUXER_CTRL_GET_PERCENT_POS:
		if (mpg_d && mpg_d->has_valid_timestamps && mpg_d->first_to_final_pts_len > 0.0)
		{
			*((int *)arg)=(int)(100 * (mpg_d->last_pts-mpg_d->first_pts) / mpg_d->first_to_final_pts_len);
			return DEMUXER_CTRL_OK;
		}
		return DEMUXER_CTRL_DONTKNOW;
*/

	case DEMUXER_CTRL_SWITCH_AUDIO:
		if (! (mpg_d && mpg_d->num_a_streams > 1 && demuxer->audio && demuxer->audio->sh))
			return DEMUXER_CTRL_NOTIMPL;
		else
		{
			demux_stream_t *d_audio = demuxer->audio;
			sh_audio_t *sh_audio = d_audio->sh;
			sh_audio_t *sh_a = sh_audio;
			int i;
			if (!sh_audio)
				return DEMUXER_CTRL_NOTIMPL;
			if (*((int*)arg) < 0)
			{
				for (i = 0; i < mpg_d->num_a_streams; i++)
				{
					if (d_audio->id == mpg_d->a_stream_ids[i]) break;
				}
				i = (i+1) % mpg_d->num_a_streams;
				sh_a = (sh_audio_t*)demuxer->a_streams[mpg_d->a_stream_ids[i]];
			}
			else
			{
				for (i = 0; i < mpg_d->num_a_streams; i++)
					if (*((int*)arg) == mpg_d->a_stream_ids[i]) break;
				if (i < mpg_d->num_a_streams)
					sh_a = (sh_audio_t*)demuxer->a_streams[*((int*)arg)];
			}
			if (i < mpg_d->num_a_streams && d_audio->id != mpg_d->a_stream_ids[i])
			{
				d_audio->id = mpg_d->a_stream_ids[i];
				d_audio->sh = sh_a;
				ds_free_packs(d_audio);
			}
		}
		*((int*)arg) = demuxer->audio->id;
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_IF_PTS_FROM_STREAM:
		*((int*)arg) = !mpg_d->no_vpts;
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_GET_1ST_APTS:
		*((float*)arg) = mpg_d->first_apts;
		return DEMUXER_CTRL_OK;
		
	default:
		return DEMUXER_CTRL_NOTIMPL;
	}
}

static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
	demuxer_t * const demuxer = demux;
	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_MPEG_PS))
	{
		unsigned int head;

		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demuxer->stream = stream;
		head = stream_read_dword(demuxer->stream);
		//mpeg1_fix = 0;
		if (head == MPEG_PACK_START_CODE || (head >= MPEG_AUDIO_PACKET_MIN_SID && head <= MPEG_VIDEO_PACKET_MAX_SID))
		{
			//MP_DEBUG("MPEG PS  selected.\r\n");
			demuxer->type = DEMUXER_TYPE_MPEG_PS;
		}
		else if (head == MPEG_PACK_START_CODE_FIX)
		{
			//demux->synced = 0;	// hack!
			//mpeg1_fix = 1;
			demuxer->type = DEMUXER_TYPE_MPEG_PS;	//fixed
		}
		else if (head == MPEG_DATA_SEQUENCE_START_CODE || head == MPEG_DATA_GOP_START_CODE || head == MPEG_DATA_PICTURE_START_CODE)
				demuxer->type = DEMUXER_TYPE_MPEG_ES;
		else	demux_type = DEMUX_UNKNOWN;

		stream->buf_pos = 0;	// Reset to the begining of buffer

		if (demuxer->type != DEMUXER_TYPE_UNKNOWN)	return 1;
		else										return (demux_type == DEMUX_UNKNOWN) ? 1 : -1;
	}
	return 0;
}

#define TIMESTAMP_PROBE_LEN 500000	// 500000 is a wild guess

static int _open()
{
//	#undef MP_DEBUG
//	#define	MP_DEBUG mpDebugPrint
	MP_DEBUG("mpg_open() 1");
	demuxer_t * const demuxer = demux;
	int ret1;

	stream_t * const s = demuxer->stream;
	mpg_demuxer_t * const mpg_d = (mpg_demuxer_t *) mem_malloc(sizeof(mpg_demuxer_t));
	memset(mpg_d, 0, sizeof(mpg_demuxer_t));
	demuxer->priv = mpg_d;

	const off_t pos = stream_tell(s);
	const off_t end_seq_start = demuxer->stream->end_pos - TIMESTAMP_PROBE_LEN;	// 500000 is a wild guess,it may cause fist parsing too much time. wait ..

	stream_seek(demuxer->stream, demuxer->movi_start);
	mpg_d->find_first_apts = 1;
	ret1 = ds_fill_buffer(demuxer->video);
	ds_fill_buffer(demuxer->audio);
	MP_DEBUG("mpg_open() 2, finding final_pts");

	if (demuxer->seekable && stream_tell(demuxer->stream) < end_seq_start)
	{
		stream_seek(s, end_seq_start);
		MP_DEBUG("mpg_open() 2.1");
		while ((!s->eof) && ds_fill_buffer(demuxer->video))
		{
			if (mpg_d->final_pts < mpg_d->last_pts)
				mpg_d->final_pts = mpg_d->last_pts;
			if (stream_tell(s) > demuxer->movi_end)
				break;
		}
		MP_DEBUG("mpg_open() 3, final_pts found: %d ms", FLOAT1000(mpg_d->final_pts));

		ds_free_packs(demuxer->audio);
		ds_free_packs(demuxer->video);
		demuxer->stream->eof = 0;	// clear eof flag
		demuxer->video->eof = 0;
		demuxer->audio->eof = 0;
		MP_DEBUG("mpg_open() 3.5, after ds_free_packs");

		stream_seek(s, pos);
		ret1 = ds_fill_buffer(demuxer->video);
		mpg_d->last2_vpts = mpg_d->last_vpts;
	}
	MP_DEBUG("mpg_open() 4");

	if (demuxer->type == DEMUXER_TYPE_MPEG_ES)
	{
		MP_DEBUG("MPEG_ES");
		new_sh_video(demuxer, 0);
		((sh_video_t *) demuxer->sh_video)->ds = demuxer->video;
		demuxer->sh_audio = 0;
	}

	MP_DEBUG("mpg_open() completed");
	#if FORCE_GENERATE_VPTS
		mpg_d->no_vpts = 1;
		mpDebugPrint("FORCE_GENERATE_VPTS");
	#endif

	ret1 = demuxer->video->pack_no > 0 || demuxer->audio->pack_no > 0;
	if (!ret1)
	{
		mem_free(demuxer->priv);
		demuxer->priv = NULL;
	}
	return ret1;
//	#undef MP_DEBUG
//	#define	MP_DEBUG
}

static void process_userdata(const unsigned char * const buf, const int len)
{
	/* if the user data starts with "CC", assume it is a CC info packet */
	if (len > 2 && buf[0] == 'C' && buf[1] == 'C')
	{
		// if(subcc_enabled) subcc_process_data(buf+2,len-2);
	}
	if (len > 2 && buf[0] == 'T' && buf[1] == 'Y')
	{
		//  ty_processuserdata( buf + 2, len - 2 );
		return;
	}
}

#ifdef MPEG1_DROP_B_FRAME
BYTE MPEG1_FRAME_TYPE = 0;
#endif


/*!
 GetPTSByRelFrame
 @ingroup	 MPG
 @param[in]	rel_seek_samples rel_seek_samples.
 @param[in]	flags	flags
 	0: seek to begin
	1: seek to end
	2: seek to "+/-" rel_seek_samples
 @return	pts
*/
static float _get_pts_by_relframe(int rel_seek_samples, int flags)
{
	const demuxer_t * const demuxer = demux;
	//MP_DEBUG("mpg_get_pts_by_relframe(%d,%d)",rel_seek_samples, flags);
	float pts=0.0;
	const sh_video_t * const sh_video = demuxer->video->sh;

	////MP_DEBUG1("mpeg:get_pts_by_relframe:sh_video->i_bps=", sh_video->i_bps, 8);
	////MP_DEBUG1("mpeg:get_pts_by_relframe:demux->movi_start=", demux->movi_start, 8);
	////MP_DEBUG1("mpeg:get_pts_by_relframe:demux->movi_end=", demux->movi_end, 8);
	////MP_DEBUG1("mpeg:get_pts_by_relframe:demux->filepos=", demux->filepos, 8);

	switch (flags)
	{
	case 2:
		if (!sh_video->i_bps)	// unspecified or VBR
			pts = (float) demuxer->filepos / 2324.0 * 75.0;	// 174.3 kbyte/sec
		else
			pts = (float) demuxer->filepos / (float) sh_video->i_bps;

		pts += rel_seek_samples;

		//test=pts*100000;
		////MP_DEBUG1("mpeg:get_pts_by_relframe:seek flag=2.....pts*1000=", pts*1000, 8);
		break;

	case 0:
		pts = 0;
		////MP_DEBUG1("mpeg:get_pts_by_relframe:seek flag=0.....pts=", pts, 8);
		break;

	case 1:
		if (!sh_video->i_bps)	// unspecified or VBR
			pts = (float) demuxer->movi_end / 2324.0 * 75.0;	// 174.3 kbyte/sec
		else
			pts = (float) demuxer->movi_end / (float) sh_video->i_bps;
		break;

	default:
		break;
	}

	return pts;
}

/*!
 @ingroup	 MPG
 @param[in]	command command to proceed.
 			0: get total time/file length.
 			1: get current time/file point.
 			2: get percentage
 			3: always 3
 @retval 0	fail
 @retval 1	success
*/
static int _get_movie_info(BYTE * buffer, int command)
{
//	MP_DEBUG("mpg_get_movie_info(,%d)",command);
	BYTE *string;
	off_t t1, t2;
	int bits;
	demuxer_t * const demuxer = demux;

	switch (command)
	{
	case 0:
	{
		t1 = t2 = demuxer->stream->file_len;
		////MP_DEBUG1("_get_movie_info:case0 t=", t, 8);

		t1 = t1 / 1024;
		t2 = t2 / 1024;

		bits = 0;
		while (t2)
		{
			t2 /= 10;
			bits++;
		}

		// convert time value to string
		string = buffer;
		t1 = Bin2Bcd(t1);
		string = HexString(string, t1, bits);

		*string = 'k';
		string++;

		*string = 0;

		return 1;
		//break;
	}

	case 1:
	{
		t1 = t2 = demuxer->stream->pos;
		// convert time value to string
		t1 = t1 / 1024;
		t2 = t2 / 1024;

		bits = 0;
		while (t2)
		{
			t2 /= 10;
			bits++;
		}

		string = buffer;
		t1 = Bin2Bcd(t1);
		string = HexString(string, t1, bits);

		*string = 'k';
		string++;

		*string = 0;

		return 1;
	}
	case 2:
	{
		t1 = t2 = demuxer->stream->pos * 100 / demuxer->stream->file_len;
		////MP_DEBUG1("_get_movie_info:case2 t=", t, 8);
		// convert time value to string

		bits = 0;
		while (t2)
		{
			t2 /= 10;
			bits++;
		}
		if (bits == 0)
			bits = 1;

		string = buffer;
		t1 = Bin2Bcd(t1);
		string = HexString(string, t1, bits);

		*string = '%';
		string++;

		*string = 0;

		return 1;

	}
	case 3:
	{
		PMedia_Info pAVIInfo = (PMedia_Info) buffer;
		sh_video_t *sh_video = (sh_video_t *) demuxer->sh_video;
		sh_audio_t *sh_audio = (sh_audio_t *) demuxer->sh_audio;

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

			//if((pAVIInfo->dwTotalTime = (demux->stream->file_len/(sh_audio->i_bps+sh_video->i_bps) - 3))>=0)  //need fix
			//  pAVIInfo->dwFlags |=(DWORD)MOVIE_TotalTime_USEFUL;
		}

		if (sh_video)
		{
			const mpg_demuxer_t * const mpg_d = (mpg_demuxer_t *) demuxer->priv;

			GetVideoCodecCategory(pAVIInfo->cVidoeCodec);
			
			pAVIInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_VIDEO;

			if ((pAVIInfo->dwFrameRate = sh_video->fps) > 0)
				pAVIInfo->dwFlags |= (DWORD) MOVIE_FrameRate_USEFUL;

			if ((pAVIInfo->dwImageHeight = sh_video->disp_h) > 0)
				pAVIInfo->dwFlags |= (DWORD) MOVIE_ImageHeight_USEFUL;

			if ((pAVIInfo->dwImageWidth = sh_video->disp_w) > 0)
				pAVIInfo->dwFlags |= (DWORD) MOVIE_ImageWidth_USEFUL;

			if ((pAVIInfo->dwTotalTime = (DWORD) mpg_d->final_pts) >= 0)
				pAVIInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;


		}

		return 1;
	}
	default:
		break;
	}
	return 0;
}

/*! \ingroup	MPG
 *  \brief		Free all mpg demuxer memory
 *	\param		none
 *	\return		check if free ok;
 *	\retval		1 suceed
 *	\retval		0 failed
 */
static int _close()
{
	demuxer_t * const demuxer = demux;

	if (demuxer->priv)
	{
		mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
		if (mpg_d->es_map)
			mem_free(mpg_d->es_map);
		mem_free(demuxer->priv);
		demuxer->priv = NULL;
	}
	demux = NULL;
	return 1;
}

demuxer_t *new_mpeg_demux()
{
	demux = new_base_demux();
	demux->check_type = _check_type;
	demux->open = _open;
	demux->close = _close;
	demux->fill_buffer = _fill_buffer;
	demux->seek = _seek;
	demux->control = _control;
	demux->get_pts_by_relframe = _get_pts_by_relframe;
	demux->get_movie_info = _get_movie_info;
	demux->type = DEMUXER_TYPE_MPEG_PS;
	demux->file_format = DEMUXER_TYPE_MPEG_PS;
	return demux;
}


#endif

