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
* Filename      : task.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@defgroup    TASK    Task
///@image html TaskState.gif
///

///
///@ingroup     TASK
///@defgroup    TM      Task Management
///
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0


/*
// Include section
*/
#include "mpTrace.h"
#include "os.h"
#include "taskid.h"

//#define __ENABLE_TASK_TIME_SCHEDULING__
//#define __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__

ST_TASK TaskBank[OS_MAX_NUMBER_OF_TASK] = {0};    // task buffer
static ST_TASK *pRunningSpot = 0;
static ST_TASK_LIST ReadyArray[OS_MAX_PRIORITY] = {0};   // schedule data structure
static ST_TASK_LIST DelayList = {0};                     // seelp task buffer

static BYTE bDispatchPriority;
BOOL boolContext = 1;

#ifdef __ENABLE_TASK_TIME_SCHEDULING__
static BYTE taskYieldIdNum = 0;

#ifdef __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__
static WORD dropTaskYieldMsg = FALSE;
static ST_EXCEPTION_MSG stTaskYieldExceptionCont = {0};
static BYTE taskYieldIdCont[OS_MAX_NUMBER_OF_TASK] = {0};
#endif
#endif

BOOL ContextStatusGet(void)
{
    return boolContext;
}



static volatile SWORD taskDeactiveLevel = 0;

void TaskContextDeactive(void)
{
    IntDisable();
    boolContext = 0;
    taskDeactiveLevel++;
}



void TaskContextActive(void)
{
    IntDisable();

    if (taskDeactiveLevel > 0)
        taskDeactiveLevel--;
    else
    {
        taskDeactiveLevel = 0;
        MP_ALERT("--W-- %s: Should not happen !!!", __FUNCTION__);
    }

    if (0 == taskDeactiveLevel)
        boolContext = 1;

    IntEnable();
}



void *TaskInit()
{
    register ST_TASK *task;
    register ST_TASK_LIST *list;
    WORD index;

    boolContext = 1;

    // initialize all task before it create
    for (index = 0, task = TASK_ZERO; index < OS_MAX_NUMBER_OF_TASK; index++, task++)
    {
        task->bState = OS_STATE_NON_EXISTENT;
        task->bBasePriority = 0;
        task->bCurrPriority = 0;
        task->bId = index;
        task->pNext = TASK_ZERO;
        task->pSleepNext = TASK_ZERO;
        task->dwSleepTime = 0;
        task->pdwExit = OS_NULL;
        task->dwTimeSchedulePeriod = 0;
        task->dwTimeScheduleTimeRemain = 0;
    }

    // initialize the dispatching table for priority handling
    list = ReadyArray;
    for (index = 0; index < OS_MAX_PRIORITY; index++)
    {
        list->dwCounter = 0;
        list->pHead = TASK_ZERO;
        list->pRear = TASK_ZERO;
        list++;
    }

    // set the task[2] to be the main task
    task = &TaskBank[MAIN_TASK];
    task->bBasePriority = CONTROL_PRIORITY;
    task->bCurrPriority = CONTROL_PRIORITY;
    task->bState = OS_STATE_RUNNING;

    pRunningSpot = task;
    bDispatchPriority = CONTROL_PRIORITY;
    list = &ReadyArray[CONTROL_PRIORITY];
    list->pHead = task;
    list->pRear = task;
    list->dwCounter = 1;

    DelayList.pHead = TASK_ZERO;
    DelayList.pRear = TASK_ZERO;
    DelayList.dwCounter = 0;

    MP_ALERT("TOTAL_TASK_NUMBER = %d, OS_MAX_NUMBER_OF_TASK = %d", TOTAL_TASK_NUMBER, OS_MAX_NUMBER_OF_TASK);


    if (TOTAL_TASK_NUMBER >= OS_MAX_NUMBER_OF_TASK)
    {
        MP_ALERT("--E-- TOTAL_TASK_NUMBER exceed to OS_MAX_NUMBER_OF_TASK, increase the value of OS_MAX_NUMBER_OF_TASK");
        __asm("break 100");
    }
}



ST_TASK *TaskGetMe()
{
    return pRunningSpot;
}



///
///@ingroup TM
///@brief   Get the ID of the current running task.
///
BYTE TaskGetId(void)
{
    return pRunningSpot->bId;
}



///
///@ingroup TM
///@brief   Get the running priority of the current task.
///
BYTE TaskGetPriority(void)
{
    return pRunningSpot->bCurrPriority;
}



// append a task to the rear of a task list
void TaskAppend(ST_TASK_LIST * link, ST_TASK * newcommer)
{
    register ST_TASK *task;

    newcommer->pList = link;

    if ((DWORD) link->pHead == (DWORD) TASK_ZERO)   // mean this link is not used
        link->pHead = newcommer;    // append to be the first task
    else
    {
        task = (ST_TASK *) link->pRear; // append to the rear
        task->pNext = newcommer;
    }

    link->pRear = newcommer;
    newcommer->pNext = TASK_ZERO;
    link->dwCounter++;              // increase the task count in this priority link
}



// preempt a task at the head of a task list
void TaskPreempt(ST_TASK_LIST * link, ST_TASK * newcommer)
{
    newcommer->pList = link;

    if ((DWORD) link->pHead == (DWORD) TASK_ZERO)   // mean this link is not used
    {
        link->pRear = newcommer;    // also set the rear to the incomming task
    }

    newcommer->pNext = (void *) link->pHead;
    link->pHead = newcommer;
    link->dwCounter++;              // increase the task count in this priority link
}



// insert a task into a task list in the order of task priority
// it will following the last task that has the same priority
void TaskInsert(ST_TASK_LIST * link, ST_TASK * newcommer)
{
    register ST_TASK *task, *next;
    register BYTE priority;

    newcommer->pList = link;
    priority = newcommer->bCurrPriority;
    task = (ST_TASK *) link->pHead;

    // if the imcomming task's priority greater than first task, preempty it directly
    if (task->bCurrPriority < priority)
    {
        link->pHead = newcommer;
        newcommer->pNext = task;
    }
    // if the imcomming task's priotity less than the first task, find out the rear
    // task of the same priority and then insert behind it.
    else
    {
        next = task->pNext;
        while (next->bCurrPriority >= priority)
        {
            task = next;
            next = task->pNext;
        }

        task->pNext = newcommer;
        newcommer->pNext = next;
    }

    link->dwCounter++;              // increase the task count in this priority link
}



// extract a task from the head of a task list
ST_TASK *TaskDequeue(ST_TASK_LIST * link)
{
    register ST_TASK *task;

    task = (ST_TASK *) link->pHead;
    link->pHead = task->pNext;

    if (link->pHead == TASK_ZERO)
        link->pRear = TASK_ZERO;

	    link->dwCounter--;      	// decrease the task count in this priority link

    return task;
}



// withdraw task from the task list it belong to, it can un-link the task at
// anywhere of the task list.
ST_TASK *TaskWithdraw(ST_TASK * task)
{
    register ST_TASK *previous;
    register ST_TASK_LIST *link;

    link = task->pList;
    previous = link->pHead;

    if ((DWORD) previous == (DWORD) task)
    {
        //link->dwCounter--;            // move in TaskDequeue
        return TaskDequeue(link);
    }

    while ((DWORD) previous->pNext != (DWORD) TASK_ZERO)
    {
        if ((DWORD) previous->pNext == (DWORD) task)
        {
            link->dwCounter--;          // decrease the task count in this priority link
            previous->pNext = task->pNext;

            if ((DWORD) task == (DWORD) link->pRear)
                link->pRear = previous;

            break;
        }

        previous = previous->pNext;
    }

    return task;
}



// arrangge task into ready array
void TaskSchedule(ST_TASK * task)
{
    register BYTE priority;
    register ST_TASK_LIST *link;

    task->bState = OS_STATE_READY;
    priority = task->bCurrPriority;
    link = &ReadyArray[priority];

    if (priority > bDispatchPriority)   // update the current highest dispatch priority
        bDispatchPriority = priority;

    TaskAppend(link, task);
    //link->dwCounter++;        // move in TaskAppend
}



// pull-out a task from the beginning of the ready array
ST_TASK *TaskDeschedule()
{
    register ST_TASK *task;
    register ST_TASK_LIST *link;

    link = &ReadyArray[bDispatchPriority];
    task = TaskDequeue(link);
    //link->dwCounter--;        // Move in TaskDeque

    // check if the number of task has this priority will be empty or not.
    // If it is, update the dispatch priority
    while (link->dwCounter == 0)
    {
        bDispatchPriority--;

        if (bDispatchPriority == 0)
            break;              // if go to the bottom, must quit

        link = &ReadyArray[bDispatchPriority];
    }

    return task;
}



///
///@ingroup TM
///@brief   Creat Task
///
///@param   id          ID number of the task te be created
///@param   *entry      Task procedure starting point, i.e., the task function name
///@param   priority    Start priority of this task
///@param   stacksize   The byte number of the stack of this task
///
///@retval  OS_STATUS_OK            Task create successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, note that the ID range is 3 <= ID <=31
///@retval  OS_STATUS_NO_MEMORY     Insufficient memory for stack
///@retval  OS_STATUS_WRONG_PARAM   Wrong priority
///@retval  OS_STATUS_ID_INUSED     Specified task ID has been allocated for another task
///
///@remark  TaskCreate create a task with an ID number specifed by 'id'. Task body will be
///         defined by a none-returned function with the function name 'entry' and has only
///         one pointer type argument. The task will be moved form NON-EXIST state to DORMANT
///         state. TaskCreate will not activate the task. See TaskStartup to know how a task
///         can more from DORMANT to READY state.
///@remark  The kernel will allocate 'stacksize' byte of memory to be the stack of this task
///         from system memory heap. If the allocatable memory is less then 'stacksize', then
///         there is no memory will be allocated and task will not be created. The error
///         message OS_STATUS_NO_MEMORY will return at the same time.
///
SDWORD TaskCreate(BYTE id, void *entry, BYTE priority, DWORD stacksize)
{
    register ST_TASK *task;

    // argument list check
    if (id == NULL_TASK)
        return OS_STATUS_INVALID_ID;

    if (id > OS_MAX_NUMBER_OF_TASK - 1)
    {
        mpDebugPrint("--E-- Task ID-%3d was exceed to max ID number %d !!!", id, OS_MAX_NUMBER_OF_TASK - 1);
        __asm("break 100");

        return OS_STATUS_INVALID_ID;
    }

    if (priority == 0 || priority > OS_MAX_PRIORITY - 1)
        return OS_STATUS_WRONG_PARAM;

    IntDisable();               // disable INT and save the old status

    task = &TaskBank[id];

    stacksize = stacksize & 0xfffffffc; // make sure this number is a multiple of 4
    task->pdwStackBase = (DWORD *)((DWORD)ker_mem_malloc(stacksize, id) & (~0x20000000));

    if ((DWORD) task->pdwStackBase == 0)
    {
        MP_ALERT("Task%d Stack malloc fail !!!", (WORD) id);
        IntEnable();

        return OS_STATUS_NO_MEMORY;
    }

    // setup the task stack
    task->dwStackSize = stacksize;
    task->pdwStackPoint = task->pdwStackBase + ((stacksize - sizeof(CONTEXT)) >> 2);

    task->bState = OS_STATE_DORMANT;
    task->bBasePriority = priority;
    task->bCurrPriority = priority;
    task->pdwEntry = entry;
    task->pdwExit = OS_NULL;

    IntEnable();

    return OS_STATUS_OK;
}



void TaskReset(BYTE id, void *entry, BYTE priority, DWORD stacksize)
{
    register ST_TASK *task;

    if ((id == NULL_TASK) || (id >= OS_MAX_NUMBER_OF_TASK))
        return;

    IntDisable();               // disable INT and save the old status

    task = &TaskBank[id];
    task->dwStackSize = stacksize & 0xfffffffc; // make sure this number is a multiple of 4
    task->pdwStackPoint = task->pdwStackBase + ((stacksize - sizeof(CONTEXT)) >> 2);
    memset(task->pdwStackBase,0x0,stacksize>>2);
    // setup the task stack
    task->bState = OS_STATE_DORMANT;
    task->bBasePriority = priority;
    task->bCurrPriority = priority;
    task->pdwEntry = entry;

    IntEnable();
}



///
///@ingroup TM
///@brief   Activate task from DORMANT state to READY state
///
///@param   id      ID number of the task to be activated
///@param   arg1    Arguments for the task body function, the number of arguments can be 0 to 4.
///@param   arg2
///@param   arg3
///@param   arg4
///
///@retval  OS_STATUS_OK            The specified task has activated successfully.
///@retval  OS_STATUS_INVALID_ID    Wrong specified task ID.
///@retval  OS_STATUS_WRONG_STATE   The state of this task is not DORMANT state.
///
///@remark  This service call startup the task specified by 'id'. The task is moved from the DORMANT
///         state to the READY state and the actions that must be taken at task startup time are
///         performed. Up to 4 arguments can be passed to the task body function by 'arg1'..'arg4'.
///
SDWORD TaskStartup(BYTE id, ...)
{
    register ST_TASK *task;
    register CONTEXT *sp;
    register DWORD host_status;
    va_list argsList;

    va_start(argsList, id);                         /* get variable arg list address */

    if ((id == NULL_TASK) || (id >= OS_MAX_NUMBER_OF_TASK))
        return OS_STATUS_INVALID_ID;

    host_status = IntDisable(); // disable INT and save the old status
    task = &TaskBank[id];

    if (task->bState != OS_STATE_DORMANT)
    {
        IntEnable();

        return OS_STATUS_WRONG_STATE;
    }

    if ((DWORD)task->pdwStackBase == 0)
    {
        task->pdwStackBase = (DWORD *) ker_mem_malloc(task->dwStackSize, id);

        if ((DWORD) task->pdwStackBase == 0)
        {
            MP_ALERT("--E-- Out of memory for task %d", id);
            IntEnable();

            return OS_STATUS_NO_MEMORY;
        }

        task->pdwStackPoint = task->pdwStackBase + ((task->dwStackSize - sizeof(CONTEXT)) >> 2);
    }

    task->bState = OS_STATE_READY;
    task->pSleepNext = TASK_ZERO;
    task->dwSleepTime = 0;

    // set the argument for the task function arguments
    sp = (CONTEXT *) (task->pdwStackBase + ((task->dwStackSize - sizeof(CONTEXT)) >> 2));

    task->dwParameter[0] = va_arg(argsList, DWORD);
    task->dwParameter[1] = va_arg(argsList, DWORD);
    task->dwParameter[2] = va_arg(argsList, DWORD);
    task->dwParameter[3] = va_arg(argsList, DWORD);
    va_end(argsList);

    MP_DEBUG("%s -",__FUNCTION__);
    MP_DEBUG("Arg1 = 0x%04X", task->dwParameter[0]);
    MP_DEBUG("Arg2 = 0x%04X", task->dwParameter[1]);
    MP_DEBUG("Arg3 = 0x%04X", task->dwParameter[2]);
    MP_DEBUG("Arg4 = 0x%04X", task->dwParameter[3]);

    // set the task function entry point
    sp->EPC = (DWORD) task->pdwEntry;
    sp->STATUS = host_status;

    TaskSchedule(task);
    ContextUpdate();
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup TM
///@brief   Get the parameter form task stack
///
///@param   id      ID number of the task to be activated
///@param   *arg1   Arguments for the task body function, the number of arguments can be 1 to 4.
///@param   *arg2
///@param   *arg3
///@param   *arg4
///
///@retval  OS_STATUS_OK            The specified task has activated successfully.
///@retval  OS_STATUS_INVALID_ID    Wrong specified task ID.
///@retval  OS_STATUS_WRONG_STATE   The state of this task is not DORMANT state.
///
///@remark  This service call startup the task specified by 'id'. The task is moved from the DORMANT
///         state to the READY state and the actions that must be taken at task startup time are
///         performed. Up to 4 arguments can be passed to the task body function by 'arg1'..'arg4'.
///
SDWORD TaskStartupParamGet(BYTE id, DWORD *arg1, DWORD *arg2, DWORD *arg3, DWORD *arg4)
{
    register ST_TASK *task;

    if (id == 0 || id > OS_MAX_NUMBER_OF_TASK - 1)
        return OS_STATUS_INVALID_ID;

    IntDisable();
    task = &TaskBank[id];

    // Get the argument for the task function arguments
    if (arg1)
        *arg1 = task->dwParameter[0];

    if (arg2)
        *arg2 = task->dwParameter[1];

    if (arg3)
        *arg3 = task->dwParameter[2];

    if (arg4)
        *arg4 = task->dwParameter[3];

    IntEnable();

    return OS_STATUS_OK;
}




///
///@ingroup TM
///@brief   Terminate the invoking task and move to DORMANT state.
///
///@remark  The invoking task will be terminated and moved from the RUNNING state
///         to the DORMANT state. This function will never return.
///
void TaskTerminate(BYTE id)
{
    register ST_TASK *task;

    IntDisable();                   // disable INT and save the old status

    task = &TaskBank[id];

    if(task->pdwExit)
        task->pdwExit();            // call the disarmming procedure first

    if((DWORD) task == (DWORD) pRunningSpot)
        task = TaskDeschedule();
    else
        task = TaskWithdraw(task);

    if(task->dwSleepTime != 0)
        TaskCancelSleeping(task);

    FreeTaskUserMem(id);
    ker_mem_free_task_mem(id);
    task->pdwStackBase = 0;
    task->bState = OS_STATE_DORMANT;

    CONTEXT_SWITCH();               // use this macro because here is in context
}



SDWORD TaskSetExitHandle(BYTE id, void * ExitHandle)
{
    if ((id == NULL_TASK) || (id >= OS_MAX_NUMBER_OF_TASK))
        return OS_STATUS_INVALID_ID;

    TaskBank[id].pdwExit = ExitHandle;

    return OS_STATUS_OK;
}



///
///@ingroup TASK
///@defgroup    TS  Task Synchronization
///

///
///@ingroup TS
///@brief   Make the invoking task sleep and move to SLEEPING state.
///
///@param   sleeptime  How many mini seconds the task sleep
///
///@return  This service call will always return OS_STATUS_OK.
///
///@remark  This service call cause the invoking task to sleep and will not return until
///         another task use the service call TaskWakeup to wake the invoking task up then
///         can return from this service call.
///
///@remark  If the sleeptime is greater than zero, the calling task will back to ready state
///         after 'sleeptime'mini seconds.
///
SDWORD TaskSleep(DWORD sleeptime)
{
    register ST_TASK *task;

    if (TaskGetId() == GHOST_TASK)
        return;

    IntDisable();               // disable INT and save the old status
    task = TaskDeschedule();
    task->bState = OS_STATE_SLEEPING;

    if(sleeptime)
    {
        TaskPreempt(&DelayList, task);
        TaskSetSleeping(task, sleeptime);
    }
    else
        task->dwSleepTime = 0;

    CONTEXT_SWITCH();           // use this macro because here is in context
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup TS
///@brief   Wake the specified task up and move it to READY state.
///
///@param   id  The ID number of the task that will be waked up
///
///@retval  OS_STATUS_OK            The target task has been waked up successfully.
///@retval  OS_STATUS_INVALID_ID    Wrong specified task ID.
///@retval  OS_STATUS_WRONG_STATE   The state of this task is not SLEEPING state.
///
///@remark  See TaskSleep.
SDWORD TaskWakeup(BYTE id)
{
    register ST_TASK *task;

    if ((id == NULL_TASK) || (id >= OS_MAX_NUMBER_OF_TASK))
        return OS_STATUS_INVALID_ID;

    IntDisable();                   // disable INT and save the old status
    task = &TaskBank[id];

    if (task->bState != OS_STATE_SLEEPING)
    {
        IntEnable();
        return OS_STATUS_WRONG_STATE;
    }

    if(task->dwSleepTime != 0)      // task is queued in delay list, need dequeue from delay list.
    {
        TaskWithdraw(task);
        TaskCancelSleeping(task);
    }

    TaskSchedule(task);
    ContextUpdate();                // use function because here may or may not in context
    IntEnable();

    return OS_STATUS_OK;
}



void SleepingTaskTimeTick(DWORD timepassed)
{
    register ST_TASK *current, *previous;

    previous = &TaskBank[0];
    current = previous->pSleepNext;

    //while (current != TASK_ZERO)
    while (((DWORD) current != (DWORD) TASK_ZERO) && current)
    {
        if (current->dwSleepTime > timepassed)
        {
            current->dwSleepTime -= timepassed;
            previous = current;
            current = current->pSleepNext;
        }
        else
        {
            current->dwSleepTime = 0;

            // delete from the expiration list
            previous->pSleepNext = current->pSleepNext;
            current->pSleepNext = TASK_ZERO;

            // withdraw from the some task list this task belong to and re-schefule it
            TaskWithdraw(current);
            TaskSchedule(current);

            // overwrite the task state with expiration condition
            current->bState = OS_STATE_WAKEUP;
            current = previous->pSleepNext;
        }
    }

    return;
}


// preempt a task at the head of a task expiration list
// and set it's expiration time if the tick value not equal
// to zero
void TaskSetSleeping(ST_TASK * task, DWORD sleeptime)
{
    if(!sleeptime)    return;

    if (task->bId == GHOST_TASK)
    {
        MP_ALERT("Don't using waiting object function in the idle function !!!");

        return;
    }

    task->dwSleepTime = sleeptime;
    task->pSleepNext = TaskBank[0].pSleepNext;
    TaskBank[0].pSleepNext = task;
}



void TaskCancelSleeping(ST_TASK * task)
{
    register ST_TASK *current, *previous;

    if(!task->dwSleepTime)
        return;                 // if task is not in expireration list, just quit

    previous = &TaskBank[0];
    current = previous->pSleepNext;

    while ((DWORD) current != (DWORD) TASK_ZERO)
    {
        // delete from the expiration list
        if(current == task)
        {
            previous->pSleepNext = current->pSleepNext;
            current->pSleepNext = TASK_ZERO;
            current->dwSleepTime = 0;
            break;
        }

        previous = current;
        current = current->pSleepNext;
    }

    return;
}


///
///@ingroup TS
///@brief   Move the invoking task from RUNNING state to READY state.
///
///@return  Always return OS_STATUS_OK.
///
///@remark  This service call rotate the procedence of the highest priority of tasks. The
///         running task will release from RUNNING state to READY state and append to the
///         task queue of the same priority. If it is the only one task has the highest
///         priority, then this service call will change nothing.
///
SDWORD TaskYield(void)
{
    register ST_TASK *task;

    IntDisable();   // disable INT and save the old status
    task = TaskDeschedule();
    TaskSchedule(task);
    ContextUpdate();    // use function because here may or may not in context
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup TS
///@brief   Change the curent task's priority
///
///@param   priority    The new priority
///
///@retval  OS_STATUS_OK            Task create successfully
///@retval  OS_STATUS_WRONG_PARAM   Wrong priority
///
///@remark  The new priority must be greater than 0 and smaller than the largest priority.
///
SDWORD TaskChangePriority(BYTE priority)
{
    register ST_TASK *task;
    SDWORD ret;

    IntDisable();

    if ((priority > 0) && (priority < OS_MAX_PRIORITY))
    {
        task = pRunningSpot;
        task->bCurrPriority = priority;
        ret = OS_STATUS_OK;
    }
    else
        ret = OS_STATUS_WRONG_PARAM;

    IntEnable();

    return ret;
}



/*
void DelayProcess(DWORD step)
{
    register ST_TASK *task, *next;

    task = DelayList.pHead;

    while (task != TASK_ZERO)
    {
        next = (ST_TASK *) task->pNext;

        if (task->dwObjectBuffer < step)
        {
            TaskWithdraw(task);
            TaskSchedule(task);
        }
        else
            task->dwObjectBuffer -= step;

        task = next;
    }
}


///
///@ingroup TS
///@brief   Delay the invoking task for a specific tick time
///
///@param   tick number that the invoking task will delay
///
///@retval  This service call always return OS_STATUS_OK.
///
///@remark  THis service call will put the invoking task to WAITING state for 'tick' time.
///         One tick will be 1 ms. And the rescheduling time may exceed the setup time 1 to 63 ms.
///         When the delay timer is out, if there are another task have higher priorities.
///         The delayed task just can be rescheduled and will not take the precedence immediately.
///
SDWORD TaskDelay(DWORD tick)
{
    register ST_TASK *task;

    IntDisable();                       // disable INT and save the old status
    task = TaskDeschedule();
    task->bState = OS_STATE_WAITING;
    task->dwObjectBuffer = tick;
    TaskAppend(&DelayList, task);
    CONTEXT_SWITCH();                   // use this macro because here is in context
    IntEnable();

    return OS_STATUS_OK;
}
*/



// when flow is running in the context, then will trying to change context
void ContextUpdate()
{
    register ST_TASK *next;
    register ST_TASK *task;

    if (boolContext)
    {
        next = ReadyArray[bDispatchPriority].pHead;

#ifdef __MONITOR_RESERVED_MEMORY_REGION
        if (mem_ReservedRegionCheck() == TRUE)
            __asm("break 100");
#endif

        if ((DWORD) next != (DWORD) pRunningSpot)
            CONTEXT_SWITCH();

#ifdef __ENABLE_TASK_TIME_SCHEDULING__
        task = &TaskBank[TaskGetId()];

        if (task->dwTimeSchedulePeriod)
        {
            task->dwTimeScheduleTimeRemain = task->dwTimeSchedulePeriod;
        }
#endif
    }
}



SDWORD OsExchangeContext(DWORD sp)
{
    register ST_TASK *next;

#ifdef __MONITOR_RESERVED_MEMORY_REGION
        if (mem_ReservedRegionCheck() == TRUE)
            __asm("break 100");
#endif

    next = ReadyArray[bDispatchPriority].pHead;
    pRunningSpot->pdwStackPoint = (DWORD *) sp;

    if (pRunningSpot->pdwStackPoint < (pRunningSpot->pdwStackBase + 512))
    {
        MP_ALERT("\r\nStack Bomb!!!!! Task Id = %d", pRunningSpot->bId);
        MP_ALERT("Please give enough stack size in CreateTask or reduce your local variable !!!\r\n");
        __asm("nop");__asm("nop");__asm("nop");__asm("nop");__asm("nop");
        __asm("break 100");
        while (1);
    }

#ifdef __MONITOR_CONTEXT_SWITCH
    if (pRunningSpot->bId != next->bId)
    {
        mpDebugPrintChar('T');
        mpDebugPrintValue(next->bId, 3);
        mpDebugPrintChar(' ');
    }
#endif

    sp = (DWORD) next->pdwStackPoint;
    pRunningSpot = next;

    return sp;
}



///
///@ingroup TS
///@brief   Move the invoking task from RUNNING state to READY state by specified task ID.
///
///@param   taskId      The ID number of the task that will be yield
///
///@retval  OS_STATUS_OK            The target task has been waked up successfully.
///@retval  OS_STATUS_INVALID_ID    Wrong specified task ID.
///
///@remark  This service call rotate the procedence of the highest priority of tasks. The
///         running task will release from RUNNING state to READY state and append to the
///         task queue of the same priority. If it is the only one task has the highest
///         priority or the task is not the head of ready list, then this service call will
///         change nothing.
///
SDWORD TaskYieldById(BYTE taskId)
{
#ifdef __ENABLE_TASK_TIME_SCHEDULING__
    register ST_TASK *task;
    register ST_TASK_LIST *link;

    if ((taskId == NULL_TASK) || (taskId >= OS_MAX_NUMBER_OF_TASK))
        return OS_STATUS_INVALID_ID;

    IntDisable();   // disable INT and save the old status

    task = &TaskBank[taskId];
    link = &ReadyArray[task->bCurrPriority];

    if ((DWORD) link->pHead == (DWORD) task)
    {
        if (((DWORD) task == (DWORD) pRunningSpot) && (bDispatchPriority == task->bCurrPriority))
            task = TaskDeschedule();
        else
            task = TaskDequeue(link);

        TaskSchedule(task);
        ContextUpdate();    // use function because here may or may not in context
    }

    IntEnable();
#endif

    return OS_STATUS_OK;
}



///
///@ingroup TS
///@brief   Move the invoking task from RUNNING state to READY state by specified task ID and period.
///
///@param   taskId          The ID number of the task that will be yield
///@param   scheduleTime    the invoking task will be yield per 'scheduleTime', unit is ms.
///
///@retval  OS_STATUS_OK            The target task has been waked up successfully.
///@retval  OS_STATUS_INVALID_ID    Wrong specified task ID.
///
///@remark  This service call cause the invoking task to yield per 'scheduleTime' ms.
///@remark  If the period is equal to zero, the invoking task will be remove from the list of task schedule.
///
SDWORD TaskTimeSchedulingRegister(BYTE taskId, DWORD scheduleTime)
{
#ifdef __ENABLE_TASK_TIME_SCHEDULING__
    register ST_TASK *task;

    if ((taskId == NULL_TASK) || (taskId >= OS_MAX_NUMBER_OF_TASK))
        return OS_STATUS_INVALID_ID;

    IntDisable();   // disable INT and save the old status
    task = &TaskBank[taskId];

    if (task->dwTimeSchedulePeriod != scheduleTime)
    {
        if (task->dwTimeSchedulePeriod == 0)
            taskYieldIdNum++;
        else if (scheduleTime == 0)
        {
            if (taskYieldIdNum)
            taskYieldIdNum--;
        }

        task->dwTimeSchedulePeriod = scheduleTime;
        task->dwTimeScheduleTimeRemain = scheduleTime;
    }

    MP_DEBUG("Task%2d will be yield per %dms, %d", taskId, scheduleTime, taskYieldIdNum);
    IntEnable();
#endif

    return OS_STATUS_OK;
}



#if (defined(__ENABLE_TASK_TIME_SCHEDULING__) && defined(__USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__))
static void taskTimeScheduling(void)
{
    BYTE index;

    for (index = 0; index < taskYieldIdNum; index++)
    {
        TaskYieldById(taskYieldIdCont[index]);
    }

    dropTaskYieldMsg = FALSE;
}
#endif



void TaskTimeSchedulingInit(void)
{
#ifdef __ENABLE_TASK_TIME_SCHEDULING__
    taskYieldIdNum = 0;
#ifdef __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__
    stTaskYieldExceptionCont.dwTag = ExceptionTagReister(taskTimeScheduling);
    stTaskYieldExceptionCont.msgAddr = 0;
    dropTaskYieldMsg = FALSE;
    memset((BYTE *) &taskYieldIdCont, 0, OS_MAX_NUMBER_OF_TASK);

    if (stTaskYieldExceptionCont.dwTag == 0)
    {
        MP_ALERT("--E-- Invalid Exception Tag Index !!!");
        __asm("break 100");
    }
#endif
#endif
}



void TaskTimeSchedulingTimeTick(DWORD timepassed)
{
#ifdef __ENABLE_TASK_TIME_SCHEDULING__
    register ST_TASK *task;
    BYTE id, counter;

#ifdef __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__
    if (dropTaskYieldMsg || (taskYieldIdNum == 0))
        return;
#else
    if (taskYieldIdNum == 0)
        return;
#endif

    counter = 0;

    for (id = 0; id < OS_MAX_NUMBER_OF_TASK; id++)
    {
        task = &TaskBank[id];

        if (task->dwTimeSchedulePeriod)
        {
            if (task->dwTimeScheduleTimeRemain > timepassed)
            {
                task->dwTimeScheduleTimeRemain -= timepassed;
            }
            else
            {
#ifdef __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__
                taskYieldIdCont[counter] = id;
                counter++;
#else
                TaskYieldById(id);
#endif
            }
        }
    }

#ifdef __USING_EXCEPTION_TASK_FOR_FORCE_TASK_YIELD__
    taskYieldIdCont[counter] = 0;

    if (counter)
    {
        if (MessageDrop(EXCEPTION_MSG_ID, (BYTE *) &stTaskYieldExceptionCont, sizeof(ST_EXCEPTION_MSG)) == OS_STATUS_OK)
            dropTaskYieldMsg = TRUE;
    }
#endif

#endif
}

