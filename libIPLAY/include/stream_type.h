
#ifndef STREAM_TYPE_H
#define STREAM_TYPE_H

#include "UtilTypeDef.h"
#include "fs.h"
#include "memwatch.h"

#define 	STREAM_BUFFER_SIZE				(1 << 16) 

typedef struct stream_st {
	// read
	int (*fill_buffer)(struct stream_st *s, BYTE *buffer, int max_len);
	// write
	int (*write_buffer)(struct stream_st *s, BYTE *buffer, int len);
	// seek
	int (*seek)(struct stream_st *s, unsigned int pos);
	// close
	void (*close)(struct stream_st *s);
	// file descriptor used by mx610
	STREAM *fd;
	// buffer position
	unsigned int buf_pos;
	unsigned int buf_len;
	
	// file length
	unsigned int file_len;
	// file position
	unsigned int pos;		           //current file pointer position
	unsigned int start_pos;	       //usually 0  start_pos+file_len=end_pos
	unsigned int end_pos;
	// end of file
	int eof;
	// stream cache
	void * cache_data;
	// stream buffer	
	BYTE *buffer;  //where exactly put the entire file here

	BOOL from_stream_seek_long;

} stream_t;


typedef struct _S_BK_STREAM_NODE_T
{
    BYTE buffer[STREAM_BUFFER_SIZE];
	struct _S_BK_STREAM_NODE_T* p_prev; 
    struct _S_BK_STREAM_NODE_T* p_next; 
	unsigned int buf_len;
	unsigned int pos;		    
}S_BK_STREAM_NODE_T;


#endif

