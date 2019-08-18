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
* Filename      : osapi.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///

#ifndef __OSAPI_H
#define __OSAPI_H

#include "UtilTypeDef.h"
#include "interrupt.h"

///
///@ingroup     SYSTIMER
///@brief       define the structure to record the system time.
typedef struct
{
    WORD     u16Year;        ///< System Year
    BYTE     u08Month;       ///< System Month
    BYTE     u08Day;         ///< System Day
    BYTE     u08Hour;        ///< System Hour
    BYTE     u08Minute;      ///< System Minute
    BYTE     u08Second;      ///< System Second
    BYTE     u08Reserved;
    DWORD    u32TimeStamp;
} ST_SYSTEM_TIME;



typedef struct
{
    BOOL    inUsed;
    void    *msgPtr;
} ST_EXCEPTION_INFO;


typedef struct
{
    DWORD   dwTag;
    DWORD   msgAddr;
} ST_EXCEPTION_MSG;


///
///@ingroup     SYSTIMER
///@brief       define the system tick's resolution. Unit is 250us.
#define SYS_TIMER_TPC_PERIOD        250     // us per TPC

///
///@ingroup     SYSTIMER
///@brief       define the system tick's period. Unit is 4ms.
#define SYS_TIMER_TICK_PERIOD       4                               // ms per tick
#define SYS_TIMER_HEAP_PERIOD       (1000 / SYS_TIMER_TICK_PERIOD)  // Hz per interrupt
#define SYS_TIMER_TPC_MAX           (SYS_TIMER_TICK_PERIOD * 1000 / SYS_TIMER_TPC_PERIOD)

void    OsInit(DWORD *, DWORD);
//BYTE    *OsGetBankBStart();

//DWORD   OsGetMemorySize();
//void    ResetUserBlk();
//BYTE    *OsGetUserBlkStart();
//DWORD   OsGetUserBlkSize();
//DWORD   OsUserBlkAllocate(DWORD);
//void    OsUserBlkRelease(DWORD);
//BYTE    OsTempMemAllocate();
//DWORD   GetOsTempMemory(BYTE);
//void    OsTempMemRelease(BYTE );


BYTE    TaskGetPriority(void);
SDWORD  TaskCreate(BYTE, void *, BYTE, DWORD);
SDWORD  TaskStartup(BYTE, ...);
SDWORD  TaskStartupParamGet(BYTE, DWORD *, DWORD *, DWORD *, DWORD *);
SDWORD  TaskDelay(DWORD);
BYTE    TaskGetId(void);
BOOL    ContextStatusGet(void);
SDWORD  TaskSleep(DWORD);
void    TaskTerminate(BYTE);
SDWORD  TaskWakeup(BYTE);
SDWORD  TaskYield(void);
SDWORD  TaskTimeScheduling(BYTE taskId);
SDWORD  TaskTimeSchedulingRegister(BYTE taskId, DWORD scheduleTime);

SDWORD  EventClear(BYTE, DWORD);
SDWORD  EventCreate(BYTE, BYTE, DWORD);
SDWORD  EventPolling(BYTE, DWORD, BYTE, DWORD *);
SDWORD  EventSet(BYTE, DWORD);
SDWORD  EventWait(BYTE, DWORD, BYTE, DWORD *);

SDWORD  SemaphoreCreate(BYTE, BYTE, BYTE);
SDWORD  SemaphorePolling(BYTE);
SDWORD  SemaphoreWait(BYTE);
SDWORD  SemaphoreRelease(BYTE);

BYTE    MailPolling(BYTE);
SDWORD  MailTrack(BYTE);
SDWORD  MailRelease(BYTE);
SDWORD  MailGetBufferStart(BYTE, DWORD *);
SDWORD  MailGetBufferSize(BYTE, DWORD *);
SDWORD  MailboxCreate(BYTE, BYTE);
SDWORD  MailboxSend(BYTE, BYTE *, DWORD, BYTE *);
SDWORD  MailboxReceive(BYTE, BYTE *);
SDWORD  MailboxPolling(BYTE, BYTE *);

SDWORD  MessageCreate(BYTE, BYTE, DWORD);
SDWORD  MessageSend(BYTE, BYTE *, BYTE);
SDWORD  MessageDrop(BYTE, BYTE *, BYTE);
SDWORD  MessageReceive(BYTE, BYTE *);
SDWORD  MessageGrab(BYTE, BYTE *);

void    SystemTimerInit(void);
void    SystemTimerReInit(void);
DWORD   SystemGetTimeStamp(void);
DWORD   SystemGetElapsedTime(DWORD);
DWORD   GetSysTime(void);
void SysTimerProcDisable(void);
void SysTimerProcEnable(void);
SDWORD  SysTimerProcAdd(DWORD, void *, BOOL);
SDWORD  SysTimerProcRemove(void *);
SDWORD  SysTimerProcPause(void *);
SDWORD  SysTimerProcResume(void *);
SDWORD  SysTimerProcArgSet(void *, void *);
SDWORD  SysTimerProcReAdd(DWORD, void *, BOOL, BOOL);
SDWORD  SysTimerProcRemoveById(DWORD);
SDWORD  SysTimerProcPauseById(DWORD);
SDWORD  SysTimerProcResumeById(DWORD);
SDWORD  SysTimerProcArgSetById(DWORD, void *);

void    SystemTimeSet(ST_SYSTEM_TIME *);
void    SystemTimeGet(ST_SYSTEM_TIME *);
void    SystemTimeAlarmSet(ST_SYSTEM_TIME *);
void    SystemTimeAlarmGet(ST_SYSTEM_TIME *);
U32     SystemTimeDateToSecConv(ST_SYSTEM_TIME *);
void    SystemTimeSecToDateConv(DWORD, ST_SYSTEM_TIME *);
void    SystemTimeCompensatingVerify(void);

BOOL    SysTimerStatusGet(void);

void    get_cur_timeL(DWORD *, DWORD *, DWORD *);
DWORD   get_elapsed_timeL(DWORD, DWORD, DWORD);

SDWORD ExceptionTagCheck(BYTE);
SDWORD ExceptionTagReister(void *);
SDWORD ExceptionTagRelease(BYTE);

#endif  //__OSAPI_H

