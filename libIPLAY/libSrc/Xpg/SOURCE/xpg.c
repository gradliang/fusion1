/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "fs.h"

#include "display.h"
#include "xpg.h"
#include "xpgUtil.h"
#include "system.h"

//---------------------------------------------------------------------------
#pragma alignvar(4)
STXPGMOVIE g_stXpgMovie;
STXPGMOVIE *g_pstXpgMovie = NULL;

BYTE g_boNeedRepaint = 0;

//------------------------------------------------------------------------------
BOOL xpgLoadFromSPINOR(DWORD dwNORStartAddr, DWORD dwNORSXPGSize)
{
    DWORD dwFileSize = 0;
    DWORD *pdwStream = NULL;
    DWORD i;
    BYTE *pAllocBuffer = NULL;

    MP_DEBUG("%s: NOR memory Start Address: 0x%X", __func__, dwNORStartAddr);

    xpgInitMemory();

    // read xpg from SPI NOR
    MP_ALERT("%s: Load xpg from SPI NOR Address : 0x%X, Size = 0x%X", __func__, dwNORStartAddr, dwNORSXPGSize);
    
    BYTE *pbNORXpg = dwNORStartAddr;
    pbNORXpg = dwNORStartAddr;
    pbNORXpg++; // skip first char
    if (!(*pbNORXpg++ == 0x67 && *pbNORXpg++ == 0x70 && *pbNORXpg++ == 0x78))
    {
        mpDebugPrint("Magic Number = pbNORXpg = 0x%X 0x%X 0x%X 0x%X", *pbNORXpg++, *pbNORXpg++, *pbNORXpg++, *pbNORXpg++);
        MP_ALERT("-E- Load NOR xpg fail by Magic Number is wrong!!");
        return false;
    }
    //mpDebugPrint("Ha! Got NOR XPG!");
    
    pdwStream = dwNORStartAddr;
    dwFileSize = dwNORSXPGSize;
    // start to parsing xpg movie from stream
    xpgOpen(pdwStream, dwFileSize);

    return true;
}


///
///@ingroup xpg
///@brief   Load from Flash
///
///@param   dwTag - parameter to ISP_GetResourceSize(dwTag)
///
///@retval  false or true
///
BOOL xpgLoadFromFlash(DWORD dwTag)
{
    DWORD dwFileSize = 0;
    DWORD *pdwStream = NULL;
    DWORD i;
    BYTE *pAllocBuffer = NULL;

    MP_DEBUG("xpgLoadFromFlash");

    xpgInitMemory();

    // read xpg from flash
    MP_ALERT("-I- Load xpg from flash...");

    dwFileSize = IspFunc_GetRESOURCESize(dwTag);
    pAllocBuffer = (BYTE *) ext_mem_mallocEx(dwFileSize + 4096, __FILE__, __LINE__);

    if (pAllocBuffer == NULL || dwFileSize == 0)
    {
        MP_ALERT("-E- Load xpg fail!!");
        if (pAllocBuffer) ext_mem_freeEx(pAllocBuffer, __FILE__, __LINE__);

        return false;
    }

    pdwStream = (DWORD *) ((DWORD) pAllocBuffer | BIT29);
    Idu_WaitBufferEnd();
    IspFunc_ReadRESOURCE(dwTag, (BYTE *) pdwStream, dwFileSize);
    //Idu_OsdOnOff(1);

    // start to parsing xpg movie from stream
    xpgOpen(pdwStream, dwFileSize);

    if (pAllocBuffer) ext_mem_freeEx(pAllocBuffer, __FILE__, __LINE__);

    return true;
}



//---------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Preload file ten allocate ext mem and Open
///
///@param   sFileHandle - pointer of FileHandle
///
///@return  true or false
///
BOOL xpgPreloadAndOpen(STREAM * sFileHandle)
{
    DWORD dwFileSize = 0;
    BYTE *xpgBuffer;
    DWORD *pdwStream = NULL;
    DWORD i, j;
    BYTE *pAllocBuffer = NULL;

    if (sFileHandle != NULL)
    {
        xpgInitMemory();
        MP_ALERT("Load xpg from SD!!");
        dwFileSize = FileSizeGet(sFileHandle);

        pAllocBuffer = (BYTE *) ext_mem_mallocEx(dwFileSize + 4096, __FILE__, __LINE__);

        if (!pAllocBuffer)
        {
            MP_ALERT("%s memory alloc fail", __FUNCTION__);

            return false;
        }

        pdwStream = (DWORD *) ((DWORD ) pAllocBuffer | BIT29);
        FileRead(sFileHandle, (BYTE *) pdwStream, dwFileSize);
        FileClose(sFileHandle);

        // start to parsing xpg movie from stream
        xpgOpen(pdwStream, dwFileSize);

        if (pAllocBuffer) ext_mem_freeEx(pAllocBuffer, __FILE__, __LINE__);

        return true;
    }
    else
        return false;
}



///
///@ingroup xpg
///@brief   Open Movie file
///
///@param   pdwStream - pointer to stream buffer
///
///@param   dwSize - Movie's size
///
///@return  true or false
///
DWORD xpgOpen(DWORD * pdwStream, DWORD dwSize)
{
    register STXPGMOVIE *pstMovie = &g_stXpgMovie;

    g_pstXpgMovie = (STXPGMOVIE *) ((DWORD) & g_stXpgMovie | BIT29);
    xpgMovieInit(pstMovie);

    xpgLoadMovie(pstMovie, pdwStream, dwSize);

    xpgMovieLoadPage(pstMovie, 0);
    xpgMovieInitObjRole(pstMovie);
    xpgMovieLoadGlobalCommand(pstMovie);

    return XPG_OK;
}


///
///@ingroup xpg
///@brief   Load a specified Movie page then goto it
///
///@param   dwPage - specified page index
///
///@return  PASS or FAIL
///
SWORD xpgGotoPage(DWORD dwPage)  //Mason 20060619  //From Athena
{
    register STXPGMOVIE *pstMovie = &g_stXpgMovie;

    MP_DEBUG("Enter %s(dwPage = %u)...", __FUNCTION__, dwPage);
    if (pstMovie == NULL)
        return FAIL;

    if (dwPage == 0)
    {
        MP_ALERT("-E- xpg goto page 0");
        return FAIL;
    }
    BYTE bPrevPageMode = pstMovie->m_pstCurPage->m_bPageMode;
    pstMovie->m_wPrevPage = pstMovie->m_wCurPage;
   // Idu_OsdErase();
   xpgGotoNewPageInit(pstMovie->m_wCurPage,dwPage);
        
    xpgMovieLoadPageCommand(pstMovie, dwPage);

    xpgMovieLoadPage(pstMovie, dwPage);

    return PASS;
}



///
///@ingroup xpg
///@brief   Go to Enter page
///
///@return  page index
///
WORD xpgGotoEnterPage()  //Mason 20060619  //From Athena
{
    WORD wPage = g_pstXpgMovie->m_pstCurPage->m_wEnterPage;

    if (wPage != 0)
        xpgGotoPage(wPage);

    return wPage;
}



//---------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Search specified name's page index then goto
///
///@param   name - page name
///
///@param   len - page name's length
///
///@return  page index
///
STXPGPAGE *xpgSearchAndGotoPage(const char *name, DWORD len)
{
    len = strlen(name);
    STXPGPAGE *pstPage = xpgMovieSearchPage(name, len);

    if (pstPage == NULL)
    {
        MP_ALERT("%s: XPG page name [%s] not found !", __FUNCTION__, name);
        return NULL;
    }

	//mpDebugPrint("---------xpgSearchAndGotoPage :%s-> %d",name,pstPage->m_wIndex);
    if (xpgGotoPage(pstPage->m_wIndex) != PASS)
    {
        MP_ALERT("xpgSearchAndGotoPage : xpgGotoPage() failed !");
        return NULL;
    }

    return pstPage;
}

void xpgForcedSearchAndGotoPage(const char *name, DWORD len)
{
    STXPGPAGE *pstPage = xpgSearchAndGotoPage(name, len);
    if(pstPage == NULL)
    {
        MP_ALERT("%s: page %s (len = %d) search fail!", __FUNCTION__, name, len);
        return;
    } 
    g_boNeedRepaint = true;
    xpgUpdateStage();
}

//-----------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Init thumb parameters
///
///@param   dwCount - Thumb buffer count
///
///@param   dwBufferSize - not used
///
///@return  true or false
///
XPGBOOL xpgInitThumbBuffer(DWORD dwCount, DWORD dwBufferSize)
{
#if XPG_THUMB_COUNT
    DWORD i;
    register STXPGMOVIE *pstMovie = &g_stXpgMovie;

    pstMovie->m_dwThumbBufferCount = dwCount;

    for (i = 0; i < XPG_THUMB_COUNT; i++)
    {
        pstMovie->m_astThumb[i].m_astImg.m_pImage = NULL;
        pstMovie->m_astThumb[i].m_dwHashKey = 0;
    }
#endif
    return true;
}



//-----------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Release Thumb Buffer
///
/// Remark  Add at 20070517 -for thumb catch use mem_malloc
void xpgReleaseThumbBuffer()
{
#if XPG_THUMB_COUNT
    int i;
    register STXPGMOVIE *pstMovie = &g_stXpgMovie;

    pstMovie->m_dwThumbBufferCount = XPG_THUMB_COUNT;

    for (i = 0; i < XPG_THUMB_COUNT; i++)
    {
        if (pstMovie->m_astThumb[i].m_astImg.m_pImage)
            ext_mem_freeEx(pstMovie->m_astThumb[i].m_astImg.m_pImage, __FILE__, __LINE__);

        pstMovie->m_astThumb[i].m_astImg.m_pImage = NULL;
        pstMovie->m_astThumb[i].m_dwHashKey = 0;
    }
#endif
}



//---------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Release All Role Buffer
///
void xpgReleaseAllRoleBuffer()
{
    int i;
    register STXPGMOVIE *pstMovie = &g_stXpgMovie;

    for (i = 0; i < pstMovie->m_wRoleCount; i++)
    {
        xpgRoleRelease(&(pstMovie->m_astRole[i]));
    }
}



//---------------------------------------------------------------------------
///
///@ingroup xpg
///@brief   Check XPG Animation version
BOOL xpgSupportAnimationCheck(void)
{
    if (g_stXpgMovie.m_dwVersion >= 5000)
        return TRUE;
    else
        return FALSE;
}

