
#ifndef CHARSET_H_BC661AD39A587CDF
#define CHARSET_H_BC661AD39A587CDF

#include "utiltypedef.h"
#include "iPlaySysconfig.h"

int _setlocale(const BYTE* locale_name);
unsigned GetACP();
BOOL isLeadByte(BYTE ch);
int ToUTF16(const BYTE* pbByteString, WORD* pwWordBuffer, DWORD dwWordBufSize, WORD wDefaultChar);
int FromUTF16(const WORD* pwWordString, BYTE* pbByteBuffer, DWORD dwByteBufSize, BYTE bDefaultChar);
int Utf8ToUtf16BE(WORD* pwDest, const BYTE* pbSrc);
int SwapUniStringEndian(WORD *pwString);
void Utf8CharToUtf32Char(const BYTE* utf8, DWORD *pdwU32char, DWORD *pdwUtf8CharLength, int *pValid);

int wcslen(const WORD* wname);
WORD *wcscpy(WORD * dst, const WORD * src);
WORD *wcscat(WORD * dst, const WORD * src);
WORD *wcsncpy(WORD *dest, const WORD * source, DWORD count);
WORD *wcschr(const WORD * string, WORD ch);
int wcscmp(const WORD * src, const WORD * dst);
int wcsncmp(const WORD * first, const WORD * last, DWORD count);
WORD *wcsncat(WORD * front, const WORD * back, DWORD count);
WORD *wcsrchr (const WORD * string, WORD ch);
WORD *wcsstr(const WORD * wcs1, const WORD * wcs2);
int wcsicmp (const WORD * dst, const WORD * src);
int wcsnicmp (const WORD * first, const WORD * last, DWORD count);

int stricmp_(const BYTE * dst, const BYTE * src);
int strnicmp_(const BYTE * first, const BYTE * last, DWORD count);

#endif //CHARSET_H_BC661AD39A587CDF


