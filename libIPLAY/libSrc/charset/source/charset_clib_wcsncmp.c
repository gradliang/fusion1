#include "utiltypedef.h"

int wcsncmp(const WORD * first, const WORD * last, DWORD count)
{
	if (!count)
		return(0);

	while (--count && *first && *first == *last)
	{
		first++;
		last++;
	}

	return((int)(*first - *last));
}
