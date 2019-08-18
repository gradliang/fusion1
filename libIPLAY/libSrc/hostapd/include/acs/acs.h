#ifndef __ACS_H
#define __ACS_H

#include "utils/common.h"
#include "ap/hostapd.h"
#include "list.h"
#include "iplaysysconfig.h"

#ifdef HAVE_ACS
#define HOSTAPD_CHAN_ACS          1
#define HOSTAPD_CHAN_VALID        2 
#define HOSTAPD_CHAN_INVALID      3 

int acs_init(struct hostapd_iface *iface);
void hostapd_notify_acs_roc(struct hostapd_iface *iface,
			    unsigned int freq,
			    unsigned int duration,
			    int roc_status);
void hostapd_notify_acs_roc_cancel(struct hostapd_iface *iface,
				   unsigned int freq,
				   unsigned int duration,
				   int roc_status);
int hostapd_acs_completed(struct hostapd_iface *iface);
#else
static inline int acs_init(struct hostapd_iface *iface)
{
	wpa_printf(MSG_ERROR, "ACS was disabled on your build, "
		   "rebuild hostapd with CONFIG_ACS=1");
	return 0;
}
static inline void hostapd_notify_acs_roc(struct hostapd_iface *iface,
					  unsigned int freq,
					  unsigned int duration,
					  int roc_status)
{
}
static inline void hostapd_notify_acs_roc_cancel(struct hostapd_iface *iface,
						 unsigned int freq,
						 unsigned int duration,
						 int roc_status)
{
}
#endif /* HAVE_ACS */

#endif /* __ACS_H */
