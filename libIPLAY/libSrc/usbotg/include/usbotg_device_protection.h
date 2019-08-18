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
* Filename      : usbotg_device_protection.h
* Programmer(s) : Calvin Liao
* Created       :
* Descriptions  : For USB PenDrive Protection
*******************************************************************************
*/
#ifndef __USBOTG_DEVICE_PROTECTION_H__
#define __USBOTG_DEVICE_PROTECTION_H__

#include "iplaysysconfig.h"
#include "UtilTypeDef.h"


#if (SC_USBDEVICE && USBOTG_DEVICE_PROTECTION)


#define USB_PROTECTION_DATA_MAX_SIZE  130    // for user password max 64 char x 2



typedef struct
{
	BYTE   OperCode;
	BYTE   FunctionCode;
	BYTE   Reserved1[14];
} USB_PROTECTION_CMD;

typedef struct
{
	WORD   DataCode;
	BYTE     Data[USB_PROTECTION_DATA_MAX_SIZE];
} USB_PROTECTION_DATA;


WORD VendorProtectionCmd( BYTE** hData, USB_PROTECTION_CMD* pVendorProtect, BYTE lun, DWORD* pData_residue, WHICH_OTG eWhichOtg);
WORD VendorProtectionData(USB_PROTECTION_DATA* pVendorProtect, DWORD dwDataSize, BYTE lun, WHICH_OTG eWhichOtg);

#endif // SC_USBDEVICE
#endif // __USBOTG_DEVICE_PROTECTION_H__

