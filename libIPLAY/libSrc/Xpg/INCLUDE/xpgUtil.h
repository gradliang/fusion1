//---------------------------------------------------------------------------

#ifndef xpg_UTIL_H__
#define xpg_UTIL_H__




#if defined(__cplusplus)
extern "C" {
#endif
//---------------------------------------------------------------------------
#include "xpgType.h"

void  xpgInitMemory(void);
void xpgResetBufSize(void);
void *xpgMalloc	( size_t size );
void *xpgCalloc	( size_t num, size_t size );
void  xpgFree	( void *memblock );
void  xpgFreeAll( void );

//---------------------------------------------------------------------------

#ifdef XPG_LOAD_FROM_FILE
XPGFILE *xpgFileOpen ( const char *filename, const char *mode );
size_t 	 xpgFileRead ( void *buffer, size_t size, size_t count, XPGFILE *stream );
char  	 xpgFileGetc ( XPGFILE *stream );
DWORD 	 xpgFileSeek ( XPGFILE *stream, DWORD offset, DWORD origin );
DWORD 	 xpgFileClose( XPGFILE *stream );
DWORD 	 xpgFileGetSize( XPGFILE *stream );
#else
XPGFILE *xpgFileOpen ( void *buffer, DWORD dwSize );
size_t 	 xpgFileRead ( void *buffer, size_t size, size_t count, XPGFILE *stream );
char  	 xpgFileGetc ( XPGFILE *stream );
DWORD 	 xpgFileSeek ( XPGFILE *stream, DWORD offset, DWORD origin );
DWORD 	 xpgFileClose( XPGFILE *stream );
DWORD 	 xpgFileGetSize( XPGFILE *stream );
#endif

extern DWORD *pdwXpgMem;
//---------------------------------------------------------------------------
#if defined(__cplusplus)
 }
#endif

#endif
