/*
 * @file
 *
 * MP52X OS wrapper header file
 *
 * Copyright (c) 2007 Magic Pixel Inc.
 * All rights reserved.
 */

#ifndef __OS_MP52X_H__
#define __OS_MP52X_H__

#include <stddef.h>
//#include "typedef.h"
#include "os.h"
#include "ndebug.h"

extern int atomic_sema;

#define	mpx_TaskStartup(id, ...)     \
({									   \
	int __ret;						   \
	__ret = TaskStartup(id, ## __VA_ARGS__); 		\
	__ret;								   \
})
#define	mpx_MessageReceive(id,target)     MessageReceive(id, target)
#define	mpx_MessageReceiveWto(id,target,tick)     MessageReceiveWithTO(id, target, tick)
#define	mpx_MessageReceiveX(id,target, max_size, tick)     MessageReceiveWithTO(id, target,tick)
#define	mpx_MessageDestroy(id)     

long	SemaphoreWait(unsigned char);
#define	mpx_SemaphoreWait(id)     SemaphoreWait(id)
#define	mpx_SemaphoreRelease(id)     SemaphoreRelease(id)

extern volatile DWORD TickCount;
#define	mpx_SystemTickerGet()     TickCount
#define	mpx_MessageDrop(id,source,size)     MessageDrop(id,source,size)
#define	mpx_TaskYield(tick)     TaskSleep(tick)

#define	mpx_Malloc(size)     mm_malloc(size)
#define	mpx_Free(mem)        mm_free(mem)
#define	mpx_Zalloc(size)      mm_zalloc(size)
#define	mpx_Realloc(ptr,size)     mm_realloc(ptr,size)

#define	mpx_EventWait(id,pattern,mode,release)  EventWait(id,pattern,mode,release)
#define	mpx_EventSet(id,pattern)  EventSet(id,pattern)
#define	mpx_EventClear(id,pattern)  EventClear(id,pattern)
#define	mpx_TimerPause(id)
#define	mpx_TimerResume(id)
#define	mpx_EventWaitWto(id,pattern,mode,release,tick)  EventWaitWithTO(id,pattern,mode,release,tick) 
#define	mpx_MessageSend(id,source,size)     MessageSend(id, source,size)
//#define	IntDisable()
//#define	IntEnable()


#define	UtilStringLength08(str)        strlen(str)

S32 mpx_SemaphoreCreate(U08 attr, U08 count);
S32 mpx_TimerCreate(void *handler, U08 resolution, U16 numerator, U16 denominator);

//
//Registry record data structure definition
//
typedef struct
{
    U08 u08Group[6];    ///< 6 bytes long group string tag
    U08 u08Name[6];     ///< 6 bytes long name string tag
    U32 u32Function;    ///< 32-bits registry data
} ST_REGISTRY;

#define OS_NULL                 (void *)0
#define OS_STATUS_OUT_OF_TIMER      -11

#define OS_TIMER_EMPTY              0       // this timer is not in used
#define OS_TIMER_READY              1       // this timer is linked to handler
#define OS_TIMER_RUNNING            2
#define OS_TIMER_PAUSED             3
#define OS_TIMER_STANDBY                        4

#ifndef MP600
#define OS_TIMER_BASE_FREQ          250
typedef struct
{
    U08     u08Id;
    U08     u08State;
    U08     u08OriginD;
    U08     u08Resolution;

    void    (*pClient) (void*);
    void    *pClientArg;
    void    *pNext;
    void    *pPrevious;
    void    *pHead;

    U32     u32Numerator;
    U32     u32Denominator;
    U32     u32Accumulator;

    U32     u32Residue;
} ST_TIMER;
#define OS_MAX_NUMBER_OF_TIMER		16
#elif defined(MP620)
typedef struct
{
	BYTE	bId;
	BYTE	bState;
	BYTE	Reserve[2];

	void	(*pClient)(void);
	void	*pNext;
	void	*pPrevious;
	void	*pHead;

	DWORD	dwNumerator;
	DWORD	dwDenominator;
	DWORD	dwAccumulator;

	DWORD	dwResidue;
} ST_TIMER;
#endif

///
///@ingroup REG
///@def     MPX_REGISTRY_SET(grouip, name, value)
///@brief   Register the registry record at link time.
///
///@param   group   the group tag string
///@param   name    the name tag string
///@param   value   the data portion of the registry record
///
///@remark  If the registry data is a point of a function. It is much
///         better to be located right after the function body.
///@remark  This macro can only be use in a file once. If the second or third invocation is need,
///         MPX_REGISTRY_2_SET and MPX_REGISTRY_3_SET can be used
///
#define MPX_REGISTRY_SET(group, name, value)\
        static ST_REGISTRY TempValuable __attribute__ ((section(".registry"))) = {(group), (name), (value)}

#define MPX_REGISTRY_2_SET(group, name, value)\
        static ST_REGISTRY TempValuable2 __attribute__ ((section(".registry"))) = {(group), (name), (value)}

#define MPX_REGISTRY_3_SET(group, name, value)\
        static ST_REGISTRY TempValuable3 __attribute__ ((section(".registry"))) = {(group), (name), (value)}
        
#define MPX_REGISTRY_4_SET(group, name, value)\
        static ST_REGISTRY TempValuable4 __attribute__ ((section(".registry"))) = {(group), (name), (value)}

#define MPX_REGISTRY_5_SET(group, name, value)\
        static ST_REGISTRY TempValuable5 __attribute__ ((section(".registry"))) = {(group), (name), (value)}

#define MPX_REGISTRY_6_SET(group, name, value)\
        static ST_REGISTRY TempValuable6 __attribute__ ((section(".registry"))) = {(group), (name), (value)}

ST_REGISTRY *mpx_RegistryNextGet(U08 *, U08 *, ST_REGISTRY *);
ST_REGISTRY *mpx_RegistryFirstGet(U08 *, U08 *);

void DPrintf(const U08 * format, ...);

#if 0
#ifndef BREAK_POINT
#define BREAK_POINT()		__asm("break 100")
//#define BREAK_POINT()
#endif
#endif

#endif //__OS_MP52X_H__
