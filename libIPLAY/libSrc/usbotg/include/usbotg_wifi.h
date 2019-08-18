/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based 
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2008, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the 
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in 
* deciphering, decoding, reverse engineering or in ay way altering the source 
* code is strictly prohibited, unless the prior written consent of Magic 
* Pixel is obtained.
*
* Filename      : usbotg_wifi.h
* Programmer(s) : Bill Wang
* Created		: 2008/08/12 
* Description	: 
*******************************************************************************
*/
#ifndef __USBOTG_WIFI_H__
#define __USBOTG_WIFI_H__ 
#include "utiltypedef.h"


enum _WIFI_INIT_SM_                /* states for WiFi init state machine */
{
    WIFI_INIT_START_STATE            = 0,
    WIFI_INIT_STATE                  = 1,
};

#define	WIFI_BULK_DATA_IN_STATE	  BIT0
#define	WIFI_BULK_DATA_OUT_STATE  BIT1
#define	WIFI_BULK_DATA_OUT_STATE2  BIT2

struct urb;

void WifiStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
void usb_urb_complete(struct urb *urb, qTD_Structure *qtd);

int Wlan_UsbOtgHostBulkIoc(qHD_Structure *qhd);
void Wlan_SetDummy(WHICH_OTG eWhichOtg);

#endif //__USBOTG_WIFI_H__
