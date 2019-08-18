/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      stream_cache.c
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

#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

/*!
    @defgroup STREAM_CACHE    Stream cache access
*/

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#define LOCAL_DEBUG_ENABLE		0
#define MP_PERFORMANCE_TRACE	0

#include "stream.h"
#include "index.h"
#include "taskid.h"
#include "mpTrace.h"

#ifdef __USE_STREAM_CACHE__

#pragma alignvar(4)
BYTE StreamCacheBuffer[STREAM_CACHE_SIZE];
typedef struct
{
	// constats:
	BYTE *buffer;				// base pointer of the alllocated buffer memory
	int buffer_size;			// size of the alllocated buffer memory
	int sector_size;			// size of a single sector (2048/2324)
	int back_size;				// we should keep back_size amount of old bytes for backward seek
	int fill_limit;				// we should fill buffer only if space>=fill_limit
	// int prefill;     // we should fill min prefill bytes if cache gets empty

	// filler's pointers:
	int eof;
	unsigned int  min_filepos;			// buffer contain only a part of the file, from min-max pos
	unsigned int  max_filepos;
	unsigned int  offset;				// filepos <-> bufferpos  offset value (filepos of the buffer's first byte)

	// reader's pointers:
	unsigned int  read_filepos;

	// callback:
	stream_t *stream;
} cache_vars_t;


/*
*******************************************************************************
*        LOCAL VARIABLE
*******************************************************************************
*/
static cache_vars_t *CacheVar;
static BYTE createdone = 0;
//STREAM_CACHE_EVENT
#define	EVENT_CACHE_START       	(1<<0)
#define	EVENT_CACHE_END      	  	(1<<1)


/*
*******************************************************************************
*        GLOBAL VARIABLE
*******************************************************************************
*/
BYTE CacheFillCompleted = 0;
extern BYTE StreamBuffer_2[];




/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/




/*!
	@ingroup	STREAM_CACHE

	@brief	Initialize a cache using the given size  

	@param	int size		The number of total size of cache buffer

	@param	int sector		The number of per sector size(up layer stream buffer size)

	@return	The function call will return 0 if cache init fails ,
	         else will return the handle of the cache.
*/
cache_vars_t *cache_init(int size, int sector)
{
	int num;
	cache_vars_t *s = (cache_vars_t *) mem_malloc(sizeof(cache_vars_t));

	if (!s)
		return 0;
	memset(s, 0, sizeof(cache_vars_t));
	num = size / sector;
	s->buffer_size = num * sector;
	s->sector_size = sector;

	memset(StreamCacheBuffer, 0, STREAM_CACHE_SIZE);
	s->buffer = StreamCacheBuffer;

	//s->fill_limit = 8 * sector;
	//s->fill_limit = 2 * sector;
	//s->back_size = size / 2;
	s->fill_limit = sector;
	s->back_size = size / 4;

	return s;
}





/*!
	@ingroup	STREAM_CACHE

	@brief	Release a cache's resources, and stop relative task   

	@param	stream_t *stream		The handle of stream who has the pointer to its cache for operating with

*/
void cache_uninit(stream_t * stream)
{
	DWORD release;
	EventWait(STREAM_CACHE_EVENT, EVENT_CACHE_END, OS_EVENT_OR, &release);
	cache_vars_t *c = stream->cache_data;

	if (!c)
		return;					// stream under cache

	mem_free(c->stream);		// free stream under cache
	c->buffer = 0;
	mem_free(c);				// free cache
	CacheVar = 0;
	EventClear(STREAM_CACHE_EVENT, ~EVENT_CACHE_START);
}

/*!
	@ingroup	STREAM_CACHE

	@brief	Fill a specific cache buffer  

	@param	cache_vars_t* s		The handle of cache to fill

	@return	The function call will return 0 if no byte is  filled into the cache buffer ,
	         else will return the number of bytes filled into the  cache buffer.
*/
static int cache_fill(cache_vars_t * s)
{
	int back, back2, newb, space, len, pos;
	unsigned int  read = s->read_filepos;

	if (read < s->min_filepos || read > s->max_filepos)
	{
		// MP_DPF("Out of boundaries... seeking to 0x%X; min_filepos=0x%X max_filepos=x%X\n", read,s->min_filepos, s->max_filepos);
		// streaming: drop cache contents only if seeking backward or too much fwd:
		if (read < s->min_filepos || read >= s->max_filepos + s->buffer_size)
		{
			s->offset =			// FIXME!?
				s->min_filepos = s->max_filepos = read;	// drop cache content :(
			if (s->stream->eof)
				stream_reset(s->stream);
			stream_seek(s->stream, read);
		}
	}

	// calc number of back-bytes:
	back = read - s->min_filepos;
	if (back < 0)
		back = 0;				// strange...
	if (back > s->back_size)
		back = s->back_size;

	// calc number of new bytes:
	newb = s->max_filepos - read;
	if (newb < 0)
		newb = 0;				// strange...

	// calc free buffer space:
	space = s->buffer_size - (newb + back);

	// calc bufferpos:
	pos = s->max_filepos - s->offset;
	if (pos >= s->buffer_size)
		pos -= s->buffer_size;	// wrap-around


	if (space < s->fill_limit)
	{
		//  EXCHANGE();             /* Meng 05122005 */
		return 0;				// no fill...
	}


	// reduce space if needed:
	if (space > s->buffer_size - pos)
		space = s->buffer_size - pos;
	if (space > 4 * s->sector_size)
		space = 4 * s->sector_size;

	// back+newb+space <= buffer_size
	back2 = s->buffer_size - (space + newb);	// max back size
	if (read >= back2)
		if (s->min_filepos < (read - back2))
			s->min_filepos = read - back2;

	len = stream_read(s->stream, (char *) (&s->buffer[pos]), space);
	if (!len)
	{
		s->eof = 1;
		//   EXCHANGE();               /* Meng 05122005 */
	}
	s->max_filepos += len;
	if (pos + len >= s->buffer_size)
	{
		s->offset += s->buffer_size;
	}
	return len;
}


/*!
	@ingroup	STREAM_CACHE

	@brief		Cache fill routine task

	@remark		Once created, it will run endlessly to check the cache start flag: CacheStartOrNot. If the flag is set,
	            it will call relative function to fill cache. When filling cache, it use a flag CacheFillCompleted for 
	            indicating a cache fill action is running.
*/
static void StreamCacheTask()
{
	int ret;
	DWORD release;
	while (1)
	{
		EventWait(STREAM_CACHE_EVENT, EVENT_CACHE_START, OS_EVENT_OR, &release);
		EventClear(STREAM_CACHE_EVENT, ~EVENT_CACHE_END);
		ret = cache_fill(CacheVar);
		EventSet(STREAM_CACHE_EVENT, EVENT_CACHE_END);
		//if (ret)	mpDebugPrint("cache_fill=0x%x", ret);
		TaskSleep(1);
		//TaskYield();
	}
}


/*!
	@ingroup	STREAM_CACHE

	@brief	Enable the cache for a specific stream  

	@param	stream_t *stream		The handle of stream to enable cache

	@remark This function is always called for  the up-layer stream(between stream cache and demux).
			It will create and initialize its stream cache and low-layer stream(between stream cache and FAT),
			then start the cache filling routine task.

*/
void stream_enable_cache(stream_t * stream)
{
	// stream under cache
	stream_t *s = (stream_t *) mem_malloc(sizeof(stream_t));

	if (!s)
		return;
	CacheVar = cache_init(STREAM_CACHE_SIZE, STREAM_BUFFER_SIZE);
	stream->cache_data = CacheVar;	// stream upon cache

	s->fill_buffer = stream->fill_buffer;
	s->write_buffer = stream->write_buffer;
	s->seek = stream->seek;
	s->close = stream->close;
	s->fd = stream->fd;
	s->buf_pos = stream->buf_pos;
	s->buf_len = stream->buf_len;
	s->pos = stream->pos;
	s->start_pos = stream->start_pos;
	s->end_pos = stream->end_pos;
	s->eof = stream->eof;
	s->cache_data = 0;			// as uncaching flag
	s->buffer = StreamBuffer_2;

	CacheVar->stream = s;		// data read into cache buffer by uncaching stream_t
	if (!createdone)
	{
		EventCreate(STREAM_CACHE_EVENT, OS_ATTR_FIFO|OS_ATTR_WAIT_MULTIPLE, EVENT_CACHE_END);
		
		TaskCreate(STREAM_CACHE_TASK, StreamCacheTask, CONTROL_PRIORITY, 0x4000);
		TaskStartup(STREAM_CACHE_TASK);
		createdone = 1;
	}

	EventClear(STREAM_CACHE_EVENT, 0);
	EventSet(STREAM_CACHE_EVENT, EVENT_CACHE_START|EVENT_CACHE_END);
}





/*!
	@ingroup	STREAM_CACHE

	@brief	Read data from cache buffer into detination address 

	@param	cache_vars_t *s		The handle of cache to read

	@param  BYTE *buf			Destination address for data stores to

	@param  int size			Max size to read

	@return	The function call will return the number of bytes read.
*/
static int cache_read(cache_vars_t * s, BYTE * buf, int size)
{
	int total = 0;

	while (size > 0)
	{
		int pos, newb, len;

		if (s->read_filepos >= s->max_filepos || s->read_filepos < s->min_filepos)
		{
			// eof?
			if (s->eof)
				break;

			TaskYield();		//CONTEXT_SWITCH();     /* Meng 04132005 */
			continue;			// waiting for cache data ready
		}

		newb = s->max_filepos - s->read_filepos;	// new bytes in the buffer


		/* Meng 050415 starts */
		/*
		 *
		 *
		 */
		pos = s->read_filepos - s->offset;
		if (pos < 0)
			pos += s->buffer_size;
		else if (pos >= s->buffer_size)
			pos -= s->buffer_size;
		/* Meng 050415 ends */


		if (newb > s->buffer_size - pos)
			newb = s->buffer_size - pos;	// handle wrap...
		if (newb > size)
			newb = size;

		// check
		if (s->read_filepos < s->min_filepos)	//is it possible?????Meng
			;					//MP_DEBUG("Ehh. s->read_filepos<s->min_filepos !!! Report bug...\n");

		memcpy(buf, &s->buffer[pos], newb);
		buf += newb;
		len = newb;

		s->read_filepos += len;
		size -= len;
		total += len;
	}

	return total;
}



/*!
	@ingroup	STREAM_CACHE

	@brief	Fill a specific stream's buffer  using the data from its cache buffer

	@param	stream_t *stream		The handle of stream to fill

	@return	The function call will return 0 if no byte is  filled into the stream buffer ,
	         else will return the number of bytes filled into the  stream buffer.
*/
int cache_stream_fill_buffer(stream_t * stream)
{
	int len;
	cache_vars_t *s = stream->cache_data;

	if (!s)						// uncaching read into cache buffer
	{
        stream->from_stream_seek_long = FALSE;
		return stream_fill_buffer(stream);
	}	

	if (stream->eof)
	{
		stream->buf_pos = stream->buf_len = 0;
		return 0;
	}

	if (stream->pos != s->read_filepos)
		;
	//MP_DEBUG("!!! read_filepos differs!!! report this bug...\n");

	len = cache_read(s, stream->buffer, s->sector_size);

	if (len <= 0)
	{
		stream->eof = 1;
		stream->buf_pos = stream->buf_len = 0;
		return 0;
	}

	stream->buf_pos = 0;
	stream->buf_len = len;
	stream->pos += len;

	return len;
}



/*!
	@ingroup	STREAM_CACHE

	@brief	Set the stream's current read cursor the new file position

	@param	stream_t *stream		The handle of stream to seek

	@param	unsigned int  pos				File position to seek

	@return	The function call will return 0 if seeking fails ,
	         else will return 1.

	@remark  If seek successfullu, the stream's buffer will be filled with
			data around this new position read from its cache. And the stream buffer's
			read cursor will point to the exact new file position byte.
*/
int cache_stream_seek_long(stream_t * stream, unsigned int pos)
{
	off_t newpos;
	cache_vars_t *s = stream->cache_data;


	if (!s)						// uncaching seek by cache
		return stream_seek_long(stream, pos);

	newpos = pos / s->sector_size;
	newpos *= s->sector_size;	// align
	stream->pos = s->read_filepos = newpos;
	s->eof = 0;

    stream->from_stream_seek_long = FALSE;
	cache_stream_fill_buffer(stream);

	pos -= newpos;
	if (pos >= 0 && pos <= stream->buf_len)
	{
		stream->buf_pos = pos;	// byte position in sector
		return 1;
	}

	return 0;
}


#endif // __USE_STREAM_CACHE__
#endif
