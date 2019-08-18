#ifndef __GSM__H__
#define __GSM__H__

#include "bitsDefine.h"
#include "global612.h"
#include "mpTrace.h"
#include "Net_api.h"
#include "BitsDefine.h"

struct Hsm;

/* GSM call sm events */
enum {
    HEVENT_PPP_REQ 		= 200,                  /* user requested PPP connection */
    HEVENT_TIMEOUT 	    = 201,
    HEVENT_GPRS_REQ,                  			/* user requested GPRS data call */
    HEVENT_SMS_REQ,                  			/* Check SMS */
    HEVENT_GPRS_DISCONNECT,                  	/* disconnect GPRS */
    HEVENT_GSM_REGISTER_STATUS,                 /* gsm register status change */
};


/* ----------  end of enum FILE  ---------- */
#define TEVENT_PPP_CONNECTED	0x0001
#define TEVENT_GPRS_CONNECTED	0x0002

/* ----------  end of enum FILE  ---------- */
#define EVENT_GSM_TIMER	BIT0

extern int      gsm_timer_event;


#endif

