/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      stream.h
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
*	  <2>	  04/13/2005	Meng Wang	 Modify for New mp612 os & file System 
****************************************************************
*/
#ifndef __STREAM_H
#define __STREAM_H

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#include "global612.h"
#include "mpTrace.h"

#include "UtilTypeDef.h"
#include "stream_type.h"
#include "Peripheral.h"

/*
*******************************************************************************
*        #define CONSTANTS
*******************************************************************************
*/


/*!
   @defgroup	CONST	Constants
    Define the stream system relative constants 
   @{
*/

//#ifdef __USE_STREAM_CACHE__
///Stream cache buffer size in byte
#define STREAM_CACHE_SIZE 				(1 << 19)	
//#endif
///Stream buffer size in byte

///Stream operations error code return
#define STREAM_ERROR					0
///Stream operations successful code return
#define STREAM_OK						1
///@}

extern stream_t * stream_create(STREAM *sHandle);
extern void stream_free(stream_t *); 
extern void stream_reset(stream_t *);

#ifdef __USE_STREAM_CACHE__
extern void stream_enable_cache(stream_t *);
extern int cache_stream_fill_buffer(stream_t *);
extern int cache_stream_seek_long(stream_t *, unsigned int);
#else
#define stream_enable_cache(x)		__asm("nop")
#define cache_stream_fill_buffer(x)	stream_fill_buffer(x)
#define cache_stream_seek_long(x,y)	stream_seek_long(x,y)
#endif

#define	STREAM_READ_COUNT	0
#if	STREAM_READ_COUNT
extern unsigned int stream_read_char_count;
extern unsigned int stream_read_count;
extern unsigned int stream_read_dword_le_count;
#endif

/*
*******************************************************************************
*        LOCAL  FUNCTIONS
*******************************************************************************
*/

/*!
	@ingroup	STREAM

	@brief	Read a char from a specific stream buffer's current position.  

	@param	stream_t *s		The stream to read 

	@return	The function call will return the char read from the stream's buffer for successful execution ,
	         else will return an error number of -256.

	@remark	If stream buffer's read cursor reaches the end , it will call relative function to fill stream's
	        buffer using the data read from either cache bufer or file.
	        
*/
inline static int stream_read_char(stream_t * const s)
{
#if STREAM_READ_COUNT
	stream_read_char_count++;
#endif
	int ret;
    s->from_stream_seek_long = FALSE;
	s->buffer = (BYTE*)((int)s->buffer | 0xa0000000);
	ret = (s->buf_pos<s->buf_len) ? s->buffer[s->buf_pos++] :
		(cache_stream_fill_buffer(s) ? s->buffer[s->buf_pos++] : -256);			
	s->buffer = (BYTE*)((int)s->buffer & 0x8FFFFFFF);
	return ret;

//	return  (s->buf_pos<s->buf_len) ? s->buffer[s->buf_pos++] :
//		(cache_stream_fill_buffer(s) ? s->buffer[s->buf_pos++] : -256);		
}


/*!
	@ingroup	STREAM

	@brief	Read a word from a specific stream buffer's current position.  

	@param	stream_t *s		The stream to read 

	@return	The function call will return the word read from the stream's buffer for successful execution ,
	         else will return an error number of -256.

	@remark	This word is stored in big endian format
	        
*/
inline static unsigned int stream_read_word(stream_t * const s)
{
	int x,y;
	x = stream_read_char(s);
	y = stream_read_char(s);
	return (x << 8) | y;
}


/*!
	@ingroup	STREAM

	@brief	Read a dword from a specific stream buffer's current position.  

	@param	stream_t *s		The stream to read 

	@return	The function call will return the dword read from the stream's buffer for successful execution ,
	         else will return an error number of -256.

	@remark	This dword is stored in big endian format
	        
*/
inline static unsigned int stream_read_dword(stream_t * const s)
{
	unsigned int y;
	y = stream_read_char(s);
	y = ( y<<8 ) | stream_read_char(s);
	y = ( y<<8 ) | stream_read_char(s);
	y = ( y<<8 ) | stream_read_char(s);
	return y;
}


/*!
	@ingroup	STREAM

	@brief	Read a word from a specific stream buffer's current position.  

	@param	stream_t *s		The stream to read 

	@return	The function call will return the word read from the stream's buffer for successful execution ,
	         else will return an error number of -256.

	@remark	This word is stored in little endian format
	        
*/
inline static unsigned int stream_read_word_le(stream_t * const s)
{
	int x,y;
	x = stream_read_char(s);
	y = stream_read_char(s);
	return (y << 8) | x;
}

#define stream_read_fourcc stream_read_dword_le
/*!
	@ingroup	STREAM

	@brief	Read a dword from a specific stream buffer's current position.  

	@param	stream_t *s		The stream to read 

	@return	The function call will return the dword read from the stream's buffer for successful execution ,
	         else will return an error number of -256.

	@remark	This dword is stored in little endian format
	        
*/
inline static unsigned int stream_read_dword_le(stream_t *s)
{
#if STREAM_READ_COUNT
	stream_read_dword_le_count++;
#endif
	unsigned int y;
	y = stream_read_char(s);
	y |= stream_read_char(s) << 8;
	y |= stream_read_char(s) << 16;
	y |= stream_read_char(s) << 24;
	return y;
}

inline static uint64_t stream_read_qword(stream_t *s)
{
	uint64_t y;
	y = stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	return y;
}

inline static unsigned int stream_read_int24(stream_t *s)
{
	unsigned int y;
	y = stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	y = (y << 8) | stream_read_char(s);
	return y;
}


/*!
	@ingroup	STREAM

	@brief	Read some data  from a specific stream buffer and store into new address.  

	@param	stream_t *s		The stream to read 

	@param	char *mem		The address to store data read from stream buffer 

	@param	int    total	The number of bytes to read 

	@return	The function call will return the actual number of bytes read from the stream buffer

	@remark	Each time the stream read cursor reaches its buffer end, it will fill stream buffer using 
	        the data either from cache or file.  
	        
*/
inline static int stream_read(stream_t *s, char* mem, int total)
{
#if STREAM_READ_COUNT
	stream_read_count++;
#endif
	//mpDebugPrint("%s(,,%d)", __FUNCTION__, total);
	MP_PERFORMANCE_TRCN("");
	int len = total;
	int i;	
	
	while (len > 0)
	{
		unsigned int x;
		x = s->buf_len - s->buf_pos;
		if (x == 0)
		{
		    s->from_stream_seek_long = FALSE;
			if (!cache_stream_fill_buffer(s))
			return total-len; // EOF
			x = s->buf_len - s->buf_pos;
		}
		if (x > len)
			x = len;
#if (CHIP_VER_MSB == CHIP_VER_650 || CHIP_VER_MSB == CHIP_VER_660)
		if (x < 0x150)	memcpy(mem, &s->buffer[s->buf_pos], x);
		else			mmcp_memcpy_polling(mem, &s->buffer[s->buf_pos], x);	
#else
		memcpy(mem, &s->buffer[s->buf_pos], x);
#endif
		s->buf_pos += x; mem += x; len -= x;
	}
	//if (GetSysTime()-LastSysTime > 10)
		MP_PERFORMANCE_TRC("%s (%d)", __FUNCTION__, ELAPSE_TIME);
	return total;
}


inline static int stream_eof(const stream_t * const s)
{
	return s->eof;
}


/*!
	@ingroup	STREAM

	@brief	Find out stream buffer's read cursor offset in the file 

	@param	stream_t *s		The stream to check 

	@return	Return stream buffer's read cursor now file position
*/
inline static unsigned int stream_tell(const stream_t * const s)
{
	return s->pos + s->buf_pos - s->buf_len;
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
inline static int stream_seek(stream_t *s, unsigned int pos)
{
	if((pos < s->pos) && (s->pos - pos <= s->buf_len))
	{
		unsigned int  x = pos - (s->pos - s->buf_len);	//s->pos point to the file current position
		if(x >= 0)
		{
			s->buf_pos = x;					//the copy of file to buffer
			return 1;
		}
	}

	return cache_stream_seek_long(s, pos);
}


/*!
	@ingroup	STREAM

	@brief	Let stream buffer's read cursor skip off some data 

	@param	stream_t *s		The stream to operate

	@param	unsigned int  len		Number of bytes to skip

	@return	1				Success

	@return	0				Fail

	@remark	If skip successfully, stream buffer will be filled with the data around the new file 
			position, with the read cursor pointing to the exact new file position's byte

*/
inline static int stream_skip(stream_t *s, unsigned int len)
{
	if ((len < 0) || (len > 2*STREAM_BUFFER_SIZE))
		return stream_seek(s, stream_tell(s) + len); // negative or big skip!
	
	while (len > 0)
	{
		int x = s->buf_len - s->buf_pos;
		if (x == 0)
		{
		    s->from_stream_seek_long = FALSE;
			if (!cache_stream_fill_buffer(s)) return 0; // EOF
			x = s->buf_len - s->buf_pos;
		}
		if (x > len) x = len;
		s->buf_pos += x; len -= x;
	}
	return 1;
}

 
#endif // __STREAM_H

