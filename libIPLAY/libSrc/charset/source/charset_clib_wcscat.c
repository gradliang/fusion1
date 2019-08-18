#include "utiltypedef.h"


WORD * wcscat(WORD * dst, const WORD * src)
{
	WORD * cp = dst;

	while( *cp )
		cp++;                   /* find end of dst */

	while( *cp++ = *src++ ) ;       /* Copy src to end of dst */

	return( dst );                  /* return dst */
}
