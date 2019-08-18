/*
 * =====================================================================================
 *
 *       Filename:  gsm_sm.c
 *
 *    Description:  Implementation of GSM Call State Machine
 *
 * =====================================================================================
 */

#define LOCAL_DEBUG_ENABLE 1

#include <string.h>

#include <linux/types.h>
#include "global612.h"
#include "mpTrace.h"
#include "../../STD_DPF/main/include/ui_timer.h"
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "mpapi.h"
#include "netware.h"
#include "taskid.h"
#include "socket.h"
#include "net_api.h"
//#include "os_defs.h"

#include "hsm.h"
#include "gsm.h"
#include "ppp.h"

#include "tevent_util.h"

#if PPP_SUPPORT > 0

#define GSM_REGISTER_INTERVAL (5 * 60 * HZ)     /* 5 min */

enum {
	GSM_NO_REQ,
	GSM_GPRS_REQ,
	GSM_SMS_REQ,
};

typedef struct gsm_hsm_struct {
    Hsm super;                                  /* must be the 1st member field */
    State gsm_registered;
    State gsm_unregistered;
    State gprs_attached;
    State gprs_detached;
    State pdp_active;
    State pdp_inactive;
    State connected;
	_timer gsm_timer;
	uint8_t gsm_event;
	short 	n_retries;
	short 	n_backoffs;
	unsigned long 	start;
	int 	gsm_user_req;
} GsmHsm;

typedef struct {
  Msg msg;
} GsmHsmEvt;

static struct gsm_hsm_struct gsmCall;

Hsm *gsmHsm = (Hsm *)&gsmCall;

/* retry interval - in seconds */
static int gsm_RetryIntervals[] = { 15, 20, 30 };

/* retry interval - in seconds */
static int gprs_RetryIntervals[] = { 15, 20, 30 };

/* backoff interval - in minutes */
static int gprs_backoff[] = { 5, 5, 5, 5, 10, 30, 60, 60 };

/* retry interval (ATD*99)- in seconds */
static int atd_retryInterval[] = { 30, 30, 30 };

/* retry interval (PDP context activation)- in minutes */
static int pdp_retryInterval[] = { 1, 1, 1, 1, 10, 15, 30, 60, 60 };

#define SEC_TO_JIFFIES(sec)	(sec * HZ)
#define MIN_TO_JIFFIES(min)	(min * 60 * HZ)
#define MIN_TO_MSEC(min)	(min * 60 * 1000)
#define SEC_TO_MSEC(sec)	(sec * 1000)

bool gsm_connected;

extern U08 u08EDGETaskEvent_ID;
extern int Signal_Test_NumRetries;

bool getcreg();
BOOL IsRegisterStateOK(void);
bool edge_pdp_context();
int edge_pdp_context2();
void pdp_error_handler(GsmHsm *me, const int err);
bool SignalTest(bool show);
BOOL IsSignalOK(void);
static void gsm_timer_cb(GsmHsm *me);
bool setGprsAttach(int attach);
void check_SMS_Request(void);
void EdgeStatus_Request(void);

/* 
 * GSM registered
 */
static Msg const *GSM_Registered(GsmHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
        case ENTRY_EVT:
            return 0;
		case START_EVT:
			if (me->gsm_user_req == GSM_GPRS_REQ)
			{
				me->gsm_user_req = GSM_NO_REQ;
			STATE_START(me, &me->gprs_detached);
			}
			return 0;
        case HEVENT_GPRS_REQ:
			gsm_cancel_timer(&me->gsm_timer,&cancelled);
			if (getcreg())
			{
				if (IsRegisterStateOK())
				{
					STATE_TRAN(me, &me->gprs_detached);
					return 0;
				}
			}
			STATE_TRAN(me, &me->gsm_unregistered);
            return 0;
        case HEVENT_SMS_REQ:
			gsm_cancel_timer(&me->gsm_timer,&cancelled);
			if (getcreg())
			{
				if (IsRegisterStateOK())
				{
					STATE_TRAN(me, &me->gprs_detached);
					return 0;
				}
			}
			STATE_TRAN(me, &me->gsm_unregistered);
            return 0;
        case HEVENT_GSM_REGISTER_STATUS:
			if (!IsRegisterStateOK())
				STATE_TRAN(me, &me->gsm_unregistered);
            return 0;
    }

    return msg;
}

static Msg const *Gsm_Unregistered(GsmHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
		case START_EVT:
			me->n_retries = 0;
#if 0
			if (getcreg())
			{
				if (IsRegisterStateOK())
				{
					me->gsm_user_req = GSM_NO_REQ;
					STATE_TRAN(me, &me->gsm_registered);
					return 0;
				}
			}

			gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(gsm_RetryIntervals[me->n_retries]));
			me->start = jiffies;
			me->n_retries++;
#endif
			return 0;
        case HEVENT_TIMEOUT:
			if (getcreg())
			{
				if (IsRegisterStateOK())
				{
					STATE_TRAN(me, &me->gsm_registered);
					return 0;
				}
			}

			if (time_after(jiffies,
			       me->start + GSM_REGISTER_INTERVAL)) {
				/* ----------  give up totally  ---------- */
			}
			else
			{
				if (me->n_retries >= ARRAY_SIZE(gsm_RetryIntervals))
					me->n_retries = 0;
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(gsm_RetryIntervals[me->n_retries]));
				me->n_retries++;
			}
			return 0;
        case HEVENT_GPRS_REQ:
        case HEVENT_SMS_REQ:
			if (getcreg())
			{
				if (IsRegisterStateOK())
				{
					if (msg->evt == HEVENT_GPRS_REQ)
						me->gsm_user_req = GSM_GPRS_REQ;
					else
						me->gsm_user_req = GSM_SMS_REQ;
					gsm_cancel_timer(&me->gsm_timer,&cancelled);
					STATE_TRAN(me, &me->gsm_registered);
					return 0;
				}
			}

			if (!timer_pending(&me->gsm_timer))
			{
				me->n_retries = 0;
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(gsm_RetryIntervals[me->n_retries]));
				me->start = jiffies;
				me->n_retries++;
			}
            return 0;
        case HEVENT_GSM_REGISTER_STATUS:
			if (IsRegisterStateOK())
				STATE_TRAN(me, &me->gsm_registered);
            return 0;
    }

    return msg;
}

/* 
 * PDP Context is inactive
 */
static Msg const *GSM_PdpInactive(GsmHsm *me, Msg const *msg) 
{
	int err;
    switch (msg->evt)
    {
		case START_EVT:
			me->n_retries = 0;
			if (err = edge_pdp_context2())
			{
				pdp_error_handler(me, err);
			}
			else
				STATE_TRAN(me, &me->connected);
			return 0;
        case ENTRY_EVT:
            return 0;
        case HEVENT_TIMEOUT:
			if (!(err = edge_pdp_context2()))
				STATE_TRAN(me, &me->connected);
			else if (me->n_retries < ARRAY_SIZE(pdp_retryInterval))
			{
				pdp_error_handler(me, err);
			}
			else
			{
				/* give up */
				setGprsAttach(0);               /* GPRS detach */
				STATE_TRAN(me, &me->gsm_registered);
			}
			return 0;
    }

    return msg;
}

/* 
 * g_bXpgStatus = XPG_MODE_NET_FUNC
 */
static Msg const *GSM_GprsAttached(GsmHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
		case START_EVT:
			me->n_retries = 0;
			if (edge_pdp_context())
				STATE_TRAN(me, &me->connected);
			else
			{
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(atd_retryInterval[me->n_retries]));
				me->n_retries++;
			}
			return 0;
        case ENTRY_EVT:
            return 0;
        case EXIT_EVT:
            return 0;
        case HEVENT_TIMEOUT:
			if (edge_pdp_context())
				STATE_TRAN(me, &me->connected);
			else if (me->n_retries < ARRAY_SIZE(atd_retryInterval))
			{
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(atd_retryInterval[me->n_retries]));
				me->n_retries++;
			}
			else 
			{
				STATE_TRAN(me, &me->pdp_inactive);
			}
			return 0;
        case HEVENT_GPRS_DISCONNECT:
			gsm_cancel_timer(&me->gsm_timer,&cancelled);
			STATE_TRAN(me, &me->gsm_registered);
            return 0;
    }

    return msg;
}

static Msg const *GSM_GprsDetached(GsmHsm *me, Msg const *msg) 
{
	uint8_t cancelled;
    switch (msg->evt)
    {
		case START_EVT:
			me->n_retries = 0;
			me->n_backoffs = 0;
			if (setGprsAttach(1))
				STATE_TRAN(me, &me->gprs_attached);
			else
			{
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(gprs_RetryIntervals[me->n_retries]));
				me->n_retries++;
			}
			return 0;
        case HEVENT_TIMEOUT:
			if (setGprsAttach(1))
				STATE_TRAN(me, &me->gprs_attached);
			else if (me->n_retries < ARRAY_SIZE(gprs_RetryIntervals))
			{
				gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(gprs_RetryIntervals[me->n_retries]));
				me->n_retries++;
			}
			else if (me->n_backoffs < ARRAY_SIZE(gprs_backoff))
			{
				gsm_set_timer(&me->gsm_timer, MIN_TO_MSEC(gprs_backoff[me->n_backoffs]));
				me->n_retries = 0;
				me->n_backoffs++;
			}
			else
			{
				/* give up: do nothing */
			}
			return 0;
        case HEVENT_GPRS_DISCONNECT:
			gsm_cancel_timer(&me->gsm_timer,&cancelled);
			STATE_TRAN(me, &me->gsm_registered);
            return 0;
    }

    return msg;
}

/* 
 * g_bXpgStatus = XPG_MODE_NET_FUNC
 */
static Msg const *Gsm_Connected(GsmHsm *me, Msg const *msg) 
{
	DWORD index;	
    switch (msg->evt)
    {
		case START_EVT:
			gsm_connected = true;
			send_gprs_event(1);
			return 0;
        case EXIT_EVT:
			gsm_connected = false;
			send_gprs_event(0);

			Signal_Test_NumRetries = 3;
			SignalTest_Request();        /* schedule a signal strength read */
			//AddTimerProc(5000, check_SMS_Request); /* schedule a sms check */
			RemoveTimerProc(check_SMS_Request);
			AddTimerProc(2*60*60*1000 ,check_SMS_Request);	//grad, change 5s to 2 hours. 
            return 0;
        case HEVENT_TIMEOUT:
			return 0;
        case HEVENT_GPRS_REQ:
			/* ignore */
            return 0;
        case HEVENT_SMS_REQ:
			/* ignore */
            return 0;
    }

    return msg;
}


static Msg const *Gsm_top(GsmHsm *me, Msg const *msg) 
{
    switch (msg->evt) {
    case START_EVT:
        STATE_START(me, &me->gsm_unregistered);
        return 0;
    case EXIT_EVT:
        return 0;
    } 

    return msg;
}

/*
 * early initialization of the GSM CALL state machine
 */
void gsmCtor(Hsm *this) 
{
    GsmHsm *me = (GsmHsm *)this; 
    HsmCtor((Hsm *)me, "GsmCall", (EvtHndlr)Gsm_top);
    StateCtor(&me->gsm_registered, "GsmRegister", 
            &((Hsm *)me)->top, (EvtHndlr)GSM_Registered);
    StateCtor(&me->gsm_unregistered, "GsmUnregister", 
            &((Hsm *)me)->top, (EvtHndlr)Gsm_Unregistered);

    StateCtor(&me->gprs_attached, "GprsAttach", 
            &me->gsm_registered, (EvtHndlr)GSM_GprsAttached);
    StateCtor(&me->gprs_detached, "GprsDetach", 
            &me->gsm_registered, (EvtHndlr)GSM_GprsDetached);

    StateCtor(&me->pdp_inactive, "PdpInactive", 
            &me->gprs_attached, (EvtHndlr)GSM_PdpInactive);

    StateCtor(&me->connected, "Connected", 
            &me->gprs_attached, (EvtHndlr)Gsm_Connected);

    init_timer(&me->gsm_timer);
    me->gsm_timer.data = (unsigned long) me;
	me->gsm_timer.function = (void *) &gsm_timer_cb;
}


void gsm_HsmOnEvent(int e)
{
    GsmHsmEvt msg;
    memset(&msg, 0, sizeof msg);
    msg.msg.evt = e;
    HsmOnEvent((Hsm *)&gsmCall, (Msg *)&msg);
}


static void gsm_timer_cb(GsmHsm *me)
{
	EventSet(u08EDGETaskEvent_ID,EEVENT_TIMEOUT);
}

void pdp_error_handler(GsmHsm *me, const int err)
{
	short i;
	MP_DEBUG("+CME ERROR: %d",err);
	log_printf(1, "PDP +CME ERROR: %d", err);
	switch (err)
	{
		case 132:
			mpDebugPrint("132: Service option not supported");
			/* fall thru */
		case 133:
			if (err == 133)
				mpDebugPrint("133: Requested service option not subscribed");
			/* fall thru */
		case 149:
			if (err == 149)
				mpDebugPrint("149: User authentication failed");

			/* retry no sooner than 15 min */

			i = me->n_retries;
			if (pdp_retryInterval[i] < 15)
				for (; i< ARRAY_SIZE(pdp_retryInterval); i++)
					if (pdp_retryInterval[i] >= 15)
					{
						me->n_retries = i;
						break;
					}

			/* retry */
			gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(pdp_retryInterval[me->n_retries]));
			me->n_retries++;
			break;
		case 148:
			mpDebugPrint("148: Unspecified GPRS error");
			/* retry */
			gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(pdp_retryInterval[me->n_retries]));
			me->n_retries++;
			break;
		case 134:
			mpDebugPrint("134: Service option temporarily out of order");
			/* fall thru */
		default:
			/* retry no sooner than 5 min */
			i = me->n_retries;
			if (pdp_retryInterval[i] < 5)
				for (; i< ARRAY_SIZE(pdp_retryInterval); i++)
					if (pdp_retryInterval[i] >= 5)
					{
						me->n_retries = i;
						break;
					}

			/* retry */
			gsm_set_timer(&me->gsm_timer, SEC_TO_MSEC(pdp_retryInterval[me->n_retries]));
			me->n_retries++;
			break;
	}
}

void gsm_sm_init(void)
{
	gsmCtor(gsmHsm);
}

void gsm_sm_start(void)
{
	HsmOnStart(gsmHsm);
}

#endif

// vim: :noexpandtab:
