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
#include "mpapi.h"
#include "flagdefine.h"
#include "ui_timer.h"
#include "setup.h"
#if NETWARE_ENABLE
  #include "netware.h"
#endif
#if RTC_ENABLE
  #include "../../ui/include/rtc.h"
#endif
#if ((BT_XPG_UI == ENABLE) && (BT_PROFILE_TYPE & BT_A2DP))
  #include "xpgBtFunc.h"
#endif
#include "xpgCamFunc.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if AUDIO_ON
//volatile WORD g_wPlayAll = 0;	//If the play all is setting, the bit 15 set 1.
extern BOOL g_boSomeMusicSelected;
extern DWORD g_dwMusicRandomHistory[FAVOR_HISTORY_SIZE];
extern DWORD g_dwMusicFavoriteHistory[FAVOR_HISTORY_SIZE];
#endif

#if ( HAVE_NETSTREAM || NET_UPNP )
extern unsigned char irready;
extern unsigned char g_internetradiorun;
extern int kcurindx;
#endif

#if HAVE_NETSTREAM
extern int audio_type;
extern unsigned char pause_flag;
extern int mmsh_connection;
extern int ir_protocol;
extern DWORD giRadioStreamtype;

#endif //HAVE_NETSTREAM

#if NET_UPNP
extern unsigned char bupnpdownload;
extern unsigned char quitpnpdownload;
#endif

//------------------------------------------------------------------------------------------
//Music Menu
//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   Update Music Icon
///
///@param   bFlag - decide icon role
///
void xpgUpdateMusicIcon(BYTE bFlag)
{
#if AUDIO_ON && MAKE_XPG_PLAYER
	BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO) || (bFlag == XPG_IMAGE_PLAY);
	BYTE bRole;

	if (bFlag == 0)
		bRole = 0; //boMusicPlaying ? XPG_ICON_PLAY : XPG_ICON_MUSIC;
	else if (bFlag == XPG_IMAGE_AUDIO)
		bRole = XPG_ICON_STOP;
	else if (bFlag == XPG_IMAGE_PLAY) {
		bRole = XPG_ICON_PLAY;
	} else if (bFlag == XPG_IMAGE_PAUSE)
		bRole = XPG_ICON_PAUSE;
	else if (bFlag == XPG_IMAGE_STOP)
		bRole = XPG_ICON_STOP;

	register STXPGMOVIE *pstMovie = &g_stXpgMovie;
	STXPGSPRITE *pstSprite;
	DWORD i;
	BOOL boMusicFile = (FileBrowserGetCurFileType() == FILE_OP_TYPE_AUDIO);


	for (i = 0; i < pstMovie->m_dwSpriteCount; i++)
	{
		pstSprite = &(pstMovie->m_astSprite[i]);
		switch (pstSprite->m_dwType)
		{
		case SPRITE_TYPE_LISTICON:
			if (pstSprite->m_boVisible)
				xpgDrawSprite_ListIcon(Idu_GetCurrWin(), pstSprite, 0);

			break;

		case SPRITE_TYPE_MUSICICON:
			if (!boMusicFile)
			{
				pstSprite->m_boVisible = false;
				break;
			}
			pstSprite->m_boVisible = true;
			if (bFlag == 0)
				bRole = boMusicPlaying ? XPG_ICON_PLAY : XPG_ICON_STOP;
			else if (bFlag == XPG_IMAGE_AUDIO)
				bRole = XPG_ICON_STOP;
			else if (bFlag == XPG_IMAGE_PLAY)
				bRole = XPG_ICON_PLAY;
			else if (bFlag == XPG_IMAGE_PAUSE)
				bRole = XPG_ICON_PAUSE;
			else if (bFlag == XPG_IMAGE_STOP)
				bRole = XPG_ICON_STOP;

			xpgSpriteSetRole(pstSprite, g_pstXpgMovie->m_pstObjRole[bRole]);
			break;

		case SPRITE_TYPE_TIMEBAR:
			pstSprite->m_boVisible = boMusicFile;
			if (pstMovie->m_pstListFrame != NULL)
			{
				//xpgSpriteMoveTo(pstSprite,
				//	pstSprite->m_wPx, pstMovie->m_pstListFrame->m_wBottom - pstSprite->m_wHeight);
			}
			break;

		case SPRITE_TYPE_TIMEBARSLIDER:
			pstSprite->m_boVisible = boMusicFile;
			if (pstMovie->m_pstListFrame != NULL)
			{
				//xpgSpriteMoveTo(pstSprite,
				//	pstSprite->m_wPx, pstMovie->m_pstListFrame->m_wBottom - 4);
			}
			break;
		}
	}
#endif
}

//------------------------------------------------------------------------------------------
// For music list select move arrow don't stop music playing
#define MUSIC_LIST_MOVE_STOPPLAY 0
#define MUSIC_LIST_AUTO_PLAY 0
//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   stop audio before Move
///
void xpgMusicListBeforeMove()
{
#if AUDIO_ON && MAKE_XPG_PLAYER

//#if MUSIC_LIST_MOVE_STOPPLAY
    if (g_bAniFlag & ANI_AUDIO)
    {
        xpgStopAudio();
        xpgUpdateMusicIcon(XPG_IMAGE_STOP);
    }
//#endif

#endif
}



//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   play audio after Move
///
void xpgMusicListAfterMoved()
{
#if AUDIO_ON && MAKE_XPG_PLAYER

//#if MUSIC_LIST_AUTO_PLAY
    if (FileBrowserGetCurFileType() == FILE_OP_TYPE_AUDIO)
        xpgPlayAudio();
//#endif

#endif
}



//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   change menu list to next
///
void xpgCb_MusicMenuListNext()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    if (g_psSystemConfig->sFileBrowser.dwAudioTotalFile == 0)
        return;

    BYTE i = 0;

    BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO);
    xpgMusicListBeforeMove();
    xpgMenuListNext();

    if (boMusicPlaying)
        xpgMusicListAfterMoved();

    g_boNeedRepaint = false;
#endif
}



//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   change menu list to previous
///
void xpgCb_MusicMenuListPrev()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    if (g_psSystemConfig->sFileBrowser.dwAudioTotalFile == 0)
        return;

    BYTE i = 0;
    BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO);

    xpgMusicListBeforeMove();
    xpgMenuListPrev();

    if (boMusicPlaying)
        xpgMusicListAfterMoved();

    g_boNeedRepaint = false;
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Exit from menu list
///
void xpgCb_MusicMenuListExit()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    MP_DEBUG("xpgCb_MusicMenuListExit");

    if (g_bAniFlag & ANI_AUDIO) {
        xpgCb_MusicPlayerExit();
        g_bAniFlag &= ~ANI_AUDIO;
        Idu_OsdPaintArea(30, 340, 370, 100, 0); // clear ProgressBar
    }
    else {
        Idu_OsdErase();
        if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO) // exit from INTERNET_RADIO
        {
            //mpDebugPrint("g_bXpgStatus == XPG_MODE_NET_PC_AUDIO");
            g_bXpgStatus = XPG_MODE_NET_FUNC;
            xpgSearchAndGotoPage("Net_Func",8);
            //extern NETFUNC_t NETFUNC;
            //NETFUNC.dwCurrentIndex = 5; // internet radio
            xpgUpdateStage();
        }
        else
            xpgSearchAndGotoPage("Main_Music", 10);
    }
#endif
}



//------------------------------------------------------------------------------------------

//void xpgCb_MusicMenuShowList() {}
//void xpgCb_MusicMenuGetList() {}
///
///@ingroup xpgAudioFunc
///@brief   change menu list to next page
///
void xpgCb_MusicMenuListNextPage()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO);

    xpgMusicListBeforeMove();
    xpgMenuListNextPage();

    if (boMusicPlaying)
        xpgMusicListAfterMoved();
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   change menu list to previous page
///
void xpgCb_MusicMenuListPrevPage()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO);

    xpgMusicListBeforeMove();
    xpgMenuListPrevPage();

    if (boMusicPlaying)
        xpgMusicListAfterMoved();
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   enter music player
///
void xpgCb_EnterMusicPlayer()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
        xpgCb_OpenFolder();
    else
        xpgCb_MusicPlay();
#endif
}

///
///@ingroup xpgAudioFunc
///@brief   auto play next audio
///
void xpgAutoPlayNextAudio()
{
#if 0
	DWORD i = 0;
	BOOL boMusicPlaying = (g_bAniFlag & ANI_AUDIO);

	if (boMusicPlaying)
		xpgStopAudio();


	//MP_DEBUG("xpgAutoPlayNextAudio");

	for (i = 0; i < g_psSystemConfig->sFileBrowser.dwAudioTotalFile; i++)
	{
		FileListAddCurIndex(1);

		if (g_wPlayAll > FILE_LIST_SIZE)
		{
			g_wPlayAll--;
			if (g_wPlayAll == FILE_LIST_SIZE)
			{
				if (g_psSetupMenu->bMusicRptState != SETUP_MENU_MUSIC_REPEAT_ONE)
					return;
				else
					g_wPlayAll = FILE_LIST_SIZE | g_psSystemConfig->sFileBrowser.dwAudioTotalFile;
			}

			if (g_wPlayAll > FILE_LIST_SIZE && FileBrowserGetCurFileType() == FILE_OP_TYPE_AUDIO)
			{
				xpgMenuListSetIndex(FileListAddCurIndex(0), false);
				TimerDelay(10);
				xpgPlayAudio();
				return;
			}
		}
		else
			return;
	}
#endif
}

//------------------------------------------------------------------------------------------
//Music Player
//------------------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   resume audio
///
void xpgResumeAudio()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
	g_bAniFlag &= ~ANI_PAUSE;

	xpgUpdateMusicIcon(XPG_IMAGE_PLAY);
	Api_MovieResume();

	MP_DEBUG("xpgResumeAudio");
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Pause audio
///
void xpgPauseAudio()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
	g_bAniFlag |= ANI_PAUSE;

	xpgUpdateMusicIcon(XPG_IMAGE_PAUSE);
	Api_MoviePause();

	MP_DEBUG("xpgPauseAudio");
#endif
}


#if AUDIO_ON
#if (AAC_SW_AUDIO || AAC_FAAD_AUDIO)
extern DWORD g_dwTotalSeconds;
static ST_SEARCH_INFO *pSearchInfo_aac;
#endif
#endif


///
///@ingroup xpgAudioFunc
///@brief   Play audio
///
void xpgPlayAudio()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    STREAM *sHandle, *lrcHandle;
    ST_SEARCH_INFO *pSearchInfo, *pSearchLyricInfo;
    RemoveTimerProc(xpgPlayAudio);

    if (g_bAniFlag & ANI_AUDIO)
        return;

    MP_DEBUG("xpgPlayAudio");

#if ((BLUETOOTH == ENABLE) && (BT_PROFILE_TYPE & BT_A2DP))
    MP_DEBUG("BT flags: GetA2dpConnect() = %u, MpxGetA2dpRecordingFlag() = %u, GetA2dpStreamStart() = %u", GetA2dpConnect(), MpxGetA2dpRecordingFlag(), GetA2dpStreamStart());
    if (GetA2dpConnect() == 1)  /* A2DP connected */
    {
        //if (GetA2dpStreamStart() == 0) /* GetA2dpStreamStart() NG!!  note: Because after xpgStopAudio(), the GetA2dpStreamStart() value is still 1 for a period of time for waiting BT state machine !! */
        if (MpxGetA2dpRecordingFlag() == 0)
        {
            MP_ALERT("%s: A2DP stream suspended => BtA2dpStreamStart()...", __FUNCTION__);
            BtA2dpStreamStart();
            return; /* let BT state machine finish its action, then process audio playing in A2DP message handler */
        }
    }
#endif

    if (!(g_bAniFlag & ANI_ScreenSaver) && (g_bXpgStatus != XPG_MODE_UPNP_FILE_LIST) )
    {
        xpgUpdateMusicIcon(XPG_IMAGE_PLAY);
        xpgUpdateStage();
    }

    int iIndex;
#if NETWARE_ENABLE
    if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO || g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
    {
        iIndex = NetGetFileIndex();
        MP_DEBUG("iIndex %x",iIndex);
        pSearchInfo = (ST_SEARCH_INFO *)NetGetFileEntry(iIndex);
        MP_ASSERT(iIndex == 0);
    }
    else
#endif
    {
        if (g_psSystemConfig->sFileBrowser.dwAudioTotalFile == 0)
        {
            MP_DEBUG("%s: No audio file in current file list.", __FUNCTION__);
            return;
        }

        // For music select none stop
        iIndex = FileBrowserGetCurIndex();
        pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(iIndex);

    }

    g_psSystemConfig->sAudioPlayer.dwPlayIndex = iIndex;
    g_psSystemConfig->sAudioPlayer.pCurPlaySearchInfo = pSearchInfo;
    // For music select none stop - end

    if (pSearchInfo == NULL)
    {
        MP_ALERT("%s: NULL pSearchInfo !", __FUNCTION__);
        return;
    }

#if (AAC_SW_AUDIO || AAC_FAAD_AUDIO)
    g_dwTotalSeconds = pSearchInfo->dwMediaInfo;
    pSearchInfo_aac=pSearchInfo;
#endif

#if NET_UPNP
    if ( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
    {
        sHandle = Net_FileListOpen(DriveGet(USB_WIFI_DEVICE),pSearchInfo);
    }
    else
#endif
    {
        DriveHandleCopy(DriveGet(MAX_DRIVE_NUM), DriveGet(DriveCurIdGet()));
        sHandle = FileListOpen(DriveGet(MAX_DRIVE_NUM), pSearchInfo);
    }
    MP_DEBUG1("sHandle = %x", sHandle);

    if (sHandle == NULL)
    {
        MP_ALERT("%s: file open failed, NULL file handle !", __FUNCTION__);
        return;
    }

    lrcHandle = NULL;

#if LYRIC_ENABLE
    MP_DEBUG("%s: LYRIC_ENABLE processing LRC ...", __FUNCTION__);

    BYTE pbTempBuffer[512]; //зяжи dynamic allocated
    DRIVE *lrcDrv = DriveGet(DriveCurIdGet());
    char bName[9];

    memcpy(bName, pSearchInfo->bName, 8);
    bName[8] = 0;

    if (lrcHandle == NULL)
    {
        if (FileSearch(lrcDrv, bName, "lrc", E_FILE_TYPE) == FS_SUCCEED)
        {
            lrcHandle = FileOpen(lrcDrv);
            MP_DEBUG("%s: %s.lrc file found.", __FUNCTION__, bName);
        }
        else
            mpDebugPrint("%s.lrc NOT found!", bName);
    }

    if (lrcHandle == NULL) // try long filename with ".lrc"
    {
        memset(pbTempBuffer, 0, 512);
        DWORD dwBufferCount;
        if (PASS == FileGetLongName(lrcDrv, pSearchInfo, pbTempBuffer, &dwBufferCount, sizeof(pbTempBuffer)))
        {
            if (pbTempBuffer[dwBufferCount*2-7] == 0x2E) //'.'
            {
                pbTempBuffer[dwBufferCount*2-5] = 0x6C; //'l'
                pbTempBuffer[dwBufferCount*2-3] = 0x72; //'r'
                pbTempBuffer[dwBufferCount*2-1] = 0x63; //'c'
            }

            if (FileSearchLN(lrcDrv, (WORD *) pbTempBuffer, dwBufferCount, E_FILE_TYPE) == PASS)
            {
                lrcHandle = FileOpen(lrcDrv);
                MP_DEBUG("%s: (UTF-16) *.lrc file found!", __FUNCTION__);
            }
            else
            {
                MP_DEBUG("%s: (UTF-16) *.lrc file not found!", __FUNCTION__);
            }
        }
        else
        {
            MP_ALERT("%s: FileGetLongName() failed !", __FUNCTION__);
        }
    } //if(lrcHandle == NULL, try long filename)

    if (lrcHandle == NULL)
    {
        MP_DEBUG("%s: -I- No corresponding .LRC file found.", __FUNCTION__);
    }
#endif

    g_bAniFlag |= ANI_AUDIO;
    g_boNeedRepaint = false;

#if (AUDIO_ON && (AAC_SW_AUDIO || AAC_FAAD_AUDIO))
    xpgUpdateTimeBar(g_dwTotalSeconds, 0);
    xpgUpdateMusicTotalTime(g_dwTotalSeconds); // added to show TotalTime
#endif

#if 0
    if ( g_bXpgStatus != XPG_MODE_UPNP_FILE_LIST )
    {
        STXPGMOVIE *pstMovie = &g_stXpgMovie;
        STXPGSPRITE *pstSprite = xpgSpriteFindType(pstMovie, SPRITE_TYPE_MUSICTIME, 0);
        //mpDebugPrint("pstSprite->m_wPx=%d, pstSprite->m_wPy=%d", pstSprite->m_wPx, pstSprite->m_wPy);
        WORD x=pstMovie->m_wScreenWidth/16*1; // 800x480 => 50,  1024x600 => 64;
        WORD y=pstMovie->m_wScreenHeight/6*5; // 800x480 => 400, 1024x600 => 500;
        xpgDrawProgressBar(x, y); // add to show ProgressBar
    }
#endif

#if NETWARE_ENABLE
    BYTE *pbExt;
    if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO)
    {
        pbExt = ((ST_NET_FILEENTRY *)pSearchInfo)->ExtName;
        mpDebugPrint("pbExt %s",pbExt);
        memcpy(pSearchInfo->bExt,pbExt,3);
    }
#endif

    SWORD swRet;
    XPG_FUNC_PTR *xpgFuncPtr = xpgGetFuncPtr();
    if (swRet = Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, lrcHandle, NULL, &g_sMovieWin, xpgFuncPtr, MUSIC_PLAY_MODE, 0))
    {
        MP_ALERT("%s: Api_EnterMoviePlayer() failed !", __FUNCTION__);
        FileClose(sHandle);
        g_bAniFlag &= ~ANI_AUDIO;
        SystemSetErrEvent(swRet);
        return;
    }

#if NETWARE_ENABLE
    //if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO)
    //    StartNetFileCache();
#endif

    TimerDelay(1);
    Api_MoviePlay();
    MP_DEBUG("MusicPlay done");

#endif //AUDIO_ON
}



#if AUDIO_ON && MAKE_XPG_PLAYER
#if (AAC_SW_AUDIO || AAC_FAAD_AUDIO)
void xpg_update_totaltime(int  total_time)
{
	STXPGSPRITE *pstListSprt = xpgSpriteFindType(g_pstXpgMovie, SPRITE_TYPE_LIST, g_pstBrowser->wListIndex);
	STXPGSPRITE *pstFrame = g_pstXpgMovie->m_pstListFrame;
	ST_IMGWIN *pCurWin = Idu_GetCurrWin();
	ST_IMGWIN *pNexWin = Idu_GetNextWin();
	WORD wLeft = pstFrame->m_wPx + pstFrame->m_wWidth + 8;
	BYTE *pbTempBuffer = (BYTE*)(&g_pstXpgMovie->m_astList[g_pstBrowser->wListIndex].m_pbTime);

	mp_sprintf(pbTempBuffer, "%02d:%02d\0", total_time / 60, total_time % 60);
	xpgDrawSprite(pNexWin, &g_pstXpgMovie->m_astSprite[0], 0);
	xpgDrawSprite_List(pNexWin, pstListSprt, 0);
	mpCopyWinArea(pCurWin, pNexWin, wLeft, pstFrame->m_wPy, wLeft + (5 * DEFAULT_FONT_SIZE), pstFrame->m_wPy + pstFrame->m_wHeight);
}
#endif
#endif

//------------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   Stop audio
///
void xpgStopAudio()
{
    MP_DEBUG("xpgStopAudio");
#if AUDIO_ON && MAKE_XPG_PLAYER
	if (!(g_bAniFlag & ANI_AUDIO))
		return;
  #if NETWARE_ENABLE
	if(g_bXpgStatus == XPG_MODE_NET_PC_AUDIO)
		DisableNetFileCache();
	if(g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST)
		quitpnpdownload = TRUE;
  #endif

	// clear current play index and searchinfo
	g_psSystemConfig->sAudioPlayer.dwPlayIndex = 0;
	g_psSystemConfig->sAudioPlayer.pCurPlaySearchInfo = NULL;
	MP_DEBUG("xpgStopAudio");
	Api_MovieStop();
	Api_ExitMoviePlayer();
	g_bAniFlag &= ~ANI_AUDIO;
	g_bAniFlag &= ~ANI_PAUSE;
  #if ( HAVE_NETSTREAM || NET_UPNP )
	g_internetradiorun = 0;
	kcurindx = 0;
  #endif

	TaskYield(); //TimerDelay(3);

  #if RTC_ENABLE
    RemoveTimerProc(UpdateClock);
    AddTimerProc(2000, UpdateClock);
  #endif

  /* note: must suspend BT A2DP stream here for BT AVRCP profile */
  #if ((BLUETOOTH == ENABLE) && (BT_PROFILE_TYPE & BT_A2DP))
    if ((GetA2dpConnect() == 1) && (GetA2dpStreamStart() == 1))
    {
        BtA2dpStreamSuspend();
        MP_ALERT("#### %s: suspend A2DP stream => BtA2dpStreamSuspend()...", __FUNCTION__);
        TaskSleep(500); /* note: let BT state machine finish its action before next xpgPlayAudio() call */
        return;
    }
  #endif
#endif
}

//---------------------------------------------------------------------------
///
///@ingroup xpgAudioFunc
///@brief   Audio End and change to next by off/one/all
///
void xpgAudioEnd()
{
    MP_DEBUG("xpgAudioEnd()");
#if AUDIO_ON && MAKE_XPG_PLAYER
	STREAM *sHandle;
	ST_SYSTEM_CONFIG *psSysConfig;
	BYTE Flag = 0;
	int iOffset = 1;
	psSysConfig = g_psSystemConfig;
	ST_SEARCH_INFO *pSearchInfo;

#if VIDEO_ON
	if (g_bAniFlag & ANI_VIDEO)
	{
		xpgVideoEnd();
		return;
	}
#endif

	if ((!g_bAniFlag & ANI_AUDIO))
		return;

	if (g_bAniFlag & ANI_PAUSE)
	{
		MP_ALERT("%s: g_bAniFlag & ANI_PAUSE - return", __FUNCTION__);
		return;
	}

	if (g_bAniFlag & ANI_SLIDE)
	{
		if (g_bAniFlag & ANI_AUDIO)
		{
			xpgStopAudio();
		}
		xpgDelay(5);

		//tallest disables slideshowsettingmusic and then scan all music files
		//=>SLIDE music on
#if FAVOR_HISTORY_SIZE
		if (g_psSetupMenu->bSlideShuffle == SETUP_MENU_ON) //Calculate the offset to next random music
			iOffset = xpgShuffleGetNextIndex(&g_dwMusicFavoriteHistory, &g_dwMusicRandomHistory, psSysConfig->sFileBrowser.dwAudioTotalFile) - psSysConfig->sFileBrowser.dwAudioCurIndex;

		else if (g_boSomeMusicSelected == true) //Calculate the offset to next selected music
			iOffset = xpgFavorGetNextIndex(&g_dwMusicFavoriteHistory, psSysConfig->sFileBrowser.dwAudioCurIndex, psSysConfig->sFileBrowser.dwAudioTotalFile, true) - psSysConfig->sFileBrowser.dwAudioCurIndex;
        else
            MP_ALERT("Unknown condition ???");
#endif            
		//mpDebugPrint("	Offset: %d", iOffset);
		pSearchInfo = (ST_SEARCH_INFO *) FileBrowserSearchNextAudio(iOffset);
		if (pSearchInfo != NULL)
		{
			DriveHandleCopy(DriveGet(MAX_DRIVE_NUM), DriveGet(DriveCurIdGet()));
			sHandle = FileListOpen(DriveGet(MAX_DRIVE_NUM), pSearchInfo);
			//sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
			MP_DEBUG1("sHandle = %x", sHandle);
			if (sHandle == NULL)
				return;

			g_bAniFlag |= ANI_AUDIO;
			XPG_FUNC_PTR *xpgFuncPtr = xpgGetFuncPtr();
			MP_DEBUG("%s: => Api_EnterMoviePlayer() ...", __FUNCTION__);
			if (Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, NULL, NULL, &g_sMovieWin, xpgFuncPtr, MUSIC_PLAY_MODE, 0))
			{
				MP_ALERT("%s: => Api_MoviePlay() failed ! => close file ...", __FUNCTION__);
				FileClose(sHandle);
			}

			MP_DEBUG("%s: => Api_MoviePlay() ...", __FUNCTION__);
			Api_MoviePlay();

			MP_DEBUG("%s: MusicPlay done", __FUNCTION__);
		}
		else
			mpDebugPrint("Search audio fail...");
	}
	else // Non-SlideShow , Muisc mode only
	{
		xpgStopAudio();
		if (psSysConfig->dwCurrentOpMode == OP_AUDIO_MODE)
		{
			//If in shuffle mode, always "repeat all"
			// temp removed, this should depend on different platform setting
#if 0
		    if (g_psSetupMenu->bSlideShuffle == SETUP_MENU_ON)
            {
    			xpgCb_MusicStop();
                psSysConfig->sFileBrowser.dwAudioCurIndex =
                xpgShuffleGetNextIndex (&g_dwMusicFavoriteHistory, &g_dwMusicRandomHistory, psSysConfig->sFileBrowser.dwAudioTotalFile);
                xpgMenuListSetIndex (psSysConfig->sFileBrowser.dwAudioCurIndex, false);
    			xpgPlayAudio();
				return;
            }
#endif
			switch (g_psSetupMenu->bMusicRptState)
			{
				case SETUP_MENU_MUSIC_REPEAT_OFF:
					if (psSysConfig->sFileBrowser.dwAudioTotalFile == (psSysConfig->sFileBrowser.dwAudioCurIndex + 1))
					{
						//MP_DEBUG3("g_wPlayAll =%d, dwAudioTotalFile=%d, dwAudioCurIndex=%d", g_wPlayAll, psSysConfig->sFileBrowser.dwAudioTotalFile, psSysConfig->sFileBrowser.dwAudioCurIndex);
						xpgCb_MusicStop();
						return;
					}
					xpgMenuListNext();
					xpgPlayAudio();
					break;

				case SETUP_MENU_MUSIC_REPEAT_ALL:
				    if (DEMO_AUTO && psSysConfig->sFileBrowser.dwAudioTotalFile == (psSysConfig->sFileBrowser.dwAudioCurIndex + 1))
					{
						//MP_DEBUG3("g_wPlayAll =%d, dwAudioTotalFile=%d, dwAudioCurIndex=%d", g_wPlayAll, psSysConfig->sFileBrowser.dwAudioTotalFile, psSysConfig->sFileBrowser.dwAudioCurIndex);
						xpgCb_MusicStop();

						xpgSearchAndGotoPage("Mode_Video", 10);
						g_boNeedRepaint=true;
                        xpgUpdateStage();
                        xpgCb_EnterVideoMenu();
                        xpgCb_VideoPlay(); // wait until xpgVideoFunc.c's xpgVideoEnd()
						return;
					}

					STXPGBROWSER *pstBrowser = g_pstBrowser;
					do
					{
						xpgMenuListNext();
					} while (pstBrowser->pCurSearchInfo->bFileType == FILE_OP_TYPE_FOLDER);

					xpgPlayAudio();
					//xpgAutoPlayNextAudio();
					break;

				case SETUP_MENU_MUSIC_REPEAT_ONE:

                    if(g_bXpgStatus == XPG_MODE_BLUETOOTH)
                    {
                        xpgPlayAudio();
                    }
                    else
                    {
    					xpgCb_MusicStop();
    					xpgDelay(4);
                        xpgPlayAudio();
                    }
					break;
			}
		}
	}
#endif
}

//------------------------------------------------------------------------------
//extern DWORD g_dwAudioTotalTime;
///
///@ingroup xpgAudioFunc
///@brief   Music Play
///
void xpgCb_MusicPlay()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    MP_DEBUG("xpgCb_MusicPlay");

#if ( HAVE_NETSTREAM || NET_UPNP )
    if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO || g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
    {
        irready = 0;
        MP_DEBUG("SET INTERNET_RADIO_EVENT");
        if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO )
            EventSet(NETWORK_STREAM_EVENT, 0x1);
        else if( g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
            EventSet(UPNP_START_EVENT, 0x00010000 );

        MP_DEBUG("END SET INTERNET_RADIO_EVENT");
        while( irready == 0 )
        {
            TaskYield();
#if NET_UPNP
			if( quitpnpdownload == TRUE )
				return;
#endif
        }

        if( irready == 0xff )
        {
#if HAVE_NETSTREAM
            //xpgShowDialog(GetSystemErrMsg(ERR_AUDIO_CODEC));
            switch(giRadioStreamtype)
            {
            case 0://shoutcast
                g_bXpgStatus = XPG_MODE_IRADIO_STATION;
                xpgSearchAndGotoPage("Net_PhotoSet",12);
                break;

            case 1://vtuner
                g_bXpgStatus = XPG_MODE_VTUNER_LOCATION;
                vtuner_free_entry();
                vtuner_Set_menu_level(2);
                xpgSearchAndGotoPage("Net_UserList",12);
                break;
            }
#endif
            return;
        }
    }
    else
#endif	//HAVE_NETSTREAM
        if (g_pstBrowser->wCount == 0)
            return;

    if (g_bAniFlag & ANI_AUDIO)
    {                           // check if av playing
        int iIndex = FileBrowserGetCurIndex();

        if (g_psSystemConfig->sAudioPlayer.dwPlayIndex == iIndex)
        {
            if (g_bAniFlag & ANI_PAUSE)
                xpgResumeAudio();
            else
                xpgPauseAudio();

            xpgDelay(1);
            g_boNeedRepaint = false;
            return;
        }
        else
        {
            xpgSetIcon(XPG_ICON_PLAY);
            xpgStopAudio();
            xpgDelay(1);
        }
    }

#if NETWARE_ENABLE
    if( g_bXpgStatus == XPG_MODE_NET_PC_AUDIO || g_bXpgStatus == XPG_MODE_UPNP_FILE_LIST )
    {
        MP_DEBUG("Network AP call xpgPlayAudio");
        xpgPlayAudio();
    }
    else
#endif
    {
#if FAVOR_HISTORY_SIZE
        if (xpgShuffleReset(&g_dwMusicFavoriteHistory, &g_dwMusicRandomHistory, g_psSystemConfig->sFileBrowser.dwAudioTotalFile))
            g_boSomeMusicSelected = true;//Check if any music selected
#endif

        ST_SEARCH_INFO *pSearchInfo = (ST_SEARCH_INFO *) FileGetCurSearchInfo();

        if (pSearchInfo == NULL)
            return;

        if (FileBrowserGetCurFileType() == FILE_OP_TYPE_FOLDER)
        {
            xpgSearchAndGotoPage("Mode_Music", 10);
            xpgCb_OpenFolder();
            g_boNeedRepaint = false; // prevent XpgUi.c xpgprocessEvent() to call xpgUpdateStage() again
            return;
        }

        if ((pSearchInfo->bParameter & SEARCH_INFO_FOLDER) // for use bParameter record invalid file
            || (pSearchInfo->bParameter & SEARCH_INFO_CHANGE_PATH)) //for use bParameter record invalid file
            return;

        if (pSearchInfo->dwMediaInfo == 0)
        {
            Media_Info *pMediaInfo = NULL;
            pMediaInfo = (Media_Info *)FileBrowserGetMediaInfo(pSearchInfo);
            pSearchInfo->dwMediaInfo = pMediaInfo->dwTotalTime;

            mpDebugPrint("pMediaInfo->dwTotalTime = %d", pMediaInfo->dwTotalTime);
            mpDebugPrint("pMediaInfo->dwBitrate = %d", pMediaInfo->dwBitrate);
        }

        if (pSearchInfo->dwMediaInfo > 0)
        {
#if ((BT_XPG_UI == ENABLE) && (BT_PROFILE_TYPE & BT_A2DP))
extern DWORD BT_OPERATE_STATUS;

            if (BT_OPERATE_STATUS & BT_OP_STATUS_A2DP_MODE) /* BT A2DP connected */
            {
                mpDebugPrint("Start to play music by BT...");
                if (GetA2dpStreamStart() == 1)
                {
                    MP_DEBUG("%s: (GetA2dpStreamStart() == 1)", __FUNCTION__);

                    if (!(GetAniFlag() & (ANI_PAUSE|ANI_AUDIO)))
                    {
                        MP_DEBUG("%s: => xpgPlayAudio()...", __FUNCTION__);
                        xpgPlayAudio();
                    }
                    else if (GetAniFlag() & ANI_PAUSE)
                    {
                        MP_DEBUG("%s: => xpgResumeAudio()...", __FUNCTION__);
                        xpgResumeAudio();
                    }
                }
                else
                {
                    MP_DEBUG("%s: (GetA2dpStreamStart() == 0) => BtA2dpStreamStart()...", __FUNCTION__);
                    BtA2dpStreamStart();
                }
            }
            else
#endif
            {
                MP_DEBUG("%s: => xpgPlayAudio()...", __FUNCTION__);
                xpgPlayAudio();
            }
        }
#if ERROR_HANDLER_ENABLE
        else //take off and need rewrite
            xpgShowDialog(GetSystemErrMsg(ERR_AUDIO_CODEC));
#endif
    }

    g_boNeedRepaint = false;
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Music Play All files
///
void xpgCb_MusicPlayAll()
{
#if AUDIO_ON
#if 0
    g_wPlayAll = g_psSystemConfig->sFileBrowser.dwAudioCurIndex;

    if (!g_wPlayAll)			// index == 0
        g_wPlayAll = (g_psSystemConfig->sFileBrowser.dwAudioTotalFile - 1) | 0x8000;
    else
        g_wPlayAll = (g_wPlayAll - 1) | 0x8000;

    xpgCb_MusicPlay();
#endif
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Music Stop
///
void xpgCb_MusicStop()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    //if(g_pstBrowser->wCount == 0) return;
    MP_DEBUG("xpgCb_MusicStop");
    //g_wPlayAll = 0;
    if (g_bAniFlag & ANI_AUDIO)
        xpgUpdateMusicIcon(XPG_IMAGE_STOP);

    //xpgPauseAudio(); // It's not necessary to do here, it should be done by [PAUSE] event
    xpgStopAudio();
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Music Pause
///
void xpgCb_MusicPause()
{

}



///
///@ingroup xpgAudioFunc
///@brief   Music Forward
///
void xpgCb_MusicForward()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    if (g_bAniFlag & ANI_PAUSE)
    {
        g_bAniFlag &= ~ANI_PAUSE;
        Api_MovieResume();
    }

    if (g_bAniFlag & ANI_AUDIO)
        Api_AudioForward();

    xpgSetIcon(XPG_ICON_FORWARD);
    AddTimerProc(5, xpgEraseIcon);
    g_boNeedRepaint = false;
#endif
}



///
///@ingroup xpgAudioFunc
///@brief   Music Backward
///
void xpgCb_MusicBackward()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    if (g_bAniFlag & ANI_PAUSE)
    {
        g_bAniFlag &= ~ANI_PAUSE;
        Api_MovieResume();
    }

    if (g_bAniFlag & ANI_AUDIO)
        Api_AudioBackward();

    xpgSetIcon(XPG_ICON_BACKWARD);
    AddTimerProc(5, xpgEraseIcon);
    g_boNeedRepaint = false;
    //MovieBackward(0);
/* - turn off to prevent from backward UI crash with EQInfo 
    if (g_bAniFlag & ANI_AUDIO)
    {
        ST_IMGWIN *ImgWin = Idu_GetCurrWin();
        STXPGMOVIE *pstMov = &g_stXpgMovie;
        ImgWin->wX = pstMov->m_wScreenWidth/16*1; // 800x480 => 50,  1024x600 => 64;
        ImgWin->wY = pstMov->m_wScreenHeight/6*5; // 800x480 => 400, 1024x600 => 500;
        Idu_PaintWinArea(ImgWin, ImgWin->wX-2, ImgWin->wY+2, 256, 4, RGB2YUV(  0, 0, 0));
    }
*/    
#endif
}



//void xpgCb_MusicSetEQ() {}
//void xpgCb_MusicUpdateDuration(){}
///
///@ingroup xpgAudioFunc
///@brief   Exit from Music Player
///
void xpgCb_MusicPlayerExit()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    MP_DEBUG("xpgCb_MusicPlayerExit");
    xpgCb_MusicStop();

    g_boNeedRepaint = true;
    //xpgGotoPage(g_pstMenuPage->m_dwIndex);
    if (g_bXpgStatus == XPG_MODE_NET_PC_AUDIO)
    {
        //mpDebugPrint("g_bXpgStatus == XPG_MODE_NET_PC_AUDIO");
        g_bXpgStatus = XPG_MODE_NET_FUNC;
        xpgSearchAndGotoPage("Net_Func",8);
        //g_bXpgStatus = XPG_MODE_IRADIO_PLS; // 48 - Radio Station
        //xpgSearchAndGotoPage("Net_UserList",12);
        xpgUpdateStage();
    }
    else
        EnterMusicMenu(); //xpgCb_EnterMusicMenu();
#endif
}

#if AUDIO_ON
//extern ST_SETUP_MENU *g_psSetupMenu;
STREAM *sPageMusicHandle=NULL;
WORD countLoadPageMusic = 0;
WORD xpgGetcountLoadPageMusic()
{
    return countLoadPageMusic;
}
void xpgResetcountLoadPageMusic()
{
    countLoadPageMusic = 0;
}
void LoadPageMusic(char * m_pText)
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    MP_DEBUG("LoadPageMusic(m_pText = %s)", m_pText);

    DRIVE *drv = DriveGet(DriveCurIdGet());
    //DRIVE *drv = FileBrowserGetCurDrive();
    //if (DirReset(drv)) return;
    //mpDebugPrint("DriveCurIdGet()=%d", DriveCurIdGet());
    //int rtv = FileSearch(drv, "XMAS", "mp3", E_FILE_TYPE); // FileSearch(drv, bShortNameBuf, bExt, E_FILE_TYPE)
    int rtv = FileSearch(drv, m_pText, "mp3", E_FILE_TYPE);
    if(rtv)
    {
        MP_DEBUG("rtv = 0x%x, %s.mp3 is not exist!", rtv, m_pText);
        return; // XMAS.mp3 is not exist!
    }

    sPageMusicHandle = FileOpen(drv);
    //mpDebugPrint("sPageMusicHandle = 0x%x", sPageMusicHandle);
    if(sPageMusicHandle == NULL)
    {
        mpDebugPrint("sHandle == NULL");
        return;
    }
    MP_DEBUG("sPageMusicHandle = 0x%X", sPageMusicHandle);
    if(countLoadPageMusic == 0)
    {
        mpDebugPrint("countLoadPageMusic == 0, call Api_AudioHWEnable()");
        g_psSystemConfig->dwCurrentOpMode = OP_AUDIO_MODE;
        xpgChangeMenuMode(OP_AUDIO_MODE, 1); // MUST, or it will hang later by next song
        Api_AudioHWEnable(); // at least one appointed MP3 file
        countLoadPageMusic += 1;
    }
    else
    {
        mpDebugPrint("countLoadPageMusic = %d", countLoadPageMusic);
        countLoadPageMusic += 1;
    }
    
    XPG_FUNC_PTR *xpgFuncPtr=xpgGetFuncPtr();
    if (Api_EnterMoviePlayer(sPageMusicHandle, "mp3", NULL, NULL, &g_sMovieWin, xpgFuncPtr, MUSIC_PLAY_MODE, 0))
    {
        MP_ALERT("PageMusic playing error");
        FileClose(sPageMusicHandle);
        g_bAniFlag &= ~ANI_AUDIO;
        return;
    }

    g_bAniFlag |= ANI_AUDIO;
    TimerDelay(1);
    Api_MoviePlay();
    MP_ALERT("PageMusic playing !");
    // In xpgAudioEnd(), it will decide next song
#endif
}

void LoadPagePhotoMusic()
{
#if AUDIO_ON && MAKE_XPG_PLAYER
    MP_DEBUG("LoadPagePhotoMusic()");
    // Because PHOTO/MUSIC task conflict problem, try to get Audio file list and backup first
    // .... TODO
    
    int iOffset = 0;
    STREAM *sHandle=NULL;
		//=>SLIDE music on
	if (g_psSetupMenu->bSlideShuffle == SETUP_MENU_ON) //Calculate the offset to next random music
#if FAVOR_HISTORY_SIZE
		iOffset = xpgShuffleGetNextIndex(&g_dwMusicFavoriteHistory, &g_dwMusicRandomHistory, g_psSystemConfig->sFileBrowser.dwAudioTotalFile) - g_psSystemConfig->sFileBrowser.dwAudioCurIndex;
	else if (g_boSomeMusicSelected == true) //Calculate the offset to next selected music
		iOffset = xpgFavorGetNextIndex(&g_dwMusicFavoriteHistory, g_psSystemConfig->sFileBrowser.dwAudioCurIndex, g_psSystemConfig->sFileBrowser.dwAudioTotalFile, true) - g_psSystemConfig->sFileBrowser.dwAudioCurIndex;
    else 
        MP_ALERT("Unknown condition ???");
#endif
	mpDebugPrint("iOffset: %d", iOffset);
	
	Api_AudioHWEnable();
	ST_SEARCH_INFO *pSearchInfo;
	pSearchInfo = (ST_SEARCH_INFO *) FileBrowserSearchNextAudio(iOffset);
	if (pSearchInfo != NULL)
	{
		DriveHandleCopy(DriveGet(MAX_DRIVE_NUM), DriveGet(DriveCurIdGet()));
		sHandle = FileListOpen(DriveGet(MAX_DRIVE_NUM), pSearchInfo);
		//sHandle = FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
		if (sHandle == NULL)
		{
		    MP_ALERT("sHandle == NULL : just return");
			return;
		}

		g_bAniFlag |= ANI_AUDIO;
		XPG_FUNC_PTR *xpgFuncPtr = xpgGetFuncPtr();
		MP_DEBUG("%s: => Api_EnterMoviePlayer() ...", __FUNCTION__);
		if (Api_EnterMoviePlayer(sHandle, pSearchInfo->bExt, NULL, NULL, &g_sMovieWin, xpgFuncPtr, MUSIC_PLAY_MODE, 0))
		{
			MP_ALERT("%s: => Api_MoviePlay() failed ! => close file ...", __FUNCTION__);
			FileClose(sHandle);
			return;
		}

		MP_DEBUG("%s: => Api_MoviePlay() ...", __FUNCTION__);
		TimerDelay(1);
		TaskYield();
		Api_MoviePlay();

		MP_DEBUG("%s: MusicPlay done", __FUNCTION__);
	}
	else
		MP_ALERT("Search audio fail...");

    TaskYield();
#endif
}

#if MUSIC_PAGE_LIST_COUNT
#pragma alignvar(4)
musicinfo_t currPageMusicInfos[MUSIC_PAGE_LIST_COUNT] = {0};

int xpgFetchMp3Id3(STREAM *psHandle, id3tag_t* pstId3)
{	
	BYTE v2tag[10];

	mpDebugPrint("xpgFetchMp3Id3()");
	pstId3->Title[0] = pstId3->Title[1] = 0;
	pstId3->Artist[0] = pstId3->Artist[1] = 0;
	pstId3->TitleEncoding = pstId3->ArtistEncoding = 0;
	if ( psHandle == NULL )
		return FAIL;
	SeekSet(psHandle);
	FileRead(psHandle, v2tag, 10);
	if ( v2tag[0]=='I' && v2tag[1]=='D' && v2tag[2]=='3' )		// Id3 v2
	{
		BYTE  *buf;
		BYTE  version, frameFlag1, frameFlag2, encoding;
		DWORD tagAllSize, cnt, i, copysize;
		DWORD frameCC, frameSize;

		mpDebugPrint("Id3 version 2");
		tagAllSize = (((DWORD)v2tag[6]&0x7f)<<21) | (((DWORD)v2tag[7]&0x7f)<<14) |
			(((DWORD)v2tag[8]&0x7f)<<7) | ((DWORD)v2tag[9]&0x7f);
		mpDebugPrint("v2tagAllSize = %d", tagAllSize);
		version = v2tag[3];
		if ( version!=3 && version!=4 )
			return FAIL;
		buf = (BYTE*) ext_mem_malloc(tagAllSize);
		if ( buf == NULL)
			return FAIL;
		FileRead(psHandle, buf, tagAllSize);
		cnt = 0;
		while ( cnt < tagAllSize )
		{
			frameCC = (buf[cnt]<<24)|(buf[cnt+1]<<16)|(buf[cnt+2]<<8)|buf[cnt+3];
			cnt += 4;
			if ( version==3 )
				frameSize = (buf[cnt]<<24)|(buf[cnt+1]<<16)|(buf[cnt+2]<<8)|buf[cnt+3];
			else if ( version==4 )
				frameSize = ((buf[cnt]&0x7f)<<21)|((buf[cnt+1]&0x7f)<<14)|((buf[cnt+2]&0x7f)<<7)|(buf[cnt+3]&0x7f);
			cnt += 4;
			//frameFlag1 = buf[cnt];
			//frameFlag2 = buf[cnt+1];		// ignore the flag1 and flag2 temporarily
			cnt += 2;
			if ( frameCC==0x54504531 ||     // TPE1
                 frameCC==0x54495432 ||     // TIT2
                 frameCC==0x54414c42        // TALB
                 )
			{
				BYTE *pText, *pEncoding;
				WORD  wDstSize;
				if (frameCC==0x54504531) {          // TPE1
					pText = pstId3->Artist;	pEncoding = &pstId3->ArtistEncoding; wDstSize = sizeof(pstId3->Artist); }
				else if (frameCC==0x54495432) {     // TIT2
					pText = pstId3->Title;	pEncoding = &pstId3->TitleEncoding;	wDstSize = sizeof(pstId3->Title); }
                else if (frameCC==0x54414c42) {     // TALB
					pText = pstId3->Album;	pEncoding = &pstId3->AlbumEncoding;	wDstSize = sizeof(pstId3->Album); }
				encoding = buf[cnt];
				mpDebugPrint("Id3 string encoding = %d", encoding);
				if ( encoding == 0 )
				{
					copysize = (frameSize-1 < wDstSize-1) ? frameSize-1 : wDstSize-1;
					strncpy(pText, &buf[cnt+1], copysize);
					pText[copysize] = 0;
					*pEncoding = 0;
				}
				else if ( encoding == 1 )
				{
					DWORD isBigEndian, jmp;
					if ( buf[cnt+1]==0xff && buf[cnt+2]==0xfe ){
						isBigEndian = 0;	jmp = 3;	copysize = frameSize-3;		}
					else if ( buf[cnt+1]==0xff && buf[cnt+2]==0x00 && buf[cnt+3]==0xfe ){
						isBigEndian = 0;	jmp = 4;	copysize = frameSize-4;		}
					else if ( buf[cnt+1]==0xfe && buf[cnt+2]==0xff ){
						isBigEndian = 1;	jmp = 3;	copysize = frameSize-3;		}
					else if ( buf[cnt+1]==0xfe && buf[cnt+2]==0x00 && buf[cnt+3]==0xff ){
						isBigEndian = 1;	jmp = 4;	copysize = frameSize-4;		}
					//-------
					if ( copysize > wDstSize-2 )
						copysize = wDstSize-2;
					//-------
					if ( isBigEndian )
					{
						for ( i = 0; i < copysize ; i+=2 )
						{
							pText[i] = buf[cnt+jmp+i];
							pText[i+1] = buf[cnt+jmp+i+1];
							if ( buf[cnt+jmp+i]==0 && buf[cnt+jmp+i+1]==0 )
								break;
						}
						pText[copysize] = pText[copysize+1] = 0;
						*pEncoding = 2;
					}
					else
					{
						for ( i = 0; i < copysize ; i+=2 )
						{
							pText[i] = buf[cnt+jmp+i+1];		//little-endian to big-endian
							pText[i+1] = buf[cnt+jmp+i];
							if ( buf[cnt+jmp+i]==0 && buf[cnt+jmp+i+1]==0 )
								break;
						}
						pText[copysize] = pText[copysize+1] = 0;
						*pEncoding = 2;
					}
				}
				else if ( encoding == 2 )
				{
					copysize = frameSize-1;
					if ( copysize > wDstSize-2 )
						copysize = wDstSize-2;
					for ( i = 0; i < copysize ; i+=2 )
					{
						pText[i] = buf[cnt+i+1];
						pText[i+1] = buf[cnt+i+2];
						if ( buf[cnt+i+1]==0 && buf[cnt+i+2]==0 )
							break;
					}
					pText[copysize] = pText[copysize+1] = 0;
					*pEncoding = 2;
				}
				else if ( encoding == 3 )
				{
					copysize = (frameSize-1 < wDstSize-1) ? frameSize-1 : wDstSize-1;
					strncpy(pText, &buf[cnt+1], copysize);
					pText[copysize] = 0;
					*pEncoding = 3;
				}
				else
				{
					copysize = (frameSize < wDstSize-1) ? frameSize : wDstSize-1;
					strncpy(pText, &buf[cnt], copysize);
					pText[copysize] = 0;
					*pEncoding = 0;
				}
			}
			cnt += frameSize;
			while ( buf[cnt]==0 )
				cnt++;
			//if ()
			//	break;
		}
		ext_mem_free(buf);
	}
	else
	{
		BYTE id3v1_buf[128];
		Fseek(psHandle, 128, SEEK_END);
		FileRead(psHandle, id3v1_buf, 128);
		if ( id3v1_buf[0]=='T' && id3v1_buf[1]=='A' && id3v1_buf[2]=='G' )	// Id3 v1
		{
			mpDebugPrint("Id3 version 1");
			memcpy(pstId3->Title, &id3v1_buf[3], 30);
			pstId3->Title[30] = 0;
			memcpy(pstId3->Artist, &id3v1_buf[33], 30);
			pstId3->Artist[30] = 0;
            memcpy(pstId3->Album, &id3v1_buf[63], 30);
			pstId3->Album[30] = 0;
			mpDebugPrint("pstId3->Title = %s", pstId3->Title);
			mpDebugPrint("pstId3->Artist = %s", pstId3->Artist);
            mpDebugPrint("pstId3->Album = %s", pstId3->Album);
		}
	}
	if ( pstId3->Title[0]==0 && pstId3->Title[1]==0 && 
			pstId3->Artist[0]==0 && pstId3->Artist[1]==0 )
		return FAIL;
	return PASS;
}

static DWORD dwPageNumBak = 0xffffffff;
void xpgMusicFetchCurrPageInfo(BOOL force)
{
    DWORD i, dwViewIndex;
    DWORD dwTotalFile, dwCurrFileIndex, dwCurrPage;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    ST_SEARCH_INFO * pSearchInfo;
    DRIVE * pCurDrive;
    STREAM * sHandle;

    mpDebugPrint("%s()", __FUNCTION__);
    dwTotalFile = psFileBrowser->dwAudioTotalFile;
    if (dwTotalFile == 0)
        return;
    if (force)
        dwPageNumBak = 0xffffffff;
    
    dwCurrFileIndex = psFileBrowser->dwAudioCurIndex;
    dwCurrPage = dwCurrFileIndex / MUSIC_PAGE_LIST_COUNT;

    if (dwCurrPage == dwPageNumBak) {
        mpDebugPrint("%s() - the same page, skip it", __FUNCTION__);
        return;
    }

    for (i = 0; i < MUSIC_PAGE_LIST_COUNT; i++) 
        memset(&currPageMusicInfos[i], 0, sizeof(musicinfo_t));

    for (i = 0; i < MUSIC_PAGE_LIST_COUNT; i++) 
    {
        Media_Info sMediaInfo;
        
        dwViewIndex = dwCurrPage * MUSIC_PAGE_LIST_COUNT + i;
        if (dwViewIndex >= dwTotalFile)
            break;
        
        pSearchInfo = (ST_SEARCH_INFO *)FileGetSearchInfo(dwViewIndex);
        if (pSearchInfo == NULL) {
            MP_ALERT("%s: Error! pSearchInfo == NULL !", __FUNCTION__);
            return NULL;
        }
        
        pCurDrive = FileBrowserGetCurDrive();
        if (pCurDrive == NULL) {
            MP_ALERT("%s: Error! NULL drive handle !", __FUNCTION__);
            return NULL;
        }

        sHandle = FileListOpen(pCurDrive, pSearchInfo);
        if (sHandle == NULL) {
            MP_ALERT("%s: FileListOpen() failed !", __FUNCTION__);
            return NULL;
        }

        // get ID3 infomation
        xpgFetchMp3Id3(sHandle, &(currPageMusicInfos[i].id3));

        // get other infomation
        memset(&sMediaInfo, 0, sizeof(sMediaInfo));
        MovieOpenFileAndGetMediaInfo(sHandle, get_type_by_extension(&pSearchInfo->bExt[0],3), (BYTE *) &sMediaInfo);
        currPageMusicInfos[i].bitrate = sMediaInfo.dwBitrate;
        currPageMusicInfos[i].length = sMediaInfo.dwTotalTime;

        FileClose(sHandle);

        currPageMusicInfos[i].enable = 1;
    }

    dwPageNumBak = dwCurrPage;
    
    return;
}


void xpgUpdateMusicDetailed()
{
    ST_IMGWIN * pCurrWin;

    pCurrWin = Idu_GetCurrWin();

    Idu_PaintWinArea(pCurrWin, 220, 120, 250, 300, RGB2YUV(255, 0, 0));

    
}
#endif
#endif




