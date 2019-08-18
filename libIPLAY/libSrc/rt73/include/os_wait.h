#ifndef _OS_WAIT_H
#define _OS_WAIT_H

#include "os_list.h"

struct __wait_queue_head {
//yuming, change spinlock to semaphore
#if 0
	spinlock_t lock;
#else
        U08 lock;
#endif
	struct list_head task_list;
};
typedef struct __wait_queue_head wait_queue_head_t;

extern void init_waitqueue_head(wait_queue_head_t *q);

#endif

