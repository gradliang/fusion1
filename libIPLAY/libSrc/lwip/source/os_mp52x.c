/*
 * @file
 *
 * OS wrapper functions for MP52X source files.
 *
 * Copyright (c) 2007 Magic Pixel Inc.
 * All rights reserved.
 */
#define LOCAL_DEBUG_ENABLE 0

#include <stdarg.h>
#include "corelib.h"
#include "typedef.h"
#include "os.h"
#include "Taskid.h"

//#define MAX_USED_OBJ_ID TOTAL_OBJECT_NUMBER

//#define MAX_USED_TASK_ID TOTAL_TASK_NUMBER

static BYTE max_obj_id;
static BYTE max_task_id;

static BYTE max_defined_obj_id;                 /* max predefined ID */
static BYTE oid[256];                 			/* used/unused flags */

/*If different config will have different number, and release liblwip.a will have problem*/
void Set_MAX_OBJ_TASK(total_object_number,total_task_number)
{
	WORD i;
	max_defined_obj_id = 
	max_obj_id = total_object_number;
	max_task_id = total_task_number;

	for (i=0; i<=max_defined_obj_id; i++)
		oid[i] = 1;
}

S32 mpx_TaskCreate(void *entry, U08 priority, U32 stacksize)
{
    S32 ret, status;
    BYTE id;

    status = mpx_SemaphoreWait(NET_SEM_ID);

    id = max_task_id+1;
    if (id >= OS_MAX_NUMBER_OF_TASK)
    {
        status = mpx_SemaphoreRelease(NET_SEM_ID);
        MP_ALERT("mpx_TaskCreate returns err (%d)", max_task_id);
        BREAK_POINT();
        return OS_STATUS_INVALID_ID;
    }

    ret = (S32) TaskCreate(id,entry,priority,stacksize);
    if (ret != OS_STATUS_OK)
    {
        MP_ASSERT(0);
    }
    else
    {
        MP_DEBUG("%s: id (%d)", __func__, id);
        ret = max_task_id = id;
    }

    status = mpx_SemaphoreRelease(NET_SEM_ID);
    return  ret;
}

S32 mpx_EventCreate(U08 attr, U32 flag)
{
    S32 ret, status;
    BYTE id;

    status = mpx_SemaphoreWait(NET_SEM_ID);

    id = max_obj_id+1;

#if 1
	/*
	 * Find an unused one
	 */
    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
		for (id=max_defined_obj_id+1; id<OS_MAX_NUMBER_OF_OBJECT; id++)
		{
			if (oid[id] == 0)
				break;
		}
	}
#endif

    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
        status = mpx_SemaphoreRelease(NET_SEM_ID);
        MP_ASSERT(0);
        return OS_STATUS_INVALID_ID;
    }

    ret = EventCreate(id,attr,flag);
    if (ret != OS_STATUS_OK)
    {
        MP_DEBUG1("EventCreate returns err max=(%d)", max_obj_id);
        MP_ASSERT(0);
    }
    else
    {
        ret = max_obj_id = id;
		oid[id] = 1;
    }

    status = mpx_SemaphoreRelease(NET_SEM_ID);

    MP_DEBUG1("mpx_EventCreate returns %d", ret);
    return  ret;
}
S32 mpx_MessageCreate(U08 attr, U32 size)
{
    S32 ret;
    BYTE id;
    SDWORD status;

    status = mpx_SemaphoreWait(NET_SEM_ID);

    id = max_obj_id+1;

#if 1
	/*
	 * Find an unused one
	 */
    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
		for (id=max_defined_obj_id+1; id<OS_MAX_NUMBER_OF_OBJECT; id++)
		{
			if (oid[id] == 0)
				break;
		}
	}
#endif

    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
        status = mpx_SemaphoreRelease(NET_SEM_ID);
        MP_ASSERT(0);
        return OS_STATUS_INVALID_ID;
    }

    ret = MessageCreate(id, attr, size);
    if (ret != OS_STATUS_OK)
    {
        MP_ALERT("mpx_MessageCreate: MessageCreate returns error %d", ret);
        MP_ASSERT(0);
    }
    else
    {
        ret = max_obj_id = id;
		oid[id] = 1;
    }

    status = mpx_SemaphoreRelease(NET_SEM_ID);

    MP_DEBUG1("mpx_MessageCreate returns %d", ret);
    return ret;
}
S32 mpx_SemaphoreCreate(U08 attr, U08 count)
{
    S32 ret;
    BYTE id;
    SDWORD status;

    status = mpx_SemaphoreWait(NET_SEM_ID);

    id = max_obj_id+1;

#if 1
	/*
	 * Find an unused one
	 */
    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
		for (id=max_defined_obj_id+1; id<OS_MAX_NUMBER_OF_OBJECT; id++)
		{
			if (oid[id] == 0)
				break;
		}
	}
#endif

    if (id >= OS_MAX_NUMBER_OF_OBJECT)
    {
        status = mpx_SemaphoreRelease(NET_SEM_ID);
        MP_ASSERT(0);
        return OS_STATUS_INVALID_ID;
    }

    ret = SemaphoreCreate(id, attr, count);
    if (ret != OS_STATUS_OK)
        MP_ASSERT(0);
    else
	{
        ret = max_obj_id = id;
		oid[id] = 1;
	}

    status = mpx_SemaphoreRelease(NET_SEM_ID);
    MP_DEBUG1("mpx_SemaphoreCreate returns %d", ret);
    return ret;
}

S32 mpx_SemaphoreDestroy(BYTE id)
{
    S32 ret;

    ret = SemaphoreDestroy(id);
    if (ret != OS_STATUS_OK)
        MP_ALERT("SemaphoreDestroy returns error(%d)", ret);
    else
		oid[id] = 0;

    return ret;
}

void DPrintf(const U08 * format, ...)
{
	va_list ap;
    char buf[256];
    int len = strlen(format);

	va_start(ap, format);
    vsprintf(buf, format, ap);
	va_end(ap);
    if (format[len-1] == '-' && format[len-2] == '\\')
    {
        len = strlen(buf);
        buf[len-2] = '\0';
        mpDebugPrint("%s", buf);
    }
    else
        mpDebugPrint("%s", buf);
}

