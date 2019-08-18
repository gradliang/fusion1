
#ifndef __HEAPUTIL_MEM_H__
#define __HEAPUTIL_MEM_H__


#include "UtilTypeDef.h"

#define MEM_ALIGNMENT           16
#define MEM_ALIGN_MASK          0xF

#undef MEM_PERF
#undef MEM_STATS

/*typedef unsigned char      u8_t;
typedef signed char        s8_t;
typedef unsigned short    u16_t;
typedef signed   short    s16_t;
typedef unsigned long    u32_t;
typedef long             s32_t;*/

#ifndef NULL
#define NULL ((void *)0)
#endif


// User memory area initial. The function will change main and extend heap size,
// it also initial both main and extend heap.
int User_mem_init();

// main heap access function, it is for AV function
//void mem_init(void);
void *mem_malloc(DWORD size);
void mem_free(void *rmem);
void *mem_reallocm(void *rmem, DWORD newsize);
DWORD mem_get_free_space();
DWORD mem_get_free_space_total();


// Extend heap access function, it is for not AV function
//void ext_mem_init(void);
void *ext_mem_malloc(DWORD size);
void ext_mem_free(void *rmem);
void *ext_mem_reallocm(void *rmem, DWORD newsize);
DWORD ext_mem_get_free_space();
int ext_mem_check();


// Kernel heap access function, it is for OS, driver and file system
void ker_mem_init(void);
void *ker_mem_malloc(DWORD size, BYTE taskId);
void ker_mem_free(void *rmem);
void *ker_mem_reallocm(void *rmem, DWORD newsize, BYTE taskid);
DWORD ker_mem_get_free_space();
int ker_mem_check();

#endif /* __HEAPUTIL_MEM_H__ */

