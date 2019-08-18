

// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"



/* bcopy -- copy memory regions of arbitary length

NAME
	bcopy -- copy memory regions of arbitrary length

SYNOPSIS
	void bcopy (char *in, char *out, int length)

DESCRIPTION
	Copy LENGTH bytes from memory region pointed to by IN to memory
	region pointed to by OUT.

BUGS
	Significant speed improvements can be made in some cases by
	implementing copies of multiple bytes simultaneously, or unrolling
	the copy loop.

*/




void	bcopy (register char *src, register char * dest, int len)
 {

  if (dest < src)
    while (len--)
      *dest++ = *src++;
  else
    {
      char *lasts = src + (len-1);
      char *lastd = dest + (len-1);
      while (len--)
        *(char *)lastd-- = *(char *)lasts--;
    }
}


