#ifndef _NDEBUG_H
#define _NDEBUG_H

//#define DEBUG_MM_LEAKAGE
#include <stdlib.h>
#include "mpTrace_copy.h"
void NetDie(const char *file,int line,const char *assertion);
#define NET_ASSERT(e)       (void)((e) ? 0 : (NetDie(__FILE__, __LINE__, #e),1))

/* check for 4-byte alignment */
#define MP_VALIDATE_POINTER(ptr)  \
    do \
    { \
        MP_ASSERT(ptr); \
        MP_ASSERT(((unsigned long)(ptr) & 3) == 0); \
    } while (0)

/* check for 4-byte alignment */
#define MP_VALIDATE_POINTER4(ptr)  MP_VALIDATE_POINTER(ptr)

/* check for 2-byte alignment */
#define MP_VALIDATE_POINTER2(ptr)  \
    do \
    { \
        MP_ASSERT(ptr); \
        MP_ASSERT(((unsigned long)(ptr) & 1) == 0); \
    } while (0)

/* check for */
#define MP_VALIDATE_POINTER3(ptr)  \
    do \
    { \
        MP_ASSERT(ptr); \
        MP_ASSERT(((unsigned long)(ptr) & 0xff000000) == 0x80000000); \
    } while (0)

//#define MP_DEBUG_FUNCTION
#ifdef MP_DEBUG_FUNCTION
#define MP_FUNCTION_ENTER() mpDebugPrint("--> %s\n", __func__)
#define MP_FUNCTION_EXIT()  mpDebugPrint("<-- %s %d\n", __func__, __LINE__)
#else
#define MP_FUNCTION_ENTER() 
#define MP_FUNCTION_EXIT()  
#endif

#ifndef __STRING
#define __STRING(x)    #x
#endif

#ifndef __STRINGSTRING
#define __STRINGSTRING(x) __STRING(x)
#endif

#ifndef __LINESTR__
#define __LINESTR__ __STRINGSTRING(__LINE__)
#endif

#ifndef __location__
#define __location__ __FILE__ ":" __LINESTR__
#endif

void mm_free(const void *ptr);

void *mm_mallocp(size_t size);
#ifdef DEBUG_MM_LEAKAGE
void *_mm_malloc(size_t size, const char *loc);
void *_mm_zalloc(size_t size, const char *loc);
void *_mm_realloc(void *ptr, size_t size, const char *loc);
#define mm_malloc(s)    _mm_malloc(s,__location__)
#define mm_zalloc(s)    _mm_zalloc(s,__location__)
#define mm_realloc(p,s)    _mm_realloc(p,s,__location__)
#else
void *mm_malloc(size_t size);
void *mm_zalloc(size_t size);
void *mm_realloc(void *ptr, size_t size);
#endif

#ifdef DEBUG_SKBUFF_LEAKAGE
#define net_buf_mem_malloc(sz) _net_buf_mem_malloc(sz, __location__)
void *_net_buf_mem_malloc(size_t size, char *loc);

#define netBufAlloc(sz) _netBufAlloc(sz, __location__)
struct sk_buff *_netBufAlloc(size_t sz, char *loc);

#define NetNewPacket(c) _NetNewPacket(c, __location__)
struct st_net_packet;
struct st_net_packet *_NetNewPacket(unsigned int c, char *loc);
#else
void *net_buf_mem_malloc(size_t size);
#endif

#define LOG_TO_SD    0
#define NET_TRACE_ENABLE    0

#if LOG_TO_SD
void log_printf(int level, char *fmt, ...);
void log_dump(char *address, int size);
void log_write(unsigned char *buffer, unsigned long size,unsigned char * strTag);
#if NET_TRACE_ENABLE > 0
void ntrace_write2sd(void);
#else
#define ntrace_write2sd() do {} while (0)
#endif
#else
#define log_printf(level,args...) do {} while (0)
#define log_dump(address, size) do {} while (0)
#define log_write(buffer, size, tag) do {} while (0)
#define ntrace_write2sd() do {} while (0)
#endif

#if NET_TRACE_ENABLE > 0
void ntrace_printf(int level, char *fmt, ...);
void ntrace_dump(int level, uint32_t address, uint32_t size);
void ntrace_write2flash(void * pHandle);
void ntrace_init(void);
void ntrace_reset(void);
void ntrace_show(void);
#else
#define ntrace_printf(level,fmt...) do {} while (0)
#define ntrace_dump(level, address, size) do {} while (0)
#define ntrace_write2flash(fp) do {} while (0)
#define ntrace_init() do {} while (0)
#define ntrace_reset() do {} while (0)
#define ntrace_show() do {} while (0)
#endif

void  mpDebugPrint(const char *fmt, ...);
#endif // _NDEBUG_H


