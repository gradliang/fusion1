#define LOCAL_DEBUG_ENABLE 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global612.h"
#include "mpTrace.h"
#include "ndebug.h"


#include <linux/types.h>
#include <linux/list.h>
#include <linux/timer.h>


#include "hsm.h"
#include "wwan.h"
#include "ppp.h"

#if PPP_SUPPORT > 0

/*************************/
/*** LOCAL DEFINITIONS ***/
/*************************/

/* free memory if the pointer is valid and zero the pointer */
#ifndef SAFE_FREE
#define SAFE_FREE(x) do { if ((x) != NULL) {mpx_Free(x); (x)=NULL;} } while(0)
#endif

/************************/
/*** LOCAL DATA TYPES ***/
/************************/

enum {
    HEVENT_CONNECT	= 300,                  /* user requested PPP connection */
    HEVENT_DISCONNECT,
    HEVENT_MODEM_STATUS,
    HEVENT_PPP_STATUS,
    HEVENT_PPP_TIMEOUT,
};

typedef struct ppp_hsm_struct {
    Hsm super;                                  /* must be the 1st member field */
    State data_mode;
    State command_mode;
    State ppp_mode;
	_timer wwan_timer;
} WwanHsm;

typedef struct {
  Msg msg;
} WwanHsmEvt;

/***********************************/
/*** LOCAL FUNCTION DECLARATIONS ***/
/***********************************/

/******************************/
/*** PUBLIC DATA STRUCTURES ***/
/******************************/

static struct ppp_hsm_struct wwanCall;
Hsm *pppHsm = (Hsm *)&wwanCall;

U08 u08GsmCallTaskId;

extern U08 u08EDGETaskEvent_ID;

void SignalTest_Request(void);
void send_ppp_event(void *ctx, int errCode, void *arg);
static void ppp_timer_cb(WwanHsm *me);
void wwan_Task(void);
static void wwan_HsmOnEvent(int e);
void ppp_sm_init(void);
void ppp_sm_start(void);

int init3(void)
{
	S32 status;

	ppp_sm_init();

	status = mpx_TaskCreate(wwan_Task, 3, TASK_STACK_SIZE*4);
	MP_ASSERT(status > 0);
	u08GsmCallTaskId = status;

    mpx_TaskStartup(u08GsmCallTaskId);
    return 0;
fail:
    return -1;
}

void wwan_Task(void)
{
	DWORD dwEvents; 

	ppp_sm_start();

	while (1)
	{
		dwEvents = 0;
		EventWait(u08EDGETaskEvent_ID, 0xffff0000, OS_EVENT_OR, &dwEvents);
		MP_DEBUG("[WWAN_CONN] dwEvents = %x",dwEvents);
		if (dwEvents & PEVENT_PPP_CONNECT)
		{
			wwan_HsmOnEvent(HEVENT_CONNECT);
		}

		if (dwEvents & PEVENT_PPP_DISCONNECT)
		{
			wwan_HsmOnEvent(HEVENT_DISCONNECT);
		}

		if (dwEvents & PEVENT_MODEM_STATUS)
		{
			wwan_HsmOnEvent(HEVENT_MODEM_STATUS);
		}

		if (dwEvents & PEVENT_PPP_STATUS)
		{
			wwan_HsmOnEvent(HEVENT_PPP_STATUS);
		}

		if (dwEvents & PEVENT_TIMEOUT)
		{
			wwan_HsmOnEvent(HEVENT_PPP_TIMEOUT);
		}
	}
}

static Msg const *WWAN_DataMode(WwanHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
	int ret;
    switch (msg->evt)
    {
        case ENTRY_EVT:
            return 0;
		case START_EVT:
			mpDebugPrint("--> %s [START_EVT]", __func__);
			ret = pppOpen(0, send_ppp_event, NULL);
			if (ret >= 0)
				gsm_set_timer(&me->wwan_timer, 20000);
			return 0;
        case HEVENT_PPP_TIMEOUT:
			if (wwan_if.ppp_state)
				STATE_TRAN(me, &me->ppp_mode);
			return 0;
		case HEVENT_PPP_STATUS:
			if (wwan_if.ppp_state)
			{
				gsm_cancel_timer(&me->wwan_timer,&cancelled);
				STATE_TRAN(me, &me->ppp_mode);
			}
			else
			{
				// TODO
			}
			return 0;
        case HEVENT_CONNECT:
			ret = pppOpen(0, send_ppp_event, NULL);
			if (ret >= 0)
				gsm_set_timer(&me->wwan_timer, 20000);
            return 0;
        case HEVENT_DISCONNECT:
			pppClose(0);
//			setGprsAttach(0);               /* GPRS detach */
			STATE_TRAN(me, &me->command_mode);
            return 0;
        case HEVENT_MODEM_STATUS:
			if (!wwan_if.gprs_state)
			{
				gsm_cancel_timer(&me->wwan_timer,&cancelled);
				STATE_TRAN(me, &me->command_mode);
			}
            return 0;
    }

    return msg;
}

static Msg const *WWAN_CommandMode(WwanHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
        case ENTRY_EVT:
            return 0;
		case START_EVT:
			return 0;
        case HEVENT_CONNECT:
			gsm_set_timer(&me->wwan_timer, 20000);
			EventSet(u08EDGETaskEvent_ID, EEVENT_REQ_GPRS);
            return 0;
        case HEVENT_MODEM_STATUS:
			if (wwan_if.gprs_state)
			{
				gsm_cancel_timer(&me->wwan_timer,&cancelled);
				STATE_TRAN(me, &me->data_mode);
			}
            return 0;
        case HEVENT_PPP_TIMEOUT:
			if (wwan_if.gprs_state)
				STATE_TRAN(me, &me->data_mode);
			return 0;
    }

    return msg;
}

static Msg const *WWAN_PppMode(WwanHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
		case START_EVT:
			return 0;
        case HEVENT_PPP_STATUS:
			if (!wwan_if.ppp_state)
				STATE_TRAN(me, &me->command_mode);
            return 0;
        case HEVENT_MODEM_STATUS:
			if (!wwan_if.gprs_state)
				STATE_TRAN(me, &me->command_mode);
            return 0;
        case HEVENT_PPP_TIMEOUT:
			return 0;
        case HEVENT_CONNECT:
			return 0;
    }

    return msg;
}

static Msg const *WWAN_top(WwanHsm *me, Msg const *msg) 
{
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, &me->command_mode);
        return 0;
    case EXIT_EVT:
        return 0;
    } 

    return msg;
}

/*
 * early initialization of the PPP Call state machine
 */
void wwanConnCtor(Hsm *this) 
{
    WwanHsm *me = (WwanHsm *)this; 
    HsmCtor((Hsm *)me, "WwanConn", (EvtHndlr)WWAN_top);
    StateCtor(&me->command_mode, "WwanCommandMode", 
            &((Hsm *)me)->top, (EvtHndlr)WWAN_CommandMode);
    StateCtor(&me->data_mode, "WwanDataMode", 
            &((Hsm *)me)->top, (EvtHndlr)WWAN_DataMode);

    StateCtor(&me->ppp_mode, "WwanPppMode", 
            &me->data_mode, (EvtHndlr)WWAN_PppMode);

    init_timer(&me->wwan_timer);
    me->wwan_timer.data = (unsigned long) me;
	me->wwan_timer.function = (void *) &ppp_timer_cb;
}


static void wwan_HsmOnEvent(int e)
{
    WwanHsmEvt msg;
    memset(&msg, 0, sizeof msg);
    msg.msg.evt = e;
    HsmOnEvent((Hsm *)&wwanCall, (Msg *)&msg);
}

static void ppp_timer_cb(WwanHsm *me)
{
	EventSet(u08EDGETaskEvent_ID,PEVENT_TIMEOUT);
}

void ppp_sm_init(void)
{
	wwanConnCtor(pppHsm);
}

void ppp_sm_start(void)
{
	HsmOnStart(pppHsm);
}

/*
 * Async call
 */
void EdgeConnect_Request(void)
{
	EventSet(u08EDGETaskEvent_ID, PEVENT_PPP_CONNECT);
}


#endif

// vim: :noexpandtab:
