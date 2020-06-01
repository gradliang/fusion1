/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0
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
#include "filebrowser.h"
#include "imageplayer.h"
#include "mpapi.h"
//#include "usbotg.h"
#include "Setup.h"
#include "filebrowser.h"
#include "Icon.h"
#include "ui_timer.h"
#if RTC_ENABLE
#include "../../ui/include/rtc.h"
#endif
#if (BT_XPG_UI == ENABLE)
#include "xpgBtfunc.h"
#endif
#include "ML_StringDefine.h"

//#include "xpgCamera.h"
#include "xpgCamFunc.h"
#include "xpgString.h"
#include "charset.h"
#include "peripheral.h"
#include "xpgDrawSprite.h"
#include "xpgProcSensorData.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

DWORD g_dwCurIndex = 0,g_dwModeIconStatus=0; //high word is page index  g_dwModeIconStatus:1bit对应一个ICON
#if (SENSOR_ENABLE == ENABLE)
//int CameraInit = 1;
//E_MINI_DV_MODE g_MinidvMode;
#endif

#if MAKE_XPG_PLAYER
extern DWORD g_dwTotalSeconds;
extern BYTE g_bXpgStatus;
extern BYTE g_boNeedRepaint;
extern BOOL bSetUpChg;
extern BYTE xpgStringBuffer[256];
extern STXPGBROWSER *g_pstBrowser;
//--------------------
#if ERROR_HANDLER_ENABLE
BOOL g_boShowDialog = false;
#endif
#if AUTO_DEMO
DWORD g_dwOldOpMode;
#endif
#if XPG_KEY_ICON_ENABLE
//DWORD *g_pDrawIconBuffer = NULL;
ST_IMGWIN g_stIconWin;
ST_IMGWIN g_stVolWin;
#endif
#if  (PRODUCT_UI==UI_SURFACE)
DWORD g_dwPassNum = 0,g_dwFailNum=0;
#endif
BYTE g_bPowerOnCheckPassword=2;

//---------------------------------------------------------------------------
// For xpg call main functions
//---------------------------------------------------------------------------
#define NULL_FUNC xpgCb_NullFunc

#pragma alignvar(4)
STACTFUNC actionFunctions[] = {
	{0, 0, 0, 0, xpgCb_NullFunc},

	{0, 0, 0, 0, xpgCb_PressUpKey},             // act0
	{0, 0, 0, 0, xpgCb_PressDownKey},           // act1
	{0, 0, 0, 0, xpgCb_PressLeftKey},               // act2
	{0, 0, 0, 0, xpgCb_PressRightKey},               // act3
	{0, 0, 0, 0, xpgCb_PressEnterKey},          // act4
	{0, 0, 0, 0, xpgCb_PressExitKey},           // act5
	{0, 0, 0, 0, NULL_FUNC},               // 

};


#if VOLUME_DEGREE
//------------------------------------------------------------------------------
///
///@ingroup xpgFuncEntry
///@brief   Turn up Voloume and show VolumeBar
///
void xpgCb_VolumnUp(void)
{
	MP_DEBUG("xpgCb_VolumnUp");
	g_bVolumeIndex &= ~0x80;    // Clean mute indicator

	if ((g_bVolumeIndex & 0x3f) < VOLUME_DEGREE)
		g_bVolumeIndex++;
#if AUDIO_ON
	MX6xx_AudioSetVolume(g_bVolumeIndex);
#endif
	bSetUpChg = 1;

	//In hign resolution mode, OSD volumeBar is unstable in Video player mode. Disable it!
	//	if((g_bXpgStatus != XPG_MODE_VIDEOPLAYER) && (Idu_GetCurrWin()->wWidth <= 1024))

	g_boNeedRepaint = false;
}

//------------------------------------------------------------------------------
///
///@ingroup xpgFuncEntry
///@brief   Turn down Volume and show VoloumeBar
///
void xpgCb_VolumnDown(void)
{
	MP_DEBUG("xpgCb_VolumnDown");

	if ((g_bVolumeIndex & 0x3f) > 0)
		g_bVolumeIndex--;
#if AUDIO_ON
	MX6xx_AudioSetVolume(g_bVolumeIndex);
#endif
	bSetUpChg = 1;

	//In hign resolution mode, OSD volumeBar is unstable in Video player mode. Disable it!
	//	if((g_bXpgStatus != XPG_MODE_VIDEOPLAYER) && (Idu_GetCurrWin()->wWidth <= 1024))

	g_boNeedRepaint = false;
}

//------------------------------------------------------------------------------
///
///@ingroup xpgFuncEntry
///@brief   Set volume mute
///
void xpgCb_VolumeMute()
{
	MP_DEBUG("xpgCb_VolumeMute");
#if AUDIO_ON
	BOOL g_bMuteFlag;

	g_bMuteFlag = (g_bVolumeIndex & 0x80);
#if AUDIO_ON
	MX6xx_SetMute();
#endif
	g_boNeedRepaint = false;

	AddTimerProc(5, xpgEraseIcon);
#endif
}
#endif
//------------------------------------------------------------------------------
///
///@ingroup xpgFuncEntry
///
///@brief   Update elasped Time Bar of Music bu Idu paint area, callback by kernel
///
///@param   dwTotal - Total value of one .mp3
///
///@param   wValue - Current progressing value of .mp3
///
void xpgUpdateTimeBar(register DWORD dwTotal, register DWORD wValue)
{
#ifdef SPRITE_TYPE_MUSICINFO
    WORD x, y;
    DWORD dwPage = g_pstXpgMovie->m_wCurPage;

#ifdef XPG_PAGE_Mode_Music
    if (dwPage != XPG_PAGE_Mode_Music)
        return PASS;
#endif
    BYTE buffer[64];
    ST_IMGWIN * pWin;
    STXPGMOVIE *pstMovie = &g_stXpgMovie;
    STXPGSPRITE *pstSprite = xpgSpriteFindType(pstMovie, SPRITE_TYPE_MUSICINFO, 5);
    if (pstSprite == NULL)
        return;

    if (langid == LANGUAGE_ENGLISH)
        x = pstSprite->m_wPx + 120;
    else
        x = pstSprite->m_wPx + 74;
    y = pstSprite->m_wPy;
    pWin = Idu_GetCurrWin();

    Idu_PaintWinArea(pWin, x-4, y-2, 100, 36, RGB2YUV(0, 0, 0));

    sprintf(buffer, "%02d : %02d", wValue/60, wValue%60);
    Idu_PrintString(pWin, buffer, x, y, 0, 0);
#endif
}

#if DISPLAY_VIDEO_SUBTITLE
void xpgUpdateOsdSRT(BYTE * pSrt)
{
#if 0
#if (___PLATFORM___ == 0x663) // || ___PLATFORM___ == 0x660)
    WORD x=10, y=210, w=320, h=240 // MP663 - 320x240
#elif (___PLATFORM___ == 0x660 || ___PLATFORM___ == 0x652)
    WORD x=40, y=500, w=1024, h=600; // MP652, MP660 - 1024x600
#else // (___PLATFORM___ == 0x660 || ___PLATFORM___ == 0x652)
    WORD x=40, y=420, w=800, h=480; // MP652, MP660 - 800x480
#endif
    Idu_OsdPaintArea(0, y, w, h-y, 0); // 0=erase
    if (pSrt == NULL)
    {
        MP_ALERT("pSrt == NULL");
        return;
    }
    WORD *pwSrt;
    MP_DEBUG("0x%X 0x%X", *pSrt, *(pSrt+1));
    if(*pSrt == 0xFF && *(pSrt+1) == 0xFE)
    {
        pwSrt= (WORD *) pSrt;
	    xpgOSDUniStringOut(pwSrt+1, x, y, 13, w-x);
	}
	else
	    Idu_OsdPutStrWithSize(Idu_GetOsdWin(), pSrt, x, y, OSD_COLOR_WHITE, 0, 0, DEFAULT_FONT_SIZE);
#endif
}
#endif

void xpgUpdateOsdTimeBar(register DWORD dwTotal, register DWORD wValue)
{
    if(wValue > dwTotal)
    {
        MP_ALERT("wValue(%d) > dwTotal(%d), skip!", wValue, dwTotal);
        return;
    }
#if SHOW_VIDEO_STATUS
    ShowVideoPalyTime(dwTotal-wValue);
#endif

}


///
///@ingroup xpgFuncEntry
///@brief   Init Time Bar of Video by OSD, callback by kernel
///
void xpgInitOsdTimeBar(void)
{
}



///
///@ingroup xpgFuncEntry
///@brief   Update Music Equlizer Info by Idu
///
void xpgUpdateEQInfo(register DWORD dwTotal, register DWORD dwSec)
{
#if 0
	MP_DEBUG("%s() dwSec = %d", __FUNCTION__, dwSec);

#if LYRIC_ENABLE
	return;
#endif

    WORD i; unsigned int eq[16];

    //if (g_pstXpgMovie->m_pstCurPage->m_bPageMode == XPG_MODE_MUSICMENU)
    if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO || g_bXpgStatus== XPG_MODE_MUSICMENU)
    {
        for(i=0; i<16; i++) eq[i] = *(AUDIO_EQ_INFO + i);
        if(AUDIO_EQ_INFO==0) return; // skip init not ready data

        ST_IMGWIN *ImgWin = Idu_GetCurrWin();
        STXPGMOVIE *pstMov = &g_stXpgMovie;
        //ImgWin->wX = 80; ImgWin->wY = 150;
        if (g_bXpgStatus== XPG_MODE_MUSICMENU)
            ImgWin->wX = pstMov->m_wScreenWidth/10;
        if (g_bXpgStatus== XPG_MODE_NET_PC_AUDIO)
        {
            STXPGMOVIE *pstMovie = &g_stXpgMovie;
            STXPGSPRITE *pstSprite = xpgSpriteFindType(pstMovie, SPRITE_TYPE_MUSICTIME, 0);
            if(pstSprite == NULL)
                ImgWin->wX = 320;
            else
                ImgWin->wX = pstSprite->m_wPx;
        }
        ImgWin->wY = pstMov->m_wScreenHeight/3;
        //unsigned int w = 8, sh = 128, h;
        unsigned int w = pstMov->m_wScreenWidth/100;
        unsigned int sh = pstMov->m_wScreenHeight/15*4;
        unsigned int h;

        for(i=0; i<16; i++)
        {
            h=(eq[i] & 0x7f); //*sh/128; // 0x7f=128

            if(sh > h)
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY, w, sh-h, RGB2YUV(0, 0, 0)); // Top Black

            // Display histogram by 4 levels of color : Red/Yellow/Green/Blue
            if(h<32 && h>0 && ImgWin->wY+sh > h)
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-h, w, h,      RGB2YUV(0, 0, 255)); // Blue
            else if(h<64 && h>=32 && ImgWin->wY+sh > h)
            {
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-32,w, 32,     RGB2YUV(0, 0, 255)); // Blue
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-h, w, (h-32), RGB2YUV(0, 255, 0)); // Green
            }
            else if(h<96 && h>=64 && ImgWin->wY+sh > h)
            {
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-32,w, 32,     RGB2YUV(0, 0, 255)); // Blue
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-64,w, 32,     RGB2YUV(0, 255, 0)); // Green
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-h, w, (h-64), RGB2YUV(255, 255, 0)); // Yelloe
            }
            else if(h<128 && h>=96 && ImgWin->wY+sh > h)
            {
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-32,w, 32,     RGB2YUV(0, 0, 255)); // Blue
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-64,w, 32,     RGB2YUV(0, 255, 0)); // Green
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-96,w, 32,     RGB2YUV(255, 255, 0)); // Yelloe
                Idu_PaintWinArea(ImgWin, ImgWin->wX+i*(w+6), ImgWin->wY+sh-h, w, (h-96), RGB2YUV(255, 0, 0)); // Red
            }

            /* note: Do not occupy CPU too long, because audio is more critical for human beings.
             * So, yield CPU here to let other tasks run (ex: BT internal task and Audio task when BT A2DP audio streaming).
             * (Test OK:  TaskYield() 秆∕ BT A2DP play music 箉 issue !! )
             */
            TaskYield();

        } // for
    }
#endif
}



#if (DISPLAY_VIDEO_DROP_INFO_FULL || DISPLAY_VIDEO_DROP_INFO_THUMB)
void xpgDispVideoDropInfo(S_VIDEO_DROP_INFO_T pVideoDropInfo)
{
    //mpDebugPrint("xpgDispVideoDropInfo()");
    Idu_OsdErase();
    WORD x=100, y=50, w=250, h=200;
    Idu_PaintWinArea(Idu_GetCurrWin(), x, y, w, h, RGB2YUV(0, 0, 0));

    Idu_OsdPutStr(Idu_GetOsdWin (), pVideoDropInfo.cVidoeCodec, x, y, OSD_COLOR_WHITE);
    Idu_OsdPutStr(Idu_GetOsdWin (), pVideoDropInfo.cAudioCodec, x, y+30, OSD_COLOR_WHITE);

    BYTE bPrintBuf[10];
    mp_sprintf(bPrintBuf, "%d", pVideoDropInfo.iTotalFrames);
    Idu_OsdPutStr(Idu_GetOsdWin (), "iTotalFrames = ", x, y+60, OSD_COLOR_WHITE);
    Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x+200, y+60, OSD_COLOR_WHITE);

    mp_sprintf(bPrintBuf, "%d", pVideoDropInfo.iMissedFrame);
    Idu_OsdPutStr(Idu_GetOsdWin (), "iMissedFrame = ", x, y+90, OSD_COLOR_WHITE);
    Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x+200, y+90, OSD_COLOR_WHITE);

    mp_sprintf(bPrintBuf, "%d", pVideoDropInfo.iPlayedFrame);
    Idu_OsdPutStr(Idu_GetOsdWin (), "iPlayedFrame = ", x, y+120, OSD_COLOR_WHITE);
    Idu_OsdPutStr(Idu_GetOsdWin (), bPrintBuf, x+200, y+120, OSD_COLOR_WHITE);

}
#endif

//;------------------------------------------------
void xpgOsdPopupError(int ErrCode)
{
#if 0
    MP_DEBUG("xpgOsdPopupError(), ErrCode=0x%X", ErrCode);

	BYTE *cMsg = (BYTE *) GetSystemErrMsg(ErrCode);
	//BYTE cMsg[] = "Not Supported";   // for temp message
	ST_IMGWIN *pWin;

	pWin = Idu_GetCurrWin();

	DWORD x, y, w, h;
	w = pWin->wWidth > 800 ? 800 : 600; //Idu_GetStringWidth(cMsg, 0) + 160;
	h = pWin->wHeight > 480 ? 480 : 320;
	x = (pWin->wWidth - w) >> 1;
	y = (pWin->wHeight - h) >> 1;
	MP_DEBUG("x=%d, y=%d, w=%d, h=%d", x, y, w, h);
	Idu_OsdPaintArea(x, y, w, h, 6); // 0=erase

    DWORD x1, y1, w1, h1;
	x1 = (pWin->wWidth - Idu_GetStringWidth(cMsg, 0)) >> 1;
    y1 = (pWin->wHeight - Idu_GetStringHeight(cMsg, 0)) >> 1;
    w1 = Idu_GetStringWidth(cMsg, 0);
    h1 = Idu_GetStringHeight(cMsg, 0);
    MP_DEBUG("x1=%d, y1=%d, w1=%d, h1=%d", x1, y1, w1, h1);
	Idu_OsdPutStr(Idu_GetOsdWin (), cMsg, x1, y1, OSD_COLOR_WHITE);

	SystemClearErrMsg();

	WaitAnyKeyContinue(1000);
	Idu_OsdPaintArea(x, y, w, h, RGB2YUV(0, 0, 0)); // 0=erase
#endif
}



XPG_FUNC_PTR xpgUpdateFuncPtr;
///
///@ingroup xpgFuncEntry
///@brief   Init Update Func pointer for kernel
///
void xpgUpdateFuncPtrInit(void)
{
  xpgUpdateFuncPtr.xpgInitOsdTimeBar   = xpgInitOsdTimeBar;
	//xpgUpdateFuncPtr.xpgPopupError       = xpgPopupError;
	xpgUpdateFuncPtr.xpgOsdPopupError    = xpgOsdPopupError;
	xpgUpdateFuncPtr.xpgUpdateEQInfo     = xpgUpdateEQInfo;
	xpgUpdateFuncPtr.xpgUpdateOsdTimeBar = xpgUpdateOsdTimeBar;
	xpgUpdateFuncPtr.xpgUpdateTimeBar    = xpgUpdateTimeBar;
#if (DISPLAY_VIDEO_DROP_INFO_FULL || DISPLAY_VIDEO_DROP_INFO_THUMB)
	xpgUpdateFuncPtr.xpgDispVideoDropInfo= xpgDispVideoDropInfo;
#endif
#if DISPLAY_VIDEO_SUBTITLE
    xpgUpdateFuncPtr.xpgUpdateOsdSRT = xpgUpdateOsdSRT;
#endif
#if LYRIC_ENABLE
    xpgUpdateFuncPtr.xpgUpdateOsdLrc = xpgUpdateOsdLrc;
    xpgUpdateFuncPtr.xpgLyricUIControl = xpgLyricUIControl;
#endif
}

///
///@ingroup xpgFuncEntry
///@brief   Return XPG_FUNC_PTR pointer
///
XPG_FUNC_PTR *xpgGetFuncPtr()
{
    return &xpgUpdateFuncPtr;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL  g_boSaveFileToSPI=0;

static ST_IMGWIN g_stCacheWin;//for main menu background show currWin
static DWORD *g_pdwCacheWinBuf=NULL;
ST_IMGWIN *Idu_Cache_BGWin(ST_IMGWIN*pWin)
{
	ST_IMGWIN *pCacheWin=&g_stCacheWin,*pCurWin = Idu_GetCurrWin();

	MP_DEBUG("!!!Idu_Cache_BGWin ");
	if (pWin->wWidth != pCurWin->wWidth || pWin->wHeight != pCurWin->wHeight )
		return;
	if (g_pdwCacheWinBuf == NULL)
	{
		g_pdwCacheWinBuf = (DWORD*) ext_mem_malloc(pWin->wWidth * pWin->wHeight * 2);
		ImgWinInit(pCacheWin, g_pdwCacheWinBuf, pWin->wHeight, pWin->wWidth);
	}
	if (g_pdwCacheWinBuf)
        mpCopyEqualWin(pCacheWin, pWin);
	return pCacheWin;
}
ST_IMGWIN *Idu_GetCache_BGWin()
{
	if (g_pdwCacheWinBuf==NULL)
		g_stCacheWin.pdwStart=NULL;
	return &g_stCacheWin;
}
void Free_Cache_BGWin()
{
	MP_DEBUG("!!!Free_CacheWin %x",g_stCacheWin.pdwStart);
	if (g_pdwCacheWinBuf )
	{
		ext_mem_free(g_stCacheWin.pdwStart);
		g_stCacheWin.pdwStart=NULL;
		g_pdwCacheWinBuf=NULL;
	}
}

void DrakWin(ST_IMGWIN* pWin, DWORD largeNum, DWORD smallNum)
{
#if 1
	WORD i,j,wW=pWin->wWidth<<1;
	BYTE *pbPixel;

	for (j = 0; j < pWin->wHeight ; j++) 
	{
		pbPixel=(BYTE *)((DWORD *)pWin->pdwStart+j*(pWin->dwOffset>>2));
		for (i = 0; i < wW; i+=4) 
		{
			pbPixel[i]>>=5;
			pbPixel[i+1]>>=5;
		}
	}
#else
    WORD i, j;
    DWORD * pLineStart;
    DWORD value;
    DWORD y1, y2;

    for (j = 0; j < pWin->wHeight ; j++) 
    {
        pLineStart = & (pWin->pdwStart [j * (pWin->wWidth / 2)]);
        for (i = 0; i < pWin->wWidth / 2; i++) 
        {
            value = pLineStart[i];
            y1 = (value >> 24) & 0xff;
            y2 = (value >> 16) & 0xff;
            y1 = y1 * smallNum / largeNum;
            y2 = y2 * smallNum / largeNum;
            pLineStart[i] = 0x8080 | (y1 << 24) | (y2 << 16);
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////
ST_IMGWIN *Idu_GetCacheWin_WithInit()
{
	ST_IMGWIN *pCacheWin=&g_stCacheWin, *pWin = Idu_GetCurrWin();
	
	mpDebugPrint("Idu_GetCacheWin_WithInit 0x%08x",g_stCacheWin.pdwStart);
	if (pCacheWin->pdwStart == NULL)
	{
		ImgWinInit(pCacheWin, NULL, pWin->wHeight, pWin->wWidth);
		pCacheWin->pdwStart = ext_mem_malloc(pWin->wWidth * pWin->wHeight * 2);
		mpCopyEqualWin(pCacheWin, pWin);
	}
	return pCacheWin;
}
ST_IMGWIN *Idu_GetCacheWin()
{
	return &g_stCacheWin;
}
void Free_CacheWin()
{
	mpDebugPrint("Free_CacheWin 0x%08x",g_stCacheWin.pdwStart);
	if (g_stCacheWin.pdwStart )
	{
		ext_mem_free(g_stCacheWin.pdwStart);
		g_stCacheWin.pdwStart=NULL;
	}
}
/////////////////////////////////////////////////////////////////////


#if (defined(WATCHDOG_GPIO)&&(WATCHDOG_GPIO!=GPIO_NULL))
void SendWatchDog(void)
{
	static BYTE bLevel=0;

	if (bLevel)
		bLevel=0;
	else
		bLevel=1;
	SetGPIOValue(WATCHDOG_GPIO, bLevel);
	//mpDebugPrint(" ------- SendWatchDog IO 0x%x=%d",WATCHDOG_GPIO,bLevel);
	AddTimerProc(700, SendWatchDog);//total 1min
}
#endif


#endif

#if 1//OSD_DISPLAY_CACHE
#if OSD_LINE_NUM
ST_LINE g_OsdLine[OSD_LINE_NUM];
static BYTE st_bNeedUpdateOsd=0;

void OsdLineInit(void)
{
	DWORD i;

	for (i=0;i<OSD_LINE_NUM;i++)
		g_OsdLine[i].dwOnPageBit=0;
	st_bNeedUpdateOsd=1;
}
//--bLineIndex
//--0-3 端面
//--4-7 光纤上边
//--8-11 光纤中心
//--12-15 光纤下边


//--18-19 电极棒位置


BYTE OsdLineSet(DWORD dwOnPageBit,BYTE bLineIndex,WORD startX, WORD startY, WORD SizeX, WORD SizeY, DWORD Color)
{
	if (bLineIndex<OSD_LINE_NUM)
	{
		g_OsdLine[bLineIndex].dwColor=Color;
		g_OsdLine[bLineIndex].dwOnPageBit=dwOnPageBit;
		g_OsdLine[bLineIndex].wH=SizeY;
		g_OsdLine[bLineIndex].wW=SizeX;
		g_OsdLine[bLineIndex].wX=startX;
		g_OsdLine[bLineIndex].wY=startY;
		st_bNeedUpdateOsd=1;
	}
}
#endif

//extern WORD g_wCenterPos;
void xpgUpdatePageOsd(void)
{
	DWORD i;
	DWORD dwPage = g_pstXpgMovie->m_wCurPage;
	ST_IMGWIN *pDstWin= Idu_GetCurrWin();

#if OSD_LINE_NUM
	if (!st_bNeedUpdateOsd)
		return;
    MP_DEBUG("-- xpgUpdatePageOsd --");
	//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,16,400-1-1,0,2,pDstWin->wHeight,OSD_COLOR_WHITE); //4 屏中竖线//OSD_COLOR_RED
	//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,17,476,0,2,pDstWin->wHeight/2,OSD_COLOR_RED);
	//OsdLineSet(1<<g_pstXpgMovie->m_wCurPage,18,324,pDstWin->wHeight/2,2,pDstWin->wHeight/2,OSD_COLOR_RED);

	Idu_OsdErase();
	for (i=0;i<OSD_LINE_NUM;i++)
	{
		if (g_OsdLine[i].dwOnPageBit&(1<<dwPage))
		{
			//mpDebugPrint("-- xpgUpdatePageOsd -%d-",i);
			Idu_OsdPaintArea(g_OsdLine[i].wX,g_OsdLine[i].wY,g_OsdLine[i].wW,g_OsdLine[i].wH,g_OsdLine[i].dwColor);
		}
	}
	ShowOSDstring();
	st_bNeedUpdateOsd=0;
#endif

}

void xpgSetUpdateOsdFlag(BYTE bMode)
{
	st_bNeedUpdateOsd=bMode;
}

void xpgForceUpdateOsd()
{
	st_bNeedUpdateOsd=1;
	xpgUpdatePageOsd();
}

#endif


///
///@ingroup xpgFuncEntry
///@brief   Stop all action of Setup/Audio/Video
///
void xpgStopAllAction()
{
    MP_DEBUG("-- xpgStopAllAction --");
    
#if (RECORD_ENABLE == ENABLE)
    if (g_bAniFlag & ANI_CAMRECORDING)
	{
        MP_DEBUG("### ANI_CAMRECORDING ###");
        Camcorder_RecordStop();
        //Camcorder_PreviewStop();         //allen add to mini-DV
    }
    if(RecordTaskStattusGet() == ExceptionCheck_state)
	{
        Idu_OsdErase();
        Camcorder_RecordStop();
    }
#endif

#if (ENABLE_SLIDESHOW_FUNCTION == 1)
    RemoveTimerProc(xpgSlideShow);
#endif

#if AUDIO_ON && MAKE_XPG_PLAYER
    if (g_bAniFlag & ANI_AUDIO)
        xpgStopAudio();
#endif
#if VIDEO_ON
    if ((g_bAniFlag & ANI_VIDEO) || (g_bAniFlag & ANI_READY))
    {
#if MAKE_XPG_PLAYER
        xpgStopVideo();			//xpgCb_VideoPlayerExit();
#else
		NonxpgStopVideo();
#endif
        //CpuClockSwitchToPll1_96MHz();
        DisplayInit(DISPLAY_INIT_DEFAULT_RESOLUTION);
    }
#endif

    //Resolve the problem of going previous page error if removing card when slideshow
    if (g_bAniFlag & ANI_SLIDE)
    {
        g_bAniFlag &= ~ANI_SLIDE;
        ScalerClockReset();
    }
    g_bAniFlag = 0;
#if (ENABLE_SLIDESHOW_FUNCTION == 1)
    g_wSlideShowIndex = 0;
#endif
#if RTC_ENABLE
//    RemoveTimerProc(UpdateClock);
//    UpdateClock();
#endif
}

void xpgGotoNewPageInit(BYTE bCurPage,BYTE bNewPage)
{
	if (bCurPage==bNewPage)
		return;
	g_dwCurIndex=bNewPage<<16;
#if MAKE_XPG_PLAYER
	g_dwModeIconStatus=bNewPage<<16;
#endif
}

void xpgDelay(DWORD dwDelayTime)
{
    MP_DEBUG("xpgDelay");
    TimerDelay(dwDelayTime);
}


#if AUTO_SLEEP_ENABLE
static int boSleepState = 0;
void AutoSleep()
{
	if (GetSleepCondition())
    {
		mpDebugPrint("%s", __func__);
		boSleepState = 1;
		TurnOffBackLight();
    }
	else
	{
		CheckAutoSleepOrAutoOff();
	}
}

void BreakSleepState()
{
	boSleepState = 0;
	TurnOnBackLight();
}

int IsSleepState()
{
    return boSleepState;
}
#endif
#if AUTO_POWER_OFF
void AutoPowerOff()
{
		mpDebugPrint("%s", __func__);
	if (GetSleepCondition())
		SetSystemPower(0);//Ui_SystemPowerHold(0);
	else
		CheckAutoSleepOrAutoOff();
}
#endif
#if AUTO_SLEEP_ENABLE||AUTO_POWER_OFF
BYTE GetSleepCondition()
{
#if Video_Preview_Play_And_Stop
	if ((g_bAniFlag & ANI_VIDEO)&&InVideoPlayWait())
		return 1;
#endif
	if (((g_bAniFlag & ANI_VIDEO)&&(!(g_bAniFlag & ANI_PAUSE)))
#if SENSOR_ENABLE
		||(g_MinidvMode == CAMRECORDING || bRecordMode())
#endif
		)
	{
		//mpDebugPrint("%s g_bAniFlag=%x sleep=%d", __func__,g_bAniFlag,g_MinidvMode);
		return 0;
	}
	return 1;
}

void CheckAutoSleepOrAutoOff()
{
		//mpDebugPrint("%s condition=%d sleep=%d", __func__,GetSleepCondition(),g_psSetupMenu->sleepmode);
    Ui_TimerProcRemove(CheckAutoSleepOrAutoOff);
#if AUTO_SLEEP_ENABLE
    Ui_TimerProcRemove(AutoSleep);
#endif
#if AUTO_POWER_OFF
    Ui_TimerProcRemove(AutoPowerOff);
#endif

    if (GetSleepCondition())
    {
		#if AUTO_SLEEP_ENABLE
		if (g_psSetupMenu->sleepmode==1)
        	Ui_TimerProcAdd(1*60*1000, AutoSleep);
		else if (g_psSetupMenu->sleepmode==2)
        	Ui_TimerProcAdd(5*60*1000, AutoSleep);
		else if (g_psSetupMenu->sleepmode==3)
        	Ui_TimerProcAdd(10*60*1000, AutoSleep);
		#endif
		#if AUTO_POWER_OFF
		if (g_psSetupMenu->offmode==1)
        	Ui_TimerProcAdd(10*60*1000, AutoPowerOff);
		else if (g_psSetupMenu->offmode==2)
        	Ui_TimerProcAdd(30*60*1000, AutoPowerOff);
		#endif
    }
	else
			Ui_TimerProcAdd(1*60*1000, CheckAutoSleepOrAutoOff);

}
#endif


//------------------------------------------------------------------------------
void xpgCb_NullFunc()
{
}

void xpgCb_PressUpKey()
{
}

void xpgCb_PressDownKey()
{
}

void xpgCb_PressLeftKey()
{
    if (g_dwCurIndex&0x0000ffff)
			g_dwCurIndex--;

}

void xpgCb_PressRightKey()
{
#if MAKE_XPG_PLAYER
	STXPGSPRITE *pstSprite;

	pstSprite = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_HILIGHTFRAME, (g_dwCurIndex&0x0000ffff)+1);
    if (pstSprite)
			g_dwCurIndex++;
#endif
}

void xpgCb_PressEnterKey()
{
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
	ST_IMGWIN stViewWin,*pWin;

#if  (PRODUCT_UI==UI_SURFACE)
    if (dwHashKey == xpgHash("Preview"))
    {
			switch (g_dwCurIndex&0x0000ffff)
			{
				case 0:
					xpgCb_EnterSetupPage();
					break;

				case 1:
					xpgCb_EnterPhotoViewPage();
					break;

				case 2:
					pWin=Idu_GetCurrWin();
					ImgWinInit(&stViewWin,pWin->pdwStart+160/2,pWin->wHeight,pWin->wWidth-160);
					stViewWin.dwOffset=pWin->dwOffset;
					xpgCb_CaptureWinToFile(&stViewWin);
					FileBrowserResetFileList();
					FileBrowserScanFileList(SEARCH_TYPE);
					break;

				default:
					break;
			}
    }
	else if (dwHashKey == xpgHash("PhotoView"))
    {
			switch (g_dwCurIndex&0x0000ffff)
			{
				case 0:
					xpgCb_EnterCamcoderPreview();
					break;

				case 1:
					xpgMenuListNext();
					break;

				case 2:
					xpgMenuListPrev();
					break;

				case 3:
					g_psSetupMenu->bReferPhoto=FileBrowserGetCurIndex();
					break;

				case 4:
					xpgFileDelete();
					break;

				default:
					break;
			}
    }
	else if (dwHashKey == xpgHash("Setup"))
    {
			switch (g_dwCurIndex&0x0000ffff)
			{
				case 0:
					if (g_psSetupMenu->bBackUp)
						g_psSetupMenu->bBackUp--;
					break;

				case 1:
					if (g_psSetupMenu->bBackUp<255)
						g_psSetupMenu->bBackUp++;
					break;

				case 2:
					if (g_psSetupMenu->bBackDown)
						g_psSetupMenu->bBackDown--;
					break;

				case 3:
					if (g_psSetupMenu->bBackDown<255)
						g_psSetupMenu->bBackDown++;
					break;

				case 4:
					if (g_psSetupMenu->bPhotoUp)
						g_psSetupMenu->bPhotoUp--;
					break;

				case 5:
					if (g_psSetupMenu->bPhotoUp<255)
						g_psSetupMenu->bPhotoUp++;
					break;

				case 6:
					if (g_psSetupMenu->bPhotoDown)
						g_psSetupMenu->bPhotoDown--;
					break;

				case 7:
					if (g_psSetupMenu->bPhotoDown<255)
						g_psSetupMenu->bPhotoDown++;
					break;

				case 8:
					g_dwPassNum=0;
					g_dwFailNum=0;
					break;

				case 9:
					xpgCb_EnterCamcoderPreview();
					break;

				default:
					break;
			}
    }
#endif

}

void xpgCb_PressExitKey()
{
}

STXPGPAGE *xpgPreactionAndGotoPage(const char *name)
{
	DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    STXPGPAGE *pstPage = xpgMovieSearchPage(name);

    if (pstPage == NULL)
    {
        MP_ALERT("%s: XPG page name [%s] not found !", __FUNCTION__, name);
        return NULL;
    }

#ifdef DIALOG_PAGE_NAME
    if (0 == strcmp(name, DIALOG_PAGE_NAME))
        g_isDialogPage = 1;
    else
        g_isDialogPage = 0;
#endif
#if  (PRODUCT_UI==UI_WELDING)
	if (dwHashKey == xpgHash("Auto_work") || dwHashKey == xpgHash("Manual_work"))
		xpgCb_StopAllSensorWork();
	if (dwHashKey == xpgHash("Main"))
		Ui_TimerProcRemove(xpgCb_EnterCamcoderPreview);
#endif

	//mpDebugPrint("---------xpgPreactionAndGotoPage :%s-> %d",name,pstPage->m_wIndex);
    if (xpgGotoPage(pstPage->m_wIndex) != PASS)
    {
        MP_ALERT("xpgPreactionAndGotoPage : xpgGotoPage() failed !");
        return NULL;
    }
#if  (PRODUCT_UI==UI_WELDING)
	if (g_pstXpgMovie->m_pstCurPage->m_dwHashKey == xpgHash("Main"))
		AddAutoEnterPreview();
#endif

    return pstPage;
}

#if (SENSOR_ENABLE == ENABLE)
STREAM *GetNewCaptureHandle()
{
    STREAM *handle;
    DWORD diskSize;

	  diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
      diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

      if (diskSize < MIN_FREE_DISK_SPACE)
      {
          MP_ALERT("--E-- %s: Disk free space small than %dMB, remain %dMB", __FUNCTION__, MIN_FREE_DISK_SPACE, diskSize);
			handle=NULL;
         	TaskSleep(4000);
      }
		else
		{
			#if (PRODUCT_UI==UI_SURFACE)
			handle = (STREAM *)CreatePhotoFileByIndex();
			#else
			//handle = (STREAM *)CreateFileByTime("/DCIM/","JPG");
			handle=(STREAM *)GetNewWeldPhotoHandle();
			#endif
		}
      return handle;

}

SWORD xpgCb_CaptureWinToFile(ST_IMGWIN *pWin)
{
    STREAM *handle;
    MP_DEBUG("%s -%d", __FUNCTION__,DriveCurIdGet());

	if (DriveCurIdGet()==NULL_DRIVE)
    {
        MP_ALERT("%s: NULL drive index!", __FUNCTION__);
        return FAIL;
    }
	if (SystemGetFlagReadOnly(DriveCurIdGet()))
	{
        MP_ALERT("Write protected !");
	      TaskSleep(4000);
	      return FAIL;
	}

	handle=(STREAM *)GetNewCaptureHandle();

	if (handle)
	{
		MP_DEBUG1("## %s ##", __FUNCTION__);		
#ifdef OSDICON_DC
		BYTE bStr[16];
		sprintf(bStr,"%d", GetNeedCaptureNumber());
		ShowActionIconWithString(OSDICON_DC,bStr,OSD_NEW_WHITE,500);
#endif

		SWORD swRet=PASS;
		DWORD JpegBufSize;
		DWORD IMG_size = 0;
		BYTE *JpegBuf = NULL;
		ST_IMGWIN stTmpWin;

		mpWinInit(&stTmpWin,NULL,pWin->wHeight,pWin->wWidth);
		stTmpWin.pdwStart = (DWORD *)ext_mem_malloc(stTmpWin.wWidth*stTmpWin.wHeight*2);
		//mpCopyWin(&stTmpWin, pWin);
		Set_Display_flag(DISABLE);
        mpCopyEqualWin(&stTmpWin, pWin);
		Set_Display_flag(ENABLE);
		JpegBufSize = pWin->wWidth*pWin->wHeight*2;
		JpegBuf = (BYTE*)ext_mem_malloc(JpegBufSize);


		//encode jpeg
#if CAPTURE_TIME_STAMP_ON
		WriteTimeStampStringToPhoto(&stTmpWin);
#endif
		IMG_size = ImageFile_Encode_Img2Jpeg(JpegBuf, &stTmpWin);

		if (JpegBufSize < IMG_size)
		{
			mpDebugPrint("--E-- %s: memory overflow", __FUNCTION__);
			//free memory
			DeleteFile(handle);
			swRet= -3;
		}
		else
		{
			FileWrite(handle, (BYTE *) JpegBuf, IMG_size);
			FileClose(handle);
			mpDebugPrint("-- %s:ok!", __FUNCTION__);
		}

		//free memory
		if(JpegBuf != NULL)
		{
			ext_mem_free(JpegBuf);
			JpegBuf = NULL;
		}
		ext_mem_free(stTmpWin.pdwStart );

		return swRet;

	}

    return FAIL;
}

static BYTE st_bWeldMode=0;// 0->auto  1->manual

void WeldModeSet(BYTE bMode)
{
	st_bWeldMode=bMode;
}

void xpgCb_EnterCamcoderPreview()
{
    //MP_DEBUG("%s", __func__);

    MP_DEBUG("%s: total=%d", __FUNCTION__,mem_get_free_space_total());

#if SENSOR_WITH_DISPLAY
       if (RecordTaskStattusGet() != Rec_StandBy_state)
		{
    		mpDebugPrint("%s:  state=%d", __FUNCTION__,RecordTaskStattusGet());
			return;
       }
#endif

#if USBOTG_WEB_CAM
    WHICH_OTG eWhichOtg = WEBCAM_USB_PORT;
	if (UsbOtgHostConnectStatusGet(eWhichOtg))
	{
    	MP_DEBUG("UsbOtgHostConnectStatusGet  ON");
		return;
	}
	if (SystemCardPresentCheck(USB_WEBCAM_DEVICE))
	{
    	MP_DEBUG("USBOTG_WEB_CAM  ON");
		return;
	}
#endif

	Ui_TimerProcRemove(xpgCb_EnterCamcoderPreview);
    //ES8328_Codec_SetRecordMode();
    //Ui_TimerProcRemove(AutoSleep);
#if RTC_ENABLE
    Ui_TimerProcRemove(UpdateClock);
#endif

	mpClearWin(Idu_GetNextWin());
#if MAKE_XPG_PLAYER
    xpgChangeMenuMode(OP_IMAGE_MODE, 1);
	if (st_bWeldMode)
		xpgPreactionAndGotoPage("Manual_work");
	else
		xpgPreactionAndGotoPage("Auto_work");
    //xpgPreactionAndGotoPage("Preview");
    xpgUpdateStage();
    mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());
#endif

#if 0//((PRODUCT_UI==UI_SURFACE))
    Camcorder_PreviewStart(CAMCORDER_RESOLUTION_720P);
#elif (PRODUCT_UI==UI_WELDING)
    //Camcorder_PreviewStart(CAMCORDER_RESOLUTION_VGA);
    //Camcorder_PreviewStart(CAMCORDER_RESOLUTION_720P);
    Camcorder_PreviewStart(CAMCORDER_RESOLUTION_800x480);
    //Camcorder_PreviewStart(CAMCORDER_RESOLUTION_SVGA);
#else
   ////(PRODUCT_UI==UI_WELDING)
    Camcorder_PreviewStart(CAMCORDER_RESOLUTION_VGA);
#endif

   // g_MinidvMode = CAMPREVIEW;
#if (AUDIO_DAC==DAC_ALC5621)
      Codec_ElecSwitch_RecordMode();
#endif
    mpCopyEqualWin(Idu_GetNextWin(), Idu_GetCurrWin());

}

void xpgCb_StopAllSensorWork()
{
#if (RECORD_ENABLE == ENABLE)
    if(RecordTaskStattusGet() == Recording_pause_state)
    	Camcorder_RecordResume();

    if(RecordTaskStattusGet() == Recording_state || RecordTaskStattusGet() == ExceptionCheck_state)
    	Camcorder_RecordStop();

#endif
#if SENSOR_WITH_DISPLAY
    if(RecordTaskStattusGet() == Preview_state)
    {
    	Camcorder_PreviewStop();
	    //g_bAniFlag &= ~ANI_PREVIEW;
    }
#endif
}

void xpgCb_EnterSetupPage()
{
	MP_DEBUG("%s", __func__);
	xpgStopAllAction();
#if SENSOR_WITH_DISPLAY
    if(RecordTaskStattusGet() != Rec_StandBy_state)
    	Camcorder_PreviewStop();
#endif
    g_bXpgStatus = XPG_MODE_SETUP;
#if MAKE_XPG_PLAYER
    xpgPreactionAndGotoPage("Setup");
    xpgUpdateStage();
#endif
    
}

void xpgCb_EnterPhotoViewPage()
{
	MP_DEBUG("%s", __func__);
	xpgStopAllAction();
#if SENSOR_WITH_DISPLAY
    if(RecordTaskStattusGet() != Rec_StandBy_state)
    	Camcorder_PreviewStop();
#endif
    g_bXpgStatus = XPG_MODE_PHOTOVIEW;
#if MAKE_XPG_PLAYER
    xpgChangeMenuMode(OP_IMAGE_MODE, 1);
    xpgPreactionAndGotoPage("PhotoView");
    xpgUpdateStage();
#endif
    
}


void AddAutoEnterPreview(void)
{
#if SENSOR_WITH_DISPLAY
	Ui_TimerProcRemove(xpgCb_EnterCamcoderPreview);
	 if(RecordTaskStattusGet() == Rec_StandBy_state && g_pstXpgMovie->m_pstCurPage->m_dwHashKey==xpgHash("Main"))
	 {
		WeldModeSet(0);
#if TEST_TWO_LED
	    Ui_TimerProcAdd(10, xpgCb_EnterCamcoderPreview);//xpgCb_EnterCamcoderPreview
#else
	    Ui_TimerProcAdd(5*1000, xpgCb_EnterCamcoderPreview);//xpgCb_EnterCamcoderPreview  10
#endif
	 }
#endif
}


void Timer_FirstEnterCamPreview()
{
#if MAKE_XPG_PLAYER
    xpgChangeMenuMode(OP_IMAGE_MODE, 1);
#endif

#if SENSOR_ENABLE
#if (PRODUCT_UI==UI_WELDING)
	WeldDataInit();
    AddAutoEnterPreview();
#else
    Ui_TimerProcAdd(10, xpgCb_EnterCamcoderPreview);//xpgCb_EnterCamcoderPreview
#endif
#endif

}

#endif //#if (SENSOR_ENABLE == ENABLE)

//>------------UI CODE FUNCION-----------------------
void DialogCb_ExitMainPagePopError(void)
{
	g_dwMachineErrorShow=0;
	exitDialog();
}
void DialogCb_ExitMainPagePopHireWord(void)
{
	 if (0 == strcmp(g_psSetupMenu->strHirePassword, strEditPassword))
	 {
			g_bPowerOnCheckPassword=0;
			exitDialog();
	 }
	 else if (g_bPowerOnCheckPassword)
	 {
			g_bPowerOnCheckPassword--;
			memset(strEditPassword, 0, sizeof(strEditPassword));
			xpgUpdateStage();
	 }
	 else
	 {
	 	//SendCmdPowerOff();
			strDialogTitle = getstr(Str_Note);
			dialogOnClose = exitDialog;
			popupDialog(Dialog_Note_ForgetHirePassword, DIALOG_PAGE_NAME,Idu_GetCacheWin());
			xpgUpdateStage();
	 }
}
void DialogCb_ExitMainPagePopOpenWord(void)
{
	 if (0 == strcmp(g_psSetupMenu->srtOpenPassword, strEditPassword))
	 {
			g_bPowerOnCheckPassword=0;
			exitDialog();
	 }
	 else if (g_bPowerOnCheckPassword)
	 {
			g_bPowerOnCheckPassword--;
			memset(strEditPassword, 0, sizeof(strEditPassword));
			xpgUpdateStage();
	 }
	 else
	 {
	 	//SendCmdPowerOff();
			strDialogTitle = getstr(Str_Note);
			dialogOnClose = exitDialog;
			popupDialog(Dialog_Note_ForgetOpenPassword, DIALOG_PAGE_NAME,Idu_GetCacheWin());
			xpgUpdateStage();
	 }
}

void uiCb_CheckPopDialogAfterUpdatestage(void)
{
	if (g_pstXpgMovie->m_pstCurPage->m_dwHashKey == xpgHash("Main"))
	{
		if (g_dwMachineErrorFlag&g_dwMachineErrorShow)
		{
            DrakWin(Idu_GetCurrWin(), 2, 1);
            //strDialogTitle = NULL;
            dialogOnClose = DialogCb_ExitMainPagePopError;
            popupDialog(Dialog_MainPageError, "Main",Idu_GetCurrWin());
            xpgUpdateStage();
		}
		else if (g_bPowerOnCheckPassword&&(g_psSetupMenu->bEnableHirePassword||g_psSetupMenu->bEnableOpenPassword))
		{
			Idu_GetCacheWin_WithInit();
			DrakWin(Idu_GetCurrWin(), 2, 1);
			mpCopyEqualWin(Idu_GetCacheWin(), Idu_GetCurrWin());
			memset(strEditPassword, 0, sizeof(strEditPassword));
			if (g_psSetupMenu->bEnableHirePassword)
			{
				strDialogTitle = getstr(Str_ShuRuZhuJieMiMa);
				dialogOnClose = DialogCb_ExitMainPagePopHireWord;
				popupDialog(Dialog_PowerOnCheckHirePassword, "Main",Idu_GetCacheWin());
			}
			else
			{
				strDialogTitle = getstr(Str_ShuRuKaiJiMiMa);
				dialogOnClose = DialogCb_ExitMainPagePopOpenWord;
				popupDialog(Dialog_PowerOnCheckOpenPassword, "Main",Idu_GetCacheWin());
			}
			xpgUpdateStage();
		}
		else
		{
			g_dwMachineErrorShow=0;
			g_bPowerOnCheckPassword=0;
		}
	}
}

void Timer_CheckPopDialogAfterUpdatestage(void)
{
	Ui_TimerProcAdd(10, uiCb_CheckPopDialogAfterUpdatestage);
}

//<------------UI CODE FUNCION-----------------------


//>------------TOUCH UI FUNCION-----------------------

void xpgCb_AutoPowerOff(BYTE bEnable,DWORD dwTime)
{

	if (bEnable)
	{
		Ui_TimerProcAdd(dwTime*60000, SendCmdPowerOff);
	}
	else
	{
		Ui_TimerProcRemove(SendCmdPowerOff);
	}
}

//<------------TOUCH UI FUNCION-----------------------


