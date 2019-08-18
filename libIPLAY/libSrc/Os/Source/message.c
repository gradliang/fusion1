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
* Filename      : message.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@ingroup OBJECT
///@defgroup    MESSAGE Message
///
/// A Message buffer is an object used for synchronization and communication by sending or receiving
/// a variable-length data via a ring buffer in an Message Buffer object. The term 'message' is
/// specifically used to identify this variable-lenght data. Message Buffer functions include the ability
/// to create a message buffer, to send or receive a message to or from a message buffer. Both of them
/// have wait and polling abilities.
///
/// A message buffer has an associated ring buffer used to contain the message that a task send to it
/// and the other tasks then receive it later. If the ring buffer is full, an associated send wait queue
/// is used to queue the sending task if it wants to wait until the ring buffer has enough space. In
/// the other end, there is an associated receive wait queue to queue the receiving task that wants to
/// retrieve message from the ring buffer and the ring buffer is empty.
///
/// Diffenent to the mailbox, the message buffer is designed for the transmission with smaller data size.
/// It copy all the message data to buffer and does not just deliver the address and size of the data block.
/// Each message's size will has a limitation of 256 bytes.
///
///@image html MessageState.gif
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
#include "osapi.h"


/*-----------------------------------------------------------------------------------*/
ST_RING_BUFFER *RingCreate(DWORD size)
{
    register ST_RING_BUFFER *ring;

    // adjust the size to be a multiple of 4
    if (size & 0x00000004)
        size = (size + 4) & 0xfffffffc;

    // allocate the ring buffer, if fail then return
    ring = (ST_RING_BUFFER *) ker_mem_malloc(sizeof(ST_RING_BUFFER) + size, TaskGetId());

    if (ring == NULL)
        return OS_NULL;

    ring = (ST_RING_BUFFER *) (((DWORD) ring) & ~0x20000000);   // catched
    ring->pbBase = ((BYTE *) ring) + sizeof(ST_RING_BUFFER);
    ring->dwSize = size;
    ring->dwCounter = 0;
    ring->dwInIndex = 0;
    ring->dwOutIndex = 0;

    return ring;
}



#define ByteCopy            memcpy
/*
void ByteCopy(BYTE * target, BYTE * source, DWORD size)
{
    while (size)
    {
        *target = *source;
        target++;
        source++;
        size--;
    }
}
*/



SDWORD RingBufferWrite(ST_RING_BUFFER * ring, BYTE * source, BYTE size)
{
    register DWORD index;
    register DWORD tail;

    if ((ring->dwSize - ring->dwCounter) < (size + 1))
        return OS_STATUS_NO_MEMORY;

    ring->dwCounter += size+1;
    index = ring->dwInIndex;

    // fill the message size first
    ring->pbBase[index] = size;
    index++;

    if (index >= ring->dwSize)
        index = 0;

    tail = ring->dwSize - index;

    if (tail < size)
    {
        ByteCopy(&ring->pbBase[index], source, tail);
        ByteCopy(&ring->pbBase[0], source + tail, size - tail);
        index = size - tail;
    }
    else
    {
        ByteCopy(&ring->pbBase[index], source, size);
        index += size;

        if (index >= ring->dwSize)
            index = 0;
    }

    ring->dwInIndex = index;
    return OS_STATUS_OK;
}



SDWORD RingBufferRead(ST_RING_BUFFER * ring, BYTE * target)
{
    register DWORD index;
    register DWORD tail;
    register BYTE size;

    if (ring->dwCounter == 0)
        return OS_STATUS_POLLING_FAILURE;

    index = ring->dwOutIndex;
    size = ring->pbBase[index];
    ring->dwCounter -= size;
    index++;
    ring->dwCounter--;

    if (index >= ring->dwSize)
        index = 0;

    tail = ring->dwSize - index;

    if (tail < size)
    {
        ByteCopy(target, &ring->pbBase[index], tail);
        ByteCopy(target + tail, &ring->pbBase[0], size - tail);
        index = size - tail;
    }
    else
    {
        ByteCopy(target, &ring->pbBase[index], size);
        index += size;

        if (index >= ring->dwSize)
            index = 0;
    }

    ring->dwOutIndex = index;

    return size;
}



///
///@ingroup MESSAGE
///@brief   Create a Message Buffer
///
///@param   id      Object ID
///@param   attr    Attribute of this message buffer
///@param   size    The size of the ring buffer of this message buffer. This value should be
///                 a multiple of 4. If it is not, then will be modified by added 1~3 bytes.
///
///@retval  OS_STATUS_OK            The message buffer created without problem.
///@retval  OS_NO_MEMORY            Not enough memory can be allocated for the ring buffer.
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range or the ID is in-used.
///
///@remark  This service call create a message buffer with the ID number specified by 'id'.
///         The value 'attr' can be any OR combination of OS_ATTR_FIFO, OS_ATTR_PRIORITY. When
///         OS_ATTR_FIFO is specified, the send-wait queue and the receive-wait queue will be in FIFO
///         order. When OS_ATTR_PRIORITY is specified, the receive-wait queue and the send-wait queue
///         will be in task priority order.
///
SDWORD MessageCreate(BYTE id, BYTE attr, DWORD size)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    SWORD status = OS_STATUS_OK;

    IntDisable();               // disable INT and save the old status

    message = (ST_MESSAGE *) ObjectAllocate(id, OS_OBJECT_MESSAGE);

    if (message)
    {
        message->pBuffer = RingCreate(size);

        if (message->pBuffer != OS_NULL)
        {
            message->bAttribute = attr;
            link = &message->SendWait;
            link->pHead = TASK_ZERO;
            link->pRear = TASK_ZERO;
            link = &message->ReceiveWait;
            link->pHead = TASK_ZERO;
            link->pRear = TASK_ZERO;
            status = OS_STATUS_OK;
        }
        else
        {
            status = OS_STATUS_NO_MEMORY;
        }
    }
    else
    {
        status = OS_STATUS_INVALID_ID;
    }

    IntEnable();

    return status;
}



///
///@ingroup MESSAGE
///@brief   Destroy the specific Message
///
///@param   id      Message ID to be distoryed
///
///@retval  OS_STATUS_OK            The message destroyed without problem.
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range or the ID is in-used.
///
///@remark  This service call distroy the specific message. It also put all tasks waiting to receive from
///         or send to the message into ready state.
///
SDWORD MessageDestroy(BYTE id)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    register ST_TASK *task;

    IntDisable();               // disable INT and save the old status
    message = MessageGetPoint(id);

    if (message == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    // check if any task waitting receive the message, if yes, put all these task into ready state
    // and set the size to 0
    link = &message->ReceiveWait;

    while (link->pHead != TASK_ZERO)
    {
        task = TaskDequeue(link);
        task->dwObjectScratch = 0;

        if (task->dwSleepTime != 0)
            TaskCancelSleeping(task);

        TaskSchedule(task);
    }

    // check if any task waitting to send the message, if yes, put all these task into ready state
    link = &message->SendWait;

    while (link->pHead != TASK_ZERO)
    {
        task = TaskDequeue(link);
        TaskSchedule(task);
    }

    ker_mem_free(message->pBuffer);
    ObjectFree(id);
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup MESSAGE
///@brief   Send a message to message buffer
///
///@param   id      ID number for the message buffer to which the message is sent
///@param   source  Start address of the message to be sent
///@param   size    Size of the message to be sent
///
///@retval  OS_STATUS_OK            The message has sent to the message buffer without problem.
///@retval  OS_STATUS_INVALID_ID    Invalid ID number.
///
///@remark  This service call send a message to the message buffer specified by 'id'. The message
///         to be sent is placed in the memory area starting from the address specified by 'source'
///         and its size in bytes is specified by 'size'.
///@remark  If already some tasks are in the message buffer's receive-wait queue, the task at the
///         head of the queue is selected to receive the message. The sent message will be copied
///         to the place specified by the waited task and the waited task will be released from
///         waiting state.
///@remark  If no task are waiting in the message buffer's receive-wait queue, this service call
///         will trying to copy the message to the tail of the ring buffer. If the available of
///         the ring buffer is not enough to hold the sent message, the invoking task will be trap
///         into the send-wait queue and move to waiting state. The invoking task will be placed
///         at the tail of the send-wait queue if the message buffer's attribute has OS_ATTR_FIFO
///         set and will be placed in the order of task's priority if the message buffer's
///         attribute has OS_ATTR_PRIORITY set.
///@remark  The invoking task is trapped in the send-wait queue, it will wait until the message buffer
///         has sufficient space to contain its message and then can be waked up and keep going.
///
SDWORD MessageSend(BYTE id, BYTE * source, BYTE size)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    register ST_TASK *task;
    register ST_RING_BUFFER *ring;

    if (ContextStatusGet() == 0)
    {
        MP_ALERT("--W-- Don't call MessageSend in ISR, change it to MessageDrop !!!");

        return MessageDrop(id, source, size);
    }

    IntDisable();                   // disable INT and save the old status
    message = MessageGetPoint(id);

    if (message == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    link = &message->ReceiveWait;

    if (link->pHead != TASK_ZERO)
    {
        task = TaskDequeue(link);

        if (task->dwSleepTime != 0)
            TaskCancelSleeping(task);

        ByteCopy((BYTE *) task->dwObjectBuffer, source, size);
        task->dwObjectScratch = size;
        TaskSchedule(task);
        ContextUpdate();
    }
    else
    {
        ring = message->pBuffer;

        if (RingBufferWrite(ring, source, size) != OS_STATUS_OK)
        {
            task = TaskDeschedule();
            task->bState = OS_STATE_WAITING;
            link = &message->SendWait;

            if (message->bAttribute & OS_ATTR_PRIORITY)
                TaskInsert(link, task);
            else
                TaskAppend(link, task);

            task->dwObjectBuffer = (DWORD) source;
            task->dwObjectScratch = (DWORD) size;

            ContextUpdate();
        }
    }

    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup MESSAGE
///@brief   Send a message to message buffer by polling
///
///@param   id      ID number for the message buffer to which the message is sent
///@param   source  Start address of the message to be sent
///@param   size    Size of the message to be sent
///
///@retval  OS_STATUS_OK            The message has sent to the message buffer without problem.
///@retval  OS_STATUS_INVALID_ID    Invalid ID number.
///@retval  OS_STATUS_NO_MEMORY     Not enough space in buffer to contain the sent message.
///
///@remark  This service call send a message to the message buffer specified by 'id'. The message
///         to be sent is placed in the memory area starting from the address specified by 'source'
///         and its size in bytes is specified by 'size'.
///@remark  If already some tasks are in the message buffer's receive-wait queue, the task at the
///         head of the queue is selected to receive the message. The sent message will be copied
///         to the place specified by the waited task and the waited task will be released from
///         waiting state.
///@remark  Different to the MessageSend, this service call will not trapped in the send-wait queue
///         if the ring buffer has no sufficient space to contain its message. It will return
///         OS_STATUS_NO_MEMORY in this case.
///
SDWORD MessageDrop(BYTE id, BYTE * source, BYTE size)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    register ST_TASK *task;
    register ST_RING_BUFFER *ring;

    IntDisable();               // disable INT and save the old status
    message = MessageGetPoint(id);

    if (message == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    link = &message->ReceiveWait;

    if (link->pHead != TASK_ZERO)
    {
        task = TaskDequeue(link);
        ByteCopy((BYTE *) task->dwObjectBuffer, source, size);
        task->dwObjectScratch = size;
        TaskSchedule(task);
        ContextUpdate();
    }
    else
    {
        ring = message->pBuffer;

        if (RingBufferWrite(ring, source, size) != OS_STATUS_OK)
        {
            IntEnable();

            return OS_STATUS_NO_MEMORY;
        }
    }

    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup MESSAGE
///@brief   Receive a message from message buffer
///
///@param   id          ID number for the message buffer to which the message is sent
///@param   target      Start address of the message to be sent
///@param   waittime    The time to wait. If zero, no time out.
///
///@retval  message-size            A positive value represent the received size in byte when succeed.
///@retval  OS_STATUS_INVALID_ID    Invalid ID number.
///
///@remark  This service call receive a message to the message buffer specified by 'id'. The message
///         to be received is placed in the memory area starting from the address specified by 'target'
///         and its size will be return by the return value.
///@remark  If already some messages are in the message buffer's ring buffer, this service call will
///         directly copy the message from the ring buffer to the target address by the read size. If
///         there is task wait in the send-wait queue of this message buffer, this service call will
///         check the whether the released space is sufficient to the requirement of the first task in
///         the send-wait queue or not. If it is, then the first task of the send-wait queue will be
///         wake up and its message will be copied to the ring buffer of this message buffer.
///@remark  If no message in the message buffer's ring buffer, this invoking task will be placed
///         at the tail of the send-wait queue if the message buffer's attribute has OS_ATTR_FIFO
///         set and will be placed in the order of task's priority if the message buffer's
///         attribute has OS_ATTR_PRIORITY set.
///@remark  The invoking task is trapped in the wait-wait queue, it will wait until some task attemp to
///         send a message to this message buffer and then wake it up to keep going.
///
SDWORD MessageReceiveWithTO(BYTE id, BYTE * target, DWORD waittime)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    register ST_TASK *task;
    register ST_RING_BUFFER *ring;
    SDWORD size;

    IntDisable();               // disable INT and save the old status
    message = MessageGetPoint(id);

    if (message == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    ring = message->pBuffer;
    size = RingBufferRead(ring, target);

    if (size < 0)
    {
        task = TaskDeschedule();
        task->bState = OS_STATE_WAITING;
        link = &message->ReceiveWait;

        if (message->bAttribute & OS_ATTR_PRIORITY)
            TaskInsert(link, task);
        else
            TaskAppend(link, task);

        task->dwObjectBuffer = (DWORD) target;
        TaskSetSleeping(task, waittime);
        ContextUpdate();

        if (task->bState == OS_STATE_WAKEUP)
        {
            size = OS_STATUS_TIMEOUT;
            task->bState = OS_STATE_READY;
        }
        else
            size = (SDWORD) task->dwObjectScratch;
    }
    else
    {
        link = &message->SendWait;

        if (link->pHead != TASK_ZERO)
        {
            task = TaskDequeue(link);

            if (RingBufferWrite(ring, (BYTE *) task->dwObjectBuffer, (BYTE) task->dwObjectScratch) != OS_STATUS_OK)
            {
                TaskPreempt(link, task);
            }
            else
            {
                TaskSchedule(task);
                ContextUpdate();
            }
        }
    }

    IntEnable();

    return size;
}



SDWORD MessageReceive(BYTE id, BYTE * target)
{
    return MessageReceiveWithTO(id, target, 0);
}


///
///@ingroup MESSAGE
///@brief   Receive a message from message buffer by polling
///
///@param   id      ID number for the message buffer to which the message is sent
///@param   target  Start address of the message to be sent
///
///@retval  message-size                A positive value represent the received size in byte when succeed.
///@retval  OS_STATUS_INVALID_ID        Invalid ID number.
///@retval  OS_STATUS_POLLING_FAILURE   No message available in the message buffer.
///
///@remark  This service call receive a message to the message buffer specified by 'id'. The message
///         to be received is placed in the memory area starting from the address specified by 'target'
///         and its size will be return by the return value.
///@remark  If already some messages are in the message buffer's ring buffer, this service call will
///         directly copy the message from the ring buffer to the target address by the read size. If
///         there is task wait in the send-wait queue of this message buffer, this service call will
///         check the whether the released space is sufficient to the requirement of the first task in
///         the send-wait queue or not. If it is, then the first task of the send-wait queue will be
///         wake up and its message will be copied to the ring buffer of this message buffer.
///@remark  Different to the MessageReceive, this service call will not trapped in the receive-wait queue
///         if the ring buffer has no message available. It will return OS_STATUS_POLLING_FAILURE in this
///         case.
///
SDWORD MessageGrab(BYTE id, BYTE * target)
{
    register ST_MESSAGE *message;
    register ST_TASK_LIST *link;
    register ST_TASK *task;
    register ST_RING_BUFFER *ring;
    SDWORD size;

    IntDisable();               // disable INT and save the old status
    message = MessageGetPoint(id);

    if (message == OS_NULL)
    {
        IntEnable();

        return OS_STATUS_INVALID_ID;
    }

    ring = message->pBuffer;
    size = RingBufferRead(ring, target);

    if (size < 0)
    {
        IntEnable();

        return OS_STATUS_POLLING_FAILURE;
    }
    else
    {
        link = &message->SendWait;

        if (link->pHead != TASK_ZERO)
        {
            task = TaskDequeue(link);

            if (RingBufferWrite(ring, (BYTE *) task->dwObjectBuffer, (BYTE) task->dwObjectScratch) != OS_STATUS_OK)
            {
                TaskPreempt(link, task);
            }
            else
            {
                TaskSchedule(task);
                ContextUpdate();
            }
        }
    }

    IntEnable();

    return size;
}


////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(MessageReceive);
MPX_KMODAPI_SET(MessageSend);
MPX_KMODAPI_SET(MessageCreate);
#endif

