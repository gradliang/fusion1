
#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/preempt.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/irqflags.h>
#include <asm/atomic.h>


struct tasklet_struct
{
	struct tasklet_struct *next;
	unsigned long state;
	atomic_t count;
	void (*func)(unsigned long);
	unsigned long data;
	int task_id;
	int event_id;
};

static inline void tasklet_enable(struct tasklet_struct *t)
{
    MP_ASSERT(t->task_id > 0);
	TaskStartup(t->task_id, t);
    TaskWakeup(t->task_id);
}

static inline void tasklet_schedule(struct tasklet_struct *t)
{
    MP_ASSERT(t->event_id > 0);
    EventSet(t->event_id, 1);
}

static inline void tasklet_hi_schedule(struct tasklet_struct *t)
{
    MP_ASSERT(t->event_id > 0);
    EventSet(t->event_id, 1);
}

#define tasklet_disable(t)
extern void tasklet_init(struct tasklet_struct *t,
			 void (*func)(unsigned long), unsigned long data);

extern void tasklet_kill(struct tasklet_struct *t);

#endif

