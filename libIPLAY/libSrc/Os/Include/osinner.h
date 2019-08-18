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
* Filename      : osinner.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
//
// Declaration for OS internal usage
//

#ifndef __OSINNER_H
#define __OSINNER_H

#include "osconst.h"
#include "osconfig.h"

//#define __MONITOR_CONTEXT_SWITCH
//#define __MONITOR_RESERVED_MEMORY_REGION

#define OS_OBJECT_FREE                  0x00
#define OS_OBJECT_SEMAPHORE             0x01
#define OS_OBJECT_EVENT                 0x02
#define OS_OBJECT_MAILBOX               0x04
#define OS_OBJECT_MESSAGE               0x08

#define OS_NULL                         (void *)0
#define CONTEXT_SWITCH()                __asm(" syscall ")


typedef struct
{
    BYTE    bState;                 // designated the current task state
    BYTE    bId;
    BYTE    bCurrPriority;          // running time priority
    BYTE    bBasePriority;          // base priority that specified at creation time

    void    *pNext;                 // link to next task
    void    *pList;                 // the task list belong to when in the wait state.

    DWORD   *pdwEntry;              // task body function entry point
    DWORD   (*pdwExit)();           // task terminate procedure entry point

    DWORD   dwStackSize;            // task stack size, spedified at creation time
    DWORD   *pdwStackBase;          // task stack base address
    DWORD   *pdwStackPoint;         // task current stack point

    DWORD   dwSleepTime;            // the time of ms to sleep
    void    *pSleepNext;            // link to the next task that has been put into sleep state

    DWORD   dwObjectBuffer;
    DWORD   dwObjectScratch;

    DWORD   dwParameter[4];

    DWORD   dwTimeSchedulePeriod;   // the time of ms to yield
    DWORD   dwTimeScheduleTimeRemain;
} ST_TASK;

// TASK_ZERO is used for the link terminator, it next will always point to itself.
// TASK ID 0 is occupied by the TASK_ZERO and can not be allocated.
extern ST_TASK TaskBank[OS_MAX_NUMBER_OF_TASK];

#define TASK_ZERO                       &TaskBank[0]


typedef struct
{
    ST_TASK *pHead;
    ST_TASK *pRear;
    DWORD   dwCounter;
} ST_TASK_LIST;



typedef struct
{
    BYTE            bObjectType;
    BYTE            bAttribute;
    BYTE            bCounter;
    BYTE            bMaximal;
    ST_TASK_LIST    Wait;
} ST_SEMAPHORE;



typedef struct
{
    BYTE            bObjectType;
    BYTE            bAttribute;
    BYTE            Reserve[2];
    DWORD           dwFlag;
    DWORD           dwInitial;
    ST_TASK_LIST    Wait;
} ST_EVENT;



typedef struct
{
    BYTE    bPriority;
    BYTE    bState;
    BYTE    bId;
    BYTE    Reserve;
    void    *pNext;
    DWORD   dwStart;
    DWORD   dwSize;
    ST_TASK *pSender;
} ST_MAILTAG;



typedef struct
{
    BYTE            bObjectType;    ///<    Indicate the type of this object, it should be OS_OBJECT_MAILBOX
    BYTE            bAttribute;     ///<    Attribute of a mailbox
    BYTE            Reserve[2];
    ST_MAILTAG      *pHead;         ///<    Point to the first mailtag of the send wait queue
    ST_TASK_LIST    Wait;           ///<    Point to task list that all the task in it are waiting for receiving mail
} ST_MAILBOX;



typedef struct
{
    BYTE    *pbBase;
    DWORD   dwSize;
    DWORD   dwCounter;
    DWORD   dwInIndex;
    DWORD   dwOutIndex;
} ST_RING_BUFFER;



typedef struct
{
    BYTE            bObjectType;
    BYTE            bAttribute;
    BYTE            Reserve[2];
    ST_RING_BUFFER  *pBuffer;
    ST_TASK_LIST    SendWait;
    ST_TASK_LIST    ReceiveWait;
} ST_MESSAGE;



typedef union
{
    ST_SEMAPHORE    Semaphore;
    ST_EVENT        EventFlag;
    ST_MAILBOX      MailBox;
    ST_MESSAGE      MessageBuffer;
} UN_OS_OBJECT;



#define OS_TIMER_EMPTY              0       // this timer is not in used
#define OS_TIMER_READY              1       // this timer is linked to handler
#define OS_TIMER_RUNNING            2
#define OS_TIMER_PAUSED             3

#define OS_TIMER_BASE_FREQ          1000
#define OS_TIMER_FINE_COEF          4
#define OS_TIMER_NORM_COEF          16
#define OS_TIMER_ROUGH_COEF         64
#define OS_TIMER_FINE_MASK          (OS_TIMER_FINE_COEF - 1)
#define OS_TIMER_NORM_MASK          (OS_TIMER_NORM_COEF - 1)
#define OS_TIMER_ROUGH_MASK         (OS_TIMER_ROUGH_COEF - 1)

DWORD   *OsMemoryAllocate(DWORD);

ST_TASK *TaskGetMe();
void    TaskAppend(ST_TASK_LIST *, ST_TASK *);
void    TaskInsert(ST_TASK_LIST *, ST_TASK *);
void    TaskPreempt(ST_TASK_LIST *, ST_TASK *);
ST_TASK *TaskDequeue(ST_TASK_LIST *);
ST_TASK *TaskWithdraw(ST_TASK *);
void    TaskSchedule(ST_TASK *);
ST_TASK *TaskDeschedule();
void    ContextUpdate();
void    TaskSetSleeping(ST_TASK *, DWORD);
void    TaskCancelSleeping(ST_TASK *);

void    TaskTimeSchedulingInit(void);
void    TaskTimeSchedulingTimeTick(DWORD timepassed);

void            *ObjectAllocate(BYTE, BYTE);
ST_SEMAPHORE    *SemaphoreGetPoint(BYTE);
ST_EVENT        *EventGetPoint(BYTE);
ST_MAILBOX      *MailboxGetPoint(BYTE);
ST_MESSAGE      *MessageGetPoint(BYTE);

#ifdef __MONITOR_RESERVED_MEMORY_REGION
BOOL mem_ReservedRegionInfoSet(DWORD len, DWORD *dataPtr);
BOOL mem_ReservedRegionCheck(void);
#endif

#endif      // __OSINNER_H

