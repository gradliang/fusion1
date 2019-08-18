/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mad.h
*
* Programmer:    Brenda Li
*                MPX E120 division
*
* Created:   03/30/2005
*
* Description: 
*              
*        
* Change History (most recent first):
*     <1>     03/30/2005    Brenda Li    first file
****************************************************************
*/

# ifdef __cplusplus
extern "C" {
# endif

#define FPM_MIPS



# define SIZEOF_INT 4
# define SIZEOF_LONG 4
# define SIZEOF_LONG_LONG 8


/* Id: version.h,v 1.24 2003/05/27 22:40:37 rob Exp */

# ifndef LIBMAD_VERSION_H
# define LIBMAD_VERSION_H

# define MAD_VERSION_MAJOR  0
# define MAD_VERSION_MINOR  15
# define MAD_VERSION_PATCH  0
# define MAD_VERSION_EXTRA  " (beta)"

# define MAD_VERSION_STRINGIZE(str) #str
# define MAD_VERSION_STRING(num)    MAD_VERSION_STRINGIZE(num)

# define MAD_VERSION        MAD_VERSION_STRING(MAD_VERSION_MAJOR) "."  \
                MAD_VERSION_STRING(MAD_VERSION_MINOR) "."  \
                MAD_VERSION_STRING(MAD_VERSION_PATCH)  \
                MAD_VERSION_EXTRA

# define MAD_PUBLISHYEAR    "2000-2003"
# define MAD_AUTHOR     "Underbit Technologies, Inc."
# define MAD_EMAIL      "info@underbit.com"

extern char const mad_version[];
extern char const mad_copyright[];
extern char const mad_author[];
extern char const mad_build[];

# endif

/* Id: fixed.h,v 1.38 2004/02/17 02:02:03 rob Exp */
#include "fixed.h"

/* Id: bit.h,v 1.12 2004/01/23 09:41:32 rob Exp */
#include "bit.h"

/* Id: timer.h,v 1.16 2004/01/23 09:41:33 rob Exp */
#include "timer.h"

/* Id: stream.h,v 1.20 2004/02/05 09:02:39 rob Exp */
#include "stream.h"

/* Id: frame.h,v 1.20 2004/01/23 09:41:32 rob Exp */
#include "frame.h"

/* Id: synth.h,v 1.15 2004/01/23 09:41:33 rob Exp */
#include "synth.h"

/* Id: decoder.h,v 1.17 2004/01/23 09:41:32 rob Exp */


# ifdef __cplusplus
}
# endif
