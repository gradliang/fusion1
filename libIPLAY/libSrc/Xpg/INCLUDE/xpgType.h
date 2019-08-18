//---------------------------------------------------------------------------

#ifndef xpg_TYPE_H__
#define xpg_TYPE_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "xpgDefine.h"
#ifdef XPG_612
	#include "UtilTypeDef.h"
	//#include "fs.h"
#else
	#include "TypeDef.h"
#endif
typedef DWORD XPGPIXEL;

/*
   Define the size_t type in the std namespace if in C++ or globally if in C.
   If we're in C++, make the _SIZE_T macro expand to std::size_t
*/

#if !defined(_SIZE_T) && !defined(_SIZE_T_DEFINED)
#  define _SIZE_T_DEFINED
   typedef unsigned int size_t;
#  if defined(__cplusplus)
#    define _SIZE_T std::size_t
#  else
#    define _SIZE_T size_t
#  endif
#endif


typedef unsigned char XPGBOOL;


#ifndef true
	#define true 1
#endif

#ifndef false
	#define false 0
#endif

#define XPG_ERR -1
#define XPG_OK 	 0

#ifdef XPG_LOAD_FROM_FILE
typedef STREAM XPGFILE;
#else
typedef struct {
	DWORD dwSize;
	DWORD dwPos;
	BYTE *pbBuffer;	
	BYTE *io_ptr;
} XPGFILE;
#endif

//SeekFile
#ifndef SEEK_SET
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif

//---------------------------------------------------------------------------
#if defined(__cplusplus)
 }
#endif

#endif
