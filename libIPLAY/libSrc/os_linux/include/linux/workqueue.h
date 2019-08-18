
#ifndef _LINUX_WORKQUEUE_H
#define _LINUX_WORKQUEUE_H

#include <linux/timer.h>
#include <asm/atomic.h>
//#include "osdep_mpixel_service.h"
#include "linux/list.h"

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
	atomic_long_t data;
#define WORK_STRUCT_PENDING 0		/* T if work item pending execution */
#define WORK_STRUCT_FLAG_MASK (3UL)
#define WORK_STRUCT_WQ_DATA_MASK (~WORK_STRUCT_FLAG_MASK)
	struct list_head entry;
	work_func_t func;
};

#define WORK_DATA_INIT()	ATOMIC_LONG_INIT(0)

struct delayed_work {
	struct work_struct work;
	struct timer_list timer;
};

/*
 * initialize a work item's function pointer
 */
#define PREPARE_WORK(_work, _func)				\
	do {							\
		(_work)->func = (_func);			\
	} while (0)

#define PREPARE_DELAYED_WORK(_work, _func)			\
	PREPARE_WORK(&(_work)->work, (_func))

#define __WORK_INITIALIZER(n, f) {				\
	.data = WORK_DATA_INIT(),				\
	.entry	= { &(n).entry, &(n).entry },			\
	.func = (f)						\
	}
#define DECLARE_WORK(n, f)					\
	struct work_struct n = __WORK_INITIALIZER(n, f)

/*
 * initialize all of a work item in one go
 *
 * NOTE! No point in using "atomic_long_set()": useing a direct
 * assignment of the work data initializer allows the compiler
 * to generate better code.
 */
#define INIT_WORK(_work, _func)						\
	do {								\
		(_work)->data = (atomic_long_t) WORK_DATA_INIT();	\
		INIT_LIST_HEAD(&(_work)->entry);			\
		PREPARE_WORK((_work), (_func));				\
	} while (0)

#define INIT_DELAYED_WORK(_work, _func)				\
	do {							\
		INIT_WORK(&(_work)->work, (_func));		\
		_init_timer(&(_work)->timer, NULL, NULL, NULL);			\
	} while (0)

#define INIT_DELAYED_WORK_DEFERRABLE(_work, _func)			\
	do {							\
		INIT_WORK(&(_work)->work, (_func));		\
		_init_timer(&(_work)->timer, NULL, NULL, NULL);		\
	} while (0)

/**
 * work_pending - Find out whether a work item is currently pending
 * @work: The work item in question
 */
#define work_pending(work) \
	test_bit(WORK_STRUCT_PENDING, work_data_bits(work))
/*
 * The first word is the work queue pointer and the flags rolled into
 * one
 */
#define work_data_bits(work) ((unsigned long *)(&(work)->data))


/**
 * work_clear_pending - for internal use only, mark a work item as not pending
 * @work: The work item in question
 */
#define work_clear_pending(work) \
	clear_bit(WORK_STRUCT_PENDING, work_data_bits(work))

/* Obsolete. use cancel_delayed_work_sync() */
struct workqueue_struct;
static inline
void cancel_rearming_delayed_workqueue(struct workqueue_struct *wq,
					struct delayed_work *work)
{
	cancel_delayed_work_sync(work);
}

/* Obsolete. use cancel_delayed_work_sync() */
static inline
void cancel_rearming_delayed_work(struct delayed_work *work)
{
	cancel_delayed_work_sync(work);
}

/*
 * Kill off a pending schedule_delayed_work().  Note that the work callback
 * function may still be running on return from cancel_delayed_work(), unless
 * it returns 1 and the work doesn't re-arm itself. Run flush_workqueue() or
 * cancel_work_sync() to wait on it.
 */
static inline int cancel_delayed_work(struct delayed_work *work)
{
	int ret;

	ret = del_timer_sync(&work->timer);
	if (ret)
		work_clear_pending(&work->work);
	return ret;
}
#define create_freezeable_workqueue(name) create_workqueue((name), 1, 1)
#define create_singlethread_workqueue(name) create_workqueue((name), 1, 0)

extern void destroy_workqueue(struct workqueue_struct *wq);

#endif

