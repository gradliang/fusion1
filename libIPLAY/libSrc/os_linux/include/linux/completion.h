#ifndef __COMPLETION_H__
#define __COMPLETION_H__

#include <linux/wait.h>

struct completion {
	unsigned int done;
#ifdef LINUX
	wait_queue_head_t wait;
#else
    int wait;
	struct list_head cleanup;
    int udev_index;
#endif
};

static inline void init_completion(struct completion *x)
{
	x->done = 0;
#ifdef LINUX
	init_waitqueue_head(&x->wait);
#else
    x->wait = 0;
//    mpDebugPrint("%s: x=%p,s=%d", __func__, x, x->wait);
#endif
}

#define INIT_COMPLETION(x)	((x).done = 0)

extern unsigned long wait_for_completion_timeout(struct completion *x,
						   unsigned long timeout);
extern void complete(struct completion *);

#endif /* !__COMPLETION_H__ */
