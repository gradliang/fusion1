/*
 * This is applicatoin level code for RTL8192SU.
 *
 * Copyright (c) 2011 Magic Pixel Inc.
 * All rights reserved.
 */

 /*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

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

typedef void (*get_scan_result_t)(void *, const WIFI_APINFO *);

extern WIFI_AP_BROWSER WiFiApBrowser;

static void get_network_cb(void *context, const WIFI_APINFO *apinfo);

/* 
 * Fill up scan results into WiFiApBrowser
 */
void MP_RTL8188C_APScanResults(struct net_device *dev)
{
	int results;

	WiFiApBrowser.dwNumberOfAP = 0;

#ifdef ADHOC_TEST
	/* 
	 * create a dummy one in UI for user to select to start a new IBSS network
	 */

	WIFI_APINFO *apinfo;
	char *ssid = Adhoc_Ssid();

	apinfo = &WiFiApBrowser.WiFiAPList[WiFiApBrowser.dwNumberOfAP];
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
#endif

	results = rtw_8188_get_scan_results(get_network_cb, &WiFiApBrowser);
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

	MP_DEBUG("%d) AP MAC=%02x:%02x:%02x:%02x:%02x:%02x",
			apBrowser->dwNumberOfAP+1,
			new->MAC[0], new->MAC[1], new->MAC[2], 
			new->MAC[3], new->MAC[4], new->MAC[5]);	

	MP_DEBUG("SSID=%s,%d,%ddBm",new->Ssid, new->Privacy, new->Rssi);
	MP_DEBUG("Security=%s",new->AuthMode);
	MP_DEBUG("Channel=%d",new->Channel);

	apBrowser->dwNumberOfAP++;	
}

