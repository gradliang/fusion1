#include "utiltypedef.h"

WORD * wcsncpy(WORD *dest, const WORD * source, DWORD count)
{
	WORD *start = dest;

	while (count && (*dest++ = *source++))    /* copy string */
		count--;

	if (count)                              /* pad out with zeroes */
		while (--count)
			*dest++ = L'\0';

	return(start);
}
