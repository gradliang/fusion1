

#ifndef __XPG_TRACE_H_
#define __XPG_TRACE_H_

//--------------------------------------------------------------
// xpg trace function time
//--------------------------------------------------------------

#if 0
void xpgClearDebugString();
void xpgPrintMessage(const char *cMsg);
void xpgDebugPrint(const char *fmt, ...);



void xpgTraceTimeStart();
DWORD xpgPrintTraceTime(char *cMsg, int iDelayTime);  

#include "mpTrace.h"



#ifndef NDEBUG
#define XPG_TRACE_TIME_ENABLED
#define MP_DEBUG_ENABLED
#define XPG_ASSERT_ENABLED
#endif

#ifdef XPG_TRACE_TIME_ENABLED
#define MP_TRACE_TIME_START xpgTraceTimeStart
#define MP_TRACE_TIME_PRINT(msg, delay) xpgPrintTraceTime(msg, delay)
#define XPG_TRACE_TIME(func, delay) { xpgTraceTimeStart(); func; xpgPrintTraceTime(#func, delay); }
#else
#define MP_TRACE_TIME_START() {}
#define MP_TRACE_TIME_PRINT(a, b)  {}
#define XPG_TRACE_TIME(a, b, c) { a; }
#endif

#ifdef MP_DEBUG_ENABLED
#define MP_DEBUG(a) {xpgDebugPrint(a);}
#define MP_DEBUG_F(a) {a;}
#define XPG_DVALUE(a) xpgDebugPrint("%s=%d", #a, a)
#else
#define MP_DEBUG(a) {}
#define MP_DEBUG_F(a) {}
#define XPG_DVALUE(a) {}
#endif

#ifdef XPG_ASSERT_ENABLED
#define XPG_ASSERT(e) ((e) ? (void)0 : xpgDebugPrint("-E- !(%s)", #e))
#else
#define XPG_ASSERT(a) {}
#endif


#endif

#endif

