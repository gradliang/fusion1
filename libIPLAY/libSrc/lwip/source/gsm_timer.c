/** 
	This function registers the card as HOST/USER(WLAN)
	\param type of the card, function pointer
	\return returns pointer to the type requested for
*/
#define LOCAL_DEBUG_ENABLE 0

#include	"linux/types.h"
#include	"UtilTypeDef.h"
#include 	"lwip_incl.h"
#include    "taskid.h"
#include    "wlan_sys.h"
#include    "os_mp52x.h"
#include	"taskid.h"
#include	"net_device.h"
#include	"list_mpx.h"
#include	"linux/netdevice.h"
#include	"linux/completion.h"

#include	"gsm.h"

static _timer   gsm_timer_head;
static int      gsm_timer_semaphore;
int      gsm_timer_event;
static int gsm_timer_task_id;

void gsm_timer_task(void);
void gsm_init_timer(void)
{
    _timer *entry = &gsm_timer_head;
	int ret;

	gsm_timer_head.next = &gsm_timer_head;
	gsm_timer_head.prev = &gsm_timer_head;
	if (gsm_timer_semaphore == 0)
	{
		int ret = mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret > 0);
		if (ret > 0)
			gsm_timer_semaphore = ret;
	}
	MP_DEBUG("gsm_init_timer %x %x\n",entry->next,entry->prev);

	gsm_timer_event = mpx_EventCreate((OS_ATTR_FIFO | OS_ATTR_EVENT_CLEAR), 0);
	MP_ASSERT(gsm_timer_event > 0);

	if(!gsm_timer_task_id)
	{
		ret = mpx_TaskCreate(gsm_timer_task,WIFI_PRIORITY,0x1000);
		MP_ASSERT(ret > 0);
		if(ret > 0)
		{
			gsm_timer_task_id = ret;
			TaskStartup((BYTE)gsm_timer_task_id);
		}
	}
}

void gsm_add_timer(_timer *newtimer)
{
    _timer *entry = &gsm_timer_head;

    if (newtimer->next || newtimer->prev)
    {
        MP_ALERT("gsm_add_timer error\n");
		MP_ASSERT(0);
    }
    mpx_SemaphoreWait(gsm_timer_semaphore);
	//mpDebugPrint("%p %p %p",gsm_timer_head.next,&gsm_timer_head,entry);
    while (entry->next != &gsm_timer_head)
	{
		//mpDebugPrint("%p %p",entry->next,newtimer);
        entry = entry->next;
	}

    newtimer->next = entry->next;
    entry->next->prev = newtimer;
    entry->next = newtimer;
    newtimer->prev = entry;
	//mpDebugPrint("gsm_add_timer 3\n");
	mpx_SemaphoreRelease(gsm_timer_semaphore);
//	MP_DEBUG("<-- gsm_add_timer\n");
}

int gsm_mod_timer(_timer *ptimer, unsigned long expires)
{
//	mpDebugPrint("%s t=%u\n", __func__, expires);
	ptimer->expires = expires;
    if (ptimer->next)                         /* timer is pending */
        ;                                     /* do nothing */
    else
        gsm_add_timer(ptimer);
}

int gsm_set_timer(_timer *ptimer, unsigned long delay_time)
{
	mpDebugPrint("%s t=%u\n", __func__, delay_time);
	ptimer->expires = jiffies + (delay_time*HZ/1000);
    if (ptimer->next)                         /* timer is pending */
        ;                                     /* do nothing */
    else
        gsm_add_timer(ptimer);
}

int gsm_cancel_timer(_timer *ptimer)
{
    _timer *entry = &gsm_timer_head;
//	mpDebugPrint("--> gsm_cancel_timer\n");

    mpx_SemaphoreWait(gsm_timer_semaphore);
    while ((unsigned long)entry->next != (unsigned long)ptimer && (unsigned long)entry->next != (unsigned long)&gsm_timer_head)
        entry = entry->next;

    if (entry->next == &gsm_timer_head)     /* not found */
    {
        MP_DEBUG("gsm_cancel_timer not found\n");
        mpx_SemaphoreRelease(gsm_timer_semaphore);
        return 0;
    }

    entry->next = ptimer->next;
    ptimer->next->prev = entry;
    ptimer->next = ptimer->prev = NULL;
    mpx_SemaphoreRelease(gsm_timer_semaphore);
//	mpDebugPrint("<-- gsm_cancel_timer\n");
    return 1;
}

void gsm_timer_proc(void)
{
	DWORD tck;

    _timer *entry = &gsm_timer_head;
    _timer *prev_entry;

    mpx_SemaphoreWait(gsm_timer_semaphore);
//        mpDebugPrint("gsm_timer_proc: got sema,e=%p,n=%p", entry, entry->next);
    while (entry->next != &gsm_timer_head)
    {
        prev_entry = entry;
        entry = entry->next;
        if ((long)(entry->expires) - (long)(jiffies) < 0)
        {
//            mpDebugPrint("gsm_timer_proc: timer fires %x\n", entry->function);
			tck = mpx_SystemTickerGet();
            //mpDebugPrint("mpx_SystemTickerGet %x\n",tck );
			
            /* ----------  Remove this timer  ---------- */
            prev_entry->next = entry->next;
            entry->next->prev = prev_entry;
            entry->next = entry->prev = NULL;

//            mpDebugPrint("entry->expires %x (%p)\n",entry->expires, entry->function);
            if (entry->function)
                (*entry->function)(entry->data);

            entry = prev_entry;
        }
    }
    mpx_SemaphoreRelease(gsm_timer_semaphore);
//	mpDebugPrint("<-- gsm_timer_proc");
}

/* 
 * Similar to gsm_add_timer but without semaphore wait/release
 */
void gsm_add_timer2(_timer *newtimer)
{
    _timer *entry = &gsm_timer_head;
//	mpDebugPrint("--> gsm_add_timer2\n");

    if (newtimer->next || newtimer->prev)
    {
        mpDebugPrint("gsm_add_timer error\n");
		MP_ASSERT(0);
    }
    while (entry->next != &gsm_timer_head)
	{
		//mpDebugPrint("%p %p",entry->next,newtimer);
        entry = entry->next;
	}

    newtimer->next = entry->next;
    entry->next->prev = newtimer;
    entry->next = newtimer;
    newtimer->prev = entry;
//	mpDebugPrint("<-- gsm_add_timer2\n");
}

//************************************
// USB Timer task
//************************************
void gsm_timer_task(void)
{
	DWORD gsmEvent;  	
	mpDebugPrint("gsm_timer_task\n");

	while(1)
	{
        gsmEvent = 0;  	
        EventWait(gsm_timer_event, EVENT_GSM_TIMER, OS_EVENT_OR, &gsmEvent);
        if (gsmEvent & EVENT_GSM_TIMER)
        {
            gsm_timer_proc();
        }

        TaskYield();
 	}
}
		
		
