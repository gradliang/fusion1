/*
 * WPA Supplicant / Example program entrypoint
 * Copyright (c) 2003-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#define LOCAL_DEBUG_ENABLE 1


#include "global612.h"
#include "mpTrace.h"

#include "includes.h"

#include "common.h"
#include "wpa_supplicant_i.h"
#include "taskid.h"
//#include "net_device.h"
#include "net_const.h"
extern BYTE wifi_device_type;
struct wpa_global *wpaglobal;
BYTE wpa_pm_flag = 0;

void Wpa_Set_PM(BYTE flag)
{
	wpa_pm_flag = flag;
}
BYTE Wpa_Get_PM(void)
{
	return wpa_pm_flag;
}

void Wpa_Change_Iface(void)
{
	struct wpa_interface iface;
	struct wpa_params params;
	
    char *ifname = "wlan0";
    char *confname = "wpa_supplicant.conf";
	
	wpa_supplicant_deinit(wpaglobal);
	
	memset(&params, 0, sizeof(params));
//	params.wpa_debug_level = MSG_INFO;
//	params.wpa_debug_level = MSG_DEBUG;
	params.wpa_debug_level = MSG_WARNING;
	wpaglobal = wpa_supplicant_init(&params);
	if (wpaglobal == NULL)
		return;
	memset(&iface, 0, sizeof(iface));
	/* TODO: set interface parameters */

    //Set interface name
    iface.ifname = ifname;
    //Set configuration name
    iface.confname = confname;

	switch(wifi_device_type)
	{
	  case WIFI_USB_DEVICE_AR2524:
	  	iface.driver = "zydas";
		mpDebugPrint("\nwifi_device_type = zydas");
	  break;
	  
	  case WIFI_USB_DEVICE_RT73:
	  	iface.driver = "ralink";
		mpDebugPrint("\nwifi_device_type = ralink");
	  break;
	}

	if (wpa_supplicant_add_iface(wpaglobal, &iface) == NULL)
		mpDebugPrint("\nWpa_Change_Iface Fail!!\n");
  
}

#ifdef PLATFORM_MPIXEL
int wpa_initialized;
#endif
void wpa_main()
{

	DWORD dwNWEvent;  
	
	struct wpa_interface iface;
	int exitcode = 0;
	struct wpa_params params;

    char *ifname = "wlan0";
    char *confname = "wpa_supplicant.conf";

    /* Wait for wireless driver to be ready 
     * Otherwise, init will be failed.
     */
WAIT_EVEN:     
	EventWait(WPA_EVENT, 0x1, OS_EVENT_OR, &dwNWEvent);
	memset(&params, 0, sizeof(params));
//	params.wpa_debug_level = MSG_INFO;
//	params.wpa_debug_level = MSG_DEBUG;
	params.wpa_debug_level = MSG_WARNING;
	wpaglobal = wpa_supplicant_init(&params);
	if (wpaglobal == NULL)
		return;

	memset(&iface, 0, sizeof(iface));
	/* TODO: set interface parameters */

        //Set interface name
        iface.ifname = ifname;
        //Set configuration name
        iface.confname = confname;

		switch(wifi_device_type)
		{
		  case WIFI_USB_DEVICE_AR2524:
		  	iface.driver = "zydas";
			mpDebugPrint("\nwifi_device_type = zydas");
		  break;
		  
		  case WIFI_USB_DEVICE_RT73:
		  	iface.driver = "ralink";
			mpDebugPrint("\nwifi_device_type = ralink");
		  break;
		}

	if (wpa_supplicant_add_iface(wpaglobal, &iface) == NULL)
	{
		exitcode = -1;
		wpa_supplicant_deinit(wpaglobal);
		UsbOtgWifiPlugout();
		goto WAIT_EVEN;

	}

        DPrintf("exitcode = %d\n", exitcode);

	if (exitcode == 0)
		exitcode = wpa_supplicant_run(wpaglobal);

	wpa_supplicant_deinit(wpaglobal);
	
	goto WAIT_EVEN;
	//return exitcode;
}

void WPATaskInit()
{
	EventCreate(WPA_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
#if Make_USB == REALTEK_RTL8188CU || Make_USB == REALTEK_RTL8188E 
    TaskCreate(WPA_MAIN_TASK_ID, wpa_main, CONTROL_PRIORITY, 0x1000 * 4);
#else
    TaskCreate(WPA_MAIN_TASK_ID, wpa_main, DRIVER_PRIORITY, 0x1000 * 4);
#endif
    TaskStartup(WPA_MAIN_TASK_ID);
}
