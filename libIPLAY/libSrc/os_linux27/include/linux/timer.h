#ifndef _LINUX_TIMER_H
#define _LINUX_TIMER_H

#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/stddef.h>
#include <linux/debugobjects.h>

struct timer_list{
    struct timer_list *next, *prev;
    unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

typedef 	struct timer_list _timer;
void usb_add_timer(_timer *newtimer);

struct tvec_base;


extern struct tvec_base boot_tvec_bases;

#ifdef LINUX
#define TIMER_INITIALIZER(_function, _expires, _data) {		\
		.entry = { .prev = TIMER_ENTRY_STATIC },	\
		.function = (_function),			\
		.expires = (_expires),				\
		.data = (_data),				\
		.base = &boot_tvec_bases,			\
	}
#else
#define TIMER_INITIALIZER(_function, _expires, _data) {		\
		.next = 0,	\
		.prev = 0,	\
		.function = (_function),			\
		.expires = (_expires),				\
		.data = (_data),				\
	}
#endif

#define DEFINE_TIMER(_name, _function, _expires, _data)		\
	struct timer_list _name =				\
		TIMER_INITIALIZER(_function, _expires, _data)

#ifdef LINUX
void init_timer(struct timer_list *timer);
#else
/**
 * init_timer - initialize a timer.
 * @timer: the timer to be initialized
 *
 * init_timer() must be done to a timer prior calling *any* of the
 * other timer functions.
 */
static inline void init_timer(struct timer_list *timer)
{
	timer->next = timer->prev = NULL;
}

#endif
void init_timer_deferrable(struct timer_list *timer);

#ifdef CONFIG_DEBUG_OBJECTS_TIMERS
extern void init_timer_on_stack(struct timer_list *timer);
extern void destroy_timer_on_stack(struct timer_list *timer);
#else
static inline void destroy_timer_on_stack(struct timer_list *timer) { }
static inline void init_timer_on_stack(struct timer_list *timer)
{
	init_timer(timer);
}
#endif

#ifdef LINUX
static inline void setup_timer(struct timer_list * timer,
				void (*function)(unsigned long),
				unsigned long data)
{
	timer->function = function;
	timer->data = data;
	init_timer(timer);
}
#endif

static inline void setup_timer_on_stack(struct timer_list *timer,
					void (*function)(unsigned long),
					unsigned long data)
{
	timer->function = function;
	timer->data = data;
	init_timer_on_stack(timer);
}

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

#ifdef LINUX
extern void add_timer_on(struct timer_list *timer, int cpu);
extern int del_timer(struct timer_list * timer);
extern int __mod_timer(struct timer_list *timer, unsigned long expires);
extern int mod_timer(struct timer_list *timer, unsigned long expires);
#else
#define del_timer_sync(timer) usb_cancel_timer(timer)
#define del_timer(timer) usb_cancel_timer(timer)
#endif

/*
 * The jiffies value which is added to now, when there is no timer
 * in the timer wheel:
 */
#define NEXT_TIMER_MAX_DELTA	((1UL << 30) - 1)

/*
 * Return when the next timer-wheel timeout occurs (in absolute jiffies),
 * locks the timer base:
 */
extern unsigned long next_timer_interrupt(void);
/*
 * Return when the next timer-wheel timeout occurs (in absolute jiffies),
 * locks the timer base and does the comparison against the given
 * jiffie.
 */
extern unsigned long get_next_timer_interrupt(unsigned long now);

/*
 * Timer-statistics info:
 */
#ifdef CONFIG_TIMER_STATS

#define TIMER_STATS_FLAG_DEFERRABLE	0x1

extern void init_timer_stats(void);

extern void timer_stats_update_stats(void *timer, pid_t pid, void *startf,
				     void *timerf, char *comm,
				     unsigned int timer_flag);

extern void __timer_stats_timer_set_start_info(struct timer_list *timer,
					       void *addr);

static inline void timer_stats_timer_set_start_info(struct timer_list *timer)
{
	__timer_stats_timer_set_start_info(timer, __builtin_return_address(0));
}

static inline void timer_stats_timer_clear_start_info(struct timer_list *timer)
{
	timer->start_site = NULL;
}
#else
static inline void init_timer_stats(void)
{
}

static inline void timer_stats_timer_set_start_info(struct timer_list *timer)
{
}

static inline void timer_stats_timer_clear_start_info(struct timer_list *timer)
{
}
#endif

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
#ifdef LINUX
	__mod_timer(timer, timer->expires);
#else
	__mod_timer(timer, timer->expires);
//    usb_add_timer(timer);
#endif
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

static inline void setup_timer(struct timer_list * timer,
				void (*function)(unsigned long),
				unsigned long data)
{
	memset(timer, 0, sizeof *timer);
	timer->function = function;
	timer->data = data;
}
#ifdef LINUX
#ifdef CONFIG_SMP
  extern int try_to_del_timer_sync(struct timer_list *timer);
  extern int del_timer_sync(struct timer_list *timer);
#else
# define try_to_del_timer_sync(t)	del_timer(t)
# define del_timer_sync(t)		del_timer(t)
#endif
#endif

#define del_singleshot_timer_sync(t) del_timer_sync(t)

extern void init_timers(void);
extern void run_local_timers(void);
struct hrtimer;
extern enum hrtimer_restart it_real_fn(struct hrtimer *);

unsigned long __round_jiffies(unsigned long j, int cpu);
unsigned long __round_jiffies_relative(unsigned long j, int cpu);
unsigned long round_jiffies(unsigned long j);
unsigned long round_jiffies_relative(unsigned long j);

#endif
