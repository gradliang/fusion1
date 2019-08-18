/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      stheader.h
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

#ifndef __ST_HEADER_H
#define __ST_HEADER_H

// for AVIStreamHeader:
#include "avi/avifmt.h"
#include "global612.h"
//#include "UtilTypeDef.h"
#include "demux_types.h"

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

typedef struct _WAVEFORMATEX
{
	WORD   wFormatTag;
	WORD   nChannels;
	DWORD  nSamplesPerSec;
	DWORD  nAvgBytesPerSec;
	WORD   nBlockAlign;
	WORD   wBitsPerSample;
	WORD   cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *NPWAVEFORMATEX, *LPWAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

#ifndef _BITMAPINFOHEADER_
#define _BITMAPINFOHEADER_

typedef struct
{
	int 	biSize;
	int  	biWidth;
	int  	biHeight;
	short 	biPlanes;
	short 	biBitCount;
	int 	biCompression;
	int 	biSizeImage;
	int  	biXPelsPerMeter;
	int  	biYPelsPerMeter;
	int 	biClrUsed;
	int 	biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER, *LPBITMAPINFOHEADER;
typedef struct
{
	BITMAPINFOHEADER bmiHeader;
	int	bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO;
#endif

// Stream headers:
#define SH_COMMON \
  demux_stream_t *ds; \
/*  struct codecs *codec; */ \
  unsigned int format; \
  int inited; \
  float stream_delay; /* number of seconds stream should be delayed (according to dwStart or similar) */ \
  /* things needed for parsing */ \
  int needs_parsing; \
  struct AVCodecContext *avctx; \
  struct AVCodecParserContext *parser; \
  /* audio: last known pts value in output from decoder \
   * video: predicted/interpolated PTS of the current frame */ \
  float pts; \
  /* codec-specific: */ \
  void* context;   /* codec-specific stuff (usually HANDLE or struct pointer) */ \
  char* lang; /* track language */ \
  int default_track; \
 

typedef struct sh_common
{
	SH_COMMON
} sh_common_t;

typedef struct
{
	SH_COMMON
	demux_stream_t ds1;
	int aid;

	float delay;				// relative (to sh_video->timer) time in audio stream

	// output format:
//	int sample_format;
	int samplerate;
	int samplesize;
	int channels;
	int o_bps;					// == samplerate * samplesize * channels   (uncompr. bytes/sec)
	int i_bps;					// == bitrate  (compressed bytes/sec)

	// in buffers:
	int audio_in_minsize;		// max. compressed packet size (== min. in buffer size)
	char* a_in_buffer;			//the exact place where data load in(the exact packet buffer)
	int a_in_buffer_len;		//buffer len of (a_in_buffer)
	int a_in_buffer_size;

	// decoder buffers:
	int audio_out_minsize;		// max. uncompressed packet size (==min. out buffsize)
	char* a_buffer;
	int a_buffer_len;
	int a_buffer_size;

	// output buffers:
	char* a_out_buffer;
	int a_out_buffer_len;
	int a_out_buffer_size;
	//void* audio_out;				// the audio_out handle, used for this audio stream
	void* afilter;				// the audio filter stream
#ifdef DYNAMIC_PLUGINS
	void *dec_handle;
#endif

	// win32-compatible codec parameters:
	AVIStreamHeader audio;		//"avih"
	WAVEFORMATEX* wf;			//"strf"

	// codec-specific:
	unsigned char* codecdata;	// extra header data passed from demuxer to codec
	int codecdata_len;
} sh_audio_t;

typedef struct
{
	SH_COMMON
	int vid;
	float timer;		  			// absolute time in video stream, since last start/seek
	// frame counters:
	float num_frames;       	// number of frames played
	int num_frames_decoded; 	// number of frames decoded
	// timing (mostly for mpeg):
	float i_pts;   				// PTS for the _next_ I/P frame
	// output format: (set by demuxer)
	float fps;              		// frames per second (set only if constant fps)
	float frametime;        		// 1/fps
	float aspect;           		// aspect ratio stored in the file (for prescaling)
	int i_bps;              		// == bitrate  (compressed bytes/sec)
	int disp_w,disp_h;      		// display size (filled by fileformat parser)
	// output driver/filters: (set by libmpcodecs core)
	unsigned int outfmtidx;
	void* video_out;        		// the video_out handle, used for this video stream
	void* vfilter;          		// the video filter chain, used for this video stream
	int vf_inited;
	int mp4_profile;
#ifdef DYNAMIC_PLUGINS
	void *dec_handle;		//DECODER
#endif
	// win32-compatible codec parameters:
	AVIStreamHeader video;
	BITMAPINFOHEADER* bih;
	void* ImageDesc; // for quicktime codecs
	BYTE b_yuv444;
	//BYTE b_wait_decode_finish;
	BYTE b_3d; // 3D project
	BYTE b_r_img; 
	// codec-specific:
} sh_video_t;

// demuxer.c:
sh_audio_t* new_sh_audio(demuxer_t * const demuxer, const int id);
sh_video_t* new_sh_video(demuxer_t * const demuxer, const int id);
sh_video_t *new_sh_video_3D(demuxer_t * const demuxer, const int id);
// video.c:
int video_read_properties(sh_video_t * const sh_video);
int video_read_frame(sh_video_t* const sh_video, float* const frame_time_ptr, unsigned char** start, const int force_fps);
void video_free_buffer();

#define	VCODEC_MPEG1	0x10000001
#define	VCODEC_MPEG2	0x10000002
#define	VCODEC_MPEG4	0x10000004
#define	VCODEC_H264		0x10000005

#endif

