
#include "utiltypedef.h"

WORD * wcscpy(WORD * dst, const WORD * src)
{
	WORD * cp = dst;

	while( *cp++ = *src++ )
		;               /* Copy src over dst */

	return( dst );
}
