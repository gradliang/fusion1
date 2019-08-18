#include "utiltypedef.h"

int wcscmp(const WORD * src, const WORD * dst)
{
	int ret = 0 ;

	while( ! (ret = (int)(*src - *dst)) && *dst)
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}
