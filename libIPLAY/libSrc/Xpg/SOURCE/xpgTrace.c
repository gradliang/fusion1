
//#ifdef _WINDOWS
#if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
#include <windows.h>
#else
#include <error.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#define  TRACE_ANYWAY
#include "xpgTrace.h"


// **************************************************************************
///
///@ingroup xpgTrace
///@brief   print debug string
///
///@param   pcOutputString - specified input string
///
void _TraceDebugString(BYTE * pcOutputString)
{
#ifndef _UNICODE
#  if defined(_WINDOWS) || defined(WIN32) || defined(_WIN32)
	OutputDebugString(pcOutputString);
#  else
	MP_FDPF(stderr, pcOutputString);
#  endif
#else

	TCHAR tcStr[500];

	mbstowcs(tcStr, pcOutputString, 499);
	tcStr[499] = 0;

	MP_FDPF(stderr, tcStr);
#endif
}

// **************************************************************************
///
///@ingroup xpgTrace
///@brief   Set input specified format then print string
///
///@param   fmt - specified format
///
void _TraceFormat(BYTE * fmt, ...)
{
	BYTE buff[1024];

	buff[0] = 0;
	mp_vsprintf(&buff[0], fmt, (BYTE *) (&fmt + 1));

	_TraceDebugString(buff);

}
