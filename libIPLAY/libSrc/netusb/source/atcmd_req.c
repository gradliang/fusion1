
#include <stdio.h>
#include <sys/errno.h>

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#include "global612.h"
#include "mpTrace.h"
#include "os_mp52x.h"
#include <linux/list.h>

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif


struct atmcmd_request;                                     /* forward declaration */
typedef void (*uart_complete_t)(struct atmcmd_request *);

struct atmcmd_request {
	DWORD actual_length;
	uart_complete_t complete;	/* (in) completion routine */

#define ATCMD_NO_OK       	BIT0
#define ATCMD_SMS        	BIT1
#define ATCMD_PENDING       BIT2
	DWORD flags;

	void *context;			/* (in) context for completion */
    char *cmd;
    char *rx_pattern;
    char *data;
    int length;
    int done;
    int result;
    int timeout;                               /* in seconds */
    int status;
	struct list_head list;
};

#define atcmd_clear_pending(work) 		\
	do {							       		\
			work->flags &= ~ATCMD_PENDING;	\
	} while (0)
#define atcmd_set_pending(work) 		\
	do {							       		\
			work->flags |= ATCMD_PENDING;	\
	} while (0)

struct completion {
	unsigned int done;
    int wait;
	struct list_head cleanup;
};

struct api_context {
	struct completion	done;
	int			status;
};

static struct list_head atcmd_uart_list; /* AT cmd request list (UART interface) */
static unsigned char atcmd_uart_lock;

static void atcmd_api_blocking_completion(struct atmcmd_request *req);
static void complete(struct completion *x);
static int wait_for_completion_timeout(struct completion *x, unsigned long timeout);
static inline void init_completion(struct completion *x)
{
	x->done = 0;
    x->wait = 0;
}


static int atcmd_submit_req(struct atmcmd_request *req, char *cmd, char *pattern, 
		char *data, int length, int timeout)
{
    req->cmd = cmd;
    req->rx_pattern = pattern;
    req->data = data;
    req->length = length;

    req->complete = atcmd_api_blocking_completion;
    req->timeout = timeout;

	spin_lock(&atcmd_uart_lock);
    list_add_tail(&req->list, &atcmd_uart_list);
	spin_unlock(&atcmd_uart_lock);

	return 0;
}


/*
 * sync call
 */
int AtCmd_Request(struct usb_device *udev, char *cmd, char *pattern, char *result, int length, int timeout)
{
	struct atmcmd_request *req;
	struct api_context ctx;
	int to;                                     /* timed out */
	int r;

	req = mpx_Zalloc(sizeof(*req));
	if (!req)
		return -1;

	init_completion(&ctx.done);
	ctx.done.udev_index = udev->devnum;
    ctx.status = -ENOENT;
	req->context = &ctx;

	r = atcmd_submit_req(req, cmd, pattern,
		         result, length, timeout /* sec */);

	to = wait_for_completion_timeout(&ctx.done, timeout);
	if (!to) {
		spin_lock(&atcmd_uart_lock);
		list_del(&req->list);
		spin_unlock(&atcmd_uart_lock);

		r = -ETIMEDOUT;
		goto error;
	}

	r = ctx.status;
error:
	SAFE_FREE(req);
	return r;
}

void sio_run_atcmd(void)
{
	int ret;
	struct atmcmd_request *req;

	MP_DEBUG("%s --> ", __func__);
	spin_lock(&atcmd_uart_lock);
	while (!list_empty(&atcmd_uart_list)) {

		req = list_first_entry(&atcmd_uart_list, struct atmcmd_request, list);
		list_del_init(&req->list);
		spin_unlock(&atcmd_uart_lock);

		atcmd_clear_pending(req);

        if (req->flags & ATCMD_NO_OK)
            ret = __edge_setcmd4(req->cmd, req->rx_pattern, req->data, req->length, req->timeout);
        else if (req->flags & ATCMD_SMS)
            ;
        else
            ret = __edge_setcmd2(req->cmd, req->rx_pattern, req->data, req->length, 60);

        MP_DEBUG("%s: req=%p", __func__, req);

		if (ret)
			req->status = 0;                    /* OK */
		else
			req->status = -1;                   /* error */
        req->complete(req);

		spin_lock(&atcmd_uart_lock);
	}
	spin_unlock(&atcmd_uart_lock);

}

void atcmd_api_init(void)
{
	int status;

    status = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
	MP_ASSERT(status > 0);
	atcmd_uart_lock = status;

    INIT_LIST_HEAD(&atcmd_uart_list);
}

static int
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
    if (!x->done)
    {
        x->wait = TaskGetId();
        TaskSleep(timeout * 1000);
    }
    MP_DEBUG("%s: done", __func__);
	if (!x->done)
    {
        return 0;                     /* 0 indicates timeout */
    }
    x->done--;
	return 1;
}

static void complete(struct completion *x)
{
	unsigned long flags;

	x->done++;
    TaskWakeup(x->wait);
}

static void atcmd_api_blocking_completion(struct atmcmd_request *req)
{
	struct api_context *ctx = req->context;

	ctx->status = req->status;
	complete(&ctx->done);
}

// vim: :noexpandtab:
