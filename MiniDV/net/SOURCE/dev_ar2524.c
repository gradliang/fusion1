/*
 * This is applicatoin level code for AR2524 (zd1211rw).
 *
 * Copyright (c) 2010 Magic Pixel Inc.
 * All rights reserved.
 */

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#define __KERNEL__  1

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "string.h" 

#include "linux/list.h"
#include "ndebug.h"
#include "net_device.h"
#include "netware.h"
#include "ieee80211_i.h"

typedef void (*get_scan_result_t)(void *, const WIFI_APINFO *);

extern WIFI_AP_BROWSER WiFiApBrowser;
extern BYTE wifi_device_type;

struct net_device * NetNicDeviceGet(int if_index);
size_t os_strlcpy(char *dest, const char *src, size_t siz);
int get_scan_results(get_scan_result_t func, void *data);
static void get_network_cb(void *context, const WIFI_APINFO *apinfo);

char *Adhoc_Ssid(void);
int Adhoc_Channel(void);
int Adhoc_Security(void);

/* 
 * Fill up scan results into WiFiApBrowser
 */
void MP_Wifi_APScanResults(struct net_device *dev)
{
	struct ieee80211_local *local = wdev_priv(dev->ieee80211_ptr);
	struct ieee80211_sta_bss *bss;
	WIFI_APINFO *apinfo;
	short i = 0;

	if(wifi_device_type == WIFI_USB_DEVICE_AR2524)//WIFI_USB_DEVICE_AR2524
	{
#ifdef ADHOC_TEST
		/* 
		 * create a dummy one for user from UI to select to start a new IBSS network 
		 */

        char *ssid = Adhoc_Ssid();
		apinfo = &WiFiApBrowser.WiFiAPList[i];
		int len = strlen(ssid);
		memcpy(apinfo->Ssid, ssid, len);     
		apinfo->Ssid[len] = '\0';
		apinfo->SsidLen = len;

		apinfo->Security = Adhoc_Security();
		if (apinfo->Security)
			apinfo->Privacy = 1;
		else
			apinfo->Privacy = 0;
		apinfo->Rssi = -86;	// dBm
		apinfo->Channel = Adhoc_Channel();
		apinfo->Mode = WIFI_MODE_IBSS;

		if (apinfo->Privacy)
		{
			apinfo->Security = WIFI_WEP;
			strcpy(apinfo->AuthMode,"WEP");
		}
		else 
		{
			apinfo->Security = WIFI_NO_SECURITY;
			strcpy(apinfo->AuthMode,"No Security");
		}

		WiFiApBrowser.dwNumberOfAP++;
		i++;
#endif
		spin_lock_bh(&local->sta_bss_lock);
		list_for_each_entry(bss, &local->sta_bss_list, list) {
		apinfo = &WiFiApBrowser.WiFiAPList[i];
        memcpy(apinfo->Ssid, bss->ssid, bss->ssid_len);     
        apinfo->Ssid[bss->ssid_len] = '\0';
        apinfo->SsidLen = bss->ssid_len;

		apinfo->Privacy = (bss->capability & (WLAN_CAPABILITY_PRIVACY)) ? 1 : 0;
		apinfo->Rssi = bss->signal - 96;	// dBm
		apinfo->Channel = ieee80211_frequency_to_channel(bss->freq);
		apinfo->Mode = (bss->capability & WLAN_CAPABILITY_IBSS) ? WIFI_MODE_IBSS : WIFI_MODE_INFRASTRUCTURE;

		memcpy(apinfo->MAC, bss->bssid, ETH_ALEN);

		memset(apinfo->AuthMode,0x00,32);	
		if (apinfo->Privacy) {
			if (bss->rsn_ie) 
			{
				apinfo->Security = WIFI_WPA2;
				strcpy(apinfo->AuthMode,"WPA2-PSK");
			}
			else if (bss->wpa_ie)
			{
				apinfo->Security = WIFI_WPA;
				strcpy(apinfo->AuthMode,"WPA-PSK");
		}
		else
		{
				apinfo->Security = WIFI_WEP;
				strcpy(apinfo->AuthMode,"WEP");
		}
		}
		else {
			strcpy(apinfo->AuthMode,"No Security");
			apinfo->Security = WIFI_NO_SECURITY;
		}

		MP_DEBUG("%d) AP MAC=%02x:%02x:%02x:%02x:%02x:%02x",
				WiFiApBrowser.dwNumberOfAP+1,
				apinfo->MAC[0], apinfo->MAC[1], apinfo->MAC[2], 
				apinfo->MAC[3], apinfo->MAC[4], apinfo->MAC[5]);	

		MP_DEBUG("SSID=%s,%d,%ddBm",apinfo->Ssid, apinfo->Privacy, apinfo->Rssi);
		MP_DEBUG("Security=%s",apinfo->AuthMode);
		MP_DEBUG("Channel=%d",apinfo->Channel);

		WiFiApBrowser.dwNumberOfAP++;
		i++;
		
	}
	spin_unlock_bh(&local->sta_bss_lock);
	}
}
