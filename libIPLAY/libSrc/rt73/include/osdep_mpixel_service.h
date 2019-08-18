
#ifndef __OSDEP_MPIXEL_SERVICE_H_
#define __OSDEP_MPIXEL_SERVICE_H_

//#include <drv_conf.h>
//#include <basic_types.h>

//#include <os.h>
 
#include <linux/list.h>
//#include "..\..\os\include\osconst.h"
//#include "..\..\heaputil\include\mem.h"

//#include <net_packet.h>

#ifndef LOCAL_DEBUG_ENABLE
#define LOCAL_DEBUG_ENABLE 1
#endif
#include "global612.h"
#include "mpTrace.h"
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

typedef 	struct _timer_obj _timer;

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

static inline void list_delete(_list *plist)
{
	list_del(plist);	
}
	
#define LIST_CONTAINOR(ptr, type, member) \
        ((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))	

        
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

#define del_timer_sync(ptimer) usb_cancel_timer(ptimer)

extern void free_sema(_sema *sema);
const char * wpa_ssid_string(char *ssid_txt, u8 *ssid, size_t ssid_len);

//extern void sleep_schedulable(int ms);



#endif











