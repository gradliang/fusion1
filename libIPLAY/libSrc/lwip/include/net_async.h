#ifndef __NET_ASYNC__H__
#define __NET_ASYNC__H__


struct ST_NET_WORK;
typedef void (*net_work_func_t)(struct net_request *work);

#define net_work_clear_pending(work) 		\
	do {							       		\
			work->flags &= ~NET_WORK_PENDING;	\
	} while (0)
#define net_work_set_pending(work) 		\
	do {							       		\
			work->flags |= NET_WORK_PENDING;	\
	} while (0)


typedef struct ST_NET_WORK {
	uint8_t  url[MAX_NET_LINK_LEN];
	uint32_t data1;
	uint32_t data2;
	uint32_t actual_length;
	uint32_t timeout;                              /* in seconds */
	net_work_func_t todo;
	net_work_func_t complete;

	uint32_t wq_data;

#define NET_WORK_PENDING       BIT0
#define NET_WORK_CANCEL        BIT1
	uint32_t flags;

	void *context;			/* (in) context for completion */
	uint32_t req_id;
    int done;
    int result;
	struct list_head work; /* for todo & done lists */
} ST_NET_WORK;

typedef ST_NET_WORK ST_NET_REQUEST ;

typedef struct {
    uint8_t lock;
	uint8_t more_work;

	/* for worklist list */
	struct list_head worklist;
} ST_NET_WORKQUEUE;

typedef void (*http_todo_t) (ST_NET_WORK *);
typedef void (*http_complete_t)(ST_NET_WORK *);

static inline void http_fill_request (ST_NET_WORK *work,
				     void *url,
				     http_todo_t todo_fn,
				     http_complete_t complete_fn,
                     int timeout)
{
	work->data1 = (DWORD) url;
//	work->data2 = (DWORD) type;
	work->todo =  todo_fn;
	work->complete =  complete_fn;
	work->timeout =  timeout;
}

#endif /* __NET_ASYNC__H__ */

