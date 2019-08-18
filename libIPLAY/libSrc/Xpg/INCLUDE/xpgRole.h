//---------------------------------------------------------------------------

#ifndef xpgRoleH
#define xpgRoleH
//---------------------------------------------------------------------------
#include "xpgType.h"
#include "Display.h"
#if defined(__cplusplus)
extern "C" {
#endif
/*
format				1	0	jpg, png, gif¡K. Or other flag
Pixel depth			1	1	
Pixel type			4	2	444, 555, 888¡K
width				2	6	
height				2	8	
Image Data Length	4	10	
Image Data Position	4	14	
Hash Key			4	18	for XPG object name
*/
///@ingroup     xpgRole
///@brief       Role struct
typedef struct {
	
    //DWORD m_bBpp;	
    //DWORD m_dwFormat;
    DWORD m_dwFilePos;       ///< File Position
    DWORD m_dwDataLen;       ///< Data length
	
  	WORD m_wWidth;           ///< Width
    WORD m_wHeight;	         ///< Height

	WORD m_wIndex;             ///< Index
	WORD m_wRawWidth;          ///< Raw Width
	
	BYTE *m_pImage;            ///< pointer to Role Image
	BYTE *m_pMaskImage;
	BYTE *m_pRawImage;         ///< pointer to Role Raw Image
	
	ST_JPEG_TAG m_stJpegTag;   ///< Image Jpeg Tag
	
	BYTE  m_bBitDepth;         ///< Image Bit Depth
	BYTE  m_bImageType;        ///< Image Type
	XPGBOOL  m_boUsedAtThisPage;///< boolean Used At This Stage 
	BYTE 	 m_bType;            ///< Type
	//BYTE	 m_bReserved; 
    
    // v5.0.1.1 add role ox, oy
    WORD m_wOx;
    WORD m_wOy;	
  
} STXPGROLE;

#if XPG_ROLE_USE_CACHE 
#define ROLE_MAX 256
BYTE xpgRoleUseCache[ROLE_MAX];
DWORD *dwRoleCacheBuf[ROLE_MAX];
WORD wRoleCacheRawWidth[ROLE_MAX];
#endif

void xpgRoleInit( STXPGROLE *pstRole );
void xpgRoleRelease( STXPGROLE *pstRole );
void xpgRolePreload( STXPGROLE *pstRole );
void xpgRoleDraw( STXPGROLE *pstRole, void *pTarget, DWORD *pdwImageBuffer, DWORD px, DWORD py, DWORD dwScrWd, DWORD dwScrHt );
void xpgRoleDrawInClip( STXPGROLE *pstRole, ST_IMGWIN *pWin, DWORD *pdwImageBuffer, DWORD px, DWORD py);

void xpgRoleDecodeRLE( STXPGROLE *pstRole, void *pTarget );
void xpgRoleDrawRLE( STXPGROLE *pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd, DWORD dwScrHt );

#if defined(__cplusplus)
}
#endif
#endif
