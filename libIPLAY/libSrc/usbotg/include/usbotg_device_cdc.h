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
* Filename      : usbotg_device_cdc.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __USBOTG_DEVICE_CDC_H__  
#define __USBOTG_DEVICE_CDC_H__
///
///@defgroup    Device_CDC Device CDC class for Debug Message via USB
///@ingroup     USB_Peripheral
///  This USB OTG Device CDC class show Debug Message on Terminal screen via USB cable.<br>
/// Please follow the steps show as below, <br>
// 1. Set USBDEVICE_CDC_DEBUG ENABLE<br>
// 2. Install MPXCDC driver for PC<br>
// 3. Open PC Terminal AP [MUST after DPF Plug-in PC]<br>
//     Terminal:Port/depend on USB device Rate/115200 Data/8 Stop/1 Parity/none<br>
// 4. Card In/Out Action and Message will be showed on PC Terminal AP screen<br>

#include "iplaysysconfig.h"
#include "UtilTypeDef.h"
//#include "usbotg_ctrl.h"
//#include "usbdevice.h"  

enum {
    SET_LINE_CODING         = 0x20,
    GET_LINE_CODING         = 0x21,
    SET_CONTORL_LINE_STATE  = 0x22,
};

enum {
    SET_CONTORL_LINE_STATE_ACTIVATE  = 0x2,
    SET_CONTORL_LINE_STATE_PRESENT = 0x1,
};

 #if SC_USBDEVICE

void UsbCdcInit(WHICH_OTG eWhichOtg);
void UsbPutChar(BYTE data, WHICH_OTG eWhichOtg);
BYTE UsbGetChar(WHICH_OTG eWhichOtg);
BOOL GetIsAbleToUseUsbdCdc(WHICH_OTG eWhichOtg);
void SetIsAbleToUseUsbdCdc(BOOL flag, WHICH_OTG eWhichOtg);

#endif // SC_USBDEVICE
#endif // __USBOTG_DEVICE_CDC_H__

