/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
* 
* 
* Filename		: net_event.c
* Programmer(s)	: billwang
* Created Date	: 2010/05/06 
* Description:  Implementation of network event callbacks.
******************************************************************************** 
*/

#define LOCAL_DEBUG_ENABLE 1
#include "linux/list.h"
#include <linux/kernel.h>

//#include <stdbool.h>
#include "global612.h"
#include "mpTrace.h"
#include "taskid.h"
#include "net_sys.h"
//#include <linux/usb.h>

#include "..\..\Xpg\include\xpg.h"
#include "..\..\..\..\std_dpf\main\include\xpgGPRSFunc.h"
#include "os_mp52x.h"


#if HAVE_USB_MODEM

static bool usb_modem_device_plugin;

extern Net_App_State App_State;

int NetUsb_SignalRead(void);

/*
 *  USB modem hotplug events
 */

void usbModemPlugin()
{
	usb_modem_device_plugin = true;
	NetUsb_SignalRead();
}

void usbModemUnplug()
{
	usb_modem_device_plugin = false;
}

int UsbOtgModemPlugin()
{
	return usb_modem_device_plugin;
}

/*
 *  PPP interface events
 */

void pppUP()
{
	App_State.dwState |= NET_PPP;
	Xpg3G_ShowNetworkName(NULL,NULL);
}

void pppDOWN()
{
	App_State.dwState &= ~NET_PPP;
	Xpg3G_ShowNetworkName(NULL,NULL);
}

/* ppp connect error */
void pppERROR(int err)
{
}

#endif

// vim: :noexpandtab:
