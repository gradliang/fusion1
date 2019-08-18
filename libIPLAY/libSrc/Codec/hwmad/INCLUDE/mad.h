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

/* Id: fixed.h,v 1.36 2003/05/28 04:36:00 rob Exp */
#include "fixed.h"

/* Id: bit.h,v 1.11 2003/05/27 22:40:36 rob Exp */
#include "bit.h"

/* Id: timer.h,v 1.15 2003/05/27 22:40:37 rob Exp */
/* Id: stream.h,v 1.18 2003/05/27 22:40:37 rob Exp */
#include "libmad_stream.h"

#include "frame.h"

/* Id: synth.h,v 1.14 2003/05/27 22:40:37 rob Exp */

/* Id: decoder.h,v 1.16 2003/05/27 22:40:36 rob Exp */


# ifdef __cplusplus
}
# endif
