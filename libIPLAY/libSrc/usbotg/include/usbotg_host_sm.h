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
* Filename		: usbotg_host_sm.h
* Programmer(s)	: Joe Luo (JL)
* Created Date	: 2008/04/27 
* Description	: 
******************************************************************************** 
*/
#ifndef __USBOTG_HOST_SM_H__
#define __USBOTG_HOST_SM_H__ 

#include "utiltypedef.h"
//#include "utilregfile.h" //temp_remove
//#include "UsbHost.h"



#if (SC_USBHOST==ENABLE)
/*
// Constant declarations
*/
#define USBH_CONFIG_VALUE_NOT_DEFINED   0xff

typedef void(*pSmFunc)(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
enum  // USB-IF Status
{
    USB_IF_TEST_DISABLE                 = 0,     
    USB_IF_TEST_ENABLE                  = 1,
    USB_IF_TEST_READY                   = 2,
    USB_IF_TEST_J                       = 3,
    USB_IF_TEST_K                       = 4,
    USB_IF_TEST_SE0_NAK                 = 5,
    USB_IF_TEST_PACKET                  = 6,
    USB_IF_TEST_FORCE_ENABLE            = 7,
    USB_IF_TEST_SUSPEND                 = 8,
    USB_IF_TEST_RESUME                  = 9,
    USB_IF_TEST_RESET                   = 10,   /* Embedded Host don't test this item */
    USB_IF_TEST_MAX                     = USB_IF_TEST_RESUME,    
};

BOOL SetUsbIfTestMode(BYTE bMode, WHICH_OTG eWhichOtg);
BYTE GetUsbIfTestMode(WHICH_OTG eWhichOtg);

#endif  //USBOTG_HOST_USBIF

pSmFunc GetSmFunctionPointer(WORD wStateMachine);
void UsbhEnableMultiLun(WHICH_OTG eWhichOtg);
void UsbhDisableMultiLun(WHICH_OTG eWhichOtg);
BYTE GetDeviceAddress(WHICH_OTG eWhichOtg);
void ClearDeviceAddress(WHICH_OTG eWhichOtg);
BOOL IsNoNeedToPolling(WHICH_OTG eWhichOtg);
void ClearNoNeedToPollingFlag(WHICH_OTG eWhichOtg);
void SetNoNeedToPollingFlag(WHICH_OTG eWhichOtg);
void UsbOtgHostMsdcPollingForFullSpeed(void *arg);
void UsbOtgHostEof1Eof2Adjustment(WHICH_OTG eWhichOtg);

#endif // SC_USBHOST

#endif /* End of __USBOTG_HOST_SM_H__ */



