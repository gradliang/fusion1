
#ifndef __OSDEP_MPIXEL_SERVICE_H_
#define __OSDEP_MPIXEL_SERVICE_H_

//#include <drv_conf.h>
//#include <basic_types.h>

//#include <os.h>
 
//#include "os_defs.h"
#include <linux/list.h>
//#include "..\..\os\include\osconst.h"
//#include "..\..\heaputil\include\mem.h"

//#include <net_packet.h>

#ifndef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 1
#endif
#include "global612.h"
#include "mpTrace.h"
#include <linux/timer.h>
extern u32 timernum ;

//#include "net_nic.h"
//#define kfree_skb(n) mem_free(n)


typedef s32 _sema;
typedef	_sema _lock;
typedef _sema _rwlock;
	

struct _timer_obj{
    struct _timer_obj *next, *prev;
    unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

#if 0
struct timer_list{
    struct _timer_obj *next, *prev;
    unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};
#endif

//typedef 	struct _timer_obj _timer;
typedef 	struct timer_list _timer;
typedef unsigned char bool;

#define false   0
#define true    1
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define BUG()
#define BUILD_BUG_ON(a)
#define BUG_ON(condition) 
#define WARN_ON(condition)  (condition)
#define WARN_ON_ONCE(condition)	
#define scnprintf   snprintf
#define __constant_htons(a)   htons(a)
#define module_init(x)	
#define module_exit(x)
#define module_exit(x)
#define module_put(x)

//#define free_netdev(x)  kfree(x)
#define printk_ratelimit() 0

#define subsys_initcall(fn)

#define GFP_ATOMIC  0
#define GFP_KERNEL  0
#define GFP_NOIO  0

#define pr_debug(fmt,arg...) 
#define dev_dbg(dev, format, arg...)		
#define might_sleep() 
#define fastcall
#define __init
#define __exit
#define kzalloc(size,f)   mpx_Zalloc(size)
#define kmemdup(s,l,gfp)   memdup(s,l)

//#define clear_bit(nr,p)			*(p) &= ~(1 <<(nr))

#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define __force	
#define __aligned(x)			__attribute__((aligned(x)))
#define USEC_PER_SEC	1000000L

struct neighbour
{

};

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
static unsigned int inline jiffies_to_msecs(const unsigned long j)
{
	return (1000L / HZ) * j;
}
static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
	/*
	 * HZ is equal to or smaller than 1000, and 1000 is a nice
	 * round multiple of HZ, divide with the factor between them,
	 * but round upwards:
	 */
	return (m + (1000 / HZ) - 1) / (1000 / HZ);
}

typedef u32 dma_addr_t;

#define local_irq_save(x) 
#define local_irq_restore(x) 

struct	__queue	{
    struct	list_head	queue;	
    _lock	lock;
};

typedef	 struct sk_buff	_pkt;
typedef struct	__queue	_queue;
typedef struct	list_head	_list;
typedef	int	_OS_STATUS;
typedef u32	_irqL;
typedef	struct	net_device * _nic_hdl;

//typedef pid_t		_thread_hdl_;
typedef SDWORD	_thread_hdl_;
	

static inline _list *get_next(_list	*list)
{
	return list->next;
}	





static inline _list	*get_list_head(_queue	*queue)
{
	return (&(queue->queue));
}

	
#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))	

#define tasklet_disable(t)
//#define tasklet_init(t, rx, m)
//#define tasklet_kill(t)
//#define tasklet_enable(t)
//#define tasklet_schedule(t)

#define smp_mb()

#define EXPORT_SYMBOL(t)
#define EXPORT_SYMBOL_GPL(t)
        
#define ilog2(n)				1

#define ____cacheline_aligned

/*
 * A trick to suppress uninitialized variable warning without generating any
 * code
 */
#define uninitialized_var(x) x = x

static void inline _enter_critical(_lock *plock, _irqL *pirqL)
{
	s32 ret;
       u8 id = (u8)(*plock);
	   
       //ret = SemaphoreWait(id);
    //   mpDebugPrint("_enter_critical\n");
	
}
static void inline _exit_critical(_lock *plock, _irqL *pirqL)
{
	u8 id = (u8)(*plock);
		
//	SemaphoreRelease(id);
     //  mpDebugPrint("_exit_critical\n");
}


static void inline _enter_hwio_critical(_rwlock *prwlock, _irqL *pirqL)
{
	s32 ret;
       u8 id = (u8)(*prwlock);
	   
    //   ret = SemaphoreWait(id);
    //    mpDebugPrint("_enter_hwio_critical\n");
	
}
static void inline _exit_hwio_critical(_rwlock *prwlock, _irqL *pirqL)
{		
	u8 id = (u8)(*prwlock);
		
//	SemaphoreRelease(id);
//	mpDebugPrint("_exit_hwio_critical\n");
}

void usb_add_timer2(_timer *newtimer);

int usb_cancel_timer(_timer *ptimer);
void usb_add_timer(_timer *newtimer);
void usb_init_timer(void);
void usb_timer_proc(void);
static inline void _init_timer(_timer *ptimer,_nic_hdl padapter,void *pfunc, void* cntx)
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

static __inline int timer_pending(_timer *ptimer)
{
    if (ptimer->next)                         /* timer is pending */
        return TRUE;
    else
        return FALSE;
}

static __inline void add_timer(_timer *ptimer)
{
	MP_ASSERT(timer_pending(ptimer) == 0);
    usb_add_timer(ptimer);
}

static __inline void add_timer2(_timer *ptimer)
{
	MP_ASSERT(timer_pending(ptimer) == 0);
    usb_add_timer2(ptimer);
}

static inline void setup_timer(struct timer_list * timer,
				void (*function)(unsigned long),
				unsigned long data)
{
	memset(timer, 0, sizeof *timer);
	timer->function = function;
	timer->data = data;
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

#ifndef del_timer_sync
#define del_timer_sync(ptimer) usb_cancel_timer(ptimer)
#define del_timer(ptimer) usb_cancel_timer(ptimer) /* TODO */
#endif

//AAADDDD
#define get_seconds() GetSysTime()/1000//GetCurSecond()

#ifndef le16_to_cpu
#define le16_to_cpu(x) \
({ \
	__u16 __x = (x); \
	((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
})
#define le32_to_cpu(x)  \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})
#define le64_to_cpu(x) \
({ \
	__u64 __x = (x); \
	((__u64)( \
		(__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
})
#define cpu_to_le16(x) \
({ \
	__u16 __x = (x); \
	((__u16)( \
		(((__u16)(__x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(__x) & (__u16)0xff00U) >> 8) )); \
})
#define cpu_to_le32(x)  \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(__x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(__x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(__x) & (__u32)0xff000000UL) >> 24) )); \
})
#define cpu_to_le64(x) \
({ \
	__u64 __x = (x); \
	((__u64)( \
		(__u64)(((__u64)(__x) & (__u64)0x00000000000000ffULL) << 56) | \
		(__u64)(((__u64)(__x) & (__u64)0x000000000000ff00ULL) << 40) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000000000ff0000ULL) << 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00000000ff000000ULL) <<  8) | \
	  (__u64)(((__u64)(__x) & (__u64)0x000000ff00000000ULL) >>  8) | \
		(__u64)(((__u64)(__x) & (__u64)0x0000ff0000000000ULL) >> 24) | \
		(__u64)(((__u64)(__x) & (__u64)0x00ff000000000000ULL) >> 40) | \
		(__u64)(((__u64)(__x) & (__u64)0xff00000000000000ULL) >> 56) )); \
})

#define be16_to_cpu(x) ((__u16)(x))
#define cpu_to_be16(x)	be16_to_cpu(x)

#define le16_to_cpup(x) \
    le16_to_cpu(*x)
#define be16_to_cpup(x) \
    be16_to_cpu(*x)
#define le32_to_cpup(x) \
    le32_to_cpu(*x)
#endif

static inline void put_unaligned_le16(u16 val, void *p)
{
	__le16 v;
	v = cpu_to_le16(val);
	memcpy(p, &v, sizeof v);

}
static inline void put_unaligned_le32(u32 val, void *p)
{
	__le32 v;
	v = cpu_to_le32(val);
	memcpy(p, &v, sizeof v);
}

static inline void put_unaligned_be16(u16 val, void *p)
{
	__be16 v;
	v = cpu_to_be16(val);
	memcpy(p, &v, sizeof v);

}
#define __put_unaligned_be(val, ptr) ({					\
	void *__gu_p = (ptr);						\
	switch (sizeof(*(ptr))) {					\
	case 1:								\
		*(u8 *)__gu_p = (__force u8)(val);			\
		break;							\
	case 2:								\
		put_unaligned_be16((__force u16)(val), __gu_p);		\
		break;							\
	case 4:								\
		put_unaligned_be32((__force u32)(val), __gu_p);		\
		break;							\
	case 8:								\
		put_unaligned_be64((__force u64)(val), __gu_p);		\
		break;							\
	default:							\
		__bad_unaligned_access_size();				\
		break;							\
	}								\
	(void)0; })
# define put_unaligned	__put_unaligned_be
static inline u16 get_unaligned_le16(const void *p)
{
	__le16 val;
	memcpy(&val, p, sizeof val);

	return le16_to_cpup(&val);
}
static inline u16 get_unaligned_be16(const void *p)
{
	__be16 val;
	memcpy(&val, p, sizeof val);

	return be16_to_cpup(&val);
}

static inline u32 get_unaligned_be32(const void *p)
{
	__be32 val;
	memcpy(&val, p, sizeof val);

	return be32_to_cpup(&val);
}
static inline u32 get_unaligned_le32(const void *p)
{
	__le32 val;
	memcpy(&val, p, sizeof val);

	return le32_to_cpup(&val);
}

#define __get_unaligned_le(ptr) ((__force typeof(*(ptr)))({			\
	__builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),			\
	__builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_le16((ptr)),	\
	__builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_le32((ptr)),	\
	__bad_unaligned_access_size())));					\
	}))

#define __get_unaligned_be(ptr) (( typeof(*(ptr)))({			\
	__builtin_choose_expr(sizeof(*(ptr)) == 1, *(ptr),			\
	__builtin_choose_expr(sizeof(*(ptr)) == 2, get_unaligned_be16((ptr)),	\
	__builtin_choose_expr(sizeof(*(ptr)) == 4, get_unaligned_be32((ptr)),	\
	__bad_unaligned_access_size())));					\
	}))

#define get_unaligned(ptr) __get_unaligned_be(ptr)

static __inline__  __u16 ___swab16(__u16 x)
{
	return x<<8 | x>>8;
}
#define ___constant_swab16(x) \
	((__u16)( \
		(((__u16)(x) & (__u16)0x00ffU) << 8) | \
		(((__u16)(x) & (__u16)0xff00U) >> 8) ))
#define swab16(x) \
(__builtin_constant_p((__u16)(x)) ? \
 ___constant_swab16((x)) : \
 ___swab16((x)))

#define synchronize_rcu()

void *mm_zalloc(size_t size);
/**
 * kcalloc - allocate memory for an array. The memory is set to zero.
 * @n: number of elements.
 * @size: element size.
 * @flags: Not used
 */
static inline void *kcalloc(size_t n, size_t size, gfp_t flags)
{
	return mm_zalloc(n * size);
}

extern void free_sema(_sema *sema);
const char * wpa_ssid_string(char *ssid_txt, u8 *ssid, size_t ssid_len);

//extern void sleep_schedulable(int ms);

#include "ndebug.h"

#endif











