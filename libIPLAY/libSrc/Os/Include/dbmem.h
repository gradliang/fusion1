
#ifndef __DEBUG_MEM_H__
#define __DEBUG_MEM_H__

//#define	a_malloc(a)			dm_ext_mem_malloc(a, __FILE__, __LINE__)
//#define 	a_realloc(a, b)		dm_ext_mem_reallocm(a, b, __FILE__, __LINE__)
//#define	a_free(a)			dm_ext_mem_free(a, __FILE__, __LINE__)


#include "flagdefine.h"

int MemPoolChk(BYTE mode);
int MemLeakChk(BYTE TaskID);
int MemLinkChk(BYTE mode);
int MemLinkRelink(BYTE mode);

void *dm_ext_mem_malloc(DWORD size, char* file, int line);
void *dm_ext_mem_calloc(DWORD element, DWORD size, char* file, int line);
void dm_ext_mem_free(void *rmem, char* file, int line);
void *dm_ext_mem_reallocm(void *rmem, DWORD newsize, char* file, int line);
void dm_dump_allo_chain(BYTE TaskId);

int dm_creat(BYTE TaskId);
int dm_release(BYTE TaskId);
int dm_init();

#endif /* __DEBUG_MEM_H__ */

