//---------------------------------------------------------------------------
#ifndef xpgMovieH
#define xpgMovieH

#include "xpgDefine.h"
#include "xpgType.h"
#include "xpgRole.h"
#include "xpgSprite.h"
#include "xpgPage.h"
//#include "display.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define XPG_ICON_FLAG_EXIST 1
//----------------------------------------------------------------------------
#define SlideShowMusicSet		0x01

///@ingroup     xpgMovie
///@brief       Movie Action struct
typedef struct {
	BYTE bOpMode;        ///< Opmode
	BYTE bPageMode;      ///< PageMode
	BYTE bStatusFlag;    ///< StatusFlag
	BYTE bIconFlag;      ///< last bit for hourglass show or not
#if XPG_STD_DPF_GUI 
    void (*hCommand)(DWORD);///< Command hook (dwKeyCode)
#else	
    void (*hCommand)(void);///< Command hook
#endif    
} STACTFUNC;

///@ingroup     xpgMovie
///@brief       Image Info
typedef struct {
    DWORD m_dwHashKey;	///< HashKey
    BYTE *m_pImage;     ///< pointer of Image
    WORD m_wImgWidth;   ///< Image Width
    WORD m_wImgHight;   ///< Image Height
} STIMGINFO;

///@ingroup     xpgMovie
///@brief       Icon struct
typedef struct {
    BYTE m_bID;                       ///< Icon ID
    BYTE m_bIndex;                    ///< Icon Index
    BYTE m_bTypeIndex;                ///< IconType Index
    BYTE m_bFlag;                     ///< Flag
    //DWORD m_dwHashKey;
    STXPGSPRITE *m_pstSpriteEnable;   ///< pointer to Sprite Enable 
    STXPGSPRITE *m_pstSpriteSelected; ///< pointer toSprite Selected
} STXPGICON;

///@ingroup     xpgMovie
///@brief       ScrollBar struct
typedef struct {
    BYTE m_bID;              ///< ID
    BYTE m_bIndex;           ///< Index
    BYTE m_bTypeIndex;       ///< TypeIndex
    BYTE m_bFlag;            ///< Flag
    STXPGSPRITE *m_pstBar;   ///< pointer to Bar
    STXPGSPRITE *m_pstButton;///< pointer to BUtton	
} STXPGSCROLLBAR;

///@ingroup     xpgMovie
///@brief       Thumb struct
typedef struct {
    DWORD m_dwHashKey;       ///< Hash Key
    STIMGINFO m_astImg;      ///< Image info
    STXPGSPRITE *m_pstSprite;///< pointer to Sprite   
} STXPGTHUMB;

///@ingroup     xpgMovie
///@brief       ThumbFrame struct
typedef struct {
    SWORD   m_swOffsetX;      ///< OffsetX
    SWORD   m_swOffsetY;      ///< OffsetY
    WORD    m_wCurIndex;      ///< CurrentIndex
    WORD    m_wColumn;        ///< Column
    STXPGSPRITE *m_pstSprite; ///< pointer to Sprite  
} STXPGTHUMBFRAME;

///@ingroup     xpgMovie
///@brief       ListFrame struct
typedef struct {
    SWORD   m_swOffsetX;     ///< Offset X
    SWORD   m_swOffsetY;     ///< Offset Y
    DWORD   m_dwCurIndex;    ///< CurrentIndex
    STXPGSPRITE *m_pstSprite;///< pointer to Sprite
} STXPGLISTFRAME;

#define XPG_FILE_NAME_BUFFER_SIZE 192//64	//Lighter changed 0207

///@ingroup     xpgMovie
///@brief       List struct
typedef struct {
    DWORD m_dwHashKey;         ///< Hash Key
    STXPGSPRITE *m_pstSprite;  ///< pointer to Sprite
	STXPGSPRITE *m_pstListIcon;	 ///< pointer to ListIcon
    //DWORD m_dwTotalTime;
	BYTE m_bFlag;                ///< Flag
	BYTE m_bFileType;            ///< FileType
	BYTE m_bStateFlag;		       ///< StateFlag, Bit 0: the Slideshow Music State
    BYTE m_bReserved;          ///< Reserved
    BYTE m_pbTime[8];	         ///< Time[8]
	char m_pbInfo[32];           ///< Info[32]
	//char m_bTitle[32];
    BYTE m_pbName[XPG_FILE_NAME_BUFFER_SIZE]; ///< Name    
    BYTE m_pbExtension[5];     ///< Extension[5], .xxx
    BYTE Rev[3];               ///< Reserved

} STXPGLIST;
//----------------------------------------------------------------------------
/*
XPG File Header
    name	length	Position	Description
    xpg	                4	0
    version	            4	4	1010 = v1.01
    Company name	    16	8	magicpixel
    Editor name	        16	24	"Magicbuilder, xpgConverter"
    Screen Width	    2	40
    Screen Height	    2	42
    Background Color 	6	44	RGB
    Transparent Color	6	50
    Maximun SpriteLayer	2	56
    Sprite Data Length	2	58
    Reserve	            16	60
    Image Number	    2	76
    Image Header Len	2	78
    Image Header Pos	4	80
    Page Number	        2	84
    Page Header Len	    2	86
    Page Header Pos	    4	88
    Script Number	    2	92
    Script Header Len	2	94
    Script Header Pos	4	96
*/
//------------------------------------------------------------------------------
///@ingroup     xpgMovie
///@brief       Movie struct
typedef struct {
	DWORD m_dwVersion;           ///< Version
 	DWORD m_dwFileSize;          ///< FileSize
    DWORD m_dwBkgColor;        ///< Background Color , 44,6
    DWORD m_dwTransColor;      ///< Transparent Color, 50,6
    
    WORD m_wScreenWidth;      ///< Screen Width, 40,2
    WORD m_wScreenHeight;     ///< Screen Height, 42,2

    WORD m_wMaxSprites;       ///< Maximun Sprite Layer, 56,2
    WORD m_wSpriteDataLen;    ///< Sprite Data Length, 58,2
    //BYTE  m_bReserve[16];      //Reserve, 60,16

    DWORD m_dwImageHeaderPos;  ///< Image Header  Position, 80,4
    DWORD m_dwPageHeaderPos;   ///< Page Header Position, 88,4
    
    WORD m_wRoleCount;        ///< Image Number, 76,2
    WORD m_wImageHeaderLen;   ///< Image Header Length, 78,2    
    WORD m_wPageCount;        ///< Page Number, 84,2 
    WORD m_wPageHeaderLen;    ///< Page Header Length, 86,2
    
    WORD m_wScriptCount;      ///< Script Number, 92,2
    WORD m_wScriptHeaderLen;  ///< Script Header Length, 94,2
    DWORD m_dwScriptHeaderPos; ///< Script Header Position, 96,4
	STXPGPAGE *m_pstPrevPage;    ///< pointer to Previous Page
    STXPGPAGE *m_pstCurPage; 	 ///< pointer to Current Page
    
    BYTE  m_dwSpriteCount;		///< total sprite count in a  single page 
    BYTE  m_dwThumbColumns;   ///< ThumbColumns
	BYTE  m_dwCmdCount;	        ///< Command Count
	BYTE  m_dwGlobalCmdCount;   ///< Global Command Count

    BYTE   m_dwListCount;     ///< this if for how many  bars in single page ex:6 for photo , 5 for BT,video , audio, if in show list page  
    BYTE   m_dwThumbCount;    ///< ThumbCount
    BYTE   m_dwCardCount;     ///< CardCount
	BYTE   m_wAniCount;         ///< Ani Count

	BYTE 	m_boPopMenu;           ///< boolean of PopMenu
	BYTE	m_bMenuItemCount;      ///< MenuItemCount
	BYTE	m_bMenuItemIndex;      ///< MenuItemIndex
	BYTE 	m_bIconAni;            ///< IconAni

	BYTE  m_wCurPage;            /// Curren Page
	BYTE  m_wPrevPage;           /// Previous Page
	BYTE  m_bUpdateStage;        /// boolean of UpdateStage
	BYTE  m_boClip;              /// boolean of Clip

	WORD  m_wMenuLeft;           /// MenuLeft
	WORD  m_wMenuTop;            /// MenuTop
	WORD  m_wMenuRight;          /// MenuRight
	WORD  m_wMenuBottom;         /// MenuBottom
	
    SWORD m_wFrameX;            ///< FrameX
    SWORD m_wFrameY;            ///< FrameY
    XPGFILE  *m_pFileHandle;    ///< FileHandle
    XPGPIXEL *m_pCanvas;        ///< pointer to Canvas

    DWORD   m_dwThumbBufferCount; ///< ThumbBufferCount
    DWORD   m_dwThumbBufferSize;  ///< ThumbBufferSize

    STXPGSPRITE *m_pstListFrame;  ///< pointer to ListFrame
    STXPGSPRITE *m_pstThumbFrame; ///< pointer to ThumbFrame
    STXPGSPRITE *m_pstButtonFrame;	///< pointer to ButtonFrame
#if XPG_SCROLLBAR_COUNT
    STXPGSPRITE *m_pstScrollBar[XPG_SCROLLBAR_COUNT]; ///< pointer to ScrollBar
    STXPGSPRITE *m_pstScrollButton[XPG_SCROLLBAR_COUNT]; ///< pointer to ScrollButton
#endif
	STXPGSPRITE *m_pstAniSprite[XPG_ICONANI_COUNT]; ///< pointer to Ani Sprite
		
	WORD   	m_dwCommand[XPG_COMMAND_COUNT + XPG_GLOBAL_CMD_COUNT][6];  ///  one cmd include 6 reference
    STXPGROLE 	*m_astRole; ///< role count defined by xpg file
    STXPGPAGE 	*m_astPage; ///< page count defined by xpg file
	STXPGSPRITE m_astSprite[XPG_SPRITE_COUNT]; ///< all sprite object in a page 
	
	STXPGROLE 	**m_pstObjRole; ///< obj role count defined by xpg file
	//STXPGICON 	m_astCardIcon[XPG_CARD_COUNT]; ///< CardIcon array	
#if XPG_THUMB_COUNT
    STXPGTHUMB  m_astThumb[XPG_THUMB_COUNT]; ///< Thumb array
#endif
#if XPG_LIST_COUNT
	STXPGLIST   m_astList[XPG_LIST_COUNT];  ///< this is the list sprite only
#endif
    //BYTE  m_bThumbName[XPG_THUMB_COUNT][12];
    //BYTE  *m_pThumbImage[XPG_THUMB_COUNT];
} STXPGMOVIE;

//---------------------------------------------------------------------------
// Define at xpgReader.c
//---------------------------------------------------------------------------
XPGBOOL xpgReadMovieHeader( register STXPGMOVIE *pstMovie, XPGFILE *fp );
XPGBOOL xpgReadRoleHeader ( register STXPGMOVIE *pstMovie, XPGFILE *fp, STXPGROLE *pstRole );
XPGBOOL xpgReadRoleData   ( register STXPGMOVIE *pstMovie, XPGFILE *fp, STXPGROLE *pstRole, WORD iRole );
XPGBOOL xpgReadPageHeader ( register STXPGMOVIE *pstMovie, XPGFILE *fp, STXPGPAGE *pstPage );
XPGBOOL xpgReadSpriteData ( register STXPGMOVIE *pstMovie, XPGFILE *fp, STXPGPAGESPRITE *pstSprite );

#ifdef XPG_LOAD_FROM_FILE
XPGBOOL xpgLoadMovie( register STXPGMOVIE *pstMovie, const char *filename );
#else
XPGBOOL xpgLoadMovie( register STXPGMOVIE *pstMovie, DWORD *pdwBuffer, DWORD dwSize );
#endif
XPGBOOL xpgReadMovie( register STXPGMOVIE *pstMovie );
//---------------------------------------------------------------------------

void xpgMovieInit       ( register STXPGMOVIE *pstMovie );
void xpgMovieClear      ( register STXPGMOVIE *pstMovie );
void xpgMovieLoadPage   ( register STXPGMOVIE *pstMovie, DWORD dwPage );
void xpgMovieLoadSecondPage   ( register STXPGMOVIE *pstMovie, DWORD dwPage );
void xpgMovieUnLoadSecondPage   ( register STXPGMOVIE *pstMovie, DWORD dwPage );

void xpgMovieInitObjRole( register STXPGMOVIE *pstMovie );
void xpgMovieLoadGlobalCommand( register STXPGMOVIE *pstMovie );
void xpgMovieLoadPageCommand( register STXPGMOVIE *pstMovie, int iPage );

STXPGSPRITE *xpgSpriteFindType( register STXPGMOVIE *pstMovie, DWORD dwType, DWORD dwIndex );

STXPGPAGE *xpgMovieSearchPage(const char *name);
STXPGSPRITE *xpgMovieSearchSprite(const char *name, DWORD len);
//-----------------------------------------------------------------------------

#if defined(__cplusplus)
 }
#endif


//---------------------------------------------------------------------------
#endif
