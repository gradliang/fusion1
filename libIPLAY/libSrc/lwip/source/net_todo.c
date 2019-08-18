
#define LOCAL_DEBUG_ENABLE 1

#include <asm/atomic.h>
//#include "osdep_mpixel_service.h"
#include "linux/kernel.h"
#include "linux/list.h"
#include "linux/spinlock.h"
#include "linux/spinlock_types.h"

#include "global612.h"
#include "mpTrace.h"
#include "net_api.h"
#include "netware.h"
//#include "os_defs.h"

#include "ui.h"
#include "taskid.h"
#include "SysConfig.h"

#ifdef NET_NAPI
#include "xpgNet.h"

static ST_NET_WORKQUEUE net_todo_list;
static BYTE net_todo_task_id;

struct net_completion_list {
	BYTE lock;
	BYTE more_entry;
	struct list_head donelist;
};

static struct net_completion_list net_completion_list;

static void run_net_todo(ST_NET_WORKQUEUE *wq);
void Net_Recv_Data_todo(ST_NET_WORK *work);
static int http_start_wait_request(ST_NET_WORK *work, int timeout, int *actual_length);

extern BYTE g_bXpgStatus;
extern BYTE g_boNeedRepaint;

/* the task */
void net_todo_thread(void)
{
	ST_NET_WORKQUEUE *wq = &net_todo_list;
	u32 dwEvent;

	for (;;) {
		EventWait(wq->more_work,0xffffffff,OS_EVENT_OR, &dwEvent);
		if (!list_empty(&wq->worklist))
            run_net_todo(wq);
	}

	return;
}

void net_insert_done(ST_NET_WORK *work);
int Xml_BUFF_isFree(void);
static void run_net_todo(ST_NET_WORKQUEUE *wq)
{
	int res;
	ST_NET_WORK *work;

	MP_DEBUG("%s --> ", __func__);
	spin_lock(&wq->lock);
	while (!list_empty(&wq->worklist)) {

        if (!Xml_BUFF_isFree())
            break;

		work = list_first_entry(&wq->worklist, ST_NET_WORK, work);
		list_del_init(&work->work);
		spin_unlock(&wq->lock);

		MP_TRACE_LINE();

		net_work_func_t f = work->todo;

		net_work_clear_pending(work);
		f(work);

        MP_DEBUG("%s: work=%p", __func__, work);
        net_insert_done(work);
		MP_TRACE_LINE();

		spin_lock(&wq->lock);
	}
	spin_unlock(&wq->lock);


}

int net_todo_init(void)
{
	int ret;
	MP_DEBUG("%s", __func__);
    if (!net_todo_list.lock)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        net_todo_list.lock = ret;
    }

    if (!net_todo_list.more_work)
    {
		ret = mpx_EventCreate(OS_ATTR_WAIT_SINGLE|OS_ATTR_EVENT_CLEAR, 0);
		MP_ASSERT(ret > 0);
		net_todo_list.more_work = ret;
    }

	INIT_LIST_HEAD(&net_todo_list.worklist);

    ret = mpx_TaskCreate(net_todo_thread, NETWORK_TASK_PRI, TASK_STACK_SIZE*2);
	MP_ASSERT(ret > 0);
	net_todo_task_id = (BYTE)ret;
    mpx_TaskStartup(ret);
	return ret;
}

static void insert_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work, int tail)
{
	/*
	 * Ensure that we get the right work->data if we see the
	 * result of list_add() below, see try_to_grab_pending().
	 */
	if (tail)
		list_add_tail(&work->work, &net_todo_list.worklist);
	else
		list_add(&work->work, &net_todo_list.worklist);
}

static void __queue_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work)
{

	spin_lock(&wq->lock);
	insert_work(wq, work, 1);
	spin_unlock(&wq->lock);
    EventSet(wq->more_work, 1);
}

int net_queue_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work)
{
	int ret = 0;

	if (!(work->flags & NET_WORK_PENDING)) {
		net_work_set_pending(work);
        work->wq_data = (DWORD)wq;
		__queue_work(wq, work);
		ret = 1;
	}
	return ret;
}

void wakeup_net_todo(void)
{
	ST_NET_WORKQUEUE *wq = &net_todo_list;
    EventSet(wq->more_work, 1);
}

int http_submit_request(ST_NET_WORK *work)
{
	return net_queue_work(&net_todo_list, work);
}

static int __cancel_work_timer(ST_NET_WORK *work,
				_timer * timer)
{
	int ret;

	do {
		ret = (timer && likely(del_timer(timer)));
#ifdef LINUX
		if (!ret)
			ret = try_to_grab_pending(work);    /* TODO */
		wait_on_work(work);
#endif
	} while (unlikely(ret < 0));

	work_clear_pending(work);
	return ret;
}

int net_cancel_work(ST_NET_REQUEST *work)
{
	ST_NET_WORKQUEUE *wq;
	int ret;

    if (!work)
        return 0;

	if (!(work->flags & NET_WORK_PENDING))
	{
		work->flags |= NET_WORK_CANCEL;         /* don't wait */
//		net_work_set_pending(work);//  XXX
		return 0;
	}

	wq = (ST_NET_WORKQUEUE *)work->wq_data;
	if (!wq)
		return -1; // XXX

	spin_lock(&wq->lock);
	if (!list_empty(&work->work)) {
		list_del_init(&work->work);
		ret = 1;
	}
	spin_unlock(&wq->lock);

	net_work_clear_pending(work);
    mpx_Free(work);
	return ret;
}

#ifdef LINUX
int net_cancel_work_sync(ST_NET_WORK *work)
{
	return __cancel_work_timer(work, NULL);
}
#endif

struct http_completion {
	unsigned int done;
    int wait;
};

void http_complete(struct http_completion *x)
{
	unsigned long flags;

//    mpDebugPrint("%s: x=%p", __func__, x);
	x->done++;
    TaskWakeup(x->wait);
}

struct http_api_context {
	struct http_completion	done;
	int			status;
};

static void http_api_blocking_completion(ST_NET_WORK *work)
{
	struct http_api_context *ctx = work->context;

	ctx->status = work->result;
	http_complete(&ctx->done);
}

static inline void http_init_completion(struct http_completion *x)
{
	x->done = 0;
    x->wait = 0;
}

/*
 * This is a synchronous call so the caller will block until the network 
 * file is completely received.
 */
int net_http_request(char *url, 
			void *data, 
            int len, 
            int *actual_length, 
            int timeout)
{
    ST_NET_WORK *work;

    work = mpx_Malloc(sizeof(*work));
    if (!work)
        return -1;
	memset(work, 0, sizeof(ST_NET_WORK));
    http_fill_request (work, url, Net_Recv_Data_todo, http_api_blocking_completion, timeout);

	return http_start_wait_request(work, timeout, actual_length);
}

unsigned long 
http_wait_for_completion_timeout(struct http_completion *x, unsigned long timeout)
{
    if (!x->done)
    {
        x->wait = TaskGetId();
#if 0
        TaskSleep(jiffies_to_msecs(timeout));
#else
        TaskSleep(10 * 1000);              /* 10 sec */
#endif
    }
    MP_DEBUG("%s: done", __func__);
	if (!x->done)
    {
        return 0;                     /* 0 indicates timeout */
    }
    x->done--;
	return 1;
}

static int http_start_wait_request(ST_NET_WORK *work, int timeout, int *actual_length)
{ 
	struct http_api_context ctx;
	unsigned long expire;
	int retval;

	http_init_completion(&ctx.done);
    ctx.status = -ENOENT;
	work->context = &ctx;
	work->actual_length = 0;
	retval = http_submit_request(work);
	if (unlikely(retval))
		goto out;

//	MP_TRACE_LINE();
	expire = timeout ? msecs_to_jiffies(timeout) : 0;
	if (!http_wait_for_completion_timeout(&ctx.done, expire)) {
		retval = (ctx.status == -ENOENT ? -ETIMEDOUT : ctx.status);
        if (retval)
            mpDebugPrint("%s: retval=%d,sts=%d\n", __func__, retval, ctx.status);

		MP_ALERT("%s timed out on URL=%s\n",
			__func__, expire);
	} else
	{
		retval = ctx.status;
		MP_ASSERT(retval == 0);
	}
out:
	if (actual_length)
		*actual_length = work->actual_length;

	mpx_Free(work);
	return retval;
}

void NetEventProcess(void)
{
	struct net_completion_list *nl;
	ST_NET_WORK *done;
	MP_DEBUG("%s: g_bXpgStatus %d",__func__, g_bXpgStatus);

    nl = &net_completion_list;

	mpx_SemaphoreWait(nl->lock);
	while (!list_empty(&nl->donelist)) {
		done = list_first_entry(&nl->donelist, ST_NET_WORK, work);
		list_del_init(&done->work);
		mpx_SemaphoreRelease(nl->lock);

#ifdef NET_NAPI
        MP_DEBUG("%s: --> xpgNet_HsmOnEvent, %p",__func__, done);
        xpgNet_HsmOnEvent(XPG_HTTP_COMPLETION, NULL, done);
#endif

        mpx_Free(done);

		mpx_SemaphoreWait(nl->lock);
	}
	mpx_SemaphoreRelease(nl->lock);

    if (g_boNeedRepaint)
        xpgUpdateStage();
}

void net_insert_done(ST_NET_WORK *done)
{
    if (!done)
        return;

	MP_DEBUG("%s: %p", __func__, done);
	mpx_SemaphoreWait(net_completion_list.lock);
    list_add_tail(&done->work, &net_completion_list.donelist);
	mpx_SemaphoreRelease(net_completion_list.lock);

    EventSet(net_completion_list.more_entry, EVENT_NET_COMPLETE);
}

int net_done_init(void)
{
	int ret;

	MP_DEBUG("%s", __func__);

    if (!net_completion_list.lock)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        MP_ASSERT(ret > 0);
        net_completion_list.lock = ret;
    }

    net_completion_list.more_entry = UI_EVENT;

	INIT_LIST_HEAD(&net_completion_list.donelist);

	return ret;
}

#endif
