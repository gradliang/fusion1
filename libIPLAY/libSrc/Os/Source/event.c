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
* Filename      : event.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@defgroup    EVENT   Eventflag
///
/// An eventflag is a synchronization object that consists of multiple bits in a bit pattern
/// where each bit represents an event. Eventflag functions include the ability to create an
/// eventflag, set and clear an eventflag, to wait for and poll an eventflag with the specified
/// pattern, and reference an eventflag. An eventflag is an object identified by an ID number.
/// The ID number is ranged form 1 to 31 in this implementation.
///
///@image html EventState.gif
///

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "utiltypedef.h"
#include "mpTrace.h"
#include "osinner.h"

///
///@ingroup EVENT
///@brief   Create a eventflag object
///
///@param   id      object ID
///@param   attr    attribute of this eventflag
///@param   flag    initial value of the eventflag
///
///@retval  OS_STATUS_OK            Eventflag create successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range
///
///@remark  This service call create an eventflag with the ID number specified by 'id' and it's
///         value is initialized by the 'flag'.
///@remark  The value 'attr' can be any OR combination of OS_ATTR_FIFO, OS_ATTR_PRIORITY,
///         OS_ATTR_WAIT_SINGLE, OS_ATTR_WAIT_MULTIPLE and OS_ATTR_EVENT_CLEAR. When OS_ATTR_FIFO
///         is specified, the eventflag's wait queue will be in FIFO order. When OS_ATTR_PRIORITY
///         is specified, the eventflag's wait queue will be in task priority order. When
///         OS_ATTR_WAIT_SINGLE is specified, only a single task can be put into wait queue.
///         When OS_ATTR_WAIT_MULTIPLE is specified, multiple tasks can be moved to the wait queue.
///         And, for the last attribute, when OS_ATTR_EVENT_CLEAR is specified, all the entire bit
///         pattern are cleared once a task's release condition is met in the wait queue.
///
SDWORD EventCreate(BYTE id, BYTE attr, DWORD flag)
{
    register ST_EVENT *event;
    register ST_TASK_LIST *link;
    SWORD status;

    IntDisable();               // disable INT and save the old status

    event = (ST_EVENT *) ObjectAllocate(id, OS_OBJECT_EVENT);

    if (event)
    {
        event->bAttribute = attr;
        event->dwFlag = flag;
        link = &event->Wait;
        link->pHead = TASK_ZERO;
        link->pRear = TASK_ZERO;
        status = OS_STATUS_OK;
    }
    else
        status = OS_STATUS_INVALID_ID;

    IntEnable();

    return status;
}



///
///@ingroup EVENT
///@brief   Destroy a specific eventflag object
///
///@param   id      object ID
///
///@retval  OS_STATUS_OK            Eventflag destroyed successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range
///
///@remark  This service call distroy the specific eventflag object. It also put all tasks waiting the flag event
///         into ready state.
///
SDWORD EventDestroy(BYTE id)
{
    register ST_EVENT *event;
    register ST_TASK_LIST *link;
    register ST_TASK *task;

    IntDisable();                   // disable INT and save the old status

    event = EventGetPoint(id);

    if (event == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    link = &event->Wait;

    while (link->pHead != TASK_ZERO)
    {
        task = TaskDequeue(link);
        task->dwObjectBuffer = 0;

        if(task->dwSleepTime != 0)
            TaskCancelSleeping(task);

        TaskSchedule(task);
    }

    ObjectFree(id);                 //undefine   Joshua
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup EVENT
///@brief   Clear an eventflag
///
///@param   id          Eventflag ID
///@param   pattern     Bit pattern to clear
///
///@retval  OS_STATUS_OK            Eventflag clear successfully
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a eventflag.
///
///@remark  This service call will do an AND operation between the 'pattern' and the 'dwFlag'
///         of this eventflag and save it to 'dwFlag'. Thay is, this service call clear all
///         the bits in 'dwFlag' corresponding to '0' bit in 'pattern'.
///
SDWORD EventClear(BYTE id, DWORD pattern)
{
    register ST_EVENT *event;
    SWORD status;

    IntDisable();                   // disable INT and save the old status
    event = EventGetPoint(id);

    if (event)
    {
        event->dwFlag &= pattern;
        status = OS_STATUS_OK;
    }
    else
        status = OS_STATUS_INVALID_ID;

    IntEnable();

    return status;
}



///
///@ingroup EVENT
///@brief   Set an eventflag
///
///@param   id          Eventflag ID
///@param   pattern     Bit pattern to set
///
///@retval  OS_STATUS_OK            Eventflag set successfully
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a eventflag.
///
///@remark  This service call will do an OR operation between the 'pattern' and the 'dwFlag'
///         of this eventflag and save it to 'dwFlag'. That is, this service call set all
///         the bits of 'dwFlag' corresponding to '1' bit in 'pattern'.
///@remark  After the eventflag's bit pattern is updated, any tasks that  satisfy their release
///         conditions are released from waiting. Specifically, each task in the eventflag's
///         wait queue is checked starting from the head and is released from waiting if its
///         release condition is satisfied. Each of the released tasks receives OS_STATUS_OK
///         from this service call. It also receives the bit pattern of the eventflag satisfying
///         the task's releasing condition.
///@remark  If the eventflag's attribute has OS_ATTR_EVENT_CLEAR set, this service call will
///         clear eventflag's bit pattern once a task matching the releasing condition. If this
///         attribute is not set, all the task in the wait queue will be checked and will be
///         released if they match the releasing condition.
///
SDWORD EventSet(BYTE id, DWORD pattern)
{
    register ST_EVENT *event;
    register ST_TASK *task, *next;
    register ST_TASK_LIST *link;
    register DWORD condition;
    register BYTE mode;

    IntDisable();               // disable INT and save the old status
    event = EventGetPoint(id);

    if (event == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    event->dwFlag |= pattern;

    // check the releasing condition for each task in the wait queue
    link = &event->Wait;
    task = link->pHead;

    while (task != TASK_ZERO)
    {
        next = (ST_TASK *) task->pNext;	// save the next task in the list //move by Bill 031308
        pattern = event->dwFlag & task->dwObjectBuffer;
        mode = (BYTE) task->dwObjectScratch;

        if ((mode == OS_EVENT_AND && pattern == task->dwObjectBuffer) ||
            (mode == OS_EVENT_OR && pattern))
        {
            //task->dwObjectBuffer = event->dwFlag;	// save the releasing bit pattern
            task->dwObjectBuffer = pattern;	// save the releasing bit pattern
            //next = (ST_TASK *) task->pNext;	// save the next task in the list //mask by Bill 031308
            TaskWithdraw(task);
            TaskSchedule(task);

            if(task->dwSleepTime != 0)
                TaskCancelSleeping(task);

            if (event->bAttribute & OS_ATTR_EVENT_CLEAR)
            {
                if(event->bAttribute & OS_ATTR_WAIT_MULTIPLE)
                    event->dwFlag &= (~task->dwObjectBuffer);
                else
                    event->dwFlag = 0;

                //break;          // if CLEAR attribute set, clear all bit and stop checking
            }

            if(event->dwFlag == 0)
                break;
        }

        task = next;            // shift to next task in the wait queue
    }

    ContextUpdate();
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup EVENT
///@brief   Wait for an specific eventflag
///
///@param   id          Eventflag ID
///@param   pattern     Bit pattern to set
///@param   mode        The waiting condition operation mode
///@param   *release    Return the matching bits pattern
///
///@retval  OS_STATUS_OK            Return without any problems.
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a eventflag.
///@retval  OS_STATUS_ILLEGAL       In this service call, its mean there already is one task
///                                 in the wait queue and the attribute is set to OS_ATTR_WAIT_SINGLE.
///
///@remark  This service call cause invoking task to wait until the eventflag specified by 'id'
///         satisfies the release condition. The release condition is determined by the bit pattern
///         specified by the 'pattern' and 'mode'. Once the release condition is satisfied, the bit
///         pattern will save to 'release' when return.
///@remark  If the release condition is already satisfied when the service calls are invoked, the
///         service calls return without causing the invoking task to wait. The eventflag's bit
///         pattern will save to 'release'. In addition, when the eventflag's attribute has
///         OS_ATTR_EVENT_CLEAR set, all the bits in the eventflag's bit pattern will be cleared.
///@remark  If the release condition is not satisfied, the invoking task may be placed in the eventflag's
///         wait queue to wait for the release condition happen. The queuing operation is illegal when
///         the eventflag's attribute has not the OS_ATTR_WAIT_MULTIPLE set and there is already a
///         task in the wait queue. In this case, OS_STATUS_ILLEGAL will be returned.
///@remark  In the other way, if the eventflag's attribute has OS_ATTR_WAIT_MULTIPLE set, the invokiing
///         will queue into the eventflag's wait queue depend on the attribute OS_ATTR_FIFO and
///         OS_ATTR_PRIORITY. If the eventflag's attribute has the OS_ATTR_FIFO set, the task will
///         queue with the FIFO order. If it has the OS_ATTR_PRIORITY set, then the order will be arranged
///         by the priority of each task that has queued in the wait queue.
///@remark  The argument 'mode' can be specified as OS_EVENT_AND or OS_EVENT_OR. When 'mode' is OS_EVENT_AND,
///         the release condition requires all the bit in pattern to be set. On the contrary, if 'mode' is
///         OS_EVENT_OR, the release condition only requires at least one bit in 'pattern' to be set.
///
SDWORD EventWaitWithTO(BYTE id, DWORD pattern, BYTE mode, DWORD * release, DWORD waittime)
{
    register ST_EVENT *event;
    register ST_TASK *me;
    register DWORD pat;

    IntDisable();               // disable INT and save the old status
    event = EventGetPoint(id);

    if (event == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    pat = pattern & event->dwFlag;
    //if ((mode == OS_EVENT_AND && pat == event->dwFlag) || (mode == OS_EVENT_OR && pat))
    if ((mode == OS_EVENT_AND && pat == pattern) || (mode == OS_EVENT_OR && pat))
    {
        *release = pat;

        if (event->bAttribute & OS_ATTR_EVENT_CLEAR)
        {
            if(event->bAttribute & OS_ATTR_WAIT_MULTIPLE)
                event->dwFlag &= (~pat);
            else
                event->dwFlag = 0;
        }

        IntEnable();

        return OS_STATUS_OK;
    }

    // when eventflag is set to wait single and the wait queue is not empty
    // the OS_STATUS_ILLEGAL will be issued
    if (event->Wait.pHead != TASK_ZERO && !(event->bAttribute & OS_ATTR_WAIT_MULTIPLE))
    {
        IntEnable();

        return OS_STATUS_ILLEGAL;
    }

    me = TaskDeschedule();
    me->bState = OS_STATE_WAITING;
    me->dwObjectBuffer = pattern;
    me->dwObjectScratch = mode;

    if (event->bAttribute & OS_ATTR_PRIORITY)
        TaskInsert(&event->Wait, me);
    else
        TaskAppend(&event->Wait, me);

    TaskSetSleeping(me, waittime);
    ContextUpdate();
    *release = me->dwObjectBuffer;
    me->dwObjectBuffer = 0;

    if(me->bState == OS_STATE_WAKEUP)
    {
        me->bState = OS_STATE_READY;
        mpDebugPrint("--E-- Event-%2d timeout !!!", id);
        IntEnable();

        return OS_STATUS_TIMEOUT;
    }
    else
    {
        IntEnable();

        return OS_STATUS_OK;
    }
}



SDWORD EventWait(BYTE id, DWORD pattern, BYTE mode, DWORD * release)
{
    return EventWaitWithTO(id, pattern, mode, release, 0);
}


///
///@ingroup EVENT
///@brief   Polling for an specific eventflag (only inquire)
///
///@param   id          Eventflag ID
///@param   pattern     Bit pattern to set
///@param   mode        The waiting condition operation mode
///@param   *release    Return the bits pattern
///
///@retval  OS_STATUS_OK                The flag satisfy the polling condition without any problems.
///@retval  OS_STATUS_INVALID_ID        The ID number out of range or the object indecated by
///                                     this ID is not a eventflag.
///@retval  OS_STATUS_ILLEGAL           In this service call, its mean there already is one task
///                                     in the wait queue and the attribute is set to OS_ATTR_WAIT_SINGLE.
///@retval  OS_STATUS_POLLING_FAILURE   The flag doesn't satisfy the polling condition.
///
///@remark  This service call check if the eventflag specified by 'id' satisfies the release condition or not.
///         The release condition is determined by the bit pattern specified by the 'pattern' and 'mode'.
///         If the release condition is satisfied, OS_STATUS_OK return, else the OS_STATUS_POLLING_FAILURE
///         return. In the case of that release condition is met, the bit pattern will save to 'release'.
///         In addition, if the eventflag's attribute has OS_ATTR_EVENT_CLEAR set in the same case, all the
///         bits in the eventflag's bit pattern will be cleared.
///@remark  The argument 'mode' can be specified as OS_EVENT_AND or OS_EVENT_OR. When 'mode' is OS_EVENT_AND,
///         the release condition requires all the bit in pattern to be set. On the contrary, if 'mode' is
///         OS_EVENT_OR, the release condition only requires at least one bit in 'pattern' to be set.
///
SDWORD EventPolling(BYTE id, DWORD pattern, BYTE mode, DWORD * release)
{
    register ST_EVENT *event;
    register DWORD pat;
    SDWORD status = OS_STATUS_OK;

    IntDisable();               // disable INT and save the old status
    event = EventGetPoint(id);

    if (event == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    pat = pattern & event->dwFlag;

    if ((mode == OS_EVENT_AND && pat == pattern) ||
        (mode == OS_EVENT_OR && pat))
    {
        *release = pat;

        if (event->bAttribute & OS_ATTR_EVENT_CLEAR)
        {
            if (event->bAttribute & OS_ATTR_WAIT_MULTIPLE)
                event->dwFlag &= (~pattern);
            else
                event->dwFlag = 0;

        }
    }
    else
        status = OS_STATUS_POLLING_FAILURE;

    IntEnable();

    return status;
}

