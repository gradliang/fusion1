#ifndef MP_TRACE_H
#define MP_TRACE_H

// To enable debug message
//  1. #define MP_DEBUG_GLOBAL_SWITCH    1
//  2. #define LOCAL_DEBUG_ENABLE 1 at top of local file before include mpTrace.h

/*
// Constant declarations
*/
#define MP_DEBUG_GLOBAL_SWITCH    1

#if (MP_DEBUG_GLOBAL_SWITCH && LOCAL_DEBUG_ENABLE)
	#define NDEBUG//CONFIG_ENABLE_DEBUG

	#define MP_DEBUG_ENABLE  1
	#define MP_TRACE_INSIGHT_ENABLE  1
	#define MP_TRACE_TIME_ENABLE 1

#else
    #undef CONFIG_ENABLE_DEBUG
	#define NDEBUG

	#define MP_DEBUG_ENABLE  0
	#define MP_TRACE_INSIGHT_ENABLE  0
	#define MP_TRACE_TIME_ENABLE 0
#endif

#include <stdarg.h>

void  mpPrintMessage(const char *cMsg);
void  mpClearDebugString();
void  mpDebugPrintN(const char *fmt, ...);
void  mpDebugPrint(const char *fmt, ...);
void  mpTraceTimeStart();
void mpDebugPrintChar(const char data);

#define	DONOTHING do { } while (0)

#if (MP_DEBUG_ENABLE && LOCAL_DEBUG_ENABLE)
    #define MP_DEBUG                        mpDebugPrint
	#define	MP_DEBUGN                       mpDebugPrintN
    #define MP_DEBUG1(a,b)                  mpDebugPrint(a,b)
    #define MP_DEBUG2(a,b,c)                mpDebugPrint(a,b,c)
    #define MP_DEBUG3(a,b,c,d)              mpDebugPrint(a,b,c,d)
    #define MP_DEBUG4(a,b,c,d,e)            mpDebugPrint(a,b,c,d,e)
    #define MP_DEBUG5(a,b,c,d,e,f)          mpDebugPrint(a,b,c,d,e,f)
    #define MP_DEBUG6(a,b,c,d,e,f,g)        mpDebugPrint(a,b,c,d,e,f,g)
    #define MP_DEBUG7(a,b,c,d,e,f,g,h)      mpDebugPrint(a,b,c,d,e,f,g,h)
    #define MP_DEBUG8(a,b,c,d,e,f,g,h,i)    mpDebugPrint(a,b,c,d,e,f,g,h,i)
    #define MP_DEBUG9(a,b,c,d,e,f,g,h,i,j)  mpDebugPrint(a,b,c,d,e,f,g,h,i,j)

	#define MP_TRACE_LINE()		            {mpDebugPrint("%s %d", __FILE__,__LINE__);}

	#define MP_TRACE_DOT()                  {mpDebugPrintChar('.');}
	#define MP_TRACE_CHAR(a)                {mpDebugPrintChar(a);}
	#define BREAK_POINT()                   {mpDebugPrint("break at file %s line %d", __FILE__,__LINE__);__asm("break 100"); }
#else
    #define MP_DEBUG(...)                   DONOTHING
	#define	MP_DEBUGN(...)                  DONOTHING
    #define MP_DEBUG1(a,b)                  DONOTHING
    #define MP_DEBUG2(a,b,c)                DONOTHING
    #define MP_DEBUG3(a,b,c,d)              DONOTHING
    #define MP_DEBUG4(a,b,c,d,e)            DONOTHING
    #define MP_DEBUG5(a,b,c,d,e,f)          DONOTHING
    #define MP_DEBUG6(a,b,c,d,e,f,g)        DONOTHING
    #define MP_DEBUG7(a,b,c,d,e,f,g,h)      DONOTHING
    #define MP_DEBUG8(a,b,c,d,e,f,g,h,i)    DONOTHING
    #define MP_DEBUG9(a,b,c,d,e,f,g,h,i,j)  DONOTHING
	#define MP_TRACE_LINE()				    DONOTHING

	#define MP_TRACE_DOT()                  DONOTHING
	#define MP_TRACE_CHAR(a)                DONOTHING
	#define BREAK_POINT()	                DONOTHING
#endif

#define MP_ALERT	mpDebugPrint
#define MP_ASSERT(exp) do{if (!(exp)) {mpDebugPrint("%s, %d, %s", __FILE__,__LINE__,#exp); }}while(0)


#if (MP_TRACE_TIME_ENABLE && LOCAL_DEBUG_ENABLE)
#define MP_TRACE_TIME_START                 mpTraceTimeStart
#define MP_TRACE_TIME_PRINT(a, b)           DONOTHING
#define MP_TRACE_TIME(func, delay)          { mpTraceTimeStart(); func; mpPrintTraceTime(#func, delay); }
#else
#define MP_TRACE_TIME_START()               DONOTHING
#define MP_TRACE_TIME_PRINT(a, b)           DONOTHING
#define MP_TRACE_TIME(a, b)                 DONOTHING
#endif

// temp add for some av debug
#define MP_DPF(fmt, ...)
#define MP_FDPF(fd, fmt, ...)
#define mp_msg(mod,lev, args... )

//print the elapse time from last peformance trace, which is different from MP_TRACE_TIME_ENABLE
#if MP_PERFORMANCE_TRACE
extern unsigned int LastSysTime, NowSysTime;

#define MP_PERFORMANCE_TRC(...)		        NowSysTime=GetSysTime();mpDebugPrint(__VA_ARGS__);LastSysTime=NowSysTime
#define MP_PERFORMANCE_TRCN(...)	        NowSysTime=GetSysTime();mpDebugPrintN(__VA_ARGS__);LastSysTime=NowSysTime
#define	ELAPSE_TIME					        NowSysTime-LastSysTime
#else
#define MP_PERFORMANCE_TRC(...)
#define MP_PERFORMANCE_TRCN(...)
#endif

#define	FLOAT1000(a)	(int)(a*1000)
#define TRACELN	mpDebugPrint("%s @ %s @ %d", __FILE__, __FUNCTION__, __LINE__)

#include "ndebug_copy.h"

#endif

