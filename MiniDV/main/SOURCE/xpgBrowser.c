/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "taskid.h"
#include "ui.h"
#include "mpapi.h"
#include "filebrowser.h"
#include "os.h"
#include "ui_timer.h"
#include "setup.h"
#include "ispfunc.h"
#if RTC_ENABLE
#include "../../ui/include/rtc.h"
extern ST_IMGWIN m_stClockWin[3];
#endif
#include "mpapi.h"

// ######## SPYCAM ######## //
#include "Camcorder_func.h"
//#include "xpgCamera.h"
//#include "xpgCamFunc.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if MAKE_XPG_PLAYER

#pragma alignvar(4)
//id3_tag_t *g_psID3Tag;

extern STXPGBROWSER g_stBrowser[OP_TOTAL_MODE];
extern STXPGBROWSER *g_pstBrowser;
extern BOOL bSetUpChg;

///
///@ingroup xpgBrowser
///@brief   Clear Browser's each bCurDriveId
///
///Remark  abel add for background Music Setting can not update file list while change card
void ClrBrowserDrvId()
{
    MP_DEBUG("ClrBrowserDrvId");
    BYTE i;

    for (i = 0; i < OP_TOTAL_MODE; i++)
        g_stBrowser[i].bCurDriveId = 0;
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgBrowser
///@brief   Get Current Media info
///
///@param   pSearchInfo - pointer of struct SearchInfo
///
#if 0
void xpgGetCurMediaInfo(ST_SEARCH_INFO *pSearchInfo)
{
    MP_DEBUG("xpgGetCurMediaInfo");
    register STXPGBROWSER *pstBrowser = g_pstBrowser;
    register volatile STXPGMOVIE *pstMovie = &g_stXpgMovie;

    /*
    if (pSearchInfo == NULL)
        pSearchInfo = pstBrowser->pCurSearchInfo = FileGetCurSearchInfo();
    */

    if (pSearchInfo == NULL)
        return;

    if (pSearchInfo->bParameter & SEARCH_INFO_FILE) // for use bParameter record invalid file
    {
        //BREAK_POINT();
        pstBrowser->bCurFileType = pSearchInfo->bFileType;	//FileGetMediaType(pSearchInfo->bExt);

        if (pstBrowser->bCurFileType == FILE_OP_TYPE_IMAGE)
        {
            register STIMGINFO *pstImg = (STIMGINFO *) & pstMovie->m_astThumb[pstBrowser->wListIndex].m_astImg;
            pstBrowser->wCurImageWidth = pstImg->m_wImgWidth;
            pstBrowser->wCurImageHeight = pstImg->m_wImgHight;
        }
        else if (pstBrowser->bCurFileType == FILE_OP_TYPE_AUDIO)
        {
            if (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAYER)
                return;

            if (pSearchInfo->dwMediaInfo == 0)
            {
                Media_Info *pMediaInfo = FileBrowserGetMediaInfo(pSearchInfo);
                pSearchInfo->dwMediaInfo = pMediaInfo->dwTotalTime;
                //if (pSearchInfo->dwMediaInfo == 0) pSearchInfo->dwMediaInfo = -1;

                g_psID3Tag = &pMediaInfo->id3_tag;
                MP_DEBUG(g_psID3Tag->Title);
                MP_DEBUG(g_psID3Tag->Artist);
                MP_DEBUG(g_psID3Tag->Album);
            }
        }
    }
}
#endif

///
///@ingroup xpgBrowser
///@brief   Set Index for Menu List
///
///@param   iIndex - index need to set
///
///@param   bFlag - the update method
///
void xpgMenuListSetIndex(DWORD iIndex, BYTE bFlag)
{
    MP_DEBUG("xpgMenuListSetIndex %d, %d", iIndex, bFlag);
    //mpDebugPrint("xpgMenuListSetIndex %d, %d", iIndex, bFlag);
    RemoveTimerProc(xpgUpdateStage);
    register STXPGBROWSER *pstBrowser = g_pstBrowser;
    register volatile STXPGMOVIE *pstMovie = &g_stXpgMovie;

    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;


 #if (PRODUCT_UI==UI_HXJ_1)
        pstBrowser->wListCount = pstMovie->m_dwThumbCount;
#else
   if (pstBrowser->dwOpMode == OP_IMAGE_MODE)
        pstBrowser->wListCount = pstMovie->m_dwThumbCount;
    else
        pstBrowser->wListCount = pstMovie->m_dwListCount;
#endif

 #if (PRODUCT_UI==UI_HXJ_1)
	if ((pstBrowser->dwOpMode == OP_MOVIE_MODE)&&(pstBrowser->wListCount))
	{
		if (pstBrowser->wIndex /pstBrowser->wListCount  != iIndex/pstBrowser->wListCount)
		{
			Draw4VideoFilesThumb();
		}
	}
#endif
    pstBrowser->wIndex = iIndex;
    //mpDebugPrint("pstBrowser->wIndex=%d", pstBrowser->wIndex);

#if EREADER_ENABLE
    if (pstBrowser->dwOpMode == OP_EBOOK_MODE)
        pstBrowser->wCount = g_psSystemConfig->sFileBrowser.dwEbookTotalFile;
    else
#endif
    {
        if (pstBrowser->dwOpMode == OP_AUDIO_MODE)
            pstBrowser->wCount = g_psSystemConfig->sFileBrowser.dwAudioTotalFile;
        else
            pstBrowser->wCount = g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile;
    }

    //mpDebugPrint("pstBrowser->wCount=%d", pstBrowser->wCount);

    /*if (g_bAniFlag & ANI_VIDEO_PREVIEW)
    xpgStopAllAction();*/

    if (!(pstBrowser->wCount))
    {
        if (pstMovie->m_pstListFrame != NULL)
        {
            pstMovie->m_pstListFrame->m_boVisible = false;
        }

        if (pstMovie->m_pstThumbFrame != NULL)
        {   //Jasmine 6/1: system track and bug fixed
            pstMovie->m_pstThumbFrame->m_boVisible = false;
            //pstMovie->m_pstListFrame->m_boVisible = false;
        }

        pstBrowser->pCurSearchInfo = NULL;

        g_boNeedRepaint = XPG_REPAINT_LIST;
#if 0			// abel 20110609 mask         
        //xpgDisableKey(XPG_KEY_SETUP);
        xpgDisableKey(XPG_KEY_ENTER);
        xpgDisableKey(XPG_KEY_PP);
        //xpgDisableKey(XPG_KEY_STOP);
#endif
        return;
    }

    pstBrowser->pCurSearchInfo = (ST_SEARCH_INFO *) FileGetSearchInfo(pstBrowser->wIndex);

#ifdef XPG_DVALUE
    //TraceSearchInfo(pstBrowser->pCurSearchInfo);
#endif

    register DWORD dwListCount = pstBrowser->wListCount;
    //mpDebugPrint("pstBrowser->wListCount=%d", pstBrowser->wListCount);
    if (dwListCount > 0)
    {
        pstBrowser->wListIndex = pstBrowser->wIndex % dwListCount;
        WORD i = pstBrowser->wIndex - pstBrowser->wListIndex;

#if 0			// abel 20110609 mask 
        if (pstBrowser->wListFirstIndex != i)
            bFlag = 1;
#endif

        pstBrowser->wListFirstIndex = i;
        //mpDebugPrint("pstBrowser->wListIndex=%d", pstBrowser->wListIndex);
        //xpgUpdateListIcon();
#if 0//AUDIO_ON
        if (pstMovie->m_pstCurPage->m_bPageMode == XPG_MODE_MUSICMENU)
            xpgUpdateMusicIcon(0);
#endif
        BYTE Play_Page = (pstBrowser->wIndex / dwListCount) + 1;
        BYTE Total_Page = (pstBrowser->wCount / dwListCount) + 1;

        if ((pstBrowser->wCount % dwListCount) == 0)
            Total_Page--;

        //xpgUpdateFileBar(Total_Page, Play_Page);
    }

    if (bFlag && pstBrowser->wCount > 0)
    {
        if (bFlag & 2)
        {
            g_boNeedRepaint = 0;
        }
        else
        {
            if (pstMovie->m_pstListFrame != NULL)
                pstMovie->m_pstListFrame->m_boVisible = false;

            if (pstMovie->m_pstThumbFrame != NULL)
                pstMovie->m_pstThumbFrame->m_boVisible = false;

            if (pstMovie->m_dwListCount > 0)
            {
                ST_SEARCH_INFO *pSearchInfo = (ST_SEARCH_INFO *) FileGetSearchInfo(pstBrowser->wListFirstIndex);

                //if (bFlag == 1 && pSearchInfo != NULL && pSearchInfo->dwMediaInfo == 0)
                if (bFlag == 1 && pSearchInfo != NULL)
                {
                    //fengrs 02/27 need check
                    g_boNeedRepaint = XPG_REPAINT_NOLIST;
#if VIDEO_ON
                    xpgUpdateStage();
#else
                    pstSprite = xpgSpriteFindType(pstMov, 0, 0);
                    xpgUpdateStageClipBySprite(Idu_GetCurrWin(), pstSprite, 1);
#endif
                    g_boNeedRepaint = XPG_REPAINT_LIST;
                }
            }
        }
    }

#if 0
    STXPGSPRITE *pstMusicIcon = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_ICON, 0);

    if (pstMusicIcon != NULL)
    {
        pstMusicIcon->m_boVisible = false;
    }
#endif

    // prepare current file information
    pstBrowser->dwCurFileSize = 0;
    pstBrowser->wCurImageWidth = 0;
    pstBrowser->wCurImageHeight = 0;
    pstBrowser->bCurFileType = FILE_OP_TYPE_FOLDER;

    pstBrowser->pCurSearchInfo = FileGetSearchInfo(iIndex);
    register ST_SEARCH_INFO *pSearchInfo = pstBrowser->pCurSearchInfo;

    if (pSearchInfo == NULL || pstBrowser->wCount == 0)
    {
        //SystemSetErrEvent(ERR_No_file_in_this_mode);
        return;
    }

    if (pSearchInfo->bParameter & SEARCH_INFO_FILE) //for use bParameter record invalid file
    {
        pstBrowser->bCurFileType = pSearchInfo->bFileType;	//FileGetMediaType(pSearchInfo->bExt);

#if 0
        if (pstBrowser->bCurFileType == FILE_OP_TYPE_IMAGE)
        {
            register STIMGINFO *pstImg = (STIMGINFO *) & pstMovie->m_astThumb[pstBrowser->wListIndex].m_astImg;
            pstBrowser->wCurImageWidth = pstImg->m_wImgWidth;
            pstBrowser->wCurImageHeight = pstImg->m_wImgHight;
        }
        else if (pstBrowser->bCurFileType == FILE_OP_TYPE_AUDIO)
        {
            if (!(g_bAniFlag & ANI_AUDIO))
            {
                g_psID3Tag = NULL;
                pSearchInfo->dwMediaInfo = 0;
            }
        }
#endif
    }

    if (bFlag == 2)
        g_boNeedRepaint = false;
}


///
///@ingroup xpgBrowser
///@brief   MenuList Set Next index
///
void xpgMenuListNext()
{
    MP_DEBUG("xpgMenuListNext");
    xpgMenuListSetIndex(FileListAddCurIndex(1), false);
}

///
///@ingroup xpgBrowser
///@brief   MenuList Set Previous Index
///
void xpgMenuListPrev()
{

    MP_DEBUG("xpgMenuListPrev");
    xpgMenuListSetIndex(FileListAddCurIndex(-1), false);
}

///
///@ingroup xpgBrowser
///@brief   MenuList Set Next Page
///
void xpgMenuListNextPage()
{
    MP_DEBUG("xpgMenuListNextPage");
    xpgMenuListSetIndex(FileListAddCurIndex(g_pstXpgMovie->m_dwListCount), true);
}

///
///@ingroup xpgBrowser
///@brief   MenuList Set Previous Page
///
void xpgMenuListPrevPage()
{
    MP_DEBUG("xpgMenuListPrevPage");
    xpgMenuListSetIndex(FileListAddCurIndex(-g_pstXpgMovie->m_dwListCount), true);
}

///
///@ingroup xpgBrowser
///@brief   Chnage MenuMode and refresh FileList
///
void xpgChangeMenuMode(DWORD dwOpMode, BYTE bFlag)
{
    MP_DEBUG1("xpgChangeMenuMode %d", dwOpMode);
    ST_SYSTEM_CONFIG *psSysConfig;

    psSysConfig = g_psSystemConfig;
    BYTE bDriveId = psSysConfig->sStorage.dwCurStorageId;
    struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;

    if (dwOpMode != OP_IMAGE_MODE)  // only  photo menu will keep thumb buffer alive
        xpgReleaseThumbBuffer();    // thumb catch use mem_malloc, release thumb buffer

    xpgStopAllAction();

    BOOL boModeChanged = (dwOpMode != psSysConfig->dwCurrentOpMode);

    psSysConfig->dwCurrentOpMode = dwOpMode;

    g_pstBrowser = (STXPGBROWSER *) ((DWORD) & (g_stBrowser[dwOpMode]) | 0x20000000);
    g_pstBrowser->dwOpMode = dwOpMode;

    STXPGBROWSER *pstBrowser = g_pstBrowser;
    BOOL boDrvieChanged = (pstBrowser->bCurDriveId != bDriveId);

    switch (dwOpMode)
    {
    case OP_FILE_MODE:
        g_bXpgStatus = XPG_MODE_FILEMENU;
        break;

#if VIDEO_ON
    case OP_MOVIE_MODE:
        g_bXpgStatus = XPG_MODE_VIDEOMENU;
        break;
#endif

    case OP_IMAGE_MODE:
        g_bXpgStatus = XPG_MODE_PHOTOMENU;
        break;

    case OP_AUDIO_MODE:
        g_bXpgStatus = XPG_MODE_MUSICMENU;
        break;
#if EREADER_ENABLE
    case OP_EBOOK_MODE:
        g_bXpgStatus = XPG_MODE_WORD;
        break;
#endif
    }

    //if (boModeChanged || boDrvieChanged)
    {
        if (dwOpMode != OP_SETUP_MODE && dwOpMode != OP_IDLE_MODE)
        {
            FileBrowserResetFileList(); /* reset old file list first */
            FileBrowserScanFileList(SEARCH_TYPE);
        }
    }

    pstBrowser->bCurDriveId = psSysConfig->sStorage.dwCurStorageId;
    xpgMenuListSetIndex(FileListAddCurIndex(0), bFlag);



    if(g_bXpgStatus == XPG_MODE_MUSICMENU)
    {
        //mpDebugPrint("g_psSystemConfig->sFileBrowser.dwFileListCount[OP_AUDIO_MODE]=%d", g_psSystemConfig->sFileBrowser.dwFileListCount[OP_AUDIO_MODE]);
        if (g_psSystemConfig->sFileBrowser.dwFileListCount[OP_AUDIO_MODE] > 0)
            g_boNeedRepaint = false;
    }
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgBrowser
///@brief   Prepare MusicMenu parameters before enter AUDIO mode
///
#if AUDIO_ON
void xpgCb_EnterMusicMenu()
{
    MP_DEBUG("xpgCb_EnterMusicMenu");

    if (g_bXpgStatus == XPG_MODE_NULL)
    {
        xpgGotoNoCardPage();
        g_boNeedRepaint = false;
        return;
    }

    g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_MUSICMENU;
    xpgChangeMenuMode(OP_AUDIO_MODE, 1);

    if (g_psSystemConfig->sFileBrowser.dwAudioTotalFile > 0)
        Api_AudioHWEnable();

    if ((bSetUpChg == true) && (!g_bAniFlag))
    {
        PutSetupMenuValue();
        bSetUpChg = 0;
    }

}
#endif


///
///@ingroup xpgBrowser
///@brief   Prepare VideoMenu parameters before enter Video mode
///
#if VIDEO_ON
void xpgCb_EnterVideoMenu()
{
    MP_DEBUG("_%s_",__FUNCTION__);

    if (g_bXpgStatus == XPG_MODE_NULL)
    {
        xpgGotoNoCardPage();
        g_boNeedRepaint = false;

        return;
    }

    g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_VIDEOMENU;

    xpgChangeMenuMode(OP_MOVIE_MODE, 0);
    //xpgChangeMenuMode(OP_IMAGE_MODE, 1);

    Api_AudioHWEnable();
    //g_boNeedRepaint = false; // Jonny 20090401 for VideoPreview

//    ext_mem_init();
   //xpgCb_VideoPlay(); // do VideoPreview

}
#endif

//------------------------------------------------------------------------------------------
//
///@ingroup xpgBrowser
///@brief   Prepare PhotoMenu parameters before enter Photo mode
///
void xpgCb_EnterPhotoMenu()
{
    MP_DEBUG("xpgCb_EnterPhotoMenu");

    xpgClearCatch();
    g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_PHOTOMENU;
    xpgChangeMenuMode(OP_IMAGE_MODE, 1);
    //mpDebugPrint("xpgCb_EnterPhotoMenu() : GetCurFileSortingBasis()=%d", GetCurFileSortingBasis());

}


///
///@ingroup xpgBrowser
///@brief   Open folder
///
void xpgCb_OpenFolder(void)
{
    MP_DEBUG("xpgCb_OpenFolder");
    ST_SEARCH_INFO *psSearchList = FileGetCurSearchInfo();

    MP_ASSERT(psSearchList != NULL);

    if (FileChangeDirAndResearch(psSearchList) == FAIL)
    {
        MP_DEBUG("-E- OpenFolder FAIL");
        SystemSetErrEvent(ERR_FILE_SYSTEM);
    }

    xpgMenuListSetIndex(FileListAddCurIndex(0), true);
}


///
///@ingroup xpgBrowser
///@brief   do FileBrowserDeleteFile() and UpdateFileList and UpdateStage
///
SWORD xpgFileDelete()
{
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    DWORD dwCurIndex;
    SWORD swRet;

  MP_DEBUG("xpgFileDelete");
    if (g_bAniFlag || psSysConfig->sStorage.dwCurStorageId == 0)
        return  FAIL;

    if (g_bXpgStatus == XPG_MODE_MUSICMENU)
    {
        if (psSysConfig->sFileBrowser.dwAudioTotalFile == 0)
            return FAIL;

        dwCurIndex = psSysConfig->sFileBrowser.dwAudioCurIndex;
    }
    else
    {
        if (psSysConfig->sFileBrowser.dwImgAndMovTotalFile == 0)
            return FAIL;

        dwCurIndex = psSysConfig->sFileBrowser.dwImgAndMovCurIndex;
    }

    swRet = FileBrowserDeleteFile();

    xpgMenuListSetIndex(FileListSetCurIndex(dwCurIndex), false);

    if (g_bXpgStatus == XPG_MODE_PHOTOVIEW)
    {
#if (SEARCH_TYPE == GLOBAL_SEARCH)
        FileBrowserSearchNextImage(0);
        //xpgViewPhoto();
#endif
        return swRet;
    }
#if VIDEO_ON && SENSOR_ENABLE
    else if (g_bXpgStatus == XPG_MODE_VIDEOMENU)
    {
		ui_EnterVideoPage();
    }
#endif
    xpgUpdateStage();

    return swRet;
}

//---------------------------------------------------------------------------
///
///@ingroup xpgBrowser
///@brief   Release Thumb buffer
///
void xpgClearCatch()
{
    MP_DEBUG("xpgClearCatch");
    xpgReleaseThumbBuffer();   // dynamic thumb buffer
}
//---------------------------------------------------------------------------
#endif

