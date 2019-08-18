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
* Filename      : usbotg_ethernet.h
* Programmer(s) : Kevin Huang
* Created		: 2010/11/30 
* Description	: 
*******************************************************************************
*/
#ifndef __USBOTG_ETHERNET_H__
#define __USBOTG_ETHERNET_H__ 
#include "utiltypedef.h"


enum _ETHERNET_INIT_SM_                /* states for Ethernet init state machine */
{
	ETHERNET_INIT_START_STATE            = 0,
	ETHERNET_INIT_STATE                  = 1,
};

#define	ETHERNET_BULK_DATA_IN_STATE	  BIT0
#define	ETHERNET_BULK_DATA_OUT_STATE  BIT1
#define	ETHERNET_BULK_DATA_OUT_STATE2  BIT2

struct urb;

void EthernetStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
void usb_urb_complete(struct urb *urb, qTD_Structure *qtd);

//int Wlan_UsbOtgHostBulkIoc(qHD_Structure *qhd);
//void Wlan_SetDummy(WHICH_OTG eWhichOtg);

#endif //__USBOTG_ETHERNET_H__
