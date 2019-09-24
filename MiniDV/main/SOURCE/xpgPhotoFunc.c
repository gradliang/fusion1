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
#include "os.h"
#include "display.h"
#include "devio.h"
#include "taskid.h"
#include "fs.h"
#include "ui.h"
#include "setup.h"

//#include "xpgCamera.h"
//#include "xpgCamFunc.h"

#if (MEM_2M == 0)
#include "slideEffect.h"
#endif

#if 0//MAKE_XPG_PLAYER

#define ZOOM_STEP   20
//extern WORD wZX, wZY, wZRatio;
BYTE bZoomMode = 0;
//------------------------------------------------------------------------------------------
//Photo Menu
//------------------------------------------------------------------------------------------


DWORD dwImgAndMovCurIndex=0;
DWORD xpgGetdwImgAndMovCurIndex()
{
    return dwImgAndMovCurIndex;
}


///
///@ingroup xpgPhotoFunc
///@brief   Move Photo Menu to next index
///
void xpgCb_PhotoMenuThumbNext()
{
	MP_DEBUG("xpgCb_PhotoMenuListNext");

	xpgMenuListNext();
	ST_SYSTEM_CONFIG *psSysConfig;
	psSysConfig = g_psSystemConfig;
	//mpDebugPrint("g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex=%d", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
	dwImgAndMovCurIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
}



///
///@ingroup xpgPhotoFunc
///@brief   Move Photo Menu to previous index
///
void xpgCb_PhotoMenuThumbPrev()
{
    MP_DEBUG("xpgCb_PhotoMenuListPrev");

    xpgMenuListPrev();
    ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    //mpDebugPrint("g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex=%d", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
    dwImgAndMovCurIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
}


/*
///
///@ingroup xpgPhotoFunc
///@brief   Move Photo Menu to next index
///
void xpgCb_CamMenuListRight()
{
	MP_DEBUG("xpgCb_CamMenuListRight@@");
    xpgCamMenuListNext();
	//xpgMenuListNext_SpyCam();
	ST_SYSTEM_CONFIG *psSysConfig;
	psSysConfig = g_psSystemConfig;
	//mpDebugPrint("g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex=%d", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
	dwImgAndMovCurIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
}

void xpgCb_CamMenuListLeft()
{
    MP_DEBUG("xpgCb_CamMenuListLeft @@");

    //xpgMenuListPrev_SpyCam();
    xpgCamMenuListPrev();
    ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    //mpDebugPrint("g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex=%d", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
    dwImgAndMovCurIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
}
*/


/*
///
///@ingroup xpgPhotoFunc
///@brief   Move Photo Menu to previous index
///
void xpgCb_PhotoMenuThumbPrev_SpyCam()
{
    MP_DEBUG("xpgCb_PhotoMenuListPrev");

    xpgMenuListPrev_SpyCam();
    ST_SYSTEM_CONFIG *psSysConfig;
    psSysConfig = g_psSystemConfig;
    //mpDebugPrint("g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex=%d", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
    dwImgAndMovCurIndex = g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex;
}
*/

/*
///
///@ingroup xpgPhotoFunc
///@brief   Exit from Photot humb
///
void xpgCb_PhotoMenuThumbExit()
{
    MP_DEBUG("xpgCb_PhotoMenuListExit, g_bXpgStatus=%d", g_bXpgStatus);
//     -- already done by xpgFunc.c's xpgCb_PageGlobalEnd()
//    if(g_bXpgStatus == XPG_MODE_MYFAVORPHOTO)
//    {
//    mpDebugPrint("Call RestoreCurrentSearchInfo();");
//    RestoreCurrentSearchInfo();
//    xpgResetMyFavorIndex();
    //}
    g_bXpgStatus = XPG_MODE_MODESEL;
    Idu_OsdErase();
    g_boNeedRepaint=true;
    xpgSearchAndGotoPage("Main_Photo");
}
*/

//void xpgCb_PhotoMenuShowList() {}
//void xpgCb_PhotoMenuGetList() {}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Change Photo Menu to Previous Line
///
void xpgCb_PhotoMenuThumbPrevLine()
{
	xpgMenuListSetIndex(FileListAddCurIndex(-3), false);//g_pstXpgMovie->m_dwThumbColumns
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Change Photo Menu to Next Line
///
void xpgCb_PhotoMenuThumbNextLine()
{
	xpgMenuListSetIndex(FileListAddCurIndex(3), false);//g_pstXpgMovie->m_dwThumbColumns
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Change Photo Menu to Previous Page
///
void xpgCb_PhotoMenuThumbPrevPage()
{
	register STXPGBROWSER *pstBrowser = g_pstBrowser;
	DWORD iFileCount = pstBrowser->wCount;
	SWORD iFileIndex = pstBrowser->wIndex;
	DWORD iThumbCount = g_pstXpgMovie->m_dwThumbCount;

	if (iFileIndex < iThumbCount)
//      iFileIndex = iFileCount;
		if ((iFileCount % iThumbCount) == 0)
			iFileIndex = iFileCount - 1;
		else
			iFileIndex = iFileCount;
	else
		iFileIndex -= iThumbCount;

	iFileIndex -= iFileIndex % iThumbCount;

	xpgMenuListSetIndex(FileListSetCurIndex(iFileIndex), true);
       xpgClearCatch();
       ImageReleaseAllBuffer();
}



///
///@ingroup xpgPhotoFunc
///@brief   Change Photo Menu to Next Line
///
void xpgCb_PhotoMenuThumbNextPage()
{
#if 0
	ST_SYSTEM_CONFIG *psSysConfig;
	DWORD *pdwCurIndex, *pdwTotalCount, move_no;

	psSysConfig = g_psSystemConfig;
	switch (psSysConfig->dwCurrentOpMode)
	{
	case OP_AUDIO_MODE:
		pdwTotalCount = &psSysConfig->sFileBrowser.dwAudioTotalFile;
		pdwCurIndex = &psSysConfig->sFileBrowser.dwAudioCurIndex;
		break;

	case OP_IMAGE_MODE:
	case OP_MOVIE_MODE:
	case OP_FILE_MODE:
		pdwTotalCount = &psSysConfig->sFileBrowser.dwImgAndMovTotalFile;
		pdwCurIndex = &psSysConfig->sFileBrowser.dwImgAndMovCurIndex;
		break;
	default:
		return;
	}

	move_no = g_pstXpgMovie->m_dwThumbCount - (*pdwCurIndex % g_pstXpgMovie->m_dwThumbCount);
	g_pstBrowser->wIndex = *pdwCurIndex + move_no;

	if (g_pstBrowser->wIndex >= *pdwTotalCount)
		g_pstBrowser->wIndex = 0;

	*pdwCurIndex = g_pstBrowser->wIndex;
	g_pstBrowser->wListIndex = g_pstBrowser->wIndex % g_pstXpgMovie->m_dwThumbCount;
	g_pstBrowser->wListFirstIndex = g_pstBrowser->wIndex - g_pstBrowser->wListIndex;
	g_pstXpgMovie->m_pstThumbFrame->m_boVisible = false;
	xpgUpdatePageBar(g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile, g_pstBrowser->wIndex);
#else
	register STXPGBROWSER *pstBrowser = g_pstBrowser;
	DWORD iFileCount = pstBrowser->wCount;
	SWORD iFileIndex = pstBrowser->wIndex;
	DWORD iThumbCount = g_pstXpgMovie->m_dwThumbCount;

	iFileIndex -= (iFileIndex % iThumbCount);
	iFileIndex += iThumbCount;
	if (iFileIndex >= iFileCount)
		iFileIndex = 0;

	xpgMenuListSetIndex(FileListSetCurIndex(iFileIndex), true);
       xpgClearCatch();
       ImageReleaseAllBuffer();
#endif
}



/*
#if FACE_DETECTION
typedef struct CvRect
{
    DWORD x;
    DWORD y;
    DWORD width;
    DWORD height;
}
CvRect;
#endif
*/
//------------------------------------------------------------------------------------------
//Photo View
//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   View photo main routine
///
///@return  false or true
///
BOOL xpgViewPhoto()
{
    return xpgViewPhotoEx(NULL);
}
BOOL xpgViewPhotoEx(STXPGSPRITE * pstSprite)
{
    MP_DEBUG("%s(), pstSprite = 0x%X", __FUNCTION__, pstSprite);

    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	ST_IMGWIN *pWin;
    SWORD swRet;
    BOOL cmp_val;

    xpgReleaseThumbBuffer(); //thumb catch use mem_malloc, release thumb buffer
    ImageReleaseAllBuffer();
    //xpgInitImagePlayer();
	psSysConfig->sImagePlayer.dwRotateInitFlag = FALSE;
	psSysConfig->sImagePlayer.dwZoomInitFlag = FALSE;
	g_boNeedRepaint = false;

	//if(g_pstBrowser->wCount == 0) return;

	//if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
	//	return FAIL;
#if NETWARE_ENABLE
	if (!isNetFunc_Mode())
#endif
	{
    BYTE bCurFileType = FileGetMediaType(&psSysConfig->sFileBrowser.sImgAndMovFileList[psSysConfig->sFileBrowser.dwImgAndMovCurIndex].bExt);
	if (bCurFileType == FILE_OP_TYPE_FOLDER)
		return FAIL;
	}

	pWin = Idu_GetNextWin();
	g_pXpgCanvasWin = pWin;

#if (ENABLE_SLIDESHOW_FUNCTION == 1)
    if ( (g_bAniFlag & ANI_SLIDE) && (g_psSetupMenu->bSlideSeparate == SETUP_MENU_SLIDESEPARATE_ON) )
        swRet = xpgSlideSeparate ();
    else
#endif
    {
        if (g_bAniFlag & ANI_SLIDE)
            swRet = ImageDraw(pWin, TRUE); // swRet = ImageDrawEx(pWin, TRUE, pstSprite); // in ImagePlayer.c
        else
            swRet = ImageDraw(pWin, FALSE); // in ImagePlayer.c
    }

	if (swRet != PASS)
	{
		if( g_bXpgStatus != XPG_MODE_CARDSEL )	//wifi unplug will jump to this page
		{
			Idu_PaintWin(pWin, 0x00008080);
#if 0
			//BYTE *cMsg = (BYTE *) GetSystemErrMsg(g_psSystemConfig->dwErrorCode);
			BYTE cMsg[] = "Not Supported";   // for temp message

			Idu_PrintString(pWin, cMsg,
							(pWin->wWidth - Idu_GetStringWidth(cMsg, 0)) >> 1,
							(pWin->wHeight - Idu_GetStringHeight(cMsg, 0)) >> 1, 0, 0);
#endif
			SystemClearErrMsg();
		}
		MP_ALERT("xpgViewPhoto(): call ImageDraw() fail, Show Not Supported message");
		return FAIL;
	}
	if( g_bXpgStatus != XPG_MODE_CARDSEL )
	{
		xpgUpdateCanvas();

		//xpgFavorUpdateIcon();
	}
#if NET_UPNP
	if( g_bXpgStatus == XPG_MODE_UPNP_USER_LIST )
	{
		xpgSearchAndGotoPage("Net_UserList");
		xpgUpdateStage();
	}
#endif
	g_boNeedRepaint = false;

//ImageDetectFace(pWin);
//ImageRemoveRedEye(pWin);
//ImageBeautifyFace(pWin);
//  ImageDynamicLighting(pWin);
//  ImageEmbossFiltering(pWin);
//  ImageMosaicFiltering(pWin);
//  ImagePosterFiltering(pWin);
//  ImageJitterFiltering(pWin);
//  ImageOilifyFiltering(pWin);

/*#if FACE_DETECTION
  CvRect Face;
  ImageDetectFace(pWin,Face);
//#endif
*/
	return (swRet == PASS ? TRUE : FAIL);

}
///
///@ingroup xpgPhotoFunc
///@brief   Enter(prepare) Photo View parameters before View Photo
///
void xpgCb_EnterPhotoView()
{
    mpDebugPrint("xpgCb_EnterPhotoView, g_bXpgStatus = %d", g_bXpgStatus);

	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;

#if 0
	if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
	{
		//xpgGotoPage(g_pstMenuPage->m_wIndex);
        mpDebugPrint("xpgSearchAndGotoPage(Mode_Photo) @@");
        xpgSearchAndGotoPage("Mode_Photo");
		xpgCb_OpenFolder();
		return;
	}
#endif

#if NETWARE_ENABLE
     if (isNetFunc_Mode())
     {
     		bZoomMode = FALSE;
     		xpgViewPhoto();
		return;
     }
#endif

	g_bXpgStatus = XPG_MODE_PHOTOVIEW;
	g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_PHOTOVIEW;

    mpDebugPrint("xpgMovieSearchPage(Photo_Viewer)");
	//g_pstMenuPage = xpgMovieSearchPage("Photo_Viewer");
	bZoomMode = FALSE;
	xpgViewPhoto();
	//Draw_Osd_Icon(20,420,NULL,0,OSDICON_Left);
	//Draw_Osd_Icon(750,420,NULL,0,OSDICON_Right);
    
}

///
///@ingroup xpgPhotoFunc
///@brief   Exit from Photot humb
///
void xpgCb_PhotoMenuThumbExit()
{
    MP_DEBUG("xpgCb_PhotoMenuListExit, g_bXpgStatus=%d", g_bXpgStatus);
    //g_bXpgStatus = XPG_MODE_MODESEL;
    g_bXpgStatus = XPG_MODE_PHOTOVIEW;
    Idu_OsdErase();
    g_boNeedRepaint=true;

    //Idu_OsdPutStr(Idu_GetOsdWin(), "Searching...", 200 , 30, SETUP_CHAR);

#if 0
    #if 1
      xpgSearchAndGotoPage("Main_SpyCam");
    #else
      xpgSearchAndGotoPage("Main_Photo");
    #endif
    Idu_OsdPutStr(Idu_GetOsdWin(), "MINIDV", 135 , 50, OSD_COLOR_WHITE);
#endif

    //Idu_OsdPutStr(Idu_GetOsdWin(), "MININDV", 200 , 30, SETUP_CHAR);
}
/*
void xpgCb_PhotoMenuThumbExit_MINIDV()
{
    mpDebugPrint("xpgCb_PhotoMenuListExit_MINIDV, g_bXpgStatus=%d", g_bXpgStatus);
    g_bXpgStatus = XPG_MODE_MODESEL;
    Idu_OsdErase();
    g_boNeedRepaint=true;
    //g_boNeedRepaint=false;
    xpgSearchAndGotoPage("Main_SpyCam");
    xpgUpdateStage();

}
*/
//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Change PhotoView to previous index
///
void xpgCb_PhotoViewPrev()
{
 #if NETWARE_ENABLE
	if (isNetFunc_Mode())
	{
		mpDebugPrint("%s --> NetFunc status =%d",__func__, g_bXpgStatus);
		xpgCb_NetBTNUp();
		xpgViewPhoto();
		return;
	}
#endif

	MP_DEBUG("xpgCb_PhotoViewPrev");
	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	DWORD dwFileType;

	do
	{
		g_pstBrowser->wIndex = FileListAddCurIndex(-1);
		g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex = g_pstBrowser->wIndex;
		dwFileType = FileBrowserGetFileType(g_pstBrowser->wIndex);
	}
	while (dwFileType == FILE_OP_TYPE_FOLDER);
	xpgViewPhoto();
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Change Photo Menu to next index
///
void xpgCb_PhotoViewNext()
{
	MP_DEBUG("xpgCb_PhotoViewNext");
#if NETWARE_ENABLE
	if (isNetFunc_Mode())
	{
		mpDebugPrint("%s --> NetFunc status =%d",__func__, g_bXpgStatus);
		xpgCb_NetBTNDown();
		xpgViewPhoto();
		return;
	}
#endif

	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	DWORD dwFileType;

	do
	{
		g_pstBrowser->wIndex = FileListAddCurIndex(1);
		g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex = g_pstBrowser->wIndex;
		dwFileType = FileBrowserGetFileType(g_pstBrowser->wIndex);
	}
	while (dwFileType == FILE_OP_TYPE_FOLDER);
	xpgViewPhoto();
}

//------------------------------------------------------------------------------------------
///
///@ingroup xpgPhotoFunc
///@brief   Exit from PhotoView and release all buffers
///
void xpgCb_PhotoViewExit()
{
#if NETWARE_ENABLE
	if (isNetFunc_Mode())
	{
		mpDebugPrint("%s --> NetFunc status =%d",__func__, g_bXpgStatus);

		//
		// Net_PhotoViewExit();
		//
		xpgSearchAndGotoPage("Net_PhotoList"); // // temp
		return;
	}
#endif

	MP_DEBUG("xpgCb_PhotoViewExit");
	if (bZoomMode) {
		bZoomMode = FALSE;
		xpgViewPhoto();
	} else
    {
    	//xpgInitImagePlayer ();
    	ImageReleaseAllBuffer();
#if FAVOR_HISTORY_SIZE
    	BYTE xpgMyFavorIndex = xpgGetMyFavorIndex();
    	if(xpgMyFavorIndex > 0)
    	{
    	   g_bXpgStatus = XPG_MODE_MYFAVORPHOTO;
           g_boNeedRepaint = true;
           xpgSearchAndGotoPage("Mode_PhotoFavor");
           xpgUpdateStage();
    	}
    	else
#endif
    	{
    	 //  xpgCb_GoModePage();
    	}
    }
}


#else

void NonxpgPhotoView()
{
	ImageDraw(Idu_GetCurrWin(),1);
	//SendUpdateEvent();
	FileListSetCurIndex(1);
}

void NonxpgEnterPhotoView()
{
	ST_IMGWIN *pWin = Idu_GetNextWin();
	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;

	{
    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());
		xpgStopAllAction();
		#if USBCAM_IN_ENABLE
		StopWebCam();
		#endif
	//	CheckToStopPreview();
    mpDebugPrint("%s: 000 =%d", __FUNCTION__,mem_get_free_space_total());
		g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
		g_pstBrowser->bCurDriveId = g_psSystemConfig->sStorage.dwCurStorageId;
		FileBrowserResetFileList(); /* reset old file list first */
		FileBrowserScanFileList(SEARCH_TYPE);

		FileListSetCurIndex(0);
//		g_bNonXpgStatus=N0N_XPG_MODE_VIDEOVIEW;
	}

	if (FileBrowserGetTotalFile() == 0) 
	{
		mpDebugPrint("%s:No  file in %d",__FUNCTION__,g_psSystemConfig->sStorage.dwCurStorageId);
		if (g_psSystemConfig->sStorage.dwCurStorageId != SD_MMC  && SystemCardPresentCheck(SD_MMC))
		{
			xpgChangeDrive(SD_MMC);
			Ui_TimerProcAdd(10, NonxpgPhotoView);
		}
		return;
	}
    Api_AudioHWEnable();
    mpDebugPrint("%s: 11  total=%d", __FUNCTION__,mem_get_free_space_total());

	NonxpgPhotoView();
	//SendUpdateEvent();
}

#endif



