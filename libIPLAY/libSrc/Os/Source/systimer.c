/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : systimer.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/


///
///@ingroup     OBJECT
///@defgroup    SYSTIMER        Timer
///
/// The timer module support a time-dependent processing. Applications can define
/// their own timer handlers without take care any H/W specifications.
///
/// All the timer can be identified by an ID number and have some operation states
/// for the convenience of controlling. A not-in-used timer has a timer state of EMPTY.
/// By assigning some attributes and a timer handler point with the service call
/// mpx_TimerCreate, it state will change to READY. In order to active this timer, the
/// service call mpx_TimerStart must be called. At this time, an execution time can be
/// specified. If the application want to have a permanant timer, a large execution
/// timer can be specified, like 0xffffffff. If the application just want to create
/// alarm and want it to asert once, a number 1 can be specified. Once the timer started,
/// another service calls can stop, pause the started timer. The timer state will change
/// to STOPED state and PAUSED state repectively. For the PAUSED state timer, service
/// call mpx_TimerResume can resume the timer and move it's state beck to RUNNING state.
///

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "corelib.h"
#include "mpTrace.h"

#include "taskid.h"
#include "osinner.h"
#include "osapi.h"
#include "bios.h"
#include "peripheral.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
///////////////////////////////////////////////////////////
// Start from 1970/1/1
#define BEGIN_YEAR      2000//1970
#define BEGIN_MONTH     1
#define BEGIN_DAY       1

#if (!defined(RTC_RESET_YEAR) || (RTC_RESET_YEAR < 1970))
    #undef RTC_RESET_YEAR
    #define RTC_RESET_YEAR          BEGIN_YEAR
#endif

#if (!defined(RTC_RESET_MONTH) || (RTC_RESET_MONTH > 12))
    #undef RTC_RESET_MONTH
    #define RTC_RESET_MONTH         BEGIN_MONTH
#endif

#if (!defined(RTC_RESET_DATE) || (RTC_RESET_DATE > 31))
    #undef RTC_RESET_DATE
    #define RTC_RESET_DATE          BEGIN_DAY
#endif

///
///@ingroup     SYSTIMER
///@brief       define the number of max timer proc.
#define MAX_SYS_TIMER_PROC_NUM      32

////////////////////////////////////////////////////////////
//
// Structure declarations
//
///////////////////////////////////////////////////////////

struct ST_SYS_TIMER_PROC_TAG
{
    struct ST_SYS_TIMER_PROC_TAG *sPrevTimerProc;
    struct ST_SYS_TIMER_PROC_TAG *sNextTimerProc;
    DWORD dwCounterBase;
    DWORD dwOffsetValue;
    void (*actionPtr)(void *);
    BOOL isOneShot;
    BOOL isRuning;
    void *actionArg;
};

struct ST_SYS_TIMER_PROC_LIST_TAG
{
    struct ST_SYS_TIMER_PROC_TAG *sHeadTimerProc;
    struct ST_SYS_TIMER_PROC_TAG *sRearTimerProc;
    DWORD dwCounter;
};

////////////////////////////////////////////////////////////
//
// Type declarations
//
///////////////////////////////////////////////////////////

typedef struct ST_SYS_TIMER_PROC_LIST_TAG   ST_SYS_TIMER_PROC_LIST;
typedef struct ST_SYS_TIMER_PROC_TAG        ST_SYS_TIMER_PROC;

////////////////////////////////////////////////////////////
//
// Variable declarations
//
///////////////////////////////////////////////////////////

volatile DWORD TickCount = 0;
static ST_SYS_TIMER_PROC        sysTimerProc[MAX_SYS_TIMER_PROC_NUM] = {0};
static ST_SYS_TIMER_PROC_LIST   sysTimerProcList = {0};

////////////////////////////////////////////////////////////
//
// Macro declarations
//
///////////////////////////////////////////////////////////

#define SYS_TIMER_PROC_ZERO     (&sysTimerProc[0])
#define SYS_TIMER_PROC_END      (&sysTimerProc[(MAX_SYS_TIMER_PROC_NUM - 1)])

////////////////////////////////////////////////////////////
//
// Function declarations
//
///////////////////////////////////////////////////////////
static void tm0Isr(void);

static void currTimerProcRemove(ST_SYS_TIMER_PROC *spTimerProc)
{
    ST_SYS_TIMER_PROC *spPrevTimerProc, *spNextTimerProc;

    MP_DEBUG("SYSTIMER currTimerProcRemove -");

    if (!spTimerProc->actionPtr)
    {
        MP_ALERT("--W-- current timer proc already removed !!!!");

        return;
    }

    spPrevTimerProc = spTimerProc->sPrevTimerProc;
    spNextTimerProc = spTimerProc->sNextTimerProc;

    if ((spPrevTimerProc == 0) && (spNextTimerProc == 0))
    {
        sysTimerProcList.sHeadTimerProc = 0;
        sysTimerProcList.sRearTimerProc = 0;
    }
    else if ((spPrevTimerProc == 0) && (spNextTimerProc != 0))
    {
        sysTimerProcList.sHeadTimerProc = spNextTimerProc;
        spNextTimerProc->sPrevTimerProc = 0;
    }
    else if ((spPrevTimerProc != 0) && (spNextTimerProc == 0))
    {
        sysTimerProcList.sRearTimerProc = spPrevTimerProc;
        spPrevTimerProc->sNextTimerProc = 0;
    }
    else //if ((spPrevTimerProc != 0) && (spNextTimerProc != 0))
    {
        spNextTimerProc->sPrevTimerProc = spPrevTimerProc;
        spPrevTimerProc->sNextTimerProc = spNextTimerProc;
    }

    memset((BYTE *) spTimerProc, 0, sizeof(ST_SYS_TIMER_PROC));
    sysTimerProcList.dwCounter--;
}



static void sysTimerProcessHandle(void)
{
    ST_SYS_TIMER_PROC *spTimerProc, *spNextTimerProc;
    void (*callBackFunPtr) (void *);
    void *arg;

    if (sysTimerProcList.dwCounter == 0)
        return;

    spTimerProc = (ST_SYS_TIMER_PROC *) sysTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        spNextTimerProc = spTimerProc->sNextTimerProc;

        if ( (spTimerProc->dwOffsetValue < SystemGetElapsedTime(spTimerProc->dwCounterBase)) &&
             spTimerProc->isRuning )
        {
            callBackFunPtr = spTimerProc->actionPtr;
            arg = spTimerProc->actionArg;

            if (spTimerProc->isOneShot)
            {
                MP_DEBUG("SysTimer Proc Handle = 0x%08X, Remain Timer Count = %02d", (DWORD) spTimerProc->actionPtr, sysTimerProcList.dwCounter - 1);
                currTimerProcRemove((ST_SYS_TIMER_PROC *) spTimerProc);
            }
            else
            {
                spTimerProc->dwCounterBase = GetSysTime();
            }

            if (callBackFunPtr)
                callBackFunPtr(arg);
        }

        spTimerProc = spNextTimerProc;
    }
}



static void sysTimerProcInit(void)
{
    memset((BYTE *) &sysTimerProc, 0, sizeof(ST_SYS_TIMER_PROC) * MAX_SYS_TIMER_PROC_NUM);

    sysTimerProcList.sHeadTimerProc = 0;
    sysTimerProcList.sRearTimerProc = 0;
    sysTimerProcList.dwCounter = 0;
}


#if (CHIP_VER_MSB == CHIP_VER_650)
#define TIMER0_CLOCK_SOURCE         E_TIMER_SOURCE_IS_OSC
#elif (CHIP_VER_MSB == CHIP_VER_615)
#define TIMER0_CLOCK_SOURCE         E_TIMER_SOURCE_IS_CPU
#else   // 66x
    #define TIMER0_CLOCK_SOURCE     E_TIMER_SOURCE_IS_AUX0
#endif

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
static BYTE cpuFreq;                // MHz
#endif

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_AUX0)
static BYTE pllFreq;                // MHz
#endif

static void initTimer0(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    DWORD tcb;

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    regClockPtr->TmClkC |= BIT1 | BIT0;         // select OSC as timer clock source
#elif (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    regClockPtr->TmClkC &= ~(BIT1 | BIT0);      // select CPU clock as timer clock source
#else
    regClockPtr->Aux0ClkC = BIT18 | BIT17;      // AUX0 = PLL2
    //regClockPtr->Aux0ClkC = BIT17;              // AUX0 = PLL1
    regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT1 | BIT0)) | BIT0;
#endif

    regClockPtr->MdClken |= CKE_TM0;

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & BIT0)
        regTimerPtr->TmC = 0;
#else
    regTimerPtr->TmC = 0;
#endif

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    tcb = MAIN_CRYSTAL_CLOCK / 1000 * SYS_TIMER_TPC_PERIOD / 1000;
#elif (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    tcb = Clock_CpuFreqGet() / 1000 * SYS_TIMER_TPC_PERIOD / 1000;
    cpuFreq = Clock_CpuFreqGet() / 1000000;
#else
    tcb = Clock_PllFreqGet(CLOCK_PLL2_INDEX) / 1000 * SYS_TIMER_TPC_PERIOD / 1000;
    pllFreq = Clock_PllFreqGet(CLOCK_PLL2_INDEX) / 1000000;
#endif

    regTimerPtr->TcB = tcb;
    regTimerPtr->TcA = tcb;
    regTimerPtr->Tpc = SYS_TIMER_TPC_MAX << 16;

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while ((regTimerPtr->TmC & BIT0) == 0)
        regTimerPtr->TmC = 0x00400003;
#else
    regTimerPtr->TmC = 0x00400003;
#endif

    MP_DEBUG("Timer0 -\r\nTcA = %d", regTimerPtr->TcA);
    MP_DEBUG("TcB = %d", regTimerPtr->TcB);
    MP_DEBUG("Tpc = 0x%08X", regTimerPtr->Tpc);
    MP_DEBUG("Tmc = 0x%08X", regTimerPtr->TmC);

    SystemIntHandleRegister(IM_TM0, tm0Isr);
    isr->MiMask |= IM_TM0;
}



//---------------------------------------------------------------------------

#ifdef __CHIP_VERIFY__
volatile BOOL enableSystemHeart = TRUE;
extern volatile DWORD uartModule;
#endif

#if ( (CHIP_VER == (CHIP_VER_650 | CHIP_VER_A)) || (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA)) )
extern volatile DWORD rtcTickCounter;
extern volatile DWORD rtcSecCounter;
#endif

static void tm0Isr(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;
    DWORD tmp;
    DWORD pcv, tick, tpc;

    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
    pcv = regTimerPtr->Tpc & 0xFFFF;

    if (pcv >= SYS_TIMER_TPC_MAX)
    {   // normal is >= SYS_TIMER_TPC_MAX
        tmp = pcv % SYS_TIMER_TPC_MAX;
        tick = pcv / SYS_TIMER_TPC_MAX;
    }
    else
    {   // overflow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / SYS_TIMER_TPC_MAX;
        tmp %= SYS_TIMER_TPC_MAX;
    }

    regTimerPtr->Tpc = (SYS_TIMER_TPC_MAX << 16) | tmp;
    TickCount += tick;

#if ( (CHIP_VER == (CHIP_VER_650 | CHIP_VER_A)) || (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA)) )
    rtcTickCounter += tick;
    rtcSecCounter += (rtcTickCounter / 250);
    rtcTickCounter %= 250;
#endif

    tick *= SYS_TIMER_TICK_PERIOD;
    SleepingTaskTimeTick(tick);
    TaskTimeSchedulingTimeTick(tick);

#ifdef __CHIP_VERIFY__
    if (((TickCount % SYS_TIMER_HEAP_PERIOD) == 0) && enableSystemHeart)
    {
        if (uartModule == HUART1_BASE)
            HUartPutChar(HUART_A_INDEX, '.');
        else
            HUartPutChar(HUART_B_INDEX, '.');
    }
    else if (!enableSystemHeart)
        mpDebugPrintCharDrop('x');
#else
    #if 0//((Make_TESTCONSOLE == 0) && (Make_DIAGNOSTIC_TC == 0))
    if ((TickCount % SYS_TIMER_HEAP_PERIOD) == 0)
        //mpDebugPrintChar('.');
        mpDebugPrintCharDrop('.');
    #endif
#endif

    sysTimerProcessHandle();
}



void SystemTimerPause(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;

    IntDisable();
    regTimerPtr->TmC &= ~BIT0;
    IntEnable();
}



void SystemTimerResume(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;

    IntDisable();
    regTimerPtr->TmC |= BIT0;
    IntEnable();
}



//---------------------------------------------------------------------------
//             function for get system timer
//---------------------------------------------------------------------------
///
///@ingroup SYSTIMER
///@brief   Get status of the system timer
///
///@param   NONE
///
///@retval  Status of system timer. 1: Runing, 0: Stoping
///
///@remark
///
BOOL SysTimerStatusGet(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;

    return (BOOL) (regTimerPtr->TmC & BIT0);
}



///
///@ingroup SYSTIMER
///@brief   Get the system tick time ( unit : ms, min resolution is 4ms )
///
///@param   NONE
///
///@retval  The system time in the unit of ms.
///
///@remark  Note that, we use 32bit for system timer. That means if system power on for
///         a long time, the timer counter will overflow and count from zero again.
///
DWORD GetSysTime(void)
{
    return (TickCount * SYS_TIMER_TICK_PERIOD);
}



///
///@ingroup SYSTIMER
///@brief   Get the system time ( unit : ms )
///
///@param   NONE
///
///@retval  The system time in the unit of ms.
///
///@remark  Note that, we use 32bit for system timer. That means if system power on for
///         a long time, the timer counter will overflow and count from zero again.
///
DWORD SystemGetTimeStamp(void)
{
    return GetSysTime();
}



///
///@ingroup SYSTIMER
///@brief   Get the intervial between start time(input parameter) and current time ( unit : ms )
///
///@param   startTime   The start time of the elapse. ( unit : ms )
///
///@retval  The intervial between start time(input parameter) and current time ( unit : ms )
///
DWORD SystemGetElapsedTime(DWORD startTime)
{
    DWORD endTime = GetSysTime();

    if (startTime > endTime)
    {
        startTime = 0xFFFFFFFF - startTime + 1;
        return (startTime + endTime);
    }
    else
    {
        return (endTime - startTime);
    }
}



///
///@ingroup SYSTIMER
///@brief   System timer initial
///
///@param   None
///
///@retval  NONE
///
///@remark  We use timer0 as the system timer. The function call initial timer0
///         interrupt period to SYS_TIMER_TICK_PERIOD(4) ms. Please do not change the period of timer0
///         nor use timer0 as other purpos. Because that will cause system abnormal
///         operation.
///@remark  In the function call we also reset system timer counter and clear all timer process
///         added before.
///
void SystemTimerInit(void)
{
    TickCount = 0;
    sysTimerProcInit();
    initTimer0();
}



///
///@ingroup SYSTIMER
///@brief   System timer reinitial (When CPU's clock was changed)
///
///@param   None
///
///@retval  NONE
///
///@remark  If timer clock is depended on CPU clock, so, timer needs re-initial when CPU clock was changed.
///
void SystemTimerReInit(void)
{
#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    initTimer0();
#endif
}




///
///@ingroup SYSTIMER
///@brief   Set timer handler argument
///
///@param   actionPtr       Callback function pointer
///@param   arg             The argument of the timer handler
///
///@retval  OS_STATUS_OK    The argument has set the handler successfully.
///@retval  -1              Callback function is null pointer.
///@retval  -2              The input pointer is no matched.
///
///@remark  This function set the a point of a buffer that contain all the information
///         for the timer handler at the calling time. Refer to the SysTimerProcAdd for more
///         information.
///
SDWORD SysTimerProcArgSet(void *actionPtr, void *arg)
{
    ST_SYS_TIMER_PROC *spTimerProc;

    if (!actionPtr)
    {
        MP_ALERT("--E-- %s - Null pointer!!!!", __FUNCTION__);

        return -1;
    }

    IntDisable();
    MP_DEBUG("SysTimerProcArgSet -");
    spTimerProc = sysTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if ((DWORD) spTimerProc->actionPtr == (DWORD) actionPtr)
        {
            spTimerProc->actionArg = arg;
            IntEnable();

            return OS_STATUS_OK;
        }

        spTimerProc = spTimerProc->sNextTimerProc;
    }

    MP_ALERT("SysTimerProcArgSet - Can't found time proc - 0x%08X!!!", (DWORD) actionPtr);
    IntEnable();

    return -1;
}



///
///@ingroup SYSTIMER
///@brief   Add a procedure to system timer by specified time period.
///
///@param   dwOffsetValue       How long will be runing. It will be align to multiplex of SYS_TIMER_TICK_PERIOD(4).
///@param   actionPtr           Callback function pointer when time up.
///@param   isOneShot           TRUE(1): Callback function will be runing only once, and will be remove by self.
///                             FALSE(0): Callback function will be runing per dwOffsetValue ms.
///
///@retval  timer ID            >= 0
///@retval  -1                  Procedure array is full or callback function is null pointer.
///@retval  -2                  Procedure array is full.
///
///@remark  The timer handler should can have argument with type of (void *). The caller
///         can use this point to specify a argument buffer to save all the arguments
///         that the timer handler need. The SysTimerProcArgSet function is used to set this
///         point argument. In just created timer the handler argument field will contain
///         a null point and will not transfer any argument to the handler.
///
SDWORD SysTimerProcAdd(DWORD dwOffsetValue, void *actionPtr, BOOL isOneShot)
{
    return SysTimerProcReAdd(dwOffsetValue, actionPtr, isOneShot, FALSE);
}



///
///@ingroup SYSTIMER
///@brief   Remove a procedure from system timer by specified callback function.
///
///@param   actionPtr           Callback function pointer will be remove.
///
///@retval  OS_STATUS_OK
///@retval  -1                  Callback function is null pointer.
///@retval  -2                  Can't found time proc.
///
///@remark
///
SDWORD SysTimerProcRemove(void *actionPtr)
{
    ST_SYS_TIMER_PROC *spTimerProc;

    if (!actionPtr)
    {
        MP_ALERT("--E-- %s - Null pointer!!!!", __FUNCTION__);

        return -1;
    }

    IntDisable();

    MP_DEBUG("%s -", __FUNCTION__);

    //get remove process
    spTimerProc = sysTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if ((DWORD) spTimerProc->actionPtr == (DWORD) actionPtr)
        {
            currTimerProcRemove(spTimerProc);

            MP_DEBUG("Remove Timer Proc Addr = 0x%08X, Remain Timer Count = %02d", (DWORD) actionPtr, sysTimerProcList.dwCounter);
            IntEnable();

            return OS_STATUS_OK;
        }

        spTimerProc = spTimerProc->sNextTimerProc;
    }

    MP_ALERT("--E-- %s - Can't found time proc - 0x%08X!!!", __FUNCTION__, (DWORD) actionPtr);
    IntEnable();

    return -2;
}



///
///@ingroup SYSTIMER
///@brief   Stop runing of specifing callback function.
///
///@param   actionPtr   Callback function pointer will be remove.
///
///@retval  OS_STATUS_OK
///@retval  -1          Callback function is null pointer.
///@retval  -2          Can't found time proc.
///
///@remark
///
SDWORD SysTimerProcPause(void *actionPtr)
{
    ST_SYS_TIMER_PROC *spTimerProc, *spPrevTimerProc, *spNextTimerProc;
    DWORD i;

    if (!actionPtr)
    {
        MP_ALERT("--E-- %s - Null pointer!!!!", __FUNCTION__);

        return -1;
    }

    IntDisable();

    //get remove process
    spTimerProc = sysTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if ((DWORD) spTimerProc->actionPtr == (DWORD) actionPtr)
        {
            spTimerProc->isRuning = FALSE;
            IntEnable();

            return OS_STATUS_OK;
        }

        spTimerProc = spTimerProc->sNextTimerProc;
    }

    IntEnable();

    return -2;
}



///
///@ingroup SYSTIMER
///@brief   Resume system timer by specified callback function pointer.
///
///@param   actionPtr   Callback function pointer will be remove.
///
///@retval  OS_STATUS_OK
///@retval  -1          Callback function is null pointer.
///@retval  -2          Can't found time proc.
///
///@remark
///
SDWORD SysTimerProcResume(void *actionPtr)
{
    ST_SYS_TIMER_PROC *spTimerProc, *spPrevTimerProc, *spNextTimerProc;
    DWORD i;

    if (!actionPtr)
    {
        MP_ALERT("--E-- %s - Null pointer!!!!", __FUNCTION__);

        return -1;
    }

    IntDisable();

    //get remove process
    spTimerProc = sysTimerProcList.sHeadTimerProc;

    while (spTimerProc != 0)
    {
        if ((DWORD) spTimerProc->actionPtr == (DWORD) actionPtr)
        {
            spTimerProc->isRuning = TRUE;
            spTimerProc->dwCounterBase = GetSysTime();
            IntEnable();

            return OS_STATUS_OK;
        }

        spTimerProc = spTimerProc->sNextTimerProc;
    }

    IntEnable();

    return -2;
}



///
///@ingroup SYSTIMER
///@brief   Add a procedure to system timer by specified time period.
///
///@param   dwOffsetValue       How long will be runing. It will be align to multiplex of SYS_TIMER_TICK_PERIOD(4).
///@param   actionPtr           Callback function pointer when time up.
///@param   isOneShot           TRUE(1): Callback function will be runing only once, and will be remove by self.
///@param   validReAdd          TRUE(1): Valid readd timer proc when actionPtr was existed.
///                             FALSE(0): Invalid readd timer proc when actionPtr was existed. Change it's period.
///
///@retval  timer ID            >= 0
///@retval  -1                  Procedure array is full or callback function is null pointer.
///@retval  -2                  Procedure array is full.
///
///@remark  The timer handler should can have argument with type of (void *). The caller
///         can use this point to specify a argument buffer to save all the arguments
///         that the timer handler need. The SysTimerProcArgSet function is used to set this
///         point argument. In just created timer the handler argument field will contain
///         a null point and will not transfer any argument to the handler.
///
SDWORD SysTimerProcReAdd(DWORD dwOffsetValue, void *actionPtr, BOOL isOneShot, BOOL validReAdd)
{
    ST_SYS_TIMER_PROC *spTimerProc;
    DWORD i;

    MP_DEBUG("%s -", __FUNCTION__);

    if (!actionPtr)
    {
        MP_ALERT("--E-- %s - callback is null pointer !!!!\r\n", __FUNCTION__);

        return -1;
    }

    if ((sysTimerProcList.dwCounter >= MAX_SYS_TIMER_PROC_NUM) || !actionPtr)
    {
        MP_ALERT("--E-- %s - Proc. array is full !!!!\r\n", __FUNCTION__);

        return -2;
    }

    IntDisable();

    if (!validReAdd)
    {
        //change period when the callback was existed
        spTimerProc = SYS_TIMER_PROC_ZERO;

        for (i = 0; i < MAX_SYS_TIMER_PROC_NUM; i++)
        {
            if ((DWORD) spTimerProc->actionPtr == (DWORD) actionPtr)
            {
                spTimerProc->dwOffsetValue = dwOffsetValue;
                spTimerProc->dwCounterBase = GetSysTime();
                MP_ALERT("--W-- %s - Callback Proc. already existed, just change time period !!!\r\n", __FUNCTION__);
                IntEnable();

                return i;
            }

            spTimerProc++;
        }
    }

    //get free timer process list
    spTimerProc = SYS_TIMER_PROC_ZERO;

    for (i = 0; i < MAX_SYS_TIMER_PROC_NUM; i++)
    {
        if (spTimerProc->actionPtr == 0)
        {
            break;
        }

        spTimerProc++;
    }

    if (i == MAX_SYS_TIMER_PROC_NUM)
    {
        MP_ALERT("--E-- %s - Can't found the empty proc !!!!\r\nThis case should not happened !!!\r\n", __FUNCTION__);
        IntEnable();

        return -2;
    }

    // change link
    sysTimerProcList.dwCounter++;

    if (sysTimerProcList.sHeadTimerProc == 0)
    {   // First
        sysTimerProcList.sHeadTimerProc = spTimerProc;
        spTimerProc->sPrevTimerProc = 0;
    }
    else
    {   // Add
        spTimerProc->sPrevTimerProc = sysTimerProcList.sRearTimerProc;
        sysTimerProcList.sRearTimerProc->sNextTimerProc = spTimerProc;
    }

    sysTimerProcList.sRearTimerProc = spTimerProc;
    spTimerProc->sNextTimerProc = 0;
    spTimerProc->actionPtr = actionPtr;
    spTimerProc->dwOffsetValue = dwOffsetValue;
    spTimerProc->dwCounterBase = GetSysTime();
    spTimerProc->isOneShot = isOneShot;
    spTimerProc->isRuning = TRUE;
    spTimerProc->actionArg = (void *) 0;

    IntEnable();

    return i;
}



///
///@ingroup SYSTIMER
///@brief   Remove a procedure from system timer by timer ID.
///
///@param   timerId     The index of timer ID.
///
///@retval  OS_STATUS_OK
///@retval  -1          Wrong timer ID
///
///@remark
///
SDWORD SysTimerProcRemoveById(DWORD timerId)
{
    ST_SYS_TIMER_PROC *spTimerProc;

    if (timerId >= MAX_SYS_TIMER_PROC_NUM)
        return -1;

    IntDisable();

    MP_DEBUG("%s -", __FUNCTION__);

    spTimerProc = (ST_SYS_TIMER_PROC *) &sysTimerProc[timerId];
    currTimerProcRemove(spTimerProc);
    MP_DEBUG("Remove Timer Proc Addr = 0x%08X, Remain Timer Count = %02d", (DWORD) spTimerProc, sysTimerProcList.dwCounter);
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup SYSTIMER
///@brief   Stop runing timer of specifing timer ID.
///
///@param   timerId     The index of timer ID.
///
///@retval  OS_STATUS_OK
///@retval  -1          Wrong timer ID
///
///@remark
///
SDWORD SysTimerProcPauseById(DWORD timerId)
{
    ST_SYS_TIMER_PROC *spTimerProc, *spPrevTimerProc, *spNextTimerProc;
    DWORD i;

    if (timerId >= MAX_SYS_TIMER_PROC_NUM)
        return -1;

    IntDisable();
    //get remove process
    spTimerProc = (ST_SYS_TIMER_PROC *) &sysTimerProc[timerId];
    spTimerProc->isRuning = FALSE;
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup SYSTIMER
///@brief   Resume system timer by specified timer ID.
///
///@param   timerId     The index of timer ID.
///
///@retval  OS_STATUS_OK
///@retval  -1          Wrong timer ID
///
///@remark
///
SDWORD SysTimerProcResumeById(DWORD timerId)
{
    ST_SYS_TIMER_PROC *spTimerProc, *spPrevTimerProc, *spNextTimerProc;
    DWORD i;

    if (timerId >= MAX_SYS_TIMER_PROC_NUM)
        return -1;

    IntDisable();
    //get remove process
    spTimerProc = (ST_SYS_TIMER_PROC *) &sysTimerProc[timerId];
    spTimerProc->isRuning = TRUE;
    spTimerProc->dwCounterBase = GetSysTime();
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup SYSTIMER
///@brief   Set timer handler argument
///
///@param   timerId     The index of timer ID.
///@param   arg         The argument of the timer handler
///
///@retval  OS_STATUS_OK    The argument has set the handler successfully.
///@retval  -1              Wrong timer ID
///
///@remark  This function set the a point of a buffer that contain all the information
///         for the timer handler at the calling time. Refer to the SysTimerProcAdd for more
///         information.
///
SDWORD SysTimerProcArgSetById(DWORD timerId, void *arg)
{
    ST_SYS_TIMER_PROC *spTimerProc;

    MP_DEBUG("%s -", __FUNCTION__);

    if (timerId >= MAX_SYS_TIMER_PROC_NUM)
        return -1;

    IntDisable();
    spTimerProc = (ST_SYS_TIMER_PROC *) &sysTimerProc[timerId];
    spTimerProc->actionArg = arg;
    IntEnable();

    return OS_STATUS_OK;
}




////////////////////////////////////////////////////////////////////////
///         System Clock Function Definition
////////////////////////////////////////////////////////////////////////
static const BYTE DayPerMonth[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                                        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

#define DAY_2_SECOND    (24 * 60 * 60)

#if (BEGIN_MONTH <= 2)
#define BEGIN_DATE      (1461 * (BEGIN_YEAR - 1) / 4 + 153 * (BEGIN_MONTH + 13) / 5 + BEGIN_DAY)
#else
#define BEGIN_DATE      (1461 * BEGIN_YEAR / 4 + 153 * (BEGIN_MONTH + 1) / 5 + BEGIN_DAY)
#endif

// return 0 mean not leap year
// return 1 mean leap year
DWORD LeapYearCheck(register WORD year)
{
    if ( !(year % 4) && ((year % 100) || !(year % 400)) )
        return 1;
    else
        return 0;
#if 0//Wesley, it's a bug
    if(year & 0x03)     return 0;   // if not multiple of 4 then not leap year;
    if(!(year % 400))   return 1;   // if multiple of 400 then leap year;
    if(year % 100)      return 0;   // if not multiple of 400 but multiple of 100 then not leap year
    return  1;                      // else is leap year
#endif
}


///
///@ingroup SYSTIMER
///@brief   Convert system time to second count.
///
///@param   time    Point of the date-time object that need to be checked
///
///@retval  second count
///
///@remark  The base of system time is 1970/1/1 00:00:00
///
DWORD SystemTimeDateToSecConv(ST_SYSTEM_TIME *time)
{
    DWORD f, g;

    if (time->u08Month <= 2)
       f = time->u16Year - 1;
    else
       f = time->u16Year;

    if (time->u08Month <= 2)
       g = time->u08Month + 13;
    else
       g = time->u08Month + 1;

    return ((1461 * f / 4 + 153 * g / 5 + time->u08Day - BEGIN_DATE) * DAY_2_SECOND +
            time->u08Hour * 60 * 60 +
            time->u08Minute * 60 + time->u08Second);
}



///
///@ingroup SYSTIMER
///@brief   Convert second count to system time.
///
///@param   second      Second count will be convert.
///@param   *time       The result of system time
///
///@retval  None
///
///@remark  The base of system time is 1970/1/1 00:00:00
///
void SystemTimeSecToDateConv(DWORD second, ST_SYSTEM_TIME *time)
{
    DWORD day;

    time->u32TimeStamp = second;                    // grad add

    day = (second / DAY_2_SECOND);
    time->u08Hour = (second % DAY_2_SECOND) / (60 * 60);
    second = (second % DAY_2_SECOND) % (60 * 60);
    time->u08Minute = second / 60;
    time->u08Second = second % 60;

    time->u16Year = BEGIN_YEAR;
    time->u08Month = BEGIN_MONTH;
    time->u08Day = BEGIN_DAY;

    while (day--)
    {
        time->u08Day++;

        if (time->u08Day > DayPerMonth[LeapYearCheck(time->u16Year)][time->u08Month])
        {
            time->u08Day = 1;
            time->u08Month++;

            if (time->u08Month > 12)
            {
                time->u08Month = 1;
                time->u16Year++;
            }
        }
    }
}



///
///@ingroup SYSTIMER
///@brief   Check if the specified date-time object is valid.
///
///@param   time    Point of the date-time object that need to be checked
///
///@retval  0       NO ERROR
///@retval  -1      The input date-time is not valid.
///
///@remark
///
SDWORD SystemTimeCheck(ST_SYSTEM_TIME *time)
{
    register DWORD index;

    if ( (time->u08Second >= 60) || (time->u08Minute >= 60) || (time->u08Hour >= 24) )
        return -1;

    if ( (time->u08Month > 12) || (time->u08Month == 0) )
        return -1;

    index = LeapYearCheck(time->u16Year);

    if (time->u08Day > DayPerMonth[index][time->u08Month] || time->u08Day == 0)
        return -1;

    return 0;
}

BYTE bGetMaxDay(WORD wYear,BYTE bMonth)
{
	return  DayPerMonth[LeapYearCheck(wYear)][bMonth];
}

///
///@ingroup SYSTIMER
///@brief   Set the system time by a new date-time object.
///
///@param   new_time    Point of the new date-time object.
///
///@retval  None
///
///@remark
///
void SystemTimeSet(ST_SYSTEM_TIME *new_time)
{
    RTC_SetCount(SystemTimeDateToSecConv(new_time));

    MP_DEBUG("SystemTimeSet = 0x%08X", SystemTimeDateToSecConv(new_time));
    MP_DEBUG("Date: %4d-%2d-%2d", new_time->u16Year, new_time->u08Month, new_time->u08Day);
    MP_DEBUG("Time: %2d:%2d:%2d", new_time->u08Hour, new_time->u08Minute, new_time->u08Second);

    if (SystemTimeDateToSecConv(new_time) != RTC_ReadCount())
    {
        MP_ALERT("--E-- Set system time fail !!!!");
    }
}



///
///@ingroup SYSTIMER
///@brief   Get the current system time.
///
///@param   curr_time    Point of the date-time object that will contain the returnned system date-time.
///
///@retval  None
///
///@remark
///
void SystemTimeGet(ST_SYSTEM_TIME *curr_time)
{
    SystemTimeSecToDateConv(RTC_ReadCount(), curr_time);

    MP_DEBUG("SystemTimeGet = 0x%08X", RTC_ReadCount());
    MP_DEBUG("Date: %4d-%2d-%2d", curr_time->u16Year, curr_time->u08Month, curr_time->u08Day);
    MP_DEBUG("Time: %2d:%2d:%2d", curr_time->u08Hour, curr_time->u08Minute, curr_time->u08Second);
}



///
///@ingroup SYSTIMER
///@brief   Set the system time by a new date-time object.
///
///@param   new_time    Point of the new date-time object.
///
///@retval  None
///
///@remark
///
void SystemTimeAlarmSet(ST_SYSTEM_TIME *new_time)
{
    RTC_AlarmSet(SystemTimeDateToSecConv(new_time));

    MP_DEBUG("SystemTimeAlarmSet = 0x%08X", SystemTimeDateToSecConv(new_time));
    MP_DEBUG("Date: %4d-%2d-%2d", new_time->u16Year, new_time->u08Month, new_time->u08Day);
    MP_DEBUG("Time: %2d:%2d:%2d", new_time->u08Hour, new_time->u08Minute, new_time->u08Second);

    if (SystemTimeDateToSecConv(new_time) != RTC_ReadAlarm())
    {
        MP_ALERT("--E-- Set system alarm time fail !!!!");
    }
}



///
///@ingroup SYSTIMER
///@brief   Get the current system time.
///
///@param   curr_time    Point of the date-time object that will contain the returnned system date-time.
///
///@retval  None
///
///@remark
///
void SystemTimeAlarmGet(ST_SYSTEM_TIME *curr_time)
{
    SystemTimeSecToDateConv(RTC_ReadAlarm(), curr_time);

    MP_DEBUG("SystemTimeAlarmGet = 0x%08X", RTC_ReadCount());
    MP_DEBUG("Date: %4d-%2d-%2d", curr_time->u16Year, curr_time->u08Month, curr_time->u08Day);
    MP_DEBUG("Time: %2d:%2d:%2d", curr_time->u08Hour, curr_time->u08Minute, curr_time->u08Second);
}



//#define __ENABLE_MEASURE_GET_ELAPSED_TIME_PERFORMANCE

void get_cur_timeL(DWORD *pDtick, DWORD *pDtpc, DWORD *pDtmv)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;

    if ((regTimerPtr->TmC & BIT0) == 0)
    {
        *pDtmv = 0;
        *pDtpc = 0;
        *pDtick = 0;

        return;
    }

    *pDtmv = regTimerPtr->TmV;
    *pDtpc = regTimerPtr->Tpc;
    *pDtick = TickCount;

#if (CHIP_VER_MSB == CHIP_VER_615)
    *pDtpc &= 0xFF;
#else
    *pDtpc &= 0xFFFF;
#endif
}



DWORD get_elapsed_timeL(DWORD Dtick0, DWORD Dtpc0, DWORD Dtmv0)
{
    TIMER *regTimerPtr = (TIMER *) TIMER0_BASE;
    DWORD Dtick1, Dtpc1, Dtmv1;
    DWORD elapsedTime;   // us
#ifdef __ENABLE_MEASURE_GET_ELAPSED_TIME_PERFORMANCE
    DWORD tmv1, tmv2, tmv3;
    DWORD tpc1, tpc2, tpc3;
    DWORD a, b;
#endif

    if (SysTimerStatusGet() == FALSE)
        return 0;

#ifdef __ENABLE_MEASURE_GET_ELAPSED_TIME_PERFORMANCE
    tmv1 = regTimerPtr->TmV;
    tpc1 = regTimerPtr->Tpc;
#endif

    Dtmv1 = regTimerPtr->TmV;
    Dtpc1 = regTimerPtr->Tpc & 0xFFFF;
    Dtick1 = TickCount;

#ifdef __ENABLE_MEASURE_GET_ELAPSED_TIME_PERFORMANCE
    tmv2 = regTimerPtr->TmV;
    tpc2 = regTimerPtr->Tpc;
#endif

    if (Dtick1 >= Dtick0)
    {
        elapsedTime = (Dtick1 - Dtick0) * SYS_TIMER_TPC_MAX;
    }
    else    // overflow
    {
        elapsedTime = (0xFFFFFFFF - Dtick0 + 1 + Dtick1) * SYS_TIMER_TPC_MAX;
    }

    elapsedTime = elapsedTime + Dtpc1 - Dtpc0;
    elapsedTime *= regTimerPtr->TcB;                // TmV
    elapsedTime = elapsedTime + Dtmv1 - Dtmv0;

#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    elapsedTime = elapsedTime * 10 / (MAIN_CRYSTAL_CLOCK / 100000);
#elif (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    elapsedTime /= cpuFreq;
#else
    elapsedTime /= pllFreq;
#endif

#ifdef __ENABLE_MEASURE_GET_ELAPSED_TIME_PERFORMANCE
    tmv3 = regTimerPtr->TmV;
    tpc3 = regTimerPtr->Tpc;

    MP_ALERT("%d %d %d", tpc1, tpc2, tpc3);
    MP_ALERT("%d %d %d", tmv1, tmv2, tmv3);
#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    a = (tmv2 - tmv1) * 10000 / (MAIN_CRYSTAL_CLOCK / 100000);
#elif (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    a = (tmv2 - tmv1) * 1000 / cpuFreq;
#else
    a = (tmv2 - tmv1) * 1000 / pllFreq;
#endif

    b = (a % 1000);
    a /= 1000;
    MP_ALERT("1. %d.%dus", a, b);
#if (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    a = (tmv3 - tmv1) * 10000 / (MAIN_CRYSTAL_CLOCK / 100000);
#elif (TIMER0_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    a = (tmv3 - tmv1) * 1000 / cpuFreq;
#else
    a = (tmv3 - tmv1) * 1000 / pllFreq;
#endif
#endif

    return (DWORD) elapsedTime;
}



void SystemTimeCompensatingVerify(void)
{
    MP_DEBUG("%s", __func__);
#if (RTC_AUTO_CORRECT_EN == 1)
    DWORD counter, reservedCounter;
    DWORD newCounter;
    ST_SYSTEM_TIME rtcTime, resvertRtcTime;
    SDWORD diff;
    BYTE bShiftBits = 0;
    DWORD dwRegExt0 = RTC_ReadExt0();
    int i;

    dwRegExt0 &= 0xFFFFFFF0; // mask last 4 bit    
    if(dwRegExt0 == 0x55AA33C0) // Ext0 fixed data's last 4 bits unstable, change to 0x55AA33C0
        MP_DEBUG("dwRegExt0 = 0x%08X - no shift left!", dwRegExt0);
    else
    {
        MP_DEBUG("dwRegExt0(0x%08X) != 0x55AA33C0 - check shift bits!", dwRegExt0);
        if(dwRegExt0 == 0xAB546780) 
            bShiftBits = 1;
        else if(dwRegExt0 == 0x56A8CF00)
                bShiftBits = 2;
        else if(dwRegExt0 == 0xAD519E00)
                bShiftBits = 3;
        else
        {
            dwRegExt0 &= 0xFFFFFF00; // more than shift 3 bits, mask last 8 bit
            MP_DEBUG("dwRegExt0(0x%08X) != 0x55AA33C0 - check shift 4-7 bits!", dwRegExt0);
            if(dwRegExt0 == 0x5AA33C00) 
                bShiftBits = 4;
            else if(dwRegExt0 == 0xB5467800)
                    bShiftBits = 5;
            else if(dwRegExt0 == 0x6A8CF000)
                    bShiftBits = 6;
            else if(dwRegExt0 == 0xD519E000)
                    bShiftBits = 7;
            else
            {
                MP_ALERT("###################################################################################");
                MP_ALERT("Alert! dwRegExt0 maybe shift left more than 7 bits, need to check! skip shift check!");
                MP_ALERT("###################################################################################");
                bShiftBits = 0; // support shift left maximum 7 bits
            }
        }
    }
    if(bShiftBits > 0)
        MP_ALERT("%s: RegExt0 bShiftBits = %d", __func__, bShiftBits);

    counter = RTC_ReadCount();
    //counter = 0x59C5A67F; //0xFFFFFFF1; // test data
    MP_DEBUG("%s: counter         = 0x%08X", __func__, counter);
    reservedCounter = RTC_ReadReservedCount();
    //reservedCounter = 0x2CE2D337; //0xFEDCBA98; // test data
    MP_DEBUG("%s: reservedCounter = 0x%08X", __func__, reservedCounter);
    SystemTimeSecToDateConv(counter, &rtcTime);
    MP_DEBUG("%s: rtcTime         = %d.%d.%d, %d:%d:%d", __func__, rtcTime.u16Year, rtcTime.u08Month, rtcTime.u08Day, rtcTime.u08Hour, rtcTime.u08Minute, rtcTime.u08Second);
    SystemTimeSecToDateConv(reservedCounter, &resvertRtcTime);
    MP_DEBUG("%s: resvertRtcTime  = %d.%d.%d, %d:%d:%d", __func__, resvertRtcTime.u16Year, resvertRtcTime.u08Month, resvertRtcTime.u08Day, resvertRtcTime.u08Hour, resvertRtcTime.u08Minute, resvertRtcTime.u08Second);
    diff = rtcTime.u16Year - resvertRtcTime.u16Year;
    MP_DEBUG("%s: diff = %d", __func__, diff);

    //if(bShiftBits > 0) // it cover diff == 0 and if ((diff < 0) || (diff > 1))
    if( (bShiftBits > 0) || (diff < 0) || (diff > 1)) // (bShiftBits > 0) cover diff == 0 
    {   // Shift case
        for(i = 0; i < bShiftBits; i++)
        {
            reservedCounter = reservedCounter >> 1;
            MP_DEBUG("shift %d bits, reservedCounter = 0x%08X", __func__, i+1, reservedCounter);
        }
        MP_DEBUG("%s: after shift %d bits, reservedCounter = 0x%08X", __func__, bShiftBits, reservedCounter);
        
        SystemTimeSecToDateConv(reservedCounter, &resvertRtcTime);
        MP_DEBUG("%s: resvertRtcTime  = %d.%d.%d, %d:%d:%d", __func__, resvertRtcTime.u16Year, resvertRtcTime.u08Month, resvertRtcTime.u08Day, resvertRtcTime.u08Hour, resvertRtcTime.u08Minute, resvertRtcTime.u08Second);
        
        for(i = 0; i < 8; i++) // support upmost to 8 bits shift
        {
            newCounter = counter >> 1;
            counter = newCounter;
            MP_DEBUG("%s: newCounter = counter >> 1 = 0x%08X", __func__, newCounter);
            if(newCounter > reservedCounter && newCounter < (reservedCounter + 2592000) ) // 2592000 = 60x60x24=86400, 86400x30=2592000- depends on battery can support how many(30?) days, if over, then reset to 2000/01/01
            {
                //MP_ALERT("RTC Counter shift %d bits!", i+1);
                break; // got it - break;
            }
        }
        if(i < 8)
            MP_ALERT("%s: Alert! check RTC Counter shift %d bits!", __func__, i+1);

/*
        if (reservedCounter & BIT31)                // exceed to 2038/1/19 3:14:8, secondary count is 0x80000000
            newCounter |= BIT31;
        else if (newCounter < reservedCounter)
        {
            resvertRtcTime.u16Year++;
            newCounter |= SystemTimeDateToSecConv(&resvertRtcTime) & BIT31;
            resvertRtcTime.u16Year--;
        }
*/
        MP_DEBUG("%s: newCounter = 0x%08X(%d)", __func__, newCounter, newCounter);
        
        SystemTimeSecToDateConv(newCounter, &rtcTime);
        diff = rtcTime.u16Year - resvertRtcTime.u16Year;
        MP_DEBUG("%s: New diff = %d", __func__, diff);
        
        if ((diff < 0) || (diff > 1))
        {   // Reset to default time, first power-on
            rtcTime.u16Year = RTC_RESET_YEAR;   // 2000
            rtcTime.u08Month = RTC_RESET_MONTH; // 01
            rtcTime.u08Day = RTC_RESET_DATE;    // 01
            rtcTime.u08Hour = 0;
            rtcTime.u08Minute = 0;
            rtcTime.u08Second = 0;
            newCounter = SystemTimeDateToSecConv(&rtcTime);
            MP_ALERT("First power-on, reset RTC counter for %d/%d/%d 00:00:00 !!!", RTC_RESET_YEAR, RTC_RESET_MONTH, RTC_RESET_DATE);
        }
        else
        {
            //diff = ((newCounter - reservedCounter) << 1) + RTC_AUTO_CORRECT_VALUE;      // Power-down shift case
            diff = (newCounter - reservedCounter) + RTC_AUTO_CORRECT_VALUE;             // Power-on shift case
            newCounter += diff;
            MP_DEBUG("%s: else - newCounter = 0x%08X(%d)", __func__, newCounter, newCounter);
        }

        RTC_SetCount(newCounter);

        MP_DEBUG("Reserved counter is 0x%08X, %d/%d/%d %d:%d:%d", reservedCounter,
                                                                  resvertRtcTime.u16Year, resvertRtcTime.u08Month, resvertRtcTime.u08Day,
                                                                  resvertRtcTime.u08Hour, resvertRtcTime.u08Minute, resvertRtcTime.u08Second);
        MP_DEBUG("Compensate vaiue is %ds", diff);
        SystemTimeSecToDateConv(newCounter, &resvertRtcTime);
        MP_ALERT("New counter is 0x%08X, %d/%d/%d %d:%d:%d\r\n", newCounter,
                                                                 resvertRtcTime.u16Year, resvertRtcTime.u08Month, resvertRtcTime.u08Day,
                                                                 resvertRtcTime.u08Hour, resvertRtcTime.u08Minute, resvertRtcTime.u08Second);
    }
    else
        RTC_SetCount(counter); // update ReservedCount to Count
        
    MP_ALERT("RTC Current counter is 0x%08X, %d/%d/%d %d:%d:%d", counter,
                                                                 rtcTime.u16Year, rtcTime.u08Month, rtcTime.u08Day,
                                                                 rtcTime.u08Hour, rtcTime.u08Minute, rtcTime.u08Second);
                                                                 
    RTC_SetExt0(0x55AA33C0); // for next boot's shift check 
#endif
}

////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(SystemTimerInit);
MPX_KMODAPI_SET(SystemTimeAlarmSet);
MPX_KMODAPI_SET(SystemTimeAlarmGet);
MPX_KMODAPI_SET(GetSysTime);
MPX_KMODAPI_SET(SystemTimeSet);
MPX_KMODAPI_SET(SystemTimeGet);
MPX_KMODAPI_SET(SystemGetElapsedTime);
MPX_KMODAPI_SET(SysTimerProcAdd);
MPX_KMODAPI_SET(SysTimerProcRemove);
MPX_KMODAPI_SET(SysTimerProcPause);
MPX_KMODAPI_SET(SysTimerProcResume);
MPX_KMODAPI_SET(SysTimerStatusGet);
MPX_KMODAPI_SET(get_cur_timeL);
MPX_KMODAPI_SET(get_elapsed_timeL);
#endif

