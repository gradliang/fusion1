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
#include "global612.h"
#if (VIDEO_ON || AUDIO_ON)

/*!
    @defgroup STREAM    Stream access
*/

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#include "stream.h"
#include "index.h"
#include "mpTrace.h"

#ifndef STREAM_TYPE_H
#include "stream_type.h"
#endif

#ifndef TASK_ID_H
#include "taskid.h"
#endif

#ifndef xpg__H__
#include "xpg.h"
#endif

/*
*******************************************************************************
*        GLOBAL VARIABLE
*******************************************************************************
*/
#if VIDEO_ON
BYTE __attribute__ ((aligned(4))) StreamBuffer_1[STREAM_BUFFER_SIZE];
BYTE __attribute__ ((aligned(4))) StreamBuffer_a[STREAM_BUFFER_SIZE];
#else
BYTE __attribute__ ((aligned(4))) StreamBuffer_1[4];
BYTE __attribute__ ((aligned(4))) StreamBuffer_a[4];
#endif
#ifdef __USE_STREAM_CACHE__
	BYTE __attribute__ ((aligned(4))) StreamBuffer_2[STREAM_BUFFER_SIZE + 4];
#endif

extern BYTE CacheFillCompleted;
extern XPG_FUNC_PTR* g_pXpgFuncPtr;
extern int g_iMovieInstructionBk;

S_BK_STREAM_NODE_T* g_BkStreamBufHead = NULL;
S_BK_STREAM_NODE_T* g_BkStreamBufTail = NULL;

#if STREAM_READ_COUNT
unsigned int stream_read_char_count		= 0;
unsigned int stream_read_count			= 0;
unsigned int stream_read_dword_le_count	= 0;
#endif

/*
*******************************************************************************
*        EXTERNAL FUNCTIONS
*******************************************************************************
*/
extern int open_file(stream_t * stream, STREAM * sHandle);


/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/




// reset the stream element's pointers
void stream_reset(stream_t * s)
{
	if (s->eof)
	{
		s->pos = 0;
		s->eof = 0;
		SeekSet(s->fd);
        while (g_BkStreamBufHead != NULL)
		{
			S_BK_STREAM_NODE_T* p_next = g_BkStreamBufHead->p_next;
			mem_free(g_BkStreamBufHead);
			g_BkStreamBufHead = p_next;
		}
	}
}




/*!
	@ingroup	STREAM
	@brief	Create a stream for the current choosen file of specific drive
	@param	DRIVE *drv		The handle of drive to open
	@return	The function call will return null if no stream handle can use,
	         else will return the point of stream handle.
*/
stream_t *stream_create(register STREAM * sHandle)
{
	int ret;
	stream_t *s = (stream_t *) mem_malloc(sizeof(stream_t));

	if (s == NULL)
	{
		return NULL;
	}
	memset(s, 0, sizeof(stream_t));

	s->buffer = (BYTE *)((DWORD)StreamBuffer_1 );

	ret = open_file(s, sHandle);

	if (ret != STREAM_OK)
	{
		mem_free(s);
		return NULL;
	}

	// stream caching go
	stream_enable_cache(s);
    s->from_stream_seek_long = FALSE;
	if (!cache_stream_fill_buffer(s)) return NULL; // EOF

	return s;
}

/*!
	@ingroup	STREAM
	@brief	Create a audio stream, not really open file, but maintain its own pointer
	@param	stream_v	video stream from which its parameter copy
	@return	The function call will return null if no stream handle can use
*/
stream_t *av_stream_create(stream_t *stream_v)
{
	stream_t * const stream_a = (stream_t *) mem_malloc(sizeof(stream_t));

	if (!stream_a)	return NULL;
	memset(stream_a, 0, sizeof(stream_t));

    int local_device = 1;
    
	if ((DEV_USB_WIFI_DEVICE == stream_v->fd->Drv->DevID) ||\
            (DEV_CF_ETHERNET_DEVICE == stream_v->fd->Drv->DevID) ||\
            (DEV_USB_ETHERNET_DEVICE == stream_v->fd->Drv->DevID) ||\
            (DEV_USB_WEBCAM == stream_v->fd->Drv->DevID))
	{
		local_device = 0;
	}    

	if (0 == local_device)
	{
	    stream_a->pos = stream_a->fd->Chain.Point;
	}

	stream_a->buffer = (BYTE *)((DWORD)StreamBuffer_a);
	stream_a->seek = stream_v->seek;
	stream_a->start_pos = stream_a->pos = 0;
	stream_a->end_pos = stream_a->file_len = stream_v->end_pos;
	stream_a->fd = stream_v->fd;
	stream_a->fill_buffer = stream_v->fill_buffer;
	stream_a->write_buffer = stream_v->write_buffer;

    stream_v->from_stream_seek_long = FALSE;
	if (!cache_stream_fill_buffer(stream_a)) return NULL; // EOF
	return stream_a;
}


/*!
	@ingroup	STREAM

	@brief	Fill a specific stream's buffer

	@param	stream_t *s		The handle of stream to fill

	@return	The function call will return 0 if no byte is  filled into the stream buffer ,
	         else will return the number of bytes filled into the  stream buffer.
	@remark If cache enbaled, data source is from cache buffer, else from file directly
*/
int stream_fill_buffer(register stream_t * s)
{
	register int len = 0;
    int local_device = 1;
    
	if (DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID || DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID )
	{
		local_device = 0;
	}

    if (s->pos >= FileSizeGet(s->fd)) /* note: here is (s->pos '>=' FileSizeGet(s->fd) for EOF reached */
    {
	    s->eof = 1;
        return 0;
    }


    if (local_device)
    {
	    if (s->pos != FilePosGet(s->fd))
	    {
		    if (!s->seek)
		    {
		        mpDebugPrint("stream_fill_buffer return 0");
			    return 0;
		    }

		    if (!s->seek(s, s->pos))
		    {
		        mpDebugPrint("stream_fill_buffer return 00");
			    return 0;
		    }
	    }
    }

	if (s->fill_buffer)
	{
#ifdef __USE_STREAM_CACHE__
		s->buffer = StreamBuffer_2;
#endif
        if ((0 == local_device) && !s->from_stream_seek_long)
        {
            SemaphoreWait(STREAM_READ_SEMA_ID);
	    }
		len = s->fill_buffer(s, s->buffer, STREAM_BUFFER_SIZE);
	}

    if (0 == local_device) // YouTube
    {
        if (len < 0)
		{
	        s->eof = 1;
		    s->buf_pos = s->buf_len = 0;

			if (!s->from_stream_seek_long)
			{
                SemaphoreRelease(STREAM_READ_SEMA_ID);
			}

		    return 0;
	    }
		else if((len < STREAM_BUFFER_SIZE) && !s->eof)
		{
		    BOOL show_loading = FALSE;
            if (0 == len)
            {
                show_loading = TRUE;
				#if NETWARE_ENABLE
				if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateYouTubeLoading)
				    g_pXpgFuncPtr->xpgUpdateYouTubeLoading(1);
				#endif
            }
			int read_len;
			int req_len;
			BYTE* buffer_ptr = (BYTE*) (s->buffer + len);
			s->buf_len = len;
			s->pos += len;
		    req_len = STREAM_BUFFER_SIZE - len;

		    do
		    {
		       if ((MOVIE_VIDEO_STOP == g_iMovieInstructionBk)  ||
			   	   (MOVIE_VIDEO_CLOSE == g_iMovieInstructionBk)
			   	  )
		       {
				   s->eof = 1;
		           s->buf_pos = s->buf_len = 0;
				   if (!s->from_stream_seek_long)
		               SemaphoreRelease(STREAM_READ_SEMA_ID);
		           return 0;
               }
		       TaskSleep(500);
			   read_len = s->fill_buffer(s, buffer_ptr, req_len);
			   if (read_len < 0)
			   {
			      if (!s->from_stream_seek_long)
		               SemaphoreRelease(STREAM_READ_SEMA_ID);
		          return 0;
			   }
			   buffer_ptr = (BYTE*) (s->buffer + read_len);
			   if ((0 == read_len) && !show_loading)
               {
                    show_loading = TRUE;
					#if NETWARE_ENABLE
					if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateYouTubeLoading)
				        g_pXpgFuncPtr->xpgUpdateYouTubeLoading(1);
					#endif
               }

	           s->buf_len += read_len;
	           s->pos += read_len;
			   req_len -= read_len;
			   if (s->pos >= s->end_pos)
               {
                  s->eof = 1;
               }
		    } while ((req_len > 0) && !s->eof);
			s->buf_pos = 0;

			if (show_loading)
			{
			    show_loading = FALSE;
				#if NETWARE_ENABLE
				if (g_pXpgFuncPtr && g_pXpgFuncPtr->xpgUpdateYouTubeLoading)
				    g_pXpgFuncPtr->xpgUpdateYouTubeLoading(0);
				#endif
			}
		}
		else
		{
		   s->buf_pos = 0;
	       s->buf_len = len;
	       s->pos += len;
		}
    }
	else // local disk
	{
	    if (len <= 0) // end of file or read error
	    {
	        s->eof = 1;
		    s->buf_pos = s->buf_len = 0;
		    return 0;
	    }
		s->buf_pos = 0;
	    s->buf_len = len;
	    s->pos += len;
	}

    if (s->pos > s->end_pos)
    {
        s->buf_len -= s->pos - s->end_pos;
		s->pos = s->end_pos;
    }

	if ((0 == local_device) && !s->from_stream_seek_long)
    {
        SemaphoreRelease(STREAM_READ_SEMA_ID);
	}


	return s->buf_len;
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

int stream_seek_long(register stream_t * s, unsigned int pos)
{
	s->from_stream_seek_long = TRUE;

    unsigned int temp_pos = pos;
	int local_device = 1;
    
	if (DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID|| DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID)
	{
		local_device = 0;
	}
    
    if (0 == local_device)
    {
        SemaphoreWait(STREAM_READ_SEMA_ID);

    }

	unsigned int newpos = 0;

	s->buf_pos = 0;
	s->buf_len = 0;
	newpos = temp_pos & (~(STREAM_BUFFER_SIZE - 1));
	temp_pos -= newpos;



    if (local_device)
    {
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
		stream_fill_buffer(s);
    }
    else
    {
	    if ((s->pos - s->buf_len <= pos) && (s->pos > pos))
	    {
		    // This should at the beginning as soon as all streams are converted
		    if (!s->seek)
		    {
                s->from_stream_seek_long = FALSE;
			    return 0;
		    }

			// Now seek
		    if (!s->seek(s, newpos))
		    {
		        mpDebugPrint("seek fail");
			    s->from_stream_seek_long = FALSE;
			    return 0;
		    }
	    }



	    if (g_BkStreamBufHead != NULL)
        {
		    S_BK_STREAM_NODE_T* p_curr_bk_stream_node = g_BkStreamBufHead;
	        while (p_curr_bk_stream_node != NULL)
		    {
	            if ((pos >= p_curr_bk_stream_node->pos - p_curr_bk_stream_node->buf_len) && (pos < p_curr_bk_stream_node->pos))
	            {
	                memcpy(s->buffer, p_curr_bk_stream_node->buffer, p_curr_bk_stream_node->buf_len);
				    s->pos = p_curr_bk_stream_node->pos;
				    s->buf_len = p_curr_bk_stream_node->buf_len;
				    break;
	            }
			    p_curr_bk_stream_node = p_curr_bk_stream_node->p_next;
	        }
	    }

        while (s->pos < pos && !s->eof)
        {
            stream_fill_buffer(s);
        }
    }

	if (temp_pos >= 0 && temp_pos <= s->buf_len)
	{
		s->buf_pos = temp_pos;		// byte position
		s->from_stream_seek_long = FALSE;

        SemaphoreRelease(STREAM_READ_SEMA_ID);
		return 1;
	}

    BYTE task_id = TaskGetId();
    mpDebugPrint("stream_seek_long after task_id = %d, newpos = 0x%x", task_id, newpos);
	mpDebugPrint("stream_seek_long after task_id = %d, s->fd->Chain.Point = 0x%x, s->pos = 0x%x, s->start_pos = 0x%x, s->end_pos = 0x%x", task_id, s->fd->Chain.Point, s->pos, s->start_pos, s->end_pos);
	mpDebugPrint("stream_seek_long after task_id = %d, s->buf_pos = 0x%x, s->buf_len = 0x%x", task_id, s->buf_pos, s->buf_len);
	mpDebugPrint("stream_seek_long return 0");

	s->from_stream_seek_long = FALSE;
    SemaphoreRelease(STREAM_READ_SEMA_ID);
	return 0;
}




/*!
	@ingroup	STREAM

	@brief	Free a stream's relative resources

	@param	stream_t *s		The handle of stream to free

	@return
*/
void stream_free(stream_t * s)
{

#ifdef __USE_STREAM_CACHE__
	if (s->cache_data)			// stream upon cache
	{
		cache_uninit(s);
		s->cache_data = 0;
	}
#endif
	close_file(s);
	s->fd = 0;
	s->buffer = 0;
	mem_free(s);
	s = 0;
}

/*!
	@ingroup	STREAM

	@brief	Create a new stream using the given buffer

	@param	unsigned char* data	Data address to create an stream

	@param	int len				Data length

	@return	The function call will return 0 if stream create fails ,
	         else will return the handle of the new stream.
*/
stream_t *new_memory_stream(unsigned char *data, int len)
{
	stream_t *s = (stream_t *) mem_malloc(sizeof(stream_t) + len);

	memset(s, 0, sizeof(stream_t));
	s->fd = 0;
	s->buf_pos = 0;
	s->buf_len = len;
	s->start_pos = 0;
	s->end_pos = len;
	stream_reset(s);
	s->pos = len;
	s->buffer = (BYTE *)((DWORD)StreamBuffer_1);

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
