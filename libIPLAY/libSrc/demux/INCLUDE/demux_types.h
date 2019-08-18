/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      demux_types.h
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

#ifndef __DEMUX_TYPES__
#define __DEMUX_TYPES__

#include "stream_type.h"
#include "id3tag.h"

/*
#ifndef	malloc
#define	malloc			mem_malloc
#endif
#ifndef	free
#define	free			mem_free
#endif
//#ifndef	realloc
//#define	realloc			mem_reallocm
//#endif
#define memalign(x,y)	mem_malloc(y)
#ifndef	calloc
#define	calloc			mem_calloc
#endif
*/

//typedef mp_codec_info_t ad_info_t;
#define MAX_PACKS (3072)
#define MAX_PACK_BYTES 0x600000

#define DEMUXER_TYPE_UNKNOWN 0
#define DEMUXER_TYPE_MPEG_ES 1
#define DEMUXER_TYPE_MPEG_PS 2
#define DEMUXER_TYPE_AVI 3
#define DEMUXER_TYPE_AVI_NI 4
#define DEMUXER_TYPE_AVI_NINI 5
#define DEMUXER_TYPE_ASF 6
#define DEMUXER_TYPE_MOV 7
#define DEMUXER_TYPE_VIVO 8
#define DEMUXER_TYPE_TV 9
#define DEMUXER_TYPE_FLI 10
#define DEMUXER_TYPE_REAL 11
#define DEMUXER_TYPE_Y4M 12
#define DEMUXER_TYPE_FILM 14
#define DEMUXER_TYPE_ROQ 15
#define DEMUXER_TYPE_MF 16
#define DEMUXER_TYPE_AUDIO 17
#define DEMUXER_TYPE_OGG 18
#define DEMUXER_TYPE_RAWAUDIO 20
#define DEMUXER_TYPE_RTP 21
#define DEMUXER_TYPE_RAWDV 22
#define DEMUXER_TYPE_PVA 23
#define DEMUXER_TYPE_SMJPEG 24
#define DEMUXER_TYPE_XMMS 25
#define DEMUXER_TYPE_RAWVIDEO 26
#define DEMUXER_TYPE_MPEG4_ES 27
#define DEMUXER_TYPE_GIF 28
#define DEMUXER_TYPE_MPEG_TS 29
#define DEMUXER_TYPE_H264_ES 30
#define DEMUXER_TYPE_MATROSKA 31
#define DEMUXER_TYPE_REALAUDIO 32
#define DEMUXER_TYPE_MPEG_TY 33
#define DEMUXER_TYPE_LMLM4 34
#define DEMUXER_TYPE_LAVF 35
#define DEMUXER_TYPE_NSV 36
#define DEMUXER_TYPE_VQF 37
#define DEMUXER_TYPE_AVS 38
#define DEMUXER_TYPE_AAC 39
#define DEMUXER_TYPE_MPC 40
#define DEMUXER_TYPE_MPEG_PES 41
#define DEMUXER_TYPE_MPEG_GXF 42
#define DEMUXER_TYPE_NUT 43
#define DEMUXER_TYPE_LAVF_PREFERRED 44
#define DEMUXER_TYPE_RTP_NEMESI 45
#define DEMUXER_TYPE_MNG 46
#define DEMUXER_TYPE_h264 47
#define DEMUXER_TYPE_h263 48

#define DEMUXER_TYPE_FLV 99


#define MP_NOPTS_VALUE (-1LL<<63) //both int64_t and double should be able to represent this exactly


// DEMUXER control commands/answers
#define DEMUXER_CTRL_NOTIMPL			-1
#define DEMUXER_CTRL_DONTKNOW			0
#define DEMUXER_CTRL_OK					1
#define DEMUXER_CTRL_GUESS				2
#define DEMUXER_CTRL_GET_TIME_LENGTH	10
#define DEMUXER_CTRL_GET_PERCENT_POS	11
#define DEMUXER_CTRL_SWITCH_AUDIO		12
#define DEMUXER_CTRL_RESYNC				13
#define DEMUXER_CTRL_SWITCH_VIDEO		14
#define DEMUXER_CTRL_IDENTIFY_PROGRAM	15
#define DEMUXER_CTRL_CORRECT_PTS		16
#define DEMUXER_CTRL_IF_PTS_FROM_STREAM 17
#define	DEMUXER_CTRL_GET_1ST_APTS		18



#define MAX_A_STREAMS 256//64 will crash mp3 playback
#define MAX_V_STREAMS 256
#define MAX_S_STREAMS 256



#define MAX_TITLE_LEN		32
#define MAX_AUTHOR_LEN		32
#define MAX_COPYRIGHT_LEN	64
#define MAX_RATING_LEN		32
#define MAX_DESCRIP_LEN		256

typedef enum DEMUX_T {
	DEMUX_AVI,
	DEMUX_MOV,
	DEMUX_MPEG_PS,
	DEMUX_MPEG_TS,
	DEMUX_FLV,
	DEMUX_ASF,
	DEMUX_MKV,
	DEMUX_h264,
	DEMUX_h263,
	DEMUX_UNKNOWN
}  DEMUX_T;

#define SEEK_ABSOLUTE (1 << 0)
#define SEEK_FACTOR   (1 << 1)

#define MP_INPUT_BUFFER_PADDING_SIZE 8

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

//void *mem_reallocm(void *, DWORD);
static inline void *realloc_struct(void *ptr, size_t nmemb, size_t size)
{
	if (nmemb > SIZE_MAX / size)
	{
		mem_free(ptr);
		return NULL;
	}
	return (void *)mem_reallocm(ptr, nmemb * size);
}

// Holds one packet/frame/whatever
# define off_t long int 

typedef struct demux_packet_st {
  int len;
  float pts;					  //time of  packet in sec not media scale 
  unsigned int pos;					// position in index (AVI) or file (MPG)
  unsigned char* buffer;		  // put sample data(or chunk if  nonvariant size) here
  int flags; 					  // keyframe, etc
  int refcount;   				  //refcounter for the master packet, if 0, buffer can be free()d
  struct demux_packet_st* master; //pointer to the master packet if this one is a cloned one
  struct demux_packet_st* next;
} demux_packet_t;
///////ref demux_packet.h//////

///////ref demux_stream.h//////
typedef struct {
	int buffer_pos;          		// position at where the first packet buffer is read, usually 0
	int buffer_size;         		// first packet buffer size
	unsigned char* buffer;   		// first packet buffer
	float pts;               		//  first packet's pts(current frame)

	int pts_bytes;           		// number of bytes read after last pts stamp
	int eof;                 		// end of demuxed stream? (true if all buffer empty)
	unsigned int pos;                  	// position in the input stream (file)
	unsigned int dpos;                 	// accumulate buffer_size  //position in the demuxed stream
	int pack_no;		   			// accumulate packets 
	int flags;              	 	// flags of current packet (keyframe etc)
	//---------------
	int packs;              		// number of packets in buffer that this stream carries
	int bytes;              		// bytes of all packets in buffer
	demux_packet_t *first;  		// read to current buffer from here
	demux_packet_t *last;   		// append new packets from input stream to here
	demux_packet_t *current;		// needed for refcounting of the buffer
	int id;                 		// stream id  (for multiple audio/video streams)

	// demuxer_t *demuxer;			// parent demuxer structure (stream handler)
	struct demuxer_st *demuxer;

	// void* demuxer;					// parent demuxer structure (stream handler)
	// ---- asf -----
	demux_packet_t *asf_packet;		// read asf fragments here
	int asf_seq;

	// ---- mov -----
	unsigned int ss_mul,ss_div;

	// ---- stream header ----
	void* sh;
} demux_stream_t;
///////ref demux_stream.h//////


///////ref stheader.h//////
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

typedef struct _WAVEFORMATEX {
  WORD   wFormatTag;
  WORD   nChannels;
  DWORD  nSamplesPerSec;
  DWORD  nAvgBytesPerSec;
  WORD   nBlockAlign;
  WORD   wBitsPerSample;
  WORD   cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *NPWAVEFORMATEX, *LPWAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

typedef struct demuxer_st {
  unsigned int filepos; // input stream current pos.
  unsigned int movi_start;
  unsigned int movi_end;
  stream_t *stream;			//video stream
  stream_t *stream_a;		//audio stream
  double reference_clock;
  int type;    // demuxer type: mpeg PS, mpeg ES, avi, avi-ni, avi-nini, asf
  int file_format;  // file format: mpeg/avi/asf
  int seekable;  // flag
  unsigned int seeked;			///< indicate if just seeked, used for skip B frames following I if closed_gop=0 after seek in MPEG series
  
  demux_stream_t *audio; // audio buffer/demuxer
  demux_stream_t *video; // video buffer/demuxer
  demux_stream_t *video_3D; // video buffer/demuxer
  demux_stream_t *sub;   // dvd subtitle buffer/demuxer

  // stream headers:
  void* a_streams[MAX_A_STREAMS]; // audio streams (sh_audio_t)
  void* v_streams[MAX_V_STREAMS]; // video sterams (sh_video_t)
  void*  sh_audio;
  void*  sh_video;
  void*  sh_video_3D;   // for stereo 3D video streams

  void* priv;  // fileformat-dependent data
  char** info;

  // Function pointers
  // check demux type based on an input data file
  int (*check_type)(stream_t*, DEMUX_T demux_type);

  // Open data streams, ready to provide audio/video data
  int (*open)();

  // Fill audio/video stream buffer
  int (*fill_buffer)(demux_stream_t* ds);

  // Seek, flags: 0 - seek from current postion, 1 - seek from begining
  // 2 - seek relative_seek_seconds relative to file postion selected
  // (current or begining). 
  int (*seek)(float relative_seek_seconds,int flags);

  // Control functions
  int (*control)(int cmd, void *arg);

  // Read audio stream
//  int (*audio_read)(int id, unsigned char* buffer, int len);

  // Free audio/video data streams
  int (*close)();

  //function GetPTSByRelFrame
  //flags=0 seek to begin
  //flags=1 seek to end
  //flags=2 seek to "+/-" rel_seek_samples
  float  (*get_pts_by_relframe)(int rel_seek_samples,int flags);

  //command=0: get total time/file length
  //command=1: get current time/file point
  //command=2: get percentage
  //return=0:  not fill the buffer
  //return=1:  fill the buffer
  int  (*get_movie_info)(BYTE* buffer,int command);

//  int (*seek_video)(float relative_seek_seconds,int flags);
} demuxer_t;

typedef struct
{
  int progid;        //program id
  int aid, vid, sid; //audio, video and subtitle id
} demux_program_t;

typedef struct
{
	char szTitle[MAX_TITLE_LEN];
	char szAuthor[MAX_AUTHOR_LEN];
	char szCopyright[MAX_COPYRIGHT_LEN];
	char szRating[MAX_RATING_LEN];
	char szDescription[MAX_DESCRIP_LEN];
}Media_Content_Video, *PMedia_Content_Video;

typedef struct
{
    // video
    char  cVidoeCodec[6];
	DWORD dwFlags;
	DWORD dwTotalTime;
	DWORD dwImageWidth;
	DWORD dwImageHeight;
	DWORD dwTotalFrame;
	float dwFrameRate;	   
	Media_Content_Video contentinfo;

	// audio
	char  cAudioCodec[6];
	DWORD dwBitrate;
	DWORD dwSampleRate;
	DWORD dwSampleSize;
	id3_tag_t id3_tag;
}Media_Info, *PMedia_Info;


#include "stheader.h"

#endif


