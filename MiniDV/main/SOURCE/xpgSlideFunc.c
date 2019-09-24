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
#include "ui_timer.h"
#include "slideEffect.h"

#if RTC_ENABLE
#include "../../ui/include/rtc.h"
#endif
#if NETWARE_ENABLE
#include "..\..\lwip\include\net_sys.h"
#endif

#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if MAKE_XPG_PLAYER

#ifdef SSIndexBCount
ST_SEARCH_INFO_Slideshow g_dwSlideShowIndexBuffer[SSIndexBCount];
ST_SEARCH_INFO_Slideshow *g_pstSlideShowIndexBuffer;
BYTE Exit_SlideShow_Info = 0;
#endif
volatile WORD g_wSlideShowPhotoIndex, g_wSlideShowIndex, g_wSlideShowIndexTotal;

extern ST_SEARCH_INFO *FileBrowserSearchNextImage(DWORD iOffset);
extern BOOL g_boSomeImageSelected;
extern DWORD g_dwImageRandomHistory[FAVOR_HISTORY_SIZE];
extern DWORD g_dwPicFavoriteHistory[FAVOR_HISTORY_SIZE];
#if AUDIO_ON
extern BOOL g_boSomeMusicSelected;
extern DWORD g_dwMusicRandomHistory[FAVOR_HISTORY_SIZE];
extern DWORD g_dwMusicFavoriteHistory[FAVOR_HISTORY_SIZE];
#endif
extern WORD wImageRealWidth;
extern WORD wImageRealHeight;

SWORD g_bPreSlidePhotoCheck;

BYTE g_bLRflag = 0x80;

#if NET_UPNP
extern unsigned char bexit_upupctrl;
#endif
#if NETWARE_ENABLE
extern Net_App_State App_State;
#endif
//------------------------------------------------------------------------------------------
//SlideShow
//------------------------------------------------------------------------------------------
void xpgCb_SlideSetDelay()
{
}
void xpgCb_SlideSetEffect()
{
}
void xpgCb_SlidePrev()
{
}
void xpgCb_SlideNext()
{
}

void xpgCb_SlideInfoMenuEnter()
{
	xpgCb_SlidePause();
	xpgCb_EnterInfoMenu();
}

void xpgCb_SlideInfoMenuExit()
{
	// may be should just continue play slideshow, or check music settings
#ifdef SSIndexBCount
	Exit_SlideShow_Info = 1;
#endif
	xpgCb_EnterPhotoViewWithMP3();
}

void xpgCb_SlideZoomIn()
{
}
void xpgCb_SlideZoomOut()
{
}
void xpgCb_SlidePlay()
{
}

void xpgCb_SlidePause()
{
#if NET_UPNP
	if (g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR)
	{
		// enter netfunc page
		bexit_upupctrl = 1;
		mpDebugPrint("xpgCb_SlidePause ==>");
		while(1)
		{
			TaskYield();
			if( bexit_upupctrl == 0 )
				break;
		}

		xpgSearchAndGotoPage("Net_Func");
		g_bXpgStatus = XPG_MODE_NET_FUNC;
		xpgUpdateStage();
		return;
	}
#endif
}
///
///@ingroup xpgSlideFunc
///@brief   Set Musc On Off by g_psSetupMenu->bMusicBackgroungState
///
void xpgCb_SlideSetMusicOnOff()
{
    MP_DEBUG("%s()", __FUNCTION__);
	g_psSetupMenu->bMusicBackgroungState++;
	if (g_psSetupMenu->bMusicBackgroungState >= 2)
		g_psSetupMenu->bMusicBackgroungState = 0;
	//DisplayIconAndStr(NULL, XPG_ICON_EMPTY,BackgroungMusicStrTab[g_psSetupMenu->bMusicBackgroungState],0,0,20);

	if (g_wSlideShowIndexTotal == 0)
		SystemSetErrEvent(ERR_There_is_no_file_set_for_background_music);

	//if(g_psSetupMenu->bMusicBackgroungState)
	//SystemSetErrEvent(ERR_Choose_files_for_background_music);
}
///
///@ingroup xpgSlideFunc
///@brief   Set Musc
///
void xpgCb_SlideMusicSetting()
{
    MP_DEBUG("%s()", __FUNCTION__);
	//xpgSearchAndGotoPage("Music_Setting");
	xpgCb_EnterMusicSetting();

	if (g_psSystemConfig->sFileBrowser.dwAudioTotalFile == 0)
		SystemSetErrEvent(ERR_There_is_no_music_file);
}



///
///@ingroup xpgSlideFunc
///@brief   Exit from Slide
///
void xpgCb_SlideExit()
{
    MP_DEBUG("%s()", __FUNCTION__);
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;

    TaskTimeSchedulingRegister(MAIN_TASK, 0);        // release
    g_bAniFlag &= ~ANI_SLIDE;
    RemoveTimerProc(xpgUpdateStage);
    xpgStopAllAction();
    Dma_PriorityDefault();

#if NETWARE_ENABLE
    if ((g_bXpgStatus == XPG_MODE_NET_FLICKR) ||
        (g_bXpgStatus == XPG_MODE_NET_PICASA) ||
#if YOUGOTPHOTO
        (g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
        (g_bXpgStatus == XPG_MODE_FRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
        (g_bXpgStatus == XPG_MODE_FRAMEIT) ||
#endif
#if HAVE_SNAPFISH
        (g_bXpgStatus == XPG_MODE_SNAPFISH) ||
    #endif
        (g_bXpgStatus == XPG_MODE_NET_PC_PHOTO))    // abel 20070930
    {
        xpgSearchAndGotoPage("Net_PhotoList");   //cj modify 102307

        return;
    }
#if NET_UPNP
    else if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
    {
        //xpgReleaseThumbBuffer();
        //ImageReleaseAllBuffer();
        xpgSearchAndGotoPage("Net_UpnpList");
        //xpgCb_NetPhotoViewExit();

        return;
    }
    else if (g_bXpgStatus == XPG_MODE_DLNA_1_5_DMR)
    {
        // enter netfunc page
        bexit_upupctrl = 1;
        mpDebugPrint("xpgCb_SlideExit ==>");

        while(1)
        {
            TaskYield();
            if( bexit_upupctrl == 0 )
                break;
        }

        xpgSearchAndGotoPage("Net_Func");
        g_bXpgStatus = XPG_MODE_NET_FUNC;
        xpgUpdateStage();
        return;
    }
#endif
#endif

    g_bXpgStatus = XPG_MODE_PHOTOMENU;
    psSysConfig->dwCurrentOpMode = OP_IMAGE_MODE;
    g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_PHOTOMENU;

    //MP_DEBUG3("g_bXpgStatus =%d, dwCurrentOpMode =%d, m_bPageMode = %d", g_bXpgStatus, psSysConfig->dwCurrentOpMode, g_pstXpgMovie->m_pstCurPage->m_bPageMode);
#if AUDIO_ON
    Api_AudioHWDisable();
#endif
    xpgReleaseThumbBuffer();
    ImageReleaseAllBuffer();
    xpgCb_GoModePage();
}



///
///@ingroup xpgSlideFunc
///@brief   Get slide interval by g_psSetupMenu->bSlideInterval
///
///@retval  interval
///
///@remark   unit : ms
DWORD xpgGetSlideInterval()
{
    MP_DEBUG("%s()", __FUNCTION__);
    return 5000; //about 5 sec
}
///
///@ingroup xpgSlideFunc
///@brief   Enter SlideShow
///
void xpgCb_EnterSlideShow()
{
    MP_DEBUG("%s()", __FUNCTION__);
    xpgCb_EnterSlideShowEx(NULL);
}    
void xpgCb_EnterSlideShowEx(STXPGSPRITE * pstSprite)
{
    MP_DEBUG("%s(), pstSprite = 0x%X", __FUNCTION__, pstSprite);  
      
    BYTE i = 0;
//    BYTE i = 0, slidetimecheck = 0;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    STREAM *sHandle;
    ST_SEARCH_INFO *pSearchInfo;
    ST_SETUP_MENU *psSetupMenu = g_psSetupMenu;
    int iOffset = 0;
#if NETWARE_ENABLE
	short is_net = false;								/* is photo from photo web site ? */
#endif
    BYTE bDriveCurId = 0;

    //MP_DEBUG("xpgCb_EnterPhotoViewWithMP3");

    xpgReleaseThumbBuffer(); //thumb catch use mem_malloc, release thumb buffer before slideshow
    TaskTimeSchedulingRegister(MAIN_TASK, 12);          // main task will be force yield per 12ms

#if NETWARE_ENABLE
    if ((g_bXpgStatus == XPG_MODE_NET_PC_PHOTO) ||
        (g_bXpgStatus == XPG_MODE_NET_PICASA) ||
#if YOUGOTPHOTO
        (g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
        (g_bXpgStatus == XPG_MODE_FRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
        (g_bXpgStatus == XPG_MODE_FRAMEIT) ||
#endif
#if HAVE_SNAPFISH
        (g_bXpgStatus == XPG_MODE_SNAPFISH) ||
#endif
#if NET_UPNP
        (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST) ||
#endif
#if Make_WLS
        (g_bXpgStatus == XPG_MODE_NET_WLS) ||
#endif
        (g_bXpgStatus == XPG_MODE_NET_FLICKR)
    )
        is_net = true;
        Dma_SecondaryPrioritySet(BIT9 | BIT10);     // Raise
        Dma_ThirdPrioritySet(BIT20);                // Descend CPU ???
#endif

    bDriveCurId = DriveCurIdGet(); // Test for MP652/660/661

    switch (bDriveCurId)
    {
    // OTG-0
    case USB_HOST_ID1:
    case USB_HOST_ID2:
    case USB_HOST_ID3:
    case USB_HOST_ID4:
    case USB_HOST_PTP:
    // OTG-1
    case USBOTG1_HOST_ID1:
    case USBOTG1_HOST_ID2:
    case USBOTG1_HOST_ID3:
    case USBOTG1_HOST_ID4:
    case USBOTG1_HOST_PTP:
        Dma_SecondaryPrioritySet(BIT9 | BIT10);     // Raise USB & OSD DMA priority
        break;
    }

    if (g_bAniFlag & ANI_PAUSE)
    {
        xpgResumeAudio();
        xpgDrawKeyIcon(KEY_PIC_MP3);
    }

    if (g_bAniFlag & ANI_SLIDE)
        return;

    if (g_bAniFlag & (ANI_VIDEO | ANI_READY))
        return;

    if (g_pstXpgMovie->m_pstCurPage->m_bPageMode!= XPG_MODE_SLIDESHOW) {
        xpgSearchAndGotoPage("SlideShow");
        g_pstXpgMovie->m_pstCurPage->m_bPageMode = XPG_MODE_SLIDESHOW;
    }

    MP_DEBUG("slide start up");

#if NETWARE_ENABLE
    if(is_net)
    {
        iOffset = 1;
        g_boSomeMusicSelected = false;
    }
    else
#endif
    {
        /* force to scan image files to make sure we have image files to SlideShow */
        //if (psSysConfig->dwCurrentOpMode != OP_IMAGE_MODE)
        {
            psSysConfig->dwCurrentOpMode = OP_IMAGE_MODE;
            FileBrowserResetFileList();
            MP_ALERT("%s: => FileBrowserScanFileList() for Image files ...", __FUNCTION__);
            FileBrowserScanFileList(SEARCH_TYPE);
        }

        //Reset shuffle record and check if any image/music be selected
#if FAVOR_HISTORY_SIZE
        if (xpgShuffleReset(&g_dwPicFavoriteHistory, &g_dwImageRandomHistory, g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile))
            g_boSomeImageSelected = true;
        else
            g_boSomeImageSelected = false;

#if AUDIO_ON
        if (xpgShuffleReset(&g_dwMusicFavoriteHistory, &g_dwMusicRandomHistory, g_psSystemConfig->sFileBrowser.dwAudioTotalFile))
            g_boSomeMusicSelected = true;
        else
            g_boSomeMusicSelected = false;
#endif
#endif
    }

#if FAVOR_HISTORY_SIZE
    if (g_boSomeImageSelected)//Start from first selected image
        iOffset = xpgFavorGetNextIndex(&g_dwPicFavoriteHistory, psSysConfig->sFileBrowser.dwImgAndMovCurIndex, psSysConfig->sFileBrowser.dwImgAndMovTotalFile, 1) - psSysConfig->sFileBrowser.dwImgAndMovCurIndex;
#endif

    TaskYield();

    if (FileBrowserSearchNextImage(iOffset) == NULL)
    {
        MP_ALERT("--E-- %s - No image - %d", __FUNCTION__, iOffset);

#if NETWARE_ENABLE
        if(is_net)
		{
			if( NetConnected()== TRUE  )
				xpgSearchAndGotoPage("Net_PhotoList");
		}
        else
#endif
            xpgCb_GoModePage();

        return;
    }

    TaskYield();
    g_bAniFlag |= ANI_SLIDE;
    g_bLRflag = 0x80;

    if (g_bXpgStatus != XPG_MODE_PHOTOVIEW)
    {
        if (!(g_bAniFlag & ANI_AUDIO))
        {
            if (xpgViewPhoto() == FAIL) // if (xpgViewPhotoEx(pstSprite) == FAIL)
            {
                MP_DEBUG("xpgViewPhoto fail");
                g_bAniFlag &= ~ANI_SLIDE;
#if NETWARE_ENABLE
                if(is_net)
				{
					if( NetConnected()== TRUE  )
						xpgSearchAndGotoPage("Mode_NWPhoto");
				}
                else
#endif
                {

                    if (!Polling_Event())
                        xpgCb_GoModePage();
                }

                return;
            }
        }

        mpCopyWin(Idu_GetNextWin(), Idu_GetCurrWin());
    }

    if (psSetupMenu->bSlideRptState == SETUP_MENU_OFF)
        g_wSlideShowPhotoIndex = psSysConfig->sFileBrowser.dwImgAndMovCurIndex;

    g_boNeedRepaint = false;
    if (Polling_Event()) return;

#if NETWARE_ENABLE
    if(!is_net)
#endif
#if AUDIO_ON
    if (psSetupMenu->bMusicBackgroungState == SETUP_MENU_BackgoundMUSIC_State_ON && !(g_bAniFlag & ANI_AUDIO))
    {

        Api_AudioHWEnable();

        psSysConfig->dwCurrentOpMode = OP_AUDIO_MODE;
        MP_ALERT("%s: => FileBrowserScanFileList() for Audio files ...", __FUNCTION__);
        FileBrowserScanFileList(SEARCH_TYPE);

#if FAVOR_HISTORY_SIZE
        iOffset = xpgFavorGetNextIndex(&g_dwMusicFavoriteHistory, psSysConfig->sFileBrowser.dwAudioCurIndex, psSysConfig->sFileBrowser.dwAudioTotalFile, 1) - psSysConfig->sFileBrowser.dwAudioCurIndex;
#endif
       TaskYield();

        pSearchInfo = (ST_SEARCH_INFO *) FileBrowserSearchNextAudio(0);
        if (pSearchInfo != NULL)
        {
            /* use extra DRIVE for audio playing of SlideShow, to avoid file system issue of concurrent access to same DRIVE */
            DriveHandleCopy(DriveGet(MAX_DRIVE_NUM), DriveGet(DriveCurIdGet()));
            sHandle = FileListOpen(DriveGet(MAX_DRIVE_NUM), pSearchInfo);
            MP_DEBUG1("sHandle = %x", sHandle);
            if (sHandle == NULL)
                return;

            xpgDelay(4);		//For stable issue

            g_bAniFlag |= ANI_AUDIO;
//#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
//			ScalerClockReduce();
//#endif
            if(Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, NULL, NULL, &g_sMovieWin, NULL, SLIDE_PLAY_MODE, 0))
            {
                FileClose(sHandle);
                g_bAniFlag &= ~ANI_AUDIO;
        		MP_ALERT("Api_EnterMoviePlayer return fail!, g_bAniFlag off ANI_AUDIO, g_bAniFlag=0x%X", g_bAniFlag);
            }
            else
            {
                xpgDelay(4);		//For stable issue
                Api_MoviePlay();
                MP_DEBUG("MusicPlay done");
            }
        }
    }
#endif

    if (Polling_Event()) return;
    //psSysConfig->dwCurrentOpMode = OP_IMAGE_MODE;  //To solve the paradox of FileGetCurSearchInfo()
        MP_DEBUG3("dwCurrentOpMode = %d, g_bAniFlag=%d, g_bXpgStatus =%d", psSysConfig->dwCurrentOpMode, g_bAniFlag, g_bXpgStatus);
#if NETWARE_ENABLE
    if(!is_net)
#endif
        g_bXpgStatus = XPG_MODE_SLIDESHOW;

//    if (slidetimecheck)
//    {
        if (xpgViewPhoto() == FAIL)
        {
            MP_DEBUG("xpgViewPhoto fail");
#if NETWARE_ENABLE
            if((g_bXpgStatus == XPG_MODE_NET_PC_PHOTO) ||
                (g_bXpgStatus == XPG_MODE_NET_PICASA) ||
#if YOUGOTPHOTO
                (g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
                (g_bXpgStatus == XPG_MODE_FRAMECHANNEL) ||
#endif

                (g_bXpgStatus == XPG_MODE_FRAMEIT) ||
                (g_bXpgStatus == XPG_MODE_NET_FLICKR)) //cj 102307
            {
                xpgSearchAndGotoPage("Mode_NWPhoto");
            }
            else
#endif
            {
				if( g_bXpgStatus != XPG_MODE_CARDSEL )	//wifi is unplug
					if (!Polling_Event())
						xpgCb_GoModePage();
            }
                return;
        }
        else
        {
            xpgSlideShow(); // xpgSlideShow(pstSprite);
        }
//        }
//        else
//        {
//            if (g_bAniFlag & ANI_SLIDE)
//                AddTimerProc((xpgGetSlideInterval()/5), xpgSlideShow);
//        }
}



///
///@ingroup xpgSlideFunc
///@brief   Same as xpgCb_EnterSlideShow()
///
void xpgCb_EnterPhotoViewWithMP3(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    xpgCb_EnterSlideShow();
}



//---------------------------------------------------------------------------
BYTE bEffectIdx = 14;

void xpgCb_EffectToggle(void)
{
}


#define SEPARATE_LINE_WIDTH	4   //Line width is twice than SEPARATE_LINE_WIDTH in practicing
#define SEPARATE_LINE_COLOR 0  //R,G,B are te same
///
///@ingroup xpgSlideFunc
///@brief   Slide separate
///
///@retval  PASS or FAIL
///
SWORD xpgSlideSeparate (void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    register BYTE LRFlag = g_bLRflag;
    ST_IMGWIN PartialWin;
    ST_IMGWIN *nWin;
    WORD wWidth, wHeight;
    SWORD swRet;
    BYTE bFlag;
    BYTE *pbTempBuf;
    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
    if (!(g_bAniFlag & ANI_SLIDE)) return FAIL;
    nWin = Idu_GetNextWin ();

    if ((nWin->wWidth >> 1) & 0x0001)
        wWidth = (nWin->wWidth >> 1 & 0xfffe) + 0x1;
    else
        wWidth = nWin->wWidth >> 1;
    if ((nWin->wHeight >> 1) & 0x0001)
        wHeight = (nWin->wHeight >> 1 & 0xfffe) + 0x1;
    else
        wHeight = nWin->wHeight >> 1;

    if (ImageDraw_Decode(nWin, 1) == FAIL) return FAIL;
    if (!(g_bAniFlag & ANI_SLIDE)) return FAIL;
    ImageCalculateRealRatio(nWin, pstPanel->wWidthInLen, pstPanel->wHeightInLen);
    bFlag = (wImageRealHeight > wImageRealWidth) ? 1 : 0;

    if (bFlag)
    {
    	pbTempBuf = (BYTE *)ext_mem_malloc((wHeight << 1) * wWidth * 2);
        ImgWinInit (&PartialWin, pbTempBuf, wHeight << 1, wWidth);
        ImageCalculateRealRatio(&PartialWin, pstPanel->wWidthInLen, pstPanel->wHeightInLen << 1);
    }
    else
    {
    	pbTempBuf = (BYTE *)ext_mem_malloc(wHeight * wWidth * 2);
        ImgWinInit (&PartialWin, pbTempBuf, wHeight, wWidth);
        ImageCalculateRealRatio(&PartialWin, pstPanel->wWidthInLen, pstPanel->wHeightInLen);
    }

	if (pbTempBuf == NULL) {
		MP_DEBUG("-E- slide alloc temp buffer FAIL");
		return FAIL;
	}
    if (!(g_bAniFlag & ANI_SLIDE)) return FAIL;
    ImageScaleFromJPEGTarget(&PartialWin, ImageGetTargetBuffer());

    if (LRFlag & 0x80)
    {
        ImgSetPoint (nWin, 0, 0);
        ImgPaintArea (nWin, nWin->wWidth, nWin->wHeight, RGB2YUV (0, 0, 0));
        LRFlag &= 0x7f;
    }
    else
    {
        Slide_WinCopy (Idu_GetCurrWin (), nWin);
    }
    if (!(g_bAniFlag & ANI_SLIDE)) return FAIL;
    if (bFlag)
    {
        if ((LRFlag & 0x3) == 0 || (LRFlag & 0xc) == 0xc)
        {

            WinCopy (nWin, 0, 0, nWin->wWidth >> 1, nWin->wHeight, PartialWin.pdwStart,
                     PartialWin.wWidth);
            LRFlag |= 0x13;
        }
        else if ((LRFlag & 0xc) == 0 || (LRFlag & 0x3) == 0x3)
        {
            WinCopy (nWin, 0, nWin->wWidth >> 2, nWin->wWidth >> 1, nWin->wHeight,
                     PartialWin.pdwStart, PartialWin.wWidth);
            LRFlag |= 0x2c;
        }
    }
    else
    {
        if ((LRFlag & 0x1) == 0)
        {
            WinCopy (nWin, 0, 0, nWin->wWidth >> 1, nWin->wHeight >> 1, PartialWin.pdwStart,
                     PartialWin.wWidth);
            LRFlag |= 0x1;
            if (LRFlag & 0x10)
            {
                ImgSetPoint (nWin, 0, nWin->wHeight >> 1);
                ImgPaintArea (nWin, nWin->wWidth >> 1, nWin->wHeight >> 1, RGB2YUV (0, 0, 0));
                LRFlag &= 0xed;
            }
        }
        else if ((LRFlag & 0x2) == 0)
        {
            WinCopy (nWin, nWin->wHeight >> 1, 0, nWin->wWidth >> 1, nWin->wHeight >> 1,
                     PartialWin.pdwStart, PartialWin.wWidth);
            LRFlag |= 0x2;
            if (LRFlag & 0x10)
            {
                ImgSetPoint (nWin, 0, 0);
                ImgPaintArea (nWin, nWin->wWidth >> 1, nWin->wHeight >> 1, RGB2YUV (0, 0, 0));
                LRFlag &= 0xee;
            }
        }
        else if ((LRFlag & 0x4) == 0)
        {
            WinCopy (nWin, 0, nWin->wWidth >> 2, nWin->wWidth >> 1, nWin->wHeight >> 1,
                     PartialWin.pdwStart, PartialWin.wWidth);
            LRFlag |= 0x4;
            if (LRFlag & 0x20)
            {
                ImgSetPoint (nWin, nWin->wWidth >> 1, nWin->wHeight >> 1);
                ImgPaintArea (nWin, nWin->wWidth >> 1, nWin->wHeight >> 1, RGB2YUV (0, 0, 0));
                LRFlag &= 0xd7;
            }
        }
        else if ((LRFlag & 0x8) == 0)
        {
            WinCopy (nWin, nWin->wHeight >> 1, nWin->wWidth >> 2, nWin->wWidth >> 1,
                     nWin->wHeight >> 1, PartialWin.pdwStart, PartialWin.wWidth);
            LRFlag |= 0x8;
            if (LRFlag & 0x20)
            {
                ImgSetPoint (nWin, nWin->wWidth >> 1, 0);
                ImgPaintArea (nWin, nWin->wWidth >> 1, nWin->wHeight >> 1, RGB2YUV (0, 0, 0));
                LRFlag &= 0xdc;
            }
        }
    }

    ext_mem_free(pbTempBuf);
    if (!(g_bAniFlag & ANI_SLIDE)) return FAIL;
    ImgSetPoint (nWin, ((nWin->wWidth >> 1) - SEPARATE_LINE_WIDTH), 0);
    ImgPaintArea (nWin, SEPARATE_LINE_WIDTH << 1, nWin->wHeight, RGB2YUV (SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR));
    if (!(LRFlag & 0x10))
    {
        ImgSetPoint (nWin, 0, ((nWin->wHeight >> 1) - SEPARATE_LINE_WIDTH));
        ImgPaintArea (nWin, nWin->wWidth >> 1, SEPARATE_LINE_WIDTH << 1, RGB2YUV (SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR));
    }
    if (!(LRFlag & 0x20))
    {
        ImgSetPoint (nWin, nWin->wWidth >> 1, ((nWin->wHeight >> 1) - SEPARATE_LINE_WIDTH));
        ImgPaintArea (nWin, nWin->wWidth >> 1, SEPARATE_LINE_WIDTH << 1, RGB2YUV (SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR, SEPARATE_LINE_COLOR));
    }

    if ((LRFlag & 0xf) == 0xf)
        LRFlag &= 0xf0;
    g_bLRflag = LRFlag;
    ImgSetPoint (nWin, 0, 0);
    return PASS;
}

void GetNextPictureIndex(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    ST_FILE_BROWSER *psFileBrowser = &g_psSystemConfig->sFileBrowser;
    TaskYield();
    {
        if (psFileBrowser->dwImgAndMovCurIndex >= psFileBrowser->dwImgAndMovTotalFile - 1)
            psFileBrowser->dwImgAndMovCurIndex = 0;
        else
            psFileBrowser->dwImgAndMovCurIndex += 1;
    }
    TaskYield();
}
///
///@ingroup xpgSlideFunc
///@brief   Preload next slide
///
///@retval  pass or fail
///
SWORD xpgNextSlidePreload()
{
    MP_DEBUG("%s()", __FUNCTION__);
    SWORD swRet;
    ST_FILE_BROWSER *psFileBrowser = &g_psSystemConfig->sFileBrowser;
#if NETWARE_ENABLE
		if((g_bXpgStatus == XPG_MODE_NET_PC_PHOTO) ||
			  (g_bXpgStatus == XPG_MODE_NET_PICASA) ||
#if YOUGOTPHOTO
			  (g_bXpgStatus == XPG_MODE_NET_YOUGOTPHOTO) ||
#endif
#if HAVE_FRAMECHANNEL
			(g_bXpgStatus == XPG_MODE_FRAMECHANNEL) ||
#endif
#if HAVE_FRAMEIT
			(g_bXpgStatus == XPG_MODE_FRAMEIT) ||
#endif
#if HAVE_SNAPFISH
			(g_bXpgStatus == XPG_MODE_SNAPFISH) ||
#endif
			(g_bXpgStatus == XPG_MODE_NET_FLICKR)) //cj 102307
		{
				NetSetFileIndex(NetAddCurIndex(1));
		}
#if NET_UPNP
		else if (g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
		{
			Net_Upnp_SearchNextEntry(1);
		}
#endif
		else
#endif
    {
#if (SEARCH_TYPE == LOCAL_SEARCH_INCLUDE_FOLDER)
        ST_SEARCH_INFO *pSearchInfo;
        do
        {
    	    GetNextPictureIndex();
            pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(psFileBrowser->dwImgAndMovCurIndex);
            //mpDebugPrint("pSearchInfo->bFileType = %d(5-FOLDER)", pSearchInfo->bFileType);
        }while (pSearchInfo->bFileType == FILE_OP_TYPE_FOLDER);
#else
        GetNextPictureIndex();
#endif
    }
        if (g_psSetupMenu->bSlideSeparate == SETUP_MENU_SLIDESEPARATE_ON)   //Dual screen
            swRet = xpgSlideSeparate ();
        else
            swRet = ImageDraw(Idu_GetNextWin(), 1); //draw the next index photo in next win

//DumpYuvImage2Bmp(Idu_GetNextWin());

    return swRet; // swOffSetTime;
}
///
///@ingroup xpgSlideFunc
///@brief   Change to next slide
///
///@retval  PASS or FAIL
///
SWORD xpgNextSlideChg(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
    SWORD swRet = PASS;
    BYTE *buffer;
	ST_IMGWIN *pWin = Idu_GetNextWin ();

    //ImageReleaseAllBuffer(); //slide show effect memory free error

    if (g_psSetupMenu->bSlideTransition == SETUP_MENU_TRANSITION_OFF) //Jasmine 5/24
        Idu_ChgWin (pWin);
    else
    {
        if (g_psSetupMenu->bSlideTransition >= SETUP_MENU_TRANSITION_RANDOM)
           bEffectIdx = SETUP_MENU_TRANSITION_RANDOM;//bEffectIdx % (SETUP_MENU_TRANSITION_RANDOM - 1) + 1;
        else
           bEffectIdx = g_psSetupMenu->bSlideTransition;

        if (ext_mem_get_free_space() < (pWin->dwOffset * pWin->wHeight + 4096))
        {
            mpDebugPrint("(ext_mem_get_free_space() <(pWin->dwOffset * pWin->wHeight + 4096))");
            return FAIL;
        }
        else
        {
            buffer = (BYTE *)ext_mem_malloc(pWin->dwOffset * pWin->wHeight + 4096);

            MP_TRACE_LINE();
            if (buffer) {
                swRet = Img_SlideShow (pWin, (BYTE *)buffer, bEffectIdx);
                MP_DEBUG("xpgNextSlideChg ext_mem_free %08x", (DWORD)buffer);
                if (buffer) ext_mem_free(buffer);
            }
            else
            {
                MP_ALERT("alloc fail");
                return FAIL;
            }
        }
    }
    ImageReleaseAllBuffer(); //slide show effect memory free error

    return swRet;
}


///
///@ingroup xpgSlideFunc
///@brief   SlideShow main routine
///
void xpgSlideShow (void)
{

    MP_DEBUG("%s()", __FUNCTION__);
    static DWORD swPreloadTime = 0, swStartTime = 0, swElapsedTime = 0, i, j;

    swStartTime = GetSysTime();
    TaskSleep(1);

    if (!(g_bAniFlag & ANI_SLIDE)) return;
    if (Polling_Event()) return;

    if (g_bPreSlidePhotoCheck == FAIL)
            swStartTime = 0;

    g_bPreSlidePhotoCheck = xpgNextSlidePreload();

    if (!(g_bAniFlag & ANI_SLIDE)) return;

    if (g_bPreSlidePhotoCheck == PASS)
    {
        swPreloadTime = SystemGetElapsedTime(swStartTime);

        if (!(g_bAniFlag & ANI_SLIDE)) return;
            MP_DEBUG1("swPreloadTime = %d", swPreloadTime);

        if (xpgGetSlideInterval() > swPreloadTime)
            swElapsedTime = (xpgGetSlideInterval() - swPreloadTime);
        else
            swElapsedTime = 0;

        MP_ALERT("swElapsedTime = %d", swElapsedTime);

        if (swElapsedTime > 0)
        {
            DWORD currentTime = GetSysTime();

            while (SystemGetElapsedTime(currentTime) < swElapsedTime)
            {
                if (!(g_bAniFlag & ANI_SLIDE)) return;
                if (Polling_Event()) return;
                TaskSleep(1);
            }
        }

        // Erease Mute Icon
        if (RemoveTimerProc(xpgEraseIcon) == PASS)
            xpgEraseIcon();

        if (!(g_bAniFlag & ANI_SLIDE)) return;
        g_bPreSlidePhotoCheck = xpgNextSlideChg();

        if (!(g_bAniFlag & ANI_SLIDE)) return;

        if (g_bPreSlidePhotoCheck != PASS)
            MP_ALERT("xpgNextSlideChg fail!!");

        //g_bPreSlidePhotoCheck = PASS;
        if (Polling_Event()) return;
    }
    else
    {
        MP_ALERT("xpgNextSlidePreload fail!!");
        ImageReleaseAllBuffer();
        #if RTC_ENABLE
        RemoveTimerProc(UpdateClock);
        UpdateClock();
        #endif
    }

    if (Polling_Event()) return;

    if (!(g_bAniFlag & ANI_SLIDE))
        return;

    if (!SystemCardPlugInCheck(DriveCurIdGet()))   //OK: Card present status match!
    {
        MP_ALERT("Mcard not present()! SlideShow stopped.");
        return;
    }
#if NETWARE_ENABLE
	if(isNetFunc_Mode())
	{
		while(!(App_State.dwState & NET_CONFIGED))
		{
		    if (!(g_bAniFlag & ANI_SLIDE))
				break;
			TaskYield();
		}
	  	AddTimerProc(10, xpgSlideShow);

	}else
#endif	
    AddTimerProc(3, xpgSlideShow);

}


//Move to slideshow page and do slideshow if could be

///
///@ingroup xpgSlideFunc
///@brief   Do SlideShow
///
///@retval  0 or 1 - if slideshow success
///
BYTE xpgSlideShowAnywhere()
{

    MP_DEBUG("%s()", __FUNCTION__);
	ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
	BYTE bCurrUsingCard = psSysConfig->sStorage.dwCurStorageId;


	xpgStopAllAction();

	if (bCurrUsingCard != NULL_DRIVE)
	{
		if (psSysConfig->dwCurrentOpMode != OP_IMAGE_MODE)
		{
#if AUTO_DEMO
			g_dwOldOpMode = psSysConfig->dwCurrentOpMode;
#endif
			psSysConfig->dwCurrentOpMode = OP_IMAGE_MODE;
			FileBrowserScanFileList(SEARCH_TYPE);
		}
		if (psSysConfig->sFileBrowser.dwImgAndMovTotalFile != 0)
		{
			xpgSearchAndGotoPage("Mode_Photo");

			xpgChangeMenuMode(OP_IMAGE_MODE, 1);
			g_pstMenuPage = g_pstXpgMovie->m_pstCurPage;
			g_bXpgStatus = XPG_MODE_PHOTOMENU;
			xpgSearchAndGotoPage("SlideShow");
			//RefreshOSD();
			Idu_OsdErase();
			xpgCb_EnterPhotoViewWithMP3();
			return 1;
		}
		else
		{
#if AUTO_DEMO
			psSysConfig->dwCurrentOpMode = g_dwOldOpMode;
#endif
			if (psSysConfig->dwCurrentOpMode == OP_IMAGE_MODE
				|| psSysConfig->dwCurrentOpMode == OP_AUDIO_MODE
				|| psSysConfig->dwCurrentOpMode == OP_MOVIE_MODE
				|| psSysConfig->dwCurrentOpMode == OP_FILE_MODE)
				FileBrowserScanFileList(SEARCH_TYPE);
		}
	}
	return 0;
    
}


//-------------------------------------------------------------------------
///
///@ingroup xpgSlideFunc
///@brief   Clean back ground music
///
void Clean_BackgroundMusic(void)
{
    MP_DEBUG("%s()", __FUNCTION__);
#ifdef SSIndexBCount
	BYTE i, j;

	g_pstSlideShowIndexBuffer =
		(ST_SEARCH_INFO_Slideshow *) ((DWORD) & g_dwSlideShowIndexBuffer | 0xa0000000);
	for (i = 0; i < SSIndexBCount; i++)
	{
		g_pstSlideShowIndexBuffer[i].dwDirStart = 0;
		g_pstSlideShowIndexBuffer[i].dwDirPoint = 0;
		g_pstSlideShowIndexBuffer[i].dwFdbOffset = 0;
		for (j = 0; j < 4; j++)
			g_pstSlideShowIndexBuffer[i].bExt[j] = 0;
	}
	g_wSlideShowIndexTotal = 0;
#endif
}

void xpgSlideShowEffectByIndex(BYTE bEffectIdx) // Push - bEffectId = 10
{
    //mpDebugPrint("%s(), bEffectIdx = %d", __FUNCTION__, bEffectIdx);

    if( !(bEffectIdx == 5 || bEffectIdx == 10)) // 5=Scroll, 10=Push
        return;

    /* force to scan image files to make sure we have image files to SlideShow */
    if (g_psSystemConfig->dwCurrentOpMode != OP_IMAGE_MODE)
    {
        g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
        FileBrowserResetFileList();
        MP_ALERT("%s: => FileBrowserScanFileList() for Image files ...", __FUNCTION__);
        FileBrowserScanFileList(SEARCH_TYPE);
    }

    GetNextPictureIndex();
    SWORD swRet = ImageDraw(Idu_GetNextWin(), 1); //draw the next index photo in next win
    g_bPreSlidePhotoCheck = swRet; //xpgNextSlidePreload();

    if (g_bPreSlidePhotoCheck != PASS)
    {
        MP_ALERT("xpgNextSlidePreload fail!!");
        return;
    }

    // g_bPreSlidePhotoCheck == PASS
    //g_bPreSlidePhotoCheck = xpgNextSlideChg();

    ST_IMGWIN *pWin = Idu_GetNextWin ();

    BYTE *buffer = (BYTE *)ext_mem_malloc(pWin->dwOffset * pWin->wHeight + 4096);
    if (buffer)
    {
        //mpDebugPrint("bEffectIdx = %d", bEffectIdx);
        SWORD swRet = Img_SlideShow (pWin, (BYTE *)buffer, bEffectIdx);
        MP_DEBUG("xpgNextSlideChg ext_mem_free %08x", (DWORD)buffer);
        if (buffer) ext_mem_free(buffer);
    }
    else
    {
        MP_ALERT("alloc fail");
        return FAIL;
    }

    g_bPreSlidePhotoCheck = swRet;
    if (g_bPreSlidePhotoCheck != PASS)
        MP_ALERT("xpgNextSlideChg fail!!");

    //mpDebugPrint("%s() ---> exit", __FUNCTION__);
}

BYTE bGoToPageSlideEffectIndex = 0;
void xpgGoToPageSlideEffectByIndex()
{
    //mpDebugPrint("%s()", __FUNCTION__);

    if(bGoToPageSlideEffectIndex == 0)
//    if(bGoToPageSlideEffectIndex == 1) // 1=Fade
//        bGoToPageSlideEffectIndex = 2;
//    else if(bGoToPageSlideEffectIndex == 2) // 2=Expansion
//        bGoToPageSlideEffectIndex = 3;
//    else if(bGoToPageSlideEffectIndex == 3) // 3=Sketch
    //    bGoToPageSlideEffectIndex = 4;
    //else if(bGoToPageSlideEffectIndex == 4) // 4=Grid - messy
        bGoToPageSlideEffectIndex = 5;
    else if(bGoToPageSlideEffectIndex == 5) // 5=Scroll
    //    bGoToPageSlideEffectIndex = 6;
    //else if(bGoToPageSlideEffectIndex == 6) // 6=KenBurns - T0525 hang
    //    bGoToPageSlideEffectIndex = 7;
    //else if(bGoToPageSlideEffectIndex == 7) // 7=3D Cube - messy
    //    bGoToPageSlideEffectIndex = 8;
    //else if(bGoToPageSlideEffectIndex == 8) // 8=3D Flip - messy
    //        bGoToPageSlideEffectIndex = 9;
    //else if(bGoToPageSlideEffectIndex == 9) // 9=3D Swap- messy
        bGoToPageSlideEffectIndex = 10;
    else
        bGoToPageSlideEffectIndex = 5;

    SWORD swRet;
    ST_IMGWIN *pWin = Idu_GetNextWin ();

    BYTE *buffer = (BYTE *)ext_mem_malloc(pWin->dwOffset * pWin->wHeight + 4096);
    if (!buffer)
    {
        MP_ALERT("alloc fail");
        return FAIL;
    }

    //mpDebugPrint("bEffectIdx = %d", bEffectIdx);
    swRet = Img_SlideShow (pWin, (BYTE *)buffer, bGoToPageSlideEffectIndex);
    MP_DEBUG("xpgNextSlideChg ext_mem_free %08x", (DWORD)buffer);
    if (buffer) ext_mem_free(buffer);

    if (swRet != PASS)
        MP_ALERT("xpgNextSlideChg fail!!");

    //mpDebugPrint("%s() ---> exit", __FUNCTION__);
}

#endif

