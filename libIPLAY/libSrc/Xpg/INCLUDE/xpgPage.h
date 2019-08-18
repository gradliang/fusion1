//---------------------------------------------------------------------------

#ifndef xpgPageH
#define xpgPageH
//---------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" {
#endif

#include "xpgDefine.h"
#include "xpgSprite.h"

///@ingroup     xpgPage
///@brief       Page struct
typedef struct {
	
    //DWORD  m_dwFilePos;
    DWORD  m_dwSpriteFilePos;   ///< Sprite File Position
	DWORD  m_dwScriptFilePos;     ///< Script File Position
    DWORD  m_dwTimeLength;      ///< Time Length
    DWORD  m_dwHashKey;         ///< Hash Key
    WORD   m_wIndex;             ///< Page index    
    WORD   m_wSpriteCount;       ///< channel object number
    BYTE  m_bPageMode;          ///< Page Mode
	BYTE  m_bPageFlag;            ///< Page Flag
    
    STXPGPAGESPRITE *m_astSprite; ///< pointer to Sprite
    
    //WORD  m_dwCommand[XPG_COMMAND_COUNT][6];  // add key event and action param - 07.24.2006 Athena
    WORD *m_wCommand;           ///< commands defined by xpg file 
    DWORD  m_dwCmdCount;        ///< command count
    
    XPGBOOL  m_boGoNext;        ///< boolean of Go Next
    XPGBOOL  m_boLoaded;        ///< boolean of Loaded
	WORD	 m_wEnterPage;          ///< Enter Page
    BYTE 	 m_bReserved[2]; // have to make sure align 4
} STXPGPAGE;

#include "xpgMovie.h"

void xpgPageInit	( STXPGPAGE *pstPage );
void xpgPageRelease	( STXPGPAGE *pstPage );
void xpgPageClear	( STXPGPAGE *pstPage );

						  
STXPGPAGESPRITE *xpgGetSprite	( STXPGPAGE *pstPage, DWORD iSprite );


//---------------------------------------------------------------------------
#if defined(__cplusplus)
 }
#endif

#endif
