
///
///@defgroup    TIMER   Timer
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


#include <linux/types.h>
#include "typedef.h"
#ifndef MP600
#include "regfile.h"
#include "bios.h"
#include "osinner.h"
#include "osapi.h"
#include "hal.h"
#else
#include "os.h"
#endif
#include "ui_timer.h"


#ifdef OS_TIME_DEBUG
    #define OS_TIME_DEBUG_ENABLE
#else
    //No Open Time Log
#endif

#ifdef MP600
SWORD AddTimerProc(register DWORD dwOffsetValue, register void (*Action) (void));
#endif

////////////////////////////////////////////////////////////////////////
///         Expiration Function Definition
////////////////////////////////////////////////////////////////////////

#ifndef MP600

ST_EXPIRER	ExpirerBin[OS_MAX_NUMBER_OF_EXPIRER];



void ExpirerInit()
{
    register ST_EXPIRER *expirer;
    register U32 index;

    // initialize the event and message expirer array
    index = 0;
    expirer = &ExpirerBin[0];
    while(index < OS_MAX_NUMBER_OF_EXPIRER)
    {
        expirer->u16Cycle = 0;
        expirer->u32Ticker = 0;
        expirer->u32Value = 0;
        expirer ++;
        index ++;
    }
}



void mpx_ExpirerSet(U08 expirer_id, U08 target_id, U16 cycle, U32 tick, U32 value)
{
    register ST_EXPIRER *expirer;

    IntDisable();

    expirer = &ExpirerBin[expirer_id];
    expirer->u08TargetId = target_id;
    expirer->u08TargetType = OsObjectType(target_id);
    expirer->u16Cycle = cycle;
    expirer->u32Ticker = tick;
    expirer->u32Initiator = tick;
    expirer->u32Value = value;

    IntEnable();
}



void ExpirationHandler(U32 u32TickCount)
{
    register ST_EXPIRER *expirer;
    register U32 index;

    index = 0;
    expirer = &ExpirerBin[0];
    while(index < OS_MAX_NUMBER_OF_EXPIRER)
    {
        if(expirer->u32Ticker)
        {
            if(expirer->u32Ticker > u32TickCount)	
                expirer->u32Ticker -= u32TickCount;
            else
            {
                expirer->u32Ticker = 0;

                if(expirer->u16Cycle)
                {
                    expirer->u16Cycle --;

                    if(expirer->u08TargetType == OS_TARGET_TYPE_MESSAGE)
                        mpx_MessageDrop(expirer->u08TargetId, (U08 *)&expirer->u32Value, 4);
                    else
                        mpx_EventSet(expirer->u08TargetId, expirer->u32Value);
                }
            }
        }

        expirer ++;
        index ++;
    }	
}



////////////////////////////////////////////////////////////////////////
///         System Clock Function Definition
////////////////////////////////////////////////////////////////////////

U08 DayPerMonth[2][13] = {{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                            {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

#define DAY_2_SECOND (24*60*60)
/* Start from 2000/1/1 */
#define BEGIN_DATE (1461*1999/4+153*14/5+1)
#define BEGIN_YEAR 2000
#define BEGIN_MONTH 1
#define BEGIN_DAY 1

// return 0 mean not leap year
// return 1 mean leap year
U32 LeapYearCheck(register U16 year)
{
    if (!(year%4) && (year%100) || !(year%400))
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


static U32 DateToSecConv(ST_SYSTEM_TIME *time)
{
    U32 f, g;

    if(time->u08Month<=2)
       f = time->u16Year-1;
    else
       f = time->u16Year;

    if(time->u08Month<=2)
       g = time->u08Month+13;
    else
       g = time->u08Month+1;

    return ((1461*f/4 + 153*g/5 + time->u08Day - BEGIN_DATE)*DAY_2_SECOND+time->u08Hour*60*60+time->u08Minute*60+time->u08Second);
}

static S32 SecToDateConv(U32 second, ST_SYSTEM_TIME *time)
{
    U32 day;
    
    day = (second/DAY_2_SECOND);
        
    time->u08Hour = (second%DAY_2_SECOND)/(60*60);
    second = (second%DAY_2_SECOND)%(60*60);
    time->u08Minute = second/60;
    time->u08Second = second%60;

    time->u16Year = BEGIN_YEAR;
    time->u08Month = BEGIN_MONTH;
    time->u08Day = BEGIN_DAY;

    DPrintf("day = 0x%x", day);

    while (day--)
    {
        time->u08Day++;

        if(time->u08Day> DayPerMonth[LeapYearCheck(time->u16Year)][time->u08Month])
        {
            time->u08Day= 1;
            time->u08Month ++;

            if(time->u08Month > 12)
            {
                time->u08Month = 1;
                time->u16Year ++;
            }
        }
    }

    return 1;
}


///
///@ingroup COSMOS
///@brief   Check if the specified date-time object is valid.
///
///@param   time    Point of the date-time object that need to be checked
///
///@retval      The input date-time is not valid.
///@retval      The input date-time is valid.
///
S32 mpx_SystemTimeCheck(ST_SYSTEM_TIME *time)
{
    register U32 index;

    if(time->u08Second >= 60)   return 0;
    if(time->u08Minute >= 60)   return 0;
    if(time->u08Hour >= 24)	return 0;

    index = LeapYearCheck(time->u16Year);
    if(time->u08Month > 12 || time->u08Month == 0)  return 0;
    if(time->u08Day > DayPerMonth[index][time->u08Month] || time->u08Day == 0)    return 0;

    return 1;
}



///
///@ingroup COSMOS
///@brief   Set the system time by a new date-time object.
///
///@param   new_time    Point of the new date-time object.
///
void mpx_SystemTimeSet(ST_SYSTEM_TIME *new_time)
{
    RtcSetCount(DateToSecConv(new_time));
    DPrintf("SysTime = 0x%x", DateToSecConv(new_time));
    DPrintf("Date: %4d-%2d-%2d", new_time->u16Year, new_time->u08Month, new_time->u08Day);
    DPrintf("Time: %2d:%2d:%2d", new_time->u08Hour, new_time->u08Minute, new_time->u08Second);
}



///
///@ingroup COSMOS
///@brief   Get the current system time.
///
///@param   curr_time    Point of the date-time object that will contain the returnned system date-time.
///
void mpx_SystemTimeGet(ST_SYSTEM_TIME *curr_time)
{
    SecToDateConv(RtcReadCount(), curr_time);
    DPrintf("SysTime = 0x%x", RtcReadCount());
    DPrintf("Date: %4d-%2d-%2d", curr_time->u16Year, curr_time->u08Month, curr_time->u08Day);
    DPrintf("Time: %2d:%2d:%2d", curr_time->u08Hour, curr_time->u08Minute, curr_time->u08Second);
}
#endif



////////////////////////////////////////////////////////////////////////
///         Timer Function Definition
////////////////////////////////////////////////////////////////////////

//The Tick time , (ms)
#define SYSTEM_TIME_RESOLUTON 4
#define UART_SIGNAL_RESOLUTON 1000
// time unit below : (us)
// to adjust JumpTimer
#define NULL_TIMER  (ST_TIMER *)0
#define AVSYNC_TIMEDIFF 2200
#define SYSTEMTICKSLICE SYSTEM_TIME_RESOLUTON*1000
#define ABS(A)          ((A) < 0 ? (-A) : (A))

U32 u32SystemTick;
U16 u16SecondDownCounter;
S32 s32TimeDiffNow = 0;

#ifdef OS_TIME_DEBUG_ENABLE

#if defined(CHIP_TYPE_MP321D)
    #define TLQueueSize 512
#elif defined(CHIP_TYPE_MP322)
    #define TLQueueSize 1024
#endif      

static volatile U32 u32TLDeQueueIdx, u32TLInQueueIdx;
static U32 u32TLName[TLQueueSize];
static U32 u32TLTick[TLQueueSize];
static U32 u32TLNano[TLQueueSize];
static U32 u32TimeLogTick;
static U32 u32TimeLogNano;
#endif

ST_TIMER    TimerBank[OS_MAX_NUMBER_OF_TIMER];
ST_TIMER    FineHead, NormHead, RoughHead;
U08     IdCounter;



S32 TimerGetAvailableId()
{
    register S32 i;

    IntDisable();
    i = 1;
    while(TimerBank[i].bState != OS_TIMER_EMPTY && i < OS_MAX_NUMBER_OF_TIMER) i ++;
    if(i >= OS_MAX_NUMBER_OF_TIMER)     i = OS_STATUS_OUT_OF_TIMER;
    else                                TimerBank[i].bState = OS_TIMER_STANDBY;
    IntEnable();

    return i;
}



#ifndef MP600
void OsTimerEnable()
{
/*
    REG_INDEX_POINT(timer, TIMER, 0);

    #ifdef IMAGIC_CHIP_TYPE
        #if IMAGIC_CHIP_TYPE == 0 
            timer->TM_C = 0x00200003;
        #elif IMAGIC_CHIP_TYPE == 1
            timer->TM_C = 0x00200007;
        #endif
    #else
        timer->TM_C = 0x00200003;
    #endif
*/
    HwTimerEnable(TIMER_OS);
    
//    mpx_IntEnable(10);            // INT 10, the timer 0 interrupt
}



void OsTimerDisable()
{
/*    
    REG_INDEX_POINT(timer, TIMER, 0);
    timer->TM_C = 0;
    
    mpx_IntDisable(10);           // INT 10, the timer 0 interrupt
*/
    HwTimerDisable(TIMER_OS, DISABLE_WITHKEEP);
}
#endif



// insert the new timer at the head of a list
/*
     _____        _____
head|     |      |     |
    |_____|--\/->|_____|
       ^     /\      ^
       |     _____   |
       |    |     |  |
       |--->|_____|<-|
             timer
*/            

void TimerInqueue(ST_TIMER *timer)
{
    register ST_TIMER *next, *head;

    head = timer->pHead;
    next = (ST_TIMER *)head->pNext;
    head->pNext = timer;
    timer->pPrevious = head;
    timer->pNext = next;

    if((U32)next != 0)    next->pPrevious = timer;

}


// delete timer in list
/*                   ____________
     _____       |      _____      |       _____
    |         |<-|     |         |    |-->|        |
    |_____|<-\/->|_____|<-\/-> |_____|
             /\   timer   /\ 

*/
void TimerDequeue(ST_TIMER *timer)
{
    register ST_TIMER *previous, *next;

    previous = (ST_TIMER *)timer->pPrevious;
    next = (ST_TIMER *)timer->pNext;
    previous->pNext = next;

    if((U32)next != 0)    next->pPrevious = previous;
}



#ifndef MP600
void TimerProcess(ST_TIMER *timer)
{
    while(timer->pNext != NULL_TIMER)
    {
        timer = timer->pNext;
        if(timer->u32Accumulator > timer->u32Denominator)
        {
            timer->u32Accumulator -= timer->u32Denominator;
            timer->pClient(timer->pClientArg);
            timer->u32Residue --;
            if(!timer->u32Residue)
            {
                TimerDequeue(timer);
                timer->u08State = OS_TIMER_READY;
            }
        }

        timer->u32Accumulator += timer->u32Numerator;
    }
}
#endif



#ifndef MP600
//extern ST_TASK *pRunningSpot;
void OsTimerInt()
{
/*
    REG_INDEX_POINT(ticker, TIMER, 0);

    #ifdef IMAGIC_CHIP_TYPE
        #if IMAGIC_CHIP_TYPE == 0 
            ticker->TM_C = 0x00200003;              // clear the interrupt cause
        #elif IMAGIC_CHIP_TYPE == 1
            ticker->TM_C = 0x00200007;              // clear the interrupt cause - 522
         #endif
    #else
        ticker->TM_C = 0x00200003;                  // clear the interrupt cause
    #endif 
*/

    HwTimerClear(TIMER_OS);

    u32SystemTick ++;

   	u16SecondDownCounter --;
   	if(!u16SecondDownCounter)
   	{
        //u16SecondDownCounter = 500;
        u16SecondDownCounter = UART_SIGNAL_RESOLUTON/SYSTEM_TIME_RESOLUTON;
        RtcCountInc();
        DPrintf(".\\-");
   	}

    // 4 ms service, for fine resolution handler
    if( u32SystemTick )
    {
        TimerProcess(&FineHead);
    }

    // 16 ms service, for normal resolution handler
    if((u32SystemTick & OS_TIMER_NORM_MASK) == 1)
    {
        TimerProcess(&NormHead);
    }

    // 64 ms service, for rough resolution handler
    if((u32SystemTick & OS_TIMER_ROUGH_MASK) == 2)
    {
        TimerProcess(&RoughHead);
        ExpirationHandler(64);
        TaskExpirationHandler(64);        
    }
}
#endif



void OsTimerInit()
{
    register U32 index;
    register ST_TIMER *timer;

    // software data structure initialization
    timer = &TimerBank[0];
    for(index = 0; index < OS_MAX_NUMBER_OF_TIMER; index ++, timer ++)
    {
#ifndef MP600
        timer->u08Id = index;
#endif
        timer->bState = OS_TIMER_EMPTY;
        timer->pPrevious = NULL_TIMER;
        timer->pNext = NULL_TIMER;
    }

#ifndef MP600
    FineHead.pPrevious = NULL_TIMER;
    FineHead.pNext = NULL_TIMER;
    NormHead.pPrevious = NULL_TIMER;
    NormHead.pNext = NULL_TIMER;
    RoughHead.pPrevious = NULL_TIMER;
    RoughHead.pNext = NULL_TIMER;

    u32SystemTick = 0;
    u16SecondDownCounter = 250;
    RtcCountSoftReset();

    // hardware register setting for timer
/*
    REG_POINT(clock, CKG);
    
    #ifdef IMAGIC_CHIP_TYPE
        #if IMAGIC_CHIP_TYPE == 0 
            clock->SEL_TIMER1_CLK = 1;      // select PLL2
        #elif IMAGIC_CHIP_TYPE == 1
            clock->SEL_TIMER1_CLK = 2;      // select PLL2  - 522
        #endif
    #else
        clock->SEL_TIMER1_CLK = 1;      // select PLL2
    #endif
    
    clock->CFG_TIMER1_CLK = 0x2ff;	// set the devider to 96
    clock->CFG_TIMER1_CLK &= 0xfffffffd;
    clock->CFG_TIMER1_CLK = 0x2ff;
*/

    HwTimerInit(TIMER_OS, TIMER_PLL2_SELECT, SYSTEMTICKSLICE);
    DPrintf("Set O/S Timer.");

/*    
    REG_INDEX_POINT(ticker, TIMER, 0);
    
    ticker->TC_A = 10;
    ticker->TC_B = 2000;                // first counter to 20
    //ticker->TPC = 100 << 16;      // second counter to 100
    ticker->TM_C &= 0xfffffffe;
*/
    
    // register the timer 0 interrupt handler
    mpx_IntRegister(OS_INT_NUMBER_TIMER0, OsTimerInt);
#endif
}



#ifndef MP600
///
///@ingroup TIMER
///@brief   Get System Ticker
///
///@retval  Tick count in 4 ms from system reset.
///
///@remark  The system tick count will increment every 4 ms and will reset to 0 every 49.71 days.
///
U32 mpx_SystemTickerGet()
{
    return u32SystemTick;
}
#endif



///
///@ingroup TIMER
///@brief   Creat Timer
///
///@param   handler     Timer handler function pint
///@param   resolution  Time resolution that this timer handler will be checked
///@param   numerator   The numerator of the period of timer
///@param   denominator The denominator of the period of timer
///
///@retval  0-255                   A timer is create successfully and represent the created timer ID.
///@retval  OS_STATUS_INVALID_ID    There is no ID is available.
///@retval  OS_STATUS_WRONG_PARAM   The wrong resolution or the denominator equal to zero.
///
///@remark  This service call create a timer handler.
///         The caller-defined handler is pointed by 'handler' and will be called every
///         (numerator/denominator) seconds. The parameter 'resolution' will define how
///         accurate the handler will be performed. If the resolution is 4 ms, then the
///         margin of error will not exceed 4 ms. There are 3 kinds of resolution has
///         implemented in the timer module. It's 4 ms, 16 ms and 64 ms.
///@remark  The timer handler should can have argument with type of (void *). The caller
///         can use this point to specify a argument buffer to save all the arguments
///         that the timer handler need. The mpx_TimerArgSet function is used to set this 
///         point argument. In just created timer the handler argument field will contain
///         a null point and will not transfer any argument to the handler.
///
S32 mpx_TimerCreate(void *handler, U08 resolution, U16 numerator, U16 denominator)
{
    register ST_TIMER *timer;
    register S32 status, id;

    // argument list check
    if(denominator == 0)
    	return OS_STATUS_WRONG_PARAM;
    else if(resolution != OS_TIMER_FINE && resolution != OS_TIMER_NORMAL && resolution != OS_TIMER_ROUGH)
        return OS_STATUS_WRONG_PARAM;
        
    IntDisable();                   // disable INT and save the old status

    id = TimerGetAvailableId();
    if(id >= 0)
    {
    	timer = &TimerBank[id];
    	if(timer->bState != OS_TIMER_EMPTY && timer->bState != OS_TIMER_STANDBY)     
            status = OS_STATUS_WRONG_STATE;
        else
        {
#ifndef MP600
            timer->u08State = OS_TIMER_READY;
            timer->u08OriginD = denominator;
            timer->u08Resolution = resolution;
            timer->pClient = handler;
            timer->pClientArg = OS_NULL;
            timer->u32Numerator = denominator * resolution;
            timer->u32Denominator = numerator * OS_TIMER_BASE_FREQ;
            timer->u32Accumulator = 0;
#else
            timer->bState = OS_TIMER_READY;
            timer->pClient = handler;
            timer->dwNumerator = numerator;
            timer->dwDenominator = denominator;
            timer->dwAccumulator = 0;
#endif

#ifndef MP600
            switch(resolution)
            {
                case OS_TIMER_FINE:     timer->pHead = &FineHead;   break;
                case OS_TIMER_NORMAL:   timer->pHead = &NormHead;   break;
                case OS_TIMER_ROUGH:    timer->pHead = &RoughHead;
            }
#endif

            status = id;
        }
    }
    else
        status = OS_STATUS_INVALID_ID;

    IntEnable();
    
    return status;
}



#ifndef MP600
///
///@ingroup TIMER
///@brief   Set timer handler argument
///
///@param   id  Timer ID
///@param   arg The argument of the timer handler
///
///@retval  OS_STATUS_OK            The argument has set the handler successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///
///@remark  This function set the a point of a buffer that contain all the information
///         for the timer handler at the calling time. Refer to the mpx_TimerCreate for more
///         information. 
///
S32 mpx_TimerArgSet(U08 id, void *arg)
{
    register ST_TIMER *timer;
    register S16 return_code;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)     
    	return_code = OS_STATUS_INVALID_ID;
    else
    {
        timer = &TimerBank[id];
        timer->pClientArg = arg;
        return_code = OS_STATUS_OK;
    }

    IntEnable();
    
    return return_code;
}
#endif


#include "net_sys.h"
#include "taskid.h"
#include "timer.h"
extern U08 u08NetTimerId;
extern timer net_timer;

///
///@ingroup TIMER
///@brief   Start timer.
///
///@param           Timer ID
///@param   times   Specify the execution times of this timer.
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in READY state.
///
///@remar   This service call start a timer with an ID number specified 'id'.
///         This timer must be created by mpx_TimerCreate and has a READY timer state
///         at the calling time. After calling, the associated handler of the
///         specified timer will execute 'times' time and the duration of every two
///         successive executions will equal to length that set in creation time.
///         After calling, the timer state will be moved to the RUNNGIN state.
///@remark  StartTimer also can start a paused timer from PAUSED state to RUNNING.
///         The remainning execution time of this timer will be reset to the number
///         specified by 'times'.
///
S32 mpx_TimerStart(U08 id, U32 times)
{
    register ST_TIMER *timer;
//        register S16 return_code;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->bState != OS_TIMER_READY && timer->bState != OS_TIMER_PAUSED)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

#ifndef MP600
    timer->u08State = OS_TIMER_RUNNING;
    timer->u32Residue = times;
    timer->u32Accumulator = 0;
    TimerInqueue(timer);
#else
    timer->bState = OS_TIMER_RUNNING;
    timer->dwResidue = times;
    timer->dwAccumulator = 0;
    if (u08NetTimerId == id)
    {

        //timer_set(&net_timer, (timer->dwNumerator * 1000)/timer->dwDenominator*HZ);
//        timer_set(&net_timer, 12);  /* 12.5 = 250 ms */
    }
    //else
    //    AddTimerProc((timer->dwNumerator * 1000)/timer->dwDenominator*HZ, timer->pClient);
#endif
    IntEnable();

    return OS_STATUS_OK;
}



#ifndef MP600
///
///@ingroup TIMER
///@brief   Trigger timer once with specific period.
///
///@param   id          Timer ID
///@param   numerator   The numerator of the period
///@param   denominator The denominator of the period
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///
///@remark  mpx_TimerOneShot provide a one-time timer starting with changing the time-out
///         time setting within one function call. No matter the state this timer is in, 
///         the period of this timer will be reset and the timer counter will start to
///         count.
///
S32 mpx_TimerOneShot(U08 id, U16 numerator, U16 denominator)
{
    register ST_TIMER *timer;
    register S16 status;

    IntDisable();               // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
        status = OS_STATUS_INVALID_ID;
    else
    {
        timer = &TimerBank[id];
        timer->u32Residue = 1;
        timer->u32Numerator = denominator * timer->u08Resolution;
        timer->u32Denominator = numerator * OS_TIMER_BASE_FREQ;
        timer->u32Accumulator = 0;
        if(timer->u08State != OS_TIMER_RUNNING)
        {
            TimerInqueue(timer);
            timer->u08State = OS_TIMER_RUNNING;
        }

        status = OS_STATUS_OK;
    }

    IntEnable();

    return status;
}
#endif



///
///@ingroup TIMER
///@brief   Stop timer.
///
///@param   id      Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in RUNNING or PAUSED state.
///
///@remark  This service call stop a started timer, that is, move a started timer from
///         RUNNGIN state to READY state. The only way to re-active this timer is use
///         the service call mpx_TimerStart. It's the the execution time will be reset.
///
S32 mpx_TimerStop(U08 id)
{
    register ST_TIMER *timer;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->bState != OS_TIMER_RUNNING && timer->bState != OS_TIMER_PAUSED)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

    timer->bState = OS_TIMER_READY;
    TimerDequeue(timer);
    IntEnable();

    return OS_STATUS_OK;
}



#ifndef MP600
///
///@ingroup TIMER
///@brief   Pause timer.
///
///@param   id      Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in RUNNING state.
///
///@remark  This service call temporarily stop a started timer, that is, move a started timer from
///         RUNNGIN state to PAUSED state. The execution time will be maintained and will keep donwcount
///         if timer has been re-activated by the service call mpx_TimerResume.
///
S32 mpx_TimerPause(U08 id)
{
    register ST_TIMER *timer;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->u08State != OS_TIMER_RUNNING)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

    timer->u08State = OS_TIMER_PAUSED;
    TimerDequeue(timer);
    IntEnable();

    return OS_STATUS_OK;
}



///
///
///
void mpx_TimerAlign(U08 id, U32 new_time)
{
//  register U32 time;
    register ST_TIMER *timer;

    timer = &TimerBank[id]; 
    timer->u32Accumulator = ((new_time >> 2) * timer->u08OriginD) % timer->u32Denominator;
}



///
///@ingroup TIMER
///@brief   Resume timer.
///
///@param   id      Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in PAUSED state.
///
///@remark  This service call re-activate a paused timer from PAUSED state to RUNNING
///         state. The valid execution time is the number that keep in this timer when
///         it had been puased.
///
S32 mpx_TimerResume(U08 id)
{
    register ST_TIMER *timer;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->u08State != OS_TIMER_PAUSED)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

    timer->u08State = OS_TIMER_RUNNING;
    TimerInqueue(timer);
    IntEnable();

    return OS_STATUS_OK;
}
#endif



///
///@ingroup TIMER
///@brief   Destroy timer.
///
///@param   id      Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has been destroy successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in READY state.
///
///@remark  This service call destroy a timer from to let it can be re-created for the
///         new application.
///
S32 mpx_TimerDestroy(U08 id)
{
    register ST_TIMER *timer;
    S32 status;

    IntDisable();

    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
        status = OS_STATUS_INVALID_ID;
    else
    {
    	timer = &TimerBank[id];
        if(timer->bState == OS_TIMER_RUNNING)	TimerDequeue(timer);	
    	timer->bState = OS_TIMER_EMPTY;
    	status = OS_STATUS_OK;
    }
    
    IntEnable();

    return status;
}



#ifndef MP600
///
///@ingroup TIMER
///@brief   Creat Jump Timer
///
///@param   handler     Timer handler function pint
///@param   resolution  Time resolution that this timer handler will be checked
///
///@retval  0-255                   A timer is create successfully and represent the created timer ID.
///@retval  OS_STATUS_INVALID_ID    No timer is avaiable.
///@retval  OS_STATUS_WRONG_PARAM   The wrong resolution or the denominator equal to zero.
///
///@remark  This service call create a jump timer handler with an ID number specified by 'id'.
///         The caller-defined handler is pointed by 'handler' and will be called when
///         the jump point is reached, for the jump point should be set by the following
///         mpx_JumpTimerAdvance() function.
///@remark  The timer specified by the 'id' must not be used or in the EMPTY state before
///         calling. After calling, the timer state will be moved to the READY state.
///
S32 mpx_JumpTimerCreate(void *handler, U08 resolution)
{
    register ST_TIMER *timer;
    register S32 status, id;

    // argument list check
    if(resolution != OS_TIMER_FINE && resolution != OS_TIMER_NORMAL && resolution != OS_TIMER_ROUGH)
        return OS_STATUS_WRONG_PARAM;
        
    IntDisable();                   // disable INT and save the old status

    id = TimerGetAvailableId();
    if(id >= 0)
    {
    	timer = &TimerBank[id];
        timer->u08State = OS_TIMER_READY;
        timer->pClient = handler;
        timer->u32Numerator = resolution * 4000;
        timer->u32Accumulator = 0;

        switch(resolution)
        {
            case OS_TIMER_FINE:     timer->pHead = &FineHead;   break;
            case OS_TIMER_NORMAL:   timer->pHead = &NormHead;   break;
            case OS_TIMER_ROUGH:    timer->pHead = &RoughHead;
        }
        
        status = id;
    }
    else
        status = OS_STATUS_INVALID_ID;

    IntEnable();
    
    return status;
}



///
///@ingroup TIMER
///@brief   Set the next trigger point of the specified timer.
///
///@param   id      Timer ID
///@param   tick    Specify the timer trigger point from the last triggerred point in micro second.
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in READY or RUNNING state.
///
///@remark  This service call set the timer next trigger time from the last trigger
///         point. If this timer is in the RUNNING state, then
///         this function will enable this timer right after it set the next trigger point.
///
S32 mpx_JumpTimerAdvance(U08 id, U32 tick)
{
    register ST_TIMER *timer;
    REG_INDEX_POINT(ticker, TIMER, 0);
    U32 u32TimeDiffBehind =0 ,u32TimeDiffFront =0;
    U08 flag =0;

    IntDisable();               // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->u08State == OS_TIMER_EMPTY)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

        //The Start of Hand behind :P
        timer->u32Accumulator = ( (SYSTEMTICKSLICE/2) - (ticker->TM_V) )*2;// 2us is timer period as 4ms = 1 tick
        u32TimeDiffBehind = (tick -(timer->u32Accumulator))%SYSTEMTICKSLICE;// calculate time diff_before between actual point & SystemTimerIsr
        u32TimeDiffFront = SYSTEMTICKSLICE - u32TimeDiffBehind;
        if(u32TimeDiffBehind > u32TimeDiffFront)
        {
            if(ABS((S32)(s32TimeDiffNow+u32TimeDiffFront))<=AVSYNC_TIMEDIFF)
            {
                s32TimeDiffNow += u32TimeDiffFront;
                flag = 1;
            }
            else
            {
                s32TimeDiffNow -= u32TimeDiffBehind;
                flag = 0;
            }            
        }
        else
        {
            if(ABS((S32)(s32TimeDiffNow-(S32)u32TimeDiffBehind))<=AVSYNC_TIMEDIFF)
            {            
                s32TimeDiffNow -= u32TimeDiffBehind;
                flag = 1;
            }
            else
            {
                s32TimeDiffNow += u32TimeDiffFront;
                flag = 0;                    
            }        
        }

        if( u32TimeDiffFront>(SYSTEMTICKSLICE/2) && flag==1)
        {
            tick -= SYSTEMTICKSLICE;
        }
        if( u32TimeDiffFront<(SYSTEMTICKSLICE/2) && flag==0)
        {
            tick -= SYSTEMTICKSLICE;
        }         
//debuging using
/*
        DPrintf("B:%5d",u32TimeDiffBehind);
        DPrintf("F:%5d",u32TimeDiffFront);
      
        if(s32TimeDiffNow>=0)
            if(ABS(s32TimeDiffNow)>=2500)
                DPrintf("L:+%d",(ABS(s32TimeDiffNow)));
            else
                DPrintf("D:+%d",(ABS(s32TimeDiffNow)));
        else
            if(ABS(s32TimeDiffNow)>=2500)
                DPrintf("L:-%d",(ABS(s32TimeDiffNow)));
            else
                DPrintf("D:-%d",(ABS(s32TimeDiffNow)));
*/
    timer->u32Residue = 0xffffffff;	// set a never stop timer
    timer->u32Denominator = tick;

    if(timer->u08State != OS_TIMER_RUNNING)
    {
        timer->u08State = OS_TIMER_RUNNING;
        TimerInqueue(timer);
    }

    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup TIMER
///@brief   Stop jump timer.
///
///@param   id      Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has started successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in RUNNING or PAUSED state.
///
///@remark  This service call stop a started timer, that is, move a started timer from
///         RUNNGIN state to READY state. The only way to re-active this timer is use
///         the service call mpx_TimerStart. It's the the execution time will be reset.
///
S32 mpx_JumpTimerStop(U08 id)
{
    register ST_TIMER *timer;

    IntDisable();                   // disable INT and save the old status

    // argument list check
    if(id > OS_MAX_NUMBER_OF_TIMER - 1)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    timer = &TimerBank[id];
    if(timer->u08State != OS_TIMER_RUNNING && timer->u08State != OS_TIMER_PAUSED)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

    timer->u08State = OS_TIMER_READY;
    timer->u32Accumulator = 0;

    TimerDequeue(timer);
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup TIMER
///@brief   Destroy timer.
///
///@param   id  Timer ID
///
///@retval  OS_STATUS_STATUS_OK     The specified timer has been destroy successfully.
///@retval  OS_STATUS_INVALID_ID    The input ID is out of the valid ID range.
///@retval  OS_STATUS_WRONG_STATE   The specified timer is not in READY state.
///
///@remark  This service call destroy a timer from to let it can be re-created for the
///         new application.
///
S32 mpx_JumpTimerDestroy(U08 id)
{
    S32 retCode;
    
        s32TimeDiffNow = 0;
    retCode=mpx_TimerDestroy(id);

    return (S32)retCode;
}



#ifdef OS_TIME_DEBUG_ENABLE
BOOL x=TRUE;
void TimeLogInit()
{
    S32 i;
    
    u32TLDeQueueIdx = u32TLInQueueIdx = 0;
    u32TimeLogTick = u32TimeLogNano = 0;
    i = TLQueueSize -1;

    for ( ;i>=0;i-- )
    {
        u32TLName[i] = 0; 
        u32TLTick[i] = 0;
        u32TLNano[i]=0;
    }
}



void TimeLogShot(U32 Name, BOOL flag)
{
    volatile U32 inQIdx=(u32TLInQueueIdx+1)&(TLQueueSize-1);
    U08 resolution = HwTimerResolutionGet(TIMER_OS);

    if ( inQIdx == u32TLDeQueueIdx )
    {
        if(TRUE==x)
        {
            DPrintf("TL Queue Full!");
            x = FALSE;
        }
        return ;  
    }

    IntDisable();        
    u32TimeLogTick = u32SystemTick;
    u32TimeLogNano = HwTimerCurValueGet(TIMER_OS)*resolution;
    IntEnable();

    u32TLName[u32TLInQueueIdx] = Name;
    if(flag==TRUE)
    {
        u32TLTick[u32TLInQueueIdx] = u32TimeLogTick;
        u32TLNano[u32TLInQueueIdx] = u32TimeLogNano;
    }
    IntDisable();
    u32TLInQueueIdx = inQIdx;
    IntEnable();
}



void TimeLogRelease()
{    
    S32 end=TLQueueSize-1, i=0;
    U32 timediff = 0;

    for(;i<=end;i++)
    {
        if(i==0)
            DPrintf("%s:%x-%d:0", u32TLName[i],u32TLTick[i],u32TLNano[i]);
        else
        {
            timediff = ( (S32)((u32TLTick[i]-u32TLTick[i-1])*4000)+(S32)(u32TLNano[i]-u32TLNano[i-1]) );
            if(u32TLName[i]!=0)
                DPrintf("%4s:%x-%d:%d(us)", u32TLName[i],u32TLTick[i],u32TLNano[i],timediff);
        }
    }
    i = 0;//ready to destroy
    for(;i<=end;i++)
    {
        u32TLName[i]=u32TLTick[i]=u32TLNano[i]=0;
    }
}
#endif
#endif

void (*(NWTimerHandler(int id)))(void)
{
    return TimerBank[id].pClient;
}



