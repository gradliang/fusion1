/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/

#include "global612.h"
#include "mpTrace.h"
#include "mpStreamApi.h"

#define _16K (16 << 10)
#define _32K (32 << 10)
#define _64K (64 << 10)
#define _128K (128 << 10)		
#define _256K (256 << 10)		
//#define BUFFER_FILL_SIZE _128K	// abel 20070802 change from 64KB to 128KB
#define BUFFER_FILL_SIZE _256K

#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
static BYTE g_ImgUseFileChainFlag = DISABLE; /*DISABLE: not use FILE CHAIN, ENABLE: Use FILE CHAIN*/
#endif

#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING

void Set_Img_FileChainFlag(void)
{
	g_ImgUseFileChainFlag = ENABLE;
}

void Clear_Img_FileChainFlag(void)
{
	g_ImgUseFileChainFlag = DISABLE;
}

BYTE Get_Img_FileChainFlag(void)
{
	return g_ImgUseFileChainFlag;
}

#endif
//---------------------------------------------------------------------------
void swap_buffer_endian(BYTE *buffer, size_t size, size_t count)
{
	int i;
	BYTE bTemp0, bTemp1, bTemp2, bTemp3;

	if (size == 1) return;

	if (size == 2) {
		for (i = 0; i < count; i++) {
			bTemp0 = buffer[0];
			buffer[0] = buffer[1];
			buffer[1] = bTemp0;
			buffer += 2;
		}
	}
	else if (size == 4) {
		for (i = 0; i < count; i++) {
			bTemp0 = buffer[0];
			bTemp1 = buffer[1];
			bTemp2 = buffer[2];
			bTemp3 = buffer[3];
			buffer[0] = bTemp3;
			buffer[1] = bTemp2;
			buffer[2] = bTemp1;
			buffer[3] = bTemp0;			
			buffer += 4;
		}
	}
}


static int _stream_seek_(ST_MPX_STREAM * s, DWORD newpos)
{
    //MP_DEBUG2("_stream_seek_ pos %x, size %x", newpos, s->file_len);
    // check if out of range
    if (s->fd && !s->fd->Drv->Flag.Present)
    {
        MP_ALERT("%s: (s->fd && !s->fd->Drv->Flag.Present) => return FAIL;", __FUNCTION__);
        return FAIL;
    }

    if (newpos > s->file_len)
    {
        MP_DEBUG("%s: Warning: (newpos (0x%x) > s->file_len (0x%x))", __FUNCTION__, newpos, s->file_len);
    }

    if (newpos >= s->file_len)
    {
        newpos = s->file_len - 1;
        s->eof = 1;
        return FAIL;
    }

    // check if in the range of catch buffer
    if ((newpos >= s->pos) && ((newpos - s->pos) < s->buf_len))
    {
        s->buf_pos = newpos - s->pos;
        return PASS;
    }

    if (!s->fd)
    {
        s->buf_pos = newpos;
        return PASS;
    }

    s->buf_pos = 0;
    s->buf_len = 0;	

    BOOL f_local_device = TRUE;
    if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) || (DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID))
        f_local_device = FALSE;
	
    // Now seek to newpos
    if (f_local_device)
    {
#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
      ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
      if(Get_Img_FileChainFlag() == ENABLE)
      {
        if (psSysConfig->dwCurrentOpMode != OP_MOVIE_MODE)
        {
            if (!Fstream_Seek(s->fd, newpos))
            {
                s->pos = newpos;
                s->file_ptr = newpos;
                return PASS;
            }
        }
        else
        {
            if (!Seek(s->fd, newpos))
            {
                s->pos = newpos;
                s->file_ptr = newpos;
                return PASS;
            }
        }
      }
      else
      {
        if (!Seek(s->fd, newpos))
        {
            s->pos = newpos;
            s->file_ptr = newpos;
            return PASS;
        }
      }
#else	
        if (!Seek(s->fd, newpos))
        {
            s->pos = newpos;
            s->file_ptr = newpos;
            return PASS;
        }
#endif
    }
    else
    {
        if (!Seek(s->fd, newpos))
        {
            s->pos = newpos;
            s->file_ptr = newpos;
            return PASS;
        }
    }

    MP_ALERT("%s: Seek() failed, return FAIL;", __FUNCTION__);
    return FAIL;
}


static int _memory_seek_(ST_MPX_STREAM * s, DWORD newpos)
	{
	//MP_DEBUG2("_stream_seek_ pos %x, size %x", newpos, s->file_len);
	// check if out of range
		
	if (newpos >= s->file_len)
	{
		newpos = s->file_len - 1;
		s->eof = 1;
		MP_ALERT("%s: (newpos (0x%x) >= s->file_len (0x%x)) => return FAIL;", __FUNCTION__, newpos, s->file_len);
		return FAIL;
	}

	// check if in the range of catch buffer
	if ((newpos >= s->pos) && ((newpos - s->pos) < s->buf_len))
	{
		s->buf_pos = newpos - s->pos;
		return PASS;
	}
	
	if (!s->fd)
	{
		s->buf_pos = newpos;
		return PASS;
	}

	s->buf_pos = 0;
	s->buf_len = 0;	
	
	// Now seek to newpos
	s->source->offset = newpos;
	s->pos = newpos;
	s->file_ptr = newpos;

	return PASS;
}


//---------------------------------------------------------------------------
static int fill_catch_buffer(ST_MPX_STREAM * s, int size)
{
    DWORD len = 0;

    if (s->fd && !s->fd->Drv->Flag.Present)
    {
        MP_DEBUG("%s: (s->fd && !s->fd->Drv->Flag.Present) => return 0;", __FUNCTION__);
        return 0;
    }

    if (s->pos >= s->file_len)
    {							// end of file
        s->eof = 1;
        s->buf_pos = s->buf_len = 0;
        MP_DEBUG("%s: (s->pos (0x%x) >= s->file_len (0x%x)) => return 0;", __FUNCTION__, s->pos, s->file_len);
        return 0;
    }

    if (s->fd) 
    {
//Frank Lin add
        BOOL f_local_device = TRUE;
        if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) || (DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID))
            f_local_device = FALSE;

        if (g_bAniFlag & ANI_AUDIO)
            TaskYield();

        if (s->file_ptr != FilePosGet(s->fd))
        {
            MP_DEBUG2("pos %x, fd pos %x", s->file_ptr, FilePosGet(s->fd));
            if (f_local_device)
            {
    #if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
          if(Get_Img_FileChainFlag() == ENABLE)
          {
                ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
                if (psSysConfig->dwCurrentOpMode != OP_MOVIE_MODE)
                {
                    if (Fstream_Seek(s->fd, s->file_ptr) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
                        return 0;
                    }
                }
                else
                {
                    if (Seek(s->fd, s->file_ptr) != FS_SUCCEED)
                    {
                        MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
                        return 0;
                    }
                }
          }
          else
          {
                if (Seek(s->fd, s->file_ptr) != FS_SUCCEED)
                {
                    MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
                    return 0;
                }
          }
    #else
                if (Seek(s->fd, s->file_ptr) != FS_SUCCEED)
                {
                    MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
                    return 0;
                }
    #endif
            }
            else
            {
                if (Seek(s->fd, s->file_ptr) != FS_SUCCEED)
                {
                    MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
                    return 0;
                }
            }
        }

        if (s->pos + size > s->file_len)
            size = s->file_len - s->pos;

        if (size > s->buf_max_size)
            size = s->buf_max_size;

        MP_DEBUG2("fileread pos %x, size %x", s->file_ptr, size);

        // Now seek to newpos
        if (f_local_device)
        {	
    #if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
            ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
          if(Get_Img_FileChainFlag() == ENABLE)
          {
            if (psSysConfig->dwCurrentOpMode != OP_MOVIE_MODE)
                len = Fstream_FileRead(s->fd, (BYTE *)((DWORD)s->buffer | 0xa0000000), size);
            else
                len = FileRead(s->fd, (BYTE *)((DWORD)s->buffer | 0xa0000000), size);
          }
          else
          {
                len = FileRead(s->fd, (BYTE *)((DWORD)s->buffer | 0xa0000000), size);
          }
    #else
            len = FileRead(s->fd, (BYTE *)((DWORD)s->buffer | 0xa0000000), size);
    #endif
        } 
        else
            len = FileRead(s->fd, (BYTE *)((DWORD)s->buffer | 0xa0000000), size);
    }

    if (len == 0)
    {							// end of file or read error    
        s->eof = 1;
        s->buf_pos = s->buf_len = 0;
        MP_DEBUG("%s: (len == 0) => return 0;", __FUNCTION__);
        return 0;
    }

    s->buf_pos = 0;
    s->buf_len = len;
    s->pos = s->file_ptr;
    s->file_ptr = FilePosGet(s->fd);
    return len;
}


static int fill_catch_buffer_memory(ST_MPX_STREAM * s, int size)
{
	DWORD len = 0;

	if (s->pos >= s->file_len)
	{							// end of file
		s->eof = 1;
		s->buf_pos = s->buf_len = 0;
		MP_DEBUG("%s: (s->pos (0x%x) >= s->file_len (0x%x)) => return 0;", __FUNCTION__, s->pos, s->file_len);
		return 0;
	}

	if (g_bAniFlag & ANI_AUDIO)
		TaskYield();
	
	if (s->file_ptr != s->source->offset)
	{
		s->source->offset = s->file_ptr;
	}

	if (s->pos + size > s->file_len)
		size = s->file_len - s->pos;

	if (size > s->buf_max_size)
		size = s->buf_max_size;

	MP_DEBUG2("fileread pos %x, size %x", s->file_ptr, size);
	if (size < s->buf_max_size)
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	mmcp_memcpy_polling(s->buffer, s->source->start + s->source->offset, size);
#else
	memcpy(s->buffer, s->source->start + s->source->offset, size);
#endif

    len = size;
	s->source->offset += size;
	
	s->buf_pos = 0;
	s->buf_len = len;
	s->pos = s->file_ptr;
	s->file_ptr = s->source->offset;
	return len;
}


static int _open_file_(ST_MPX_STREAM * s, STREAM * psHandle)
{
    if (psHandle == NULL)
    {
        MP_ALERT("%s: -E- psHandle is NULL", __FUNCTION__);
        return FAIL;
    }

    int local_disk = 1;
    MP_DEBUG("open_file %d", psHandle->Drv->DevID);
    if ((DEV_USB_WIFI_DEVICE == psHandle->Drv->DevID) || (DEV_CF_ETHERNET_DEVICE == psHandle->Drv->DevID))
        local_disk = 0;

    //Frank Lin add
#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING 
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    if ((psSysConfig->dwCurrentOpMode != OP_MOVIE_MODE) && (Get_Img_FileChainFlag() == ENABLE))
    {
        if (local_disk != 0)
		{
			if(InitPreload_FileChainClustersCache(psHandle) != PASS)
			{
				if (! SystemCardPresentCheck(psHandle->Drv->DrvIndex))
				   return FAIL;
			}
		}
    }
#endif

    DWORD len = FileSizeGet(psHandle);
    MP_DEBUG1("file size %d", len);
    SeekSet(psHandle);

    s->seek = _stream_seek_;
    s->file_len = len;
    s->fd = psHandle;
    s->fill_buffer = fill_catch_buffer; 
    s->pos = FilePosGet(s->fd);
    return PASS;
}


static int _open_memory_(ST_MPX_STREAM * s, BUFFER * psMemory)
{
    s->source = psMemory;
    s->seek = _memory_seek_;
    s->file_len = psMemory->size;
    s->fd = NULL;
    s->fill_buffer = fill_catch_buffer_memory; 
    s->pos = 0;
    return PASS;
}

//--------------------------------------------------------------------------


//---------------------------------------------------------------------------


// reset the stream element's pointers
void mpxStreamReset(ST_MPX_STREAM * s)
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

//ST_MPX_STREAM g_sStream;
int mpxStreamOpen(ST_MPX_STREAM *s, void * sHandle, BYTE *pBuffer, DWORD dwBufferSize, BYTE boProgressBar, BYTE boSource)
{	
	if (s == NULL)
	{
		MP_ALERT("%s: (s == NULL) => return FAIL;", __FUNCTION__);
		return FAIL;
	}

	memset(s, 0, sizeof(ST_MPX_STREAM));

	MP_DEBUG1("open buffer size %d", (DWORD)dwBufferSize);

	s->buffer = (BYTE *)((DWORD)pBuffer | 0xa0000000); 
	
	if (s->buffer != NULL) 
	{
		s->buf_max_size = dwBufferSize;

		if (boSource == STREAM_SOURCE_FILE)
		{
			if ( _open_file_(s, (STREAM *)sHandle) == PASS)
			{
				MP_DEBUG1("open file %x", (DWORD)s);
				return PASS;
			}
		}
		else if (boSource == STREAM_SOURCE_MEMORY)
		{
			if (_open_memory_(s, (BUFFER *)sHandle) == PASS)
			{
				MP_DEBUG1("open file %x", (DWORD)s);
				return PASS;
			}
	    }
	}
	
	MP_ALERT("%s: return FAIL;", __FUNCTION__);
	return FAIL;
}



int mpxStreamOpenBuffer(ST_MPX_STREAM *s, BYTE *pBuffer, DWORD dwBufferSize)
{	
	if (s == NULL)
	{
		MP_ALERT("%s: (s == NULL) => return FAIL;", __FUNCTION__);
		return FAIL;
	}

	//MP_DEBUG1("mpxStreamOpenBuffer size %d", dwBufferSize);
	//MP_DEBUG2("stream buffer head tag %2x%2x", pBuffer[0], pBuffer[1]);

	memset(s, 0, sizeof(ST_MPX_STREAM));

	s->buffer = pBuffer;

	if (s->buffer != NULL) 
	{
		s->fd = NULL;		
		s->file_ptr = s->file_len = s->buf_len = s->buf_max_size = dwBufferSize;
		s->pos = 0;
		s->buf_pos = 0;
	    s->seek = _stream_seek_;
    	s->fill_buffer = fill_catch_buffer; 
		return PASS;
	}

	MP_ALERT("%s: return FAIL;", __FUNCTION__);
	return FAIL;
}

/*!
	@ingroup	STREAM

	@brief	Free a stream's relative resources 

	@param	ST_MPX_STREAM *s		The handle of stream to free

	@return	
*/
void mpxStreamClose(ST_MPX_STREAM * s)
{
    if (s->fd)
    {
        int local_disk = 1;

        if ((DEV_USB_WIFI_DEVICE == s->fd->Drv->DevID) || (DEV_CF_ETHERNET_DEVICE == s->fd->Drv->DevID))
            local_disk = 0;

#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        if ((psSysConfig->dwCurrentOpMode != OP_MOVIE_MODE) && (Get_Img_FileChainFlag() == ENABLE))
        {
            if (local_disk != 0)
                Clear_FileChainClustersCache();
        }
#endif

        FileClose(s->fd);		
        if (s->pbBufferAllocated)
        {
            MP_DEBUG("s->pbBufferAllocated %08x", s->pbBufferAllocated);
            ext_mem_free(s->pbBufferAllocated);
        }
    }
    else
    {
        if (s->pbBufferAllocated)
            ext_mem_free(s->pbBufferAllocated);
    }

    s->fd = NULL;
}


DWORD mpxStreamGetSize(ST_MPX_STREAM * s)
{
    return s->file_len;
}


DWORD mpxStreamTell(ST_MPX_STREAM * s)
{
    return s->pos + s->buf_pos;
}


/*!
	@ingroup	STREAM

	@brief	Move stream buffer's read cursor to new file position 

	@param	ST_MPX_STREAM *s		The stream to operate

	@param	off_t pos		New file position

	@return	1				Success

	@return	0				Fail

	@remark	If seek successfully, stream buffer will be filled with the data around the new file 
			position, with the read cursor pointing to the exact new file position's byte

*/



//---------------------------------------------------------------------------
int mpxStreamSeek(ST_MPX_STREAM * s, int offset, DWORD origin)
{
	DWORD newpos = 0;

	// get new position
	switch (origin)
	{
	case SEEK_SET:
		newpos = offset;
		break;

	case SEEK_CUR:
		newpos = s->pos + s->buf_pos + offset;
		break;

	case SEEK_END:
		if (s->file_len > offset)
			newpos = s->file_len - offset;
		else
		{
			newpos = 0;
			MP_ALERT("%s: (s->file_len (0x%x) <= offset (0x%x)) => return FAIL;", __FUNCTION__, s->file_len, offset);
			return FAIL;
		}
		break;
	}

	return s->seek(s, newpos);
}

//---------------------------------------------------------------------------
int mpxStreamSkip(ST_MPX_STREAM * s, DWORD offset)
{
	s->buf_pos += offset;
	if (s->buf_pos < s->buf_len)
	{
		return PASS;
	}

	return s->seek(s, s->pos + s->buf_pos);
}

//---------------------------------------------------------------------------
int mpxStreamFillCache(ST_MPX_STREAM * s, DWORD dwPos, DWORD dwSize)
{
	if (dwPos + dwSize > s->file_len)
		dwSize = s->file_len - dwPos;

	if (s->fd)
	{
		s->pos = s->file_ptr = dwPos;
		s->buf_pos = s->buf_len = 0;
	}
	else
	{
		s->buf_pos = dwPos;
		return dwSize;
	}
				
	if (dwSize > s->buf_max_size)
		dwSize = s->buf_max_size;

//	if (s->pos + dwSize > s->file_len)
//		dwSize = s->file_len - s->pos;

	return s->fill_buffer(s, dwSize);
}


#if 1
//---------------------------------------------------------------------------
size_t mpxStreamDirectRead(void *buffer, size_t pos, size_t size, size_t count, register ST_MPX_STREAM *s)
{
	DWORD iSize = size * count;
	DWORD n = 0;
	DWORD iReadSize = 0;
	
	if (s->pos >= s->file_len)
	{
		s->eof = 1;
		MP_DEBUG("%s: (s->pos (0x%x) >= s->file_len (0x%x)) => return 0;", __FUNCTION__, s->pos, s->file_len);
		return 0;
	}

#if 0	
	if (s->buf_pos < s->buf_len)
	{
		n = s->buf_len - s->buf_pos;
		memcpy(buffer, s->buffer + s->buf_pos, n);
		iSize -= n;		
		s->buf_pos += n;
		iReadSize = n;
	}	
#endif	

	if (s->file_ptr != FilePosGet(s->fd))
	{
		MP_DEBUG2("pos %x, fd pos %x", s->file_ptr, FilePosGet(s->fd));
		if (Seek(s->fd, s->file_ptr) != FS_SUCCEED)
		{
			MP_ALERT("%s: Seek() failed => return 0;", __FUNCTION__);
			return 0;
		}
	}

	n = FileRead(s->fd, buffer + iReadSize, iSize);	
	s->file_ptr = FilePosGet(s->fd);
	s->pos = s->file_ptr;
	s->buf_pos = 0; 
	s->buf_len = 0;
	iReadSize += n;
	iSize -= n;
	
	if (iSize != 0)
	{
		MP_ALERT("%s: read size error", __FUNCTION__);
		//if (s->pos < s->file_len) 
		//	iReadSize += mpxStreamRead(buffer + iReadSize, 1, iSize, s);		
		//else
		s->eof = 1;
	}
	
	return iReadSize;
}
#endif

#if AUDIO_ON
extern BYTE g_slide;
#endif
//---------------------------------------------------------------------------
size_t mpxStreamRead(void *buffer, size_t size, size_t count, register ST_MPX_STREAM *s)
{
	DWORD iSize = size * count;
	DWORD n = 0;
	DWORD iReadSize = 0;

#if 0
	if (iSize > s->buf_max_size)
	{
		return mpxStreamDirectRead(buffer, size, count, s);
	}  
	else
#endif	
	{
		do 
		{
			n = iSize - iReadSize;
			if (s->buf_pos >= s->buf_len)
			{
				int iBufSize = (n > BUFFER_FILL_SIZE) ? s->buf_max_size : BUFFER_FILL_SIZE;

				if (s->fill_buffer(s, iBufSize) == 0)
					return iReadSize;
			}			
			
			if (n > s->buf_len - s->buf_pos)
				n = s->buf_len - s->buf_pos;

			if (n == 1)
				*((BYTE *)buffer + iReadSize) = *(s->buffer + s->buf_pos);
			else
			{
                if (g_bAniFlag & ANI_SLIDE)
                {
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))                
                    #if AUDIO_ON
					while(g_slide)
					#endif	
#endif
						TaskYield();
                }

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
//MP_ALERT("ff 0831 buffer=0x%x,s->buffer=0x%x,s->buf_pos=0x%x",buffer,s->buffer,s->buf_pos);
//MP_ALERT("ff 0831 s->pos=0x%x",s->pos);
				mmcp_memcpy_polling(buffer + iReadSize, s->buffer + s->buf_pos, n);
#else
				memcpy(buffer + iReadSize, s->buffer + s->buf_pos, n);
#endif
			}

			if (g_bAniFlag & ANI_SLIDE)
			{
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))                
                #if AUDIO_ON
				while(g_slide)
				#endif	
#endif						
					TaskYield();
			}
			s->buf_pos += n;
			iReadSize += n;
		} while (iReadSize < iSize);
	}
	
	//if (size > 1)
	//	swap_buffer_endian(buffer, size, count);

	return iReadSize;
}


//---------------------------------------------------------------------------
//  bypass all bytes except the 0xff to find the bSearchMarker
DWORD mpxStreamSearchString (ST_MPX_STREAM *s, BYTE *str, int str_len, int search_size)
{
	int i;
	char ch;

	if (search_size == 0) search_size = s->buf_len;

	i = 0;   
	UartOutText("\r\n search ");
	UartOutText(str);
	UartOutText(" -- ");

	do 
	{
		if (s->buf_pos >= s->buf_len)
		{
			if (fill_catch_buffer(s, BUFFER_FILL_SIZE) == 0)
			{
				MP_ALERT("%s: fill_catch_buffer() == 0 => return FAIL;", __FUNCTION__);
				return FAIL;
			}
		}

		ch = s->buffer[s->buf_pos++];

		if (ch != str[i])
			i = 0;
		else
		{
			i++;
			//UartOutText(ch);		//Mark by C.W 080918

			if (i == str_len)
			{
				UartOutText("\r\n");
				return s->pos + s->buf_pos;
			}    
		} 
	}
	while (!s->eof && search_size--);

	return 0;
}


//---------------------------------------------------------------------------
//  bypass all bytes except the 0xff to find the bSearchMarker
int mpxStreamSearchMarker (ST_MPX_STREAM *s, BYTE bSearchMarker)
{
    BYTE bMarker;
    
	do {
		if (s->buf_pos >= s->buf_len)
		{
			if (s->fill_buffer(s, BUFFER_FILL_SIZE) == 0)
			{
				MP_ALERT("%s: (s->fill_buffer(s, BUFFER_FILL_SIZE) == 0) => return FAIL;", __FUNCTION__);
				return FAIL;
			}
		}
		
        bMarker = s->buffer[s->buf_pos++];
    }
	while (bMarker != bSearchMarker);

    return PASS;
}


//---------------------------------------------------------------------------
WORD mpxStreamReadAndSkipLength(ST_MPX_STREAM *s)
{
	WORD offset;
	
	if (s->buf_pos + 2 < s->buf_len)
	{
		BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
		offset = *pBuf << 8;
		offset |= *(pBuf+1);
	}
	else
	{
		mpxStreamRead(&offset, 2, 1, s);
	}
	
	s->buf_pos += offset;
	if (s->buf_pos < s->buf_len)
	{
		return PASS;
	}

	return mpxStreamSeek(s, s->pos + s->buf_pos, SEEK_SET);
}


//---------------------------------------------------------------------------
BYTE mpxStreamGetc(ST_MPX_STREAM *s)
{
	if (s->buf_pos >= s->buf_len)
	{
		if (s->fill_buffer(s, BUFFER_FILL_SIZE) == 0)
		{
			MP_DEBUG("%s: (s->fill_buffer(s, BUFFER_FILL_SIZE) == 0) => return 0;", __FUNCTION__);
			return 0;
		}
	}

	//return s->buffer[s->buf_pos++];
	BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
	s->buf_pos++;	
	return *pBuf;
}


//---------------------------------------------------------------------------
WORD mpxStreamReadWord(ST_MPX_STREAM *s)
{
	WORD i;
	if (s->buf_pos + 2 < s->buf_len)
	{
		BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
		i = *pBuf << 8;
		i |= *(pBuf+1);
		s->buf_pos += 2;
	}
	else
	{
		mpxStreamRead((BYTE *)&i, 2, 1, s);
	}
	return i;
}


//---------------------------------------------------------------------------
DWORD mpxStreamReadDWord(ST_MPX_STREAM *s)
{
	DWORD i;

	if (s->buf_pos + 4 < s->buf_len)
	{
		BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
		i = *pBuf << 24;
		i |= *(pBuf+1) << 16;
		i |= *(pBuf+2) << 8;
		i |= *(pBuf+3);
		s->buf_pos += 4;
	}
	else
	{
		mpxStreamRead((BYTE *)&i, 4, 1, s);
	}

	return i;
}


//---------------------------------------------------------------------------
WORD mpxStreamReadWord_le(ST_MPX_STREAM *s)
{
	WORD i;

	if (s->buf_pos + 2 < s->buf_len)
	{
		BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
		i = *pBuf;
		i |= *(pBuf+1) << 8;
		s->buf_pos += 2;
	}
	else
	{
		mpxStreamRead((BYTE *)&i, 1, 2, s);
		swap_buffer_endian((BYTE *)&i, 2, 1);
	}

	return i;
}


//---------------------------------------------------------------------------
DWORD mpxStreamReadDWord_le(ST_MPX_STREAM *s)
{
	DWORD i;

	if (s->buf_pos + 4 < s->buf_len)
	{
		BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);
		i = *pBuf;
		i |= *(pBuf+1) << 8;
		i |= *(pBuf+2) << 16;
		i |= *(pBuf+3) << 24;
		s->buf_pos += 4;
	}
	else
	{
		mpxStreamRead((BYTE *)&i, 1, 4, s);
		swap_buffer_endian((BYTE *)&i, 4, 1);
	}

	return i;
}
//---------------------------------------------------------------------------


void mpxStreamPrintOut(ST_MPX_STREAM *s, int n)
{
#if 1 //(MP_DEBUG_ENABLE)
	int i = 0;
	BYTE *pBuf = (BYTE *)((DWORD)s->buffer + s->buf_pos);

	mpDebugPrint("Stream buffer %d", s->pos + s->buf_pos);
	if (s->buf_pos + n >= s->buf_len)
		n = s->buf_len - s->buf_pos;
	
	for (i = 0; i < n; i+= 8)
	{
		mpDebugPrint("%02x%02x%02x%02x  %02x%02x%02x%02x", 
		pBuf[0],pBuf[1],pBuf[2],pBuf[3],pBuf[4],pBuf[5],pBuf[6],pBuf[7]);
		pBuf += 8;
	}
#endif	
}


