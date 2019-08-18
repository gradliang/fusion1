// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 1  

#include "global612.h"
#include "mpTrace.h"

#include <linux/types.h>
#include <linux/list.h>
#include "net_sys.h"
#include "timer.h"
#include "taskid.h"
#include "netware.h"
#include "devio.h"
#include "net_autosearch.h"
#include "net_nic.h"
#include "net_dhcp.h"

#include "net_device.h"
#include "wlan_sys.h"
#include "typedef.h"
#include "wireless.h"
#include "wlan_common.h"
#include <linux/if_ether.h>
#include "net_smbclient.h"
#include "net_async.h"

#define perror mpDebugPrint

typedef struct smbitem smbitem;

struct smbitem{
    smbitem	*next;
    int		type;
    char	name[1];
};
static ST_NET_WORKQUEUE net_smb_list;
static u8 net_smb_task_id;

extern Net_App_State App_State;

static void smbclient_run(ST_NET_WORKQUEUE *wq);
NWSERVER *add_server(char *hostname, char *url);
int SmbCli_GetServers(void);
static int smbc_submit_request(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work);
static void __queue_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work);
void async_req_done2(struct async_req2 *req);
bool AutoSearch_IsDone(void);

//#define list_first_entry(ptr, type, member) 
static void SmbTask(ST_NET_WORKQUEUE *wq)
{
    DWORD dwEvent;

    for (;;) {
        EventWait(wq->more_work,0xffffffff,OS_EVENT_OR, &dwEvent);
        if (!list_empty(&wq->worklist))
            smbclient_run(wq);
    }

}


void SmbCli_Init(void)
{
    int ret;

    if (!net_smb_list.more_work)
    {
        ret = mpx_EventCreate(OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR, 0);
        if(ret < 0)
        {
            DPrintf("[SMB] event create fail");
            BREAK_POINT();
        }

        net_smb_list.more_work = ret;
    }

    if (!net_smb_list.lock)
    {
        ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
        if(ret < 0)
        {
            DPrintf("[SMB] event create fail");
            BREAK_POINT();
        }
        net_smb_list.lock = ret;
    }

    INIT_LIST_HEAD(&net_smb_list.worklist);

    if (!net_smb_task_id)
    {
        ret = mpx_TaskCreate(SmbTask, WIFI_PRIORITY-1, 0x2000);
        if(ret < 0)
        {
            DPrintf("[SMB] event create fail");
            BREAK_POINT();
        }
        net_smb_task_id = (BYTE)ret;
        mpx_TaskStartup((unsigned char) ret, &net_smb_list);  
    }
//    talloc_enable_leak_report();

}

static void smbclient_run(ST_NET_WORKQUEUE *wq)
{
    int res;
    u32 cmd;
    ST_NET_WORK *work;
    struct async_req2 *req;

    MP_DEBUG("%s --> ", __func__);

    spin_lock(&wq->lock);
    while (!list_empty(&wq->worklist)) {

        work = list_first_entry(&wq->worklist, ST_NET_WORK, work);
        list_del_init(&work->work);
        spin_unlock(&wq->lock);

        MP_TRACE_LINE();

        cmd = work->data1;

        net_work_clear_pending(work);
        switch (cmd)
        {
            case SMBCLIENT_BROWSE:
                SmbCli_GetServers();
                break;
        }

//        MP_DEBUG("%s: work=%p", __func__, work);
        req = work->context;
        async_req_done2(req);
//        smb_insert_done(work);
        MP_TRACE_LINE();

        spin_lock(&wq->lock);
    }
    spin_unlock(&wq->lock);


}

/**
 * Async request
 */
void * SmbCli_BrowserReq(void)
{
    ST_NET_WORK *work;
    struct async_req2 *req;
    int ret;

    mpDebugPrint("%s(%d)", __func__, __LINE__);
    work = mpx_Zalloc(sizeof(*work));
    if (!work)
        return NULL;

    INIT_LIST_HEAD(&work->work);

    req = mpx_Zalloc(sizeof(*req));
    if (!req)
        return NULL;

    work->data1 = (u32) SMBCLIENT_BROWSE;
    req->state = ASYNC_REQ_IN_PROGRESS;
    req->work = work;
    work->context = req;
    mpDebugPrint("%s(%d): work=%p,req=%p", __func__, __LINE__,work,req);
    ret = smbc_submit_request(&net_smb_list, work);
    if (ret == 0)
    {
        mpx_Free(work);
        work = NULL;
	    __asm("break 100");
        return NULL;
    }
    return work;
}

BYTE Polling_Event(void);
SmbCli_LoopWait(ST_NET_WORK *work)
{
    struct async_req2 *req;
    req = work->context;
    BYTE evt;

//    struct event_context *ev;
//    ev = event_context_init2();
    mpDebugPrint("%s(%d): work=%p,req=%p,state=%d", __func__, __LINE__,work,req,req->state );
    while (req->state < ASYNC_REQ_DONE) {
        evt=Polling_Event();
        if (evt)
        {
            mpDebugPrint("%s(%d)", __func__, __LINE__,evt );
            break;
        }
        TaskYield();
//        mevent_loop_once(ev);
    }
//    mpx_Free(ev);
}

static void insert_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work, int tail)
{
	/*
	 * Ensure that we get the right work->data if we see the
	 * result of list_add() below, see try_to_grab_pending().
	 */
	if (tail)
		list_add_tail(&work->work, &net_smb_list.worklist);
	else
		list_add(&work->work, &net_smb_list.worklist);
}

static void __queue_work(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work)
{
    spin_lock(&wq->lock);
    insert_work(wq, work, 1);
    spin_unlock(&wq->lock);

    EventSet(wq->more_work, 1);
}

static int smbc_submit_request(ST_NET_WORKQUEUE *wq, ST_NET_WORK *work)
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

void SmbEventProcess(void)
{
    ST_NET_WORK *done;
}

void smb_insert_done(ST_NET_WORK *done)
{
    StopAutoSearch();
//    EventSet(net_completion_list.more_entry, EVENT_NET_COMPLETE);
}

static bool run_events2(struct mevent_context *ev,
		int selrtn, fd_set *read_fds, fd_set *write_fds)
{
	struct tevent_fd *fde;
	struct timeval now;

	if (ev->signal_events &&
	    tevent_common_check_signal(ev)) {
		return true;
	}

	if (ev->immediate_events &&
	    tevent_common_loop_immediate(ev)) {
		return true;
	}

        if (AutoSearch_IsDone())
            return true;

	return false;
}
static int s4_event_loop_once(struct mevent_context *ev, const char *location)
{
	struct timeval now, to;
	int maxfd = 0;
	int ret;


	to.tv_sec = 9999;	/* Max timeout */
	to.tv_usec = 0;

//    mpDebugPrint("%s", __location__);
	if (run_events2(ev, 0, NULL, NULL)) {
		return 0;
	}

	return 0;
}

/*
  do a single event loop using the events defined in ev 
*/
int mevent_loop_once(struct mevent_context *ev, const char *location)
{
	int ret;
	void *nesting_stack_ptr = NULL;

	ev->nesting.level++;

	if (ev->nesting.level > 1) {
		if (!ev->nesting.allowed) {
			errno = ELOOP;
			return -1;
		}
		if (ev->nesting.hook_fn) {
			int ret2;
			ret2 = ev->nesting.hook_fn(ev,
						   ev->nesting.hook_private,
						   ev->nesting.level,
						   true,
						   (void *)&nesting_stack_ptr,
						   location);
			if (ret2 != 0) {
				ret = ret2;
				goto done;
			}
		}
	}

//    mpDebugPrint("%s: %p", __location__, ev->ops->loop_once);
    mpDebugPrint("%s(%d)", __FUNCTION__, __LINE__);
#ifdef LINUX
	ret = ev->ops->loop_once(ev, location);
#else
	ret = s4_event_loop_once(ev, location);
#endif

//    mpDebugPrint("%s: %p", __location__, ev->ops->loop_once);
	if (ev->nesting.level > 1) {
		if (ev->nesting.hook_fn) {
			int ret2;
			ret2 = ev->nesting.hook_fn(ev,
						   ev->nesting.hook_private,
						   ev->nesting.level,
						   false,
						   (void *)&nesting_stack_ptr,
						   location);
			if (ret2 != 0) {
				ret = ret2;
				goto done;
			}
		}
	}
//    mpDebugPrint("%s", __location__);

done:
	ev->nesting.level--;
	return ret;
}

static void async_req_finish2(struct async_req2 *req, enum async_req_state state)
{
	req->state = state;
	if (req->async.fn != NULL) {
		req->async.fn(req);
	}
}

/**
 * @brief An async request has successfully finished
 * @param[in] req	The finished request
 *
 * async_req_done is to be used by implementors of async requests. When a
 * request is successfully finished, this function calls the user's completion
 * function.
 */

void async_req_done2(struct async_req2 *req)
{
	async_req_finish2(req, ASYNC_REQ_DONE);
}

struct mevent_context *event_context_init2()
{
    struct mevent_context *ev;
    ev = mpx_Zalloc(sizeof(*ev));
    return ev;
}

size_t smb_write_func(void *ptr, size_t size, void *buf)
{
    XML_BUFF_link_t *xp = buf;
    size_t len = size;
    size_t write_len;		
    while(xp && xp->link)
        xp = xp->link;

    while (len > 0)
    {
        /* check data length to avoid buffer overflow */
        if (len <= (IMAGE_BUF - xp->buff_len) )
        {
            memcpy(&xp->BUFF[xp->buff_len], ptr, len);
            write_len = len;
            len = 0;
            xp->buff_len += write_len;
            App_State.dwTotallen += write_len;	
#if MAKE_XPG_PLAYER
            NetListProcess(App_State.dwTotallen, 0, 0, 2); /* refresh current downloaded size on screen */
#endif
            MP_DEBUG1("1 smb_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 
        }
        else
        {
            XML_BUFF_link_t *extra_xp;

            MP_DEBUG("smb_write_func: data length exceeds left buffer space");
            extra_xp = (XML_BUFF_link_t *)ext_mem_malloc(sizeof(XML_BUFF_link_t));
            if (!extra_xp)
            {
                MP_ALERT("smb_write_func(): cannot allocate memory => discard such big data!");
                return 0;
            } else
            {
                extra_xp->buff_len = 0;
                extra_xp->link = 0;
            }

            MP_DEBUG("IMAGE_BUF %d",IMAGE_BUF);
            memcpy(&xp->BUFF[xp->buff_len], ptr, IMAGE_BUF - xp->buff_len);
            write_len = IMAGE_BUF - xp->buff_len;
            len -= write_len;
            xp->buff_len = IMAGE_BUF;
            xp->link = extra_xp;                /* TODO */
            xp = extra_xp;
#if MAKE_XPG_PLAYER
            NetListProcess(App_State.dwTotallen, 0, 0, 2); /* refresh current downloaded size on screen */
#endif
            App_State.dwTotallen += write_len;		
            MP_DEBUG1("2 smb_write_func(): write App_State.dwTotallen (%d) ", App_State.dwTotallen); 	
        }
    }

}
