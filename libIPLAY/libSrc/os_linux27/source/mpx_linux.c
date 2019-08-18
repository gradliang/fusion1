
#define LOCAL_DEBUG_ENABLE 0

#include <linux/device.h>
#include <linux/usb.h>
#include <linux/workqueue.h>

#include "os_mp52x.h"
#include "mpTrace.h"

#ifdef Make_USB == REALTEK_RTL8188CU
#define MAX_NUM_SEMAPHORES	104
#else
#define MAX_NUM_SEMAPHORES   72
#endif

static int mp_semaphore_max;                    /* number of created semaphores */
static int mp_semaphores[MAX_NUM_SEMAPHORES]; /* a semaphore pool */
static int mp_semaphore_index;

#define MAX_NUM_TASKLETS   3

static int tasklet_events[MAX_NUM_TASKLETS];
static int tasklet_tasks[MAX_NUM_TASKLETS];
static int tasklet_task_index;
static int tasklet_event_index;


int mp_OsSemaphore(void)
{
    MP_ASSERT(mp_semaphore_index < mp_semaphore_max);
	MP_DEBUG("%s: mp_semaphore_index=%d", __func__, mp_semaphore_index);
    if (mp_semaphore_index >= mp_semaphore_max)
        return -1;
    return mp_semaphores[mp_semaphore_index++];
}

/*
 * Create semaphores before actual driver initialization.
 */
int mp_OsSemaphoreInit(int max)
{
    short i;
    int ret;

    mp_semaphore_index = 0;

    if (max <= mp_semaphore_max)
        return 0;

    MP_ASSERT(max <= MAX_NUM_SEMAPHORES);
    if (max > MAX_NUM_SEMAPHORES)
        return -1;

    for (i=mp_semaphore_max; i<max; i++)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        mp_semaphores[i] = ret;
    }
    mp_semaphore_max = max;
    return 0;
}

typedef void (*mp_tasklet_t)(unsigned long);

int ieee80211_tasklet_task(void)
{
    int ret;

    ret = tasklet_tasks[tasklet_task_index++];
    MP_ASSERT(tasklet_task_index <= MAX_NUM_TASKLETS);
    return ret;
}

int ieee80211_tasklet_event(void)
{
    int ret;

    ret = tasklet_events[tasklet_event_index++];
    MP_ASSERT(tasklet_event_index <= MAX_NUM_TASKLETS);
    return ret;
}

#if 0
static void mp_tasklet_thread(struct tasklet_struct *t)
{
    mp_tasklet_t f;
	unsigned long dwEvent;

    f = t->func;

//    __asm("break 100");
    while (1)
    {
        MP_DEBUG("%s: enter func", __func__);
        EventWait(t->event_id,0xffffffff,OS_EVENT_OR, &dwEvent); /* TODO */
        f(t->data);
        MP_DEBUG("%s: exit func", __func__);
//        TaskSleep(0);
    }
}
#endif

