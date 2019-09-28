//---------------------------------------------------------------------------

#ifndef xpg__H__
#define xpg__H__

#define XPG_UI

#include "global612.h"
#include "xpgDefine.h"
#include "xpgType.h"
#include "xpgUtil.h"
#include "xpgRole.h"
#include "xpgSprite.h"
#include "xpgPage.h"
#include "xpgMovie.h"
#include "fs.h"

#if MAKE_XPG_PLAYER
    #define XPG_ENABLE          1
#else
    #define XPG_ENABLE          0
#endif

#if defined(__cplusplus)
extern "C" {
#endif

enum {
    MUSIC_PLAY_MODE = 0,
    VIDEO_PLAY_MODE,
    YOUTUBE_PLAY_MODE,
    SLIDE_PLAY_MODE,
    NUM_OF_PLAY_MODE
};

#if (DISPLAY_VIDEO_DROP_INFO_FULL || DISPLAY_VIDEO_DROP_INFO_THUMB)
typedef struct
{
   char cVidoeCodec[6];
   char cAudioCodec[6];
   int iTotalFrames;
   int iMissedFrame;
   int iPlayedFrame;
} S_VIDEO_DROP_INFO_T;
#endif

typedef struct
{
    void (*xpgUpdateEQInfo)(DWORD dwTotal, DWORD dwSec);
    void (*xpgInitOsdTimeBar)(void);
    void (*xpgUpdateOsdTimeBar)(DWORD dwTotal, DWORD wValue);
    void (*xpgUpdateTimeBar)(DWORD dwTotal, DWORD wValue);
    //void (*xpgPopupError)(int ErrCode);
    void (*xpgOsdPopupError)(int ErrCode);

#if (DISPLAY_VIDEO_DROP_INFO_FULL || DISPLAY_VIDEO_DROP_INFO_THUMB)
    void (*xpgDispVideoDropInfo)(S_VIDEO_DROP_INFO_T sDropInfo);
#endif

#if DISPLAY_VIDEO_SUBTITLE
    void (*xpgUpdateOsdSRT)(BYTE* p_srt);
#endif

#if LYRIC_ENABLE
    void (*xpgUpdateOsdLrc)(BYTE* p_Lrc, WORD x, WORD y);
    void (*xpgLyricUIControl)(BYTE type, char * pagename, DWORD param);
#endif
} XPG_FUNC_PTR;



extern BYTE *g_xpgTempBuffer;
extern BYTE *xpgCanvasBuffer;
extern STXPGMOVIE *g_pstXpgMovie;
extern STXPGMOVIE g_stXpgMovie;
extern ST_IMGWIN g_sXpgCanvasWin;
extern ST_IMGWIN *g_pXpgCanvasWin;
extern ST_IMGWIN g_sXpgBuffWin;
extern ST_IMGWIN *g_pXpgBuffWin;
extern BYTE g_boNeedRepaint;
extern BOOL g_isDialogPage;

DWORD xpgGetXpgMemorySize(); // get final XPG Memory need size

BOOL xpgPreloadAndOpen(STREAM *sFileHandle);
BOOL xpgLoadFromFlash(DWORD dwTag);
#ifdef XPG_LOAD_FROM_FILE
  DWORD xpgOpen(const char *filename);
#else
  DWORD xpgOpen(DWORD *pbBuffer, DWORD dwSize);
#endif

SWORD xpgGotoPage(DWORD iPage);
WORD  xpgGotoEnterPage(void);
STXPGPAGE *xpgSearchAndGotoPage(const char *name);
void  xpgUpdateStage(void);
#if VIDEO_ON
void  xpgClearStage(void);
#endif
//void  xpgStepFrame(void);
void  xpgInitCanvas(void);
void  xpgUpdateCanvas(void);

XPGBOOL xpgInitThumbBuffer(DWORD dwCount, DWORD dwBufferSize);

void xpgTestFunction(void);
void xpgReleaseAllRoleBuffer();
void xpgReleaseThumbBuffer();

void xpgUpdateStageClip(ST_IMGWIN *, WORD, WORD, WORD, WORD, BOOL);
void xpgUpdateStageClipBySprite(ST_IMGWIN *, STXPGSPRITE *, BOOL);
void xpgDirectDrawRoleOnWin( ST_IMGWIN *pWin, STXPGROLE *pstRole, DWORD x, DWORD y, STXPGSPRITE *pstSprite, BOOL boClip );
void SetNotToDrawHourGlass(BYTE val);
void xpgUpdateStage();
ST_IMGWIN *xpgGetCanvasWin(void);

void xpgUiActionFunctionRegister(STACTFUNC *xpgActionFunctionsTable);
void xpgUiDrawKeyIconFunctionRegister(void *funcPtr);

BOOL xpgSupportAnimationCheck(void);

//---------------------------------------------------------------------------
#if defined(__cplusplus)
 }
#endif

#endif
