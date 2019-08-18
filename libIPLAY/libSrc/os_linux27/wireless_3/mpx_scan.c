/*
 * Read scan results
 *
 * Copyright 2010, Magic Pixel, Inc.
 */

#include <linux/types.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/pm_qos_params.h>
#include <linux/slab.h>
#include <net/sch_generic.h>
#include <core.h>
#include "net/cfg80211.h"

//#include "ieee80211_i.h"
//#include "driver-ops.h"
//#include "mesh.h"

#include "netware.h"                            /* for WIFI_APINFO */
#include "os_mp52x.h"
#include "mpTrace.h"

typedef void (*get_scan_result_t)(void *, const WIFI_APINFO *);

extern struct wiphy *g_wiphy;

int mac80211_scan_results(get_scan_result_t func, void *data)
{
	struct cfg80211_registered_device *dev = wiphy_to_dev(g_wiphy);
	struct cfg80211_internal_bss *bss;
	short i = 0;
	u8 *ie;
	int rem;
	u8 *wpa_ie, *rsn_ie;
	WIFI_APINFO _apinfo, *apinfo=&_apinfo;

	spin_lock_bh(&dev->bss_lock);

	list_for_each_entry(bss, &dev->bss_list, list) {
        i++;
        wpa_ie = rsn_ie = NULL;
		{
			memset(apinfo, 0, sizeof(*apinfo));

			memcpy(apinfo->MAC, bss->pub.bssid, ETH_ALEN);

            ie = bss->pub.information_elements;
            rem = bss->pub.len_information_elements;

            while (rem >= 2) {
                /* invalid data */
                if (ie[1] > rem - 2)
                    break;

                switch (ie[0]) {
                    case WLAN_EID_SSID:
                        apinfo->SsidLen = ie[1];
                        if (apinfo->SsidLen > 0)
                            memcpy(apinfo->Ssid, &ie[2], apinfo->SsidLen);     
                        apinfo->Ssid[apinfo->SsidLen] = '\0';
                        break;
                    case WLAN_EID_WPA:
                        if (ie[1] >= 4 && ie[2] == 0x00 && ie[3] == 0x50 &&
                                ie[4] == 0xf2 && ie[5] == 1)
                            wpa_ie = &ie[0];
                        break;
                    case WLAN_EID_RSN:
                        rsn_ie = &ie[0];
                        break;
                }
                rem -= ie[1] + 2;
                ie += ie[1] + 2;
            }

			apinfo->Privacy = (bss->pub.capability & (WLAN_CAPABILITY_PRIVACY)) ? 1 : 0;
			apinfo->Rssi = bss->pub.signal / 100;	// dBm
            apinfo->Channel = ieee80211_frequency_to_channel(bss->pub.channel->center_freq);

			if (apinfo->Privacy)
			{
                if (rsn_ie) 
					apinfo->Security = WIFI_WPA2;
                else if (wpa_ie)
					apinfo->Security = WIFI_WPA;
				else
					apinfo->Security = WIFI_WEP;
			}
			else
				apinfo->Security = WIFI_NO_SECURITY;

			func(data, apinfo);
		}
	}
	spin_unlock_bh(&dev->bss_lock);

	return i;
}

