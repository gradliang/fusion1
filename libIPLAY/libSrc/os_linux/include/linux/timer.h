#ifndef _LINUX_TIMER_H
#define _LINUX_TIMER_H

#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/stddef.h>

struct timer_list{
    struct timer_list *next, *prev;
    unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

typedef 	struct timer_list _timer;
void usb_add_timer(_timer *newtimer);

/**
 * timer_pending - is a timer pending?
 * @timer: the timer in question
 *
 * timer_pending will tell whether a given timer is currently pending,
 * or not. Callers must ensure serialization wrt. other operations done
 * to this timer, eg. interrupt contexts, or other CPUs on SMP.
 *
 * return value: 1 if the timer is pending, 0 if not.
 */
static inline int timer_pending(const struct timer_list * timer)
{
	return timer->next != NULL;
}

#define del_timer_sync(timer) usb_cancel_timer(timer)
#define del_timer(timer) usb_cancel_timer(timer)
extern int __mod_timer(struct timer_list *timer, unsigned long expires);

/**
 * add_timer - start a timer
 * @timer: the timer to be added
 *
 * The kernel will do a ->function(->data) callback from the
 * timer interrupt at the ->expires point in the future. The
 * current time is 'jiffies'.
 *
 * The timer's ->expires, ->function (and if the handler uses it, ->data)
 * fields must be set prior calling this function.
 *
 * Timers with an ->expires field in the past will be executed in the next
 * timer tick.
 */
static inline void add_timer(struct timer_list *timer)
{
	BUG_ON(timer_pending(timer));
    usb_add_timer(timer);
}

void usb_add_timer2(_timer *newtimer);
static __inline void add_timer2(_timer *ptimer)
{
	MP_ASSERT(timer_pending(ptimer) == 0);
    usb_add_timer2(ptimer);
}

static inline void _init_timer(_timer *ptimer,void * padapter,void *pfunc, void* cntx)
{	
	memset(ptimer, 0, sizeof *ptimer);
	ptimer->function = pfunc;
	ptimer->data = (unsigned long)cntx;
}
extern DWORD TickCount;
static inline int mod_timer(_timer *ptimer, unsigned long expires)
{
//	mpDebugPrint("%s t=%u\n", __func__, expires);
	ptimer->expires = expires;
    if (ptimer->next)                         /* timer is pending */
        ;                                     /* do nothing */
    else
        usb_add_timer(ptimer);
}
static inline void _set_timer(_timer *ptimer,u32 delay_time)
{	
#ifdef LINUX
	mod_timer(ptimer , (jiffies+(delay_time*HZ/1000)));	
#else
	ptimer->expires = TickCount + (delay_time*HZ/1000);
    if (ptimer->next)                         /* timer is pending */
        ;                                     /* do nothing */
    else
        usb_add_timer(ptimer);
#endif	
}
static __inline void _cancel_timer(_timer *ptimer,u8 *bcancelled)
{
	usb_cancel_timer(ptimer);
	*bcancelled=  TRUE;//TRUE
}

/**
 * init_timer - initialize a timer.
 * @timer: the timer to be initialized
 *
 * init_timer() must be done to a timer prior calling *any* of the
 * other timer functions.
 */
static inline void init_timer(struct timer_list *timer)
{
	memset(timer, 0, sizeof *timer);
}

static inline void setup_timer(struct timer_list * timer,
				void (*function)(unsigned long),
				unsigned long data)
{
	memset(timer, 0, sizeof *timer);
	timer->function = function;
	timer->data = data;
}

#define round_jiffies(j)    j
#endif

