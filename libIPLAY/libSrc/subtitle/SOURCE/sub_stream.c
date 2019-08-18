/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      stream.c
*
* Programmer:    Meng Wang
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Meng Wang    first file
*	  <2>	  04/13/2005	Meng Wang	 Modify for New mp612 os & file System 
****************************************************************
*/

/*!
    @defgroup STREAM    Stream access
*/

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#ifndef __GLOBAL612_H
#include "global612.h"
#endif

#if (DISPLAY_VIDEO_SUBTITLE||LYRIC_ENABLE)

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "stream.h"
//#include "index.h"
#include "mpTrace.h"


#define SUB_STREAM_BUFFER_SIZE (1 << 12)

/*
*******************************************************************************
*        GLOBAL VARIABLE
*******************************************************************************
*/
BYTE __attribute__ ((aligned(4))) SubStreamBuffer[SUB_STREAM_BUFFER_SIZE];



/*
*******************************************************************************
*        EXTERNAL FUNCTIONS
*******************************************************************************
*/
//extern int sub_open_file(stream_t * stream, STREAM * sHandle);
//extern BYTE CacheFillCompleted;

/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/




// reset the stream element's pointers
void sub_stream_reset(stream_t * s)
{
	if (s->eof)
	{
		s->pos = 0;
		s->eof = 0;
	}
}




/*!
	@ingroup	STREAM
	@brief	Create a stream for the current choosen file of specific drive 
	@param	DRIVE *drv		The handle of drive to open
	@return	The function call will return null if no stream handle can use,
	         else will return the point of stream handle.
*/
stream_t *sub_stream_create(register STREAM * sHandle)
{
    MP_DEBUG("sub_stream_create");
	int ret;
	stream_t *s = (stream_t *) mem_malloc(sizeof(stream_t));

	if (s == NULL)
	{
		return NULL;
	}
	memset(s, 0, sizeof(stream_t));

	s->buffer = (BYTE *)((DWORD)SubStreamBuffer);

	ret = sub_open_file(s, sHandle);

	if (ret != STREAM_OK)
	{
		mem_free(s);
		return NULL;
	}

	// stream caching go
	//stream_enable_cache(s);

	if (!sub_stream_fill_buffer(s)) return NULL; // EOF

	return s;
}



/*!
	@ingroup	STREAM

	@brief	Fill a specific stream's buffer  

	@param	stream_t *s		The handle of stream to fill

	@return	The function call will return 0 if no byte is  filled into the stream buffer ,
	         else will return the number of bytes filled into the  stream buffer.
	@remark If cache enbaled, data source is from cache buffer, else from file directly
*/
int sub_stream_fill_buffer(register stream_t * s)
{
    MP_DEBUG("sub_stream_fill_buffer");
	register int len = 0;

    if (s->pos >= FileSizeGet(s->fd)) /* note: here is (s->pos '>=' FileSizeGet(s->fd)) for EOF reached */
    {	
	    s->eof = 1;
    }
	if (s->eof)
	{							// end of file
		s->buf_pos = s->buf_len = 0;
		return 0;
	}

	if (s->pos != FilePosGet(s->fd))
	{
		if (!s->seek)	return 0;
		if (!s->seek(s, s->pos))	return 0;
	}

	if (s->fill_buffer)
	{
		len = s->fill_buffer(s, s->buffer, SUB_STREAM_BUFFER_SIZE);
	}

	if (len <= 0)
	{							// end of file or read error    
		s->eof = 1;
		s->buf_pos = s->buf_len = 0;
		return 0;
	}

	s->buf_pos = 0;
	s->buf_len = len;
	s->pos += len;

	return len;
}





/*!
	@ingroup	STREAM

	@brief	Move stream buffer's read cursor to new file position 

	@param	stream_t *s		The stream to operate

	@param	unsigned int  pos		New file position

	@return	1				Success

	@return	0				Fail

	@remark	If seek successfully, stream buffer will be filled with the data around the new file 
			position, with the read cursor pointing to the exact new file position's byte

*/

int sub_stream_seek_long(register stream_t * s, unsigned int pos)
{
    MP_DEBUG("sub_stream_seek_long");
	unsigned int newpos = 0;

	s->buf_pos = 0;
	s->buf_len = 0;
	newpos = pos & (~(SUB_STREAM_BUFFER_SIZE - 1));
	pos -= newpos;

	if (newpos == 0 || newpos != s->pos)
	{
		// This should at the beginning as soon as all streams are converted
		if (!s->seek)
			return 0;
		// Now seek
		if (!s->seek(s, newpos))
		{
			return 0;
		}
	}

	sub_stream_fill_buffer(s);
	if (pos >= 0 && pos <= s->buf_len)
	{
		s->buf_pos = pos;		// byte position 
		return 1;
	}

	return 0;
}




/*!
	@ingroup	STREAM

	@brief	Free a stream's relative resources 

	@param	stream_t *s		The handle of stream to free

	@return	
*/
void sub_stream_free(stream_t * s)
{
	//sub_close_file(s);
	s->fd = 0;
	s->buffer = 0;
	mem_free(s);
	s = 0;
}

#if 0
/*!
	@ingroup	STREAM

	@brief	Create a new stream using the given buffer  

	@param	unsigned char* data	Data address to create an stream

	@param	int len				Data length
	
	@return	The function call will return 0 if stream create fails ,
	         else will return the handle of the new stream.
*/
stream_t *sub_new_memory_stream(unsigned char *data, int len)
{
	stream_t *s = (stream_t *) mem_malloc(sizeof(stream_t) + len);

	memset(s, 0, sizeof(stream_t));
	s->fd = 0;
	s->buf_pos = 0;
	s->buf_len = len;
	s->start_pos = 0;
	s->end_pos = len;
	sub_stream_reset(s);
	s->pos = len;
	s->buffer = (BYTE *)((DWORD)SubStreamBuffer);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	mmcp_memcpy_polling(s->buffer, data, len);
#else
	memcpy(s->buffer, data, len);
#endif
	return s;
}


/*!
	@ingroup	STREAM

	@brief	Get a specific stream's fiel size 

	@param	stream_t *s		The handle of stream to fill

	@return	The function call will return the total file size of the stream.
	@remark 
*/
DWORD stream_get_file_size(register stream_t * s)
{
    return FileSizeGet(s->fd);
}
#endif
#endif
