//---------------------------------------------------------------------------

#ifndef xpgSpriteH
#define xpgSpriteH

#include "xpgType.h"
#include "xpgRole.h"
#include "xpgSpriteFrame.h"

#if defined(__cplusplus)
extern "C" {
#endif


//---------------------------------------------------------------------------
/*
Layer	2	0	Zbuffer Depth
X	2	2	
Y	2	4	
Image Index	2	6	
ink	1	8	
ink value	3	9	
Hash Key	4	12	
*/
///@ingroup     xpgSprite
///@brief       PageSprite struct
typedef struct {
	
    WORD m_wLayer;  ///< Layer
	WORD m_wRole;     ///< Role index
    WORD m_wPx;     ///< Position of X
    WORD m_wPy;  	  ///< Position of Y
	DWORD m_dwInk;    ///< Ink
	DWORD m_dwType;   ///< Type
    DWORD m_dwHashKey;///< Hash Key
    WORD m_wTextLen;
    WORD REV;
    char *m_Text;
    
    int m_iFrameCount;
    int m_iKeyFrameCount;

    xpg_sprite_frame_t *m_astFrame; // list of animation frames
} STXPGPAGESPRITE;   

typedef struct {
    WORD m_dwType;      ///< Type
    WORD m_dwTypeIndex; ///< Type Index
    WORD m_bFlag;
    WORD m_noused;
}XPGEXTRASPRITE;

typedef struct {
    WORD m_touchAreaX;
    WORD m_touchAreaY;
    WORD m_touchAreaW;
    WORD m_touchAreaH;
    WORD m_boEnable;
} STXPGTOUCHINFO;

///@ingroup     xpgSprite
///@brief       Sprite struct    
typedef struct {
	
    WORD m_wLayer;      ///< Layer
	WORD m_wRole;         ///< Role
    WORD m_wPx;         ///< Position of X
    WORD m_wPy;		      ///< Position of Y
	DWORD m_dwInk;        ///< Ink
    DWORD m_dwHashKey;  ///< Hash Key
    WORD m_dwType;      ///< Type
    WORD m_dwTypeIndex; ///< Type Index
	
	STXPGROLE *m_pstRole;	///< pointer to Role
	  
	// position and region
	WORD m_wWidth;       ///< Width
	WORD m_wHeight;      ///< Height
	WORD m_wLeft;        ///< Left
	WORD m_wRight;       ///< Right
	WORD m_wTop;         ///< Top
	WORD m_wBottom;      ///< Bottom
	WORD m_wAngle;
	WORD m_wTextLen;
    char *m_pText;
    
    BYTE m_bBlend;
    BYTE m_bScaleW;
    BYTE m_bScaleH;
	BYTE m_bReserved1; //
	BYTE *m_pImage;      ///< pointer to Image
	
	WORD m_wFrameCount;
	WORD m_wKeyFrameCount;
	WORD m_wCurFrame;
	SWORD m_iGotoFrame;
	
	SWORD m_iGotoRepeat;
	SWORD m_iGotoRepeated;
	SWORD m_iCurLoopFrame;
	SWORD m_iCurLoopGoto;
	
    xpg_sprite_frame_t *m_astFrame; // list of animation frames
    
    //BYTE m_bType;    
    XPGBOOL m_boExist;   ///< boolean of Exist
	XPGBOOL m_boChanged;   ///< boolean of Changed
    XPGBOOL m_boVisible; ///< boolean of Visible
	BYTE 	m_bFlag;         ///< Flag

	//BYTE m_bReserved[2];
	STXPGTOUCHINFO  m_touchInfo;

} STXPGSPRITE;

#include "xpgMovie.h"

void xpgSpriteInit	 ( STXPGSPRITE *pstSprite );
void xpgSpriteRelease( STXPGSPRITE *pstSprite );

void xpgSpriteSetRole( STXPGSPRITE *pstSprite, STXPGROLE *pstRole );
void xpgSpriteMoveTo ( STXPGSPRITE *pstSprite, DWORD x, DWORD y );
void xpgSpriteMove   ( STXPGSPRITE *pstSprite, DWORD x, DWORD y );
void xpgSpriteCopy	 ( register STXPGMOVIE *pstMovie, STXPGSPRITE *pstDst, STXPGPAGESPRITE *pstSrc );

void xpgExtraSpriteCopy(STXPGSPRITE * pstDst, XPGEXTRASPRITE * pstSrc);
DWORD getCurDialogExtraSpriteCount();
XPGEXTRASPRITE* getCurDialogExtraSpriteList();

int xpgAddDialog(int dialogId, char * backToPage, ST_IMGWIN* backupWin);
int xpgDeleteDialog();
int xpgAddDialogSprite(WORD m_dwType, WORD m_dwTypeIndex, BYTE flag);
int xpgGetCurrDialogTypeId();
ST_IMGWIN* xpgGetCurrDialogCacheWin();
char* xpgGetCurrDialogBackPage();

void xpgSpriteSetTouchArea (STXPGSPRITE * pstSprite, WORD startX, WORD startY, WORD width, WORD height);
void xpgSpriteEnableTouch(STXPGSPRITE * pstSprite);
void xpgSpriteDisableTouch(STXPGSPRITE * pstSprite);


#if defined(__cplusplus)
 }
#endif

#endif
