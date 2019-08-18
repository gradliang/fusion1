#define LOCAL_DEBUG_ENABLE 0

#include <sys/time.h>
//#include "typedef.h"
#include "ndebug.h"
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/if_packet.h>
#include <linux/mutex.h>
#include <linux/kref.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/mempool.h>
#include <linux/net.h>
#include <net/dst.h>
#include <net/arp.h>
#include <net/sock.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <net/cfg80211.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31))
//#include <net/wireless.h>
#endif
#include <net/net_namespace.h>

#include "UtilTypeDef.h"
#include "mpTrace.h"
#include "os_mp52x.h"
//#include "net_nic.h"
#include "net_const.h"
#include "taskid.h"                             /* for DRIVER_PRIORITY */

#define MAX_NUM_TASKLETS   5

static int tasklet_events[MAX_NUM_TASKLETS];
static int tasklet_tasks[MAX_NUM_TASKLETS];
static int tasklet_task_index;
static int tasklet_event_index;

static int mpx_tasklet_task(void);
static int mpx_tasklet_event(void);

struct net_device g_net_device2;
extern struct net_device   *netdev_global;

/* ----------  kernel/softirq.c  ---------- */

void tasklet_init(struct tasklet_struct *t,
		  void (*func)(unsigned long), unsigned long data)
{
    int ret;
    ret = mpx_tasklet_task();
    MP_ASSERT(ret > 0);
    MP_DEBUG("%s: task %d", __func__, ret);
	t->task_id = ret;
    ret = mpx_tasklet_event();
    MP_ASSERT(ret > 0);
    MP_DEBUG("tasklet_init evt %d", ret);
	t->event_id = ret;
	t->next = NULL;
	t->state = 0;
	atomic_set(&t->count, 0);
	t->func = func;
	t->data = data;
	t->enabled = false;
}

void tasklet_kill(struct tasklet_struct *t)
{
    EventSet(t->event_id, 1);
}

typedef void (*mp_tasklet_t)(unsigned long);

static int mpx_tasklet_task(void)
{
    int ret;

    ret = tasklet_tasks[tasklet_task_index++];
    MP_ASSERT(tasklet_task_index <= MAX_NUM_TASKLETS);
    return ret;
}

static int mpx_tasklet_event(void)
{
    int ret;

    ret = tasklet_events[tasklet_event_index++];
    MP_ASSERT(tasklet_event_index <= MAX_NUM_TASKLETS);
    return ret;
}

static void mp_tasklet_thread(void)
{
    mp_tasklet_t f;
	unsigned long dwEvent;
    struct tasklet_struct *t; 

    TaskStartupParamGet(TaskGetId(), (DWORD *)&t, NULL, NULL, NULL);

    MP_ASSERT(t);
    f = t->func;
    MP_ASSERT(f);

//    __asm("break 100");
    while (1)
    {
        MP_DEBUG("%s: enter func t=%p,t->func=%p", __func__, t, t->func);
        EventWait(t->event_id,0xffffffff,OS_EVENT_OR, &dwEvent); /* TODO */
        f(t->data);
        MP_DEBUG("%s: exit func", __func__);
//        TaskSleep(0);
        TaskYield();
    }
}

int mpx_TaskletInit(int num)
{
    short i;
    int ret;

    MP_ASSERT(num <= MAX_NUM_TASKLETS);

    if (num > MAX_NUM_TASKLETS)
	{
		BREAK_POINT();
        return -1;
	}

//    __asm("break 100");
    tasklet_task_index = 0;
    tasklet_event_index= 0;

    if (tasklet_tasks[0])
        return 0;

    for (i=0; i<num; i++)
    {
        ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
        MP_ASSERT(ret > 0);
        tasklet_events[i] = ret;
#if Make_USB == REALTEK_RTL8188CU
        ret = mpx_TaskCreate(mp_tasklet_thread, WIFI_PRIORITY-1, 0x2000);
#else
        ret = mpx_TaskCreate(mp_tasklet_thread, WIFI_PRIORITY+1, 0x4000);
#endif
        MP_ASSERT(ret > 0);
        tasklet_tasks[i] = ret;
    }

    return 0;
}

