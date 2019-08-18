#include "utiltypedef.h"

WORD * wcschr(const WORD * string, WORD ch)
{
	while (*string && *string != (WORD)ch)
		string++;

	if (*string == (WORD)ch)
		return((WORD *)string);
	return(NULL);
}
