#ifndef __STRING_H
#define __STRING_H
/*
// Include section 
*/
#include	<stdarg.h>
#include	<stdlib.h>
//#include	<float.h>
#include	<math.h>

/*
// Constant declarations
*/

/*
// Structure declarations
*/

/*
// Type declarations
*/

/*
// Variable declarations
*/

/*
// Macro declarations
*/
/*
// Static function prototype
*/

/*
// Definition of external functions
*/

/*
// Definition of internal functions
*/
//extern int _vsprintf_(char *buf,const char *fmt,va_list args);
extern int mp_sprintf(char *buf,const char *fmt,...);
extern int mp_sscanf(const char * buf, const char * fmt, ...);
extern int mp_strcmp (const char * str1, const char * str2 );
extern int mp_strlen (const char* str);


/*
// Definition of local functions 
*/
#endif  // __STRING_H

