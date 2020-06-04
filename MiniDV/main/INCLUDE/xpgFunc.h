#ifndef __XPG__FUNC__H__
#define __XPG__FUNC__H__


#include "global612.h"
#include "xpg.h"


//-----------------------------------------------
/*
// Constant declarations
*/
enum {
    NULL_ACTION,
    ZOOM_IN,
    ZOOM_OUT,
    ROTATE_CLOCKWISE,
    ROTATE_ANTI_CLOCKWISE,
    MOVE_UP,
    MOVE_RIGHT,
    MOVE_DOWN,
    MOVE_LEFT,
};

typedef struct{
    DWORD    dwOnPageBit;				/* 1bit for 1page */
    WORD    wX;                    /* left top */
    WORD    wY;                    /* left top */
    WORD    wW;                    
    WORD    wH;                    
    DWORD    dwColor;				/* index for OSD,YYUV for idu*/
} ST_LINE;

#define AUTO_SLEEP_TIME         (120*1000)
#if (SENSOR_ENABLE == ENABLE)
#define MIN_FREE_DISK_SPACE1             200
#endif


extern DWORD g_bAniFlag;
extern BYTE g_bXpgStatus;
extern STXPGBROWSER *g_pstBrowser;
extern BOOL  g_boSaveFileToSPI;

extern DWORD g_dwCurIndex;

extern char * strDialogTitle;
extern void (*dialogOnClose)();
extern void exitDialog();


void xpgCb_NullFunc(void);

//Main Menu
void xpgCb_EnterMusicMenu();
void xpgCb_EnterVideoMenu();
void xpgCb_EnterPhotoMenu();

//Music Menu
void xpgCb_MusicMenuListNext();
void xpgCb_MusicMenuListPrev();
void xpgCb_MusicMenuListExit(); // act5=MusicMenuListExit
void xpgCb_MusicMenuShowList();
void xpgCb_MusicMenuGetList();
void xpgCb_MusicMenuListNextPage();
void xpgCb_MusicMenuListPrevPage();
void xpgCb_MusicMenuGetMediaInfo();
void xpgCb_MusicMenuGetFileInfo();
void xpgCb_MusicMenuSetFocusItem();
void xpgCb_EnterMusicPlayer();
#if VIDEO_ON
//Video Menu
void xpgCb_VideoMenuListNext();
void xpgCb_VideoMenuListPrev();
void xpgCb_VideoMenuListExit(); //act15=VideoMenuListExit
void xpgCb_VideoMenuListNextLine();
void xpgCb_VideoMenuListPrevLine();
void xpgCb_VideoMenuListNextPage();
void xpgCb_VideoMenuListPrevPage();
void xpgCb_VideoMenuGetThumb();
void xpgCb_VideoMenuDrawThumb();
void xpgCb_VideoMenuGetMediaInfo();
void xpgCb_VideoMenuGetFileInfo();
void xpgCb_VideoMenuSetFocusItem();
void xpgCb_EnterVideoPlayer();
void xpgVideoDrawPreview(ST_IMGWIN * pWin, DWORD x, DWORD y, DWORD w, DWORD h);
#endif

//Photo Menu
void xpgCb_PhotoMenuListRight();
void xpgCb_PhotoMenuListLeft();
void xpgCb_PhotoMenuThumbExit(); // act27=PhotoMenuThumbExit
void xpgCb_PhotoMenuListEnter();
void xpgCb_PhotoMenuShowList();
void xpgCb_PhotoMenuGetList();
void xpgCb_PhotoMenuThumbNextLine();
void xpgCb_PhotoMenuThumbPrevLine();
void xpgCb_PhotoMenuThumbNextPage();
void xpgCb_PhotoMenuThumbPrevPage();
void xpgCb_PhotoMenuGetMediaInfo();
void xpgCb_PhotoMenuGetFileInfo();
void xpgCb_PhotoMenuSetFocusItem();
void xpgCb_EnterPhotoView();

//Music Player
void xpgCb_MusicPlay();
void xpgCb_MusicStop();
void xpgCb_MusicPause();
void xpgCb_MusicForward();
void xpgCb_MusicBackward();
void xpgCb_MusicPlayAll();
void xpgCb_MusicUpdateDuration();
void xpgCb_MusicPlayerExit();

//Video Player
#if VIDEO_ON
void xpgCb_VideoPlay();
void xpgCb_VideoStop();
void xpgCb_VideoPause();
void xpgCb_VideoForward();
void xpgCb_VideoBackward();
void xpgCb_VideoUpdateDuration();
void xpgCb_VideoFullScreen();
void xpgCb_VideoPlayerExit();
#endif

//Photo View
void xpgCb_PhotoViewPrev();
void xpgCb_PhotoViewNext();
void xpgCb_PhotoViewExit();

void xpgCb_VolumnUp(void);
void xpgCb_VolumnDown(void);
void xpgCb_VolumeMute(void);

//---------------------------------------
void xpgEnableDrive(E_DRIVE_INDEX_ID drive_id);
void xpgDisableDrive(E_DRIVE_INDEX_ID drive_id);
BOOL xpgChangeDrive(E_DRIVE_INDEX_ID drive_id);
void xpgChangeMenuMode(DWORD dwOpMode, BYTE bFlag);

DWORD xpgProcessEvent(DWORD dwEvent, DWORD dwKeyCode);
//DWORD xpgProcessRepeatEvent(DWORD dwEvent, DWORD dwKeyCode);

void xpgStopAllAction();

void xpgUpdateTimeBar(DWORD dwTotal, DWORD wValue);
void xpgMenuListPrev();
void xpgMenuListNext();
void xpgMenuListPrevPage();
void xpgMenuListNextPage();




void mpCopyWinArea(ST_IMGWIN *pDstWin, ST_IMGWIN *pSrcWin, SWORD left, SWORD top, WORD right, WORD bottom) ;
BYTE xpgCheckDriveCount(void);
void xpgClearCatch();

XPG_FUNC_PTR *xpgGetFuncPtr();
void xpgCb_PressUpKey();
void xpgCb_PressDownKey();
void xpgCb_PressLeftKey();
void xpgCb_PressRightKey();
void xpgCb_PressExitKey();
void xpgCb_PressEnterKey();
void xpgCb_PressPowerKey();
void xpgCb_PressCamKey();




void WriteSetupChg(void);
void AutoSleep();
int IsSleepState();
void BreakSleepState();
int spi_ClearOldFileSpaceForNewFile(ST_SEARCH_INFO * pSearchInfo, DWORD dwTotalFiles);

ST_IMGWIN *Idu_Cache_BGWin(ST_IMGWIN*pWin);
ST_IMGWIN *Idu_GetCache_BGWin();
void Free_Cache_BGWin();

ST_IMGWIN *Idu_GetCacheWin_WithInit();
ST_IMGWIN *Idu_GetCacheWin();
void Free_CacheWin();
void DrakWin(ST_IMGWIN* pWin, DWORD largeNum, DWORD smallNum);
STXPGPAGE *xpgSearchtoPageWithAction(const char *name);

void xpgCb_AutoPowerOff(BYTE bEnable,DWORD dwTime);
void xpgCb_EnterCamcoderPreview();
void xpgCb_StopAllSensorWork();
void AddAutoEnterPreview(void);
void uiCb_CheckPopDialogAfterUpdatestage(void);
void DialogCb_ExitLowPowerPopWarning(void);
void DialogCb_ExitLowNetsignalPopWarning(void);

#endif


