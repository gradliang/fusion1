/*
 * linux/kernel/workqueue.c
 *
 * Generic mechanism for defining kernel helper threads for running
 * arbitrary tasks in process context.
 *
 * Started by Ingo Molnar, Copyright (C) 2002
 *
 * Derived from the taskqueue/keventd code by:
 *
 *   David Woodhouse <dwmw2@infradead.org>
 *   Andrew Morton <andrewm@uow.edu.au>
 *   Kai Petzke <wpp@marie.physik.tu-berlin.de>
 *   Theodore Ts'o <tytso@mit.edu>
 *
 * Made to use alloc_percpu by Christoph Lameter <clameter@sgi.com>.
 */

#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include "ndebug.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "osconst.h"

/*
 * The externally visible workqueue abstraction is an array of
 * per-CPU workqueues:
 */
struct workqueue_struct {
	spinlock_t lock;
	struct list_head worklist;
	int taskid;
	void *thread;

	int more_work;                              /* act as an event */
	struct list_head list;
	const char *name;
	int singlethread;
};

static struct workqueue_struct *keventd_wq ;

extern int ar2524_initialized;

#define MAX_NUM_WORKQUEUES  2
static struct workqueue_struct *wq_global[MAX_NUM_WORKQUEUES];

int mp_workqueue_event(char *name);
int mp_workqueue_semaphore(char *name);
int mp_workqueue_task(char *name);

/* All the per-cpu workqueues on the system, for hotplug cpu to add/remove
   threads to each one as cpus come/go. */
static DEFINE_MUTEX(workqueue_mutex);
static LIST_HEAD(workqueues);

static void cleanup_workqueue_thread(struct workqueue_struct *cwq);

/* If it's single threaded, it isn't in the list of workqueues. */
static inline int is_single_threaded(struct workqueue_struct *wq)
{
	return wq->singlethread;
}

/*
 * Set the workqueue on which a work item is to be run
 * - Must *only* be called if the pending flag is set
 */
static inline void set_wq_data(struct work_struct *work,
				struct workqueue_struct *cwq)
{
	unsigned long new;

	BUG_ON(!work_pending(work));
    MP_ASSERT(((unsigned long)cwq & WORK_STRUCT_FLAG_MASK) == 0);

	new = (unsigned long) cwq | (1UL << WORK_STRUCT_PENDING);
	new |= WORK_STRUCT_FLAG_MASK & *work_data_bits(work);
	atomic_long_set(&work->data, new);
}

static inline
struct workqueue_struct *get_wq_data(struct work_struct *work)
{
	return (void *) (atomic_long_read(&work->data) & WORK_STRUCT_WQ_DATA_MASK);
}

static void insert_work(struct workqueue_struct *cwq,
				struct work_struct *work, int tail)
{
	set_wq_data(work, cwq);
	/*
	 * Ensure that we get the right work->data if we see the
	 * result of list_add() below, see try_to_grab_pending().
	 */
	if (tail)
		list_add_tail(&work->entry, &cwq->worklist);
	else
		list_add(&work->entry, &cwq->worklist);
    EventSet(cwq->more_work, 1);
}

/* Preempt must be disabled. */
static void __queue_work(struct workqueue_struct *cwq,
			 struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&cwq->lock, flags);
	insert_work(cwq, work, 1);
	spin_unlock_irqrestore(&cwq->lock, flags);
}

/**
 * queue_work - queue work on a workqueue
 * @wq: workqueue to use
 * @work: work to queue
 *
 * Returns 0 if @work was already on a queue, non-zero otherwise.
 */
int queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	int ret = 0;

	if (!test_and_set_bit(WORK_STRUCT_PENDING, work_data_bits(work))) {
		__queue_work(wq, work);
		ret = 1;
	}
	return ret;
}

void delayed_work_timer_fn(unsigned long __data)
{
	struct delayed_work *dwork = (struct delayed_work *)__data;
	struct workqueue_struct *wq = get_wq_data(&dwork->work);

//    MP_FUNCTION_ENTER();
	__queue_work(wq, &dwork->work);
}

/**
 * queue_delayed_work - queue work on a workqueue after delay
 * @wq: workqueue to use
 * @dwork: delayable work to queue
 * @delay: number of jiffies to wait before queueing
 *
 * Returns 0 if @work was already on a queue, non-zero otherwise.
 */
int queue_delayed_work(struct workqueue_struct *wq,
			struct delayed_work *dwork, unsigned long delay)
{
#ifdef LINUX
	timer_stats_timer_set_start_info(&dwork->timer);
#endif
	if (delay == 0)
		return queue_work(wq, &dwork->work);

	return queue_delayed_work_on(-1, wq, dwork, delay);
}
EXPORT_SYMBOL_GPL(queue_delayed_work);

/**
 * queue_delayed_work_on - queue work on specific CPU after delay
 * @cpu: CPU number to execute work on
 * @wq: workqueue to use
 * @dwork: work to queue
 * @delay: number of jiffies to wait before queueing
 *
 * Returns 0 if @work was already on a queue, non-zero otherwise.
 */
int queue_delayed_work_on(int cpu, struct workqueue_struct *wq,
			struct delayed_work *dwork, unsigned long delay)
{
	int ret = 0;
	_timer *timer = &dwork->timer;
	struct work_struct *work = &dwork->work;

	if (!test_and_set_bit(WORK_STRUCT_PENDING, work_data_bits(work))) {
		BUG_ON(timer_pending(timer));
		BUG_ON(!list_empty(&work->entry));

        MP_DEBUG("queue_delayed_work_on: delay=%d", delay);
		/* This stores cwq for the moment, for the timer_fn */
		set_wq_data(work, wq);
		timer->expires = jiffies + delay;
		timer->data = (unsigned long)dwork;
		timer->function = delayed_work_timer_fn;

        add_timer(timer);
		ret = 1;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(queue_delayed_work_on);

static void run_workqueue(struct workqueue_struct *cwq)
{
    struct list_head *worklist = &cwq->worklist;
//    MP_FUNCTION_ENTER();
	spin_lock_irq(&cwq->lock);
//    MP_ALERT("%s: worklist=%p", __func__,worklist); 
	while (!list_empty(worklist)) {
		struct work_struct *work = list_entry(worklist->next,
						struct work_struct, entry);
		work_func_t f = work->func;
//        MP_DEBUG("%s: work=%p", __func__,work); 

		list_del_init(worklist->next);
		spin_unlock_irq(&cwq->lock);

		work_clear_pending(work);
        MP_DEBUG("%s: work=%p f=%p", __func__,work,f); 
		f(work);

		spin_lock_irq(&cwq->lock);
	}
	spin_unlock_irq(&cwq->lock);
//    MP_FUNCTION_EXIT();
}

int worker_thread0(void)
{
	struct workqueue_struct *wq = wq_global[0];
	__u32 dwEvent;

	for (;;) {
		EventWait(wq->more_work,0xffffffff,OS_EVENT_OR, &dwEvent);
		if (!list_empty(&wq->worklist))
            run_workqueue(wq);
#if (DEMO_PID&&(Make_DM9621_ETHERNET==0))
     TaskSleep(1);
#endif
	}

	return 0;
}

int worker_thread1(void)
{
	struct workqueue_struct *wq = wq_global[1];
	__u32 dwEvent;

	for (;;) {
		EventWait(wq->more_work,0xffffffff,OS_EVENT_OR, &dwEvent);
		if (!list_empty(&wq->worklist))
            run_workqueue(wq);
#if (DEMO_PID&&(Make_DM9621_ETHERNET==0))
     TaskSleep(1);
#endif
		
	}

	return 0;
}

struct wq_barrier {
	struct work_struct	work;
	struct completion	done;
};

static void wq_barrier_func(struct work_struct *work)
{
	struct wq_barrier *barr = container_of(work, struct wq_barrier, work);
	complete(&barr->done);
}

static void insert_wq_barrier(struct workqueue_struct *cwq,
			struct wq_barrier *barr, struct list_head *head)
{
	INIT_WORK(&barr->work, wq_barrier_func);
	__set_bit(WORK_STRUCT_PENDING, work_data_bits(&barr->work));

	init_completion(&barr->done);

	insert_work(cwq, &barr->work, head);
}

/**
 * flush_workqueue - ensure that any scheduled work has run to completion.
 * @wq: workqueue to flush
 *
 * Forces execution of the workqueue and blocks until its completion.
 * This is typically used in driver shutdown handlers.
 *
 * We sleep until all works which were queued on entry have been handled,
 * but we are not livelocked by new incoming ones.
 *
 * This function used to run the workqueues itself.  Now we just wait for the
 * helper threads to do it.
 */
int flush_workqueue(struct workqueue_struct *cwq)
{
	int active;
    struct wq_barrier barr;

    active = 0;
    spin_lock_irq(&cwq->lock);
    if (!list_empty(&cwq->worklist)) {
        insert_wq_barrier(cwq, &barr, &cwq->worklist);
        active = 1;
    }
    spin_unlock_irq(&cwq->lock);

    if (active)
        wait_for_completion(&barr.done);

    return active;
}

void flush_scheduled_work(void)
{
	flush_workqueue(keventd_wq);
}

static int __cancel_work_timer(struct work_struct *work,
				_timer * timer)
{
	int ret;

	do {
		ret = (timer && likely(del_timer(timer)));
#ifdef LINUX
		if (!ret)
			ret = try_to_grab_pending(work);
#endif
	} while (unlikely(ret < 0));

	work_clear_pending(work);
	return ret;
}

/**
 * cancel_work_sync - block until a work_struct's callback has terminated
 * @work: the work which is to be flushed
 *
 * Returns true if @work was pending.
 *
 * cancel_work_sync() will cancel the work if it is queued. If the work's
 * callback appears to be running, cancel_work_sync() will block until it
 * has completed.
 *
 * It is possible to use this function if the work re-queues itself. It can
 * cancel the work even if it migrates to another workqueue, however in that
 * case it only guarantees that work->func() has completed on the last queued
 * workqueue.
 *
 * cancel_work_sync(&delayed_work->work) should be used only if ->timer is not
 * pending, otherwise it goes into a busy-wait loop until the timer expires.
 *
 * The caller must ensure that workqueue_struct on which this work was last
 * queued can't be destroyed before this function returns.
 */
int cancel_work_sync(struct work_struct *work)
{
	return __cancel_work_timer(work, NULL);
}

/**
 * schedule_delayed_work - put work task in global workqueue after delay
 * @dwork: job to be done
 * @delay: number of jiffies to wait or 0 for immediate execution
 *
 * After waiting for a given time this puts a job in the kernel-global
 * workqueue.
 */
int schedule_delayed_work(struct delayed_work *dwork,
					unsigned long delay)
{
#ifdef LINUX
	timer_stats_timer_set_start_info(&dwork->timer);
#endif
	return queue_delayed_work(keventd_wq, dwork, delay);
}
/**
 * cancel_delayed_work_sync - reliably kill off a delayed work.
 * @dwork: the delayed work struct
 *
 * Returns true if @dwork was pending.
 *
 * It is possible to use this function if @dwork rearms itself via queue_work()
 * or queue_delayed_work(). See also the comment for cancel_work_sync().
 */
int cancel_delayed_work_sync(struct delayed_work *dwork)
{
	return __cancel_work_timer(&dwork->work, &dwork->timer);
}

#ifdef LINUX
static int create_workqueue_thread(struct cpu_workqueue_struct *cwq, int cpu)
#else
static int create_workqueue_thread(struct workqueue_struct *cwq)
#endif
{
#if 0
	int tid = mpx_TaskCreate(worker_thread0, 4, 0x1000);
#else
	int tid = mp_workqueue_task(cwq->name);
#endif

    if (tid <= 0)
        return -1;

	return tid;
}

static void start_workqueue_thread(int taskid)
{
    TaskStartup(taskid);                           /* TODO */
}

/**
 * destroy_workqueue - safely terminate a workqueue
 * @wq: target workqueue
 *
 * Safely destroy a workqueue. All work currently pending will be done first.
 */
void destroy_workqueue(struct workqueue_struct *wq)
{
	list_del(&wq->list);

    cleanup_workqueue_thread(wq);

#ifdef LINUX
	kfree(wq);
#endif
}
EXPORT_SYMBOL_GPL(destroy_workqueue);
struct workqueue_struct *create_workqueue(const char *name,
					    int singlethread, int freezeable)
{
	struct workqueue_struct *wq;
	int err = 0;

    MP_FUNCTION_ENTER();

    if (ar2524_initialized)
    {
        if (strcmp(name, "ar2524") == 0)
            wq = wq_global[0];
        else if (strcmp(name, "ar2524_wiphy") == 0)
            wq = wq_global[1];
        wq->thread = &wq->taskid;
        return wq;
    }

	wq = kzalloc(sizeof(*wq), GFP_KERNEL);
	if (!wq)
		return NULL;
    MP_DEBUG("create_workqueue: wq=%p", wq);  
    MP_ALERT("create_workqueue: name=%s", name);  

#if 0
	err = mp_OsEvent((OS_ATTR_FIFO|OS_ATTR_EVENT_CLEAR), 0);
#else
	err = mp_workqueue_event(name);
#endif
    MP_ASSERT(err > 0);
    if(err <= 0)
    {
  		destroy_workqueue(wq);
		wq = NULL;
		return wq;
    }

	wq->more_work = err;
	wq->name = name;

	INIT_LIST_HEAD(&wq->list);
	INIT_LIST_HEAD(&wq->worklist);

#ifdef LINUX
    spin_lock_init(&wq->lock);
#else
    wq->lock = mp_workqueue_semaphore(name);
#endif

    err = create_workqueue_thread(wq);
    MP_ASSERT(err > 0);
    wq->taskid = err;
    wq->thread = &wq->taskid;

	if (err <= 0) {
		destroy_workqueue(wq);
		wq = NULL;
	}
    else
    {
        keventd_wq = wq;
        if (strcmp(name, "ar2524") == 0)
            wq_global[0] = wq;
        else if (strcmp(name, "ar2524_wiphy") == 0)
            wq_global[1] = wq;
		
		if(!UsbOtgWifiPlugin())
		{
				destroy_workqueue(wq);
				wq = NULL;
				return wq;

		}
		
        start_workqueue_thread(err);
    }

    MP_FUNCTION_EXIT();
	return wq;
}

/**
 * schedule_work - put work task in global workqueue
 * @work: job to be done
 *
 * This puts a job in the kernel-global workqueue.
 */
int schedule_work(struct work_struct *work)
{
	return queue_work(keventd_wq, work);
}

static void cleanup_workqueue_thread(struct workqueue_struct *cwq)
{
	/*
	 * Our caller is either destroy_workqueue() or CPU_POST_DEAD,
	 * cpu_add_remove_lock protects cwq->thread.
	 */
	if (cwq->thread == NULL)
		return;

#ifdef LINUX
	lock_map_acquire(&cwq->wq->lockdep_map);
	lock_map_release(&cwq->wq->lockdep_map);

	flush_cpu_workqueue(cwq);
#else
	flush_workqueue(cwq);
#endif
	/*
	 * If the caller is CPU_POST_DEAD and cwq->worklist was not empty,
	 * a concurrent flush_workqueue() can insert a barrier after us.
	 * However, in that case run_workqueue() won't return and check
	 * kthread_should_stop() until it flushes all work_struct's.
	 * When ->worklist becomes empty it is safe to exit because no
	 * more work_structs can be queued on this cwq: flush_workqueue
	 * checks list_empty(), and a "normal" queue_work() can't use
	 * a dead CPU.
	 */
#ifdef LINUX
	kthread_stop(cwq->thread);
#endif
	cwq->thread = NULL;
}
