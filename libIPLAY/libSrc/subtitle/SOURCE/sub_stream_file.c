/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      sub_stream_file.c
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
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/


/*!
    @defgroup STREAM_FAT   Stream & FAT  
*/

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#ifndef __GLOBAL612_H
#include "global612.h"
#endif

#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)
#include "mpTrace.h"
#include "stream.h"
//#include "index.h"


/*
*******************************************************************************
*        LOCAL FUNCTIONS
*******************************************************************************
*/
/*!
	@ingroup	STREAM_FAT

	@brief	Read the stream's file handle's content  

	@param	stream_t  *s		The handle of stream , which file handle is for data accessing

	@param	BYTE 	 *buffer  	Destination address for storing data read from stream's file handle

	@param	int 	 max_len  	Max number of bytes to read


	@return	The function call will return -1 if no byte is read ,
	         else will return the number of bytes read from file handle.
*/
static int sub_fill_buffer(stream_t * s, BYTE * buffer, int max_len)
{
	int r;
	
	r = FileRead(s->fd, buffer, max_len);	
	return (r <= 0) ? -1 : r;
}


/*!
	@ingroup	STREAM_FAT

	@brief	Write the data into file handle   

	@param	stream_t  *s		The handle of stream, whose file handle is for data writing to

	@param	BYTE 	 *buffer  	Source data address

	@param	int 	 len  		Number of bytes to write

	@return	The function call will return -1 if no byte is  write the stream's file handle ,
	         else will return the number of bytes written.
*/
static int sub_write_buffer(stream_t * s, BYTE * buffer, int len)
{
	int r = FileWrite(s->fd, buffer, len);
	
	return (r <= 0) ? -1 : r;
}


/*!
	@ingroup	STREAM_FAT

	@brief	Move file handle's read cursor to new position  

	@param	stream_t  *s		The handle of stream , which has the pointer to file handle for operating with

	@param	unsigned int  newpos	New file position

	@return	The function call will return 0 if seek fails ,
	         else will return 1.
*/
static int sub_seek(stream_t * s, unsigned int newpos)
{
    int r;

	r = Seek(s->fd, newpos);
    if (r == 0) //success
		s->pos = newpos;

	return (r < 0)? 0:1;
}



/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/

void sub_close_file(stream_t * s)
{
	FileClose(s->fd);
}

/*!
*	@ingroup	STREAM_FAT
*
*	@brief	Open the current choosen file of a specific drive, and initialize stream's elements.    
*
*	@param	stream_t *stream		The handle of stream to initialize
*
*	@param	DRIVE *drv				The handle of drive to open
*
*	@return	STREAM_OK			Open ok.
*	@return	STREAM_ERROR		Open fails
*/
int sub_open_file(stream_t * stream, STREAM * sHandle)
{
	uint32_t len;

	if (!sHandle)
	{
		MP_DEBUG("open_file(): -E- invalid file handle !");
		return STREAM_ERROR;
	}    

	len = FileSizeGet(sHandle);
	SeekSet(sHandle);

	stream->seek = sub_seek;
	stream->end_pos = stream->file_len = len;
	stream->fd = sHandle;
	stream->fill_buffer = sub_fill_buffer;
	stream->write_buffer = sub_write_buffer;
    stream->close = sub_close_file;
	return STREAM_OK;
}

#endif


