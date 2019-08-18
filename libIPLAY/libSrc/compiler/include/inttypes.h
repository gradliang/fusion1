
#ifndef _INTTYPES_H
#define _INTTYPES_H

#define __PRI64_PREFIX "ll"
#define PRIu64  __PRI64_PREFIX "u"
#define PRIx64  __PRI64_PREFIX "x"

/* Macros for printing format specifiers.  */

/* Decimal notation.  */
# define PRId8		"d"
# define PRId16		"d"
# define PRId32		"d"
# define PRId64		__PRI64_PREFIX "d"

#endif

