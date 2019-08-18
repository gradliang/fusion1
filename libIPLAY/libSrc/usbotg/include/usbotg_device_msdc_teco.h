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
* Filename      : usbotg_device_msdc_teco.h
* Programmer(s) : Calvin Liao
* Created       :
* Descriptions  : Vendor MSDC Command for TECO
*******************************************************************************
*/
#ifndef __USBOTG_DEVICE_MSDC_TECO_H__
#define __USBOTG_DEVICE_MSDC_TECO_H__

#include "iplaysysconfig.h"
#include "UtilTypeDef.h"


#if (SC_USBDEVICE && USBOTG_DEVICE_MSDC_TECO)


typedef struct
{
	BYTE   bOperCode;
	BYTE   bLUN;
	BYTE   bFunctionCode;
	BYTE   bData[13];
} USB_TECO_CMD;

enum
{
	FC_DEFAULT                = 0x00 , // Default  (Function code:FC)
	FC_GET_MPX_DEVICE  = 0x01 , // Get MPX Device
	FC_SET_IDU_MODE     = 0x02 ,
	FC_SEND_DATA           = 0x03 ,
	FC_PREVIEW                = 0x04 ,
	FC_GET_RESOLUTION  = 0x05
};

enum
{
	MODE_DEFAULT           = 0x00 ,
	MODE_422                   = 0x01 , 
	MODE_444                   = 0x02
};


WORD VendorTecoCmd( BYTE** hData, USB_TECO_CMD* pVendorCmd, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg);
WORD VendorTecoDataOut(BYTE* pVendorProtect, DWORD dwDataSize, BYTE lun, WHICH_OTG eWhichOtg);
void    VendorTecoSetIduMode(BYTE bMode);

#endif // SC_USBDEVICE
#endif // __USBOTG_DEVICE_MSDC_TECO_H__

