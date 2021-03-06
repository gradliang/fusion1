/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "fontdisp.h"
#if 0
#include "Font12.h"
#include "Font12Tab.h"
#include "Font24.h"
#include "Font24Tab.h"
#include "Font18.h"
#include "Font18Tab.h"
#include "Font40.h" 
#endif
#ifdef FONT_ID_TAHOMA19
#include "Tahoma19Tab.h"
#include "Tahoma19Data.h"
#endif
#ifdef FONT_ID_YAHEI19
#include "YaHei19Data.h"
#include "YaHei19Tab.h"
#endif
#ifdef FONT_ID_LibMono17
#include "OsdFont17.h"
#include "OsdFontTab17.h"
#endif
#ifdef FONT_ID_DotumChe18
#include "OsdFont18.h"
#include "OsdFontTab18.h"
#endif
#ifdef FONT_ID_HeiTi16
#include "HeiTi16Data.h"
#include "HeiTi16Tab.h"
#endif
#ifdef FONT_ID_HeiTi19
#include "HeiTi19Data.h"
#include "HeiTi19Tab.h"
#endif
#ifdef FONT_ID_HeiTi10
#include "HeiTi10Data.h"
#include "HeiTi10Tab.h"
#endif
#ifdef FONT_ID_ARIAL_36
#include "Arial36Data.h"
#include "Arial36Tab.h"
#endif

#if BMP_FONT_ENABLE

struct _tagNewFontTabHeader
{
	BYTE	strNewTabTag[8];		// "NewFont"
	BYTE	bBitsPerPixel;
	BYTE	bMaxHeight;
	BYTE	bAlign;
	BYTE	bReserve;
};
typedef struct _tagNewFontTabHeader NewFontTabHeader_t;

typedef struct {
    unsigned int code;
    unsigned int offset;
} LargeFontTab_t;
typedef struct {
  unsigned short start;
  unsigned short total;
} LargeFontFastIndex_t;


struct _tagFontInfoData
{
	const WORD* pTab;
	const BYTE* pData;
	LargeFontTab_t * largeTab;
	LargeFontFastIndex_t * fastIdxTab;
	DWORD		dwCurrFontID;	
	WORD		wSpaceSize;
	BYTE		bFlag;			//0->00-ff 1bit old font   1-> with fastIdxTab new font
};
typedef struct _tagFontInfoData FontInfo_t;
static FontInfo_t idufontinfo = {0};
static FontInfo_t osdfontinfo = {0};

DWORD GetCurrIduFontID()
{
	return idufontinfo.dwCurrFontID;
}

DWORD GetCurrOsdFontID()
{
	return osdfontinfo.dwCurrFontID;
}

static BOOL SetFontDataInfo(FontInfo_t * pstInfo, DWORD wFontID)
{
    pstInfo->pData = NULL;
    pstInfo->pTab  = NULL;
    pstInfo->largeTab = NULL;
    pstInfo->fastIdxTab = NULL;
    pstInfo->dwCurrFontID = wFontID;
    pstInfo->wSpaceSize = 6;
    pstInfo->bFlag = 0;
    switch (wFontID)
    {
#ifdef FONT_ID_YAHEI19
    case FONT_ID_YAHEI19:
        pstInfo->pData = YaHei19_fontdata;
        pstInfo->largeTab = YaHei19_FontTab;
        pstInfo->fastIdxTab = YaHei19_FastIndexTab;
        pstInfo->bFlag = 1;
        break;
#endif

#ifdef FONT_ID_LibMono17
    case FONT_ID_LibMono17:
        pstInfo->pData = LiberationMono17_Data;
        pstInfo->pTab = LiberationMono17_Tab;
    	pstInfo->bFlag = 0;
        break;
#endif
#ifdef FONT_ID_DotumChe18
    case FONT_ID_DotumChe18:
        pstInfo->pData = DotumChe18_Data;
        pstInfo->pTab = DotumChe18_Tab;
        pstInfo->bFlag = 0;
        break;
#endif
#ifdef FONT_ID_HeiTi16
    case FONT_ID_HeiTi16:
        pstInfo->pData = HeiTi16_fontdata;
        pstInfo->largeTab = HeiTi16_FontTab;
        pstInfo->fastIdxTab = HeiTi16_FastIndexTab;
        pstInfo->bFlag = 1;
        break;
#endif
#ifdef FONT_ID_HeiTi19
        case FONT_ID_HeiTi19:
            pstInfo->pData = HeiTi19_fontdata;
            pstInfo->largeTab = HeiTi19_FontTab;
            pstInfo->fastIdxTab = HeiTi19_FastIndexTab;
            pstInfo->bFlag = 1;
            break;
#endif
#ifdef FONT_ID_HeiTi10
        case FONT_ID_HeiTi10:
            pstInfo->pData = HeiTi10_fontdata;
            pstInfo->largeTab = HeiTi10_FontTab;
            pstInfo->fastIdxTab = HeiTi10_FastIndexTab;
            pstInfo->bFlag = 1;
            break;
#endif
#ifdef FONT_ID_ARIAL_36
        case FONT_ID_ARIAL_36:
            pstInfo->pData = Arial36Data;
            pstInfo->largeTab = Arial36Tab;
            pstInfo->fastIdxTab = Arial36FastIdxTab;
            pstInfo->bFlag = 1;
            break;
#endif
    default:
        pstInfo->pData = Tahoma19_fontdata;
        pstInfo->pTab = Tahoma19_OffsetList;
        pstInfo->bFlag = 0;
        break;
    }
    if (pstInfo->pData && pstInfo->pTab)
        return TRUE;
    return FALSE;
}

void SetCurrIduFontID(DWORD id)
{
	SetFontDataInfo(&idufontinfo, id);
}

void SetCurrOsdFontID(DWORD id)
{
	SetFontDataInfo(&osdfontinfo, id);
}

BYTE * GetIduFontDataStartAddress()
{
	return idufontinfo.pData;
}

BYTE * GetOsdFontDataStartAddress()
{
	return osdfontinfo.pData;
}

BYTE IduFontGetBitPerPixel()
{
	NewFontTabHeader_t * pFontHeader;
	pFontHeader = (NewFontTabHeader_t*) GetIduFontDataStartAddress();
	if ( pFontHeader==NULL )
		return 0;
	return pFontHeader->bBitsPerPixel;
}

BYTE OsdFontGetBitPerPixel()
{
	NewFontTabHeader_t * pFontHeader;
	pFontHeader = (NewFontTabHeader_t*) GetOsdFontDataStartAddress();
	if ( pFontHeader==NULL )
		return 0;
	return pFontHeader->bBitsPerPixel;
}

static BOOL IsNewTabFont(FontInfo_t * pstInfo)
{
	DWORD tag;
	BYTE *strTag;
	NewFontTabHeader_t * pFontHeader;
	pFontHeader = (NewFontTabHeader_t*) pstInfo->pData;
	if ( pFontHeader == NULL )
		return 0;
	strTag = pFontHeader->strNewTabTag;
	tag = (strTag[0]<<24) + (strTag[1]<<16) + (strTag[2]<<8) + strTag[3];
	if ( tag==0x4e657746 )
		return 1;
	else
		return 0;
}

BOOL IduFontIsNewTabFont()
{
	return IsNewTabFont(&idufontinfo);
}

BOOL OsdFontIsNewTabFont()
{
	return IsNewTabFont(&osdfontinfo);
}

static BYTE FontGetFontAlignment(FontInfo_t * pstInfo)
{
	NewFontTabHeader_t * pFontHeader;
	pFontHeader = (NewFontTabHeader_t*) pstInfo->pData;
	if ( pFontHeader==NULL )
		return 0;
	return pFontHeader->bAlign;
}

BYTE IduFontGetFontAlignment()
{
	return FontGetFontAlignment(&idufontinfo);
}

BYTE OsdFontGetFontAlignment()
{
	return FontGetFontAlignment(&osdfontinfo);
}

static BYTE FontGetMaxHeight(FontInfo_t * pstInfo)
{
	NewFontTabHeader_t * pFontHeader;
	pFontHeader = (NewFontTabHeader_t*) pstInfo->pData;
	if ( pFontHeader==NULL )
		return 0;
	return pFontHeader->bMaxHeight;
}

BYTE IduFontGetMaxHeight()
{
	return FontGetMaxHeight(&idufontinfo);
}

BYTE OsdFontGetMaxHeight()
{
	return FontGetMaxHeight(&osdfontinfo);
}

static BYTE* GetCharBitmapAddress(FontInfo_t * pstInfo, WORD wch)
{
	BYTE * pData;
	
	if (pstInfo->bFlag == 0) 
	{
		WORD * pTab;
		if (wch > 0xff)
			return NULL;
		pData = pstInfo->pData;
		pTab = pstInfo->pTab;
		return (pTab[wch] == 0xffff)? NULL : (pData + pTab[wch]);
	}
	else if (pstInfo->bFlag == 1) 
	{
		unsigned short start;
		unsigned short total;
		unsigned short i;
		int found;
		LargeFontTab_t * tab;
		LargeFontTab_t * largeTab;
		LargeFontFastIndex_t * fastIdxTab;
		pData = pstInfo->pData;
		largeTab = pstInfo->largeTab;
		fastIdxTab = pstInfo->fastIdxTab;
		
		start = fastIdxTab[(wch >> 8) & 0xff].start;
		total = fastIdxTab[(wch >> 8) & 0xff].total;
		if (total == 0)
			return NULL;
		
		i = 0;
		found = 0;
		while (i < total)
		{
			tab = &largeTab[start+i];
			if (tab->code==0 && tab->offset==0)
				return NULL;
			if (tab->code == (unsigned)wch) {
				found = 1;
				break;
			}
			i++;
		}
		if (found) 
			return (& pData[tab->offset]);
		else
			return NULL;
	}
}

BYTE* IduFontGetCharBitmapAddr(WORD wch)
{
	return GetCharBitmapAddress(&idufontinfo, wch);
}

BYTE* OsdFontGetCharBitmapAddr(WORD wch)
{
	return GetCharBitmapAddress(&osdfontinfo, wch);
}

BYTE* IduFontGetCharWidth(WORD wch)
{
	BYTE *bitmap= GetCharBitmapAddress(&idufontinfo, wch);

	if (bitmap == NULL)
	{
		bitmap =(BYTE *)IduFontGetCharBitmapAddr(UNKNOWN_CHAR);
		if (bitmap == NULL) 
			return 0;
		return (*bitmap);
	}
}

DWORD IduFontGetBlankWidth()
{
	return idufontinfo.wSpaceSize;
}

DWORD OsdFontGetBlankWidth()
{
	return osdfontinfo.wSpaceSize;
}

//============================================================================================
//Initial font resource
//============================================================================================

void FontInit()		//Mason 0209 //SourceSafe	// Lighter modify 20090903
{
#if BMP_FONT_ENABLE

#ifdef FONT_ID_TAHOMA19
    SetCurrIduFontID(FONT_ID_TAHOMA19);
#endif

#ifdef FONT_ID_DotumChe18
    SetCurrOsdFontID(FONT_ID_DotumChe18);
#endif

#endif
}


#endif



