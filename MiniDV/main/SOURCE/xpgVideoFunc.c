
#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "display.h"
#include "fs.h"
#include "mpapi.h"
#include "ui_timer.h"
#include "setup.h"
#include "ui.h"
#include "Taskid.h"
#include "../../../libIPLAY/libSrc/subtitle/include/sub_common.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if VIDEO_ON

#if MAKE_XPG_PLAYER

extern BYTE g_bVideoCurFilePreview;
static SWORD st_swFastSpeed=0; 

#define VIDEO_FULL_SCREEN   0
#define VIDEO_DRAW_THUMB    1
#if MINIDV_YBD_FUNCION  //crack flick tear
WORD VideoScreenMode = VIDEO_DRAW_THUMB;
#else
WORD VideoScreenMode = 0;
#endif
WORD xpgGetVideoScreenMode()
{
    return VideoScreenMode;
}
WORD videoFileNote; // 0 - disable
WORD xpgGetVideoFileNote()
{
    return videoFileNote;
}

//#if VIDEO_ON
void xpgPlayVideo();
void xpgStopVideo();

#if Video_Preview_Play_And_Stop
#define Video_Preview_In_Start   			1
#define Video_Preview_In_End   				9
static DWORD st_dwVideoPreviewMode=0;

BYTE InVideoPlayWait(void)
{
	return st_dwVideoPreviewMode;
}

void VideoPreviewKeyProcess(DWORD *pdwEvent, BYTE *pbKeyCode)
{
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

	mpDebugPrintN("VideoPreviewKeyProcess");
		if (dwHashKey != xpgHash("Video_Viewer"))
		{
			if (st_dwVideoPreviewMode)
				st_dwVideoPreviewMode=0;
			return;
		}
		mpDebugPrint("st_dwVideoPreviewMode=%d  *pbKeyCode=%d",st_dwVideoPreviewMode,*pbKeyCode);
		if (st_dwVideoPreviewMode==Video_Preview_In_Start)
		{
			if (*pbKeyCode == KEY_ENTER)
			{
				*pbKeyCode=KEY_NULL;
				xpgCb_VideoPlay();
			}
			else if (*pbKeyCode == KEY_UP)
			{
				*pbKeyCode=KEY_NULL;
				st_dwVideoPreviewMode=0;
				xpgMenuListPrev();
				xpgCb_VideoPlay();
			}
			else if (*pbKeyCode == KEY_DOWN)
			{
				*pbKeyCode=KEY_NULL;
				st_dwVideoPreviewMode=0;
				xpgMenuListNext();
				xpgCb_VideoPlay();
			}
		}
		else if (st_dwVideoPreviewMode==Video_Preview_In_End)
		{
			if (*pbKeyCode == KEY_ENTER)
			{
				*pbKeyCode=KEY_NULL;
				xpgVideoEnd();
			}
			else if (*pbKeyCode == KEY_UP)
			{
				*pbKeyCode=KEY_NULL;
				st_dwVideoPreviewMode=0;
				xpgStopAllAction();
				TimerDelay(1);
				xpgMenuListPrev();
				xpgCb_VideoPlay();
			}
			else if (*pbKeyCode == KEY_DOWN)
			{
				*pbKeyCode=KEY_NULL;
				st_dwVideoPreviewMode=0;
				xpgStopAllAction();
				TimerDelay(1);
				xpgMenuListNext();
				xpgCb_VideoPlay();
			}
		}
}

#endif
#if 0
void xpgGetVideoPreviewxywh(WORD *px, WORD *py, WORD *pw, WORD *ph)
{
#if VIDEO_ON
        WORD x, y, w, h;
        STXPGSPRITE *pSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_CURFILEPREVIEW, 0); // 30=SPRITE_TYPE_CURFILEPREVIEW
	    if (pSprite != NULL)
	    {
	        x = pSprite->m_wPx;
            y = pSprite->m_wPy;
            w = pSprite->m_wWidth;
            h = pSprite->m_wHeight;
        }
        else
        {
            STXPGMOVIE *pstMov = &g_stXpgMovie;
            x = pstMov->m_wScreenWidth/100*4;
            y = pstMov->m_wScreenHeight/48*14;
            w = pstMov->m_wScreenWidth/10*4;
            h = pstMov->m_wScreenHeight/48*19;
        }

        *px = x;
        *py = y;
        *pw = w;
        *ph = h;
#endif
}
#endif


//------------------------------------------------------------------------------------------
//Video Menu
//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Change Video Menu list to Next
///
void xpgCb_VideoMenuListNext()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoMenuListNext");
    if(g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile==0)
        return;
    if (g_bAniFlag & ANI_VIDEO || g_bAniFlag & ANI_READY)
    {
        mpDebugPrint("- Video_viewer() -");
        BYTE OriginalMode = g_bAniFlag;
        xpgStopVideo();
        FileListAddCurIndex(1);
        xpgSearchAndGotoPage("Video_Viewer");        
        xpgCb_EnterVideoPlayer();
        if (OriginalMode & ANI_VIDEO)
            xpgPlayVideo();
    }
    else
    {
        xpgMenuListNext();
//        if (g_bVideoCurFilePreview)
//            xpgUpdateVideoPreview(); // Jonny 20090401 for VideoPreview
#if 0
        xpgSetTimeBarXY(1024, 1024); // don't display
        ST_IMGWIN *pWin = Idu_GetCurrWin();
        WORD x, y, w, h;
        xpgGetVideoPreviewxywh(&x, &y, &w, &h);
        xpgVideoDrawPreview(pWin, x, y, w, h);
#endif
    }
#endif
}



///
///@ingroup xpgVideoFunc
///@brief   Change Video Menu list to previous index
///
void xpgCb_VideoMenuListPrev()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoMenuListPrev");

    if(g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile==0)
        return;

    if (g_bAniFlag & ANI_VIDEO || g_bAniFlag & ANI_READY)
    {
        BYTE OriginalMode = g_bAniFlag;

        xpgStopVideo();
        FileListAddCurIndex(-1);
        xpgSearchAndGotoPage("Video_Viewer");
        xpgCb_EnterVideoPlayer();

        if (OriginalMode & ANI_VIDEO)
            xpgPlayVideo();
    }
    else
    {
        xpgMenuListPrev();

//        if (g_bVideoCurFilePreview)
//            xpgUpdateVideoPreview(); // Jonny 20090401 for VideoPreview
#if 0
        xpgSetTimeBarXY(1024, 1024); // don't display
        ST_IMGWIN *pWin = Idu_GetCurrWin();
        WORD x, y, w, h;
        xpgGetVideoPreviewxywh(&x, &y, &w, &h);
        xpgVideoDrawPreview(pWin, x, y, w, h);
#endif
    }
#endif
}



///
///@ingroup xpgVideoFunc
///@brief   Exit from Video Menu list
///
void xpgCb_VideoMenuListExit(void) // act15
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoMenuListExit");
    g_bAniFlag &= ~ANI_VIDEO_PREVIEW;
    Idu_OsdErase();
    //xpgSearchAndGotoPage("Main_Video");
#endif
}



#if 0
void xpgCb_VideoMenuListNextLine()
{
    MP_DEBUG("xpgCb_VideoMenuListNextLine");
    xpgMenuListSetIndex(FileListAddCurIndex(g_pstXpgMovie->m_dwThumbColumns), false);
}



void xpgCb_VideoMenuListPrevLine()
{
    MP_DEBUG("xpgCb_VideoMenuListPrevLine");
    xpgMenuListSetIndex(FileListAddCurIndex(-g_pstXpgMovie->m_dwThumbColumns), false);
}
#endif



//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Change Video Menu list to Next page
///
void xpgCb_VideoMenuListNextPage()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoMenuListNextPage");
    xpgMenuListNextPage();
#endif
}



///
///@ingroup xpgVideoFunc
///@brief   Change Video Menu list to Previous page
///
void xpgCb_VideoMenuListPrevPage()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoMenuListPrevPage");
    xpgMenuListPrevPage();
#endif
}



//------------------------------------------------------------------------------------------
//Video Player
//------------------------------------------------------------------------------------------
//ST_IMGWIN *g_pMovieScreenWin;
//#if 0 // Jonny 20090401 for VideoPreview
//---------------------------------------------------------------------------

extern BOOL boVideoPreview;

///
///@ingroup xpgVideoFunc
///@brief   Do preview of Video file
///
///@param   pWin - selected image Win
///
///@param   x - left
///@param   y - top
///@param   w - width
///@param   h - height
///
void xpgVideoDrawPreview(ST_IMGWIN * pWin, DWORD x, DWORD y, DWORD w, DWORD h)
{
#if VIDEO_ON
    MP_DEBUG("xpgVideoDrawPreview");

    ST_SEARCH_INFO *pSearchInfo;
    pSearchInfo = &g_psSystemConfig->sFileBrowser.sImgAndMovFileList[g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex];
    if (pSearchInfo == NULL) return;

    if (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER)
        return;

    STREAM *sHandle;
    sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
    if (sHandle == NULL)
        return;

    boVideoPreview = TRUE;
    Idu_ChgWin(pWin);

    g_pMovieScreenWin = Idu_GetCurrWin();
    int rc;
    XPG_FUNC_PTR *xpgFuncPtr = (XPG_FUNC_PTR *)xpgGetFuncPtr();
    rc = Api_EnterMoviePreview(sHandle, pSearchInfo->bExt, g_pMovieScreenWin, &g_sMovieWin, x, y, w, h, xpgFuncPtr);
    if(rc >= 0) // 0-PASS
    {
        Api_ExitMoviePlayer(); //Api_MoviePlay(); // Jonny 20090401 for VideoPreview
        g_bAniFlag |= ANI_VIDEO_PREVIEW;
        //TimerDelay(1);
    }

    boVideoPreview = FALSE;
    return;
#endif
}



//---------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Draw Thumb Play of Video file
///
///@param   pWin - specified Win
///@param   x - left
///@param   y - top
///@param   w - width
///@param   h - height
///
int xpgVideoDrawThumb(ST_IMGWIN * pWin, DWORD x, DWORD y, DWORD w, DWORD h)
{
#if VIDEO_ON
    MP_DEBUG("xpgVideoDrawThumb()");
    ST_SEARCH_INFO *pSearchInfo;

    pSearchInfo = &g_psSystemConfig->sFileBrowser.sImgAndMovFileList[g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex];
    if (pSearchInfo == NULL) return;

    if (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER)
    {
        xpgSearchAndGotoPage("Mode_Video");
        xpgCb_OpenFolder();
        g_boNeedRepaint = false; // prevent XpgUi.c xpgprocessEvent() to call xpgUpdateStage() again
        return;
    }

    STREAM *sHandle;
    sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
    if (sHandle == NULL)
        return DECODE_FAIL;

    g_bXpgStatus = XPG_MODE_ENTERVIDEOPLAYER;
    boVideoPreview = TRUE;
    Idu_ChgWin(pWin);

    g_pMovieScreenWin = Idu_GetCurrWin();
    //int rc = Api_VideoDrawThumb(sHandle, pSearchInfo->bExt, pWin, x, y, w, h);
    XPG_FUNC_PTR *xpgFuncPtr = (XPG_FUNC_PTR *)xpgGetFuncPtr();
    int rc = Api_VideoDrawThumb(sHandle, pSearchInfo->bExt, g_pMovieScreenWin, &g_sMovieWin, x, y, w, h, xpgFuncPtr, VIDEO_PLAY_MODE);
    if (rc < 0) // Fail
    {
        FileClose(sHandle);
        Api_ExitMoviePlayer(); //Api_MoviePlay();
        MP_DEBUG("-E- video decode fail");
        return DECODE_FAIL;
    }

    g_bXpgStatus = XPG_MODE_VIDEOPLAYER;
    //xpgPlayVideo();
#endif

    return PASS;
}


//---------------------------------------------------------------------------
//#endif
///
///@ingroup xpgVideoFunc
///@brief   Prepare parameters to Play Video
///
void xpgPlayVideo()
{
#if VIDEO_ON
    MP_DEBUG("xpgPlayVideo");

    //if (g_bXpgStatus != XPG_MODE_VIDEOPLAYER)
    // return;
    g_boNeedRepaint = false;

    if (g_bAniFlag & ANI_VIDEO)
    {   // check if av playing
        if (g_bAniFlag & ANI_PAUSE)
        {
            Api_MovieResume();
            g_bAniFlag &= ~ANI_PAUSE;
#if SHOW_VIDEO_STATUS
			 Show_Video_Action_Icon(OSDICON_PLAY);
#endif
        }
        else
        {
            Api_MoviePause();
            xpgDelay(1);
            g_bAniFlag |= ANI_PAUSE;
#if SHOW_VIDEO_STATUS
			 Show_Video_Action_Icon(OSDICON_PAUSE);
#endif
        }

        xpgDelay(1);

        return;
    }

    g_bAniFlag &= ~ANI_READY;
    g_bAniFlag |= ANI_VIDEO;

    Idu_ChgWin(g_pMovieScreenWin);

    {
        STXPGMOVIE *pstMov = &g_stXpgMovie;
        WORD x=pstMov->m_wScreenWidth/10*3;
        WORD y=pstMov->m_wScreenHeight/10;
        WORD w=pstMov->m_wScreenWidth-x;
        WORD h=pstMov->m_wScreenHeight/12;
        Idu_OsdPaintArea(0, y, pstMov->m_wScreenWidth, h, 0);
    }

    if(VideoScreenMode != VIDEO_DRAW_THUMB){
        MP_ALERT("### call to Api_MovieVideoFullScreen() ###");
        Api_MovieVideoFullScreen();
    }
    // 0 : FULL SCREEN ON
   // MP_ALERT("### g_psSetupMenu->bVideoFullScreen = %d ###", g_psSetupMenu->bVideoFullScreen);

    Api_MoviePlay();
#if SHOW_VIDEO_STATUS
			 Show_Video_Action_Icon(OSDICON_PLAY);
#endif
#endif
}

//------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Stop Video Play
///
void xpgStopVideo()
{
#if VIDEO_ON
    MP_DEBUG("xpgStopVideo");
    g_boNeedRepaint = false;

    if (g_bAniFlag & ANI_VIDEO)
    {
        Api_MovieStop();

 #if (SC_USBDEVICE)
       if (SystemCheckUsbdPlugIn())
            TaskYield();
        else
#endif
            TimerDelay(1);
    }

    if ((g_bXpgStatus == XPG_MODE_VIDEOPLAYER)
    || (g_bXpgStatus == XPG_MODE_YOUTUBEFAVORSHOW)
    || (g_bXpgStatus == XPG_MODE_YOUTUBETHUMBSHOW)
    )
        Api_ExitMoviePlayer();

    g_bAniFlag &= ~ANI_VIDEO;
    g_bAniFlag &= ~ANI_READY;

//#if (!MINIDV)
    g_bXpgStatus = XPG_MODE_VIDEOMENU;
//#endif

#if (SC_USBDEVICE)
    if (SystemCheckUsbdPlugIn())
        TaskYield();
    else
#endif
        TimerDelay(1);
		//Idu_OsdPaintArea(0,OSD_ICON_ACTION_POS_Y,OSD_ICON_ACTION_POS_X+70+100,50,0); // clear OSD time display.
#if SHOW_VIDEO_STATUS
		CleanVideoPlayTime();
#endif
#endif
}



#if 0
//------------------------------------------------------------------------------
void xpgStopVideoPreview()
{
    MP_DEBUG("xpgStopVideoPreview");
    Api_MovieStop();
    TimerDelay(1);
    Api_ExitMoviePlayer();
    g_bAniFlag &= ~ANI_VIDEO_PREVIEW;
    TimerDelay(1);
}
#endif

//------------------------------------------------------------------------------
void VideoPlayerExit()
{
    MP_DEBUG("%s", __FUNCTION__);
#if VIDEO_ON
#if 1
    if (g_bAniFlag & ANI_VIDEO)
    {
        xpgStopVideo();
		xpgVideoEnd();
    }
#else
    if(videoFileNote == 1)
    {
        videoFileNote=0;
        Idu_OsdPaintArea(40, 420, 600, 30, 0); // 0=erase
    }

    if (g_bAniFlag & ANI_VIDEO)
    {
        xpgStopVideo();
    }

    #if AUTO_DEMO
    if (g_bDoingAutoDemo)
        g_bDoingAutoDemo = 0;
    #endif

#if HAVE_NETSTREAM
    if(xpgGetYouTubeThumb())
    {
        if(xpgGetVideoScreenMode() == VIDEO_FULL_SCREEN)
        {
            xpgSearchAndGotoPage("YouTubeThumb");
            xpgCb_YouTubeThumbEnter();
        } // else skip // VIDEO_DRAW_THUMB
    }
    else if(xpgGetYouTubeFavor())
    {
        if(xpgGetVideoScreenMode() == VIDEO_FULL_SCREEN)
        {
            xpgSearchAndGotoPage("YouTubeFavor");
            xpgCb_YouTubeFavorEnter();
        } // else skip // VIDEO_DRAW_THUMB
    }
    else
#endif
        xpgCb_GoModePage();

    Idu_OsdSetPainted();
    //Idu_OsdErase();

#if HAVE_NETSTREAM
    if(!xpgGetYouTubeThumb())
#endif
        Idu_OsdPaintArea(30, 340, 370, 100, 0); // clear ProgressBar

    Idu_OsdLoadPalette(OSD_BLEND_MAX);

    //In hign resolution mode, xpg volumeBar is not saved. We need to save volume value if changed
    if((Idu_GetCurrWin()->wWidth >= 1024))
    {
        extern BOOL bSetUpChg;

        if (bSetUpChg == true)
            PutSetupMenuValueWait();
    }
#endif
#endif
}

#if 1

//------------------------------------------------------------------------------
void xpgVideoEnd()
{
    MP_DEBUG("%s", __FUNCTION__);
#if VIDEO_ON

    BOOL boVideoPlaying;
    BYTE bVideoRpt;

#if Video_Preview_Play_And_Stop
	if (st_dwVideoPreviewMode==Video_Preview_In_End)
	{
		st_dwVideoPreviewMode=0;
	}
	else
	{
		st_dwVideoPreviewMode=Video_Preview_In_End;
		return;
	}
#endif

#if (PRODUCT_SUB_FUN==SF50_V10)
		SetSaveVideoIndex(FileBrowserGetCurIndex());
#endif
    if(videoFileNote == 1)
    {
        videoFileNote=0;
        Idu_OsdPaintArea(40, 420, 600, 30, 0); // 0=erase
    }
#if MINIDV_YBD_FUNCION
		g_psSetupMenu->bMovieRptState = SETUP_MENU_MOVIE_REPEAT_OFF;
#endif
    bVideoRpt = SETUP_MENU_MOVIE_REPEAT_ALL;//g_psSetupMenu->bMovieRptState;

#if HAVE_NETSTREAM
    if(xpgGetYouTubeFavor() || xpgGetYouTubeThumb() )
        bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;
#endif

    if ((g_psSystemConfig->dwCurrentOpMode == OP_MOVIE_MODE)
        && (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_OFF))
    {
        boVideoPlaying = (g_bAniFlag & ANI_VIDEO);

        {
#if AUTO_DEMO
            if (g_bDoingAutoDemo)
                g_bDoingAutoDemo = 0;
#endif
            xpgStopAllAction();
            TimerDelay(1);
        }

      //  bVideoRpt = g_psSetupMenu->bMovieRptState;

#if AUTO_DEMO
        if (g_bDoingAutoDemo)	//If movie is played by auto-demo event, then repeat all movies no matter what user sett
            bVideoRpt = SETUP_MENU_MOVIE_REPEAT_ALL;
#endif

        if (!boVideoPlaying)
            bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;

        if ((bVideoRpt == SETUP_MENU_MOVIE_REPEAT_ALLONCE) && (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex + 1 >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))	// not repeat and rewind
        {
            //xpgCb_GoModePage();
           // xpgCb_PressExitKey();
            //TimerDelay(3);
#if SHOW_VIDEO_STATUS
			CleanVideoPlayTime();
#endif
#if SENSOR_ENABLE
	        xpgCb_EnterCamcoderPreview();
            return;
#endif
        }
#if 0
        if ((bVideoRpt == SETUP_MENU_MOVIE_REPEAT_ALL) && (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex + 1 >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))	// not repeat and rewind
        {
            if(DEMO_AUTO)
            {
                VideoPlayerExit(); // do VideoExit first

                xpgSearchAndGotoPage("Mode_Photo");
                g_boNeedRepaint=true;
                xpgUpdateStage();
                xpgCb_EnterPhotoMenu();
                xpgCb_EnterPhotoView(); // wait until xpgPhotoFunc.c's xpgCb_EnterPhotoView()
                /*
                xpgSearchAndGotoPage("Mode_Music");
                g_boNeedRepaint=true;
                xpgUpdateStage();
                xpgCb_EnterMusicMenu();
                xpgCb_MusicPlay();*/
            }
            else
                xpgCb_GoModePage();
            return;
        }
#endif
        if (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_ONE)
        {   // not repeat one
            ST_SEARCH_INFO *pSearchInfo;
            do
            {
                FileListAddCurIndex(1);
                pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();
                //mpDebugPrint("pSearchInfo->bFileType = %d(5-FOLDER)", pSearchInfo->bFileType);
            } while (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER);
        }

        if (boVideoPlaying)
        {
            //xpgSearchAndGotoPage("Video_Viewer");
            g_bXpgStatus = XPG_MODE_NULL;

            //xpgCb_EnterVideoPlayer();
            if (VideoScreenMode != VIDEO_DRAW_THUMB)
            {
				#if (PRODUCT_UI==UI_HXJ_1)
				SetSaveVideoIndex(FileBrowserGetCurIndex());
				EnterVideoModePlay();
				#else
                xpgCb_EnterVideoPlayer();
				#endif
            }
            else
            {
				xpgVideoDrawThumb(Idu_GetCurrWin(), 0, 0, Idu_GetCurrWin()->wWidth, Idu_GetCurrWin()->wHeight);
            }

            TimerDelay(1);

            if (g_bXpgStatus == XPG_MODE_VIDEOPLAYER)
                xpgPlayVideo();

            return;
        }
    }
    else
    {
        VideoPlayerExit();
        TimerDelay(4);
#if MINIDV_YBD_FUNCION
		#if (PRODUCT_UI==UI_HXJ_1)
		EnterVideoModePlay();
		#else
		xpgSearchAndGotoPage("Mode_Video");
		Idu_OsdErase();
		xpgUpdateStage();
		#endif
#endif
    }
#endif
}


#else
//---------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Video Play End and check repeat off/one/all
///
void xpgVideoEnd()
{
#if VIDEO_ON
    mpDebugPrint("xpgVideoEnd ###");

    BOOL boVideoPlaying;
    BYTE bVideoRpt;
#if VIDEO_ON
    xpgClearStage();
#endif
    //Idu_OsdErase();
    if(videoFileNote == 1)
    {
        videoFileNote=0;
        Idu_OsdPaintArea(40, 420, 600, 30, 0); // 0=erase
    }

    bVideoRpt = g_psSetupMenu->bMovieRptState;

    if(xpgGetYouTubeFavor() || xpgGetYouTubeThumb() )
        bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;

    if ((g_psSystemConfig->dwCurrentOpMode == OP_MOVIE_MODE)
        && (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_OFF))
    {
        boVideoPlaying = (g_bAniFlag & ANI_VIDEO);

        {
#if AUTO_DEMO
            if (g_bDoingAutoDemo)
                g_bDoingAutoDemo = 0;
#endif
            xpgStopAllAction();
            TimerDelay(1);
        }

        bVideoRpt = g_psSetupMenu->bMovieRptState;

#if AUTO_DEMO
        if (g_bDoingAutoDemo)	//If movie is played by auto-demo event, then repeat all movies no matter what user sett
            bVideoRpt = SETUP_MENU_MOVIE_REPEAT_ALL;
#endif

        if (!boVideoPlaying)
            bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;

        if ((bVideoRpt == SETUP_MENU_MOVIE_REPEAT_OFF) && (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex + 1 >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))	// not repeat and rewind
        {
            xpgCb_GoModePage();
            //TimerDelay(3);
            return;
        }
#if 0
        if ((bVideoRpt == SETUP_MENU_MOVIE_REPEAT_ALL) && (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex + 1 >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))	// not repeat and rewind
        {
            if(DEMO_AUTO)
            {
                xpgCb_VideoPlayerExit(); // do VideoExit first

                xpgSearchAndGotoPage("Mode_Photo");
                g_boNeedRepaint=true;
                xpgUpdateStage();
                xpgCb_EnterPhotoMenu();
                xpgCb_EnterPhotoView(); // wait until xpgPhotoFunc.c's xpgCb_EnterPhotoView()
                /*
                xpgSearchAndGotoPage("Mode_Music");
                g_boNeedRepaint=true;
                xpgUpdateStage();
                xpgCb_EnterMusicMenu();
                xpgCb_MusicPlay();*/
            }
            else
                xpgCb_GoModePage();
            return;
        }
#endif
        if (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_ONE)
        {   // not repeat one
            ST_SEARCH_INFO *pSearchInfo;
            do
            {
                FileListAddCurIndex(1);
                pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();
                //mpDebugPrint("pSearchInfo->bFileType = %d(5-FOLDER)", pSearchInfo->bFileType);
            } while (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER);
        }

        if (boVideoPlaying)
        {
            //xpgSearchAndGotoPage("Video_Viewer");
            g_bXpgStatus = XPG_MODE_NULL;

            //xpgCb_EnterVideoPlayer();
            if (VideoScreenMode != VIDEO_DRAW_THUMB)
                xpgCb_EnterVideoPlayer();
            else
            {
                WORD x, y, w, h;
                xpgGetVideoPreviewxywh(&x, &y, &w, &h);
                xpgVideoDrawThumb(Idu_GetCurrWin(), x, y, w, h);
            }

            TimerDelay(1);

            if (g_bXpgStatus == XPG_MODE_VIDEOPLAYER)
                xpgPlayVideo();

            return;
        }
    }
    else
    {
        xpgCb_VideoPlayerExit();
        TimerDelay(4);
    }
#endif
}

#endif

//------------------------------------------------------------------------------
void EnterVideoPlayer()
{
    MP_DEBUG("%s", __FUNCTION__);
#if VIDEO_ON
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;

    xpgStopAllAction();

    if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
    {
        //xpgGotoPage(g_pstMenuPage->m_wIndex);
        xpgSearchAndGotoPage("Mode_Video");
        xpgCb_OpenFolder();
        g_boNeedRepaint = false; // prevent XpgUi.c xpgprocessEvent() to call xpgUpdateStage() again
        return;
    }

    if ((g_bXpgStatus == XPG_MODE_VIDEOPLAYER) || (g_bAniFlag & ANI_VIDEO)) // || g_bXpgStatus == XPG_MODE_ENTERVIDEOPLAYER)
        return;
//#if 0
    boVideoPreview = FALSE;
//#endif
    g_bXpgStatus = XPG_MODE_ENTERVIDEOPLAYER;
    struct ST_FILE_BROWSER_TAG *psFileBrowser;

    psFileBrowser = (struct ST_FILE_BROWSER_TAG *) &(g_psSystemConfig->sFileBrowser);
    MP_DEBUG1("dwCurrentOpMode = %x", g_psSystemConfig->dwCurrentOpMode);
    if (psFileBrowser->dwImgAndMovTotalFile == 0)
    {
        MP_DEBUG1("video file count = %x", psFileBrowser->dwImgAndMovTotalFile);
        g_boNeedRepaint = true;
        //xpgCb_GoModePage();

        return;
    }

    BOOL boSuccess = true;

    ST_SEARCH_INFO *pSearchInfo;
    STREAM *sHandle;

    pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();

    if (pSearchInfo == NULL)
        boSuccess = false;

    if (boSuccess)
    {
        sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
        if (sHandle == NULL)
            boSuccess = false;
        else
        {
            BYTE *fileName;
            ENTRY_FILENAME_TYPE filenameType;
            DWORD dwBufferCount;

            if (fileName = (BYTE *) ext_mem_malloc(640))
            {
                GetFilenameOfCurrentNode(DriveGet(DriveCurIdGet()), &filenameType, fileName, &dwBufferCount, 640);

                if (filenameType == E_FILENAME_UTF16_LONG_NAME)
                    mpx_UtilU16ToU08(fileName, (U16 *) fileName);

                MP_ALERT("%s - %s", __FUNCTION__, fileName);
                ext_mem_free(fileName);
            }
        }
    }

    if (boSuccess)
    {
        xpgSearchAndGotoPage("Video_Viewer");

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
        g_psDma->BtBtc |= 0x00800000;       // increase the priority of SDRAM auto reflash
#endif
        ST_IMGWIN *pWin = Idu_GetNextWin();
        BOOL boChangeResolution = false;

        if (DisplayInit(DISPLAY_INIT_LOW_RESOLUTION))
            boChangeResolution = true;

        MP_TRACE_LINE();

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
        ScalerClockReduce();
#endif

        //Idu_OsdSetNullPalette();
        g_pMovieScreenWin = Idu_GetCurrWin();

        videoFileNote=0;
        if(!memcmp(pSearchInfo->bName, "VCLP006", 7))
            videoFileNote=1;

        STREAM *srtHandle=NULL;
        #if DISPLAY_VIDEO_SUBTITLE
        BYTE pbTempBuffer[512];
        BYTE pbTempExtBuffer[5];
        int srt;
        DRIVE *srtDrv = DriveGet(DriveCurIdGet());
        char bName[9];
        memcpy(bName, pSearchInfo->bName, 8);
        bName[8]=0;
        //mpDebugPrint("bName=%s", &bName[0]);
        srtHandle=NULL;
        if(srtHandle == NULL)
        {
            srt = FileSearch(srtDrv, bName, "srt", E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                mpDebugPrint("%s.srt found, srtHandle = 0x%X", bName, srtHandle);
            }
            else
                mpDebugPrint("%s.srt NOT found!", bName);
        }
        if(srtHandle == NULL)
        {
            srt = FileSearch(srtDrv, bName, "smi", E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                mpDebugPrint("%s.smi found, subtitleHandle = 0x%X", bName, srtHandle);
            }
            else
                mpDebugPrint("%s.smi NOT found!", bName);
        }
        if(srtHandle == NULL) // try 8.3 with ".sami"
        {
            //mpDebugPrint("bName=%s", &bName[0]);
            memset(pbTempBuffer, 0, 512);
            int i;
            for(i=0; i < 8; i++)
                if(bName[i] != 0x20)
                    pbTempBuffer[i*2+1] = bName[i];
                else
                    break;
            int len = i;
            //mpDebugPrint("len = %d", len);
            pbTempBuffer[len*2+1] = 0x2e; // "."
            pbTempBuffer[len*2+3] = 0x73;//"s"
            pbTempBuffer[len*2+5] = 0x61; //"a"
            pbTempBuffer[len*2+7] = 0x6D; //"m"
            pbTempBuffer[len*2+9] = 0x69; //"i"
            //mpDebugPrint("00-07: %X %X %X %X %X %X %X %X", pbTempBuffer[0], pbTempBuffer[1], pbTempBuffer[2], pbTempBuffer[3], pbTempBuffer[4], pbTempBuffer[5], pbTempBuffer[6], pbTempBuffer[7]);
            //mpDebugPrint("08-15: %X %X %X %X %X %X %X %X", pbTempBuffer[8], pbTempBuffer[9], pbTempBuffer[10], pbTempBuffer[11], pbTempBuffer[12], pbTempBuffer[13], pbTempBuffer[14], pbTempBuffer[15]);
            //mpDebugPrint("16-23: %X %X %X %X %X %X %X %X", pbTempBuffer[16], pbTempBuffer[17], pbTempBuffer[18], pbTempBuffer[19], pbTempBuffer[20], pbTempBuffer[21], pbTempBuffer[22], pbTempBuffer[23]);


            srt = FileSearchLN(srtDrv, (WORD *) pbTempBuffer, len+5, E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                ;//mpDebugPrint("%s.sami found, srtHandle = 0x%X", bName, srtHandle);
            }
            else
                ;//mpDebugPrint("%s.sami NOT found!", bName);
        }

        if(srtHandle == NULL) // try long filename with ".sami"
        {
            memset(pbTempBuffer, 0, 512);
            DWORD dwBufferCount;
            if (PASS == FileGetLongName(srtDrv, pSearchInfo, pbTempBuffer, &dwBufferCount, sizeof(pbTempBuffer)))
            {
                //mpDebugPrint("dwBufferCount = %d", dwBufferCount);
                //mpDebugPrint("00-07: %X %X %X %X %X %X %X %X", pbTempBuffer[0], pbTempBuffer[1], pbTempBuffer[2], pbTempBuffer[3], pbTempBuffer[4], pbTempBuffer[5], pbTempBuffer[6], pbTempBuffer[7]);
                //mpDebugPrint("08-15: %X %X %X %X %X %X %X %X", pbTempBuffer[8], pbTempBuffer[9], pbTempBuffer[10], pbTempBuffer[11], pbTempBuffer[12], pbTempBuffer[13], pbTempBuffer[14], pbTempBuffer[15]);
                //mpDebugPrint("16-23: %X %X %X %X %X %X %X %X", pbTempBuffer[16], pbTempBuffer[17], pbTempBuffer[18], pbTempBuffer[19], pbTempBuffer[20], pbTempBuffer[21], pbTempBuffer[22], pbTempBuffer[23]);
                //mpDebugPrint("24-31: %X %X %X %X %X %X %X %X", pbTempBuffer[24], pbTempBuffer[25], pbTempBuffer[26], pbTempBuffer[27], pbTempBuffer[28], pbTempBuffer[29], pbTempBuffer[30], pbTempBuffer[31]);

                if(pbTempBuffer[dwBufferCount*2-7] == 0x2E) //.mov
                {
                    pbTempBuffer[dwBufferCount*2-5] = 0x73; //"s"
                    pbTempBuffer[dwBufferCount*2-3] = 0x61; //"a"
                    pbTempBuffer[dwBufferCount*2-1] = 0x6D; //"m"
                    pbTempBuffer[dwBufferCount*2+1] = 0x69; //"i"
                    dwBufferCount += 1;
                }
                if(pbTempBuffer[dwBufferCount*2-5] == 0x2E) // .rm
                {
                    pbTempBuffer[dwBufferCount*2-3] = 0x73; //"s"
                    pbTempBuffer[dwBufferCount*2-1] = 0x61; //"a"
                    pbTempBuffer[dwBufferCount*2+1] = 0x6D; //"m"
                    pbTempBuffer[dwBufferCount*2+3] = 0x69; //"i"
                    dwBufferCount += 2;
                }

                srt = FileSearchLN(srtDrv, (WORD *) pbTempBuffer, dwBufferCount, E_FILE_TYPE);
                if(srt == PASS)
                {
                  srtHandle = FileOpen(srtDrv);
                  //mpDebugPrint("*.same found!, after FileOpen(srtDrv), srtHandle = 0x%X", srtHandle);
                }
                else
                    ; //mpDebugPrint("%s.sami NOT found!", pbTempBuffer);
            }
            else
                ; //mpDebugPrint("FileGetLongName return FAIl! try 8.");
        } //if(srtHandle == NULL, try long filename)
        #endif

        STXPGMOVIE *pstMov = &g_stXpgMovie;
        STXPGSPRITE *pstSprite;
        int i;
        BYTE bRedCurtain = 0;

        if(xpgSearchAndGotoPage("RedCurtain"))
        {
            bRedCurtain = 1;
            for(i=0; i<20; i++) // total 20 .jpgs
            {
                DWORD startTime = GetSysTime();
                pstSprite = xpgSpriteFindType(g_pstXpgMovie, 0, i); // i - type index
                xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, 0, 0, pstSprite, 0);
                Idu_ChgWin(Idu_GetNextWin());
                IODelay(500);
                DWORD elapsedTime = SystemGetElapsedTime(startTime);
                //mpDebugPrint("elapsedTime = %d", elapsedTime);
            }

            if(!xpgSearchAndGotoPage("VideoViewer$$$"))
                MP_ALERT("GotoPage VideoViewer$$$  fail!");
        }

        //Idu_PaintWin(g_pMovieScreenWin, 0x00008080);
        Idu_PaintWin(g_pMovieScreenWin, RGB2YUV(0, 0, 0));
        SWORD swRet = 0;
        MP_TRACE_LINE();
        g_bAniFlag |= ANI_VIDEO;
        XPG_FUNC_PTR *xpgFuncPtr = (XPG_FUNC_PTR *)xpgGetFuncPtr();

        swRet = Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, srtHandle, g_pMovieScreenWin, &g_sMovieWin, xpgFuncPtr, VIDEO_PLAY_MODE, 0);
        if (swRet == FAIL)
        {
            FileClose(sHandle);
            if(srtHandle)
                FileClose(srtHandle);
            g_bAniFlag &= ~ANI_VIDEO;
            Idu_PaintWin(g_pMovieScreenWin, RGB2YUV(0, 0, 0));
#if 0//BMP_FONT_ENABLE
            BYTE cMsg[] = "Not Supported";

            Idu_PrintString(pWin, cMsg, (pWin->wWidth - Idu_GetStringWidth(cMsg, 0)) >> 1, (pWin->wHeight - Idu_GetStringHeight(cMsg, 0)) >> 1, 0, 0);
#endif
            SystemClearErrMsg();
            TimerDelay(5);

            VideoPlayerExit();
            //SystemSetErrEvent(swRet);

            return;
        }
        // else - PASS
        #if DISPLAY_VIDEO_SUBTITLE
		int ok;
        S_SUBTITLE_INFO_T subtitle_info;
        ok = Api_MovieGetSubtitleInfo(&subtitle_info);
		#endif
		//ShowItemIndex();
        #if 0
        Media_Info media_info;
        Media_Info *p_media_info=&media_info;
        swRet =Api_MovieGetMediaInfo(sHandle, pSearchInfo->bExt, p_media_info);
        if (swRet == PASS && !bRedCurtain)
        {
            BYTE bDispSec, bDispMin, bDispHour;
            BYTE bPrintBuf[32];
            WORD wValue = p_media_info->dwTotalTime;
            WORD screenWidth = g_psSystemConfig->sScreenSetting.pstCurTCON->wWidth;
            WORD screenHeight = g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight;
            WORD x, xa, xCodec, y, yLine, yGap;
            x=screenWidth / 8;
            xa=screenWidth / 2;
            xCodec=screenWidth / 5;
            y=screenHeight / 8;
            yLine = 42; //screenHeight/200*9;
            yGap = screenHeight / 12;
            bDispHour = wValue / 3600;
            wValue -= bDispHour * 3600;
            bDispMin = wValue / 60;
            bDispSec = wValue - bDispMin*60;
            Idu_OsdPutStr(Idu_GetOsdWin (), "Video Info", x, 20,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "------------------------", x, yLine,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "Video Codec :", x, y,      OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cVidoeCodec, x+160, y,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cVidoeCodec, x+xCodec, y,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "TotalTime : %d:%02d:%02d", bDispHour, bDispMin, bDispSec);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "ImageWidth : %d", p_media_info->dwImageWidth);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*2, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "ImageHeight : %d", p_media_info->dwImageHeight);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*3, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "TotalFrame : %d", p_media_info->dwTotalFrame);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*4, OSD_COLOR_WHITE);
            wValue = (WORD)(p_media_info->dwFrameRate+0.5);
            mp_sprintf(bPrintBuf, "FrameRate : %d", wValue);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*5, OSD_COLOR_WHITE);

            Idu_OsdPutStr(Idu_GetOsdWin (), "Audio Info", xa, 20,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "------------------------", xa, yLine,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "Audio Codec :", xa, y,      OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cAudioCodec, xa+160, y,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cAudioCodec, xa+xCodec, y,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "BitRate : %d kbps", p_media_info->dwBitrate/1000);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap, OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), "kbps", xa+180, y+yGap, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "SampleRate : %d", p_media_info->dwSampleRate);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap*2, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "SampleSize : %d", p_media_info->dwSampleSize);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap*3, OSD_COLOR_WHITE);

            MP_DEBUG("GetMediaInfo PASS!!");
            WaitAnyKeyContinue(5000);
            Idu_OsdPaintArea(x, 20, screenWidth, y+yGap*5+18, 0); // bottome line + font height //Idu_OsdErase();
        }
        #endif

        g_bAniFlag &= ~ANI_VIDEO;
#if ANTI_TEARING_ENABLE
        Idu_PaintWin(Idu_GetExtraWin(0), 0x00008080);
        Idu_PaintWin(Idu_GetExtraWin(1), 0x00008080);
#endif
        g_bXpgStatus = XPG_MODE_VIDEOPLAYER;
        g_bAniFlag |= ANI_READY;
        g_boNeedRepaint = false;

        if (boChangeResolution)
            TimerDelay(10);
    }
    else
    {
        //xpgCb_GoModePage();
        g_boNeedRepaint = true;
    }
#endif
}

#if 1

//*** VideoPlayer --------------------------------------------------------------
// act24
void xpgCb_EnterVideoPlayer()
{
    MP_DEBUG("%s", __FUNCTION__);
#if VIDEO_ON
    EnterVideoPlayer();
#endif
}

#else
//---------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Prepare parameters then call VideoPlay routine
///
void xpgCb_EnterVideoPlayer()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_EnterVideoPlayer");
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;

    xpgStopAllAction();

    if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
    {
        //xpgGotoPage(g_pstMenuPage->m_wIndex);
        xpgSearchAndGotoPage("Mode_Video");
        xpgCb_OpenFolder();
        g_boNeedRepaint = false; // prevent XpgUi.c xpgprocessEvent() to call xpgUpdateStage() again
        return;
    }

    if ((g_bXpgStatus == XPG_MODE_VIDEOPLAYER) || (g_bAniFlag & ANI_VIDEO)) // || g_bXpgStatus == XPG_MODE_ENTERVIDEOPLAYER)
        return;
//#if 0
    boVideoPreview = FALSE;
//#endif
    g_bXpgStatus = XPG_MODE_ENTERVIDEOPLAYER;
    struct ST_FILE_BROWSER_TAG *psFileBrowser;

    psFileBrowser = (struct ST_FILE_BROWSER_TAG *) &(g_psSystemConfig->sFileBrowser);
    MP_DEBUG1("dwCurrentOpMode = %x", g_psSystemConfig->dwCurrentOpMode);
    if (psFileBrowser->dwImgAndMovTotalFile == 0)
    {
        MP_DEBUG1("video file count = %x", psFileBrowser->dwImgAndMovTotalFile);
        g_boNeedRepaint = true;
        xpgCb_GoModePage();

        return;
    }

    BOOL boSuccess = true;

    ST_SEARCH_INFO *pSearchInfo;
    STREAM *sHandle;

    pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();

    if (pSearchInfo == NULL)
        boSuccess = false;

    if (boSuccess)
    {
        sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
        if (sHandle == NULL)
            boSuccess = false;
        else
        {
            BYTE *fileName;
            ENTRY_FILENAME_TYPE filenameType;
            DWORD dwBufferCount;

            if (fileName = (BYTE *) ext_mem_malloc(640))
            {
                GetFilenameOfCurrentNode(DriveGet(DriveCurIdGet()), &filenameType, fileName, &dwBufferCount, 640);

                if (filenameType == E_FILENAME_UTF16_LONG_NAME)
                    mpx_UtilU16ToU08(fileName, (U16 *) fileName);

                MP_ALERT("%s - %s", __FUNCTION__, fileName);
                ext_mem_free(fileName);
            }
        }
    }

    if (boSuccess)
    {
        xpgSearchAndGotoPage("Video_Viewer");
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
        g_psDma->BtBtc |= 0x00800000;       // increase the priority of SDRAM auto reflash
#endif
        ST_IMGWIN *pWin = Idu_GetNextWin();
        BOOL boChangeResolution = false;

        if (DisplayInit(DISPLAY_INIT_LOW_RESOLUTION))
            boChangeResolution = true;
        else
            xpgClearStage();

        MP_TRACE_LINE();

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
        ScalerClockReduce();
#endif

        //Idu_OsdSetNullPalette();
        g_pMovieScreenWin = Idu_GetCurrWin();

        videoFileNote=0;
        if(!memcmp(pSearchInfo->bName, "VCLP006", 7))
            videoFileNote=1;

        STREAM *srtHandle=NULL;
        #if DISPLAY_VIDEO_SUBTITLE
        BYTE pbTempBuffer[512];
        BYTE pbTempExtBuffer[5];
        int srt;
        DRIVE *srtDrv = DriveGet(DriveCurIdGet());
        char bName[9];
        memcpy(bName, pSearchInfo->bName, 8);
        bName[8]=0;
        //mpDebugPrint("bName=%s", &bName[0]);
        srtHandle=NULL;
        if(srtHandle == NULL)
        {
            srt = FileSearch(srtDrv, bName, "srt", E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                mpDebugPrint("%s.srt found, srtHandle = 0x%X", bName, srtHandle);
            }
            else
                mpDebugPrint("%s.srt NOT found!", bName);
        }
        if(srtHandle == NULL)
        {
            srt = FileSearch(srtDrv, bName, "smi", E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                mpDebugPrint("%s.smi found, subtitleHandle = 0x%X", bName, srtHandle);
            }
            else
                mpDebugPrint("%s.smi NOT found!", bName);
        }
        if(srtHandle == NULL) // try 8.3 with ".sami"
        {
            //mpDebugPrint("bName=%s", &bName[0]);
            memset(pbTempBuffer, 0, 512);
            int i;
            for(i=0; i < 8; i++)
                if(bName[i] != 0x20)
                    pbTempBuffer[i*2+1] = bName[i];
                else
                    break;
            int len = i;
            //mpDebugPrint("len = %d", len);
            pbTempBuffer[len*2+1] = 0x2e; // "."
            pbTempBuffer[len*2+3] = 0x73;//"s"
            pbTempBuffer[len*2+5] = 0x61; //"a"
            pbTempBuffer[len*2+7] = 0x6D; //"m"
            pbTempBuffer[len*2+9] = 0x69; //"i"
            //mpDebugPrint("00-07: %X %X %X %X %X %X %X %X", pbTempBuffer[0], pbTempBuffer[1], pbTempBuffer[2], pbTempBuffer[3], pbTempBuffer[4], pbTempBuffer[5], pbTempBuffer[6], pbTempBuffer[7]);
            //mpDebugPrint("08-15: %X %X %X %X %X %X %X %X", pbTempBuffer[8], pbTempBuffer[9], pbTempBuffer[10], pbTempBuffer[11], pbTempBuffer[12], pbTempBuffer[13], pbTempBuffer[14], pbTempBuffer[15]);
            //mpDebugPrint("16-23: %X %X %X %X %X %X %X %X", pbTempBuffer[16], pbTempBuffer[17], pbTempBuffer[18], pbTempBuffer[19], pbTempBuffer[20], pbTempBuffer[21], pbTempBuffer[22], pbTempBuffer[23]);


            srt = FileSearchLN(srtDrv, (WORD *) pbTempBuffer, len+5, E_FILE_TYPE);
            if(srt == FS_SUCCEED)
            {
                srtHandle = FileOpen(srtDrv);
                ;//mpDebugPrint("%s.sami found, srtHandle = 0x%X", bName, srtHandle);
            }
            else
                ;//mpDebugPrint("%s.sami NOT found!", bName);
        }

        if(srtHandle == NULL) // try long filename with ".sami"
        {
            memset(pbTempBuffer, 0, 512);
            DWORD dwBufferCount;
            if (PASS == FileGetLongName(srtDrv, pSearchInfo, pbTempBuffer, &dwBufferCount, sizeof(pbTempBuffer)))
            {
                //mpDebugPrint("dwBufferCount = %d", dwBufferCount);
                //mpDebugPrint("00-07: %X %X %X %X %X %X %X %X", pbTempBuffer[0], pbTempBuffer[1], pbTempBuffer[2], pbTempBuffer[3], pbTempBuffer[4], pbTempBuffer[5], pbTempBuffer[6], pbTempBuffer[7]);
                //mpDebugPrint("08-15: %X %X %X %X %X %X %X %X", pbTempBuffer[8], pbTempBuffer[9], pbTempBuffer[10], pbTempBuffer[11], pbTempBuffer[12], pbTempBuffer[13], pbTempBuffer[14], pbTempBuffer[15]);
                //mpDebugPrint("16-23: %X %X %X %X %X %X %X %X", pbTempBuffer[16], pbTempBuffer[17], pbTempBuffer[18], pbTempBuffer[19], pbTempBuffer[20], pbTempBuffer[21], pbTempBuffer[22], pbTempBuffer[23]);
                //mpDebugPrint("24-31: %X %X %X %X %X %X %X %X", pbTempBuffer[24], pbTempBuffer[25], pbTempBuffer[26], pbTempBuffer[27], pbTempBuffer[28], pbTempBuffer[29], pbTempBuffer[30], pbTempBuffer[31]);

                if(pbTempBuffer[dwBufferCount*2-7] == 0x2E) //.mov
                {
                    pbTempBuffer[dwBufferCount*2-5] = 0x73; //"s"
                    pbTempBuffer[dwBufferCount*2-3] = 0x61; //"a"
                    pbTempBuffer[dwBufferCount*2-1] = 0x6D; //"m"
                    pbTempBuffer[dwBufferCount*2+1] = 0x69; //"i"
                    dwBufferCount += 1;
                }
                if(pbTempBuffer[dwBufferCount*2-5] == 0x2E) // .rm
                {
                    pbTempBuffer[dwBufferCount*2-3] = 0x73; //"s"
                    pbTempBuffer[dwBufferCount*2-1] = 0x61; //"a"
                    pbTempBuffer[dwBufferCount*2+1] = 0x6D; //"m"
                    pbTempBuffer[dwBufferCount*2+3] = 0x69; //"i"
                    dwBufferCount += 2;
                }

                srt = FileSearchLN(srtDrv, (WORD *) pbTempBuffer, dwBufferCount, E_FILE_TYPE);
                if(srt == PASS)
                {
                  srtHandle = FileOpen(srtDrv);
                  //mpDebugPrint("*.same found!, after FileOpen(srtDrv), srtHandle = 0x%X", srtHandle);
                }
                else
                    ; //mpDebugPrint("%s.sami NOT found!", pbTempBuffer);
            }
            else
                ; //mpDebugPrint("FileGetLongName return FAIl! try 8.");
        } //if(srtHandle == NULL, try long filename)
        #endif

        //Idu_PaintWin(g_pMovieScreenWin, 0x00008080);
        Idu_PaintWin(g_pMovieScreenWin, RGB2YUV(0, 0, 0));
        SWORD swRet = 0;
        MP_TRACE_LINE();
        g_bAniFlag |= ANI_VIDEO;
        XPG_FUNC_PTR *xpgFuncPtr = (XPG_FUNC_PTR *)xpgGetFuncPtr();

        swRet = Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, srtHandle, g_pMovieScreenWin, &g_sMovieWin, xpgFuncPtr, VIDEO_PLAY_MODE, 0);
        if (swRet == FAIL)
        {
            FileClose(sHandle);
            if(srtHandle)
                FileClose(srtHandle);
            g_bAniFlag &= ~ANI_VIDEO;
            Idu_PaintWin(g_pMovieScreenWin, RGB2YUV(0, 0, 0));
            BYTE cMsg[] = "Not Supported";

            //mpDebugPrint("%s %s:Idu_PrintString ###", __FUNC__, __LINE__);
            Idu_PrintString(pWin, cMsg, (pWin->wWidth - Idu_GetStringWidth(cMsg, 0)) >> 1, (pWin->wHeight - Idu_GetStringHeight(cMsg, 0)) >> 1, 0, 0);
            SystemClearErrMsg();
            TimerDelay(5);

            xpgCb_VideoPlayerExit();
            //SystemSetErrEvent(swRet);

            return;
        }
        // else - PASS
        #if DISPLAY_VIDEO_SUBTITLE
		  int ok;
          S_SUBTITLE_INFO_T subtitle_info;
          ok = Api_MovieGetSubtitleInfo(&subtitle_info);
		#endif

        Media_Info media_info;
        Media_Info *p_media_info=&media_info;
        swRet =Api_MovieGetMediaInfo(sHandle, pSearchInfo->bExt, p_media_info);
        if(swRet == PASS)
        {
            BYTE bDispSec, bDispMin, bDispHour;
            BYTE bPrintBuf[32];
            WORD wValue = p_media_info->dwTotalTime;
            WORD screenWidth = g_psSystemConfig->sScreenSetting.pstCurTCON->wWidth;
            WORD screenHeight = g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight;
            WORD x, xa, xCodec, y, yLine, yGap;
            x=screenWidth / 8;
            xa=screenWidth / 2;
            xCodec=screenWidth / 5;
            y=screenHeight / 8;
            yLine = 42; //screenHeight/200*9;
            yGap = screenHeight / 12;
            bDispHour = wValue / 3600;
            wValue -= bDispHour * 3600;
            bDispMin = wValue / 60;
            bDispSec = wValue - bDispMin*60;
            Idu_OsdPutStr(Idu_GetOsdWin (), "Video Info", x, 20,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "------------------------", x, yLine,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "Video Codec :", x, y,      OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cVidoeCodec, x+160, y,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cVidoeCodec, x+xCodec, y,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "TotalTime : %d:%02d:%02d", bDispHour, bDispMin, bDispSec);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "ImageWidth : %d", p_media_info->dwImageWidth);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*2, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "ImageHeight : %d", p_media_info->dwImageHeight);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*3, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "TotalFrame : %d", p_media_info->dwTotalFrame);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*4, OSD_COLOR_WHITE);
            wValue = (WORD)(p_media_info->dwFrameRate+0.5);
            mp_sprintf(bPrintBuf, "FrameRate : %d", wValue);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x, y+yGap*5, OSD_COLOR_WHITE);

            Idu_OsdPutStr(Idu_GetOsdWin (), "Audio Info", xa, 20,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "------------------------", xa, yLine,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), "Audio Codec :", xa, y,      OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cAudioCodec, xa+160, y,      OSD_COLOR_WHITE);
            Idu_OsdPutStr(Idu_GetOsdWin (), p_media_info->cAudioCodec, xa+xCodec, y,      OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "BitRate : %d kbps", p_media_info->dwBitrate/1000);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap, OSD_COLOR_WHITE);
            //Idu_OsdPutStr(Idu_GetOsdWin (), "kbps", xa+180, y+yGap, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "SampleRate : %d", p_media_info->dwSampleRate);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap*2, OSD_COLOR_WHITE);
            mp_sprintf(bPrintBuf, "SampleSize : %d", p_media_info->dwSampleSize);
            Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, xa, y+yGap*3, OSD_COLOR_WHITE);

            MP_DEBUG("GetMediaInfo PASS!!");
            WaitAnyKeyContinue(5000);
            Idu_OsdPaintArea(x, 20, screenWidth, y+yGap*5+18, 0); // bottome line + font height //Idu_OsdErase();
        }

        g_bAniFlag &= ~ANI_VIDEO;
#if ANTI_TEARING_ENABLE
        Idu_PaintWin(Idu_GetExtraWin(0), 0x00008080);
        Idu_PaintWin(Idu_GetExtraWin(1), 0x00008080);
#endif
        g_bXpgStatus = XPG_MODE_VIDEOPLAYER;
        g_bAniFlag |= ANI_READY;
        g_boNeedRepaint = false;

        if (boChangeResolution)
            TimerDelay(10);
    }
    else
    {
        xpgCb_GoModePage();
        g_boNeedRepaint = true;
    }
#endif
}

#endif

//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Exit from VideoPlayer and do clear OSD
///
void xpgCb_VideoPlayerExit()
{
#if VIDEO_ON
    mpDebugPrint("xpgCb_VideoPlayerExit");
    if (g_bAniFlag & ANI_VIDEO)
    {
        xpgStopVideo();
    }

#if 1
	//SetSaveVideoIndex(FileBrowserGetCurIndex());
	EnterVideoModePlay();
#else
    if(videoFileNote == 1)
    {
        videoFileNote=0;
        Idu_OsdPaintArea(40, 420, 600, 30, 0); // 0=erase
    }

    #if AUTO_DEMO
    if (g_bDoingAutoDemo)
        g_bDoingAutoDemo = 0;
    #endif

#if HAVE_NETSTREAM
    if(xpgGetYouTubeThumb())
    {
        if(xpgGetVideoScreenMode() == VIDEO_FULL_SCREEN)
        {
            xpgSearchAndGotoPage("YouTubeThumb");
            xpgCb_YouTubeThumbEnter();
        }   // else skip // VIDEO_DRAW_THUMB
    }
    else if(xpgGetYouTubeFavor())
    {
        if(xpgGetVideoScreenMode() == VIDEO_FULL_SCREEN)
        {
            xpgSearchAndGotoPage("YouTubeFavor");
            xpgCb_YouTubeFavorEnter();
        }   // else skip // VIDEO_DRAW_THUMB
    }
    else
#endif
        xpgCb_GoModePage();

    Idu_OsdSetPainted();
    //Idu_OsdErase();

#if HAVE_NETSTREAM
    if(!xpgGetYouTubeThumb())
#endif
        Idu_OsdPaintArea(30, 340, 370, 100, 0); // clear ProgressBar

    Idu_OsdLoadPalette(OSD_BLEND_MAX);

    //In hign resolution mode, xpg volumeBar is not saved. We need to save volume value if changed
    if((Idu_GetCurrWin()->wWidth >= 1024))
    {
        extern BOOL bSetUpChg;

        if (bSetUpChg == true)
            PutSetupMenuValueWait();
    }
#endif
#endif
}



//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Check Preview or Play and check Full/Thumb of Video Play
///
#define DEBUG_MSN
//#undef DEBUG_MSN

void xpgCb_VideoPlay()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoPlay");

    if (g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile == 0)
    {
        g_boNeedRepaint = false;
        return;
    }
#if 0
    //if (g_bXpgStatus != XPG_MODE_VIDEOPLAYER) // Jonny 20090424 to prevent ReEntrant when video playing
    if ( (g_bXpgStatus != XPG_MODE_VIDEOPLAYER) && (g_bXpgStatus != XPG_MODE_ENTERVIDEOPLAYER) )
    {
        if (g_bXpgStatus != XPG_MODE_VIDEOPREVIEW) // Jonny 20090401 for VideoPreview
        {
            if(g_bVideoCurFilePreview) // set in xpgFunc.c's xpgDrawSprite_CurFilePreview()
            {
                g_bXpgStatus = XPG_MODE_VIDEOPREVIEW;
                //xpgInitTimeBar();
                ST_IMGWIN *pWin = Idu_GetCurrWin();
                xpgVideoDrawPreview(pWin, x, y, w, h);
                g_boNeedRepaint = false;
                return;
            }
        }
    }
#endif
#if SHOW_VIDEO_STATUS
		if (st_swFastSpeed!=0)
		{
			st_swFastSpeed=0;
			xpgCb_VideoFastSpeed();
			return;
		}

#endif

    MP_ALERT("### g_bXpgStatus=%d ###", g_bXpgStatus);
    //g_bXpgStatus = XPG_MODE_VIDEOPLAYER;
    if (g_bXpgStatus != XPG_MODE_VIDEOPLAYER)
    {

#if Video_Preview_Play_And_Stop
        xpgSearchAndGotoPage("Video_Viewer");
        ST_IMGWIN *pWin = Idu_GetNextWin();

	if (st_dwVideoPreviewMode==Video_Preview_In_Start)
	{
		st_dwVideoPreviewMode=0;
		xpgCb_EnterVideoPlayer();
	}
	else
	{
		st_dwVideoPreviewMode=Video_Preview_In_Start;
		xpgVideoDrawPreview(pWin, 0, 0, pWin->wWidth, pWin->wHeight); 
		mpCopyEqualWin(Idu_GetCurrWin(), pWin);
		g_bAniFlag &= ~ANI_VIDEO_PREVIEW;
		g_boNeedRepaint=0;
	}

#else

        //xpgClearStage();
        //Idu_OsdPaintArea(500, 90, 100, 40, 0); // reset
        if(VideoScreenMode != VIDEO_DRAW_THUMB)
        {

          #ifdef DEBUG_MSN
            mpDebugPrint("xpgCb_EnterVideoPlayer() @@");
          #endif

            Idu_OsdErase();
            MP_ALERT("### xpgCb_EnterVideoPlayer() ###");
            xpgCb_EnterVideoPlayer();
        }
        else
        {
#if 1//MINIDV_YBD_FUNCION  //crack flick tear
			xpgVideoDrawThumb(Idu_GetCurrWin(), 0, 0, Idu_GetCurrWin()->wWidth, Idu_GetCurrWin()->wHeight);
#else
			WORD x, y, w, h;
			xpgGetVideoPreviewxywh(&x, &y, &w, &h);
			xpgVideoDrawThumb(Idu_GetCurrWin(), x, y, w, h);
#endif
        }
#endif

        xpgDelay(1);
    }

#if 1
    if (g_bXpgStatus == XPG_MODE_VIDEOPLAYER)
    {
        #ifdef DEBUG_MSN
        mpDebugPrint("xpgPlayVideo() @@");
        #endif

        xpgPlayVideo(); // It will check Pause/Resume first!
       // RemoveTimerProc(xpgEraseIcon);
       // AddTimerProc(5, xpgEraseIcon);
    }
#endif
#endif
}


//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Stop Video
///
void xpgCb_VideoStop()
{
#if VIDEO_ON
    MP_DEBUG("xpgCb_VideoStop");

    g_boNeedRepaint = false;

    xpgStopVideo();
    //Api_MovieShowFirstFrame();
#endif
}



//------------------------------------------------------------------------------------------
///
///@ingroup xpgVideoFunc
///@brief   Pause video Play
///
void xpgCb_VideoPause()
{
  MP_DEBUG("xpgCb_VideoPause");
}

#define ABS(a) ((a) >= 0 ? (a) : (-(a)))

void ChangeFastSpeed(SWORD swDirect)
{
	SWORD swChange;

	#if 1
	st_swFastSpeed += swDirect;
	if (st_swFastSpeed<-8)
		st_swFastSpeed=-8;
	else if (st_swFastSpeed>8)
		st_swFastSpeed=8;
	#else
	if (swDirect < 0)
	{
		if (st_swFastSpeed == 0)
			st_swFastSpeed = -2;
		else if (st_swFastSpeed <= -16) 
			st_swFastSpeed = 0;
		else if (st_swFastSpeed < 0)
			st_swFastSpeed += st_swFastSpeed;
		else //if (st_swFastSpeed >0)
		{
			st_swFastSpeed -= (st_swFastSpeed>>1);
			if (st_swFastSpeed < 2)
				st_swFastSpeed = 0;
			else if (st_swFastSpeed>8)
				st_swFastSpeed = 8;
		}
	}
	else if (swDirect > 0)
	{
		if (st_swFastSpeed == 0)
			st_swFastSpeed = 2;
		else if (st_swFastSpeed>=16)
			st_swFastSpeed = 0;
		else if (st_swFastSpeed > 0)
			st_swFastSpeed +=st_swFastSpeed;
		else if (st_swFastSpeed >=-2)
			st_swFastSpeed = 0;
		else if (st_swFastSpeed<=-16)
			st_swFastSpeed = -8;
		else
			st_swFastSpeed += ABS(st_swFastSpeed)/2;
	}
	#endif
}

SWORD GetFastSpeed(void)
{
	return st_swFastSpeed;
}

void xpgCb_VideoFastSpeed()
{
#if SHOW_VIDEO_STATUS
	SWORD swSpeed=st_swFastSpeed;

    g_boNeedRepaint = false;
	if (!(g_bAniFlag & ANI_VIDEO))
	{
		st_swFastSpeed = 0;
		Clear_Video_Action_Icon();
		return;
	}
	if (swSpeed > 0)
    {
		 Show_Video_Action_Icon(OSDICON_FF);
        Api_MovieForward(swSpeed);
    }
	else if (swSpeed < 0)
    {
		 Show_Video_Action_Icon(OSDICON_FB);
        Api_MovieBackward(ABS(swSpeed));
    }
	else
	{
		Show_Video_Action_Icon(OSDICON_PLAY);
    	//Api_MoviePlay();
		Api_MovieResume();
		return;
	}
	Ui_TimerProcAdd(100, xpgCb_VideoFastSpeed);
#endif
}
///
///@ingroup xpgVideoFunc
///@brief   Forward Video Play by g_psSetupMenu->bVideoForwardSecond
///
void xpgCb_VideoForward()
{
	ChangeFastSpeed(1);
	xpgCb_VideoFastSpeed();
#if 0//VIDEO_ON
    MP_DEBUG("xpgCb_VideoForward");
    BYTE i; DWORD videoForwardSecond=1;

    for(i=0; i<= g_psSetupMenu->bVideoForwardSecond; i++)
        videoForwardSecond *= 2;

    if ((g_bAniFlag & ANI_VIDEO))       //IR_FORWARD Video
    {
		 Show_Video_Action_Icon(OSDICON_FF);
        Api_MovieForward(videoForwardSecond);
    }

    g_boNeedRepaint = false;
#endif
}



///
///@ingroup xpgVideoFunc
///@brief   Backward Video Play by g_psSetupMenu->bVideoForwardSecond
///
void xpgCb_VideoBackward()
{
	ChangeFastSpeed(-1);
	xpgCb_VideoFastSpeed();
#if 0//VIDEO_ON
    MP_DEBUG("xpgCb_VideoBackward");
    BYTE i; DWORD videoForwardSecond=1;

    for(i=0; i<= g_psSetupMenu->bVideoForwardSecond; i++)
        videoForwardSecond *= 2;

    if ((g_bAniFlag & ANI_VIDEO))       //IR_BACKWARD Video
    {
		 Show_Video_Action_Icon(OSDICON_FB);
        Api_MovieBackward(videoForwardSecond);
    }

    g_boNeedRepaint = false;
#endif
}



///
///@ingroup xpgVideoFunc
///@brief   Toggle FULL/THUMB screen mode OSD display
///
void xpgCb_VideoFullScreen()
{
#if VIDEO_ON
    MP_DEBUG("%s()", __FUNCTION__);
    if(g_bXpgStatus == XPG_MODE_VIDEOPREVIEW)
    {
        g_boNeedRepaint = false;
        return; // skip
    }

    if(VideoScreenMode == VIDEO_FULL_SCREEN)
    {
        VideoScreenMode = VIDEO_DRAW_THUMB;
        Api_audio_channel_select(1, 1); // (0, 1)
    }
    else
    {
        VideoScreenMode = VIDEO_FULL_SCREEN;
        Api_audio_channel_select(1, 1); // (1, 0)
    }

#endif

#if HAVE_NETSTREAM
    if(xpgGetYouTubeThumb())
        g_boNeedRepaint = false;

    if(xpgGetYouTubeFavor())
        g_boNeedRepaint = false;
#endif
}

#else //#if MAKE_XPG_PLAYER

//extern BYTE g_bNonXpgStatus;

void NonxpgStopVideo()
{
#if VIDEO_ON
    MP_DEBUG("xpgStopVideo");
    //g_boNeedRepaint = false;

    if (g_bAniFlag & ANI_VIDEO)
    {
        Api_MovieStop();

 #if (SC_USBDEVICE)
       if (SystemCheckUsbdPlugIn())
            TaskYield();
        else
#endif
            TimerDelay(1);
    }

    if ((g_bXpgStatus == XPG_MODE_VIDEOPLAYER)
    || (g_bXpgStatus == XPG_MODE_YOUTUBEFAVORSHOW)
    || (g_bXpgStatus == XPG_MODE_YOUTUBETHUMBSHOW)
    )
        Api_ExitMoviePlayer();

    g_bAniFlag &= ~ANI_VIDEO;
    g_bAniFlag &= ~ANI_READY;

//#if (!MINIDV)
    g_bXpgStatus = XPG_MODE_VIDEOMENU;
//#endif

#if (SC_USBDEVICE)
    if (SystemCheckUsbdPlugIn())
        TaskYield();
    else
#endif
        TimerDelay(1);
		//Idu_OsdPaintArea(0,OSD_ICON_ACTION_POS_Y,OSD_ICON_ACTION_POS_X+70+100,50,0); // clear OSD time display.
#if SHOW_VIDEO_STATUS
		CleanVideoPlayTime();
#endif
#endif
}
void  NonxpgExitVideoPlay()
{
    MP_DEBUG("NonxpgExitVideoPlay");
		if (g_bAniFlag & ANI_VIDEO)
		{
			Api_MovieStop();
			TimerDelay(1);
			Api_ExitMoviePlayer();
		}
}

void NonxpgVideoEnd()
{
#if VIDEO_ON
	BOOL boVideoPlaying;
	BYTE bVideoRpt;
    mpDebugPrint("%s(),%p", __FUNCTION__,g_bAniFlag);

	//g_psSetupMenu->bMovieRptState = SETUP_MENU_MOVIE_REPEAT_ALL;
	bVideoRpt = SETUP_MENU_MOVIE_REPEAT_ALL;//g_psSetupMenu->bMovieRptState;

	if (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_OFF)
	{
		boVideoPlaying = (g_bAniFlag & ANI_VIDEO);
		xpgStopAllAction();
		TimerDelay(1);

		if (!boVideoPlaying)
			bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;

		if ((bVideoRpt == SETUP_MENU_MOVIE_REPEAT_ALLONCE) && (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex + 1 >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))	// not repeat and rewind
		{
			bVideoRpt = SETUP_MENU_MOVIE_REPEAT_OFF;
		}
		else if (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_ONE)
		{
			ST_SEARCH_INFO *pSearchInfo;
			do
			{
				FileListAddCurIndex(1);
				pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();
				//mpDebugPrint("pSearchInfo->bFileType = %d(5-FOLDER)", pSearchInfo->bFileType);
			} while (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER);
		}

		if (bVideoRpt != SETUP_MENU_MOVIE_REPEAT_OFF)
		{
			NonxpgEnterVideoPlay();
			//TimerDelay(1);
			//Api_MovieVideoFullScreen();
			//Api_MoviePlay();

		}
	}
	
	if (bVideoRpt == SETUP_MENU_MOVIE_REPEAT_OFF)
	{
		NonxpgExitVideoPlay();
		TimerDelay(4);
	}
    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());
#endif
}

void  NonxpgEnterVideoPlay()
{
	ST_SEARCH_INFO *pSearchInfo;
	STREAM *sHandle;
    //mpDebugPrint("%s()", __FUNCTION__);

    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());
	g_bXpgStatus = XPG_MODE_ENTERVIDEOPLAYER;
	pSearchInfo = (ST_SEARCH_INFO *)FileGetCurSearchInfo();

	if (pSearchInfo)
	{
		sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
		g_pMovieScreenWin = Idu_GetCurrWin();
		mpClearWin(g_pMovieScreenWin);
		g_bAniFlag |= ANI_VIDEO;
		SWORD swRet = Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, NULL, g_pMovieScreenWin, &g_sMovieWin, NULL, VIDEO_PLAY_MODE, 0);
		if (swRet == FAIL)
		{
			FileClose(sHandle);
			g_bAniFlag &= ~ANI_VIDEO;

			NonxpgExitVideoPlay();
			return;
		}
    mpDebugPrint("%s: 111=%d", __FUNCTION__,mem_get_free_space_total());

		g_bAniFlag &= ~ANI_VIDEO;
		g_bXpgStatus = XPG_MODE_VIDEOPLAYER;
		g_bAniFlag |= ANI_READY;
		xpgDelay(1);
		//xpgPlayVideo();
		g_bAniFlag &= ~ANI_READY;
		g_bAniFlag |= ANI_VIDEO;
		Api_MovieVideoFullScreen();
		Api_MoviePlay();

	}
    mpDebugPrint("%s: 222=%d", __FUNCTION__,mem_get_free_space_total());

}

void NonxpgVideoView()
{
	ST_IMGWIN *pWin = Idu_GetNextWin();
	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;

/*
	if (g_bNonXpgStatus==N0N_XPG_MODE_VIDEOVIEW)
	{
		NonxpgExitVideoPlay();
		FileListAddCurIndex(1);
	}
	else
*/
	{
    mpDebugPrint("%s: total=%d", __FUNCTION__,mem_get_free_space_total());
		xpgStopAllAction();
		#if USBCAM_IN_ENABLE
		StopWebCam();
		#endif
	//	CheckToStopPreview();
    mpDebugPrint("%s: 000 =%d", __FUNCTION__,mem_get_free_space_total());
		g_psSystemConfig->dwCurrentOpMode = OP_MOVIE_MODE;
		g_pstBrowser->bCurDriveId = g_psSystemConfig->sStorage.dwCurStorageId;
		FileBrowserResetFileList(); /* reset old file list first */
		FileBrowserScanFileList(SEARCH_TYPE);

		FileListSetCurIndex(0);
//		g_bNonXpgStatus=N0N_XPG_MODE_VIDEOVIEW;
	}

	if (FileBrowserGetTotalFile() == 0) 
	{
		mpDebugPrint("%s:No  file.",__FUNCTION__);
		return;
	}
    Api_AudioHWEnable();
    mpDebugPrint("%s: 11  total=%d", __FUNCTION__,mem_get_free_space_total());

       NonxpgStopVideo();
	NonxpgEnterVideoPlay();
	//xpgCb_VideoPlay();
}

#endif

void EnterVideoModePlay()
{
#if MAKE_XPG_PLAYER
    mpDebugPrint("_%s_",__FUNCTION__);
	xpgSearchAndGotoPage("Mode_Video");
#if 0//VIDEO_ON
	ui_EnterVideoPage();
#endif
//	g_boNeedRepaint=true;
//	xpgUpdateStage();

	FileBrowserResetFileList();
	FileBrowserScanFileList(SEARCH_TYPE);
	xpgCb_EnterVideoMenu();

	xpgSearchAndGotoPage("Video_Viewer");
	xpgCb_EnterVideoPlayer();
	xpgCb_VideoPlay();


#else
	NonxpgVideoView();
#endif
}

#endif

