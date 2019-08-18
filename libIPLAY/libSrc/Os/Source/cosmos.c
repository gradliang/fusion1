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
* Filename      : cosmos.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

///
///@defgroup    COSMOS      Cosmos
///

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "mpTrace.h"

#include "osapi.h"
#include "osinner.h"
#include "taskid.h"

#define CONTEXT_INSIDE          1
#define CONTEXT_OUTSIDE         0
#define CPU_SLEEP()             __asm(" .word 0x42000038 ")

//UN_OS_OBJECT *ObjectTable;      // point to the start of the allocated object buffer

// idle function relative
static DWORD serialNum;
static SBYTE (*IdleFuncPtr[OS_MAX_NUMBER_OF_IDLE_FUNC]) (void*);
static void *IdleFuncArg[OS_MAX_NUMBER_OF_IDLE_FUNC];
static DWORD IdleFuncSerialNum[OS_MAX_NUMBER_OF_IDLE_FUNC];
static DWORD idleFuncCount;
static BYTE currExecIdleFuncId;

static void exceptionTask(void);

static void idleFuncInit(void)
{
    DWORD i;

    memset(IdleFuncPtr, 0, sizeof(IdleFuncPtr));
    memset(IdleFuncArg, 0, sizeof(IdleFuncArg));

    for (i = 0; i < OS_MAX_NUMBER_OF_IDLE_FUNC; i++)
        IdleFuncSerialNum[i] = OS_STATUS_INVALID_ID;

    idleFuncCount = 0;
    serialNum = 0;
}



///@ingroup COSMOS
///@brief   Register a procedure to idel task. (backround)
///
///@param   *Func               Callback function pointer
///@param   *arg                Callback function's parameter
///
///@retval  ID                      >= 0
///@retval  OS_STATUS_INVALID_ID    Procedure array is full.
///
///@remark  The procedure should can have argument with type of (void *). The caller
///         can use this point to specify a argument buffer to save all the arguments
///         that the procedure need.
///         In just created service the handler argument field will contain
///         a null point and will not transfer any argument to the handler.
///@remark  The procedure will be release by himeself when the return value of procedure is OS_IDLE_FUNC_TERMINATE.
///@remark  Note: Don't waiting any OS's object in the procedure, like EventWait, MessageReceive, MailboxReceive, ....
///
SDWORD IdleFuncRegister(SBYTE (*Func)(void*), void* arg)
{
    SDWORD id;

    IntDisable();

    for (id = 0; id < OS_MAX_NUMBER_OF_IDLE_FUNC; id++)
    {
        if (!IdleFuncPtr[id])
        {
            IdleFuncPtr[id] = Func;
            IdleFuncArg[id] = arg;
            IdleFuncSerialNum[id] = serialNum;
            idleFuncCount++;
            //DPrintf("I:%d", serialNum);

            break;
        }
    }

    if (OS_MAX_NUMBER_OF_IDLE_FUNC == id)
        id = OS_STATUS_INVALID_ID;
    else
        id = serialNum++;

    if (serialNum == OS_STATUS_INVALID_ID)
        serialNum++;

    IntEnable();

    return id;
}



///@ingroup COSMOS
///@brief   Release a procedure from idel task.
///
///@param   serialNum               The ID of idle function
///
///@retval  OS_STATUS_OK            No error
///@retval  OS_STATUS_INVALID_ID    Invalid ID
///
///@remark
///
SDWORD IdleFuncRelease(DWORD serialNum)
{
    DWORD i;

    IntDisable();

    if (serialNum == OS_STATUS_INVALID_ID)
    {
        mpDebugPrint("BG func not exsit");
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    if (idleFuncCount)
    {
        for (i = 0; i < OS_MAX_NUMBER_OF_IDLE_FUNC; i++)
        {
            if (IdleFuncSerialNum[i] == serialNum)
            {
                IdleFuncPtr[i] = 0;
                IdleFuncSerialNum[i] = OS_STATUS_INVALID_ID;

                if (idleFuncCount)
                    idleFuncCount--;

                //DPrintf("R:%d", serialNum);
            }
        }
    }

    IntEnable();

    return OS_STATUS_OK;
}



static void execIdleFunc(void)
{
    DWORD serNum;

    while (idleFuncCount)
    {
        SDWORD i;

        for (i = 0; i < OS_MAX_NUMBER_OF_IDLE_FUNC; i++)
        {
            if (IdleFuncPtr[i])
            {
                serNum = IdleFuncSerialNum[i];
                currExecIdleFuncId = i;

                if (OS_IDLE_FUNC_TERMINATE == (*IdleFuncPtr[i])(IdleFuncArg[i]))
                    IdleFuncRelease(serNum);
            }
        }
    }
}



static void ghostTask()
{
    idleFuncInit();
    TaskChangePriority(GHOST_PRIORITY);
    TaskYield();
    IntEnable();

    while(1)
    {
        execIdleFunc();
/*
        if (!idleFuncCount)   // no Idle function exist
        {
            //mpDebugPrint("CPU will sleep !!!");
            IntEnable();
            CPU_SLEEP();        // Lexra 4180 specific SLEEP instruction
        }
*/
    }
}



///
///@ingroup COSMOS
///@brief   Initial all the data structres and operation variales for operating system
///
///@param   helpSize    The size of the heap used for all the tasks and objects
///
///@remark  OsInit will allocate the exact buffer to contain the tasks and objects (including Semaphore,
///         EventFlag, Mailbox, MessageBuffer, etc.) and reserve a heap for the stack buffer of each
///         task. The caller should preciously estimate the exact memory size in order to use the buffer
///         efficiently.
///
void OsInit(DWORD * heapAddr, DWORD heapSize)
{
    SystemIntInit();
    TaskInit();
    ObjectInit();
    MailboxInit();
    SystemTimerInit();

    // create the default Ghost Task, it's ID is fixed to 1
    if (TaskCreate(GHOST_TASK, ghostTask, HIGHEST_TASK_PRIORITY, 0x2000) == OS_STATUS_OK)
        TaskStartup(GHOST_TASK);
    else
    {
        MP_ALERT("--E-- Ghost Task create fail !!!\n\r");
        __asm("break 100");
    }

    // Exception Task
    if (TaskCreate(EXCEPTION_TASK, exceptionTask, OS_MAX_PRIORITY - 1, 0x1000) == OS_STATUS_OK)
        TaskStartup(EXCEPTION_TASK);
    else
    {
        MP_ALERT("--E-- Exception Task create fail !!!\n\r");
        __asm("break 100");
    }

    TaskTimeSchedulingInit();
}



/*
DWORD SysCallHandler(DWORD sp, DWORD serviceNo)
{
    OsExchangeContext(sp);
}
*/



#define START_EXCEPTION_TAG_NUM                 0x10
static ST_EXCEPTION_INFO msgContentInfo[OS_MAX_NUMBER_OF_EXCEPTION_TAG];

///@ingroup COSMOS
///@brief   Check exception ID's validity
///
///@param   tagId                   The ID of procedure will be release
///
///@retval  OS_STATUS_OK            ID is valid.
///@retval  OS_STATUS_INVALID_ID    ID is not valid
///
///@remark
///
SDWORD ExceptionTagCheck(BYTE tagId)
{
    BYTE index;

    if ( (tagId < START_EXCEPTION_TAG_NUM) || (tagId >= (START_EXCEPTION_TAG_NUM + OS_MAX_NUMBER_OF_EXCEPTION_TAG)) )
        return OS_STATUS_INVALID_ID;

    tagId -= START_EXCEPTION_TAG_NUM;

    if (msgContentInfo[tagId].inUsed == 0)
        return OS_STATUS_INVALID_ID;

    return OS_STATUS_OK;
}



///@ingroup COSMOS
///@brief   Register a procedure to exception task. (heightest priority task)
///
///@param   *callbackPtr        Callback function pointer
///
///@retval  0                   Procedure array is full.
///@retval  ID                  > 0, return exception tag
///
///@remark  Note: If possible, don't waiting any OS's object in the procedure, like EventWait, MessageReceive, MailboxReceive, ....
///
SDWORD ExceptionTagReister(void *callbackPtr)
{
    BYTE index;

    IntDisable();

    for (index = 0; index < OS_MAX_NUMBER_OF_EXCEPTION_TAG; index++)
    {
        if (msgContentInfo[index].inUsed == 0)
        {
            msgContentInfo[index].inUsed = 1;
            msgContentInfo[index].msgPtr = callbackPtr;

            IntEnable();

            return (SDWORD) (index + START_EXCEPTION_TAG_NUM);
        }
    }

    MP_ALERT("--E-- %s - Tag was full !!!, Increase the constane of OS_MAX_NUMBER_OF_EXCEPTION_TAG !!!", __FUNCTION__);
    IntEnable();

    return 0;
}



///@ingroup COSMOS
///@brief   Release a procedure from exception task.
///
///@param   tagId                   The ID of procedure will be release
///
///@retval  OS_STATUS_OK            No error
///@retval  OS_STATUS_INVALID_ID    Invalid ID
///
///@remark
///
SDWORD ExceptionTagRelease(BYTE tagId)
{
    tagId -= START_EXCEPTION_TAG_NUM;

    if (tagId >= OS_MAX_NUMBER_OF_EXCEPTION_TAG)
    {
        MP_ALERT("--E-- %s - Wrong Tag-%d", __FUNCTION__, tagId + START_EXCEPTION_TAG_NUM);

        return OS_STATUS_INVALID_ID;
    }
    else if (msgContentInfo[tagId].inUsed == 0)
    {
        MP_ALERT("--E-- %s - Tag-%d is not in used", __FUNCTION__, tagId + START_EXCEPTION_TAG_NUM);

        return OS_STATUS_INVALID_ID;
    }

    IntDisable();
    msgContentInfo[tagId].inUsed = 0;
    msgContentInfo[tagId].msgPtr = 0;
    IntEnable();

    return OS_STATUS_OK;
}



static void exceptionTask(void)
{
    ST_EXCEPTION_MSG stMsgContent;
    volatile void (*callbackFunPtr)(DWORD);
    SDWORD status;

    memset((U08 *) &msgContentInfo, 0, sizeof(msgContentInfo));
    MessageCreate(EXCEPTION_MSG_ID, OS_ATTR_FIFO, sizeof(ST_EXCEPTION_MSG) * 128);
    EventCreate(SYSTEM_EVENT_ID, OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR, 0);
    IntEnable();

    while(1)
    {
        MP_DEBUG("%s - Wait next msg", __FUNCTION__);
        status = MessageReceive(EXCEPTION_MSG_ID, (BYTE *) &stMsgContent);
        MP_DEBUG("Exception Tag - 0x%08X\r\nException Arg - 0x%08X", stMsgContent.dwTag, (DWORD) stMsgContent.msgAddr);

        stMsgContent.dwTag -= START_EXCEPTION_TAG_NUM;

        if (status < 0)
        {
            MP_ALERT("--E-- %s - Message recive error 0x%08X", __FUNCTION__, status);
        }
        else if (status > sizeof(stMsgContent))
        {
            MP_ALERT("--E-- %s - Message length was exceed to %dbytes", __FUNCTION__, sizeof(stMsgContent));
            __asm("break 100");
        }
        else if (stMsgContent.dwTag >= OS_MAX_NUMBER_OF_EXCEPTION_TAG)
        {
            MP_ALERT("--E-- %s - Wrong Tag-%d", __FUNCTION__, stMsgContent.dwTag + START_EXCEPTION_TAG_NUM);
        }
        else if (!msgContentInfo[stMsgContent.dwTag].inUsed)
        {
            MP_ALERT("--E-- %s - Tag%d is not in used", __FUNCTION__, stMsgContent.dwTag + START_EXCEPTION_TAG_NUM);
        }
        else if (!msgContentInfo[stMsgContent.dwTag].msgPtr)
        {
            MP_ALERT("--E-- Null Exception Pointer");
        }
        else
        {
            callbackFunPtr = msgContentInfo[stMsgContent.dwTag].msgPtr;

            if (callbackFunPtr)
                callbackFunPtr(stMsgContent.msgAddr);
            else
                MP_ALERT("--E-- Null pointer for %d", stMsgContent.dwTag);
        }
    }

    MP_ALERT("--E-- %s end", __FUNCTION__);
    __asm("break 100");
}

