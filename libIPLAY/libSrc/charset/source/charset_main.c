
#define LOCAL_DEBUG_ENABLE  0
#include "global612.h"
#include "mpTrace.h"

#include "charset.h"
#include "charset_int.h"
#include "cp1250.h"
#include "cp1251.h"
#include "cp1252.h"
#include "gbk.h"
#include "big5.h"
#include "korean.h"
#include "shift_jis.h"


static DWORD codepage = 1252;
static DWORD locale_id = 0;
typedef struct tagCodePageMap {
	DWORD		codepage;
	const BYTE* encoding;
}
ST_CODEPAGE_MAP;

static ST_CODEPAGE_MAP g_stCodePageMap [] = {
	{	1252,	"en_US"		},
	{	1252,	"cp1252"	},
	{	1250,	"cp1250"	},
	{	1251,	"cp1251"	},
	{	936,	"zh_CN"		},
	{	936,	"gbk"		},
	{	936,	"gb2312"	},
	{	950,	"zh_TW"		},
	{	950,	"big5"		},
	{	949,	"cp949"		},
	{	932,	"jp_JP"		},
	{	932,	"shift_jis"	},
};

typedef struct {
	DWORD		codepage;
	BOOL (*isLeadByte)(BYTE ch);
	int (*ToUTF16)(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);
	int (*FromUTF16)(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);
} ST_LOCALE_FUNC;

static ST_LOCALE_FUNC locale_funcs[] = 
{
#if CHARSET_CP1252_ENABLE
	{   1252,    cp1252_isLeadByte,    cp1252_ToUTF16,    cp1252_FromUTF16 },
#endif
#if CHARSET_GBK_ENABLE
	{   936,     gbk_isLeadByte,       gbk_ToUTF16,       gbk_FromUTF16 },
#endif
#if CHARSET_BIG5_ENABLE
	{   950,     big5_isLeadByte,      big5_ToUTF16,      big5_FromUTF16 },
#endif
#if CHARSET_CP949_ENABLE
	{   949,     cp949_isLeadByte,     cp949_ToUTF16,     cp949_FromUTF16 },		// korean 
#endif
#if CHARSET_SHIFT_JIS_ENABLE
	{	932,	 shift_jis_isLeadByte, shift_jis_ToUTF16, shift_jis_FromUTF16 },	// Japan 
#endif
#if CHARSET_CP1250_ENABLE
	{	1250,	 cp1250_isLeadByte,    cp1250_ToUTF16,    cp1250_FromUTF16 },		// Central Europe
#endif
#if CHARSET_CP1251_ENABLE
	{	1251,	 cp1251_isLeadByte,    cp1251_ToUTF16,    cp1251_FromUTF16 },		// Russian
#endif

};

int _setlocale(const BYTE* locale_name)
{
	DWORD i, j;
	const BYTE* pbEncodingName;

	if ( locale_name==NULL || locale_name[0]==0 )
		return FAIL;
	pbEncodingName = strrchr(locale_name, '.');
	if ( pbEncodingName )
		pbEncodingName++;
	else
		pbEncodingName = locale_name;
	
	for ( i = 0; i < sizeof(g_stCodePageMap)/sizeof(ST_CODEPAGE_MAP); i++)
	{
		if (0 == stricmp_(pbEncodingName, g_stCodePageMap[i].encoding))
		{
			codepage = g_stCodePageMap[i].codepage;
			for ( j = 0; j < sizeof(locale_funcs)/sizeof(ST_LOCALE_FUNC); j++)
			{
				if ( codepage==locale_funcs[j].codepage )
				{
					locale_id = j;
					break;
				}
			}
			return (int) codepage;
		}
	}
	return FAIL;
}

unsigned GetACP()
{
	return codepage;
}

BOOL isLeadByte(BYTE ch)
{
	return locale_funcs[locale_id].isLeadByte(ch);
}

int ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar)
{
	return locale_funcs[locale_id].ToUTF16(pbByteString, pwWordBuffer, dwWordBufSize, wDefaultChar);
}

int FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar)
{
	return locale_funcs[locale_id].FromUTF16(pwWordString, pbByteBuffer, dwByteBufSize, bDefaultChar);
}

int SwapUniStringEndian(WORD *pwString)
{
	WORD ch, i;
	i = 0;
	while (1)
	{
		ch = *(pwString + i);
		ch = ((ch & 0xff) << 8) | (ch >> 8);
		*(pwString + i) = ch;
		if ( ch == 0 )
			break;
		i++;
	}
	return i;
}

int Utf8ToUtf16BE(WORD* pwDest, const BYTE* pbSrc)					//UTF-8 To UTF-16(Big-Endian)
{
	WORD* wname = pwDest;
	const BYTE* name = pbSrc;
	unsigned len, count, wcount;
	BYTE byte1,byte2,byte3;
	WORD wch;
	
	len = strlen(name)+1;
	count = 0;
	wcount = 0;
	while ( count < len )
	{
		byte1 = name[count];
		if ( byte1 < 0x80 )
		{
			wname[wcount] = byte1;
			count++;
			wcount++;
		}
		else if ( (byte1 & 0xE0) == 0xC0 )
		{
			byte2 = name[count+1];
			wch = byte1 & 0x1F;
			wch <<= 6;
			wch += ( byte2 & 0x3F );
			wname[wcount] = wch;
			count+=2;
			wcount++;
		}
		else if ( (byte1 & 0xF0) == 0xE0 )
		{
			byte2 = name[count+1];
			byte3 = name[count+2];
			wch = byte1 & 0x0F;
			wch <<= 6;
			wch += ( byte2 & 0x3F );
			wch <<= 6;
			wch += ( byte3 & 0x3F );
			wname[wcount] = wch;
			count+=3;
			wcount++;
		}
		else
		{
			wname[wcount] = byte1;
			count++;
			wcount++;
		}
	}
	return wcount;
}

int Utf16BEToUtf8(BYTE* pbDest, const WORD* pwSrc)	//UTF-8 To UTF-16(Big-Endian)
{
	WORD wch;
	DWORD count, i;

	count = 0;
	i = 0;
	while ( wch = *(pwSrc+i) )
	{
		if ( wch < 0x80 ) 
			pbDest[count++] = wch;
		else if ( wch < 0x0800 )
		{
			pbDest[count++] = 0xC0 | (wch >> 6);
			pbDest[count++] = 0x80 | (wch & 0x3F);
		}
		else
		{
			pbDest[count++] = 0xE0 | (wch >> 12);
			pbDest[count++] = 0x80 | ((wch >> 6)&0x3F);
			pbDest[count++] = 0x80 | (wch & 0x3F);
		}
		i++;
	}
	pbDest[count] = 0;
	
	return count;
}

void Utf8CharToUtf32Char(const BYTE* utf8, DWORD *pdwU32char, DWORD *pdwUtf8CharLength, int *pValid)
{
	DWORD u32char;
	// 1 byte
	if ( utf8[0] < 0x80 ) {
		*pdwU32char = utf8[0];
		*pdwUtf8CharLength = 1;
		*pValid = TRUE;
		return;
	}
	
	// 2 bytes
	if ( (utf8[0] & 0xE0) == 0xC0 )	{
		if ( (utf8[1] & 0xC0) == 0x80 ) {
			u32char = utf8[0] & 0x1F;
			u32char <<= 6;
			u32char |= ( utf8[1] & 0x3F );
			*pdwU32char = u32char;
			*pdwUtf8CharLength = 2;
			*pValid = TRUE;
			return;
		}
		else
			goto Fail;
	}

	// 3 bytes
	if ( (utf8[0] & 0xF0) == 0xE0 )	{
		if ( (utf8[1] & 0xC0) == 0x80 && (utf8[2] & 0xC0) == 0x80 ) {
			u32char = utf8[0] & 0x0F;
			u32char <<= 6;
			u32char |= ( utf8[1] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[2] & 0x3F );
			*pdwU32char = u32char;
			*pdwUtf8CharLength = 3;
			*pValid = TRUE;
			return;
		}
		else
			goto Fail;
	}

	// 4 bytes
	if ( (utf8[0] & 0xF8) == 0xF0 )	{
		if ( (utf8[1] & 0xC0) == 0x80 && (utf8[2] & 0xC0) == 0x80 && (utf8[3] & 0xC0) == 0x80 ) {
			u32char = utf8[0] & 0x07;
			u32char <<= 6;
			u32char |= ( utf8[1] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[2] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[3] & 0x3F );
			*pdwU32char = u32char;
			*pdwUtf8CharLength = 4;
			*pValid = TRUE;
			return;
		}
		else
			goto Fail;
	}

	// 5 bytes
	if ( (utf8[0] & 0xFC) == 0xF8 )	{
		if ( (utf8[1] & 0xC0) == 0x80 && (utf8[2] & 0xC0) == 0x80 
				&& (utf8[3] & 0xC0) == 0x80 && (utf8[4] & 0xC0) == 0x80 ) {
			u32char = utf8[0] & 0x03;
			u32char <<= 6;
			u32char |= ( utf8[1] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[2] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[3] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[4] & 0x3F );
			*pdwU32char = u32char;
			*pdwUtf8CharLength = 5;
			*pValid = TRUE;
			return;
		}
		else
			goto Fail;
	}

	// 6 bytes
	if ( (utf8[0] & 0xFE) == 0xFC )	{
		if ( (utf8[1] & 0xC0) == 0x80 && (utf8[2] & 0xC0) == 0x80 
				&& (utf8[3] & 0xC0) == 0x80 && (utf8[4] & 0xC0) == 0x80 && (utf8[5] & 0xC0) == 0x80 ) {
			u32char = utf8[0] & 0x01;
			u32char <<= 6;
			u32char |= ( utf8[1] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[2] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[3] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[4] & 0x3F );
			u32char <<= 6;
			u32char |= ( utf8[5] & 0x3F );
			*pdwU32char = u32char;
			*pdwUtf8CharLength = 6;
			*pValid = TRUE;
			return;
		}
		else
			goto Fail;
	}
Fail:
	*pdwU32char = utf8[0];
	*pdwUtf8CharLength = 1;
	*pValid = FALSE;
	return;
}



