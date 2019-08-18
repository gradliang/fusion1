

#ifndef FONT_EXT_INSIDE_H_EC3C69208B9BF499
#define FONT_EXT_INSIDE_H_EC3C69208B9BF499

#include "utiltypedef.h"
#include "iPlaySysconfig.h"

#define FONT_FLAG_LOAD_ALL			BIT0
#define FONT_FLAG_LOAD_TAB_ONLY		BIT1

#define FONT_CACHE_BUF_SIZE			(4*1024)

struct tagFontTabHeaderExt
{
	unsigned int   dwFontTag;      // This font Tag for double check with code
	unsigned short codepage;       // codepage
	unsigned short wFixSize;       // font data size of every fix-width characters, only bTableStyle==0 valid
	unsigned char  bMaxHeight;     // This font data maximum Height
	unsigned char  bMaxWidth;      // This font data maximum Width
	unsigned char  bWithWidth;     // This Font data first byte with width, 0: for do not have, 1: for had
	unsigned char  bBitsPerPixel;  // Every Pixel used bit number on this font data
	unsigned char  bAlign;         // Every character data align byte number
	unsigned char  bTableStyle;    // This Font table style, 0: for index (WORD), 1: for offset (DWORD), 2: offset (WORD)
	unsigned char  bFontSize;      // font-data size of you choosed
	unsigned char  bDpi;           // The font dpi
	unsigned short wMaxDataSize;   // the max font data size (in bytes)
	unsigned char  tmAveWidth;	   // Specifies the average width of characters in the font (generally defined as the width of the letter x
	unsigned char  tmHeight;	   // Specifies the height (ascent + descent) of characters.
	unsigned char  tmAscent;	   // Specifies the ascent (units above the base line) of characters.
	unsigned char  tmDescent;	   // Specifies the descent (units below the base line) of characters.
	unsigned char  tmInternalLeading;//Specifies the amount of leading (space) inside the bounds set by the tmHeight member. Accent marks and other diacritical characters may occur in this area. The designer may set this member to zero.
	unsigned char  tmExternalLeading;//Specifies the amount of extra leading (space) that the application adds between rows. Since this area is outside the font, it contains no marks and is not altered by text output calls in either OPAQUE or TRANSPARENT mode. The designer may set this member to zero.
	unsigned char  reserve[8];
};
typedef struct tagFontTabHeaderExt ST_TAB_HEADER_EXT;

typedef struct
{
	unsigned char	bWidth;			// current character font-cell width
	unsigned char	bHeight;		// current character font-cell height
	signed char		cornerX;		// font-cell's upper left corner X
	signed char		cornerY;		// font-cell's upper left corner Y
	signed char		offsetX;		// next character x offset
	signed char		offsetY;		// next character y offset
}
ST_FONT_CELL_DESC;

typedef struct{
	DWORD		dwX;				//
	DWORD		dwY;				//
	DWORD		dwMaxWidth;			//
	DWORD		dwDisplayWidth;		//
	DWORD		dwStrLength;		//
	BYTE    	bFontColor;			// font color. image: Y, OSD: index
	BYTE    	bFontCb;
	BYTE    	bFontCr;
	BYTE		bDefChar;
	ST_IMGWIN*	pWin;
	//----------------
	DWORD		dwAreaCnt;			//
	const BYTE*	pbAreaMap;			//
	const DWORD*pdwOffsets;			//
	const WORD*	pwCellIdx;			//
	const BYTE* pbCellInfo;			//
} ST_FONT_EX;

//------- the static function declare --------
static void _clear_load_info(ST_FONT_LOAD_INFO * pstInfo);
static void InitFontExt(ST_FONT_EX* pstFt, ST_IMGWIN* pWin, FONT_HANDLE handle, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth);
static void LockFontDataBuffer(FONT_HANDLE handle);
static void UnLockFontDataBuffer(FONT_HANDLE handle);
static DWORD GetFontDataPosition(ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch);
static BYTE getByte(FONT_HANDLE handle, DWORD pointer);
static BYTE ReadBits(FONT_HANDLE handle, const ST_TAB_HEADER_EXT *pstHeader);
static void DrawFontCell(FONT_HANDLE handle, ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch);
static void CalculateFontExtent(FONT_HANDLE handle, ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch);
static void Mix2Point(DWORD* pdwDisplayBuf, BYTE level0, BYTE level1, ST_FONT_EX* pfnt);
static BYTE SpreadLevel(BYTE level, BYTE bBitsPerPixel);
static BYTE SpreadLevel_1bit(BYTE level);
static BYTE SpreadLevel_4bit(BYTE level);
static WORD Uni2Ascii(WORD uniChar, BYTE bDefChar);
#ifdef Support_EPD
static void EPD_DrawFontCell(FONT_HANDLE handle, DWORD dwYDataWidth, ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch);
#endif

extern DWORD g_dwSystemFontColor;

#endif	//FONT_EXT_INSIDE_H_EC3C69208B9BF499


