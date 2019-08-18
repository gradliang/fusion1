// define this module show debug message or not,  0 : disable, 1 : enable

#define LOCAL_DEBUG_ENABLE 1

#include "global612.h"
#include "mpTrace.h"

#include "linux/types.h"
#include "linux/list.h"
#include "uip.h"
#include "net_sys.h"
#include "timer.h"
#include "taskid.h"
#include "net_netctrl.h"
#include "devio.h"
#include "net_autosearch.h"
#include "net_nic.h"
#include "net_dhcp.h"
#include "SysConfig.h"

#include "net_device.h"
#include "wlan_sys.h"
#include "typedef.h"
#include "wireless.h"

#include "wlan_common.h"
#include <linux/if_ether.h>
#include "ndebug.h"


#if DM9621_ETHERNET_ENABLE
#include "dm9621.h"
#endif

#ifdef NETWORK_DEBUG_ENABLE


static S32 netDebugTimerId;
static short _debug_current, _debug_old = 1;
static struct dm9621_statistics _dm9621_stat[2];
static struct l2_statistics _l2_stat[2];
static struct ip_statistics _ip_stat[2];

static void netDebugTimerProc(void)
{
    _dm9621_stat[_debug_current] = dm9621_stat;
    _l2_stat[_debug_current] = l2_stat;
    _ip_stat[_debug_current] = ip_stat;

#if DM9621_ETHERNET_ENABLE
    /* dm9621 */
	mpDebugPrint("\r\n[NET] -------------------- DM9621 -----------------------");
    if (_dm9621_stat[0].rx_pkts != _dm9621_stat[1].rx_pkts)
        mpDebugPrint("[NET] rx_pkts=%u", (long)_dm9621_stat[_debug_current].rx_pkts - (long)_dm9621_stat[_debug_old].rx_pkts);

    if (_dm9621_stat[0].rx_ucast != _dm9621_stat[1].rx_ucast)
        mpDebugPrint("[NET] rx_ucast=%u", (long)_dm9621_stat[_debug_current].rx_ucast - (long)_dm9621_stat[_debug_old].rx_ucast);

    if (_dm9621_stat[0].rx_bcast != _dm9621_stat[1].rx_bcast)
        mpDebugPrint("[NET] rx_bcast=%u", (long)_dm9621_stat[_debug_current].rx_bcast - (long)_dm9621_stat[_debug_old].rx_bcast);

    if (_dm9621_stat[0].rx_dropped != _dm9621_stat[1].rx_dropped)
        mpDebugPrint("[NET] rx_dropped=%u", (long)_dm9621_stat[_debug_current].rx_dropped - (long)_dm9621_stat[_debug_old].rx_dropped);
#endif

    /* layer 2 */
	mpDebugPrint("[NET] -------------------- Layer 2 -----------------------");
    if (_l2_stat[0].rx_pkts != _l2_stat[1].rx_pkts)
        mpDebugPrint("[NET] rx_pkts=%u", (long)_l2_stat[_debug_current].rx_pkts - (long)_l2_stat[_debug_old].rx_pkts);
    if (_l2_stat[0].rx_dropped != _l2_stat[1].rx_dropped)
        mpDebugPrint("[NET] rx_dropped=%u", (long)_l2_stat[_debug_current].rx_dropped - (long)_l2_stat[_debug_old].rx_dropped);
    if (_l2_stat[0].rx_arp != _l2_stat[1].rx_arp)
        mpDebugPrint("[NET] rx_arp=%u", (long)_l2_stat[_debug_current].rx_arp - (long)_l2_stat[_debug_old].rx_arp);
    if (_l2_stat[0].rx_bcast_arp != _l2_stat[1].rx_bcast_arp)
        mpDebugPrint("[NET] rx_bcast_arp=%u", (long)_l2_stat[_debug_current].rx_bcast_arp - (long)_l2_stat[_debug_old].rx_bcast_arp);
    if (_l2_stat[0].rx_ucast_arp != _l2_stat[1].rx_ucast_arp)
        mpDebugPrint("[NET] rx_ucast_arp=%u", (long)_l2_stat[_debug_current].rx_ucast_arp - (long)_l2_stat[_debug_old].rx_ucast_arp);


    /* ip layer TODO */

    _debug_old = _debug_current;
    _debug_current = (_debug_current == 0) ? 1 : 0;
}

void NetDebugEnable(void)
{
    S32 status;
//    status = NetTimerInstall(netDebugTimerProc, NET_TIMER_1_SEC * 300); /* 300 seconds == 5 min */
    status = NetTimerInstall(netDebugTimerProc, NET_TIMER_1_SEC * 120); /* 120 seconds == 2 min */
    if(status >= 0)
    {
        netDebugTimerId = status;
        NetTimerRun(netDebugTimerId);
    }
    else
        mpDebugPrint("netDebugTimerId creation failed");
}
#endif

