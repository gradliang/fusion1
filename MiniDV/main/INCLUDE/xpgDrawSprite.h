#include "xpgFunc.h"
//#include "xpgString.h"
#include "mpTrace.h"
#include "global612.h"
#include "xpg.h"
#include "os.h"
#include "display.h"
#include "ui.h"
#include "filebrowser.h"
#include "Setup.h"

#ifndef XPGDRAWSPRITE_H
#define XPGDRAWSPRITE_H




#define DIALOG_DEFAULT_WIDTH						352
#define DIALOG_DEFAULT_HEIGHT					180


extern MODEPARAM tempModeParam;
extern DWORD dwDialogTempValue;
extern BOOL  boDialogValueIsFloat;
extern char strEditPassword[8];
extern char strEditValue[32];
extern DWORD * pdwEditingFusionValue;
extern BOOL isSelectOnlineOPM;
extern WORD g_wElectrodeRandomCode;


SWORD xpgDrawSprite_Null				  ( ST_IMGWIN *, STXPGSPRITE *, BOOL );
SWORD xpgDrawSprite						    ( ST_IMGWIN *, STXPGSPRITE *, BOOL ); // type0
SWORD xpgDrawSprite_TextWhiteLeft			  (ST_IMGWIN * , STXPGSPRITE *, BOOL ); // type1
SWORD xpgDrawSprite_TextBlackLeft			    ( ST_IMGWIN *, STXPGSPRITE *, BOOL );// type2
SWORD xpgDrawSprite_TextBlackCenter(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);//// type4
SWORD xpgDrawSprite_HilightFrame(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);// type5
SWORD xpgDrawSprite_ModeIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);//type6
SWORD xpgDrawSprite_FileView(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);//type7
SWORD xpgDrawSprite_Background(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip); //type11
SWORD xpgDrawSprite_Icon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);          // type12
SWORD xpgDrawSprite_LightIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);     // type13
SWORD xpgDrawSprite_DarkIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);      // type14
SWORD xpgDrawSprite_Mask(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);          // type15
SWORD xpgDrawSprite_Title(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);         // type16
SWORD xpgDrawSprite_Aside(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);         // type17
SWORD xpgDrawSprite_BackIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);      // type18
SWORD xpgDrawSprite_CloseIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);     // type19
SWORD xpgDrawSprite_Text(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);          // type20
SWORD xpgDrawSprite_Selector(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);      // type21
SWORD xpgDrawSprite_List(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);          // type22
SWORD xpgDrawSprite_Dialog(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);        // type23
SWORD xpgDrawSprite_Status(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);        // type24
SWORD xpgDrawSprite_Radio(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);         // type25
SWORD xpgDrawSprite_Scroll(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);        // type26
SWORD xpgDrawSprite_Frame(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);         // type27
SWORD xpgDrawSprite_RepeatIcon(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);          // type29
SWORD xpgDrawSprite_HomeStatus(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boClip);    // type30


int popupDialog(int dialogType, char * backToPage, ST_IMGWIN* pWin_Background);
void exitDialog();


typedef struct _OPMDATAITEM{
    char    itemName[64];
    WORD    year;
    BYTE    month;
    BYTE    day;
    BYTE    hour;
    BYTE    minute;
    BYTE    second;
    DWORD   nm;
    int     db1;
    int     db2;
}OPMDATAITEM;

extern DWORD opmLocalDataCurrentPage;
extern DWORD opmCloudDataCurrentPage;
extern DWORD opmLocalDataTotal;
extern DWORD opmCloudDataTotal;
extern OPMDATAITEM * localOpmData;
extern OPMDATAITEM * cloudOpmData;

//////////////////////////////////////////////////////
#define KEYBOARD_BUFFER_SIZE    128
extern char keyboardBuffer[];
extern const char * keyboardTitle;
extern BOOL boCapsLock;
extern BOOL boKeyLight;
extern DWORD dwKeyID;
void (*keyboardGoBack)(BOOL boEnter);




#endif // XPGDRAWSPRITE_H


