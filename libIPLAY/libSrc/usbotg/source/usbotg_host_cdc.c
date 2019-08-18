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
* Filename		: usbotg_host_cdc.c
* Programmer(s)	: Calvin
* Created Date	: 2008/02/18 
* Description        : USB OTG CDC Class
******************************************************************************** 
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
#include "usbotg_host_cdc.h"
#include "Usbotg_at.h"

#include "taskid.h"
#include "os.h"

#if USBOTG_HOST_CDC

/*
// Static function prototype
*/

/*
// Definition of internal functions
*/

// For Usbotg_at.c Usage
SDWORD CdcDataOut(BYTE *pbBuffer, DWORD dwlength, WHICH_OTG eWhichOtg)
{
    WORD	bi;
    BYTE *pbuf;
    pbuf = pbBuffer;
    		
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    pUsbhDevDes->psAppClass->dwBulkOnlyState    = CBI_DATA_OUT_STATE;  // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    pUsbhDevDes->psAppClass->dDataOutBuffer     = (DWORD)pbBuffer;
    pUsbhDevDes->psAppClass->dDataOutLen        = dwlength;
	
    return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
}

SDWORD CdcDataIn(BYTE *pbBuffer, DWORD dwlength, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    pUsbhDevDes->psAppClass->dwBulkOnlyState    = CBI_DATA_IN_STATE;  // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE    
    pUsbhDevDes->psAppClass->dDataBuffer        = (DWORD)pbBuffer;
    pUsbhDevDes->psAppClass->dDataInLen         = dwlength;
    
    return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);	
}

SDWORD CdcInterruptIn(BYTE *pbBuffer, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    pUsbhDevDes->psAppClass->dwBulkOnlyState    = CBI_INTERRUPT_STATE;  // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE    
    pUsbhDevDes->psAppClass->dIntDataBuffer     = ((DWORD)pbBuffer | 0xa0000000);
    pUsbhDevDes->psAppClass->bIntDataInLen      = \
    pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum]->bMaxPacketSize;
    
    return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);	
}

void UsbOtgHostCdcBulkActive(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    Host20_BufferPointerArray_Structure stBufAry;
    DWORD dwDataLen = 0;
    BYTE  bDataDir = 0; 

    MP_DEBUG("UsbOtgHostCdcCbiActive %s", pUsbhDevDes->psAppClass->dwBulkOnlyState==CBI_DATA_OUT_STATE?"Out":"In");

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_DATA_OUT_STATE:
            stBufAry.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dDataOutBuffer);
            stBufAry.BufferPointerArray[1] = 0;	
            stBufAry.BufferPointerArray[2] = 0;	
            stBufAry.BufferPointerArray[3] = 0;
            stBufAry.BufferPointerArray[4] = 0;
    
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum,
        									pUsbhDevDes->psAppClass->dDataOutLen,
                                            (&stBufAry.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_OUT,
                                            eWhichOtg);
            break;
        
        case CBI_DATA_IN_STATE:
            stBufAry.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dDataBuffer);
            stBufAry.BufferPointerArray[1] = 0;	
            stBufAry.BufferPointerArray[2] = 0;	
            stBufAry.BufferPointerArray[3] = 0;
            stBufAry.BufferPointerArray[4] = 0;
    
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkInQHDArrayNum,
        									pUsbhDevDes->psAppClass->dDataInLen,
                                            (&stBufAry.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_IN,
                                            eWhichOtg);
            break;

        default:
            MP_DEBUG("Unknow USB Direction");
            break;
    }
    
	

}

void UsbOtgHostCdcInterruptActive(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    Host20_BufferPointerArray_Structure stBufAry;
    DWORD dwDataLen = 0;
    BYTE  bDataDir = 0;

    MP_DEBUG("IntActive Len %x Data %x", pUsbhDevDes->psAppClass->dDataOutLen, *(BYTE*)pUsbhDevDes->psAppClass->dDataBuffer);

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_INTERRUPT_STATE:
            MP_DEBUG("CDC_INTERRUPT_STATE");
            break;

        default:
            MP_DEBUG("Unknow USB State %x", pUsbhDevDes->psAppClass->dwBulkOnlyState);
            break;
    }
    
    stBufAry.BufferPointerArray[0] = (DWORD)(pUsbhDevDes->psAppClass->dIntDataBuffer);
    stBufAry.BufferPointerArray[1] = 0;	
    stBufAry.BufferPointerArray[2] = 0;	
    stBufAry.BufferPointerArray[3] = 0;
    stBufAry.BufferPointerArray[4] = 0;

    flib_Host20_Issue_Interrupt_Active( pUsbhDevDes->psAppClass->bIntInQHDArrayNum,
                                        pUsbhDevDes->psAppClass->bIntDataInLen, 
                                        (&stBufAry.BufferPointerArray[0]), 
                                        0, 
                                        OTGH_DIR_IN,
                                        eWhichOtg); 
}

void UsbOtgHostCdcBulkIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    DWORD dwBulkOnlyState =0;

    if (pUsbhDevDes->dwWhichBulkPipeDone&0xffff)  // In/Out at same time (for P2P protocol)
        dwBulkOnlyState = CBI_DATA_OUT_STATE;
    else if (pUsbhDevDes->dwWhichBulkPipeDone&0xffff0000)
        dwBulkOnlyState = CBI_DATA_IN_STATE;
    else
        MP_DEBUG("CDC Bulk-IOC Err!!");      

    switch (dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_DATA_OUT_STATE:
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);
            break;
        
        case CBI_DATA_IN_STATE:           
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
            break;

        default:
            MP_DEBUG("Unknow USB Direction %x", pUsbhDevDes->psAppClass->dwBulkOnlyState);
            break;
    }    
    
}

void UsbOtgHostCdcInterruptIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    
    MP_DEBUG("UsbOtgHostCdcCbiIntIoc");

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_INTERRUPT_STATE:
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum], eWhichOtg);
            break; 

        default:
            MP_DEBUG("Unknow USB Direction %x", pUsbhDevDes->psAppClass->dwBulkOnlyState);
            break;
    }    
    
}
#endif
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
void NetUsb_Hotplug(WHICH_OTG eWhichOtg, BOOL plug);
int NetUsb_Init(WHICH_OTG eWhichOtg);
void UsbOtgHostCdcAtCmdInit(WHICH_OTG eWhichOtg)
{
#if HAVE_USB_MODEM
    NetUsb_Init(eWhichOtg);
#endif
}

void UsbOtgHostCdcAtCmdPlug(WHICH_OTG eWhichOtg, BOOL bPlug)
{
#if HAVE_USB_MODEM
    NetUsb_Hotplug(eWhichOtg, bPlug);
#endif
}
#endif	//(USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
#if USBOTG_HOST_CDC
/*
//  Control End-Pointer (SETUP)
*/

//BYTE CDC_VENDOR_CMD_DATA[0x10] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x02, 0x90, 0x30, 0x00, 0x00, 0x00, 0x00};
/*Need*/ // should use LineCoding Structure
//BYTE CDC_SET_LINE_CODING_DATA[0x07] = {0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08};
SWORD SetupCdcClassCmd(BYTE bRequest, WORD wValue, WHICH_OTG eWhichOtg)
{
    WORD    data_out_len    = 0;
    DWORD   data_addr       = 0;
    SWORD   err             = USB_NO_ERROR;
    USB_CTRL_REQUEST    requestContainer;    
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PUSB_HOST_CDC psCdc = (PUSB_HOST_CDC)UsbOtgHostCdcDsGet(eWhichOtg);
    
    switch(bRequest)
    {
        case CDC_SET_LINE_CODING:
            /*Need*/ // interface 
            data_out_len = 0x07;
            SetupBuilder(&requestContainer, USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE, bRequest, wValue, USB_CDC_INTERFACE_1, data_out_len);
            data_addr = (DWORD)psCdc->pbCdcSetLineCodingData;//CDC_SET_LINE_CODING_DATA;            
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            /*Need*/ // wValue &  interface
            data_out_len = 0x00;
            SetupBuilder(&requestContainer, USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE, bRequest, wValue, USB_CDC_INTERFACE_1, data_out_len);
            break;

        case CDC_VENDOR_CMD_STATE: // Vendor Command 
            data_out_len = 0x10;
            SetupBuilder(&requestContainer, USB_DIR_OUT|USB_TYPE_VENDOR, CDC_QISDA_VENDOR_CMD_STATE, wValue, 0x00, data_out_len);
            data_addr = (DWORD)psCdc->pbCdcVendorCmdData;//CDC_VENDOR_CMD_DATA;
            break;

        default:
#ifdef USBOTG_HOST_CDC_HUAWEI_TESTING
            mpDebugPrint("HUAWEI:%s:%d", __FUNCTION__, __LINE__);
            data_out_len = 0;
            SetupBuilder(&requestContainer, USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_ENDPOINT, bRequest, wValue, 0x01, data_out_len);
            data_addr = (DWORD)psCdc->pbCdcVendorCmdData;//CDC_VENDOR_CMD_DATA;
#else
            MP_DEBUG("CDC bRequest is not defined");
#endif
            break;
    }

    MP_ALERT("SetupCdcClassCmd 0x%x", bRequest);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB, 0, data_out_len, data_addr, &requestContainer, eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupCdcClassCmd %x", bRequest);
    }

    return err;
}

#endif //USBOTG_HOST_CDC

