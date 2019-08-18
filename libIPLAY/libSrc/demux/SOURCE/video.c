/*
 * video frame reading
 *
 */
 #include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

#define LOCAL_DEBUG_ENABLE 0
#define MP_PERFORMANCE_TRACE 0

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mpTrace.h"
#include "app_interface.h"
#include "stream.h"
#include "demux_stream.h"
#include "demux.h"
#include "parse_es.h"
#include "mpeg_hdr.h"
#include "stheader.h"
#include "video_decoder.h"

//#include "mp_msg.h"
//#include "help_mp.h"

#include "parse_es.h"
#include "mpeg_hdr.h"
#include "mpeg/demux_mpg.h"

/* sub_cc (closed captions)*/
//#include "sub_cc.h"

/* biCompression constant */
#define BI_RGB        0L

#ifdef CONFIG_LIVE555
#include "demux_rtp.h"
#endif

#ifdef	mp_msg
#undef	mp_msg
#define mp_msg(mod, lev, ...)	do{MP_DEBUGN(__VA_ARGS__);mpDebugPrintN("\r");}while(0)
#define mp_dbg(mod, lev, ...)	do{MP_DEBUGN(__VA_ARGS__);mpDebugPrintN("\r");}while(0)
#define	mp_msg_test(MSGT_DECVIDEO, MSGL_V)	0
#define MSGTR_MpegNoSequHdr "MPEG: FATAL: EOF while searching for sequence header.\n"
#define MSGTR_CannotReadMpegSequHdr "FATAL: Cannot read sequence header.\n"
#define MSGTR_CannotReadMpegSequHdrEx "FATAL: Cannot read sequence header extension.\n"
#define MSGTR_BadMpegSequHdr "MPEG: bad sequence header\n"
#define MSGTR_BadMpegSequHdrEx "MPEG: bad sequence header extension\n"

#define MSGTR_ShMemAllocFail "Cannot allocate shared memory.\n"
#define MSGTR_CantAllocAudioBuf "Cannot allocate audio out buffer.\n"
#define MSGTR_LeaveTelecineMode "\ndemux_mpg: 30000/1001fps NTSC content detected, switching framerate.\n"
#define MSGTR_EnterTelecineMode "\ndemux_mpg: 24000/1001fps progressive NTSC content detected, switching framerate.\n"


#endif


static mp_mpeg_header_t picture;

static int telecine=0;
static float telecine_cnt=-2.5;

typedef enum
{
	VIDEO_MPEG12,
	VIDEO_MPEG4,
	VIDEO_H264,
	VIDEO_VC1,
	VIDEO_OTHER
} video_codec_t;

void video_free_buffer()
{
	if (videobuffer0)
	{
		mem_free(videobuffer0);
		videobuffer0 = NULL;
	}
	if (videobuffer1)
	{
		mem_free(videobuffer1);
		videobuffer1 = NULL;
	}
	videobuffer = NULL;
}

static int video_alloc_buffer(const unsigned int number)
{
	if (!videobuffer0)
	{
		videobuffer0=(char*)mem_malloc(VIDEOBUFFER_SIZE + MP_INPUT_BUFFER_PADDING_SIZE);
		if (videobuffer0) memset(videobuffer0+VIDEOBUFFER_SIZE, 0, MP_INPUT_BUFFER_PADDING_SIZE);
		else
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_ShMemAllocFail);
			return -1;
		}
	}
	if (number > 1 && !videobuffer1)
	{
		videobuffer1=(char*)mem_malloc(VIDEOBUFFER_SIZE + MP_INPUT_BUFFER_PADDING_SIZE);
		if (videobuffer1) memset(videobuffer1+VIDEOBUFFER_SIZE, 0, MP_INPUT_BUFFER_PADDING_SIZE);
		else
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_ShMemAllocFail);
			return -1;
		}
	}
	videobuffer = videobuffer0;
	return 0;
}

void swap_videobuffer()
{
	if (!videobuffer1)
		return;
	if (videobuffer == videobuffer0)
		videobuffer = videobuffer1;
	else
		videobuffer = videobuffer0;
	//mpDebugPrint("%s: videobuffer=0x%x from(%x, %x)", __FUNCTION__, videobuffer, videobuffer0, videobuffer1);
}

static video_codec_t find_video_codec(const sh_video_t * const sh_video)
{
	const demux_stream_t * const d_video=sh_video->ds;
	const int fmt = d_video->demuxer->file_format;

	if (
	    (fmt == DEMUXER_TYPE_PVA) ||
	    (fmt == DEMUXER_TYPE_MPEG_ES) ||
	    (fmt == DEMUXER_TYPE_MPEG_GXF) ||
	    (fmt == DEMUXER_TYPE_MPEG_PES) ||
	    (
	        (fmt == DEMUXER_TYPE_MPEG_PS || fmt == DEMUXER_TYPE_MPEG_TS) &&
	        ((! sh_video->format) || (sh_video->format==VCODEC_MPEG1) || (sh_video->format==VCODEC_MPEG2))
	    ) ||
	    (fmt == DEMUXER_TYPE_MPEG_TY)
#ifdef CONFIG_LIVE555
	    || ((fmt == DEMUXER_TYPE_RTP) && demux_is_mpeg_rtp_stream(d_video->demuxer))
#endif
	)
		return VIDEO_MPEG12;
	else if ((fmt == DEMUXER_TYPE_MPEG4_ES) ||
	         ((fmt == DEMUXER_TYPE_MPEG_TS) && (sh_video->format==VCODEC_MPEG4)) ||
	         ((fmt == DEMUXER_TYPE_MPEG_PS) && (sh_video->format==VCODEC_MPEG4))
	        )
		return VIDEO_MPEG4;
	else if ((fmt == DEMUXER_TYPE_H264_ES) ||
	         ((fmt == DEMUXER_TYPE_MPEG_TS) && (H264_FOURCC(sh_video->format) || sh_video->format==VCODEC_H264)) ||
	         ((fmt == DEMUXER_TYPE_MPEG_PS) && (H264_FOURCC(sh_video->format) || sh_video->format==VCODEC_H264))
	        )
		return VIDEO_H264;
	else if ((fmt == DEMUXER_TYPE_MPEG_PS ||  fmt == DEMUXER_TYPE_MPEG_TS) &&
	         (sh_video->format==mmioFOURCC('W', 'V', 'C', '1')))
		return VIDEO_VC1;
	else if (fmt == DEMUXER_TYPE_ASF && sh_video->bih && sh_video->bih->biCompression == mmioFOURCC('D', 'V', 'R', ' '))
		return VIDEO_MPEG12;
	else
		return VIDEO_OTHER;
}

int video_read_properties(sh_video_t * const sh_video)
{
	demux_stream_t * const d_video=sh_video->ds;
	const video_codec_t video_codec = find_video_codec(sh_video);
	demuxer_t * const demuxer = d_video->demuxer;
// Determine image properties:
	switch (video_codec)
	{
	case VIDEO_OTHER:
	{
		//mpDebugPrint("%s: %d", __FUNCTION__, __LINE__);
		if ((d_video->demuxer->file_format == DEMUXER_TYPE_ASF) || (d_video->demuxer->file_format == DEMUXER_TYPE_AVI))
		{
			// display info:
			// in case no strf chunk has been seen in avi, we have no bitmap header
			if (!sh_video->bih) return 0;
			sh_video->format=sh_video->bih->biCompression;
			sh_video->disp_w=sh_video->bih->biWidth;
			sh_video->disp_h=abs(sh_video->bih->biHeight);
		}
		break;
	}
	case VIDEO_MPEG4:
	{
		//mpDebugPrint("%s: %d", __FUNCTION__, __LINE__);
		#if VIDEO_ENABLE
		int pos = 0, vop_cnt=0, units[3];
		videobuf_len=0;
		videobuf_code_len=0;
		mp_msg(MSGT_DECVIDEO,MSGL_V,"Searching for Video Object Start code... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			if (i<=0x11F) break; // found it!
			if (!i || !skip_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\n");
		if (video_alloc_buffer(1))
			return 0;
		
		mp_msg(MSGT_DECVIDEO,MSGL_V,"Searching for Video Object Layer Start code... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			mp_msg(MSGT_DECVIDEO,MSGL_V,"M4V: 0x%X\n",i);
			if (i>=0x120 && i<=0x12F) break; // found it!
			if (!i || !read_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		pos = videobuf_len+4;
		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Can't read Video Object Layer Header\n");
			return 0;
		}
		mp4_header_process_vol(&picture, &(videobuffer[pos]));
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK! FPS SEEMS TO BE %d/1000\nSearching for Video Object Plane Start code... ", FLOAT1000(sh_video->fps));
mp4_init:
		while (1)
		{
			int i=sync_video_packet(d_video);
			if (i==MPEG4_VOP_START_CODE) break; // found it!
			if (!i || !read_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		pos = videobuf_len+4;
		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Can't read Video Object Plane Header\n");
			return 0;
		}
		mp4_header_process_vop(&picture, &(videobuffer[pos]));
		sh_video->disp_w = picture.display_picture_width;
		sh_video->disp_h = picture.display_picture_height;
		units[vop_cnt] = picture.timeinc_unit;
		vop_cnt++;
		//mp_msg(MSGT_DECVIDEO,MSGL_V, "TYPE: %d, unit: %d\n", picture.picture_type, picture.timeinc_unit);
		if (!picture.fps)
		{
			int i, mn, md, mx, diff;
			if (vop_cnt < 3)
				goto mp4_init;

			i=0;
			mn = mx = units[0];
			for (i=0; i<3; i++)
			{
				if (units[i] < mn)
					mn = units[i];
				if (units[i] > mx)
					mx = units[i];
			}
			md = mn;
			for (i=0; i<3; i++)
			{
				if ((units[i] > mn) && (units[i] < mx))
					md = units[i];
			}
			mp_msg(MSGT_DECVIDEO,MSGL_V, "MIN: %d, mid: %d, max: %d\n", mn, md, mx);
			if (mx - md > md - mn)
				diff = md - mn;
			else
				diff = mx - md;
			if (diff > 0)
			{
				picture.fps = ((float)picture.timeinc_resolution) / diff;
				mp_msg(MSGT_DECVIDEO,MSGL_V, "FPS seems to be: %d/1000, resolution: %d, delta_units: %d\n", FLOAT1000(picture.fps), picture.timeinc_resolution, diff);
			}
		}
		if (picture.fps)
		{
			sh_video->fps=picture.fps;
			sh_video->frametime=1.0/picture.fps;
			mp_msg(MSGT_DECVIDEO,MSGL_INFO, "FPS seems to be: %d/1000\n", FLOAT1000(picture.fps));
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\n");
		sh_video->format=VCODEC_MPEG4;
		#endif
		break;
	}
	case VIDEO_H264:
	{
		int pos = 0;
		#if VIDEO_ENABLE
		videobuf_len=0;
		videobuf_code_len=0;
		mp_msg(MSGT_DECVIDEO,MSGL_V,"Searching for sequence parameter set... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			if ((i&~0x60) == 0x107 && i != 0x107) break; // found it!
			if (!i || !skip_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\n");
		if (video_alloc_buffer(1))
			return 0;
		
		pos = videobuf_len+4;
		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Can't read sequence parameter set\n");
			return 0;
		}
		h264_parse_sps(&picture, &(videobuffer[pos]), videobuf_len - pos);
		sh_video->disp_w=picture.display_picture_width;
		sh_video->disp_h=picture.display_picture_height;
		mp_msg(MSGT_DECVIDEO,MSGL_V,"Searching for picture parameter set... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			mp_msg(MSGT_DECVIDEO,MSGL_V,"H264: 0x%X\n",i);
			if ((i&~0x60) == 0x108 && i != 0x108) break; // found it!
			if (!i || !read_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\nSearching for Slice... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			if ((i&~0x60) == 0x101 || (i&~0x60) == 0x102 || (i&~0x60) == 0x105) break; // found it!
			if (!i || !read_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\n");
		sh_video->format=VCODEC_H264;
		if (picture.fps)
		{
			sh_video->fps=picture.fps;
			sh_video->frametime=1.0/picture.fps;
			mp_msg(MSGT_DECVIDEO,MSGL_INFO, "FPS seems to be: %d/1000\n", FLOAT1000(picture.fps));
		}
		#endif
		break;
	}
	case VIDEO_MPEG12:
	{
		#if VIDEO_ENABLE
		if (d_video->demuxer->file_format == DEMUXER_TYPE_ASF)   // DVR-MS
		{
			if (!sh_video->bih) return 0;
			sh_video->format=sh_video->bih->biCompression;
		}
mpeg_header_parser:
		// Find sequence_header first:
		videobuf_len=0;
		videobuf_code_len=0;
		telecine=0;
		telecine_cnt=-2.5;
		mp_msg(MSGT_DECVIDEO,MSGL_V,"Searching for sequence header... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			if (i==MPEG_DATA_SEQUENCE_START_CODE) break; // found it!
			if (!i || !skip_video_packet(d_video))
			{
				if ( mp_msg_test(MSGT_DECVIDEO,MSGL_V) )  mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_MpegNoSequHdr);
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_V,"OK!\n");
		// ========= Read & process sequence header & extension ============
		if (video_alloc_buffer(2))
			return 0;

		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_CannotReadMpegSequHdr);
			return 0;
		}
		if (mp_header_process_sequence_header (&picture, &videobuffer[4]))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_BadMpegSequHdr);
			goto mpeg_header_parser;
		}
		if (sync_video_packet(d_video)==MPEG_DATA_EXTENSION_START_CODE) // next packet is seq. ext.
		{
			int pos=videobuf_len;
			if (!read_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_CannotReadMpegSequHdrEx);
				return 0;
			}
			if (mp_header_process_extension (&picture, &videobuffer[pos+4]))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_ERR,MSGTR_BadMpegSequHdrEx);
				return 0;
			}
		}
		// fill aspect info:
		sh_video->aspect = mpeg12_aspect_info(&picture);

		// display info:
		sh_video->format=picture.mpeg1?VCODEC_MPEG1:VCODEC_MPEG2; // mpeg video
		sh_video->fps=picture.fps * picture.frame_rate_extension_n / picture.frame_rate_extension_d;
		if (!sh_video->fps)
		{
			sh_video->frametime=0;
		}
		else
		{
			sh_video->frametime=1.0/sh_video->fps;
		}
		sh_video->disp_w=picture.display_picture_width;
		sh_video->disp_h=picture.display_picture_height;
		// bitrate:
		if (picture.bitrate!=0x3FFFF) // unspecified/VBR ?
			sh_video->i_bps=picture.bitrate * 400 / 8;
		else// unspecified/VBR ?
		{
			mpDebugPrint("calculate video bps");
			stream_t * const stream = demuxer->stream;
			mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
			sh_video->i_bps = stream_get_file_size(stream)/mpg_d->final_pts;
		}
		// info:
		mp_dbg(MSGT_DECVIDEO,MSGL_DBG2,"mpeg bitrate: %d (%X)\n",picture.bitrate,picture.bitrate);
		mp_msg(MSGT_DECVIDEO,MSGL_INFO,"VIDEO:  %s  %dx%d  (aspect %d)  %d/1000 fps  %d bps (%d byte/s)\n",
		       picture.mpeg1?"MPEG1":"MPEG2",
		       sh_video->disp_w,sh_video->disp_h,
		       picture.aspect_ratio_information,
		       FLOAT1000(sh_video->fps),
		       sh_video->i_bps * 8,
		       sh_video->i_bps);
		#endif
		break;
	}
	case VIDEO_VC1:
	{
		// Find sequence_header:
		#if VIDEO_ENABLE
		videobuf_len=0;
		videobuf_code_len=0;
		mp_msg(MSGT_DECVIDEO,MSGL_INFO,"Searching for VC1 sequence header... ");
		while (1)
		{
			int i=sync_video_packet(d_video);
			if (i==0x10F) break; // found it!
			if (!i || !skip_video_packet(d_video))
			{
				if ( mp_msg_test(MSGT_DECVIDEO,MSGL_V) )  mp_msg(MSGT_DECVIDEO,MSGL_V,"NONE :(\n");
				mp_msg(MSGT_DECVIDEO,MSGL_ERR, "Couldn't find VC-1 sequence header\n");
				return 0;
			}
		}
		mp_msg(MSGT_DECVIDEO,MSGL_INFO,"found\n");
		if (video_alloc_buffer(1))
			return 0;
		
		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_ERR, "Couldn't read VC-1 sequence header!\n");
			return 0;
		}

		while (1)
		{
			int i=sync_video_packet(d_video);
			if (i==0x10E) break; // found it!
			if (!i || !skip_video_packet(d_video))
			{
				mp_msg(MSGT_DECVIDEO,MSGL_V,"Couldn't find VC-1 entry point sync-code:(\n");
				return 0;
			}
		}
		if (!read_video_packet(d_video))
		{
			mp_msg(MSGT_DECVIDEO,MSGL_V,"Couldn't read VC-1 entry point sync-code:(\n");
			return 0;
		}

		if (mp_vc1_decode_sequence_header(&picture, &videobuffer[4], videobuf_len-4))
		{
			sh_video->bih = (BITMAPINFOHEADER*)mem_calloc(1, sizeof(BITMAPINFOHEADER) + videobuf_len);
			if (sh_video->bih == NULL)
			{
				mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Couldn't alloc %d bytes for VC-1 extradata!\n", sizeof(BITMAPINFOHEADER) + videobuf_len);
				return 0;
			}
			sh_video->bih->biSize= sizeof(BITMAPINFOHEADER) + videobuf_len;
			memcpy(sh_video->bih + 1, videobuffer, videobuf_len);
			sh_video->bih->biCompression = sh_video->format;
			sh_video->bih->biWidth = sh_video->disp_w = picture.display_picture_width;
			sh_video->bih->biHeight = sh_video->disp_h = picture.display_picture_height;
			if (picture.fps > 0)
			{
				sh_video->frametime=1.0/picture.fps;
				sh_video->fps = picture.fps;
			}
			mp_msg(MSGT_DECVIDEO,MSGL_INFO,"VIDEO:  VC-1  %dx%d, %d/1000 fps, header len: %d\n",
			       sh_video->disp_w, sh_video->disp_h, FLOAT1000(sh_video->fps), videobuf_len);
		}
		#endif
		break;
	}
	} // switch(file_format)

	return 1;
}

static void process_userdata(unsigned char* buf,int len)
{
	/*
		int i;
		// if the user data starts with "CC", assume it is a CC info packet
		if (len>2 && buf[0]=='C' && buf[1]=='C')
		{
	//    mp_msg(MSGT_DECVIDEO,MSGL_DBG2,"video.c: process_userdata() detected Closed Captions!\n");
			subcc_process_data(buf+2,len-2);
		}
		if ( len > 2 && buf[ 0 ] == 'T' && buf[ 1 ] == 'Y' )
		{
			ty_processuserdata( buf + 2, len - 2 );
			return;
		}
	//    if(verbose<2) return;
		MP_ALERT("user_data: len=%3d  %02X %02X %02X %02X",
		         len, buf[0], buf[1], buf[2], buf[3]);
		for (i=0; i<len; i++)
	//    if(buf[i]>=32 && buf[i]<127) fputc(buf[i], stderr);
			if (buf[i]&0x60) MP_ALERT("%c", buf[i]&0x7F);
	*/
}

int video_read_frame(sh_video_t* const sh_video, float* const frame_time_ptr, unsigned char** start, const int force_fps)
{
#if VIDEO_ON
	demux_stream_t * const d_video=sh_video->ds;
	demuxer_t * const demuxer=d_video->demuxer;
	float frame_time=1;
	const float pts1 = d_video->pts;
	float pts=0;
	float fps;
	int picture_coding_type=0;
	int in_size=0;
	const video_codec_t video_codec = find_video_codec(sh_video);
	const int eof = -1;

	*start=NULL;
#if VIDEO_ENABLE
	if (video_codec == VIDEO_MPEG12)
	{		
		int in_frame=0;
		mpg_demuxer_t * const mpg_d=(mpg_demuxer_t*)demuxer->priv;
		const int has_vpacket_header = mpg_d->has_vpacket_header;
		*start = NULL;
		mpg_d->no_pts_after_vframes++;
		mpg_d->has_vpacket_header = 0;
		mpg_d->curvframe_vpts = mpg_d->nextvframe_vpts;
		mpg_d->nextvframe_vpts = 0;


		//mpDebugPrint("%s : %d", __FUNCTION__, __LINE__);
		while (videobuf_len<VIDEOBUFFER_SIZE-MAX_VIDEO_PACKET_SIZE)
		{
			int i=sync_video_packet(d_video);
			//void* buffer=&videobuffer[videobuf_len+4];
			int payload=videobuf_len+4;
			if (in_frame)
			{
				if (i<0x101 || i>=0x1B0)  // not slice code -> end of frame
				{
					MP_DEBUG("not slice code -> end of frame, ds->pts has been updated to next frame");
					//assign pts for this frame
					if (!mpg_d->no_vpts)
					{
						//mpDebugPrint("update pts");
						if (!has_vpacket_header)
						{
							if (mpg_d->nextvframe_vpts)
							{
								d_video->pts = mpg_d->last2_vpts;
								d_video->current->pts = mpg_d->last2_vpts;
							}
							else
							{
								d_video->current->pts = mpg_d->last_vpts;
								d_video->pts = mpg_d->last_vpts;
							}
						}
						else if (mpg_d->curvframe_vpts)
						{
							d_video->pts = mpg_d->curvframe_vpts;
							d_video->current->pts = mpg_d->curvframe_vpts;
						}
						MP_DEBUG("has_vpacket_header=%d, nextvframe_vpts=%d, curvframe_vpts=%d, pts=%d, while last12_vpts=(%d, %d)", has_vpacket_header,FLOAT1000(mpg_d->nextvframe_vpts),FLOAT1000(mpg_d->curvframe_vpts),FLOAT1000(d_video->current->pts),FLOAT1000(mpg_d->last_vpts),FLOAT1000(mpg_d->last2_vpts));
					}
					if (!i)	return eof;	// EOF
					break;
				}
			}
			else
			{
				if (i == MPEG_DATA_PICTURE_START_CODE)
				{
					pts=d_video->pts;
					d_video->pts=0;
				}
				if (i > MPEG_DATA_PICTURE_START_CODE && i < 0x1B0)
					in_frame = 1;	// slice_start_code (01~AF)
				else if (!i)
					return eof;		// EOF
			}
			if (!read_video_packet(d_video)) return eof; // EOF
			// process headers:
			switch (i)
			{
			case MPEG_DATA_SEQUENCE_START_CODE://sequence_header_code
				mp_header_process_sequence_header(&picture, &videobuffer[payload]);
				break;
			case MPEG_DATA_EXTENSION_START_CODE://extension_start_code
				mp_header_process_extension(&picture, &videobuffer[payload]);
				break;
			case MPEG_DATA_USER_DATA_START_CODE://user_data_start_code
				process_userdata(&videobuffer[payload], videobuf_len - payload);
				break;
			case MPEG_DATA_PICTURE_START_CODE://picture_start_code
				picture_coding_type = (videobuffer[payload + 1] >> 3) & 7;
				break;
			}
		}
		fps = picture.fps * picture.frame_rate_extension_n / picture.frame_rate_extension_d;

		*start=videobuffer;
		in_size=videobuf_len;

		// get mpeg fps:
		if (sh_video->fps!=fps) if (!force_fps && !telecine)
			{
				mp_msg(MSGT_CPLAYER,MSGL_WARN,"Warning! FPS changed %d/1000 -> %d/1000  (%d/1000) [%d]  \n", FLOAT1000(sh_video->fps), FLOAT1000(fps), FLOAT1000(sh_video->fps-fps), picture.frame_rate_code);
				sh_video->fps=fps;
				sh_video->frametime=1.0/fps;
			}

		// fix mpeg2 frametime:
		frame_time=(picture.display_time)*0.01f;
		picture.display_time=100;
		videobuf_len=0;

		telecine_cnt*=0.9; // drift out error
		telecine_cnt+=frame_time-5.0/4.0;
		mp_msg(MSGT_DECVIDEO,MSGL_DBG2,"\r telecine = %d  %d     \n",FLOAT1000(frame_time), FLOAT1000(telecine_cnt));

		if (telecine)
		{
			frame_time=1;
			if (telecine_cnt<-1.5 || telecine_cnt>1.5)
			{
				mp_msg(MSGT_DECVIDEO,MSGL_INFO,MSGTR_LeaveTelecineMode);
				telecine=0;
			}
		}
		else if (telecine_cnt>-0.5 && telecine_cnt<0.5 && !force_fps)
		{
			sh_video->fps=sh_video->fps*4/5;
			sh_video->frametime=sh_video->frametime*5/4;
			mp_msg(MSGT_DECVIDEO,MSGL_INFO,MSGTR_EnterTelecineMode);
			telecine=1;
		}
	}
	else if (video_codec == VIDEO_MPEG4)
	{
		//mpDebugPrint("%s: %d", __FUNCTION__, __LINE__);
		while (videobuf_len<VIDEOBUFFER_SIZE-MAX_VIDEO_PACKET_SIZE)
		{
			int i=sync_video_packet(d_video);
			if (!i) return -1;
			if (!read_video_packet(d_video)) return eof; // EOF
			if (i==MPEG4_VOP_START_CODE) break;
		}
		*start=videobuffer;
		in_size=videobuf_len;
		videobuf_len=0;
	}
	else if (video_codec == VIDEO_H264)
	{
		//mpDebugPrint("%s @ %d", __FUNCTION__, __LINE__);
		int in_picture = 0;
		while (videobuf_len<VIDEOBUFFER_SIZE-MAX_VIDEO_PACKET_SIZE)
		{
			int i=sync_video_packet(d_video);
			int pos = videobuf_len+4;
			if (!i) return -1;
			if (!read_video_packet(d_video)) return eof; // EOF
			if ((i&~0x60) == 0x107 && i != 0x107)
			{
				h264_parse_sps(&picture, &(videobuffer[pos]), videobuf_len - pos);
				if (picture.fps > 0)
				{
					sh_video->fps=picture.fps;
					sh_video->frametime=1.0/picture.fps;
				}
				i=sync_video_packet(d_video);
				if (!i) return -1;
				if (!read_video_packet(d_video)) return eof; // EOF
			}

			// here starts the access unit end detection code
			// see the mail on MPlayer-dev-eng for details:
			// Date: Sat, 17 Sep 2005 11:24:06 +0200
			// Subject: Re: [MPlayer-dev-eng] [RFC] h264 ES parser problems
			// Message-ID: <20050917092406.GA7699@rz.uni-karlsruhe.de>
			if ((i&~0x60) == 0x101 || (i&~0x60) == 0x102 || (i&~0x60) == 0x105)
				// found VCL NAL with slice header i.e. start of current primary coded
				// picture, so start scanning for the end now
				in_picture = 1;
			if (in_picture)
			{
				i = sync_video_packet(d_video) & ~0x60; // code of next packet
				if (i == 0x106 || i == 0x109) break; // SEI or access unit delim.
				if (i == 0x101 || i == 0x102 || i == 0x105)
				{
					// assuming arbitrary slice ordering is not allowed, the
					// first_mb_in_slice (golomb encoded) value should be 0 then
					// for the first VCL NAL in a picture
					if (demux_peekc(d_video) & 0x80)
						break;
				}
			}
		}
		*start=videobuffer;
		in_size=videobuf_len;
		videobuf_len=0;
	}
	else if (video_codec == VIDEO_VC1)
	{
		while (videobuf_len<VIDEOBUFFER_SIZE-MAX_VIDEO_PACKET_SIZE)
		{
			int i=sync_video_packet(d_video);
			if (!i) return -1;
			if (!read_video_packet(d_video)) return eof; // EOF
			if (i==0x10D) break;
		}
		*start=videobuffer;
		in_size=videobuf_len;
		videobuf_len=0;
	}
	else
#endif		
	{
		MP_DEBUG("%s @ %d", __FUNCTION__, __LINE__);
		// frame-based file formats: (AVI,ASF,MOV)
		in_size=ds_get_packet(d_video,start);
		if (in_size<0) return eof; // EOF
	}


//------------------------ frame decoded. --------------------

	// Increase video timers:
	sh_video->num_frames+=frame_time;
	++sh_video->num_frames_decoded;

	frame_time*=sh_video->frametime;

	// override frame_time for variable/unknown FPS formats:
	if (!force_fps) switch (demuxer->file_format)
		{
		case DEMUXER_TYPE_GIF:
		case DEMUXER_TYPE_MATROSKA:
		case DEMUXER_TYPE_MNG:
			if (d_video->pts>0 && pts1>0 && d_video->pts>pts1)
				frame_time=d_video->pts-pts1;
			break;
		case DEMUXER_TYPE_TV:
		case DEMUXER_TYPE_MOV:
		case DEMUXER_TYPE_FILM:
		case DEMUXER_TYPE_VIVO:
		case DEMUXER_TYPE_OGG:
		case DEMUXER_TYPE_ASF:
		{
			double next_pts = ds_get_next_pts(d_video);
			double d= (next_pts != MP_NOPTS_VALUE) ? next_pts - d_video->pts : d_video->pts-pts1;
			if (d>=0)
			{
				if (d>0)
				{
					if ((int)sh_video->fps==1000)
						mp_msg(MSGT_CPLAYER,MSGL_V,"\navg. framerate: %d fps             \n",(int)(1.0f/d));
					sh_video->frametime=d; // 1ms
					sh_video->fps=1.0f/d;
				}
				frame_time = d;
			}
			else
			{
				mp_msg(MSGT_CPLAYER,MSGL_WARN,"\nInvalid frame duration value (%d/%d => %d). Defaulting to %d sec.\n", FLOAT1000(d_video->pts), FLOAT1000(next_pts), FLOAT1000(d), FLOAT1000(frame_time));
				// frame_time = 1/25.0;
			}
		}
		break;
		case DEMUXER_TYPE_LAVF:
		case DEMUXER_TYPE_LAVF_PREFERRED:
			if ((int)sh_video->fps==1000 || (int)sh_video->fps<=1)
			{
				double next_pts = ds_get_next_pts(d_video);
				double d= (next_pts != MP_NOPTS_VALUE) ? next_pts - d_video->pts : d_video->pts-pts1;
				if (d>=0)
				{
					frame_time = d;
				}
			}
			break;
		case DEMUXER_TYPE_REAL:
		{
			double next_pts = ds_get_next_pts(d_video);
			double d = (next_pts != MP_NOPTS_VALUE) ? next_pts - d_video->pts : d_video->pts - pts1;

			frame_time = (d >= 0 && pts1 > 0) ? d : 0.001;
		}
		break;
		}

	if (video_codec == VIDEO_MPEG12)
	{
		sh_video->pts+=frame_time;
		if (picture_coding_type==1)
			d_video->flags |= 1;
		if (picture_coding_type<=2 && sh_video->i_pts)
		{
			sh_video->pts=sh_video->i_pts;
			sh_video->i_pts=pts;
		}
		else
		{
			if (pts)
			{
				if (picture_coding_type<=2) sh_video->i_pts=pts;
				else sh_video->pts=pts;
			}
		}
	}
	else
		sh_video->pts=d_video->pts;

	if (frame_time_ptr) *frame_time_ptr=frame_time;
	//memset(videobuffer+in_size, 0, 8);	//This is work around for HW 8190
	swap_videobuffer();
	return in_size;
#endif	
}
#endif
