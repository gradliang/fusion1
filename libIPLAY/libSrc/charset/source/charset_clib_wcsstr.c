#include "utiltypedef.h"

WORD * wcsstr(const WORD * wcs1, const WORD * wcs2)
{
	WORD *cp = (WORD *) wcs1;
	WORD *s1, *s2;

	if ( !*wcs2)
		return (WORD *)wcs1;

	while (*cp)
	{
		s1 = cp;
		s2 = (WORD *) wcs2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}
