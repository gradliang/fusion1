#include "utiltypedef.h"

WORD * wcsrchr (const WORD * string, WORD ch)
{
	WORD *start = (WORD *)string;

	while (*string++)                       /* find end of string */
		;
                                                /* search towards front */
	while (--string != start && *string != (WORD)ch)
		;

	if (*string == (WORD)ch)             /* WORD found ? */
		return( (WORD *)string );

	return(NULL);
}
