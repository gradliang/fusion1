
#include "utiltypedef.h"

int wcslen(const WORD* wname)
{
	int len;
	len = 0;
	while(1)
	{
		if ( wname[len] == 0 )
			break;
		len++;
	}
	return len&0x7fffffff;
}
