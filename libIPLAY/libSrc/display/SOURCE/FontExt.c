

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#if EXT_FONT_FORMAT_ENABLE
#include "idu.h"
#include "display.h"
#include "charset.h"
#include "FontExt.h"
#include "FontExt_inside.h"

BYTE  g_bFont = 0;

static FONT_LIST_ITEM fontlist[] = 
{
	{ NULL, "YaHei18-Data.fnt",	"YaHei18-Unicode.tab",	EXT_FONT_YAHEI_18,	SYS_DRV_ID,	'?', FONT_FLAG_LOAD_TAB_ONLY },
};


int	OpenFont(BYTE font_id)
{
	DRIVE * drv;
	STREAM *fp;
	int i, iRet;
	DWORD dwFileSize;
	ST_FONT_LOAD_INFO stLoadInfo;
	FONT_HANDLE handle;
	FONT_LIST_ITEM* psItem;
	WORD uniName[128];

	if ( font_id >= sizeof(fontlist)/sizeof(FONT_LIST_ITEM) ) {
		mpDebugPrint("-E- font not define");
		return FAIL;
	}
	psItem = &fontlist[font_id];

	if ( psItem->handle ) {
		mpDebugPrint("-E- font have been open");
		return PASS;
	}
	psItem->handle = NULL;
	memset(&stLoadInfo, 0, sizeof(stLoadInfo));
	
	//---- Load Table file -----
	drv = DriveGet(psItem->bDriveId);
	DirReset(drv);
	DirFirst(drv);
	if ( drv == NULL ) {
		mpDebugPrint("fail get drive");
		return FAIL;
	}
	uniName[sizeof(uniName)/sizeof(WORD) - 1] = 0;
	for ( i = 0; i < sizeof(uniName)/sizeof(WORD) - 1; i++ ) {
		uniName[i] = psItem->strTableFileName[i];
		if ( uniName[i] == 0 )
			break;
	}
	iRet = FileSearchLN(drv, uniName, wcslen(uniName), E_FILE_TYPE);
	if ( iRet != FS_SUCCEED ) {
		mpDebugPrint("cannot find TAB %s", psItem->strTableFileName);
		return FAIL;
	}
	fp = FileOpen(drv);
	if ( fp == NULL ) {
		mpDebugPrint("cannot open file %s", psItem->strTableFileName);
		return FAIL;
	}
	dwFileSize = FileSizeGet(fp);
	stLoadInfo.pbTabBuff = (BYTE*) ext_mem_malloc(dwFileSize+4);
	if ( stLoadInfo.pbTabBuff == NULL ) {
		mpDebugPrint("-E- cann't alloc pbTabBuff");
		return FAIL;
	}
	if ( dwFileSize )
		FileRead(fp, stLoadInfo.pbTabBuff, dwFileSize);
	FileClose(fp);

	//---- Load font data file -----
	DirReset(drv);
	DirFirst(drv);
	for ( i = 0; i < sizeof(uniName)/sizeof(WORD) - 1; i++ ) {
		uniName[i] = psItem->strDataFileName[i];
		if ( uniName[i] == 0 )
			break;
	}
	iRet = FileSearchLN(drv, uniName, wcslen(uniName), E_FILE_TYPE);
	if ( iRet != FS_SUCCEED ) {
		_clear_load_info(&stLoadInfo);
		mpDebugPrint("cannot find DATA %s", psItem->strDataFileName);
		return FAIL;
	}
	fp = FileOpen(drv);
	if ( fp == NULL ) {
		_clear_load_info(&stLoadInfo);
		mpDebugPrint("cannot open file %s", psItem->strDataFileName);
		return FAIL;
	}
	dwFileSize = FileSizeGet(fp);
	stLoadInfo.dwDataFileSize = dwFileSize;
	if ( psItem->bFlag & FONT_FLAG_LOAD_ALL )
	{
		stLoadInfo.pdwDataBuff = (unsigned int*) ext_mem_malloc(dwFileSize+4);
		if ( stLoadInfo.pdwDataBuff == NULL ) {
			_clear_load_info(&stLoadInfo);
			mpDebugPrint("-E- cann't alloc pdwDataBuff");
			return FAIL;
		}
		if ( dwFileSize )
		FileRead(fp, (BYTE*) stLoadInfo.pdwDataBuff, dwFileSize);
		FileClose(fp);
	}
	else if ( psItem->bFlag & FONT_FLAG_LOAD_TAB_ONLY )
	{
		DWORD cnt;
		cnt = dwFileSize / FONT_CACHE_BUF_SIZE;
		if ( dwFileSize % FONT_CACHE_BUF_SIZE )
			cnt++;
		stLoadInfo.pdwDataBuff = (unsigned int*) ext_mem_malloc(cnt * sizeof(DWORD*) + 4);
		if ( stLoadInfo.pdwDataBuff == NULL ) {
			_clear_load_info(&stLoadInfo);
			mpDebugPrint("-E- cann't alloc pdwDataBuff");
			return FAIL;
		}
		memset(stLoadInfo.pdwDataBuff, 0, cnt * sizeof(DWORD*));
		stLoadInfo.fpData = fp;
		stLoadInfo.dwCacheCnt = cnt;
	}
	stLoadInfo.pstListItem = psItem;

	//---- get area count -----
	unsigned char* pbAreaMap = & stLoadInfo.pbTabBuff[sizeof(ST_TAB_HEADER_EXT)];
	stLoadInfo.bAreaCnt = 0;
	for ( i = 0; i < 256; i++ ) {
		if ( pbAreaMap[i] != 0xff )
			stLoadInfo.bAreaCnt ++;
	}
	
	//---- return ----
	handle = (FONT_HANDLE) ext_mem_malloc(sizeof(ST_FONT_LOAD_INFO));
	if ( handle == NULL ) {
		_clear_load_info(&stLoadInfo);
		mpDebugPrint("-E- cann't alloc handle");
		return FAIL;
	}
	memcpy(handle, &stLoadInfo, sizeof(ST_FONT_LOAD_INFO));
	psItem->handle = handle;

	//
	

	
	return PASS;	
}

int CloseFont(BYTE font_id)
{
	FONT_LIST_ITEM* psItem;
	psItem = &fontlist[font_id];
	
	if ( psItem->handle == NULL )
		return 0;
	_clear_load_info(psItem->handle);
	ext_mem_free(psItem->handle);
	psItem->handle = NULL;
	
	return 1;
}

void CloseAllFont(void)
{
	int id;
	for ( id = 0; id < sizeof(fontlist)/sizeof(FONT_LIST_ITEM); id++ ) 
		CloseFont(id);
}

DWORD GetTextExtent(BYTE font_id, const BYTE* lpString, DWORD* pdwWidth, DWORD* pdwHeight)
{
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;
	WORD ch, ch1, ch2;
	DWORD i;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;

	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return 0;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, NULL, psItem->handle, 0, 0, 0);
	ft.dwStrLength = strlen(lpString);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	for ( i = 0; i < ft.dwStrLength; i++ )
	{
		if ( pstHeader->codepage != 1200 )	// font not unicode
		{
			ch1 = * (lpString + i);
			if ( isLeadByte(ch1)) {
				i++;
				ch2 = * (lpString + i);
				ch = (ch1 << 8)|ch2;
			}
			else
				ch = ch1;
		}
		else
		{
			BYTE bstr[4];
			WORD wstr[4];
			if ( isLeadByte(*(lpString + i)) ) {
				bstr[0] = * (lpString + (i++));
				bstr[1] = * (lpString + i);
				bstr[2] = 0;
			}
			else {
				bstr[0] = * (lpString + i);
				bstr[1] = 0;
			}
			ToUTF16(bstr, wstr, 4, ft.bDefChar);
			ch = wstr[0];
		}
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			continue;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			continue;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			continue;
		}
		CalculateFontExtent(psItem->handle, &ft, pstHeader, ch);
	}

	if ( pdwWidth )
		*pdwWidth = ft.dwX;
	if ( pdwHeight )
		*pdwHeight = pstHeader->tmAscent + pstHeader->tmDescent;

	if ( boOpenFlag )
		CloseFont(font_id);
	
	return ft.dwX;
}

DWORD GetTextExtentW(BYTE font_id, const WORD* lpwString, DWORD* pdwWidth, DWORD* pdwHeight, BOOL boLittleEndian)
{
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;
	WORD ch;
	DWORD i;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;

	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return 0;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, NULL, psItem->handle, 0, 0, 0);
	ft.dwStrLength = wcslen(lpwString);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	for ( i = 0; i < ft.dwStrLength; i++ )
	{
		ch = * (lpwString + i);
		if ( boLittleEndian )
			ch = ((ch >> 8) & 0xff) | ((ch & 0xff) << 8);
		if ( pstHeader->codepage != 1200 )	// font not unicode
			ch = Uni2Ascii(ch, ft.bDefChar);
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			continue;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			continue;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			continue;
		}
		CalculateFontExtent(psItem->handle, &ft, pstHeader, ch);
	}

	if ( pdwWidth )
		*pdwWidth = ft.dwX;
	if ( pdwHeight )
		*pdwHeight = pstHeader->tmAscent + pstHeader->tmDescent;

	if ( boOpenFlag )
		CloseFont(font_id);
	
	return ft.dwX;
}

DWORD GetUtf8TextExtent(BYTE font_id, const BYTE* u8string, DWORD* pdwWidth, DWORD* pdwHeight)
{
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;
	WORD ch;
	int valid;
	DWORD i, u32char, u8charlen;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;

	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return 0;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, NULL, psItem->handle, 0, 0, 0);
	ft.dwStrLength = strlen(u8string);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	for ( i = 0; i < ft.dwStrLength; i+=u8charlen )
	{
		Utf8CharToUtf32Char(&u8string[i], &u32char, &u8charlen, &valid);
		if ( valid && u32char < 0xffff )
			ch = (WORD)u32char;
		else
			ch = ft.bDefChar;
		
		if ( pstHeader->codepage != 1200 )	// font is not unicode
		{
			BYTE bstr[8];
			WORD wstr[4];
			wstr[0] = ch;
			wstr[1] = 0;
			FromUTF16(wstr, bstr, sizeof(bstr), ft.bDefChar);
			if ( isLeadByte(bstr[0]) )
				ch = (bstr[0] << 8) | bstr[1];
			else
				ch = bstr[0];
		}
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			continue;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			continue;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			continue;
		}
		CalculateFontExtent(psItem->handle, &ft, pstHeader, ch);
	}

	if ( pdwWidth )
		*pdwWidth = ft.dwX;
	if ( pdwHeight )
		*pdwHeight = pstHeader->tmAscent + pstHeader->tmDescent;

	if ( boOpenFlag )
		CloseFont(font_id);
	
	return ft.dwX;
}


DWORD GetFontHeight(BYTE font_id)
{
	DWORD value;
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;
	const ST_TAB_HEADER_EXT *pstHeader;
	
	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return 0;
		boOpenFlag = TRUE;
	}
	
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;
	value = pstHeader->tmAscent + pstHeader->tmDescent;
	if ( boOpenFlag )
		CloseFont(font_id);
	
	return value;
}

DWORD GetFontAveWidth(BYTE font_id)
{
	DWORD value;
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;
	const ST_TAB_HEADER_EXT *pstHeader;
	
	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return 0;
		boOpenFlag = TRUE;
	}
	
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;
	value = pstHeader->tmAveWidth;
	if ( boOpenFlag )
		CloseFont(font_id);
	
	return pstHeader->tmAveWidth;
}

DWORD TextOut(ST_IMGWIN* pWin, BYTE font_id, const BYTE* lpString, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth)
{
	WORD ch, ch1, ch2;
	DWORD i;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;

	if (pWin == NULL || pWin->pdwStart == NULL || lpString == NULL)
		return nXStart;
	if ( nXStart >= pWin->wWidth )
		return nXStart;
	if ( nYStart >= pWin->wHeight )
		return nXStart;
	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return nXStart;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, pWin, psItem->handle, nXStart, nYStart, dwMaxWidth);
	ft.dwStrLength = strlen(lpString);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	LockFontDataBuffer(psItem->handle);
	for ( i = 0; i < ft.dwStrLength; i++ )
	{
		if ( pstHeader->codepage != 1200 )	// font not unicode
		{
			ch1 = * (lpString + i);
			if ( isLeadByte(ch1)) {
				i++;
				ch2 = * (lpString + i);
				ch = (ch1 << 8)|ch2;
			}
			else
				ch = ch1;
		}
		else
		{
			BYTE bstr[4];
			WORD wstr[4];
			if ( isLeadByte(*(lpString + i)) ) {
				bstr[0] = * (lpString + (i++));
				bstr[1] = * (lpString + i);
				bstr[2] = 0;
			}
			else {
				bstr[0] = * (lpString + i);
				bstr[1] = 0;
			}
			ToUTF16(bstr, wstr, 4, ft.bDefChar);
			ch = wstr[0];
		}
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		DrawFontCell(psItem->handle, &ft, pstHeader, ch);
End_DrawCell:
		if ( dwMaxWidth )
			if ( ft.dwDisplayWidth >= dwMaxWidth )
				break;
		if ( ft.dwX >= pWin->wWidth || ft.dwY >= pWin->wHeight )
			break;
	}
	UnLockFontDataBuffer(psItem->handle);

	if ( boOpenFlag )
		CloseFont(font_id);
	
	if( ft.dwX < pWin->wWidth )
		return ft.dwX;
	else
		return 0;
}

DWORD TextOutW(ST_IMGWIN* pWin, BYTE font_id, const WORD* lpwString, DWORD nXStart, DWORD nYStart, 
				DWORD dwMaxWidth, BOOL boLittleEndian)
{
	WORD ch;
	DWORD i;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;

	if (pWin == NULL || pWin->pdwStart == NULL || lpwString == NULL)
		return nXStart;
	if ( nXStart >= pWin->wWidth )
		return nXStart;
	if ( nYStart >= pWin->wHeight )
		return nXStart;
	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return nXStart;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, pWin, psItem->handle, nXStart, nYStart, dwMaxWidth);
	ft.dwStrLength = wcslen(lpwString);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	LockFontDataBuffer(psItem->handle);
	for ( i = 0; i < ft.dwStrLength; i++ )
	{
		ch = * (lpwString + i);
		if ( boLittleEndian )
			ch = ((ch >> 8) & 0xff) | ((ch & 0xff) << 8);
		if ( pstHeader->codepage != 1200 )	// font not unicode
			ch = Uni2Ascii(ch, ft.bDefChar);
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		DrawFontCell(psItem->handle, &ft, pstHeader, ch);
End_DrawCell:
		if ( dwMaxWidth )
			if ( ft.dwDisplayWidth >= dwMaxWidth )
				break;
		if ( ft.dwX >= pWin->wWidth || ft.dwY >= pWin->wHeight )
			break;
	}
	UnLockFontDataBuffer(psItem->handle);

	if ( boOpenFlag )
		CloseFont(font_id);
	
	if( ft.dwX < pWin->wWidth )
		return ft.dwX;
	else
		return 0;
}

DWORD TextOutUtf8(ST_IMGWIN* pWin, BYTE font_id, const BYTE* u8string, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth)
{
	WORD ch;
	DWORD i, u32char, u8charlen;
	int valid;
	ST_FONT_EX ft;
	const ST_TAB_HEADER_EXT *pstHeader;
	BOOL boOpenFlag;
	FONT_LIST_ITEM* psItem;

	if (pWin == NULL || pWin->pdwStart == NULL || u8string == NULL)
		return nXStart;
	if ( nXStart >= pWin->wWidth )
		return nXStart;
	if ( nYStart >= pWin->wHeight )
		return nXStart;
	psItem = &fontlist[font_id];
	boOpenFlag = FALSE;
	if ( psItem->handle == NULL )
	{
		if ( FAIL==OpenFont(font_id) )
			return nXStart;
		boOpenFlag = TRUE;
	}
	
	memset(&ft, 0, sizeof(ST_FONT_EX));
	InitFontExt(&ft, pWin, psItem->handle, nXStart, nYStart, dwMaxWidth);
	ft.dwStrLength = strlen(u8string);
	pstHeader = (ST_TAB_HEADER_EXT*) psItem->handle->pbTabBuff;

	LockFontDataBuffer(psItem->handle);
	for ( i = 0; i < ft.dwStrLength; i+=u8charlen )
	{
		Utf8CharToUtf32Char(&u8string[i], &u32char, &u8charlen, &valid);
		if ( valid && u32char < 0xffff )
			ch = (WORD)u32char;
		else
			ch = ft.bDefChar;
		
		if ( pstHeader->codepage != 1200 )	// font is not unicode
		{
			BYTE bstr[8];
			WORD wstr[4];
			wstr[0] = ch;
			wstr[1] = 0;
			FromUTF16(wstr, bstr, sizeof(bstr), ft.bDefChar);
			if ( isLeadByte(bstr[0]) )
				ch = (bstr[0] << 8) | bstr[1];
			else
				ch = bstr[0];
		}
		
		if ( isBlank(ch) ) {
			ft.dwX += pstHeader->tmAveWidth;
			ft.dwDisplayWidth += pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isFullBlank(ch) ) {
			ft.dwX += 2*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 2*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		else if ( isTabBlank(ch) ) {
			ft.dwX += 8*pstHeader->tmAveWidth;
			ft.dwDisplayWidth += 8*pstHeader->tmAveWidth;
			goto End_DrawCell;
		}
		DrawFontCell(psItem->handle, &ft, pstHeader, ch);
End_DrawCell:
		if ( dwMaxWidth )
			if ( ft.dwDisplayWidth >= dwMaxWidth )
				break;
		if ( ft.dwX >= pWin->wWidth || ft.dwY >= pWin->wHeight )
			break;
	}
	UnLockFontDataBuffer(psItem->handle);

	if ( boOpenFlag )
		CloseFont(font_id);
	
	if( ft.dwX < pWin->wWidth )
		return ft.dwX;
	else
		return 0;
}


void SetCurrFont(BYTE font_id)
{
	g_bFont = font_id;
}

BYTE GetCurrFont()
{
	return g_bFont;
}

DWORD TextOutString(ST_IMGWIN* pWin, const BYTE* lpString, DWORD nXStart, DWORD nYStart, 
				DWORD dwMaxWidth, BOOL boUnicode)
{
	if ( boUnicode )
		return TextOutW(pWin, g_bFont, lpString, nXStart, nYStart, dwMaxWidth, 0);
	else
		return TextOut(pWin, g_bFont, lpString, nXStart, nYStart, dwMaxWidth);
}

DWORD GetStringWidth(const BYTE* pbString, BOOL boUnicode)
{
	if ( boUnicode )
		return GetTextExtentW(g_bFont, pbString, NULL, NULL, 0);
	else
		return GetTextExtent(g_bFont, pbString, NULL, NULL);
}


static void _clear_load_info(ST_FONT_LOAD_INFO * pstInfo)
{
	if ( pstInfo->pbTabBuff ) {
		ext_mem_free(pstInfo->pbTabBuff);
		pstInfo->pbTabBuff = NULL;
	}
	if ( pstInfo->pdwDataBuff ) {
		ext_mem_free(pstInfo->pdwDataBuff);
		pstInfo->pdwDataBuff = NULL;
	}
	if ( pstInfo->fpData ) {
		FileClose(pstInfo->fpData);
		pstInfo->fpData = NULL;
	}
}

static void InitFontExt(ST_FONT_EX* pstFt, ST_IMGWIN* pWin, FONT_HANDLE handle, DWORD nXStart, DWORD nYStart, DWORD dwMaxWidth)
{
	int i;
	const ST_TAB_HEADER_EXT *pstHeader;
	const BYTE* pbAreaMap;
	
	pstFt->pWin = pWin;
	pstFt->bDefChar = handle->pstListItem->bDefaultChar;
	pstFt->dwX = nXStart;
	pstFt->dwY = nYStart;

	pstFt->bFontColor = (g_dwSystemFontColor >> 16) & 0xff;	//YYCbCr - Y
	pstFt->bFontCb = (g_dwSystemFontColor >> 8) & 0xff;
	pstFt->bFontCr = g_dwSystemFontColor & 0xff;
	pstFt->dwMaxWidth = dwMaxWidth;
	pstFt->dwDisplayWidth = 0;

	pstHeader = (ST_TAB_HEADER_EXT*) handle->pbTabBuff;
	
	pstFt->pbAreaMap = & handle->pbTabBuff[sizeof(ST_TAB_HEADER_EXT)];
	pstFt->dwAreaCnt = handle->bAreaCnt;

	pstFt->pdwOffsets = (DWORD*) & handle->pbTabBuff[256+sizeof(ST_TAB_HEADER_EXT)];
	if (pstHeader->bTableStyle ==1) {
		pstFt->pwCellIdx = (WORD*) & handle->pbTabBuff[(pstFt->dwAreaCnt*256*sizeof(int)) + 256+sizeof(ST_TAB_HEADER_EXT)];
		pstFt->pbCellInfo = & handle->pbTabBuff[(pstFt->dwAreaCnt*256*sizeof(short)) + 
								(pstFt->dwAreaCnt*256*sizeof(int)) + 256+sizeof(ST_TAB_HEADER_EXT)];
	}
	else {
		pstFt->pwCellIdx = (WORD*) & handle->pbTabBuff[(pstFt->dwAreaCnt*256*sizeof(short)) + 256+sizeof(ST_TAB_HEADER_EXT)];
		pstFt->pbCellInfo = & handle->pbTabBuff[(pstFt->dwAreaCnt*256*sizeof(short)) + 
								(pstFt->dwAreaCnt*256*sizeof(short)) + 256+sizeof(ST_TAB_HEADER_EXT)];
	}
}


static void LockFontDataBuffer(FONT_HANDLE handle)
{
	DWORD i;
	if (handle->pstListItem->bFlag & FONT_FLAG_LOAD_TAB_ONLY )
	{
		unsigned char** ppbDataMapTab = (unsigned char**)handle->pdwDataBuff;
		for ( i = 0; i < handle->dwCacheCnt; i++ )
			ppbDataMapTab[i] = NULL;
	}
}

static void UnLockFontDataBuffer(FONT_HANDLE handle)
{
	DWORD i;
	if (handle->pstListItem->bFlag & FONT_FLAG_LOAD_TAB_ONLY )
	{
		unsigned char** ppbDataMapTab = (unsigned char**)handle->pdwDataBuff;
		for ( i = 0; i < handle->dwCacheCnt; i++ )
		{
			if ( ppbDataMapTab[i] )
			{
				ext_mem_free(ppbDataMapTab[i]);
				ppbDataMapTab[i] = NULL;
			}
		}
	}
}

static DWORD GetFontDataPosition(ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch)
{
	BYTE area;
	DWORD offset;
	
	area = pfnt->pbAreaMap[(ch>>8)&0xff];
	if ( area==0xff )
		return 0xffffffff;
	if ( pstHeader->bTableStyle == 0 )
	{
		const WORD *pStart, *pwIndexes;
		pwIndexes = (WORD*) pfnt->pdwOffsets;
		pStart = & pwIndexes[area*256];
		offset = pStart[ch & 0xff];
		if ( offset == 0xffff )
			offset = 0xffffffff;
		offset = (pstHeader->wFixSize * offset) + sizeof(ST_TAB_HEADER_EXT);
	}
	else if ( pstHeader->bTableStyle == 1 )
	{
		const DWORD* pStart;
		pStart = & pfnt->pdwOffsets[area*256];
		offset = pStart[ch & 0xff];
	}
	else if ( pstHeader->bTableStyle == 2 )
	{
		const WORD *pStart, *pwOffsets;
		pwOffsets = (WORD*) pfnt->pdwOffsets;
		pStart = & pwOffsets[area*256];
		offset = pStart[ch & 0xff];
		if ( offset == 0xffff )
			offset = 0xffffffff;
	}
	return offset;
}

static BYTE getByte(FONT_HANDLE handle, DWORD pointer)
{
	const BYTE* pbData;
	if ( handle->pstListItem->bFlag & FONT_FLAG_LOAD_ALL )
	{
		pbData = (const BYTE*) handle->pdwDataBuff;
		return pbData[pointer];
	}
	else if ( handle->pstListItem->bFlag & FONT_FLAG_LOAD_TAB_ONLY )
	{
		DWORD section, offset;
		unsigned char** ppbDataMapTab = (unsigned char**)handle->pdwDataBuff;
		section = pointer / FONT_CACHE_BUF_SIZE;
		offset = pointer % FONT_CACHE_BUF_SIZE;
		if ( ppbDataMapTab[section] == NULL )
		{
			ppbDataMapTab[section] = (unsigned char*) ext_mem_malloc(FONT_CACHE_BUF_SIZE);
			if ( ppbDataMapTab[section] == NULL ) {
				mpDebugPrint("-E- cannot alloc font cache buffer.");
				return 0;
			}
			Fseek(handle->fpData, section*FONT_CACHE_BUF_SIZE, SEEK_SET);
			FileRead(handle->fpData, ppbDataMapTab[section], FONT_CACHE_BUF_SIZE);
		}
		return ppbDataMapTab[section][offset];
	}
}

static DWORD stream_cnt = 0;		// the font data stream counter
static BYTE resi_bits = 0;			// residual bit counter
static BYTE ReadBits(FONT_HANDLE handle, const ST_TAB_HEADER_EXT *pstHeader)
{
	BYTE bit_num, r;
	const static BYTE mask1[] = { 0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff };
	static BYTE lastbyte;

	bit_num = pstHeader->bBitsPerPixel;
	//----------------
	if ( resi_bits == 0 ) {
		lastbyte = getByte(handle, stream_cnt++);
		resi_bits = 8;
	}
	//----------------
	if ( bit_num < resi_bits ) {
		r = lastbyte & mask1[bit_num];
		r >>= (8-bit_num);
		lastbyte <<= bit_num;
		resi_bits -= bit_num;
	}
	else if ( bit_num == resi_bits ) {
		r = lastbyte & mask1[bit_num];
		r >>= (8-bit_num);
		lastbyte = 0;
		resi_bits = 0;
	}
	return r;
}

static void Mix2Point(DWORD* pdwDisplayBuf, BYTE level0, BYTE level1, ST_FONT_EX* pfnt)
{
	BYTE y0,y1,u,v;
	BYTE new_y0,new_y1,new_u,new_v;
	DWORD L0, L1;
	
	L0 = level0;
	L1 = level1;
	
	y0 = *pdwDisplayBuf >> 24;
	y1 = (*pdwDisplayBuf >> 16)&0xff;
	u = (*pdwDisplayBuf >> 8)&0xff;
	v = *pdwDisplayBuf & 0xff;

	new_y0 = (pfnt->bFontColor * L0 + y0 * (255-L0))/255;
	new_y1 = (pfnt->bFontColor * L1 + y1 * (255-L1))/255;

	L0 = L0 + L1;
	L1 = (255*2) - L0;
	new_u = (u*L1 + pfnt->bFontCb * L0)/((255*2));
	new_v = (v*L1 + pfnt->bFontCr * L0)/((255*2));
	
	*pdwDisplayBuf = (new_y0 << 24)|(new_y1<<16)|(new_u<<8)|new_v;
}

static BYTE SpreadLevel_1bit(BYTE level)
{
	return level ? 0xff : 0x00;
}

static BYTE SpreadLevel_4bit(BYTE level)
{
	level &= 0x0f;
	return (level << 4) | level;
}

static BYTE SpreadLevel(BYTE level, BYTE bBitsPerPixel)
{
	static BYTE (*SpreadLevelFunc[])(BYTE) = {
		NULL,
		SpreadLevel_1bit,
		NULL,
		NULL,
		SpreadLevel_4bit,
	};
	return SpreadLevelFunc[bBitsPerPixel](level);
}

static void DrawFontCell(FONT_HANDLE handle, ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch)
{
	ST_IMGWIN *pWin;
	BYTE area;
	BYTE level0, level1;
	BYTE boBeginS, boEndS;						// the flag for odd number point
	WORD wCellIdx;
	const WORD *pwCellStart;
	const ST_FONT_CELL_DESC* pstCellDesc;		// pointer of font cell base information
	DWORD dwDrawX, dwDrawY;						// the (X,Y) of starting draw font cell
	DWORD *pdwStartAddr, *pdwLineAddr;			// the draw address of frame buffer
	DWORD y ,j;
	DWORD dwCellWidth, dwCellHeight, dwMidCnt;
	int   originY, addLine;
	
	pWin = pfnt->pWin;
	stream_cnt = GetFontDataPosition(pfnt, pstHeader, ch);

	if ( stream_cnt==0xffffffff ) {
		if ( pfnt->bDefChar == ' ' ) {
			pfnt->dwX += pstHeader->tmAveWidth;
			pfnt->dwDisplayWidth += pstHeader->tmAveWidth;
			return;
		}
		stream_cnt = GetFontDataPosition(pfnt, pstHeader, pfnt->bDefChar);
		ch = pfnt->bDefChar;
	}
	if ( stream_cnt==0xffffffff ) {
		mpDebugPrint("-E- get font data address fail");
		return;
	}
	// get font cell information
	area = pfnt->pbAreaMap[(ch>>8)&0xff];
	pwCellStart = & pfnt->pwCellIdx[area*256];
	wCellIdx = pwCellStart[ch & 0xff];
	pstCellDesc = (ST_FONT_CELL_DESC*) & pfnt->pbCellInfo[wCellIdx*sizeof(ST_FONT_CELL_DESC)];

	if( pstHeader->bBitsPerPixel > 4 )
		return;

	//mpDebugPrint("stream_cnt = 0x%08x, CELL 0x%02x : %d, %d, %d, %d, %d, %d", stream_cnt, ch, pstCellDesc->bWidth, pstCellDesc->bHeight,
	//	pstCellDesc->cornerX, pstCellDesc->cornerY, pstCellDesc->offsetX, pstCellDesc->offsetY);
	
	// get the starting draw coordinate
	dwDrawX = pfnt->dwX + (int) pstCellDesc->cornerX;
	originY = pstHeader->tmAscent;
	addLine = originY - (int) pstCellDesc->cornerY;
	dwDrawY = pfnt->dwY + addLine;

	//mpDebugPrint("pfnt->dwX = %d, pfnt->dwY = %d", pfnt->dwX, pfnt->dwY);
	//mpDebugPrint("dwDrawX = %d, dwDrawY = %d", dwDrawX, dwDrawY);

	pdwStartAddr = & pWin->pdwStart[ dwDrawY*(pWin->dwOffset/4) + dwDrawX/2 ];
	dwCellWidth = (DWORD)pstCellDesc->bWidth;
	dwCellHeight = (DWORD)pstCellDesc->bHeight;
	resi_bits = 0;					// the residual bit counter clean 0
	for ( y = 0; y < dwCellHeight; y++ )
	{
		boBeginS = boEndS = 0;
		if ( dwDrawX & 0x01 )
			boBeginS = 1;
		if ( (dwDrawX + dwCellWidth) & 0x01 )
			boEndS = 1;
		pdwLineAddr = pdwStartAddr + y*(pWin->dwOffset/4);
		if ( boBeginS )
		{
			level1 = ReadBits(handle, pstHeader);
			level1 = SpreadLevel(level1, pstHeader->bBitsPerPixel);
			Mix2Point(pdwLineAddr++, 0, level1, pfnt);
		}
		dwMidCnt = (dwCellWidth - boBeginS - boEndS) / 2;
		for ( j = 0; j < dwMidCnt; j++ )
		{
			level0 = ReadBits(handle, pstHeader);
			level0 = SpreadLevel(level0, pstHeader->bBitsPerPixel);
			level1 = ReadBits(handle, pstHeader);
			level1 = SpreadLevel(level1, pstHeader->bBitsPerPixel);
			Mix2Point(pdwLineAddr++, level0, level1, pfnt);
		}
		if ( boEndS )
		{
			level0 = ReadBits(handle, pstHeader);
			level0 = SpreadLevel(level0, pstHeader->bBitsPerPixel);
			Mix2Point(pdwLineAddr++, level0, 0, pfnt);
		}
	}
	
	pfnt->dwX += (int) pstCellDesc->offsetX;
	pfnt->dwY += (int) pstCellDesc->offsetY;
	pfnt->dwDisplayWidth += (int) pstCellDesc->offsetX;

}

static void CalculateFontExtent(FONT_HANDLE handle, ST_FONT_EX* pfnt, const ST_TAB_HEADER_EXT *pstHeader, WORD ch)
{
	BYTE area;
	WORD wCellIdx;
	const WORD *pwCellStart;
	const ST_FONT_CELL_DESC* pstCellDesc;		// pointer of font cell base information
	
	// get font cell information
	area = pfnt->pbAreaMap[(ch>>8)&0xff];
	if ( area==0xff ) {
		if ( pfnt->bDefChar == ' ' ) {
			pfnt->dwX += pstHeader->tmAveWidth;
			pfnt->dwDisplayWidth += pstHeader->tmAveWidth;
			return;
		}
		else if ( pfnt->bDefChar == ch )
			return;
		else {
			CalculateFontExtent(handle, pfnt, pstHeader, pfnt->bDefChar);
			return;
		}
	}
	pwCellStart = & pfnt->pwCellIdx[area*256];
	wCellIdx = pwCellStart[ch & 0xff];
	if ( wCellIdx == 0xffff ) {
		if ( pfnt->bDefChar == ' ' ) {
			pfnt->dwX += pstHeader->tmAveWidth;
			pfnt->dwDisplayWidth += pstHeader->tmAveWidth;
			return;
		}
		else if ( pfnt->bDefChar == ch )
			return;
		else {
			CalculateFontExtent(handle, pfnt, pstHeader, pfnt->bDefChar);
			return;
		}
	}
	pstCellDesc = (ST_FONT_CELL_DESC*) & pfnt->pbCellInfo[wCellIdx*sizeof(ST_FONT_CELL_DESC)];
	
	pfnt->dwX += (int) pstCellDesc->offsetX;
	pfnt->dwY += (int) pstCellDesc->offsetY;
	pfnt->dwDisplayWidth += (int) pstCellDesc->offsetX;
}

static WORD Uni2Ascii(WORD uniChar, BYTE bDefChar)
{
	WORD wstr[2];
	BYTE bstr[8];
	wstr[0] = uniChar;
	wstr[1] = 0;
	FromUTF16(wstr, bstr, 8, bDefChar);
	if ( bstr[1] == 0 )
		return bstr[0];
	else {
		if (isLeadByte(bstr[0]))
			return (bstr[0]<<8)|bstr[1];
		else
			return bstr[0];
	}
}

BOOL isBlank(WORD unicode_char)
{
	if ( unicode_char == 0x20 || unicode_char == 0xA0 )
		return TRUE;
	else
		return FALSE;
}

BOOL isFullBlank(WORD unicode_char)
{
	if ( unicode_char == 0x3000 )
		return TRUE;
	else
		return FALSE;
}

BOOL isTabBlank(WORD unicode_char)
{
	if ( unicode_char == '\t' )
		return TRUE;
	else
		return FALSE;
}





#endif	//EXT_FONT_FORMAT_ENABLE


