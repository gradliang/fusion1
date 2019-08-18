#define LOCAL_DEBUG_ENABLE 0

#include <stdio.h>
#include <stdarg.h>
#include <linux/types.h>
#include "global612.h"
#include "typedef.h"
#include "mpTrace.h"
#include "typedef.h"
#include "os_mp52x.h"
#include "os.h"
#include "ndebug.h"

#include "taskid.h"
#include "list_mpx.h"
#include "wlan_adhoc.h"
#include "netware.h"

#ifdef HAVE_HOSTAPD

/* some dummy wpa_supplicant-related variables to prevent undefined symbols */
int wlan_mode;
int wlan_security;
int wlan_channel;                                   /**< channel for IBSS only */
int wep_tx_keyidx = -1;                             /**< WEP transmit key index */
char connect_ssid[32];  /**< ESSID */
int connect_ssidlen;    /**< Length of ESSID */

int is_packet_4_of_4;
int wpa_initialized;

void syslog(int __pri, __const char *__fmt, ...)
{

}

void closelog(void)
{

}

void openlog (__const char *__ident, int __option, int __facility)
{

}

void Wpa_Change_Iface(void)
{

}

#include <net/iw_handler.h>
#include <wireless.h>
#include "net_device.h"

void wireless_send_event(struct net_device *dev, unsigned int cmd, union iwreq_data *wrqu, char *extra)
{
}

#endif

