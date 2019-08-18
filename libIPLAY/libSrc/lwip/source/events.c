
#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include <tevent.h>
#include <tevent_internal.h>
#include <ttime.h>

#include "ppp_opt.h" 	// jeffery temp add, 
#include "wwan.h"

extern U08 u08PPPUART_ID;
#if PPP_SUPPORT > 0

void send_event(struct event_context *ev)
{
	EventSet(ev->event, LEVENT_AT_CMD_DONE);
}


/*
 * Return if there's something in the queue
 */
bool event_add_to_select_args(struct tevent_context *ev,
			      const struct timeval *now,
			      unsigned long *write_fds,
			      struct timeval *timeout)
{
	struct timeval diff;
	bool ret = false;
	struct tevent_fd *fde;

	for (fde = ev->fd_events; fde; fde = fde->next) {
		if (fde->flags & WWAN_INTERFACE_EXIT) {
			*write_fds |= LEVENT_USER_INPUT;
			ret = true;
		}
		if (fde->flags & WWAN_INTERFACE_PPP) {
			*write_fds |= LEVENT_PPP_STATUS;
			ret = true;
		}
		if (fde->flags & WWAN_INTERFACE_GPRS) {
			*write_fds |= LEVENT_GPRS_STATUS;
			ret = true;
		}
	}

	if (*write_fds == 0)
	{
		*write_fds = LEVENT_AT_CMD_DONE;
		ret = true;
	}

	if (ev->timer_events == NULL) {
		return ret;
	}

	diff = timeval_until(now, &ev->timer_events->next_event);
	*timeout = timeval_min(timeout, &diff);

	return true;
}

bool run_events(struct event_context *ev, uint16_t if_state)
{
	struct timeval now;
	struct tevent_fd *fde;

//	if (if_state)
//		mpDebugPrint("[EVENTS]:%x", if_state);

	GetTimeOfDay(&now);

	if ((ev->timer_events != NULL)
	    && (timeval_compare(&now, &ev->timer_events->next_event) >= 0)) {

		ev->timer_events->handler(ev, ev->timer_events, now,
					  ev->timer_events->private_data);
		mpDebugPrint("[EVENT]: timeout");
		return true;
	}

	if (if_state == 0)
		return false;

	for (fde = ev->fd_events; fde; fde = fde->next) {

		if (if_state & fde->flags) {
			fde->handler(ev, fde, fde->private_data);
			return true;
		}
	}

	return false;
}

extern int dod6;
int dod3_save;
void wwan_get_state(uint16_t *flags);
void wwan_get_state2(uint16_t *flags);
static int s3_event_loop_once(struct tevent_context *ev)
{
	struct timeval now, to;
	int ret;
	uint32_t evts;                              /* events */
	uint32_t dwEvent;
	uint16_t flags = 0;

	MP_DEBUG("--> %s", __func__);

	to.tv_sec = 9999;	/* Max timeout */
	to.tv_usec = 0;

	if (run_events(ev, 0)) {
        BREAK_POINT();
		return 0;
	}

	evts = 0;

	GetTimeOfDay(&now);

	if (!event_add_to_select_args(ev, &now, &evts, &to)) {
		return -1;
	}

	MP_DEBUG("[EVENT] evts=%x", evts);
	ret = EventWaitWithTO(ev->event, evts, OS_EVENT_OR, &dwEvent,1000);

//	BREAK_POINT();

	if (dod6)
		mpDebugPrint("[EVENT] ret=%d", ret);

	if (ret == OS_STATUS_OK)
	{
		if (dwEvent & LEVENT_USER_INPUT)
			wwan_get_state2(&flags);
	}

//	if (ret == OS_STATUS_OK)
		wwan_get_state(&flags);

	if (dod6)
		mpDebugPrint("[EVENT] flags=%d", flags);
	run_events(ev, flags);
	return 0;
}

static const struct tevent_ops s3_event_ops = {
	.add_timer		= tevent_common_add_timer,
	.loop_once		= s3_event_loop_once,
};

struct tevent_context *event_context_init(void)
{
	struct tevent_context *ev;
//	mpDebugPrint("--> %s", __func__);
	ev = mpx_Zalloc(sizeof(struct event_context));
	if (ev)
	{
		ev->event = u08PPPUART_ID;
		ev->ops = &s3_event_ops;

	}

	return ev;
}
#endif
