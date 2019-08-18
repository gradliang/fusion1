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
* Filename      : usbotg_host_hub.c
* Programmer(s) : Joe Luo
* Created		: 2008/04/23 
* Description	: 
*******************************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0
/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"

#include "usbotg_ctrl.h"
#include "usbotg_host_setup.h"
#include "usbotg_host_sm.h"


#if (SC_USBHOST==ENABLE)
///////////////////////////////////////////////
// Shift to next gUsbhDeviceDescriptor  //byAlexWang
////////////////////////////////////////////
void UsbHubGoToNextDevicePointer ( WHICH_OTG eWhichOtg)
{
	WORD i = 0;
    BYTE controlEdListIndex = 0;
    PST_USB_OTG_HOST            psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppDes     = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR    pHubDes = (PST_HUB_CLASS_DESCRIPTOR)psHost->psHubClassDescriptor;

    pHubDes->bDeviceAddress     = pUsbhDevDes->bDeviceAddress;

	MP_DEBUG("gpUsbhDeviceDescriptor %x" , pUsbhDevDes);
	MP_DEBUG("gpAppClassDescriptor %x" , pAppDes);

	pUsbhDevDes->bUsbhDevDesIdx++;
    UsbOtgHostCurDevDescSet(eWhichOtg, pUsbhDevDes->bUsbhDevDesIdx);
    pUsbhDevDes = psHost->psUsbhDeviceDescriptor + 1;
    //*hUsbhDevDes = *hUsbhDevDes + 1;

    pAppDes->bAppDesIdx++;
    UsbOtgHostCurAppClassDescSet(eWhichOtg, pAppDes->bAppDesIdx);
    pAppDes = psHost->psAppClassDescriptor + 1;
    //*hAppDes = *hAppDes + 1;
    
	pAppDes->bDeviceConfigVal = USBH_CONFIG_VALUE_NOT_DEFINED;

    UsbOtgHostDeviceDescriptorInit(pUsbhDevDes);

    pUsbhDevDes->bMaxInterruptInEpNumber     = (psHost->psUsbhDeviceDescriptor)->bMaxInterruptInEpNumber;
    pUsbhDevDes->hstInterruptInqHD     = (psHost->psUsbhDeviceDescriptor)->hstInterruptInqHD;
    pUsbhDevDes->hstInterruptInqTD     = (psHost->psUsbhDeviceDescriptor)->hstInterruptInqTD;
    pUsbhDevDes->hstIntInWorkqTD       = (psHost->psUsbhDeviceDescriptor)->hstIntInWorkqTD;
    pUsbhDevDes->hstIntInSendLastqTD   = (psHost->psUsbhDeviceDescriptor)->hstIntInSendLastqTD;
    
    pUsbhDevDes->bDeviceStatus    = USB_STATE_DEFAULT;
    pUsbhDevDes->bConnectStatus   = UsbOtgHostConnectStatusGet(eWhichOtg);// mwHost20_PORTSC_ConnectStatus_Rd(); // Update Connected Status
    pUsbhDevDes->bDataBuffer      = (psHost->psUsbhDeviceDescriptor)->bDataBuffer;
    pUsbhDevDes->pstControlqHD[0] = (psHost->psUsbhDeviceDescriptor)->pstControlqHD[0];
    pUsbhDevDes->pstControlqHD[1] = (psHost->psUsbhDeviceDescriptor)->pstControlqHD[1];
    pUsbhDevDes->pstControlqHD[0]->bDeviceAddress         = 0; 
    pUsbhDevDes->pstControlqHD[0]->bHubAddr               = pHubDes->bDeviceAddress;
    pUsbhDevDes->pstControlqHD[0]->bPortNumber            = pHubDes->bPortNumber;
    pUsbhDevDes->pstControlqHD[0]->bSplitTransactionMask  = 0xF8; //??
    pUsbhDevDes->pstControlqHD[0]->bInterruptScheduleMask = 0x01; //??
    pUsbhDevDes->pstControlqHD[0]->bControlEdFlag         = 1;

    pUsbhDevDes->pstControlqHD[0]->bInactiveOnNextTransaction = 0;
    pUsbhDevDes->pstControlqHD[0]->bDataToggleControl         = 1;

    pUsbhDevDes->pstControlqHD[1]->bInactiveOnNextTransaction = 0;
    pUsbhDevDes->pstControlqHD[1]->bDataToggleControl         = 1;
    if (pHubDes->sHubGetPortStatus[pHubDes->bPortNumber-1].sHubPortStatus.bfHighSpeed == 1)
    {
	MP_DEBUG("High Speed Device");
        pUsbhDevDes->pstControlqHD[0]->bEdSpeed = HOST20_Attach_Device_Speed_High;
        pUsbhDevDes->pstControlqHD[1]->bEdSpeed = HOST20_Attach_Device_Speed_High;
        pUsbhDevDes->bDeviceSpeed = HOST20_Attach_Device_Speed_High;
    }
    else if (pHubDes->sHubGetPortStatus[pHubDes->bPortNumber-1].sHubPortStatus.bfLowSpeed == 1)
    {
	MP_DEBUG("Low Speed Device");
        pUsbhDevDes->pstControlqHD[0]->bEdSpeed = HOST20_Attach_Device_Speed_Low;
        pUsbhDevDes->pstControlqHD[1]->bEdSpeed = HOST20_Attach_Device_Speed_Low;
        pUsbhDevDes->bDeviceSpeed = HOST20_Attach_Device_Speed_Low;
    }
    else
    {
	MP_DEBUG("Full Speed Device");
        pUsbhDevDes->pstControlqHD[0]->bEdSpeed = HOST20_Attach_Device_Speed_Full;
        pUsbhDevDes->pstControlqHD[1]->bEdSpeed = HOST20_Attach_Device_Speed_Full;
        pUsbhDevDes->bDeviceSpeed = HOST20_Attach_Device_Speed_Full;
    }
	
	MP_DEBUG("gpUsbhDeviceDescriptor %x" , pUsbhDevDes);
	MP_DEBUG("gpAppClassDescriptor %x" , pAppDes);
}

void UsbOtgHostHubInterruptInActive (WHICH_OTG eWhichOtg)
{
    Host20_BufferPointerArray_Structure aTemp;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
//__asm("break 100");
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    MP_DEBUG("<SET device address %d for hub INT>", pUsbhDevDes->psHubClass->bDeviceAddress);
    UsbOtgHostSetAddressForUsbAddressState(pUsbhDevDes->psHubClass->bDeviceAddress, pUsbhDevDes);
    pUsbhDevDes->psAppClass->bIntInQHDArrayNum = 0;
    /*  // Had assigned by alloc qHD before this.(Don't assign again)
    pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bDeviceAddress = \
        pUsbhDevDes->psHubClass->bDeviceAddress;
    pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bEdNumber = \
        ((pUsbhDevDes->psHubClass->sInterruptInDescriptor.bEndpointAddress)&0x0f);
    */
    pUsbhDevDes->psAppClass->bIntDataInLen = \
        pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bMaxPacketSize;
    pUsbhDevDes->psAppClass->dIntDataBuffer = (DWORD)allo_mem((DWORD)pUsbhDevDes->psAppClass->bIntDataInLen,eWhichOtg);
    memset((BYTE*)pUsbhDevDes->psAppClass->dIntDataBuffer, 0, pUsbhDevDes->psAppClass->bIntDataInLen);
    aTemp.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dIntDataBuffer);
    aTemp.BufferPointerArray[1] = 0;	
    aTemp.BufferPointerArray[2] = 0;	
    aTemp.BufferPointerArray[3] = 0;
    aTemp.BufferPointerArray[4] = 0;
    MP_DEBUG("<INT Active>");
    flib_Host20_Periodic_Setting(HOST20_Enable, eWhichOtg);
    flib_Host20_Issue_Interrupt_Active( pUsbhDevDes->psAppClass->bIntInQHDArrayNum,
                                        pUsbhDevDes->psAppClass->bIntDataInLen, 
                                        (&aTemp.BufferPointerArray[0]), 
                                        0, 
                                        OTGH_DIR_IN,
                                        eWhichOtg);
}

void UsbOtgHostHubInterruptInIoc (WHICH_OTG eWhichOtg)
{
    BYTE i=0;
    BYTE whichPortChanged = 0;
    ST_MCARD_MAIL   *pSendMailDrv;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
//__asm("break 100");
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
    {
        return;
    }    
    if (pUsbhDevDes->bCmdTimeoutError == 1)
    {
        pUsbhDevDes->bCmdTimeoutError = 0;
        return;
    }

    MP_DEBUG("<INT>");
    flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[0], eWhichOtg);
    whichPortChanged = *((BYTE*)(pUsbhDevDes->psAppClass->dIntDataBuffer));
    i = 1;
    //__asm("break 100");
    while(i<8)
    {
        if (whichPortChanged & (1<<i))
        {
            pUsbhDevDes->psHubClass->bPortNumber = i;
            MP_DEBUG("port #%d connect", pUsbhDevDes->psHubClass->bPortNumber);
            break;
        }
        i++;
    };
    pUsbhDevDes->psHubClass->bIsPortChanged = 1;
    pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg);
    pSendMailDrv->wStateMachine             = SETUP_HUB_SM;
    pSendMailDrv->wCurrentExecutionState    = SETUP_HUB_GET_PORT_STATUS_STATE;
	SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);

    free1((void*)pUsbhDevDes->psAppClass->dIntDataBuffer, eWhichOtg);
}

#endif //SC_USBHOST


