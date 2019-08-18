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
* Filename      : semaphore.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@defgroup    SEMA    Semaphore
///
/// A semaphore is an object used for mutual exclusive and synchronization. A semaphore
/// indicates availability and the number of unused resources by a resource count. Semaphore
/// functions include the ability to create a semaphore, to wait for or polling a semaphore
/// and release resources of a semaphore. An eventflag is an object identified by an ID number.
/// The ID number is ranged form 1 to 31 in this implementation.
///
/// A semaphore has an associated resource count and a wait queue. The resource count indicates
/// the resource availability or the number of unused resources. The wait queue manages the
/// tasks waiting for resources from the semaphore. When a task releases a semaphore resource,
/// the resource count is incremented by 1. When a task acquires a semaphore resource, the
/// resource count is decremented by 1. If a semaphore has no resources available or more precisely
/// the resource count is 0, a task attempting to acquire a resource will wait in the wait queue
/// until a resource is returned to the semaphore.
///
///@image html SemaphoreState.gif
///

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "osinner.h"
#include "taskid.h"

//#define __DEBUG_SEMA_INFO__

///
///@ingroup SEMA
///@brief   Create a semaphore
///
///@param   id      An specified semaphore ID number
///@param   attr    attribute of this semaphore
///@param   count   the maximal resource count of this semaphore
///
///@retval  OS_STATUS_OK            Semaphore create successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range
///@retval  OS_STATUS_ID_INUSED     Specified task ID has been allocated for another object
///
///@remark  This service call create a semaphore with an ID number spicified by 'id' and has
///         a resource count number specified by 'count'. Note that the maximal resource
///         number can exceed 256 by this impletement.
///@remark  'attr' can be OS_ATTR_FIFO or OS_ATTR_PRIORITY. If OS_ATTR_FIFO, the semaphore's
///         wait queue will be in FIFO order. If OS_ATTR_PRIORITY, the semaphore's wait queue
///         is in task priority order.
///
SDWORD SemaphoreCreate(BYTE id, BYTE attr, BYTE count)
{
    register ST_SEMAPHORE *semaphore;
    register ST_TASK_LIST *link;
    SWORD status;

    IntDisable();       // disable INT and save the old status

    semaphore = (ST_SEMAPHORE *) ObjectAllocate(id, OS_OBJECT_SEMAPHORE);

    if (semaphore)
    {
        semaphore->bAttribute = attr;
        semaphore->bCounter = count;
        semaphore->bMaximal = count;
        link = &semaphore->Wait;
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
///@ingroup SEMA
///@brief   Destroy a specific semaphore
///
///@param   id      An specified semaphore ID number
///
///@retval  OS_STATUS_OK            Semaphore destroyed successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range
///
///@remark  This service call distroy the specific semaphore. Note that it DO NOT put the task waiting the semaphore
///         into ready state. All these saks need to be restarted.
///
SDWORD SemaphoreDestroy(BYTE id)
{
    register ST_SEMAPHORE *semaphore;

    IntDisable();
    semaphore = SemaphoreGetPoint(id);

    if (semaphore == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    ObjectFree(id);
    IntEnable();

    return OS_STATUS_OK;
}


///
///@ingroup SEMA
///@brief   Release semaphore resource
///
///@param   id      semaphore ID number
///
///@retval  OS_STATUS_OK            Resource release successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range or is inused
///
///@remark  This service call release one resource to the semaphore specified by 'id'. If any
///         tasks are waiting for the specified semaphore, the task at the head of the semaphore's
///         wait queue is released form waiting. When this happens, the associated semaphore resource
///         is not changed. The released task receives OS_STATUS_OK from the service call that caused
///         it to wait in the semaphore's wait queue. If no tasks are waiting for the specified semaphore,
///         the semaphore resource count is incremented by 1.
///@remark  This service call return OS_STATUS_OVERFLOW if incremented resource counter exceed the
///         the maximal resource value of this semaphore.
///
SDWORD SemaphoreRelease(BYTE id)
{
    register ST_SEMAPHORE *semaphore;
    register ST_TASK_LIST *link;
    SWORD status;

    IntDisable();               // disable INT and save the old status
    semaphore = SemaphoreGetPoint(id);

    if (semaphore == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    if (semaphore->bCounter == semaphore->bMaximal)
    {
        IntEnable();
        return OS_STATUS_OVERFLOW;
    }

    semaphore->bCounter++;
    link = &semaphore->Wait;

    if (link->pHead != TASK_ZERO)
    {
        semaphore->bCounter--;
        TaskSchedule(TaskDequeue(link));
    }

    ContextUpdate();
    IntEnable();
    return OS_STATUS_OK;
}



///
///@ingroup SEMA
///@brief   Wait for an semaphore has resource available
///
///@param   id          Semaphore ID number
///
///@retval  OS_STATUS_OK            Return without any problems and has got a resource.
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a semaphore.
///
///@remark  This service call cause invoking task to wait until the semaphore specified by 'id'
///         has any resource available. Or, by another words, the resource counter greater then
///         0. If the recource counter is already not zero when the service calls are invoked, the
///         service calls return without causing the invoking task to wait.
///@remark  If the semaphore has no any resource available, then the invoking task will be placed into
///         the wait queue to wait for any resource has been released. The queuing order will depend
///         on the attribute of the semaphore.
///
SDWORD SemaphoreWait(BYTE id)
{
    register ST_SEMAPHORE *semaphore;
    register ST_TASK *me;

    IntDisable();				// disable INT and save the old status
    semaphore = SemaphoreGetPoint(id);

    if (semaphore == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    if (semaphore->bCounter)
    {
#ifdef __DEBUG_SEMA_INFO__
        MP_ALERT("Task-%d waited Sema-%d!!!", TaskGetId(), id);
#endif
        semaphore->bCounter--;
        IntEnable();
        return OS_STATUS_OK;
    }

#ifdef __DEBUG_SEMA_INFO__
    MP_ALERT("Task-%d is locked by Sema-%d!!!", TaskGetId(), id);
#endif
    me = TaskDeschedule();
    me->bState = OS_STATE_WAITING;
    if (semaphore->bAttribute & OS_ATTR_PRIORITY)
        TaskInsert(&semaphore->Wait, me);
    else
        TaskAppend(&semaphore->Wait, me);

    CONTEXT_SWITCH();

#ifdef __DEBUG_SEMA_INFO__
    MP_ALERT("Task-%d waited Sema-%d!!!", TaskGetId(), id);
#endif

    IntEnable();
    return OS_STATUS_OK;
}



///
///@ingroup SEMA
///@brief   Polling a semaphore for any available resource
///
///@param   id          Semaphore ID number
///
///@retval  OS_STATUS_OK                Return without any problems and has got a resource.
///@retval  OS_STATUS_INVALID_ID        The ID number out of range or the object indecated by
///                                     this ID is not a semaphore.
///@retval  OS_STATUS_POLLING_FAILURE   No resource available
///
///@remark  This service call will return the status of the specifed semaphore has available
///         resources or not. It will not cause the invoking task to wait in the wait queue.
///
SDWORD SemaphorePolling(BYTE id)
{
    register ST_SEMAPHORE *semaphore;
    register ST_TASK *me;

    IntDisable();               // disable INT and save the old status
    semaphore = SemaphoreGetPoint(id);

    if (semaphore == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    if (semaphore->bCounter)
    {
        semaphore->bCounter--;
        IntEnable();
        return OS_STATUS_OK;
    }

    IntEnable();
    return OS_STATUS_POLLING_FAILURE;
}

/*
 * Set maximum value of a semaphore
 */
SDWORD SemaphoreSet(BYTE id, BYTE max)
{
    register ST_SEMAPHORE *semaphore;
    SWORD status = OS_STATUS_OK;

    IntDisable();       // disable INT and save the old status

    semaphore = SemaphoreGetPoint(id);

    if (semaphore)
        semaphore->bMaximal = max;
    else
        status = OS_STATUS_INVALID_ID;

    IntEnable();

    return status;
}

