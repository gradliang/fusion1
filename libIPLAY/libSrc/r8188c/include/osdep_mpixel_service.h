
#ifndef __OSDEP_MPIXEL_SERVICE_2_H_
#define __OSDEP_MPIXEL_SERVICE_2_H_

#ifndef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 0
#endif

//#error dod1
#include <linux/version.h>
#include <linux/spinlock.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))	
#include <asm/semaphore.h>
#else	
#include <linux/semaphore.h>
#endif
#include <asm/atomic.h>		//refer for ATOMIC_T
#include <linux/list.h>			//refer for list_head
#include <linux/timer.h>
#include <linux/if_ether.h>		//for ETH_ALEN in rtw_mlme.h
#include <linux/workqueue.h>	//refer for _workitem
#include <linux/netdevice.h>	//refer for net_device
#include <linux/skbuff.h>		//refer for sk_buff
#include <linux/etherdevice.h>
#include <net/iw_handler.h>
#include <linux/if_arp.h>
#include <linux/proc_fs.h>	// Necessary because we use the proc fs
#ifdef CONFIG_USB_HCI	
#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))	
#include <linux/usb_ch9.h>
#else	
#include <linux/usb/ch9.h>
#endif
#endif

#ifdef PLATFORM_MPIXEL	
#include <asm/byteorder.h>
#else
#include <rtw_byteorder.h>
#endif
#include <basic_types.h>

#define _SUCCESS		1
#define _FAIL		0

#undef _TRUE
#define _TRUE		1

#ifdef CONFIG_USB_HCI	
typedef struct urb *  PURB;
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22))
#ifdef CONFIG_USB_SUSPEND
#define CONFIG_AUTOSUSPEND	1
#endif
#endif
#endif


typedef struct semaphore _sema;
typedef	spinlock_t	_lock;
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))	
typedef struct mutex 		_mutex;
//#else	
//typedef struct semaphore	_mutex;
//#endif

struct	__queue	
{		
	struct	list_head	queue;
	_lock	lock;	
};
typedef	struct sk_buff	_pkt;	
typedef unsigned char	_buffer;

typedef struct	__queue	_queue;
typedef struct	list_head	_list;
typedef unsigned long _irqL;
typedef	struct	net_device * _nic_hdl;
typedef pid_t		_thread_hdl_;
typedef int		thread_return;
typedef void*	thread_context;
//Atomic integer operations
#define ATOMIC_T atomic_t

struct rtw_netdev_priv_indicator 
{	
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device * rtw_alloc_etherdev(int sizeof_priv);
#define rtw_netdev_priv(netdev) ( ((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv )


/* Macros for handling unaligned memory accesses */
#define RTW_GET_BE16(a) ((u16) (((a)[0] << 8) | (a)[1]))

#define RTW_GET_LE16(a) ((u16) (((a)[1] << 8) | (a)[0]))
#define RTW_PUT_LE16(a, val)			\
	do {								\
		(a)[1] = ((u16) (val)) >> 8;		\
		(a)[0] = ((u16) (val)) & 0xff;	\
		} while (0)
#define RTW_GET_BE24(a) ((((u32) (a)[0]) << 16) | (((u32) (a)[1]) << 8) | 	((u32) (a)[2]))

typedef struct work_struct _workitem;

#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))

static inline _list *get_next(_list	*list)
{
	return list->next;
}

static inline _list *get_list_head(_queue	*queue)
{
	return (&(queue->queue));
}

static inline void _enter_critical(_lock *plock, _irqL *pirqL)
{
	spin_lock_irqsave(plock, *pirqL);
}

static inline void _exit_critical(_lock *plock, _irqL *pirqL)
{
	spin_unlock_irqrestore(plock, *pirqL);
}

static inline void _enter_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_lock(pmutex);
//#else
//	down(pmutex);
//#endif
}
static inline void _exit_critical_mutex(_mutex *pmutex, _irqL *pirqL)
{
//#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_unlock(pmutex);
//#else
//	up(pmutex);
//#endif
}

static inline void rtw_list_delete(_list *plist)
{	
	list_del_init(plist);
}

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
#define RTW_TIMER_HDL_ARGS void *FunctionContext
#define RTW_TIMER_HDL_NAME(name) rtw_##name##_timer_hdl
#define RTW_DECLARE_TIMER_HDL(name) void RTW_TIMER_HDL_NAME(name)(RTW_TIMER_HDL_ARGS)
#endif

static inline void _enter_critical_bh(_lock *plock, _irqL *pirqL)
{	
	spin_lock_bh(plock);
}

static inline void _exit_critical_bh(_lock *plock, _irqL *pirqL)
{
	spin_unlock_bh(plock);
}

static inline void _init_workitem(_workitem *pwork, void *pfunc, PVOID cntx)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))	
	INIT_WORK(pwork, pfunc);
#else	
	INIT_WORK(pwork, pfunc,pwork);
#endif
}

static inline void _set_workitem(_workitem *pwork)
{
	schedule_work(pwork);
}

extern u8*	_rtw_vmalloc(u32 sz);
extern u8*	_rtw_zvmalloc(u32 sz);
extern void	_rtw_vmfree(u8 *pbuf, u32 sz);
extern u8*	_rtw_zmalloc(u32 sz);
extern u8*	_rtw_malloc(u32 sz);
extern void	_rtw_mfree(u8 *pbuf, u32 sz);
#define rtw_vmalloc(sz)			_rtw_vmalloc((sz))
#define rtw_zvmalloc(sz)			_rtw_zvmalloc((sz))
#define rtw_vmfree(pbuf, sz)		_rtw_vmfree((pbuf), (sz))
#define rtw_zmalloc(sz)			_rtw_zmalloc((sz))
#define rtw_malloc(sz)			_rtw_malloc((sz))
#define rtw_mfree(pbuf, sz)		_rtw_mfree((pbuf), (sz))

#ifndef del_timer_sync
#define del_timer_sync(ptimer) usb_cancel_timer(ptimer)
#define del_timer(ptimer) usb_cancel_timer(ptimer) /* TODO */
#endif

extern void	rtw_list_delete(_list *plist);

extern void _rtw_init_sema(_sema *sema, int init_val, int create);
extern void _rtw_free_sema(_sema *sema);
extern void _rtw_up_sema(_sema *sema);
extern u32 _rtw_down_sema(_sema *sema);


static inline unsigned char _cancel_timer_ex(_timer *ptimer)
{
	return del_timer_sync(ptimer);
}

static inline u32 _RND4(u32 sz)
{
	u32	val;	
	val = ((sz >> 2) + ((sz & 3) ? 1: 0)) << 2;
	return val;
}

static inline u32 _RND8(u32 sz)
{
	u32	val;	
	val = ((sz >> 3) + ((sz & 7) ? 1: 0)) << 3;
	return val;
}

static inline u32 _RND128(u32 sz)
{
	u32	val;	
	val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;
	return val;
}

static inline void thread_enter(void *context)
{
#ifdef PLATFORM_LINUX	
	//struct net_device *pnetdev = (struct net_device *)context;	
	//daemonize("%s", pnetdev->name);	
	daemonize("%s", "RTKTHREAD");
	allow_signal(SIGTERM);
#endif
}

static inline void flush_signals_thread(void) 
{
#ifdef PLATFORM_LINUX
	if (signal_pending (current))
	{
		flush_signals(current);	
	}
#endif
}

#include "mpTrace.h"


#endif	//__OSDEP_MPIXEL_SERVICE_H_
