//---------------------------------------------------------------------------
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"


#include "xpgUtil.h"
#include "fs.h"



XPGFILE stXpgMovieStream;
static DWORD dwXpgMemOffset;
DWORD xpgMemoryGetOffset()
{
    return dwXpgMemOffset;
}
void xpgMemorySetOffset(DWORD offset) // called by xpgDriveFunc.c 
{
    dwXpgMemOffset = offset;
}
DWORD xpgGetXpgMemorySize() // called in main.c
{
    return (dwXpgMemOffset / 1024);
}

static DWORD dwXpgMemOffsetSave;
DWORD xpgMemoryGetOffsetSave() // called in xpgMovie.c
{
    //mpDebugPrint("%s: dwXpgMemOffsetSave = 0x%X", __func__, dwXpgMemOffsetSave);
    return dwXpgMemOffsetSave;
}
void xpgMemorySetOffsetSave(DWORD offset) // called by xpgDriveFunc.c 
{
    dwXpgMemOffsetSave = offset;
}

static BYTE *pbXpgMem = NULL;
static DWORD dwXpgMemSize = 0;


//---------------------------------------------------------------------------
//  xpg memory util
//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Init memory
///
void xpgInitMemory(void)
{
    pbXpgMem = (BYTE *) (SystemGetMemAddr(XPG_BUF_MEM_ID) | 0x20000000);
    dwXpgMemOffset = 0;
    dwXpgMemSize = SystemGetMemSize(XPG_BUF_MEM_ID);
    memset(pbXpgMem, 0, dwXpgMemSize);
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Reest Buffer Size
///
void xpgResetBufSize(void)
{
#if 0
    DWORD dwTotalBufSize = SystemGetMemSize(JPEG_SOURCE_MEM_ID) + SystemGetMemSize(JPEG_TARGET_MEM_ID);
    SystemSetMemAddr(JPEG_TARGET_MEM_ID, g_xpgTargetBuffer);
    SystemSetMemSize(JPEG_SOURCE_MEM_ID, SystemGetJPEGChasingUnit(3)); //(DWORD)SystemGetMemAddr(JPEG_TARGET_MEM_ID) - (DWORD)SystemGetMemAddr(JPEG_SOURCE_MEM_ID));
    SystemSetMemSize(JPEG_TARGET_MEM_ID, dwTotalBufSize - SystemGetMemSize(JPEG_SOURCE_MEM_ID));
#endif
}
//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Menory allocate
///
void *xpgMalloc(size_t size)
{
    register volatile BYTE *ptr = NULL;

    // precheck if size is 0 and alert
    if (size == 0)
    {
        MP_ALERT("-E- xpgMalloc size 0");

        return NULL;
    }

    dwXpgMemOffset = ALIGN_4(dwXpgMemOffset);
    size = ALIGN_4(size);

    if ((dwXpgMemOffset + size) >= dwXpgMemSize)
    {
        MP_ALERT("::xpgMalloc overflow -");
        MP_ALERT("dwXpgMemSize - %d, Current size = %d", dwXpgMemSize, dwXpgMemOffset);
        MP_ALERT("need = %d", size);
        __asm("break 100");

        return NULL;
    }

    ptr = (BYTE *) pbXpgMem + dwXpgMemOffset;
    dwXpgMemOffset += size;

    MP_DEBUG("xpgMalloc %d", dwXpgMemOffset);

    return (void *) (ptr);
}

DWORD xpgMemoryGetFreeSpace()
{
    return (dwXpgMemSize - dwXpgMemOffset);
}
//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Call xpgMalloc num *size
///
///@param   num - count
///
///@param   size - unit size
///
void *xpgCalloc(size_t num, size_t size)
{
	return xpgMalloc(num * size);
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Free memory
///
void xpgFree(void *memblock)
{
	//free( memblock );
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Free all
///
void xpgFreeAll()
{
	dwXpgMemOffset = 0;
}

//---------------------------------------------------------------------------
//  xpg file util
//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Open stream file by input buffer and size
///
///@param   buffer - specified buffer
///
///@param   dwSize - appointed size
///
///@return  XPGFILE
///
XPGFILE *xpgFileOpen(void *buffer, DWORD dwSize)
{
	if (dwSize == 0)
	{
		stXpgMovieStream.pbBuffer = (BYTE *) buffer + 4;
		stXpgMovieStream.dwSize = *(DWORD *) buffer;
	}
	else
	{
		stXpgMovieStream.pbBuffer = (BYTE *) buffer;
		stXpgMovieStream.dwSize = dwSize;
	}
	stXpgMovieStream.dwPos = 0;
	stXpgMovieStream.io_ptr = stXpgMovieStream.pbBuffer + stXpgMovieStream.dwPos;
	return &stXpgMovieStream;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Read stream by input buffer and size
///
///@param   buffer - specified buffer
///
///@param   size - appointed size
///
///@param   count - count of size
///
///@param   stream - specified stream
///
///@retval size_t
///
size_t xpgFileRead(void *buffer, size_t size, size_t count, register XPGFILE * stream)
{
	register BYTE *pbStreamBuffer;
	register BYTE *pbDstBuffer;
	register DWORD i;

	if (stream->dwPos < 0)
		stream->dwPos = 0;
	if (stream->dwPos >= stream->dwSize)
		return -1;

	pbStreamBuffer = stream->pbBuffer + stream->dwPos;
	count *= size;
	pbDstBuffer = (BYTE *) buffer;

	for (i = 0; i < count; i++)
		pbDstBuffer[i] = pbStreamBuffer[i];

	stream->dwPos += count;
	stream->io_ptr = stream->pbBuffer + stream->dwPos;
	return 0;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Seek stream specified offset
///
///@param   stream - specified stream
///
///@param   offset - appointed size
///
///@param   origin - seek type
///@retval  DWORD
///
DWORD xpgFileSeek(register XPGFILE * stream, DWORD offset, DWORD origin)
{
	switch (origin)
	{
	case SEEK_SET:
		stream->dwPos = offset;
		break;
	case SEEK_CUR:
		stream->dwPos += offset;
		break;
	case SEEK_END:
		stream->dwPos = stream->dwSize - offset;
		break;
	}

	if (stream->dwPos < 0)
		stream->dwPos = 0;
	if (stream->dwPos >= stream->dwSize)
		stream->dwPos = stream->dwSize - 1;
	stream->io_ptr = stream->pbBuffer + stream->dwPos;
	return 0;
}

DWORD xpgFileTell(register XPGFILE * stream)
{
    return stream->dwPos;
}
//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Get char fro stream
///
///@param   stream - specified stream
///
///@retval char - char in current offset
///
char xpgFileGetc(register XPGFILE * stream)
{
	register char *pcStreamBuffer;

	pcStreamBuffer = (char *) (stream->pbBuffer + stream->dwPos);
	stream->dwPos++;
	stream->io_ptr = stream->pbBuffer + stream->dwPos;
	return *pcStreamBuffer;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Close stream file
///
///@param   stream - specified stream
///
///@retval DWORD
///
DWORD xpgFileClose(register XPGFILE * stream)
{
	stream->dwPos = 0;
	stream->io_ptr = stream->pbBuffer + stream->dwPos;
	return 0;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Get stream size
///
///@param   stream - specified stream
///
///@retval DWORD - stream size
///
DWORD xpgFileGetSize(register XPGFILE * stream)
{
	return stream->dwSize;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgUtil
///@brief   Get Hash value by input string and length
///
///@param   str - specified string
///
///@param   len - appointed string length
///
///@retval  DWORD - hash value
///
DWORD xpgHash(const char *str, DWORD len)
{
	register DWORD hash = 0;
	register DWORD i = 0;

    if (str == NULL)
        return 0;
    len = strlen(str);      // grad add

	for (i = 0; i < len && *str; str++, i++)
	{
		hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}
	return (hash & 0x7FFFFFFF);
	//return hash;
}

void xpgHashReverse(DWORD dwHash)
{
    // TODO: must change xpgHash()'s return to 'return hash;' first 

	return ;
}


//---------------------------------------------------------------------------
