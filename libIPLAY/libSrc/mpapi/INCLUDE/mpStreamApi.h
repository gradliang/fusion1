#ifndef __MPAPI_STREAM_H__
#define __MPAPI_STREAM_H__

#include "fs.h"

//Frank Lin add
#if (ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING == 1)
//  #define ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING  0
  #define ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING  1
#else
  #define ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING  0
#endif

#define STREAM_SOURCE_FILE		1
#define STREAM_SOURCE_MEMORY	2

typedef struct _buffer_st {
	BYTE	*start;
	DWORD	offset;
	DWORD	size;
} BUFFER;

typedef struct _stream_st {
	// read
//	int (*fill_buffer)(struct _stream_st *s, BYTE *buffer, int max_len);
	int (*fill_buffer)(struct _stream_st *s, int size);
	// seek
	int (*seek)(struct _stream_st *s, DWORD pos);
	// close
	void (*close)(struct _stream_st *s);
	
	// file descriptor used by mx610
	STREAM *fd;
	
	// buffer position
	DWORD buf_max_size;
	DWORD buf_pos;
	DWORD buf_len;
	
	// file length
	DWORD file_len;
	
	// file position
	DWORD file_ptr;
	DWORD pos;

	// end of file
	int eof;

	// source buffer
	BUFFER *source;

	// stream buffer	
	BYTE *buffer;
	BYTE *fat_buffer;
	BYTE *pbBufferAllocated;

	BYTE bFileFormat;
	
	BYTE bReserve[3];
} ST_MPX_STREAM;


//SeekFile
#ifndef SEEK_SET
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif

int 		mpxStreamOpen			(ST_MPX_STREAM *, void *, BYTE *, DWORD, BYTE, BYTE);
void 		mpxStreamClose		(ST_MPX_STREAM *s);
DWORD 		mpxStreamTell			(ST_MPX_STREAM *s);
DWORD 		mpxStreamGetSize		(ST_MPX_STREAM *s);
int 		mpxStreamSeek			(ST_MPX_STREAM *s, int offset, DWORD origin);
size_t 		mpxStreamRead			(void *, size_t , size_t , ST_MPX_STREAM *);
BYTE 		mpxStreamGetc			(ST_MPX_STREAM *s);
WORD 		mpxStreamReadWord	(ST_MPX_STREAM *s);
DWORD 		mpxStreamReadDWord	(ST_MPX_STREAM *s);

#endif

