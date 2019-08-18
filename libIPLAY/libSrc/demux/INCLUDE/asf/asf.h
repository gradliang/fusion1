/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      asf.h
*
* Programmer:    Greg Xu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Greg Xu    first file
****************************************************************
*/

#ifndef __ASF_H
#define __ASF_H

#include "UtilTypeDef.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

///////////////////////
// MS GUID definition
///////////////////////
#ifndef GUID_DEFINED
#define GUID_DEFINED
#endif

// Size of GUID is 16 bytes!
typedef struct   __attribute__((packed)) {
//typedef struct {
	uint32_t	Data1;		// 4 bytes
	uint16_t	Data2;		// 2 bytes
	//uint8_t	pad1[2];
	uint16_t	Data3;		// 2 bytes
	//uint8_t	pad2[2];	
	uint8_t	Data4[8];	// 8 bytes	
} GUID_t;


///////////////////////
// ASF Object Header 
///////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  uint8_t guid[16];
  uint64_t size;
} ASF_obj_header_t;

////////////////
// ASF Header 
////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  ASF_obj_header_t objh;
  uint32_t cno; // number of subchunks
  uint8_t v1; // unknown (0x01)
  //uint8_t pad1[3];
  uint8_t v2; // unknown (0x02)
  //uint8_t pad2[3];
} ASF_header_t;

/////////////////////
// ASF File Header 
/////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  uint8_t stream_id[16]; // stream GUID
  uint64_t file_size;
  uint64_t creation_time; //File creation time FILETIME 8
  uint64_t num_packets;    //Number of packets UINT64 8
  uint64_t play_duration; //Timestamp of the end position UINT64 8
  uint64_t send_duration;  //Duration of the playback UINT64 8
  uint64_t preroll; //Time to bufferize before playing UINT32 4
  uint32_t flags; //Unknown, maybe flags ( usually contains 2 ) UINT32 4
  uint32_t min_packet_size; //Min size of the packet, in bytes UINT32 4
  uint32_t max_packet_size; //Max size of the packet  UINT32 4
  uint32_t max_bitrate; //Maximum bitrate of the media (sum of all the stream)
} ASF_file_header_t;

///////////////////////
// ASF Stream Header
///////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  uint8_t type[16]; // Stream type (audio/video) GUID 16
  uint8_t concealment[16]; // Audio error concealment type GUID 16
  uint64_t unk1; // Unknown, maybe reserved ( usually contains 0 ) UINT64 8
  uint32_t type_size; //Total size of type-specific data UINT32 4
  uint32_t stream_size; //Size of stream-specific data UINT32 4
  uint16_t stream_no; //Stream number UINT16 2
  //uint8_t	pad1[2];  
  uint32_t unk2; //Unknown UINT32 4
} ASF_stream_header_t;

///////////////////////////
// ASF Content Description
///////////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  uint16_t title_size;
  //uint8_t	pad1[2];  
  uint16_t author_size;
  //uint8_t	pad2[2];    
  uint16_t copyright_size;
  //uint8_t	pad3[2];    
  uint16_t comment_size;
  //uint8_t	pad4[2];    
  uint16_t rating_size;
  //uint8_t	pad5[2];    
} ASF_content_description_t;

/*structure of audio file content, jackyang 20050607*/
typedef struct _ASF_Content_
{
	char szTitle[MAX_TITLE_LEN];
	char szAuthor[MAX_AUTHOR_LEN];
	char szCopyright[MAX_COPYRIGHT_LEN];
	char szRating[MAX_RATING_LEN];
	char szDescription[MAX_DESCRIP_LEN];
}ASF_Content, *PASF_Content;
/*added end jackyang 20050607*/

////////////////////////
// ASF Segment Header 
////////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
  uint8_t streamno;
  //uint8_t	pad1[3];    
  uint8_t seq;
  //uint8_t	pad2[3];    
  uint32_t x;
  uint8_t flag;
  //uint8_t	pad3[3];    
} ASF_segmhdr_t;

//////////////////////
// ASF Stream Chunck
//////////////////////
typedef struct __attribute__((packed)) {
//typedef struct {
	uint16_t	type;
	//uint8_t	pad1[2];  	
	uint16_t	size;
	//uint8_t	pad2[2];  		
	uint32_t	sequence_number;
	uint16_t	unknown;
	//uint8_t	pad3[2];  		
	uint16_t	size_confirm;
	//uint8_t	pad4[2];  	
} ASF_stream_chunck_t;

typedef struct __attribute__((packed)) 
{
	uint64_t file_id1;
	uint64_t file_id2;
	uint64_t idx_entry_time_interval;
	uint32_t max_pkt_cnt;
	uint32_t idx_entries_cnt;
} ASF_SIMPLE_INDEX_OBJ_T;


// Definition of the stream type
#ifdef WORDS_BIGENDIAN
	#define ASF_STREAMING_CLEAR	0x2443		// $C
	#define ASF_STREAMING_DATA	0x2444		// $D
	#define ASF_STREAMING_END_TRANS	0x2445		// $E
	#define	ASF_STREAMING_HEADER	0x2448		// $H
#else
	#define ASF_STREAMING_CLEAR	0x4324		// $C
	#define ASF_STREAMING_DATA	0x4424		// $D
	#define ASF_STREAMING_END_TRANS	0x4524		// $E
	#define	ASF_STREAMING_HEADER	0x4824		// $H
#endif

// Definition of the differents type of ASF streaming
typedef enum {
	ASF_Unknown_e,
	ASF_Live_e,
	ASF_Prerecorded_e,
	ASF_Redirector_e,
	ASF_PlainText_e,
	ASF_Authenticate_e
} ASF_StreamType_e;

typedef struct {
	ASF_StreamType_e streaming_type;
	int request;
	int packet_size;
	int *audio_streams,n_audio,*video_streams,n_video;
	int audio_id, video_id;
} asf_http_streaming_ctrl_t;


typedef struct {
    uint32_t packet_number;
    uint16_t packet_count;
} ASF_INDEX_ENTRY_T;

typedef struct {
  // index stuff:
  void* idx;
  uint64_t idx_size;
  unsigned int  idx_pos;
  unsigned int  idx_pos_a;
  unsigned int  idx_pos_v;
  unsigned int  idx_pos_v_3D;
  unsigned int  idx_offset;  // ennyit kell hozzaadni az index offset ertekekhez
  // bps-based PTS stuff:
  int video_pack_no;
  int audio_block_size;
  unsigned int audio_block_no;
  // interleaved PTS stuff:
  int skip_video_frames;
  int audio_streams;
  float avi_audio_pts;
  float avi_video_pts;
  float pts_correction;
  unsigned int pts_corr_bytes;
  unsigned char pts_corrected;
  unsigned char pts_has_video;
  unsigned int numberofframes;
} ASF_PRIV_T;


/*
 * Some macros to swap little endian structures read from an ASF file
 * into machine endian format
 */
 
#ifdef WORDS_BIGENDIAN
#define	le2me_ASF_obj_header_t(h) {					\
    (h)->size = le2me_64((h)->size);					\
}

#define	le2me_ASF_header_t(h) {						\
    le2me_ASF_obj_header_t(&(h)->objh);					\
    (h)->cno = le2me_32((h)->cno);					\
}

#define le2me_ASF_stream_header_t(h) {					\
    (h)->unk1 = le2me_64((h)->unk1);					\
    (h)->type_size = le2me_32((h)->type_size);				\
    (h)->stream_size = le2me_32((h)->stream_size);			\
    (h)->stream_no = le2me_16((h)->stream_no);				\
    (h)->unk2 = le2me_32((h)->unk2);					\
}

#define le2me_ASF_file_header_t(h) {					\
    (h)->file_size = le2me_64((h)->file_size);				\
    (h)->creation_time = le2me_64((h)->creation_time);			\
    (h)->num_packets = le2me_64((h)->num_packets);			\
    (h)->play_duration = le2me_64((h)->play_duration);			\
    (h)->send_duration = le2me_64((h)->send_duration);			\
    (h)->preroll = le2me_64((h)->preroll);				\
    (h)->flags = le2me_32((h)->flags);					\
    (h)->min_packet_size = le2me_32((h)->min_packet_size);		\
    (h)->max_packet_size = le2me_32((h)->max_packet_size);		\
    (h)->max_bitrate = le2me_32((h)->max_bitrate);			\
}

#define le2me_ASF_content_description_t(h) {				\
    (h)->title_size = le2me_16((h)->title_size);			\
    (h)->author_size = le2me_16((h)->author_size);			\
    (h)->copyright_size = le2me_16((h)->copyright_size);		\
    (h)->comment_size = le2me_16((h)->comment_size);			\
    (h)->rating_size = le2me_16((h)->rating_size);			\
}

#define le2me_BITMAPINFOHEADER(h) {					\
    (h)->biSize = le2me_32((h)->biSize);				\
    (h)->biWidth = le2me_32((h)->biWidth);				\
    (h)->biHeight = le2me_32((h)->biHeight);				\
    (h)->biPlanes = le2me_16((h)->biPlanes);				\
    (h)->biBitCount = le2me_16((h)->biBitCount);			\
    (h)->biCompression = le2me_32((h)->biCompression);			\
    (h)->biSizeImage = le2me_32((h)->biSizeImage);			\
    (h)->biXPelsPerMeter = le2me_32((h)->biXPelsPerMeter);		\
    (h)->biYPelsPerMeter = le2me_32((h)->biYPelsPerMeter);		\
    (h)->biClrUsed = le2me_32((h)->biClrUsed);				\
    (h)->biClrImportant = le2me_32((h)->biClrImportant);		\
}

#define le2me_WAVEFORMATEX(h) {						\
    (h)->wFormatTag = le2me_16((h)->wFormatTag);			\
    (h)->nChannels = le2me_16((h)->nChannels);				\
    (h)->nSamplesPerSec = le2me_32((h)->nSamplesPerSec);		\
    (h)->nAvgBytesPerSec = le2me_32((h)->nAvgBytesPerSec);		\
    (h)->nBlockAlign = le2me_16((h)->nBlockAlign);			\
    (h)->wBitsPerSample = le2me_16((h)->wBitsPerSample);		\
    (h)->cbSize = le2me_16((h)->cbSize);				\
}

#define le2me_ASF_stream_chunck_t(h) {					\
    (h)->size = le2me_16((h)->size);					\
    (h)->sequence_number = le2me_32((h)->sequence_number);		\
    (h)->unknown = le2me_16((h)->unknown);				\
    (h)->size_confirm = le2me_16((h)->size_confirm);			\
}

#define le2me_ASF_simple_index_obj_t(h) {                                   \
	(h)->file_id1                = le2me_64((h)->file_id1);                 \
	(h)->file_id2                = le2me_64((h)->file_id2);                 \
	(h)->idx_entry_time_interval = le2me_64((h)->idx_entry_time_interval);  \
	(h)->max_pkt_cnt             = le2me_32((h)->max_pkt_cnt);              \
	(h)->idx_entries_cnt         = le2me_32((h)->idx_entries_cnt);          \
}

#else
#define	le2me_ASF_obj_header_t(h)	/**/
#define	le2me_ASF_header_t(h)		/**/
#define le2me_ASF_stream_header_t(h)	/**/
#define le2me_ASF_file_header_t(h)	/**/
#define le2me_ASF_content_description_t(h) /**/
#define le2me_BITMAPINFOHEADER(h)   /**/
#define le2me_WAVEFORMATEX(h)	    /**/
#define le2me_ASF_stream_chunck_t(h) /**/
#define le2me_ASF_simple_index_obj_t(h) /**/
#endif

#endif
