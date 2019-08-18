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
* Filename      : usbotg_bt.h
* Programmer(s) : Rick Chen
* Created		: 2008/06/27 
* Description	: 
*******************************************************************************
*/
#ifndef _USBOTG_BT_H_
#define _USBOTG_BT_H_
#include "iplaysysconfig.h"
#include "utiltypedef.h"

//rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
#include "BtApi.h"
#endif
#include "Devio.h"

//#if (BT_PROFILE_TYPE & BT_HF)
//#define USBOTG_ISO_READY 1
//#else
//#define USBOTG_ISO_READY 0
//#endif

extern BYTE *gpBT_Buffer;
extern BYTE *gpBT_Buffer1;


extern BYTE *gpBT_Buffer_Bulk_In;
extern BYTE *gpBT_Buffer_Bulk_Out;
extern BYTE *gpBT_Buffer_Int_In;

#define MPX_BT_USB_INIT_EN			0x00000001
#define MPX_BT_USB_CONTROL_EN		0x00000002
#define MPX_BT_USB_INTERRUPT_EN		0x00000004
#define MPX_BT_USB_BULK_IN_EN		0x00000008
#define MPX_BT_USB_BULK_OUT_EN		0x00000010
#define MPX_BT_USB_INTERRUPT_GET	0x00000020
#define MPX_BT_USB_CONTROL_DONE		0x00000040
#define MPX_BT_USB_BULK_IN_DONE		0x00000080
#define MPX_BT_USB_BULK_OUT_DONE	0x00000100
#define MPX_BT_USB_PLUG_IN          0x00000200
#define MPX_BT_USB_ISO_IN_EN		0x00000400
#define MPX_BT_USB_ISO_OUT_EN		0x00000800
#define MPX_BT_USB_ISO_IN_DONE		0x00001000
#define MPX_BT_USB_ISO_OUT_DONE	    0x00002000



enum _BT_INIT_SM_
{
	BT_INIT_START_STATE								= 0,
	BT_SET_INTERFACE_STATE					= 1,
	BT_SET_INTERFACE_DONE_STATE				= 2,	
	BT_HCI_COMMAND_STATUS_EVENT_STATE			= 3,
	BT_HCI_COMMAND_STATUS_EVENT_DONE_STATE	= 4,
	BT_GET_STATUS_STATE						= 5,
	BT_GET_STATUS_DONE_STATE					= 6,
	
	BT_IDENTIFY_START_STATE							= 7,
	BT_CONTROL_OUT_STATE							= 8,
	BT_CONTROL_OUT_DONE_STATE						= 9,
	BT_INTERRUUPT_IN_STATE							= 10,
	BT_INTERRUUPT_IN_DONE_STATE						= 11,
	BT_BULK_IN_START_STATE							= 12,
	BT_BULK_IN_DONE_STATE							= 13,	
	BT_BULK_OUT_START_STATE							= 14,
	BT_BULK_OUT_DONE_STATE							= 15,	
	BT_INIT_STOP_STATE								= 16,		

	BT_ISO_IN_START_STATE							= 17,	
	BT_ISO_IN_DONE_STATE							= 18,
	BT_ISO_OUT_START_STATE							= 19,	
	BT_ISO_OUT_DONE_STATE							= 20,		
    

    BT_CLEAR_FEATURE_BULK_IN_START                  = 21,
    BT_CLEAR_FEATURE_BULK_IN_DONE                   = 22,    
    BT_CLEAR_FEATURE_BULK_OUT_START                 = 23,
    BT_CLEAR_FEATURE_BULK_OUT_DONE                  = 24,        
};
typedef void (*BtPlugInCallback)(void);
typedef void (*BtPlugOutCallback)(void);
static BtPlugInCallback BpiCallback;
static BtPlugOutCallback BpoCallback;
static void UsbOtgBTActiveInterrupt(WHICH_OTG eWhichOtg123);
static SWORD BTPipe0Out(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes,BYTE *pbBuffer,DWORD dwLength);
static void UsbOtgHostBTActive (WHICH_OTG eWhichOtgArg);
static void UsbOtgHostBTIoc (WHICH_OTG eWhichOtgArg);
static void UsbOtgHostBTSetupIoc(WHICH_OTG eWhichOtgArg);
static void UsbOtgHostBTInterruptIoc(WHICH_OTG eWhichOtgArg);
static SDWORD UsbBTTaskWaitEvent(DWORD * pdwEvent, DWORD dwNextEvent);
static void Next_Flow(void);
static void Add_Next_Flow(void);
static void UsbBTTask(void);
void RegisterBpiCallback(BtPlugInCallback callback);
void RegisterBpoCallback(BtPlugOutCallback callback);
void DeRegisterBpiCallback(void);
void DeRegisterBpoCallback(void);
void BTInitStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtgArg);

void BtUsbTaskInit(void);
void BtUsbTaskDeInit(void);
void EnBTBulkInRead(void);
DWORD EnBTBulkOutWrite(BYTE *pbBuffer,DWORD dwlength);
BOOL BTControlWrite(BYTE * pbBuffer,DWORD dwLength);
void EnBTInterruptRead(void);
BYTE CopyInterruptToBuffer(BYTE *pbBuffer,WORD *leftcount);
WORD CopyBulkReadToBuffer(BYTE *pbBuffer,WORD *leftcount);
void BT_USB_Off(void);
//void ResetBtEventWithDonglePlugIn(void);
void UsbOtgPlugOut(WHICH_OTG eWhichOtgArg);
#if USBOTG_HOST_ISOC == 1
DWORD UsbOtgHostBtIsocDataOut(BYTE *pSrcData, DWORD dwSrcLength, DWORD dwHciDataLength);
#endif
#endif //#if _USBOTG_BT_H_

