#ifndef __WWAN__H__
#define __WWAN__H__

#include "bitsDefine.h"
#include "global612.h"
#include "mpTrace.h"
#include "Net_api.h"
#include "BitsDefine.h"


/* bits for interface flags */
#define WWAN_INTERFACE_PPP  BIT0
#define WWAN_INTERFACE_GPRS BIT1
#define WWAN_INTERFACE_EXIT BIT2

struct wwan_interface {
	uint16_t ppp_state; /* */
	uint16_t gprs_state; /* */
	uint16_t flags;
};
	
extern struct wwan_interface wwan_if;

/* for event loop */
#define LEVENT_AT_CMD_DONE	    0x00000010
#define LEVENT_USER_INPUT	    0x00000020
#define LEVENT_PPP_STATUS	    0x00000040
#define LEVENT_GPRS_STATUS	    0x00000080

#endif

