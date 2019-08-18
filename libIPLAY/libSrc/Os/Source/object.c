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
* Filename      : object.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

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


static UN_OS_OBJECT ObjectBank[OS_MAX_NUMBER_OF_OBJECT];


void ObjectInit()
{
    register BYTE i, *point;

    for (i = 0; i < OS_MAX_NUMBER_OF_OBJECT; i++)
    {
        point = (BYTE *) & ObjectBank[i];
        *point = OS_OBJECT_FREE;
    }

    MP_ALERT("TOTAL_OBJECT_NUMBER = %d, OS_MAX_NUMBER_OF_OBJECT = %d", TOTAL_OBJECT_NUMBER, OS_MAX_NUMBER_OF_OBJECT);

    if (TOTAL_OBJECT_NUMBER >= OS_MAX_NUMBER_OF_OBJECT)
    {
        MP_ALERT("--E-- TOTAL_OBJECT_NUMBER exceed to OS_MAX_NUMBER_OF_OBJECT, increase the value of OS_MAX_NUMBER_OF_OBJECT");

        for (i = 0; i < 255; i++)
            __asm("NOP");

        __asm("break 100");
    }
}



void *ObjectAllocate(BYTE id, BYTE type)
{
    register BYTE *point;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
    {
        MP_ALERT("--E-- Object ID-%3d was exceed to max ID number %d !!!", id, OS_MAX_NUMBER_OF_OBJECT - 1);
        __asm("break 100");

        return OS_NULL;
    }

    point = (BYTE *) &ObjectBank[id];

    if (*point != OS_OBJECT_FREE)
    {
        MP_ALERT("--E-- Object ID-%3d was used at Task%2d !!!", id, TaskGetId());

        return OS_NULL;
    }

    *point = type;

    return point;
}



void ObjectFree(BYTE id)
{
    register BYTE *point;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
        return;

    point = (BYTE *) &ObjectBank[id];
    *point = OS_OBJECT_FREE;
}



ST_SEMAPHORE *SemaphoreGetPoint(BYTE id)
{
    register ST_SEMAPHORE *semaphore;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
        return OS_NULL;

    semaphore = (ST_SEMAPHORE *) & ObjectBank[id];

    if (semaphore->bObjectType != OS_OBJECT_SEMAPHORE)
        return OS_NULL;

    return semaphore;
}



ST_EVENT *EventGetPoint(BYTE id)
{
    register ST_EVENT *event;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
        return OS_NULL;

    event = (ST_EVENT *) & ObjectBank[id];

    if (event->bObjectType != OS_OBJECT_EVENT)
        return OS_NULL;

    return event;
}



ST_MAILBOX *MailboxGetPoint(BYTE id)
{
    register ST_MAILBOX *mailbox;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
        return OS_NULL;

    mailbox = (ST_MAILBOX *) & ObjectBank[id];

    if (mailbox->bObjectType != OS_OBJECT_MAILBOX)
        return OS_NULL;

    return mailbox;
}



ST_MESSAGE *MessageGetPoint(BYTE id)
{
    register ST_MESSAGE *message;

    if (id > OS_MAX_NUMBER_OF_OBJECT - 1)
        return OS_NULL;

    message = (ST_MESSAGE *) & ObjectBank[id];

    if (message->bObjectType != OS_OBJECT_MESSAGE)
        return OS_NULL;

    return message;
}

