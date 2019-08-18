#include "utiltypedef.h"

static WORD __ascii_towlower_w(WORD ch)
{
	if ( ch >= 0x41 && ch <= 0x5A )
		ch += 0x20;
	return ch;
}

int wcsicmp (const WORD * dst, const WORD * src)
{
	WORD f,l;

	do  {
		f = __ascii_towlower_w(*dst);
		l = __ascii_towlower_w(*src);
		dst++;
		src++;
	} while ( (f) && (f == l) );
	return (int)(f - l);
}

int wcsnicmp (const WORD * first, const WORD * last, DWORD count)
{
	WORD f,l;
	int result = 0;

	if(count)
	{
		do {
			f = __ascii_towlower_w(*first);
			l = __ascii_towlower_w(*last);
			first++;
			last++;
		} while ( (--count) && f && (f == l) );

		result = (int)(f-l);
	}

	return result;
}
