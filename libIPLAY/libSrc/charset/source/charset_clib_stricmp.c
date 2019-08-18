#include "utiltypedef.h"

static BYTE __ascii_towlower(BYTE ch)
{
	if ( ch >= 0x41 && ch <= 0x5A )
		ch += 0x20;
	return ch;
}

int stricmp_(const BYTE * dst, const BYTE * src)
{
	BYTE f,l;
	do  {
		f = __ascii_towlower(*dst);
		l = __ascii_towlower(*src);
		dst++;
		src++;
	} while ( (f) && (f == l) );
	return (int)(f - l);
}

