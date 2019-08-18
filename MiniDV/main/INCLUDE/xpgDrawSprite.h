#include "xpgFunc.h"
//#include "xpgString.h"
#include "mpTrace.h"
#include "global612.h"
#include "xpg.h"
#include "os.h"
#include "display.h"
#include "ui.h"
#include "filebrowser.h"



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




