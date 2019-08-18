/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "display.h"
//#include "xpgFunc.h"
#include "taskid.h"
#include "ui.h"
#include "util.h"
#include "../../image/include/jpeg.h"

#if ( ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650) || ((CHIP_VER & 0xFFFF0000) == CHIP_VER_660))
#define memcpy                  mmcp_memcpy
#endif

void xpgRoleJpegDecodeBufferFree(DWORD *pImgBuf)
{
    ext_mem_free(pImgBuf);
}

// Example : in ClockType1, each digit role need to decode first/ 
DWORD * xpgRoleJpegDecodeToBuffer(STXPGROLE * pstRole)
{
    DWORD dwAllocSize = 0;
	dwAllocSize = (ALIGN_16(pstRole->m_wWidth) + 16) * ALIGN_16(pstRole->m_wHeight) * 2 + 4096;
	
	BYTE *pbAllocBuf = NULL;
	pbAllocBuf = (BYTE *)ext_mem_malloc(dwAllocSize);
	if (pbAllocBuf == NULL) 
    {
		MP_ALERT("%s: role size %d > buffer alloc fail", __func__, dwAllocSize);
		return NULL;
	}
	
	SWORD swRet;
	DWORD *pImgBuf = NULL;
	pImgBuf = (DWORD *) ((DWORD) pbAllocBuf | 0x20000000);
	swRet = Img_Jpeg2ImgBuf(pstRole->m_pImage, (BYTE *) pImgBuf, 0, pstRole->m_dwDataLen, dwAllocSize);
	if (swRet != 0) 
    {
			if (pImgBuf) ext_mem_freeEx(pImgBuf, __FILE__, __LINE__);
			return;
	}
	pstRole->m_wRawWidth = Jpg_GetTargetWidth();
	return pImgBuf;
}
