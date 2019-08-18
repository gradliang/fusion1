/**
 * @file
 *
 * RTL8188CU related code for MP6XX platforms.
 *
 * Copyright (c) 2011 Magic Pixel Inc.
 * All rights reserved.
 */

#define LOCAL_DEBUG_ENABLE 0

#ifdef RTL8192CU
#include <rtl8188cu_autoconf.h>
#include <osdep_mpixel_service.h>
#include <drv_types.h>


#include <rtl8192c_spec.h>
#include <rtl8192c_hal.h>
#endif
#include <linux/netdevice.h>		/*for __LINK_STATE_START*/
#include "taskid.h"					/*for NETWARE_TASK*/
#include "netware.h"				/*for MP_ASSERT*/ /* for WIFI_APINFO */

#define NIC_INDEX_WIFI          2	/*copy from net_nic.h*/

#define MAX_NUM_SEMAPHORES	104//80//72	/* sync with MAX_NUM_SEMAPHORES in mpx_linux.c */

extern BYTE wifi_device_unplug;
extern int usb_sema;

/* Static Function Declarations */
static int rtl8188cu_initialized;
struct usb_device_id *rtl8188cu_pdid;

int rtl8188cu_wifi_event_create = 1;
struct workqueue_struct *rtl8188cu_workqueue;

typedef void (*get_scan_result_t)(void *, const WIFI_APINFO *);

void usb_init_timer(void);
void usb_timer_task(void);
void urb_cleanup_init(struct usb_device * udev);
void urb_queue_init(struct usb_device * udev);

static int __init rtl8188cu_usb_preinit(void)
{
	int r = 0;
	
	rtl8188cu_workqueue = create_singlethread_workqueue("wlan0");
	if (rtl8188cu_workqueue == NULL) {
		printk(KERN_ERR "%s couldn't create workqueue\n", "wlan0");
		r = -ENOMEM;
	}

	return r;
}

int mp_rtl8188cu_preinit(void)
{
	int ret;
	
	if(mp_OsSemaphoreInit(MAX_NUM_SEMAPHORES) < 0)
		goto error;
	if(!rtl8188cu_initialized)
	{
		if(mpx_WorkQueueInit(2) < 0)
			goto error;
	}
	if(!usb_sema)
	{
		ret = (int)SemaphoreCreate(USB_CONTROL_SEMA, OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret == 0);
		SemaphoreWait(USB_CONTROL_SEMA);
		
		ret = (int)mpx_SemaphoreCreate(OS_ATTR_PRIORITY, 1);
		MP_ASSERT(ret > 0);
		if(ret <= 0)
			return -1;
		usb_sema = ret;
	}
	else
		SemaphorePolling(USB_CONTROL_SEMA);

	/* ----------  simulate Linux kernel internal timer  ---------- */
	mpx_TaskletInit(5);
	
	usb_init_timer();

	if(!rtl8188cu_initialized)
	{
		if(rtl8188cu_wifi_event_create)
		{
			rtl8188cu_wifi_event_create = EventCreate(WIFI_EVENT, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);
			MP_ASSERT(rtl8188cu_wifi_event_create == 0);
		}
		ret = TaskCreate(HISR_TASK, usb_timer_task, WIFI_PRIORITY, 0x1000);
		MP_ASSERT(ret == 0);
	}
	
	urb_queue_init(NULL);
	urb_cleanup_init(NULL);
	rtl8188cu_initialized++;
	TaskStartup(HISR_TASK);
	return 0;
error:
	return -1;
}

void chk_dod10();
int usb_r8188c_init(struct usb_driver *udriver, struct usb_interface *intf, WHICH_OTG eWhichOtg)
{
	struct net_device *netdev;
	int ret = 0;

	
	wifi_device_unplug = FALSE;

	/* to check USB device plug-out */
	if(UsbOtgCheckPlugOut(eWhichOtg))
	{
		goto error;
	}

	mpDebugPrint("%s", __func__);

	if(mp_rtl8188cu_preinit() != 0)
	{
		BreakPoint();
		goto error;
	}

//	chk_dod10();
	if(rtl8188cu_usb_preinit() != 0)
	{
		BreakPoint();
		goto error;
	}
	
//	chk_dod10();
//		__asm("break 100");
//mpDebugPrint("intf = %x	udriver = %x", intf, udriver);
//mpDebugPrint("%x	%x\n", getUsbInterface(), interface_to_usbdev(intf));
	
	//udriver->probe(intf, NULL);		//to call rtw_drv_init()
	udriver->probe(intf, udriver->id_table);
	
	chk_dod10();
	netdev = usb_get_intfdata(intf);
	MP_ASSERT(netdev);

	/*
	  *		Call device private open method
	*/
	set_bit(__LINK_STATE_START, &netdev->state);
	
	TaskStartup(NETWARE_TASK);

	ret = dev_open(netdev);	//in stubs.c

	chk_dod10();
	return ret;
	
error:
	ret = -1;
	return ret;
}

int dod10;
void chk_dod10();
int rtw_8188_get_scan_results(get_scan_result_t func, void * data)
{
	//struct net_device *dev = NetNicDeviceGet(NIC_INDEX_WIFI);
	struct net_device *dev = NetNicDeviceGet(NIC_INDEX_WIFI);
	_adapter *padapter = (_adapter *) rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv= &(padapter->mlmepriv);
	_queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	_irqL irqL;
	short i = 0;
	WIFI_APINFO _apinfo, *apinfo = &_apinfo;
		
	dod10 = 1;
	_enter_critical_bh(&queue->lock, &irqL);
	list_for_each_entry(pnetwork, &queue->queue, list)
	{
		chk_dod10();
		i++;
		//__asm("break 100");
		memset(apinfo, 0, sizeof(*apinfo));

		if(pnetwork->network.Ssid.SsidLength > 0)
		{
			memcpy(apinfo->Ssid, pnetwork->network.Ssid.Ssid, pnetwork->network.Ssid.SsidLength);
			apinfo->SsidLen = pnetwork->network.Ssid.SsidLength;
			apinfo->Ssid[apinfo->SsidLen] = '\0';
		}
		memcpy(apinfo->MAC, pnetwork->network.MacAddress, ETH_ALEN);
		apinfo->Channel = pnetwork->network.Configuration.DSConfig;
		apinfo->Rssi = pnetwork->network.Rssi;
		apinfo->Privacy = pnetwork->network.Privacy;
		if(pnetwork->network.InfrastructureMode == Ndis802_11Infrastructure)
			apinfo->Mode = WIFI_MODE_INFRASTRUCTURE;
		else if(pnetwork->network.InfrastructureMode == Ndis802_11IBSS)
			apinfo->Mode = WIFI_MODE_IBSS;
		if(apinfo->Privacy)
		{
			//parsing WPA/WPA2 IE
			u8 wpa_ie[255],rsn_ie[255];
			u16 wpa_len=0,rsn_len=0;
			sint out_len=0;
			out_len=rtw_get_sec_ie(pnetwork->network.IEs ,pnetwork->network.IELength,rsn_ie,&rsn_len,wpa_ie,&wpa_len);
			mpDebugPrint("%s: [%s-"MAC_FMT"]wpa_len=%d; rsn_len=%d\n", __func__, apinfo->Ssid, MAC_ARG(apinfo->MAC), wpa_len, rsn_len);
			if (rsn_len) 
				apinfo->Security = WIFI_WPA2;
			else if (wpa_len)
				apinfo->Security = WIFI_WPA;
			else
				apinfo->Security = WIFI_WEP;
				
		}
		else
				apinfo->Security = WIFI_NO_SECURITY;
#if 0		//for debug
		mpDebugPrint("ssid = %s", apinfo->Ssid);
		mpDebugPrint("ssid_len = %d", apinfo->SsidLen);
		mpDebugPrint("mac = "MAC_FMT"", MAC_ARG(apinfo->MAC));
		mpDebugPrint("channel = %d", apinfo->Channel);
		mpDebugPrint("rssi = %d", apinfo->Rssi);
		mpDebugPrint("privacy = %d", apinfo->Privacy);		
		mpDebugPrint("mode = %d", apinfo->Mode);
		mpDebugPrint("security = %d\n", apinfo->Security);
#endif
		func(data, apinfo);
	}
	chk_dod10();
	dod10 = 2;
	mpDebugPrint("\n%d BSS", i);
	chk_dod10();
//	__asm("break 100");
	_exit_critical_bh(&queue->lock, &irqL);

	dod10 = 3;
	//__asm("break 100");
	return i;
}

void chk_dod10()
{
	if ((unsigned int)dod10 > 10 )
		__asm("break 100");
}

#if 0
void test_scan(void)
{
	//TaskSleep(1000);
//IntDisable();	
	NetScanRequestEventSet();
//IntEnable();	
}
#endif
