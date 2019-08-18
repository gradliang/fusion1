/*
 * Rename Linux's sys/types.h to sys/types2.h to avoid conflict w/ gcc's sys/types.h 
 */

#ifndef	_SYS_TYPES2_H
#define	_SYS_TYPES2_H	1

#include <features.h>

#include <bits/types.h>

/* But these were defined by ISO C without the first `_'.  */
#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef unsigned char u_int8_t;
typedef unsigned short int u_int16_t;
typedef unsigned int u_int32_t;
#endif

/* Now add the thread types.  */
#if defined __USE_POSIX199506 || defined __USE_UNIX98
# include <bits/pthreadtypes.h>
#endif

#endif /* _SYS_TYPES2_H */
