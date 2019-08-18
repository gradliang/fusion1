/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      stream_file.c
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

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/

#include "mpTrace.h"

#ifndef TASK_ID_H
#include "taskid.h"
#endif

/*!
    @defgroup STREAM_FAT   Stream & FAT  
*/

/*
*******************************************************************************
*        #include
*******************************************************************************
*/
#include "stream.h"
#include "index.h"

#ifndef __FILTER_GRAPH__
#include "filter_graph.h"
#endif


extern S_BK_STREAM_NODE_T* g_BkStreamBufHead;
extern S_BK_STREAM_NODE_T* g_BkStreamBufTail;
extern filter_graph_t filter_graph;
extern BYTE g_bAudioCodecInit;

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
static int fill_buffer(stream_t * s, BYTE * buffer, int max_len)
{ 
	int read_len;
	int local_device = 1;

	if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) ||\
            (DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID) ||\
            (DEV_USB_ETHERNET_DEVICE == s->fd->Drv->DevID) ||\
            (DEV_USB_WEBCAM == s->fd->Drv->DevID) )
	{
		local_device = 0;
	}	   
	
	if (local_device != 0)
	{
		#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING
		read_len = Fstream_FileRead(s->fd, buffer, max_len);
		#else
		read_len = FileRead(s->fd, buffer, max_len);
		#endif
	}
	else
	{	  	    
		if (g_BkStreamBufHead != NULL)// && !s->from_stream_seek_long)
		{			
			S_BK_STREAM_NODE_T* p_curr_bk_stream_node = g_BkStreamBufHead;
	        while (p_curr_bk_stream_node != NULL)
			{	
			    int len = (s->pos + max_len) >= s->end_pos ? s->end_pos - s->pos : max_len;	            
	            if (((s->pos + len) > (p_curr_bk_stream_node->pos - p_curr_bk_stream_node->buf_len)) && ((s->pos + len) <= p_curr_bk_stream_node->pos))
	            {
					S_BK_STREAM_NODE_T* p_next = p_curr_bk_stream_node->p_next;
					int buf_len = (int) p_curr_bk_stream_node->buf_len;
					if (buf_len != len)
						return len;	            
	                #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
                    mmcp_memcpy_polling(buffer, p_curr_bk_stream_node->buffer, p_curr_bk_stream_node->buf_len);
                    #else	
				    memcpy(buffer, p_curr_bk_stream_node->buffer, p_curr_bk_stream_node->buf_len);
			        #endif

					S_BK_STREAM_NODE_T* p_del;
					for (p_del = g_BkStreamBufHead; p_del != p_next; p_del = p_del->p_next)
					{
			            mem_free(p_del);						
					}	
			        g_BkStreamBufHead = p_next;
				    return buf_len;
	            }
				p_curr_bk_stream_node = p_curr_bk_stream_node->p_next;
	        } 			
		}		
    		
		if (s->fd->Chain.Point != s->pos)
		{		    
            BYTE task_id = TaskGetId();
			mpDebugPrint("-E s->fd->Chain.Point != s->pos: task_id = %d, s->fd->Chain.Point = 0x%x, s->pos = 0x%x", task_id, s->fd->Chain.Point, s->pos);						
		    return -1;
		}
	    read_len = FileRead(s->fd, buffer, max_len);		                        
        #if (0 == PURE_VIDEO)
        if ((filter_graph.demux->sh_audio != NULL) || (0 == g_bAudioCodecInit))
        {            
            if (read_len > 0)
            {
                S_BK_STREAM_NODE_T* p_bk_stream_node = (S_BK_STREAM_NODE_T*)mem_malloc(sizeof(S_BK_STREAM_NODE_T)); 
			    memset(p_bk_stream_node, 0, sizeof(S_BK_STREAM_NODE_T));
		        p_bk_stream_node->p_next = NULL;
		        if (NULL == g_BkStreamBufHead)
		        {		    
	                p_bk_stream_node->p_prev = NULL;
	                g_BkStreamBufHead = p_bk_stream_node; 
                    g_BkStreamBufTail = p_bk_stream_node;
		        }
		        else
		        {		        
		            p_bk_stream_node->p_prev = g_BkStreamBufTail;
			        g_BkStreamBufTail->p_next = p_bk_stream_node;
		            g_BkStreamBufTail = p_bk_stream_node;	
		        }	
			    
			    #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
                mmcp_memcpy_polling(p_bk_stream_node->buffer, buffer, read_len);
                #else	
				memcpy(p_bk_stream_node->buffer, buffer, read_len);
			    #endif
			    p_bk_stream_node->pos = s->pos + read_len; 
			    p_bk_stream_node->buf_len = read_len;			    
            }	
		}				
		#endif
	}
	
	return read_len;
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
static int write_buffer(stream_t * s, BYTE * buffer, int len)
{
	int r = FileWrite(s->fd, buffer, len);
	return (r <= 0) ? -1 : r;
}


/*!
	@ingroup	STREAM_FAT

	@brief	Move file handle's read cursor to new position  

	@param	stream_t  *s		The handle of stream , which has the pointer to file handle for operating with

	@param	unsigned int 	  newpos	New file position

	@return	The function call will return 0 if seek fails ,
	         else will return 1.
*/
static int seek(stream_t * s, unsigned int newpos)
{
    int r;
    int local_device = 1;

	if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) ||\
           (DEV_CF_ETHERNET_DEVICE== s->fd->Drv->DevID) ||\
           (DEV_USB_ETHERNET_DEVICE == s->fd->Drv->DevID) ||\
           (DEV_USB_WEBCAM == s->fd->Drv->DevID))
	{
		local_device = 0;
	}	   	  
 
    if (local_device != 0)
	{ 
	    #if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING	
	    r = Fstream_Seek(s->fd, newpos);
		#else
		r = Seek(s->fd, newpos);
		#endif
	    s->pos = newpos;
		return (r < 0)? 0:1;
	}	
    else //follow normal flow in file.c and chain.c
    {
    #if 0 // YouTube
	    r = Seek(s->fd, newpos);
	    if(r == 0) //success
	#endif		
		s->pos = newpos;
	    return 1;
    }		
}



/*
*******************************************************************************
*        GLOBAL FUNCTIONS
*******************************************************************************
*/

void close_file(stream_t * s)
{
	int local_disk = 1;
	
	if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) ||\
           (DEV_CF_ETHERNET_DEVICE== s->fd->Drv->DevID) ||\
           (DEV_USB_ETHERNET_DEVICE == s->fd->Drv->DevID) ||\
           (DEV_USB_WEBCAM == s->fd->Drv->DevID))
	{
		local_disk = 0;
	}	   
	   
#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING	
    if (local_disk != 0)
	{
	    Clear_FileChainClustersCache();
	}	
#endif
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
int open_file(stream_t * stream, STREAM * sHandle)
{
	uint32_t len;

	if (!sHandle)
	{
		MP_DEBUG("open_file(): -E- invalid file handle !");
		return STREAM_ERROR;
	}

    int local_disk = 1;
	mpDebugPrint("open_file %d",sHandle->Drv->DevID);
	if ((DEV_USB_WIFI_DEVICE == sHandle->Drv->DevID) ||\
            (DEV_CF_ETHERNET_DEVICE == sHandle->Drv->DevID) ||\
            (DEV_USB_ETHERNET_DEVICE == sHandle->Drv->DevID) ||\
            (DEV_USB_WEBCAM == sHandle->Drv->DevID))
	{
		local_disk = 0;
	}	   	           

#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING	
    if (local_disk != 0)
	{
	   if (InitPreload_FileChainClustersCache(sHandle) != PASS)
	   {
	   	  if (! SystemCardPresentCheck(sHandle->Drv->DrvIndex))
		  	return STREAM_ERROR;
	   }	
	   SeekSet(sHandle); //reset chain info to its initial value
	}   
#endif

	len = FileSizeGet(sHandle);
    mpDebugPrint("len %d",len);
	SeekSet(sHandle);

	stream->seek = seek;
	stream->end_pos = stream->file_len = len;
	stream->fd = sHandle;
	stream->fill_buffer = fill_buffer;
	stream->write_buffer = write_buffer;
    stream->close = close_file;
	return STREAM_OK;
}

#endif


