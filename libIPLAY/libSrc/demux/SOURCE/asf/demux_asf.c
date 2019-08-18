/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_asf.c
*
* Programmer:    Greg Xu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: ASF format file demux operation 
*        
* Change History (most recent first):
*     <1>     03/30/2005    Greg Xu    first file
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

#include "mptrace.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>

#include "demux_types.h"
//#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux_packet.h"
#include "demux.h"


#include "asf.h"

///
///@ingroup group1_Demux
///@defgroup     ASF ASF File Format
///

extern demuxer_t *new_base_demux();


// Asf demuxer
static demuxer_t *demux = 0;


/*
 * Load 16/32-bit values in little endian byte order
 * from an unaligned address
 */
#ifdef ARCH_X86
#define	LOAD_LE32(p)	(*(unsigned int*)(p))
#define	LOAD_LE16(p)	(*(unsigned short*)(p))
#else
#define	LOAD_LE32(p)	(((unsigned char*)(p))[0]     | \
 			 ((unsigned char*)(p))[1]<< 8 | \
 			 ((unsigned char*)(p))[2]<<16 | \
 			 ((unsigned char*)(p))[3]<<24 )
#define	LOAD_LE16(p)	(((unsigned char*)(p))[0]     | \
			 ((unsigned char*)(p))[1]<<8)
#endif

// defined at asfheader.c:
extern unsigned char *asf_packet;
extern int asf_scrambling_h;
extern int asf_scrambling_w;
extern int asf_scrambling_b;
extern int asf_packetsize;
extern double asf_packetrate;
extern int asf_movielength;

/*asf file content by jackyang 20050607*/
extern ASF_Content asfcontent;

/*add end 20050607*/

// based on asf file-format doc by Eugene [http://divx.euro.ru]

static void asf_descrambling(unsigned char *src, int len)
{
	unsigned char *dst = (unsigned char *) mem_malloc(len);
	unsigned char *s2 = src;
	int i = 0, x, y;

	//CHK_MALLOC(dst, "asf_descrambling failed");
	while (len - i >= asf_scrambling_h * asf_scrambling_w * asf_scrambling_b)
	{
		for (x = 0; x < asf_scrambling_w; x++)
			for (y = 0; y < asf_scrambling_h; y++)
			{
				memcpy(dst + i, s2 + (y * asf_scrambling_w + x) * asf_scrambling_b,
					   asf_scrambling_b);
				i += asf_scrambling_b;
			}
		s2 += asf_scrambling_h * asf_scrambling_w * asf_scrambling_b;
	}
	memcpy(src, dst, i);
	mem_free(dst);
}


static int demux_asf_read_packet(demuxer_t * demux, unsigned char *data, int len, int id, int seq,
								 unsigned long time, unsigned short dur, int offs, int keyframe)
{
	demux_stream_t *ds = NULL;

	if (demux->video->id == -1)
		if (demux->v_streams[id])
			demux->video->id = id;

	if (demux->audio->id == -1)
		if (demux->a_streams[id])
			demux->audio->id = id;

	if (id == demux->audio->id)
	{
		// audio
		ds = demux->audio;
		if (!ds->sh)
		{
			ds->sh = demux->a_streams[id];
		}
	}
	else if (id == demux->video->id)
	{
		// video
		ds = demux->video;
		if (!ds->sh)
		{
			ds->sh = demux->v_streams[id];
		}
	}

	if (ds)
	{
		if (ds->asf_packet)
		{
			if (ds->asf_seq != seq)
			{
				// closed segment, finalize packet:
				if (ds == demux->audio)
					if (asf_scrambling_h > 1 && asf_scrambling_w > 1)
						asf_descrambling(ds->asf_packet->buffer, ds->asf_packet->len);
				ds_add_packet(ds, ds->asf_packet);
				ds->asf_packet = NULL;
			}
			else
			{
				// append data to it!
				demux_packet_t *dp = ds->asf_packet;

				if (dp->len != offs && offs != -1)
				{
				}
				dp->buffer = (unsigned char *) mem_reallocm((void *) dp->buffer, dp->len + len);
				memcpy(dp->buffer + dp->len, data, len);
				dp->len += len;
				// we are ready now.
				return 1;
			}
		}
		// create new packet:
		{
			demux_packet_t *dp;

			if (offs > 0)
			{
				return 0;
			}
			dp = (demux_packet_t *) new_demux_packet(len);
			memcpy(dp->buffer, data, len);
			dp->pts = time * 0.001f;
			dp->flags = keyframe;
//      if(ds==demux->video) MP_DPF("ASF time: %8d  dur: %5d  \n",time,dur);
			dp->pos = demux->filepos;
			ds->asf_packet = dp;
			ds->asf_seq = seq;
			// we are ready now.
			return 1;
		}
	}

	return 0;
}


// return value:
//     0 = EOF or no stream found
//     1 = successfully read a packet
static int _fill_buffer(demux_stream_t * dst)
{

	demux->filepos = stream_tell(demux->stream);
	// Brodcast stream have movi_start==movi_end
	// Better test ?
	if ((demux->movi_start < demux->movi_end) && (demux->filepos >= demux->movi_end))
	{
		demux->stream->eof = 1;
		return 0;
	}

	stream_read(demux->stream, (char *) asf_packet, asf_packetsize);
	if (demux->stream->eof)
		return 0;				// EOF

	{
		unsigned char ecc_flags = asf_packet[0];
		unsigned char *p = &asf_packet[1 + (ecc_flags & 15)];
		unsigned char *p_end = asf_packet + asf_packetsize;
		unsigned char flags = p[0];
		unsigned char segtype = p[1];
		int padding;
		int plen;
		int sequence;
		unsigned long time = 0;
		unsigned short duration = 0;

		int segs = 1;
		unsigned char segsizetype = 0x80;
		int seg = -1;


		p += 2;					// skip flags & segtype

		// Read packet size (plen):
		switch ((flags >> 5) & 3)
		{
		case 3:
			plen = LOAD_LE32(p);
			p += 4;
			break;				// dword
		case 2:
			plen = LOAD_LE16(p);
			p += 2;
			break;				// word
		case 1:
			plen = p[0];
			p++;
			break;				// byte
		default:
			plen = 0;
		}

		// Read sequence:
		switch ((flags >> 1) & 3)
		{
		case 3:
			sequence = LOAD_LE32(p);
			p += 4;
			break;				// dword
		case 2:
			sequence = LOAD_LE16(p);
			p += 2;
			break;				// word
		case 1:
			sequence = p[0];
			p++;
			break;				// byte
		default:
			sequence = 0;
		}

		// Read padding size (padding):
		switch ((flags >> 3) & 3)
		{
		case 3:
			padding = LOAD_LE32(p);
			p += 4;
			break;				// dword
		case 2:
			padding = LOAD_LE16(p);
			p += 2;
			break;				// word
		case 1:
			padding = p[0];
			p++;
			break;				// byte
		default:
			padding = 0;
		}

		if (((flags >> 5) & 3) != 0)
		{
			// Explicit (absoulte) packet size
			if (plen > asf_packetsize)
			{
			}
		}
		else
		{
			// Padding (relative) size
			plen = asf_packetsize - padding;
		}

		// Read time & duration:
		time = LOAD_LE32(p);
		p += 4;
		duration = LOAD_LE16(p);
		p += 2;

		// Read payload flags:
		if (flags & 1)
		{
			// multiple sub-packets
			segsizetype = p[0] >> 6;
			segs = p[0] & 0x3F;
			++p;
		}

		for (seg = 0; seg < segs; seg++)
		{
			//ASF_segmhdr_t* sh;
			unsigned char streamno;
			unsigned int seq;
			unsigned int x;		// offset or timestamp
			unsigned int rlen;

			//
			int len;
			unsigned int time2 = 0;
			int keyframe = 0;

			if (p >= p_end)
			{
			}

			/*
			   if(verbose>1){
			   int i;
			   MP_DPF("seg %d:",seg);
			   for(i=0;i<16;i++) MP_DPF(" %02X",p[i]);
			   MP_DPF("\n");
			   }
			 */

			streamno = p[0] & 0x7F;
			if (streamno == 0)
			{
				streamno = demux->video->id;
			}	
			
			if (p[0] & 0x80)
				keyframe = 1;
			p++;

			// Read media object number (seq):
			switch ((segtype >> 4) & 3)
			{
			    case 3:
				    seq = LOAD_LE32(p);
				    p += 4;
				    break;			// dword
			    case 2:
				    seq = LOAD_LE16(p);
				    p += 2;
				    break;			// word
			    case 1:
				    seq = p[0];
				    p++;
				    break;			// byte
			    default:
				    seq = 0;
			}

			// Read offset or timestamp:
			switch ((segtype >> 2) & 3)
			{
			    case 3:
				    x = LOAD_LE32(p);
				    p += 4;
				    break;			// dword
			    case 2:
				    x = LOAD_LE16(p);
				    p += 2;
				    break;			// word
			    case 1:
				    x = p[0];
				    p++;
				    break;			// byte
			    default:
				    x = 0;
			}

			// Read replic.data len:
			switch ((segtype) & 3)
			{
			    case 3:
				    rlen = LOAD_LE32(p);
				    p += 4;
				    break;			// dword
			    case 2:
				    rlen = LOAD_LE16(p);
				    p += 2;
				    break;			// word
			    case 1:
				    rlen = p[0];
				    p++;
				    break;			// byte
			    default:
				    rlen = 0;
			}

//        MP_DPF("### rlen=%d   \n",rlen);

			switch (rlen)
			{
			    case 0x01:			// 1 = special, means grouping
				    //MP_DPF("grouping: %02X  \n",p[0]);
				    ++p;			// skip PTS delta
				    break;
			    default:
				    if (rlen >= 8)
			   	    {
					    p += 4;		// skip object size
					    time2 = LOAD_LE32(p);	// read PTS
					    p += rlen - 4;
				    }
				    else
				    {
					    time2 = 0;	// unknown
					    p += rlen;
				    }
			}

			if (flags & 1)
			{
				// multiple segments
				switch (segsizetype)
				{
				    case 3:
					    len = LOAD_LE32(p);
					    p += 4;
					    break;		// dword
				    case 2:
					    len = LOAD_LE16(p);
					    p += 2;
					    break;		// word
				    case 1:
					    len = p[0];
					    p++;
					    break;		// byte
				    default:
					    len = plen - (p - asf_packet);	// ???
				}
			}
			else
			{
				// single segment
				len = plen - (p - asf_packet);
			}
			if (len < 0 || (p + len) > p_end)
			{
			}

			switch (rlen)
			{
			case 0x01:
				// GROUPING:
				//MP_DPF("ASF_parser: warning! grouping (flag=1) not yet supported!\n",len);
				//MP_DPF("  total: %d  \n",len);
				while (len > 0)
				{
					int len2 = p[0];

					p++;
					//MP_DPF("  group part: %d bytes\n",len2);
					if (!demux_asf_read_packet(demux, p, len2, streamno, seq, x, duration, -1, keyframe))
						return 0;
					p += len2;
					len -= len2 + 1;
					++seq;
				}
				if (len != 0)
				{
				}
				break;
			default:
				// NO GROUPING:
				//MP_DPF("fragment offset: %d  \n",sh->x);
				if (!demux_asf_read_packet(demux, p, len, streamno, seq, time2, duration, x, keyframe))
					return 0;
				p += len;
				break;
			}

		}						// for segs
		return 1;				// success
	}
	//return 0;
}

#include "stheader.h"

extern void resync_audio_stream(sh_audio_t * sh_audio);
extern void skip_audio_frame(sh_audio_t * sh_audio);

static int _seek(float rel_seek_secs, int flags)
{
	demux_stream_t *d_audio = demux->audio;
	demux_stream_t *d_video = demux->video;
	sh_audio_t *sh_audio = demux->audio == NULL ? NULL : d_audio->sh;

    if ((0 == rel_seek_secs) && (1 == flags)) // seek to movi_start
    {
        stream_seek(demux->stream, demux->movi_start);
	    demux->filepos = stream_tell(demux->stream);
		ds_fill_buffer(d_video);
		demux->filepos = stream_tell(demux->stream);
		stream_seek(demux->stream_a, demux->movi_start);
		demux->filepos = stream_tell(demux->stream_a);
		ds_fill_buffer(d_audio);
		demux->filepos = stream_tell(demux->stream_a);
		resync_audio_stream(sh_audio);
    }

#if 0	
//    sh_video_t *sh_video=d_video->sh;

	//FIXME: OFF_T - didn't test ASF case yet (don't have a large asf...)
	//FIXME: reports good or bad to steve@daviesfam.org please

	//================= seek in ASF ==========================
	float p_rate = asf_packetrate;	// packets / sec
	off_t rel_seek_packs = (flags & 2) ?	// FIXME: int may be enough?
		(rel_seek_secs * (demux->movi_end - demux->movi_start) / asf_packetsize) :
		(rel_seek_secs * p_rate);
	off_t rel_seek_bytes = rel_seek_packs * asf_packetsize;
	off_t newpos;

	//MP_DPF("ASF: packs: %d  duration: %d  \n",(int)fileh.packets,*((int*)&fileh.duration));
//    MP_DPF("ASF_seek: %d secs -> %d packs -> %d bytes  \n",
//       rel_seek_secs,rel_seek_packs,rel_seek_bytes);
	newpos = ((flags & 1) ? demux->movi_start : demux->filepos) + rel_seek_bytes;

	////MP_DEBUG1("-----------------asf_packetrate=", asf_packetrate, 8);
	////MP_DEBUG1("-----------------asf_packetsize=", asf_packetsize, 8);
	////MP_DEBUG1("-----------------rel_seek_packs=", rel_seek_packs, 8);
	////MP_DEBUG1("-----------------rel_seek_secs=", rel_seek_secs, 8);

	////MP_DEBUG1("-----------------rel_seek_bytes=", rel_seek_bytes, 8);
	//MP_DEBUG1("----------seek:-------demux->filepos=", demux->filepos, 8);
	//MP_DEBUG1("----------seek:-------newpos=", newpos, 8);
////MP_DEBUG1("-----------------demux->movi_start=", demux->movi_start, 8);
////MP_DEBUG1("-----------------demux->movi_end=", demux->movi_end, 8);

	if (newpos < 0 || newpos < demux->movi_start)
		newpos = demux->movi_start;
    MP_DEBUG1("-- asf: newpos=%d --",newpos);
	stream_seek(demux->stream, newpos);
	MP_DEBUG1("sh_audio %08x", sh_audio);

	ds_fill_buffer(d_video);
	if (sh_audio)
	{
		ds_fill_buffer(d_audio);
		resync_audio_stream(sh_audio);  // Deming commneted. Uncomment it later
	}

	int iRetry = 100;
	while (iRetry--)
	{
		if (sh_audio && !d_audio->eof && d_video->pts && d_audio->pts)
		{
			float a_pts = d_audio->pts;

			a_pts += (ds_tell_pts(d_audio) - sh_audio->a_in_buffer_len) / (float) sh_audio->i_bps;
			// sync audio:
			if (d_video->pts > a_pts)
			{
				//   skip_audio_frame(sh_audio); // Deming commneted. Uncomment it later
//        if(!ds_fill_buffer(d_audio)) sh_audio=NULL; // skip audio. EOF?
				continue;
			}
		}
		if (d_video->flags & 1)
			break;				// found a keyframe!
		if (!ds_fill_buffer(d_video))
			break;				// skip frame.  EOF?
	}

//MP_DEBUG1("--------seek---------demux->filepos=", demux->filepos, 8);
#endif
	return 1;
}

static int _control(int cmd, void *arg)
{

	switch (cmd)
	{
	case DEMUXER_CTRL_GET_TIME_LENGTH:
		*((unsigned long *) arg) = (unsigned long) (asf_movielength);
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_GET_PERCENT_POS:
		if (demux->movi_end == demux->movi_start)
		{
			return DEMUXER_CTRL_DONTKNOW;
		}
		*((int *) arg) =
			(int) ((demux->filepos -
					demux->movi_start) / ((demux->movi_end - demux->movi_start) / 100));
		return DEMUXER_CTRL_OK;

	case DEMUXER_CTRL_IF_PTS_FROM_STREAM:
		*((int*)arg) = 1;//Liwu: It seems to be
		return DEMUXER_CTRL_OK;
		
	default:
		return DEMUXER_CTRL_NOTIMPL;
	}
}



static int _check_type(stream_t * stream, DEMUX_T demux_type)
{
//char tmp[16];
//char *tmp1 = (char*)malloc(16);
	if ((demux_type == DEMUX_UNKNOWN) || (demux_type == DEMUX_ASF))
	{
//MP_DEBUG("asf file\r\n");
		stream_reset(stream);
		stream_seek(stream, stream->start_pos);
		demux->stream = stream;

		if (asf_check_header(demux))
		{

//MP_DEBUG("asf_check_header return 1\r\n");
//DebugBreak();

			return 1;
		}
		else
		{
//MP_DEBUG("asf_check_header return 0\r\n");
//DebugBreak();

			return (demux_type == DEMUX_UNKNOWN) ? 0 : -1;
		}
	}
	//MP_DEBUG("not asf\r\n");
	return 0;
}

static int _open()
{
	//---- ASF header:
	MP_DEBUG("open asf");

	if (!read_asf_header(demux))
		return 0;

	MP_DEBUG("read_asf_header OK");

	stream_reset(demux->stream);
	stream_seek(demux->stream, demux->movi_start);

	if (demux->video->id != -2)
	{
		//MP_DEBUG("before ds_fill_buffer");

		if (ds_fill_buffer(demux->video))
		{
			sh_video_t *sh_video = demux->video->sh;

			sh_video->ds = demux->video;
			sh_video->fps = 1000.0f;
			sh_video->frametime = 0.001f;	// 1ms
		} else
			return 0;

		//MP_DEBUG("after ds_fill_buffer\r\n");
	}

	if (demux->audio->id != -2 && demux->audio->sh)
	{
		if (ds_fill_buffer(demux->audio))
		{
			sh_audio_t *sh_audio = demux->audio->sh;
			if (sh_audio != NULL) {
				sh_audio->ds = demux->audio;
				sh_audio->format = sh_audio->wf->wFormatTag;
			}
		}
	}
	return 1;
}

static int _close()
{
	ASF_PRIV_T *priv = demux->priv;

	if (!priv)
		return 1;

	if (priv->idx != NULL)
		mem_free(priv->idx);
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


	switch (flags)
	{
	case 2:
		{
			pts = (float) demux->filepos / (float) asf_packetsize / asf_packetrate;

			////MP_DEBUG1("-----------------asf_packetrate=", asf_packetrate, 8);
//  //MP_DEBUG1("-----------------asf_packetsize=", asf_packetsize, 8);
			//MP_DEBUG1("-----------------demux->filepos=", demux->filepos, 8);

////MP_DEBUG1("!!!!!!!!!!rel_seek_samples=", rel_seek_samples, 8);
			//MP_DEBUG1("!!!!!!!!!!pts1=", pts, 8);
			pts += rel_seek_samples;

			if (pts < 0)
				pts = 0;
			else if (pts > ((float) demux->movi_end / (float) asf_packetsize / asf_packetrate))
				pts = (float) demux->movi_end / (float) asf_packetsize / asf_packetrate;

			break;
		}

	case 0:
		{
			pts = 0;


			break;
		}

	case 1:
		{
			pts = (float) demux->movi_end / (float) asf_packetsize / asf_packetrate;


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
//command=3: get asf file content
//return=0:  not fill the buffer
//return=1:  fill the buffer
static int _get_movie_info(BYTE * buffer, int command)
{
	BYTE *string;
	off_t t1, t2;
	int bits;


	switch (command)
	{
	case 0:
		{

			t1 = t2 = demux->stream->file_len;
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


			t1 = t2 = demux->stream->pos;
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



			t1 = t2 = demux->stream->pos * 100 / demux->stream->file_len;
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
		//get asf content , jackyang 20050607
	case 3:
		{
			/* Meng 2005.6.7 Updated starts */

			PMedia_Info pASFInfo = (PMedia_Info) buffer;
			sh_video_t *sh_video = (sh_video_t *) demux->sh_video;
			sh_audio_t *sh_audio = (sh_audio_t *) demux->sh_audio;

			if (sh_audio)
			{
                GetAudioCodecCategory(pASFInfo->cAudioCodec);
				pASFInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_AUDIO;

				if ((pASFInfo->dwSampleRate = sh_audio->samplerate) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_SampleRate_USEFUL;

				if ((pASFInfo->dwSampleSize = sh_audio->samplesize) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_SampleSize_USEFUL;

				if ((pASFInfo->dwBitrate = sh_audio->i_bps) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_Bitrate_USEFUL;

				if ((pASFInfo->dwTotalTime = demux->stream->file_len / sh_audio->i_bps) >= 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
			}

			if (sh_video)
			{
			    GetVideoCodecCategory(pASFInfo->cVidoeCodec);
				pASFInfo->dwFlags |= (DWORD) MOVIE_INFO_WITH_VIDEO;

				if ((pASFInfo->dwFrameRate = sh_video->fps) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_FrameRate_USEFUL;

				if ((pASFInfo->dwImageHeight = sh_video->disp_h) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_ImageHeight_USEFUL;

				if ((pASFInfo->dwImageWidth = sh_video->disp_w) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_ImageWidth_USEFUL;

				if ((pASFInfo->dwTotalFrame = asf_packetsize * asf_packetrate) > 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_TotalFrame_USEFUL;

				if ((pASFInfo->dwTotalTime = asf_movielength) >= 0)
					pASFInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;

				if (strlen(asfcontent.szAuthor) > 0)
				{
					memcpy(pASFInfo->contentinfo.szAuthor, asfcontent.szAuthor, MAX_AUTHOR_LEN);
					pASFInfo->dwFlags |= (DWORD) MOVIE_TotalTime_USEFUL;
				}
				if (strlen(asfcontent.szCopyright) > 0)
				{
					memcpy(pASFInfo->contentinfo.szCopyright, asfcontent.szCopyright,
						   MAX_COPYRIGHT_LEN);
					pASFInfo->dwFlags |= (DWORD) MOVIE_Copyright_USEFUL;
				}
				if (strlen(asfcontent.szDescription) > 0)
				{
					memcpy(pASFInfo->contentinfo.szDescription, asfcontent.szDescription,
						   MAX_DESCRIP_LEN);
					pASFInfo->dwFlags |= (DWORD) MOVIE_Description_USEFUL;
				}
				if (strlen(asfcontent.szRating) > 0)
				{
					memcpy(pASFInfo->contentinfo.szRating, asfcontent.szRating, MAX_RATING_LEN);
					pASFInfo->dwFlags |= (DWORD) MOVIE_Rating_USEFUL;
				}
				if (strlen(asfcontent.szTitle) > 0)
				{
					memcpy(pASFInfo->contentinfo.szTitle, asfcontent.szTitle, MAX_TITLE_LEN);
					pASFInfo->dwFlags |= (DWORD) MOVIE_Title_USEFUL;
				}
			}



			return 1;

			/* Meng 2005.6.7 Updated ends */
		}
		//add end jackyang 20050607 
	default:
		break;
	}
	return 0;
}


demuxer_t *new_asf_demux()
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
	demux->type = DEMUXER_TYPE_ASF;
	demux->file_format = DEMUXER_TYPE_ASF;
	return demux;
}

#endif
