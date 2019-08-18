
#include "utiltypedef.h"

int strnicmp_(const BYTE * first, const BYTE * last, DWORD count )
{
    if(count)
    {
        int f=0;
        int l=0;

        do
        {

            if ( ((f = (unsigned char)(*(first++))) >= 'A') &&
                    (f <= 'Z') )
                f -= 'A' - 'a';

            if ( ((l = (unsigned char)(*(last++))) >= 'A') &&
                    (l <= 'Z') )
                l -= 'A' - 'a';

        }
        while ( --count && f && (f == l) );

        return ( f - l );
    }
    else
    {
        return 0;
    }
}


