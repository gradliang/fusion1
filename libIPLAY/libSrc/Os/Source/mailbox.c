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
* Filename      : mailbox.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@ingroup OBJECT
///@defgroup    BOX Mailbox
///
/// A mailbox is an object used for synchronization and communication by sending or receiving
/// a message placed in a shared memory. The term 'mail' is specifically used to identify this
/// special message delivered by mailbox. Mailbox functions include the ability to create a
/// mailbox, to send or receive a message to or from a mailbox. Both of them have wait and
/// polling abilities.
///
/// A mailbox has an associated mail queue used to store sent mail and an associated wait queue
/// for receiving mails. A task sending a mail places the mail to be send in the mail queue.
/// A task receiving a mail from the mailbox removes the first mail from the mail queue. If
/// there is no mail in the mail queue, the task will be in the receiving waiting state until
/// a mail is send to the mailbox. The task waiting to receive a mail from the mailbox is placed
/// in the mailbox's wait queue.
///
/// With the mailbox functions, only the start address of the data placed in the shared memory
/// is actually passed between tasks. The data itself is not copied. Note that totally 32 mails
/// can be used between all the created mailbox in this implement.
///
/// During the mail delivery procedure, there are 5 states to indicate the status of a mail.
/// Before the first time a mail be used, it is in the OS_MAIL_EMPTY state. Once a task want
/// to deliver a memory buffer and select this mail, it will move the OS_MAIL_PACKED state. At
/// same time a memory buffer's start address and size has been recorded in this mail. This
/// mail right away be placed at the tail of the mail queue of the mailbox the invoking task
/// specified. When a task wants pick a mail from the same mailbox and if this mail is the
/// first mail in mail queue, this mail will leave the mail queue and move to the OS_MAIL_DELIVERED
/// state. If the sending task is eager to retrieve the memory buffer linked with this mail at
/// this state, it may use the MailTrack to track this mail and finally trap into the
/// OS_STATE_WAITING state. The MailTrack service call also move this mail to OS_MAIL_WAITED
/// state. If the receiving task no longer need the memory buffer again, it should invoking
/// the service call MailRelease to release the control of the memory buffer. If this mail
/// is in the OS_MAIL_WAITED state, then it will move to the OS_MAIL_STATE directly. If this
/// mail is in the OS_MAIL_DELIVERED, then it will move the OS_MAIL_RELEASED state and will
/// be moved to the OS_MAIL_EMPTY state after the sending task invoking the service call
/// MailTrack.
///
///    <a> <img src= mailbox.jpg> </a>
///
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

static ST_MAILTAG TagBin[OS_MAX_NUMBER_OF_MAILTAG];



// insert a task into a task list in the order of task priority
// it will following the last task that has the same priority
void TagInsert(ST_MAILBOX * mailbox, ST_MAILTAG * newcommer)
{
    ST_MAILTAG *tag, *next;
    BYTE priority;

    priority = newcommer->bPriority;
    tag = mailbox->pHead;

    // if the imcomming tag's priority greater than first tag, preempty it directly
    if (tag->bPriority < priority)
    {
        mailbox->pHead = newcommer;
        newcommer->pNext = tag;
    }

    // if the imcomming tag's priotity less than the first tag, find out the last
    // tag of the same priority and then insert behind it.
    else
    {
        next = tag->pNext;

        while (next->bPriority >= priority)
        {
            tag = next;
            next = tag->pNext;
        }

        tag->pNext = newcommer;
        newcommer->pNext = next;
    }
}



// extract a tag from the head of a tag list
BYTE TagDequeue(ST_MAILBOX * box)
{
    ST_MAILTAG *tag;

    tag = box->pHead;
    box->pHead = (ST_MAILTAG *) tag->pNext;

    return tag->bId;
}



void MailboxInit()
{
    WORD index;
    ST_MAILTAG *tag;

    tag = &TagBin[0];
    tag->pNext = tag;           // TagBin[0] is used to be the terminator, not for normal usage
    index = 0;

    while (index < OS_MAX_NUMBER_OF_MAILTAG)
    {
        tag->bPriority = 0;
        tag->bState = OS_MAIL_EMPTY;
        tag->bId = index;
        tag->pNext = &TagBin[0];
        tag->pSender = TASK_ZERO;
        tag++;
        index++;
    }
}



///
///@ingroup BOX
///@brief   Polling the state of a mail
///
///@param   mail_id     The ID number of the tag of the mail
///
///@return  The status of the spicified mail. If the status is OS_MAIL_RELEASED, it
///         will be change to OS_MAIL_EMPTY and free this tag automatically.
///
///@remark  The mail sender can use this service call to check the mail's status.
///         It is useful for determining when the shared memory associated with a mail
///         will be released. The valid values will be OS_MAIL_EMPTY, OS_MAIL_PACKED,
///         OS_MAIL_DELIVERED, OS_MAIL_WAITED, OS_MAIL_RELEASED.
///         This does not check the mail tag ID. If a invalid ID is specified, the return
///         value will not be predicted.
///
BYTE MailPolling(BYTE mail_id)
{
    BYTE status;
    ST_MAILTAG *tag;

    IntDisable();
    tag = &TagBin[mail_id];

    if (tag->bState == OS_MAIL_RELEASED)
    {
        tag->bState = OS_MAIL_EMPTY;
        status = OS_MAIL_RELEASED;
    }
    else
        status = tag->bState;

    IntEnable();

    return status;
}



///
///@ingroup BOX
///@brief   Tracking and wait the specified mail sign back to the sender
///
///@param   mail_id     The ID number of the tag of the mail
///
///@retval  OS_STATUS_OK        The tracked mail has been sign back without problem.
///@retval  OS_STATUS_ILLEGAL   The mail is not in OS_MAIL_DELIVER state
///
///@remark  This service call will track the state of the specified mail by the tag number 'mail_id'.
///         If the mail is not in OS_MAIL_DELIVER state, the error code OS_STATUS_ILLEGAL will return.
///         If the mail is in OS_MAIL_DELIVER state, then this service call will change the mail state
///         to OS_MAIL_WAITED state and, record the invoking task to 'pSender' field of the mail and
///         force the invoking task into WAITING state. After the mail receiving task sign back the
///         mail, the invoking task will return OS_STATUS_OK when it acquire the context again.If the
///         mail is in the OS_MAIL_RELEASED state, then this service call will behavior just as
///         MailCheck does.
///@remark  This service call is designed for the purpose that the invoking task need to wait the
///         shared memory associated with a mail to be released.
///
SDWORD MailTrack(BYTE mail_id)
{
    BYTE status;
    ST_MAILTAG *tag;
    ST_TASK *task;

    IntDisable();
    tag = &TagBin[mail_id];

    if (tag->bState == OS_MAIL_DELIVERED || tag->bState == OS_MAIL_PACKED)
    {
        tag->bState = OS_MAIL_WAITED;
        task = TaskDeschedule();
        task->bState = OS_STATE_WAITING;
        tag->pSender = task;

        CONTEXT_SWITCH();
        tag->bState = OS_MAIL_EMPTY;
        status = OS_STATUS_OK;
    }
    else if (tag->bState == OS_MAIL_RELEASED)
    {
        tag->bState = OS_MAIL_EMPTY;
        status = OS_STATUS_OK;
    }
    else
        status = OS_STATUS_ILLEGAL;

    IntEnable();

    return status;
}



///
///@ingroup BOX
///@brief   Release the shared memory of a mail when a received mail is processed complete.
///
///@param   mail_id     The ID number of a mail
///
///@retval  OS_STATUS_OK            The mail has been signed back without problem.
///@retval  OS_STATUS_INVALID_ID    The mailtag's ID is invalid.
///
///@remark  When the shared memory block of a mail has been processed by the receiving task,
///         this service call must be used to notify the sending task that the shared
///         memory accociated with this mail can be recycled. If the receiving task doesn't
///         release that memory block by calling this service call, then that memory block
///         will be locked forever and no more be used again.
///@remark  If the sending task has been waited the shared memory block to be released by
///         calling the service call MainTrack, this service call will also release the
///         sending task from OS_STATE_WAITING to OS_STATE_READY and return from MainTrack.
///
SDWORD MailRelease(BYTE mail_id)
{
    ST_MAILTAG *tag;

    if (mail_id >= OS_MAX_NUMBER_OF_MAILTAG)
    return OS_STATUS_INVALID_ID;

    IntDisable();               // disable INT and save the old status
    tag = &TagBin[mail_id];

    if (tag->bState == OS_MAIL_WAITED)
    {
        TaskSchedule(tag->pSender);
        tag->bState = OS_MAIL_RELEASED;
        ContextUpdate();
    }
    else
    {
        tag->bState = OS_MAIL_RELEASED;
    }

    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup BOX
///@brief   Get the the start address of the shared memory buffer from a received mail
///
///@param   mail_id     The ID number of a mail
///@param   start       Byte pointer point to the pointer of the start address
///
///@retval  OS_STATUS_OK            The start address has been saved to 'start' successfully.
///@retval  OS_STATUS_INVALID_ID    Invalid mail tag ID.
///@retval  OS_STATUS_WRONG_STATE   The specifed mail in not in correct state.
///
SDWORD MailGetBufferStart(BYTE mail_id, volatile DWORD * start)
{
    if (mail_id >= OS_MAX_NUMBER_OF_MAILTAG)
        return OS_STATUS_INVALID_ID;

    if (TagBin[mail_id].bState == OS_MAIL_DELIVERED || TagBin[mail_id].bState == OS_MAIL_WAITED)
    {
        *start = TagBin[mail_id].dwStart;

        return OS_STATUS_OK;
    }
    else
        return OS_STATUS_WRONG_STATE;
}



///
///@ingroup BOX
///@brief   Get the the size of the shared memory buffer from a received mail
///
///@param   mail_id     The ID number of a mail
///@param   size        Size in byte of the shared memory buffer
///
///@retval  OS_STATUS_OK            The size has been saved to 'size' successfully.
///@retval  OS_STATUS_INVALID_ID    Invalid mail tag ID.
///@retval  OS_STATUS_WRONG_STATE   The specifed mail in not in correct state.
///
SDWORD MailGetBufferSize(BYTE mail_id, DWORD * size)
{
    if (mail_id >= OS_MAX_NUMBER_OF_MAILTAG)
        return OS_STATUS_INVALID_ID;

    if (TagBin[mail_id].bState == OS_MAIL_DELIVERED || TagBin[mail_id].bState == OS_MAIL_WAITED)
    {
        *size = TagBin[mail_id].dwSize;

        return OS_STATUS_OK;
    }
    else
        return OS_STATUS_WRONG_STATE;
}



///
///@ingroup BOX
///@brief   Create a mailbox object
///
///@param   box_id  object ID
///@param   attr    attribute of this eventflag
///
///@retval  OS_STATUS_OK            Eventflag create successfully
///@retval  OS_STATUS_INVALID_ID    Invalid ID, mean the ID number out of range
///
///@remark  This service call create an mailbox with the ID number specified by 'box_id'.
///         The value 'attr' can be any OR combination of OS_ATTR_FIFO, OS_ATTR_PRIORITY. When
///         OS_ATTR_FIFO is specified, the receiving wait queue will be in FIFO order. When
///         OS_ATTR_PRIORITY is specified, the receiving wait queue will be in task priority order.
///         When OS_ATTR_WAIT_SINGLE is specified, only a single task can be put into wait queue.
///         When OS_ATTR_WAIT_MULTIPLE is specified, multiple tasks can be moved to the wait queue.
///         And, for the last attribute, when OS_ATTR_EVENT_CLEAR is specified, all the entire bit
///         pattern are cleared once a task's release condition is met in the wait queue.
///
SDWORD MailboxCreate(BYTE box_id, BYTE attr)
{
    ST_MAILBOX *box;
    ST_TASK_LIST *link;
    SDWORD status;

    IntDisable();               // disable INT and save the old status

    box = (ST_MAILBOX *) ObjectAllocate(box_id, OS_OBJECT_MAILBOX);

    if (box)
    {
        box->bAttribute = attr;
        box->pHead = &TagBin[0];
        link = &box->Wait;
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
///@ingroup BOX
///@brief   Destroy mailbox
///
///@param   box_id  The ID number of the mailbox to be destroyed
///
///@retval  OS_STATUS_OK                The mail destroyed successed.
///@retval  OS_STATUS_INVALID_ID        The ID number out of range or the object indecated by
///                                     this ID is not a mailbox.
///
///@remark  This service call distroy the specific mail box. It also put all tasks waiting the mail
///         from the mailbox or tracking the mail to the mailbox into ready state.
///
SDWORD MailboxDestroy(BYTE box_id)
{
    ST_MAILBOX *mailbox;
    ST_TASK_LIST *link;
    ST_TASK *task;
    ST_MAILTAG *tag;
    BYTE index;

    mailbox = MailboxGetPoint(box_id);

    if (mailbox == OS_NULL)
        return OS_STATUS_INVALID_ID;

    IntDisable();               // disable INT and save the old status

    // check if any task waiting the mail box, if yes, put all these task into ready state
    link = &mailbox->Wait;
    task = link->pHead;

    while(task != TASK_ZERO)
    {
        TaskDequeue(link);
        task->dwObjectBuffer = OS_MAX_NUMBER_OF_MAILTAG;
        TaskSchedule(task);
        task = link->pHead;
    }

    // check if any tag belong to the mail box, and has task tracking it
    // if yes, put the tracking task into ready state
    index = TagDequeue(mailbox);

    while(index != 0)
    {
        tag = &TagBin[index];

        if (tag->bState == OS_MAIL_WAITED)
            TaskSchedule(tag->pSender);

        tag->bState = OS_MAIL_EMPTY;
        index = TagDequeue(mailbox);
    }

    ObjectFree(box_id);
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup BOX
///@brief   Send a mail to specified mailbox
///
///@param   box_id  Mailbox ID number
///@param   start   Start address of the shared memory of this mail
///@param   size    Memory size of the shared memory of this mail
///@param   mail_id The ID of the of this mail will be return to the place that mail_id point to.
///
///@retval  OS_STATUS_OK            The mail has been sent successfully and the 'mail_id' has pointed to
///                                 valid mailtag.
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a mailbox.
///@retval  OS_STATUS_INVALID_TAG   No tag available for new mail.
///
///@remark  This service call send a mail to the mailbox specified by 'id'. The start address
///         and size of the shared memory of the mail have recorded into a tag and only this tag
///         is actually put into the mailbox. If this service call send this mail to the mailbox
///         successfully, the number of the tag will be returnned. The invoking task then can use
///         this tag number to track this mail to determine when the shared memory can be released.
///
SDWORD MailboxSend(BYTE box_id, BYTE * start, DWORD size, BYTE * mail_id)
{
    BYTE index;
    ST_MAILTAG *tag;
    ST_MAILBOX *mailbox;
    ST_TASK_LIST *link;
    ST_TASK *task;

    IntDisable();               // disable INT and save the old status
    mailbox = MailboxGetPoint(box_id);

    if (mailbox == OS_NULL)
    {
        IntEnable();
        return OS_STATUS_INVALID_ID;
    }

    // try to get a empty tag
    tag = &TagBin[1];           // reserve 0 tag to be the terminator
    index = 1;

    while (index < OS_MAX_NUMBER_OF_MAILTAG)
    {
        if (tag->bState == OS_MAIL_EMPTY)
            break;

        tag++;
        index++;
    }

    // check if any tag available
    if (index == OS_MAX_NUMBER_OF_MAILTAG)
    {
        IntEnable();

        return OS_STATUS_INVALID_TAG;
    }

    // setup the tag
    tag->bPriority = TaskGetPriority();	// invoking task's priority is mail's priority
    tag->bState = OS_MAIL_PACKED;
    tag->dwStart = (DWORD) start;
    tag->dwSize = size;

    link = &mailbox->Wait;
    task = link->pHead;

    if (task == TASK_ZERO)      // if no task waiting for mail, put mail in mail queue
        TagInsert(mailbox, tag);
    else                        // else, send the mail to the waiting task
    {
        tag->bState = OS_MAIL_DELIVERED;
        task->dwObjectBuffer = index;
        TaskDequeue(link);
        TaskSchedule(task);
    }

    *mail_id = index;
    ContextUpdate();
    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup BOX
///@brief   Receive mail from mailbox
///
///@param   box_id  The ID number of the receiving mailbox
///@param   mail_id The point used to save the return mail ID
///
///@retval  OS_STATUS_OK            The mail tag has returned without problem.
///@retval  OS_STATUS_INVALID_ID    The ID number out of range or the object indecated by
///                                 this ID is not a mailbox.
///
///@remark  This service call receive a mail from the mailbox specified by 'box_id'. If the mailbox's
///         send-wait queue already has mails, this service call remove the first mail and return the
///         mail ID to the place that 'mail_id' pointed. If there are no mail in the sned-wait queue,
///         the invoking task will be placed in the receive-wait queue and moved to the OS_STATE_WAITING
///         state. When the mailbox's attribute has the OS_ATTR_FIFO set, the invoking task will be in the
///         receive-wait queue in FIFO order. When the mailbox's attribute has the OS_ATTR_PRIORITY set,
///         the invoking task will be in the receive-wait queue with the order of the task's priority.
///         If the receive-wait queue contains tasks with the the same priority as the invoking task,
///         the invoking task will be placed after those tasks.
///@remark  When the invoking task return with OS_STATUS_OK, the start address and size of the shared
///         memory of the received mail can be retrieved by the functions MailGetBufferStart and
///         MailGetBufferSize respectively.
///@remark  Note that the sender may be continuously waiting the shared memory buffer to be released
///         in order to re-use it. So the receiving task should use the service call to release the
///         sending task from waited.
///
SDWORD MailboxReceive(BYTE box_id, BYTE * mail_id)
{
    ST_MAILBOX *mailbox;
    ST_TASK *me;
    SDWORD status;

    mailbox = MailboxGetPoint(box_id);

    if (mailbox == OS_NULL)
        return OS_STATUS_INVALID_ID;

    IntDisable();               // disable INT and save the old status
    *mail_id = TagDequeue(mailbox);

    if (*mail_id == 0)
    {
        me = TaskDeschedule();
        me->bState = OS_STATE_WAITING;

        if (mailbox->bAttribute & OS_ATTR_PRIORITY)
            TaskInsert(&mailbox->Wait, me);
        else
            TaskAppend(&mailbox->Wait, me);

#ifdef __MONITOR_RESERVED_MEMORY_REGION
        if (mem_ReservedRegionCheck() == TRUE)
            __asm("break 100");
#endif

        CONTEXT_SWITCH();
        *mail_id = (BYTE) me->dwObjectBuffer;
    }

    if (TagBin[*mail_id].bState == OS_MAIL_PACKED)
        TagBin[*mail_id].bState = OS_MAIL_DELIVERED;

    IntEnable();

    return OS_STATUS_OK;
}



///
///@ingroup BOX
///@brief   Polling mail from mailbox
///
///@param   box_id  The ID number of the receiving mailbox
///@param   mail_id The point used to save the return mail ID
///
///@retval  OS_STATUS_OK                The mail tag has returned without problem.
///@retval  OS_STATUS_INVALID_ID        The ID number out of range or the object indecated by
///                                     this ID is not a mailbox.
///@retval  OS_STATUS_POLLING_FAILURE   The mailbox is empty.
///
///@remark  This service call polling a mail from the mailbox specified by 'box_id'. If the mailbox's
///         send-wait queue already has mails, this service call remove the first mail and return the
///         mail ID to the place that 'mail_id' pointed and return OS_STATUS_OK. If there is no mail in
///         the sned-wait queue, this service call will return OS_STATUS_POLLING_FAILURE and the invoking
///         task should ignore the value of 'mail_id.
///
SDWORD MailboxPolling(BYTE box_id, BYTE * mail_id)
{
    ST_MAILBOX *mailbox;
    SDWORD status;

    mailbox = MailboxGetPoint(box_id);

    if (mailbox == OS_NULL)
        return OS_STATUS_INVALID_ID;

    IntDisable();               // disable INT and save the old status
    *mail_id = TagDequeue(mailbox);

    if (*mail_id)
    {
        if (TagBin[*mail_id].bState == OS_MAIL_PACKED)
            TagBin[*mail_id].bState = OS_MAIL_DELIVERED;

        status = OS_STATUS_OK;
    }
    else
        status = OS_STATUS_POLLING_FAILURE;

    IntEnable();

    return status;
}

