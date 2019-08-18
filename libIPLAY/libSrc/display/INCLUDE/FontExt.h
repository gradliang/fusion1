
#ifndef FONT_FORMAT_EXT_H_8FB617BC2099BEF3A6E2B586DF6D9C84
#define FONT_FORMAT_EXT_H_8FB617BC2099BEF3A6E2B586DF6D9C84

#include "utiltypedef.h"
#include "iPlaySysconfig.h"

#if EXT_FONT_FORMAT_ENABLE

#define FONT_LIBSANS_17				0
#define FONT_LIBSANS_17_BOLD		1
#define FONT_LIBSANS_20_BOLD		2

struct _tagFontLoadInfo;
typedef struct _tagFontListItem
{
	struct _tagFontLoadInfo* handle;
	unsigned char *		strDataFileName;
	unsigned char *		strTableFileName;
	unsigned char		bFontId;
	unsigned char		bDriveId;
	unsigned char		bDefaultChar;
	unsigned char		bFlag;
} FONT_LIST_ITEM;

typedef struct _tagFontLoadInfo
{
	unsigned char *		pbTabBuff;
	unsigned int *		pdwDataBuff;
	STREAM*				fpData;
	unsigned int		dwDataFileSize;
	unsigned int		dwCacheCnt;
	const FONT_LIST_ITEM* pstListItem;
	BYTE				bAreaCnt;					// the font tab area count
} ST_FONT_LOAD_INFO;
typedef ST_FONT_LOAD_INFO* FONT_HANDLE;

int	OpenFont(BYTE font_id);
int CloseFont(BYTE font_id);
void CloseAllFont(void);

DWORD TextOut(ST_IMGWIN* pWin, BYTE font_id, const BYTE* lpString, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth);
DWORD TextOutW(ST_IMGWIN* pWin, BYTE font_id, const WORD* lwString, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth, BOOL boLittleEndian);
DWORD TextOutUtf8(ST_IMGWIN* pWin, BYTE font_id, const BYTE* u8string, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth);
DWORD GetTextExtent(BYTE font_id, const BYTE* lpString, DWORD* pdwWidth, DWORD* pdwHeight);
DWORD GetTextExtentW(BYTE font_id, const WORD* lpwString, DWORD* pdwWidth, DWORD* pdwHeight, BOOL boLittleEndian);
DWORD GetFontHeight(BYTE font_id);
DWORD GetFontAveWidth(BYTE font_id);

void SetCurrFont(BYTE font_id);
BYTE GetCurrFont();
DWORD TextOutString(ST_IMGWIN* pWin, const BYTE* lpString, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth, BOOL boUnicode);
DWORD GetStringWidth(const BYTE* pbString, BOOL boUnicode);

BOOL isBlank(WORD unicode_char);
BOOL isFullBlank(WORD unicode_char);
BOOL isTabBlank(WORD unicode_char);

#if Support_EPD
DWORD EPD_TextOut(BYTE* Y_data, DWORD dwYDataWidth, BYTE font_id, const BYTE* lpString, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth);
DWORD EPD_TextOutW(BYTE* Y_data, DWORD dwYDataWidth, BYTE font_id, const WORD* lpwString, DWORD nXStart, DWORD nYStart, 
				DWORD dwMaxWidth, BOOL boLittleEndian);
#endif

extern BYTE  g_bFont;

#endif	//EXT_FONT_FORMAT_ENABLE
#endif	//FONT_FORMAT_EXT_H_8FB617BC2099BEF3A6E2B586DF6D9C84


