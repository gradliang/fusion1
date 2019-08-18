#ifndef MP_TRACE_H
#define MP_TRACE_H

#include "flagdefine.h"
#include "utiltypedef.h"

// To enable debug message
//  1. #define MP_DEBUG_GLOBAL_SWITCH    1
//  2. #define LOCAL_DEBUG_ENABLE 1 at top of local file before include mpTrace.h

/*
// Constant declarations
*/
#define MP_DEBUG_GLOBAL_SWITCH    1

#ifndef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 0
#endif

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
#include "../libsrc/bios/include/agent.h"
#include "../libsrc/bios/include/gdb.h"
#include "../libsrc/bios/include/uart.h"
#include "../libsrc/bios/include/exception.h"

void  mpPrintMessage(const char *cMsg);
void  mpDebugPrintN(const char *fmt, ...);
void  mpDebugPrint(const char *fmt, ...);
void  mpTraceTimeStart();
DWORD mpPrintTraceTime(char *cMsg, int iDelayTime);
void mpDebugPrintChar(const char data);
void mpDebugPrintCharDrop(const char data);
void mpDebugPrintValue(DWORD value, BYTE length);

#define	DONOTHING

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
#define MP_TRACE_TIME_PRINT(msg, delay)     mpPrintTraceTime(msg, delay)
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


///////////////////////////////////////////////////////////////////////////
//
//  Wrapper function
//
///////////////////////////////////////////////////////////////////////////
///
///@ingroup     UART_MODULE
///@brief       Common interface for HUART initial. (compatible with old platfrom for debug)
///
///@param       None
///
///@retval      None
///
///@remark      See \b HUartInit
///@remark      HUartInit(DEBUG_COM_PORT, ENABLE, HUART_BAUD)
void Uart_Init(void);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartOutText. (compatible with old platfrom)
///
///@param       buffer      A buffer pointer for send data.
///
///@remark      See \b HUartOutText.
///
void UartOutText(BYTE *buffer);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartOutValueHex. (compatible with old platfrom)
///
///@param       value       The number will be convert and send.
///@param       length      value width will be see.
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///
///@remark      See \b HUartOutValueHex.
///
void UartOutValue(DWORD value, BYTE length);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartGetChar. (compatible with old platfrom)
///
///@param       None
///
///@retval      <= 0xFFFF   Data
///@retval      -1          Time out.
///
///@remark      See \b HUartGetChar.
///
BYTE GetUartChar(void);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartPutChar. (compatible with old platfrom)
///
///@param       data        The char will be send.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartPutChar.
///
void PutUartChar(BYTE data);

void PutUartCharDrop(BYTE data);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartWaitStatus. (compatible with old platfrom)
///
///@param       dwStatus    Waitting condiion. Bits mapping to cause of HUART's interrupt.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartWaitStatus.
///@remark      HUartWaitStatus(DEBUG_COM_PORT, dwStatus, UART_WAIT_EMPTY_TIMEOUT)
int WaitUartStatus(DWORD dwStatus);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartWaitTxComplete. (compatible with old platfrom)
///
///@param       None
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartWaitTxComplete.
///@remark      HUartWaitTxComplete(DEBUG_COM_PORT)
void WaitUartTxComplete(void);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartWaitTxComplete. (compatible with old platfrom)
///
///@param       None
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartWaitTxComplete.
///
void EnaUartInt(void);

///
///@ingroup     UART_MODULE
///@brief       Common interface for SystemIntDis. (compatible with old platfrom)
///
///@param       None
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartWaitTxComplete.
///
void DisUartInt(void);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartBufferRead. (compatible with old platfrom)\n
///@brief       Using polling method and disable RTS/CTS.
///
///@param       buffer              A buffer pointer for restore data.
///@param       length              Specified data length will be read.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartBufferRead.
///@remark      HUartBufferRead(DEBUG_COM_PORT, buffer, length, RECV_THR_0, TRUE, DISABLE, 0xFF, DISABLE)
SDWORD UartRead(BYTE *buffer, DWORD length);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartBufferWrite. (compatible with old platfrom)\n
///@brief       Using polling method and disable RTS/CTS.
///
///@param       buffer      A buffer pointer for send data.
///@param       length      Specified data length will be send.
///
///@retval      PASS(0)     No error.
///@retval      FAIL(-1)    Time out.
///
///@remark      See \b HUartBufferWrite.
///@remakk      HUartBufferWrite(DEBUG_COM_PORT, buffer, length, TRUE, DISABLE)
SDWORD UartWrite(BYTE *buffer, DWORD length);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartDmaRead. (compatible with old platfrom)\n
///@brief       Using polling method and disable RTS/CTS.
///
///@param       buffer              A buffer pointer for restore data. It has 16-byte boundary limit.
///@param       length              Specified data length will be read. It must be multiplex of 16.
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///@retval      -2                  buffer point to a non 4-DWORD address.
///@retval      -3                  buffer length is not multiplex of 16.
///
///@remark      See \b HUartDmaRead.
///@remark      HUartDmaRead(DEBUG_COM_PORT, buffer, length, length, TRUE, DISABLE, 0xFF, DISABLE)
SDWORD UartDMARead(BYTE *buffer, DWORD length);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartDmaWrite. (compatible with old platfrom)\n
///@brief       Using polling method and disable RTS/CTS.
///
///@param       buffer              A buffer pointer for send data. It has 16-byte boundary limit.
///@param       length              Specified data length will be read. It must be multiplex of 16.
///
///@retval      PASS(0)             No error.
///@retval      FAIL(-1)            Time out.
///@retval      -2                  buffer point to a non 4-DWORD address.
///@retval      -3                  buffer length is not multiplex of 16.
///
///@remark      See \b HUartDmaWrite.
///@remark      HUartDmaWrite(DEBUG_COM_PORT, (BYTE *) buffer, length, TRUE, DISABLE)
SDWORD UartDMAWrite(BYTE *buffer, DWORD length);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartIntEnable. (compatible with old platfrom)\n
///
///@param       mask        The mask of HUART's interrupt.\n
///
///@retval      None
///
///@remark      See \b HUartIntEnable.
///@remark      HUartIntEnable(DEBUG_COM_PORT, mask)
void UartIntEnable(DWORD mask);

///
///@ingroup     UART_MODULE
///@brief       Common interface for HUartIntDisable. (compatible with old platfrom)\n
///
///@param       mask        The mask of HUART's interrupt.\n
///
///@retval      None
///
///@remark      See \b HUartIntDisable.
///@remark      HUartIntDisable(DEBUG_COM_PORT, mask)
void UartIntDisable(DWORD mask);

#endif

