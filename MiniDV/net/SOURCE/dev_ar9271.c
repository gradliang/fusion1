#define LOCAL_DEBUG_ENABLE 0


/*
// Include section 
*/
#include <linux/types.h>
#include <linux/list.h>
#include "netware.h"
#include "netware.h"
#include "taskid.h"
//#include "ieee80211_i.h"
//#include "net/cfg80211.h"

#include "ndebug.h"
#include "mpTrace.h"

typedef void (*get_scan_result_t)(void *, const WIFI_APINFO *);

extern WIFI_AP_BROWSER WiFiApBrowser;

static void get_network_cb(void *context, const WIFI_APINFO *apinfo);
extern int mac80211_scan_results(get_scan_result_t func, void *data);

/* 
 * Fill up scan results into WiFiApBrowser
 */
void MP_ATH9K_APScanResults(struct net_device *dev)
{
	int results;

	WiFiApBrowser.dwNumberOfAP = 0;

	results = mac80211_scan_results(get_network_cb, &WiFiApBrowser);
	if (!results)
		return;
}

static void get_network_cb(void *context, const WIFI_APINFO *apinfo)
{
	WIFI_AP_BROWSER *apBrowser = (WIFI_AP_BROWSER *)context;
	WIFI_APINFO *new;

	if (!apinfo)
		return;

	apBrowser->WiFiAPList[apBrowser->dwNumberOfAP] = *apinfo;

	new = &apBrowser->WiFiAPList[apBrowser->dwNumberOfAP];

	MP_DEBUG("%d) AP MAC=%02x:%02x:%02x:%02x:%02x:%02x",
			apBrowser->dwNumberOfAP+1,
			new->MAC[0], new->MAC[1], new->MAC[2], 
			new->MAC[3], new->MAC[4], new->MAC[5]);	

	MP_DEBUG("SSID=%s,%d,%ddBm",new->Ssid, new->Privacy, new->Rssi);
	MP_DEBUG("Security=%s",new->AuthMode);
	MP_DEBUG("Channel=%d",new->Channel);

	apBrowser->dwNumberOfAP++;

	switch (new->Security)
	{
		case WIFI_WPA2:
			strcpy(new->AuthMode,"WPA2-PSK");
			break;
		case WIFI_WPA:
			strcpy(new->AuthMode,"WPA-PSK");
			break;
		case WIFI_WEP:
			strcpy(new->AuthMode,"WEP");
			break;
		default:
			strcpy(new->AuthMode,"No Security");
			break;
	}
}


#if 0
void mpx_ath9k_insert_bss(struct cfg80211_bss *bss, bool update)
{
	WIFI_APINFO *apinfo;
	short i = 0;
	u8 *ie = bss->information_elements;
	int rem = bss->len_information_elements, sig;
	u8 *wpa_ie=NULL ,*rsn_ie = NULL;

	i = WiFiApBrowser.dwNumberOfAP;

	{

//	spin_lock_bh(&local->sta_bss_lock);
	 {
		apinfo = &WiFiApBrowser.WiFiAPList[i];

		apinfo->SsidLen = 0;

		while (rem >= 2) {
			 /* invalid data */
			 if (ie[1] > rem - 2)
				 break;

			 switch (ie[0]) {
				 case WLAN_EID_SSID:
					 apinfo->SsidLen = ie[1];
					 memcpy(apinfo->Ssid, &ie[2], apinfo->SsidLen);     
					 break;
				 case WLAN_EID_WPA:
					 wpa_ie = &ie[0];
					 break;
				 case WLAN_EID_RSN:
					 rsn_ie = &ie[0];
					 break;
			 }
			 rem -= ie[1] + 2;
			 ie += ie[1] + 2;
		}

        apinfo->Ssid[apinfo->SsidLen] = '\0';

		apinfo->Privacy = (bss->capability & (WLAN_CAPABILITY_PRIVACY)) ? 1 : 0;
		apinfo->Rssi = bss->signal - 96;	// dBm
		apinfo->Channel = ieee80211_frequency_to_channel(bss->channel->center_freq);

		memcpy(apinfo->MAC, bss->bssid, ETH_ALEN);

		memset(apinfo->AuthMode,0x00,32);	
		if (apinfo->Privacy) {
			if (rsn_ie) 
			{
				apinfo->Security = WIFI_WPA2;
				strcpy(apinfo->AuthMode,"WPA2-PSK");
			}
			else if (wpa_ie)
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
#if Make_Adhoc
		if(bss->capability&IEEE80211_CAP_IBSS)
		{
		    mpDebugPrint("\nWIFI_MODE_IBSS\n");
			apinfo->Mode = WIFI_MODE_IBSS;
			mBssInfo[mBssCnt].cap = bss->capability;
			memcpy(&mBssInfo[mBssCnt].bssid.mac,bss->bssid,ETH_ALEN);
			mBssCnt++;
		}
		else
		{
			apinfo->Mode = WIFI_MODE_INFRASTRUCTURE;
		}
#endif			

		MP_DEBUG("%d) AP MAC=%02x:%02x:%02x:%02x:%02x:%02x",
				WiFiApBrowser.dwNumberOfAP+1,
				apinfo->MAC[0], apinfo->MAC[1], apinfo->MAC[2], 
				apinfo->MAC[3], apinfo->MAC[4], apinfo->MAC[5]);	

		MP_DEBUG("SSID=%s,%d,%ddBm",apinfo->Ssid, apinfo->Privacy, apinfo->Rssi);
		MP_DEBUG("Security=%s",apinfo->AuthMode);
		MP_DEBUG("Channel=%d",apinfo->Channel);

		WiFiApBrowser.dwNumberOfAP++;
	}
//	spin_unlock_bh(&local->sta_bss_lock);
	}
}
#endif

