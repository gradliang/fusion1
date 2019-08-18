#include "utiltypedef.h"

WORD * wcsncat(WORD * front, const WORD * back, DWORD count)
{
	WORD *start = front;

	while (*front++)
                ;
	front--;

	while (count--)
	if (!(*front++ = *back++))
		return(start);

	*front = L'\0';
	return(start);
}
