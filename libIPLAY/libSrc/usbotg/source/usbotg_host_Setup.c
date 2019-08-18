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
* Filename      : usbotg_host_setup.c
* Programmer(s) : Joe Luo    
* Created		: 2008/04/24 
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

#include "taskid.h"
#include "os.h"

#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
#include "usbotg_bt.h"
#endif

#if USBOTG_WEB_CAM
#include "usbotg_host_web_cam.h"
#endif

#if (SC_USBHOST==ENABLE)

/*
// Constant declarations
*/


/*
// Structure declarations
*/


    
/*
// Variable declarations
*/

/*
// Extern Variable declarations
*/
//extern PST_USBH_DEVICE_DESCRIPTOR  gpUsbhDeviceDescriptor;
//extern DWORD	gDummy_Mem;

/*
// Static function prototype
*/
//static void SetupBuilder(   PUSB_CTRL_REQUEST   pCtrlRequest,
//                            BYTE                bRequestType,
//                            BYTE                bRequest,
//                            WORD                wValue,
//                            WORD                wIndex,
//                            WORD                wLength);
static void GetInterfaceDescriptor( BYTE        idx,
                                    BYTE        *pData,
                                    WHICH_OTG   eWhichOtg);
static void GetHubEndpointDescriptor(   BYTE        idx,
                                        BYTE        *ep_data_Addr,
                                        WHICH_OTG   eWhichOtg);

static void GetMsdcEndpointDescriptor(  BYTE        idx,
                                        BYTE        *ep_data_Addr,
                                        WHICH_OTG   eWhichOtg);
#if USBOTG_HOST_CDC
static BOOL UsbhCdcGetInterfaceInformation(WHICH_OTG eWhichOtg);
#endif

#if USBOTG_HOST_DESC
BOOL DumpConfigDescriptorInfo(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes);
BOOL DumpUsbInterfaceInfo(USB_INTERFACE_NUMBER *pUsbIFNum, BYTE bUsbIFs);
BOOL ParseConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes);
BOOL SetAppClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
                        PST_APP_CLASS_DESCRIPTOR    pAppClassDes,
                        PST_HUB_CLASS_DESCRIPTOR    pHubClassDes);
#endif

void SetupBuilder(
        PUSB_CTRL_REQUEST   pCtrlRequest,
        BYTE                bRequestType,
        BYTE                bRequest,
        WORD                wValue,
        WORD                wIndex,
        WORD                wLength)
{
    pCtrlRequest->bRequestType  = bRequestType;
    pCtrlRequest->bRequest      = bRequest;
    pCtrlRequest->wValue        = byte_swap_of_word(wValue);
    pCtrlRequest->wIndex        = byte_swap_of_word(wIndex);
    pCtrlRequest->wLength       = byte_swap_of_word(wLength);
}
#if USBOTG_WEB_CAM
extern  int g_print_led;
#endif
SDWORD SetupCommand(
    PST_USBOTG_SETUP_PB pSetupPB,
    WORD                data_in_len,
    WORD                data_out_len,
    DWORD               data_addr,
    PUSB_CTRL_REQUEST   pCtrl_request,
    WHICH_OTG           eWhichOtg)
{
    pSetupPB->wDataInLength     = data_in_len;
    pSetupPB->wDataOutLength    = data_out_len;
    pSetupPB->dwDataAddress     = data_addr;
    pSetupPB->dwSetupState      = SETUP_COMMAND_STATE;
    
#if USBOTG_WEB_CAM
    if (g_print_led)
        MP_ALERT("pSetupPB->dwDataAddress 0x%08X", pSetupPB->dwDataAddress);
#endif
    memcpy((BYTE*)&(pSetupPB->stCtrlRequest), (BYTE*)pCtrl_request, sizeof(USB_CTRL_REQUEST));

//UartOutText("<e>");
    return EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_SETUP);
}

SWORD SetupClearFeature(BYTE end_point, WHICH_OTG eWhichOtg)
{
    SWORD               err = USB_NO_ERROR;
    USB_CTRL_REQUEST    ctrl_request;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_USBOTG_SETUP_PB psSetupPB = (PST_USBOTG_SETUP_PB)&pUsbhDevDes->sSetupPB;
    
//__asm("break 100");
    MP_DEBUG("ClearFeature");
    SetupBuilder(  &ctrl_request,
                    (USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_ENDPOINT),
                    USB_REQ_CLEAR_FEATURE,
                    0,
                    end_point,
                    0);

    err = SetupCommand( psSetupPB,
                        0,
                        0,
                        0,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupClearFeature() fail");
        MP_DEBUG1("err = %x", err);
    }

    return err;
}


SWORD SetupGetDeviceDescriptor(WHICH_OTG eWhichOtg)
{
    DWORD   data_addr;
    SDWORD   err = 0;
    WORD    val=0;
    DWORD   bufferRounding = 1;
    USB_CTRL_REQUEST	ctrl_request;
    BYTE    controlEdListIndex = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    //MP_DEBUG("SetupGetDeviceDescriptor() ");        
    data_addr   = (DWORD)allo_mem(pUsbhDevDes->sDeviceDescriptor.bLength, eWhichOtg);
    val         = (WORD)(USB_DT_DEVICE << 8); // put DT val in high byte

    SetupBuilder (  &ctrl_request,
                    (BYTE) USB_DIR_IN,
                    (BYTE) USB_REQ_GET_DESCRIPTOR,
                    (WORD) val,
                    (WORD) 0,
                    (WORD) pUsbhDevDes->sDeviceDescriptor.bLength);

    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        pUsbhDevDes->sDeviceDescriptor.bLength,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != 0)
    {
        MP_DEBUG("ERROR:SetupGetDeviceDescriptor() fail");
        MP_DEBUG1("err = %x", err);
    }

    return err;
}


SWORD SetupSetAddress(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    WORD    val = 0;
    SWORD	err = USB_NO_ERROR;

    USB_CTRL_REQUEST	ctrl_request;

    //MP_DEBUG("SetupSetAddress");
    SetupBuilder(  &ctrl_request,
                    USB_DIR_OUT,
                    USB_REQ_SET_ADDRESS,
                    GetDeviceAddress(eWhichOtg),
                    0x00,
                    0x00);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        OTGH_NULL,
                        &ctrl_request,
                        eWhichOtg);
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetAddress");
        MP_DEBUG1("err = %x", err);
    }

    return err;
}


SWORD SetupGetConfigurationDescriptor (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);    
    SWORD	err         = USB_NO_ERROR;
    WORD    val         = 0;
    DWORD   ep_length   = 0;
    WORD    length      = 0;
    BYTE    usb_dir     = 0;
    BYTE    i           = 0;
    BYTE    num_of_interface = 0;
    
    DWORD	data_addr;
    DWORD   ep_data_Addr;
    
    USB_CTRL_REQUEST    ctrl_request;
    BYTE    controlEdListIndex = 0;

#if USBOTG_HOST_DESC
    length = GetConfigTotalLength(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
    if (length == 0)
    {
        length = GetConfigLength(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
    }     
#else  
    length = byte_swap_of_word(pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].wTotalLength);
    if (length == 0)
    {
        length = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bLength;
    }  
#endif
    
    data_addr  = (DWORD)allo_mem(length, eWhichOtg);
    val = uchar_to_ushort(USB_DT_CONFIG, (pUsbhDevDes->bDeviceConfigIdx));  // put DT val in high byte
    
    //MP_DEBUG("SetupGetConfigurationDescriptor");
    SetupBuilder(   &ctrl_request,
                    USB_DIR_IN,
                    USB_REQ_GET_DESCRIPTOR,
                    val,
                    0x00,
                    length);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        length,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("SetupGetConfigurationDescriptor fail");
        MP_DEBUG1("err = %x", err);
        return err;
    }
    
    return err;
}

void SetupGetConfigurationDescriptorDone  (WHICH_OTG eWhichOtg)
{
    BYTE    i                = 0;
    WORD    length           = 0;
    BYTE    num_of_interface = 0;
    PST_USB_OTG_HOST            psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR    pHubClassDes = (PST_HUB_CLASS_DESCRIPTOR)psHost->psHubClassDescriptor;
    
#if USBOTG_HOST_DESC
    length = GetConfigTotalLength(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
    if (length == 0)
    {
        length = GetConfigLength(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
    }     
#else  
    length = byte_swap_of_word(pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].wTotalLength);
    if (length == 0)
    {
        length = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bLength;
    }
#endif

#if USBOTG_HOST_DESC
    if (GetConfigTotalLength(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx) == 0)
#else
    if (pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].wTotalLength == 0)
#endif     
    {   // we have 9 bytest only
#if USBOTG_HOST_DESC
        memcpy( (BYTE*)pUsbhDevDes->pConfigDescriptor,
                (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress,
                length);
#else
        memcpy( (BYTE*)&pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx],
                (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress,
                length);
#endif
    }
    else
    {   // we have total bytes of config
#if USBOTG_HOST_DESC // Dynamic Configuration Descriptor for test temporarily

        if(ParseConfigDescriptor(pUsbhDevDes) != TRUE) // Parse ConfigDescriptor and Alloc
        {
            MP_ALERT("--E-- %s !!Parse Config Descriptor Error!!", __FUNCTION__);
        }
        else
        {
            MP_DEBUG("Parse Config Descriptor OK and Initialize all variable");
            
            pUsbhDevDes->bDeviceConfigIdx   = 0; // Only ONE for now
            pUsbhDevDes->bDeviceInterfaceIdx= 0; // decide the bDeviceInterfaceIdx by different USB CLASS

            pAppClassDes->bBulkInQHDArrayNum = 0; // decide the qHD ( One BulK EP Num = 0 ) by different USB CLASS
            pAppClassDes->bBulkOutQHDArrayNum = 0;
            pAppClassDes->bIntInQHDArrayNum = 0;
            pAppClassDes->bIntOutQHDArrayNum = 0;

            pUsbhDevDes->bDeviceConfigVal   = GetConfigConfigurationValue(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
            pAppClassDes->bDeviceAddress    = pUsbhDevDes->bDeviceAddress;
            pAppClassDes->bDeviceConfigVal  = pUsbhDevDes->bDeviceConfigVal;

            // Set AppClass by Class of Device 
            SetAppClass(pUsbhDevDes, pAppClassDes, pHubClassDes);

            // Please Set bDeviceInterfaceIdx & bXXXQHDArrayNum etc for your wanted EPs at UsbOtgHostSetDeviceDescriptor( )
        }

#else // USBOTG_HOST_DESC

        #if USBOTG_WEB_CAM
		if (webCamParseInterfaceForBestIsoEp(eWhichOtg) == TRUE)
		{
      	 	pUsbhDevDes->bDeviceConfigVal   = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bConfigurationValue;
	        pAppClassDes->bDeviceAddress    = pUsbhDevDes->bDeviceAddress;
    	    pAppClassDes->bDeviceConfigVal  = pUsbhDevDes->bDeviceConfigVal;
			return;
		}
        #endif

        #if USBOTG_HOST_CDC // if USBOTG_MAGICSYNC TRUE, USBOTG_HOST_CDC FALSE
        if(UsbhCdcGetInterfaceInformation(eWhichOtg) == TRUE)
        {
      	 	pUsbhDevDes->bDeviceConfigVal   = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bConfigurationValue;
	        pAppClassDes->bDeviceAddress    = pUsbhDevDes->bDeviceAddress;
    	    pAppClassDes->bDeviceConfigVal  = pUsbhDevDes->bDeviceConfigVal;
			return;
        }
        #endif   

        num_of_interface = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bNumInterfaces;
        for (i = 0; i < num_of_interface; i++)
        {
            GetInterfaceDescriptor (i,(BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress,eWhichOtg);
        }
#endif // USBOTG_HOST_DESC
    }
}

SWORD SetupSetConfguration (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR pAppClassDes   = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err = USB_NO_ERROR;
    WORD    val         = 0;

    val = (WORD)(pAppClassDes->bDeviceConfigVal);  // put val in low byte   
    
    SetupBuilder(  &ctrl_request,
                    USB_DIR_OUT,
                    USB_REQ_SET_CONFIGURATION,
                    val, //0x0100;DT
                    0x00,
                    0x00);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        0,
                        &ctrl_request,
                        eWhichOtg);
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetConfguration");
        MP_DEBUG1("err = %x", err);
    }

    return err;
}

SWORD SetupSetInterface (WORD wInterface, WORD wAltInterface, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);    
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err = USB_NO_ERROR;
    WORD    val         = 0;
    BOOL    isHub = FALSE;

//    	val = (WORD)(pAppClassDes->bDeviceConfigVal);  // put val in low byte   , 0x00 01
    	SetupBuilder(  &ctrl_request,// 0x01 0B 00 00 01 00 00 00
                    (USB_DIR_OUT |USB_RECIP_INTERFACE), // 0x01
                    USB_REQ_SET_INTERFACE, // 0x0B
					wAltInterface, 
					wInterface, 
                    0x00);

    	err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        0,
                        &ctrl_request,
                        eWhichOtg);

    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetInterface");
        MP_DEBUG1("err = %x", err);
    }
    return err;
}

SWORD SetupGetHubDescriptor(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);    
    PST_HUB_CLASS_DESCRIPTOR pHubDes        = (PST_HUB_CLASS_DESCRIPTOR)UsbOtgHostDevHubDescGet(eWhichOtg);     
    PUSB_HUB_DESCRIPTOR pUsbhHubDes         = &pHubDes->sHubDescriptor;
        
    DWORD   data_addr;
    SWORD   err = USB_NO_ERROR;
    WORD    val=0;
    USB_CTRL_REQUEST	ctrl_request;
    BYTE    controlEdListIndex = 0;

    //MP_DEBUG("SetupGetHubDescriptor() ");        
    data_addr   = (DWORD)allo_mem(pUsbhHubDes->bDescLength, eWhichOtg);

    SetupBuilder (  &ctrl_request,
                    (BYTE) (USB_DIR_IN|USB_TYPE_CLASS),
                    (BYTE) USB_REQ_GET_DESCRIPTOR,
                    (WORD) 0,
                    (WORD) 0,
                    (WORD) pUsbhHubDes->bDescLength);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        pUsbhHubDes->bDescLength,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetupGetHubDescriptor() fail: %x", err);
    }

    return err;
}

SWORD SetupGetSatus(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    DWORD   data_addr;
    SWORD   err = USB_NO_ERROR;
    WORD    val=0;
    USB_CTRL_REQUEST	ctrl_request;
    DWORD   len = 0;

    //MP_DEBUG("SetupGetSatus() "); 
    len = 2;//sizeof(pUsbhDevDes->sDeviceStatus);
    data_addr   = (DWORD)allo_mem(len, eWhichOtg);

    SetupBuilder (  &ctrl_request,
                    (BYTE) (USB_DIR_IN|USB_TYPE_STANDARD),
                    (BYTE) USB_REQ_GET_STATUS,
                    (WORD) 0,
                    (WORD) 0,
                    (WORD) len);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        len,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetupGetSatus() fail: %x", err);
    }

    return err;
}



SWORD SetupGetPortSatus (WORD port_num, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    DWORD   data_addr;
    SWORD   err = USB_NO_ERROR;
    USB_CTRL_REQUEST	ctrl_request;
    DWORD   len = 0;

    len = 4;//sizeof(HUB_GET_PORT_STATUS);
    data_addr = (DWORD)allo_mem(len,eWhichOtg);

    SetupBuilder (  &ctrl_request,
                    (BYTE) (USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_PORT),
                    (BYTE) USB_REQ_GET_STATUS,
                    (WORD) 0,
                    (WORD) port_num,
                    (WORD) len);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        len,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetupGetPortSatus() fail: %x", err);
    }

    return err;
}

SWORD SetPortFeature (  WORD port_num,
                            WORD feature_selector,
                            WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err = USB_NO_ERROR;

    SetupBuilder(  &ctrl_request,
                    (BYTE) (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_PORT),
                    (BYTE) USB_REQ_SET_FEATURE,
                    (WORD) feature_selector,
                    (WORD) port_num,
                    (WORD) 0);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        OTGH_NULL,
                        &ctrl_request,
                        eWhichOtg);
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetPortFeature() fail: %x", err);
    }

    return err;
}


SWORD ClearPortFeature (    WORD port_num,
                                WORD feature_selector,
                                WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err = USB_NO_ERROR;

    SetupBuilder ( &ctrl_request,
                   (BYTE) (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_PORT),
                   (BYTE) USB_REQ_CLEAR_FEATURE,
                   (WORD) feature_selector,
                   (WORD) port_num,
                   (WORD) 0);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        OTGH_NULL,
                        &ctrl_request,
                        eWhichOtg);

    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:ClearPortFeature() fail: %x", err);
    }

    return err;
}


//
//  MSDC Setup Command
//
SWORD SetupMsdcReset (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    DWORD	data_addr;
    SWORD   err = USB_NO_ERROR;
    
    USB_CTRL_REQUEST	ctrl_request;
      
    SetupBuilder (  &ctrl_request,
                    0x21,                       
                    USB_REQ_MSDC_RESET,
                    0, 
                    0,
                    0);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        0,
                        0,
                        OTGH_NULL,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetupGetMaxLun() fail err = %x", err);
    }
    
    return err; 
}


SWORD SetupGetMaxLun (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    DWORD	data_addr;
    SWORD   err = USB_NO_ERROR;
    
    USB_CTRL_REQUEST	ctrl_request;
      
    data_addr  = (DWORD)allo_mem(1,eWhichOtg);
    SetupBuilder (  &ctrl_request,
                    (USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE), // 0xa1                       
                    USB_REQ_MSDC_GET_MAX_LUN,
                    0, 
                    0,
                    1);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        1,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG1("ERROR:SetupGetMaxLun() fail err = %x", err);
    }
    
    return err; 
}

#if USBOTG_HOST_CDC

static BOOL UsbhCdcGetqHDArrayNum(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PUSB_ENDPOINT_DESCRIPTOR pEpDesc;
    BYTE    num_of_interface = 0, bCnt, bInterfaceClass = 0, bCntEp, bEpCnt = 0;

    num_of_interface = GetConfigNumInterface(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
    
    for(bCnt = 0; bCnt < num_of_interface; bCnt++)
    {
        bInterfaceClass = GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx);
        switch(bInterfaceClass)
        {
            case USB_CLASS_COMM:   // Set Int qHD array number
                pUsbhDevDes->bDeviceInterfaceIdx= bCnt; // decide the bDeviceInterfaceIdx by different USB CLASS
                bEpCnt = GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx);
                for(bCntEp = 0; bCntEp < bEpCnt; bCntEp++)
                {
                    pEpDesc = GetEndpointStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx, bCntEp);
                    if((pEpDesc->bmAttributes & 0x03) == OTGH_ED_INT)
                    {
                        if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
                            pAppClassDes->bIntOutQHDArrayNum = pEpDesc->bQHDArrayNum;
                        else if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
                            pAppClassDes->bIntInQHDArrayNum = pEpDesc->bQHDArrayNum;
                        else
                            MP_DEBUG("--W-- Other INT Type 0x%x", (pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK));
                    }
                }  
                break;

            case USB_CLASS_CDC_DATA:   // Set Bulk qHD array number
                bEpCnt = GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx);
                for(bCntEp = 0; bCntEp < bEpCnt; bCntEp++)
                {
                    pEpDesc = GetEndpointStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx, bCntEp);
                    if((pEpDesc->bmAttributes & 0x03) == OTGH_ED_BULK)
                    {
                        if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
                            pAppClassDes->bBulkOutQHDArrayNum = pEpDesc->bQHDArrayNum;
                        else if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
                            pAppClassDes->bBulkInQHDArrayNum = pEpDesc->bQHDArrayNum;
                        else
                            MP_DEBUG("--W-- Other Bulk Type 0x%x", (pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK));
                    }
                }                            
                break;

            case USB_CLASS_VENDOR_SPEC:
                // Nothing
                break;    

            default:
                MP_ALERT("--W-- %s CDC Err InterfaceClass 0x%x", __FUNCTION__, bInterfaceClass);
                break; 
        }
    }

}


/*Need*/ // Only for spec. CDC class. It will be compatibile for all-class
static BOOL UsbhCdcGetInterfaceInformation(WHICH_OTG eWhichOtg)
{
	WORD wTotalLength;
	BOOL bRtValue = FALSE;
    BYTE num_of_interface = 0;
    BYTE num_of_endpoint = 0;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    BYTE *pData = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;

    
    wTotalLength = byte_swap_of_word(*((WORD *)(pData+2)));
    
	MP_DEBUG("Interface Descriptor length %x", wTotalLength);
    
	do {
		switch (*(pData+1)) // check bDescriptorType
		{
			case USB_DT_CONFIG: // CONFIG
				MP_DEBUG("CONFIG");
				break;
            
			case USB_DT_INTERFACE: // INTERFACE
			    MP_DEBUG("INTERFACE %d", num_of_interface);
                USB_INTERFACE_DESCRIPTOR *pInterface = (USB_INTERFACE_DESCRIPTOR *)pData;
                if(pInterface->bInterfaceClass != USB_CLASS_VENDOR_SPEC) /*Need*/ // skip USB_CLASS_VENDOR_SPEC
                {
                    memcpy( (BYTE*)&pUsbhDevDes->sInterfaceDescriptor[num_of_interface],
    								pData,
    								USB_DT_INTERFACE_SIZE);  

                    #if 0
                    pUsbhDevDes->sInterfaceDescriptor[num_of_interface].bInterfaceClass = pInterface->bInterfaceClass;
                    #endif
                    MP_DEBUG("idx %d Class %d", num_of_interface, pUsbhDevDes->sInterfaceDescriptor[num_of_interface].bInterfaceClass);
                    if(pUsbhDevDes->sInterfaceDescriptor[num_of_interface].bInterfaceClass == USB_CLASS_COMM) 
                        bRtValue = TRUE; // CDC Class
                    num_of_interface += 1;
                    if (num_of_interface >= MAX_NUM_OF_INTERFACE)
                    {
                        return bRtValue;
                    }
                }
				break;
                
			case USB_DT_CS_INTERFACE:  // CS_INTERFACE
				MP_DEBUG("CS_INTERFACE");
				break;
                
			case USB_DT_ENDPOINT:
                MP_DEBUG("ENDPOINT %d", num_of_endpoint);
                if(num_of_interface > 0) /*Need*/ // Don't get interface:0
                {
                    USB_ENDPOINT_DESCRIPTOR *pEndpoint = (USB_ENDPOINT_DESCRIPTOR *)pData;
                    memcpy((BYTE*)&pUsbhDevDes->sEndpointDescriptor[num_of_endpoint],
    								pData,
    								USB_DT_ENDPOINT_SIZE);

                    MP_DEBUG("MaxPacketSize %d/%d",pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].wMaxPacketSize,num_of_endpoint);

                    #if 0                
                    pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].bEndpointAddress = pEndpoint->bEndpointAddress;
                    pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].bmAttributes = pEndpoint->bmAttributes;
                    pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].wMaxPacketSize = (WORD*)pEndpoint->wMaxPacketSize;
                    #endif
                    MP_DEBUG("idx %d Addr %d Attr %d", num_of_endpoint,pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].bEndpointAddress, pUsbhDevDes->sEndpointDescriptor[num_of_endpoint].bmAttributes);
                    num_of_endpoint += 1;				
                    if (num_of_endpoint >= MAX_NUM_OF_ENDPOINT)
                    {
                        return bRtValue;
                    }
                }
				break;

			default:
				MP_DEBUG("unknown bDescriptotType!!");
				break;
		}
		pData += *(pData); // Next Discriptor by before Discriptor bLength

	} while (pData < ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress + wTotalLength));

    MP_DEBUG("UsbhCdcGetInterfaceInformation EP cnt %d", num_of_endpoint);
    pUsbhDevDes->sInterfaceDescriptor[0].bNumEndpoints = num_of_endpoint; /*Need*/ // Not only for [0]
    
	return bRtValue;

}

#endif // USBOTG_HOST_CDC

#if USBOTG_HOST_DATANG
static BOOL UsbhDatangGetqHDArrayNum(WHICH_OTG eWhichOtg)
{
	PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	PST_APP_CLASS_DESCRIPTOR    pAppClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
	PUSB_ENDPOINT_DESCRIPTOR pEpDesc;
	BYTE    num_of_interface = 0, bCnt, bInterfaceClass = 0, bCntEp, bEpCnt = 0;

	num_of_interface = GetConfigNumInterface(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);

	for(bCnt = 0; bCnt < num_of_interface; bCnt++)
	{
		bInterfaceClass = GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx);
		switch(bInterfaceClass)
		{
            		case USB_CLASS_VENDOR_SPEC:   // Set Int/Bulk qHD array number
				{
				pUsbhDevDes->bDeviceInterfaceIdx= bCnt; // decide the bDeviceInterfaceIdx by different USB CLASS
				bEpCnt = GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx);
				for(bCntEp = 0; bCntEp < bEpCnt; bCntEp++)
				{
			            pEpDesc = GetEndpointStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bCnt, AlterSettingDefaultIdx, bCntEp);
			            if((pEpDesc->bmAttributes & 0x03) == OTGH_ED_INT)
			            {
                        			if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
                            			pAppClassDes->bIntOutQHDArrayNum = pEpDesc->bQHDArrayNum;
                        			else if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
                           			pAppClassDes->bIntInQHDArrayNum = pEpDesc->bQHDArrayNum;
                        			else
                            			MP_DEBUG("--W-- Other INT Type 0x%x", (pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK));
                    		}
	                    	if((pEpDesc->bmAttributes & 0x03) == OTGH_ED_BULK)
					{
						if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
							pAppClassDes->bBulkOutQHDArrayNum = pEpDesc->bQHDArrayNum;
						else if((pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
							pAppClassDes->bBulkInQHDArrayNum = pEpDesc->bQHDArrayNum;
						else
							MP_DEBUG("--W-- Other Bulk Type 0x%x", (pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK));
					}
				}
				}
				break;    

            		default:
				MP_ALERT("--W-- %s DATANG Err InterfaceClass 0x%x", __FUNCTION__, bInterfaceClass);
				break; 
		}
	}
}
#endif	//USBOTG_HOST_DATANG

#if (USBOTG_HOST_DESC == DISABLE)
static void GetInterfaceDescriptor( BYTE        idx,
                                    BYTE        *pData,
                                    WHICH_OTG   eWhichOtg)
{
    DWORD   ep_data_Addr;
    WORD    ep_length   = 0;
    WORD    inf_offset      = 0;
    WORD    ep_offset      = 0;
    WORD    num_of_ep   = 0;
    WORD    i           = 0;

    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR    pHubClassDes = (PST_HUB_CLASS_DESCRIPTOR)UsbOtgHostDevHubDescGet(eWhichOtg);
                                    
//	rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))		
    if(USB_CLASS_WIRELESS_BT == pUsbhDevDes->sInterfaceDescriptor[0].bInterfaceClass)
    {
	if(idx >= 1)
	{
		MP_DEBUG("Only get 1 interface..give up others");
		return;
	}
    }
#endif    

    if (idx == 0)
    {
        inf_offset  = USB_DT_CONFIG_SIZE;
        ep_offset   = USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE;
    }
    else
    {
        for (i = 0; i < idx; i++)
        {
            num_of_ep += pUsbhDevDes->sInterfaceDescriptor[i].bNumEndpoints;
        }

        inf_offset  =   (USB_DT_CONFIG_SIZE) +
                        (USB_DT_INTERFACE_SIZE * idx) +
                        (USB_DT_ENDPOINT_SIZE * num_of_ep);

        ep_offset   =   (USB_DT_CONFIG_SIZE) +
                        (USB_DT_INTERFACE_SIZE * (idx + 1)) +
                        (USB_DT_ENDPOINT_SIZE * num_of_ep);

    }

    memcpy( (BYTE*)&pUsbhDevDes->sInterfaceDescriptor[idx],
            (BYTE*)(pData + inf_offset),
            USB_DT_INTERFACE_SIZE);

    ep_length    = USB_DT_ENDPOINT_SIZE * pUsbhDevDes->sInterfaceDescriptor[idx].bNumEndpoints;
    ep_data_Addr = (DWORD)allo_mem(ep_length,eWhichOtg);

    memcpy( (BYTE*) ep_data_Addr,
            (BYTE*) (pData + ep_offset),
            ep_length);

    if ((pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_MASS_STORAGE) ||
        (pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_STILL_IMAGE)  ||
//rick BT
#if ((BLUETOOTH == ENABLE) && (BT_DRIVER_TYPE == BT_USB))	
        (pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_WIRELESS_BT)  ||        
#endif        
        (pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_VENDOR_SPEC)  ||        
		(pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_PER_INTERFACE)||  
        (pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass == USB_CLASS_HUB))
    {
        pUsbhDevDes->bDeviceConfigVal   = pUsbhDevDes->sConfigDescriptor[pUsbhDevDes->bDeviceConfigIdx].bConfigurationValue;
        pAppClassDes->bDeviceAddress    = pUsbhDevDes->bDeviceAddress;
        pAppClassDes->bDeviceConfigVal  = pUsbhDevDes->bDeviceConfigVal;
        for (i = 0; i < pUsbhDevDes->sInterfaceDescriptor[idx].bNumEndpoints; i++)
        {
        	if (USB_CLASS_HUB == pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass)
        	{
                     GetHubEndpointDescriptor(i, (BYTE*)ep_data_Addr, eWhichOtg);
        	}
                else if (USB_CLASS_VENDOR_SPEC == pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass)
                {
                     memcpy((BYTE*)&pUsbhDevDes->sEndpointDescriptor[i],
                            (BYTE*)(ep_data_Addr + (i * USB_DT_ENDPOINT_SIZE)),
                            USB_DT_ENDPOINT_SIZE);
                }
                else if (USB_CLASS_PER_INTERFACE == pUsbhDevDes->sInterfaceDescriptor[idx].bInterfaceClass)
                {
                     memcpy((BYTE*)&pUsbhDevDes->sEndpointDescriptor[i],
                            (BYTE*)(ep_data_Addr + (i * USB_DT_ENDPOINT_SIZE)),
                            USB_DT_ENDPOINT_SIZE);
                }

        	else
        	{
                     GetMsdcEndpointDescriptor(i, (BYTE*)ep_data_Addr, eWhichOtg);
                }
        }

        pUsbhDevDes->bDeviceInterfaceIdx = idx;
    }

    free1((void*)ep_data_Addr, eWhichOtg);    
}



static void GetMsdcEndpointDescriptor(  BYTE idx,
                                        BYTE *ep_data_Addr,
                                        WHICH_OTG eWhichOtg)
{
    BYTE usb_dir    = 0;
    PST_USB_OTG_HOST           psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR pMsdcDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);

    memcpy( (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
            (BYTE*)(ep_data_Addr + (idx * USB_DT_ENDPOINT_SIZE)),
            USB_DT_ENDPOINT_SIZE);
    if (pUsbhDevDes->sEndpointDescriptor[idx].bDescriptorType == USB_DT_ENDPOINT)
    {
        if (pUsbhDevDes->sEndpointDescriptor[idx].bmAttributes == USB_ENDPOINT_XFER_BULK)
        {
            usb_dir = pUsbhDevDes->sEndpointDescriptor[idx].bEndpointAddress & USB_ENDPOINT_DIR_MASK;
            if (usb_dir == USB_DIR_OUT)
            {
                memcpy( (BYTE*)&pMsdcDes->sBulkOutDescriptor,
                        (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
                        USB_DT_ENDPOINT_SIZE);
            }
            else if (usb_dir == USB_DIR_IN)
            {
                memcpy( (BYTE*)&pMsdcDes->sBulkInDescriptor,
                        (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
                        USB_DT_ENDPOINT_SIZE);
            }
        }
        

	else if ( USB_ENDPOINT_XFER_INT ==  pUsbhDevDes->sEndpointDescriptor[idx].bmAttributes)
	{
	   usb_dir = pUsbhDevDes->sEndpointDescriptor[idx].bEndpointAddress & USB_ENDPOINT_DIR_MASK;	  	
	   MP_DEBUG("getMsdcEndDscr INT ep dir = 0x%x", usb_dir);
           if (usb_dir == USB_DIR_IN)
           {
               memcpy( (BYTE*)&pMsdcDes->sInterruptInDescriptor,
                     (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
                     USB_DT_ENDPOINT_SIZE);
           }
	  	
         }
         
    }
}
static void GetHubEndpointDescriptor(BYTE idx, BYTE *ep_data_Addr, WHICH_OTG eWhichOtg)
{
    BYTE usb_dir    = 0;
    PST_USB_OTG_HOST           psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR pHubDes = (PST_HUB_CLASS_DESCRIPTOR)UsbOtgHostDevHubDescGet(eWhichOtg);

    

    memcpy( (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
            (BYTE*)(ep_data_Addr + (idx * USB_DT_ENDPOINT_SIZE)),
            USB_DT_ENDPOINT_SIZE);
    if (pUsbhDevDes->sEndpointDescriptor[idx].bDescriptorType == USB_DT_ENDPOINT)
    {
        if (pUsbhDevDes->sEndpointDescriptor[idx].bmAttributes == USB_ENDPOINT_XFER_INT)
        {
            usb_dir = pUsbhDevDes->sEndpointDescriptor[idx].bEndpointAddress & USB_ENDPOINT_DIR_MASK;
            if (usb_dir == USB_DIR_OUT)
            { // interrupt has no OUT dir
                ;
            }
            else if (usb_dir == USB_DIR_IN)
            {
                memcpy( (BYTE*)&pHubDes->sInterruptInDescriptor,
                        (BYTE*)&pUsbhDevDes->sEndpointDescriptor[idx],
                        USB_DT_ENDPOINT_SIZE);
            }
        }
    }
}
#endif //(USBOTG_HOST_DESC == DISABLE)

#if USBOTG_WEB_CAM
////////////////////// WebCam API for FAE ///////////////////////////////////////
PUSBH_UVC_FORMAT_INFORMATION Api_UsbhWebCamVideoGetForamtInfo(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return &psUsbHostAVdc->sUvcFormatInfo;
}

DWORD Api_UsbhWebCamVideoGetTotalResolutions(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwVideoStreamTotalFormatIndex;
}


DWORD Api_UsbhWebCamVideoGetResolutionByIndex(DWORD index, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	if (index < psUsbHostAVdc->dwVideoStreamTotalFormatIndex)
		return psUsbHostAVdc->dwVideoStreamSize[index];
	return 0;
}

DWORD Api_UsbhWebCamVideoGetCurResolution(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex];
}

DWORD Api_UsbhWebCamVideoGetCurResolutionIndex(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwVideoStreamCurrentFormatIndex;
}

void Api_UsbhWebCamVideoSetResolutionByIndex(DWORD index, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	if (index < psUsbHostAVdc->dwVideoStreamTotalFormatIndex)
		psUsbHostAVdc->dwVideoStreamCurrentFormatIndex = index;
}
////////////////////////////////////////////////////////////

DWORD webCamVideoGetStreamInterfaceEndPointNumber(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber;
}

DWORD webCamAudioGetStreamInterfaceEndPointNumber(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwAudioStreamInterfaceEndPointNumber;
}
#if 0
DWORD webCamVideoGetStreamInterfaceEndPointMaxPacketSize(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD maxPacketSize = 1;
	DWORD epPacketSize = psUsbHostAVdc->dwVideoStreamMaxPacketSize & 0x7ff;
	while(epPacketSize > maxPacketSize) maxPacketSize <<= 1;

	return maxPacketSize;
}

DWORD webCamVideoGetStreamInterfaceEndPointMult(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD mult = psUsbHostAVdc->dwVideoStreamMaxPacketSize & (BIT11|BIT12);

	if (mult & BIT11)
		return 2;
	else if (mult & BIT12)
		return 3;
	return 1;
}

DWORD webCamVideoGetStreamInterfaceEndPointMaxPayloadTransferSize(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwVideoStreamMaxPacketSize;
}

DWORD webCamAudioGetStreamInterfaceEndPointMaxPacketSize(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	DWORD maxPacketSize = 1;
	while(psUsbHostAVdc->dwAudioStreamMaxPacketSize > maxPacketSize) maxPacketSize <<= 1;
	return maxPacketSize;
}
#endif // #if 0

void webCamResetParam(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
	PUSB_HOST_UVC  psUsbhUvc = (PUSB_HOST_UVC)UsbOtgHostUvcDsGet(eWhichOtg);
	DWORD index, j;
    
    mpDebugPrint("%s:%d", __func__, __LINE__);
    psUsbhUvc->eNewFrame               = HAS_NO_FRAME;
    psUsbhUvc->dwFrameNumber           = 0;
    psUsbhUvc->dwOriginalFrameNumber   = 0;
    psUsbhUvc->dwLastFrameNumber       = 0;
    psUsbhUvc->bFrameID                = 0;
    psUsbhUvc->bStartOfFrame           = 0;
    psUsbhUvc->bNewOneFrame            = 0;    
    
	//psUsbHostAVdc->dwVideoStreamInterfaceNumber = 0;
	//psUsbHostAVdc->dwVideoStreamInterfaceAlternateSetting = 0;
	psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber = 0;
	//psUsbHostAVdc->dwVideoStreamMaxPacketSize = 0;

	psUsbHostAVdc->dwVideoStreamTotalFormatIndex = 0;
	psUsbHostAVdc->dwVideoStreamCurrentFormatIndex = 0;
	for (index = 0; index < MAX_NUM_VIDEO_FORMAT_INDEX; ++index)
	{
		psUsbHostAVdc->dwVideoStreamFormat[index]  = 0;
		psUsbHostAVdc->dwVideoStreamFormatIndex[index]  = 0;
		psUsbHostAVdc->dwVideoStreamFrameIndex[index]  = 0;
		psUsbHostAVdc->dwVideoStreamSize[index]  = 0;
        psUsbHostAVdc->sUvcFormatInfo[index].eFormat = USING_NONE;
        psUsbHostAVdc->sUvcFormatInfo[index].dwFormatIndex = 0;
        psUsbHostAVdc->sUvcFormatInfo[index].dwTotalFrameNumber= 0;
        for (j = 0; j < MAX_NUM_FRAME_INDEX; j++)
        {
            psUsbHostAVdc->sUvcFormatInfo[index].sFrameResolution[j].wHigh = 0;
            psUsbHostAVdc->sUvcFormatInfo[index].sFrameResolution[j].wWidth = 0;
        }
	}

	psUsbHostAVdc->dwAudioStreamInterfaceNumber = 0;
	psUsbHostAVdc->dwAudioStreamInterfaceAlternateSetting = 0;
	psUsbHostAVdc->dwAudioStreamInterfaceEndPointNumber = 0;
	psUsbHostAVdc->dwAudioStreamMaxPacketSize = 0;
	psUsbHostAVdc->dwAudioFormatType = 0;
	psUsbHostAVdc->dwAudioFreqRate = 0;
	psUsbHostAVdc->dwAudioSampleSize = 0;
}

// JL, 09302011
void webCamGetAudioVideoInfo(BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;
    
    pUsbConfig = GetConfigStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
/*/////////////////////////////
-- psUsbHostAVdc->dwVideoStreamTotalFormatIndex
-- psUsbHostAVdc->dwVideoStreamCurrentFormatIndex
-- psUsbHostAVdc->dwVideoStreamFormat
-- psUsbHostAVdc->dwVideoStreamFormatIndex
-- psUsbHostAVdc->dwVideoStreamSize
-- psUsbHostAVdc->dwVideoStreamMaxPacketSize
-- psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber
-- psUsbHostAVdc->dwVideoStreamInterfaceNumber
-- psUsbHostAVdc->dwVideoStreamInterfaceAlternateSetting
psUsbHostAVdc->dwAudioSampleSize
psUsbHostAVdc->dwAudioFormatType
psUsbHostAVdc->dwAudioFreqRate

/////////////////////////////*/


    PST_USB_OTG_HOST_ISOC_DES pIsocInEpDes;
    pIsocInEpDes = GetBestIsocDes(bInterfaceClass, bInteraceNumber, USB_DIR_IN, eWhichOtg);
    if (pIsocInEpDes == NULL)
    {
        MP_ALERT("%s:%d:pIsocInEpDes is NULL!", __FUNCTION__, __LINE__);
        return;
    }

#if 1
    pUsbhDevDes->bIsocAlternateSetting                      = pIsocInEpDes->bAlternativeSetting;    
    psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber     = pIsocInEpDes->bEdPointNumber;
    mpDebugPrint("-USBOTG%d- %s:wMaxPacketSize = %d", eWhichOtg,__FUNCTION__, pIsocInEpDes->wMaxPacketSize);
    mpDebugPrint("-USBOTG%d- %s:bEdPointNumber = %d", eWhichOtg,__FUNCTION__, pIsocInEpDes->bEdPointNumber);
    mpDebugPrint("-USBOTG%d- %s:bInterfaceNumber = %d", eWhichOtg,__FUNCTION__, pIsocInEpDes->bInterfaceNumber);
    mpDebugPrint("-USBOTG%d- %s:bAlternativeSetting = %d", eWhichOtg,__FUNCTION__, pIsocInEpDes->bAlternativeSetting);
#else
    pUsbhDevDes->bIsocAlternateSetting = pIsocInEpDes->bAlternativeSetting;    
    psUsbHostAVdc->dwVideoStreamMaxPacketSize               = pIsocInEpDes->wMaxPacketSize;
    psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber     = pIsocInEpDes->bEdPointNumber;
    psUsbHostAVdc->dwVideoStreamInterfaceNumber             = pIsocInEpDes->bInterfaceNumber;
    psUsbHostAVdc->dwVideoStreamInterfaceAlternateSetting   = pIsocInEpDes->bAlternativeSetting;
    mpDebugPrint("-USBOTG%d- %s:dwVideoStreamMaxPacketSize = %d", eWhichOtg,__FUNCTION__, psUsbHostAVdc->dwVideoStreamMaxPacketSize);
    mpDebugPrint("-USBOTG%d- %s:dwVideoStreamInterfaceEndPointNumber = %d", eWhichOtg,__FUNCTION__, psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber);
    mpDebugPrint("-USBOTG%d- %s:dwVideoStreamInterfaceNumber = %d", eWhichOtg,__FUNCTION__, psUsbHostAVdc->dwVideoStreamInterfaceNumber);
    mpDebugPrint("-USBOTG%d- %s:dwVideoStreamInterfaceAlternateSetting = %d", eWhichOtg,__FUNCTION__, psUsbHostAVdc->dwVideoStreamInterfaceAlternateSetting);
#endif
//__asm("break 100");
    BYTE inf_idx, alt_idx, ext_idx, k;
    USB_INTERFACE_DESCRIPTOR *pUsbInfDesc;    
    USB_EXTRA_DESCRIPTOR     *pUsbExtra;
    for (inf_idx = 0; inf_idx < pUsbConfig->bNumInterfaces; inf_idx++)
    {
        for (alt_idx = 0; alt_idx < pUsbConfig->pInterface[inf_idx].max_altsetting; alt_idx++)
        {
            pUsbInfDesc = GetInterfaceStruct (  pUsbhDevDes, 
                                                pUsbhDevDes->bDeviceConfigIdx,
                                                inf_idx,
                                                alt_idx);
            if (pUsbInfDesc->bInterfaceSubClass == SC_VIDEOSTREAMING)
            {
                for (ext_idx = 0; ext_idx < pUsbInfDesc->extra_num; ext_idx++)
                {
                    pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbInfDesc->extra+ext_idx);
                    if ((pUsbExtra->bDescriptorSubtype == VS_FORMAT_UNCOMPRESSED) ||
                        (pUsbExtra->bDescriptorSubtype == VS_FORMAT_MJPEG))
                    {
						psUsbHostAVdc->dwVideoStreamCurrentFormatIndex = psUsbHostAVdc->dwVideoStreamTotalFormatIndex++;
						psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = \
                            pUsbExtra->bDescriptorSubtype;
						psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = \
                            pUsbExtra->bData[0]; // index
                        
						MP_DEBUG("-format %x formatIndex %x",\
							psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],\
							psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex]);
////
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].eFormat = \
                            pUsbExtra->bDescriptorSubtype;
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].dwFormatIndex = \
                            psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex];
////
                        
                    }
                    else if ((pUsbExtra->bDescriptorSubtype == VS_FRAME_UNCOMPRESSED) ||
                            (pUsbExtra->bDescriptorSubtype == VS_FRAME_MJPEG))
                    {
						psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = \
                            pUsbExtra->bData[0]; // index
                        // Video Stream Size is (wWidth<<16 + wHeight)
						psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = \
                            ((pUsbExtra->bData[3]<<24) | (pUsbExtra->bData[2] << 16) | \
                             (pUsbExtra->bData[5]<<8)  | (pUsbExtra->bData[4]));
						MP_DEBUG("=format %x formatIndex %x frameIndex %x W %d H %d",
							psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],
							psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],
							psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex], 
							psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] >> 16,
							psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] & 0x0000ffff);
////
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].dwTotalFrameNumber = \
                            pUsbExtra->bData[0];
////
                        k = psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] - 1;
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].sFrameResolution[k].wFrameIndex = \
                            psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex];
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].sFrameResolution[k].wWidth = \
                            psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] >> 16;
                        psUsbHostAVdc->sUvcFormatInfo[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex].sFrameResolution[k].wHigh= \
							psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] & 0x0000ffff;
////
                    }
                    else if (pUsbExtra->bDescriptorSubtype == VS_FORMAT_MPEG2TS)
                    {
						mpDebugPrint("== VS_FORMAT_MPEG2TS");
                        // for MPEG2TS, has two informations bPacketLength and bStrideLength
                        // what's the action for?
                        //
                        //
                        //
                    }
                }
            }
        }
    }
////////////////////////////////
	return;
}

#if 0
DWORD webCamParseInterfaceForBestIsoEp(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	BYTE *pc = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;
	DWORD interfaceNum = 0;
	DWORD alternateSetting = 0;
	DWORD audioFormatType = 0;
	DWORD audioFreqRate = 0;
	WORD length;
	BOOL processVideoInterface = FALSE;
	BOOL processAudioInterface = FALSE;
	BOOL rtn = FALSE;

    length = byte_swap_of_word(*((WORD *)(pc+2)));
	do {
		switch (*(pc+1))
		{
			case 2:
				//mpDebugPrint("configuration");
				break;
			case 11:
				//mpDebugPrint("interface association");
				break;
			case 4:
				{
					USB_INTERFACE_DESCRIPTOR *pInterface = (USB_INTERFACE_DESCRIPTOR *)pc;

					processVideoInterface = FALSE;
					processAudioInterface = FALSE;
					interfaceNum = pInterface->bInterfaceNumber;
					alternateSetting = pInterface->bAlternateSetting;
					if (pInterface->bInterfaceClass == USB_CLASS_VIDEO && interfaceNum != 0)
					{
						processVideoInterface = TRUE;
						rtn = TRUE;
					}
					else if (pInterface->bInterfaceClass == USB_CLASS_AUDIO)
					{
						processAudioInterface = TRUE;
						rtn = TRUE;
					}
					memcpy( (BYTE*)&pUsbhDevDes->sInterfaceDescriptor[interfaceNum & 0x03],
								pc,
								USB_DT_INTERFACE_SIZE);
				}
				break;
			case 36:
				//mpDebugPrint("cs_interface");
				if (processAudioInterface && *(pc+2) == 1)
					audioFormatType = *(pc+5);
				else if (processAudioInterface && *(pc+2) == 2 && *(pc+3) == 1 && *(pc+7) == 1)
				{
					audioFreqRate = *(pc+10)*256*256 + *(pc+9)*256 + *(pc+8);
					psUsbHostAVdc->dwAudioSampleSize = *(pc+6);
				}
				else if (processVideoInterface)
				{
					if ((*(pc+2) == VS_FORMAT_MJPEG) || (*(pc+2) == VS_FORMAT_UNCOMPRESSED))
					{
						psUsbHostAVdc->dwVideoStreamCurrentFormatIndex = psUsbHostAVdc->dwVideoStreamTotalFormatIndex++;
						psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = *(pc+2);
						psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = *(pc+3);

						mpDebugPrint("-format %x formatIndex %x",
							psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],
							psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex]);

					}
					else if ((*(pc+2) == VS_FRAME_MJPEG) || (*(pc+2) == VS_FRAME_UNCOMPRESSED))
					{
						psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = *(pc+3);
						psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] = (*(pc+6)*256*256*256 + *(pc+5))*256*256 + *(pc+8)*256 + *(pc+7);
						mpDebugPrint("=format %x formatIndex %x frameIndex %x W %d H %d",
							psUsbHostAVdc->dwVideoStreamFormat[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],
							psUsbHostAVdc->dwVideoStreamFormatIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex],
							psUsbHostAVdc->dwVideoStreamFrameIndex[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex], 
							psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] >> 16,
							psUsbHostAVdc->dwVideoStreamSize[psUsbHostAVdc->dwVideoStreamCurrentFormatIndex] & 0x0000ffff);
					}

				}
				break;
			case 5:
				{
					USB_ENDPOINT_DESCRIPTOR *pEndPoint = (USB_ENDPOINT_DESCRIPTOR *)pc;

					if (pEndPoint->bEndpointAddress & BIT7)
					{
						memcpy((BYTE*)&pUsbhDevDes->sEndpointDescriptor[pEndPoint->bEndpointAddress & 0x07],
								pc,
								USB_DT_ENDPOINT_SIZE);

						if (processVideoInterface && ((pEndPoint->bmAttributes & 0x03) == OTGH_ED_ISO) && ((*(pc+5) * 256 + *(pc+4)) > psUsbHostAVdc->dwVideoStreamMaxPacketSize))
						{
							psUsbHostAVdc->dwVideoStreamInterfaceEndPointNumber = pEndPoint->bEndpointAddress & 0x0f;
							psUsbHostAVdc->dwVideoStreamMaxPacketSize = (*(pc+5) * 256 + *(pc+4));
							pUsbhDevDes->bDeviceInterfaceIdx = interfaceNum;
							psUsbHostAVdc->dwVideoStreamInterfaceNumber = interfaceNum;
							psUsbHostAVdc->dwVideoStreamInterfaceAlternateSetting = alternateSetting;
						}
						else if (processAudioInterface && ((pEndPoint->bmAttributes & 0x03) == OTGH_ED_ISO) && ((*(pc+5) * 256 + *(pc+4)) > psUsbHostAVdc->dwAudioStreamMaxPacketSize))
						{
							psUsbHostAVdc->dwAudioStreamInterfaceEndPointNumber = pEndPoint->bEndpointAddress & 0x0f;
							psUsbHostAVdc->dwAudioStreamMaxPacketSize = (*(pc+5) * 256 + *(pc+4));
							psUsbHostAVdc->dwAudioStreamInterfaceNumber = interfaceNum;
							psUsbHostAVdc->dwAudioStreamInterfaceAlternateSetting = alternateSetting;
							psUsbHostAVdc->dwAudioFormatType = audioFormatType;
							psUsbHostAVdc->dwAudioFreqRate = audioFreqRate;

							mpDebugPrint("webCamAudioFormatType %x", psUsbHostAVdc->dwAudioFormatType);
							mpDebugPrint("webCamAudioFreqRate %i", psUsbHostAVdc->dwAudioFreqRate);
						}
					}
				}
				break;
			case 37:
				//mpDebugPrint("cs_endpoint type");
				break;
			default:
				//mpDebugPrint("unknown type");
				break;
		}
		pc += *(pc);
	} while (pc < ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress + length));

	//psUsbHostAVdc->dwVideoStreamFrameIndex = 1; // default frame index

	return rtn;
}
#endif // #if 0

SWORD webCamVideoSetupSetInterface(WORD wInterfaceNumber, WORD wAlternateSetting, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err			= USB_NO_ERROR;

	SetupBuilder(	&ctrl_request,// 0x01 0B 00 00 01 00 00 00
					(USB_DIR_OUT |USB_RECIP_INTERFACE),
					USB_REQ_SET_INTERFACE,
					wAlternateSetting, 
					wInterfaceNumber,
					0x00);

	err = SetupCommand( &pUsbhDevDes->sSetupPB,
					0,
					0,
					0,
					&ctrl_request,
					eWhichOtg);

    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetInterface");
        MP_DEBUG1("err = %x", err);
    }
    return err;
}

SWORD webCamAudioSetupSetInterface(BOOL action, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err			= USB_NO_ERROR;

	mpDebugPrint("interface %x alter %x", psUsbHostAVdc->dwAudioStreamInterfaceNumber, psUsbHostAVdc->dwAudioStreamInterfaceAlternateSetting);

	if (psUsbHostAVdc->dwAudioStreamInterfaceNumber == 0)
		return 0;

	SetupBuilder(	&ctrl_request,
					(USB_DIR_OUT |USB_RECIP_INTERFACE),
					USB_REQ_SET_INTERFACE,
					action ? psUsbHostAVdc->dwAudioStreamInterfaceAlternateSetting : 0, 
					psUsbHostAVdc->dwAudioStreamInterfaceNumber,
					0x00);

	err = SetupCommand( &pUsbhDevDes->sSetupPB,
					0,
					0,
					0,
					&ctrl_request,
					eWhichOtg);

    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetInterface");
        MP_DEBUG1("err = %x", err);
    }
    return err;
}

SWORD webCamSetupSetCmd(BYTE *cmdData, BYTE bRequest, WORD wValue, WORD wIndex, WORD wLength, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err			= USB_NO_ERROR;

	SetupBuilder(	&ctrl_request,
					(USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE),
					bRequest,
					wValue, 
					wIndex,
					wLength);

#if USBOTG_WEB_CAM
    if (g_print_led)
        MP_ALERT("cmdData 0x%08X", cmdData);
#endif
	err = SetupCommand( &pUsbhDevDes->sSetupPB,
					0,
					wLength,
					(DWORD)cmdData,
					&ctrl_request,
					eWhichOtg);

    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupSetInterface");
        MP_DEBUG1("err = %x", err);
    }
    return err;
}

SWORD webCamSetupGetCmd(BYTE bRequest, WORD wValue, WORD wIndex, WORD wLength, WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes	= (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    DWORD	data_addr;
    USB_CTRL_REQUEST	ctrl_request;
    SWORD err			= USB_NO_ERROR;

    data_addr  = (DWORD)allo_mem(wLength,eWhichOtg);
	SetupBuilder(	&ctrl_request,
					(USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE),
					bRequest,
					wValue, 
					wIndex,
					wLength);

    err = SetupCommand( &pUsbhDevDes->sSetupPB,
                        wLength,
                        0,
                        data_addr,
                        &ctrl_request,
                        eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_ALERT("-E- %s:error %x", __func__, err);
    }
    return err;
}


#if 0
DWORD webCamAudioGetType(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwAudioFormatType;
}

DWORD webCamAudioGetFreqRate(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwAudioFreqRate;
}

DWORD webCamAudioGetSampleSize(WHICH_OTG eWhichOtg)
{
	PUSB_HOST_AVDC psUsbHostAVdc = (PUSB_HOST_AVDC)UsbOtgHostAvdcDsGet(eWhichOtg);

	return psUsbHostAVdc->dwAudioSampleSize;
}
#endif //#if 0
#endif //#if USBOTG_WEB_CAM


#if USBOTG_HOST_DESC // Dynamic Configuration Descriptor

void UsbOtgHostSetDeviceDescriptor(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_HOST            psHost = (PST_USB_OTG_HOST)UsbOtgHostDsGet(eWhichOtg);
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pAppClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
    PST_HUB_CLASS_DESCRIPTOR    pHubClassDes = (PST_HUB_CLASS_DESCRIPTOR)psHost->psHubClassDescriptor;
    PUSB_ENDPOINT_DESCRIPTOR pEpDesc;
    BYTE    num_of_interface = 0, bCnt, bInterfaceClass = 0, bCntEp, bEpCnt = 0;

    num_of_interface = GetConfigNumInterface(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);

    if(num_of_interface > 1)  // Multi-Interface
    {
        pUsbhDevDes->bDeviceConfigVal   = GetConfigConfigurationValue(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);

        // Set bDeviceInterfaceIdx & bXXXQHDArrayNum etc for your wanted EPs.
        switch(pUsbhDevDes->sDeviceDescriptor.bDeviceClass)
        {
            case USB_CLASS_COMM:  // For 3.5G
                #if USBOTG_HOST_CDC
                UsbhCdcGetqHDArrayNum(eWhichOtg);
                #endif
                break;
#if USBOTG_HOST_DATANG
            case USB_CLASS_PER_INTERFACE:
			UsbhDatangGetqHDArrayNum(eWhichOtg);	
			break;
#endif

            default:
                MP_ALERT("--W-- %s Multi-Interface-Device", __FUNCTION__); // Take care endpoints for class of Multi-Interface.
                break;
        }
                
        // Set AppClass by Class of Device 
        SetAppClass(pUsbhDevDes, pAppClassDes, pHubClassDes);
    }

    MP_DEBUG("%s InterfaceIdx %d ConfigVal %d\r\n qHDArrayNum : BulkIn %d BulkOut %d IntIn %d IntOut %d",
                __FUNCTION__, 
                pUsbhDevDes->bDeviceInterfaceIdx,
                pUsbhDevDes->bDeviceConfigVal,
                pAppClassDes->bBulkInQHDArrayNum,
                pAppClassDes->bBulkOutQHDArrayNum,
                pAppClassDes->bIntInQHDArrayNum,
                pAppClassDes->bIntOutQHDArrayNum);

}


BOOL DumpConfigDescriptorInfo(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    BYTE *pData = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;
    BYTE total_of_config    = 0; 
    BYTE total_of_interface = 0;
    BYTE total_of_endpoint  = 0;
    BYTE total_of_extra     = 0;
    BYTE len_of_extra       = 0;    
    BYTE bCFCnt, bIFCnt, bEPCnt, bExtra, bASCnt;
    USB_CONFIG_DESCRIPTOR    *pUsbConfig;
    USB_INTERFACE_DESCRIPTOR *pUsbIFDesc;
    USB_INTERFACE            *pUsbInterface;
    USB_ENDPOINT_DESCRIPTOR  *pUsbEndpoint;
    USB_EXTRA_DESCRIPTOR     *pUsbExtra;
    PUVC_GET_REQUEST_OF_PROCESSING pUvcReq;
    BYTE control_units = 0;
    BYTE isDone = 0;
    BYTE i = 0;

    MP_DEBUG("===================================");
    MP_DEBUG(" Dump GetDescriptor(Configuration) ");
    MP_DEBUG("===================================");   

    pUvcReq = pUsbhDevDes->psUvcReq;
    total_of_config = pUsbhDevDes->sDeviceDescriptor.bNumConfigurations;
    for(bCFCnt = 0; bCFCnt < total_of_config; bCFCnt++)
    {
        // Configuration descriptor
        pUsbConfig = (USB_CONFIG_DESCRIPTOR *)(pUsbhDevDes->pConfigDescriptor+bCFCnt);
        MP_DEBUG("Config Desc. [%02d]     / Dec  / Hex", bCFCnt);
        MP_DEBUG("bLength               / %04d / 0x%02x", pUsbConfig->bLength, pUsbConfig->bLength);
        MP_DEBUG("bDescriptorType       / %04d / 0x%02x", pUsbConfig->bDescriptorType, pUsbConfig->bDescriptorType);
        MP_DEBUG("wTotalLength          / %04d / 0x%04x", 0xFFFF & byte_swap_of_word(pUsbConfig->wTotalLength), 0xFFFF & byte_swap_of_word(pUsbConfig->wTotalLength));
        MP_DEBUG("bNumInterfaces        / %04d / 0x%02x", pUsbConfig->bNumInterfaces, pUsbConfig->bNumInterfaces);
        MP_DEBUG("bConfigurationValue   / %04d / 0x%02x", pUsbConfig->bConfigurationValue, pUsbConfig->bConfigurationValue);
        MP_DEBUG("iConfiguration        / %04d / 0x%02x", pUsbConfig->iConfiguration, pUsbConfig->iConfiguration);
        MP_DEBUG("bmAttributes          / %04d / 0x%02x", pUsbConfig->bmAttributes, pUsbConfig->bmAttributes);
        MP_DEBUG("bMaxPower             / %04d / 0x%02x", pUsbConfig->bMaxPower, pUsbConfig->bMaxPower);

        // Interface descriptor
        total_of_interface = pUsbConfig->bNumInterfaces;
        for(bIFCnt = 0; bIFCnt < total_of_interface; bIFCnt++)
        {
            pUsbInterface = (USB_INTERFACE *)(pUsbConfig->pInterface+bIFCnt);

            for(bASCnt = 0; bASCnt < pUsbInterface->max_altsetting; bASCnt++)
            {
                pUsbIFDesc = (pUsbInterface->altsetting+bASCnt);
                MP_DEBUG("===================================");
                MP_DEBUG("Interface Desc. [%02d]  / Dec  / Hex ", bIFCnt);
                MP_DEBUG("AlternateSetting[%02d]  / Dec  / Hex ", bASCnt);
                MP_DEBUG("bLength               / %04d / 0x%02x", pUsbIFDesc->bLength, pUsbIFDesc->bLength);
                MP_DEBUG("bDescriptorType       / %04d / 0x%02x", pUsbIFDesc->bDescriptorType, pUsbIFDesc->bDescriptorType);
                MP_DEBUG("bInterfaceNumber      / %04d / 0x%02x", pUsbIFDesc->bInterfaceNumber, pUsbIFDesc->bInterfaceNumber);
                MP_DEBUG("bAlternateSetting     / %04d / 0x%02x", pUsbIFDesc->bAlternateSetting, pUsbIFDesc->bAlternateSetting);
                MP_DEBUG("bNumEndpoints         / %04d / 0x%02x", pUsbIFDesc->bNumEndpoints, pUsbIFDesc->bNumEndpoints);
                MP_DEBUG("bInterfaceClass       / %04d / 0x%02x", pUsbIFDesc->bInterfaceClass, pUsbIFDesc->bInterfaceClass);
                MP_DEBUG("bInterfaceSubClass    / %04d / 0x%02x", pUsbIFDesc->bInterfaceSubClass, pUsbIFDesc->bInterfaceSubClass);
                MP_DEBUG("bInterfaceProtocol    / %04d / 0x%02x", pUsbIFDesc->bInterfaceProtocol, pUsbIFDesc->bInterfaceProtocol);
                MP_DEBUG("iInterface            / %04d / 0x%02x", pUsbIFDesc->iInterface, pUsbIFDesc->iInterface);

                // Extra descriptor
                total_of_extra = pUsbIFDesc->extra_num;
                for(bEPCnt = 0; bEPCnt < total_of_extra; bEPCnt++)
                {
                    pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbIFDesc->extra+bEPCnt);
                    MP_DEBUG("===================================");
                    MP_DEBUG("Extra Desc. [%02d]      / Dec  / Hex ", bEPCnt);
                    MP_DEBUG("bFunctionLength       / %04d / 0x%02x", pUsbExtra->bFunctionLength, pUsbExtra->bFunctionLength);
                    MP_DEBUG("bDescriptorType       / %04d / 0x%02x", pUsbExtra->bDescriptorType, pUsbExtra->bDescriptorType);
                    MP_DEBUG("bDescriptorSubtype    / %04d / 0x%02x", pUsbExtra->bDescriptorSubtype, pUsbExtra->bDescriptorSubtype);
//////////////
                    if ((pUsbIFDesc->bInterfaceClass   == CC_VIDEO) && \
                        (pUsbExtra->bDescriptorType    == CS_INTERFACE) && \
                        (pUsbExtra->bDescriptorSubtype == VC_PROCESSING_UNIT) && \
                        (isDone == 0))
                    {
                        isDone = 1;
                        control_units = *(pUsbExtra->bData+4)*8;
                        pUsbhDevDes->sUvcProcessUnit.bControlUnits = control_units;
                        MP_DEBUG("control_units %d", control_units);
                        //pUvcReq = (PUVC_GET_REQUEST_OF_PROCESSING)ker_mem_malloc(control_units*sizeof(UVC_GET_REQUEST_OF_PROCESSING), TaskGetId());                  
                        pUsbhDevDes->psUvcReq = (PUVC_GET_REQUEST_OF_PROCESSING)ker_mem_malloc(control_units*sizeof(UVC_GET_REQUEST_OF_PROCESSING), TaskGetId());
                        pUvcReq = pUsbhDevDes->psUvcReq;
                        if (pUvcReq == 0)
                        {
                            MP_ALERT("%s:%d:ker_mem_malloc for pUvcReq failed!!", __func__, __LINE__);
                            return;
                        }

                        MP_DEBUG("pUvcReq 0x%x", pUvcReq);
                        MP_DEBUG("sInfo* 0x%x", &pUvcReq[0].sInfo);
                        MP_DEBUG("bStatus* 0x%x", &pUvcReq[0].wStatus);
                        MP_DEBUG("wMin* 0x%x", &pUvcReq[0].wMin);
                        MP_DEBUG("wMax* 0x%x", &pUvcReq[0].wMax);
                        MP_DEBUG("wRes* 0x%x", &pUvcReq[0].wRes);
                        MP_DEBUG("wDef* 0x%x", &pUvcReq[0].wDef);
                        memset ((BYTE*) pUvcReq, 0, control_units*sizeof(UVC_GET_REQUEST_OF_PROCESSING));
                        MP_DEBUG("sInfo %d", pUvcReq[0].sInfo);
                        MP_DEBUG("bStatus %d", pUvcReq[0].wStatus);
                        MP_DEBUG("wMin %d", pUvcReq[0].wMin);
                        MP_DEBUG("wMax %d", pUvcReq[0].wMax);
                        MP_DEBUG("wRes %d", pUvcReq[0].wRes);
                        MP_DEBUG("wDef %d", pUvcReq[0].wDef);
                        pUsbhDevDes->sUvcProcessUnit.bUnitID = *(pUsbExtra->bData);
                        memcpy ( (BYTE*)&pUsbhDevDes->sUvcProcessUnit.sUnit, (pUsbExtra->bData+5), 2);
                        
                        MP_DEBUG("pUvcReq size %d", control_units*sizeof(UVC_GET_REQUEST_OF_PROCESSING));
                        MP_DEBUG("bUnitID %d", pUsbhDevDes->sUvcProcessUnit.bUnitID);
                        MP_DEBUG("--------------------------------");
                        MP_DEBUG("bData[8]             / %04d / 0x%02x", *(pUsbExtra->bData+5), *(pUsbExtra->bData+5));
                        MP_DEBUG("bData[9]             / %04d / 0x%02x", *(pUsbExtra->bData+6), *(pUsbExtra->bData+6));
                        MP_DEBUG("bmBrightness              / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmBrightness);
                        MP_DEBUG("bmContrast                / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmContrast);
                        MP_DEBUG("bmHue                     / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmHue);
                        MP_DEBUG("bmSaturation              / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmSaturation);
                        MP_DEBUG("bmSharpness               / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmSharpness);
                        MP_DEBUG("bmGamma                   / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmGamma);
                        MP_DEBUG("bmWbt                     / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbt);
                        MP_DEBUG("bmWbc                     / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbc);
                        MP_DEBUG("bmBacklightCompensation   / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmBacklightCompensation);
                        MP_DEBUG("bmGain                    / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmGain);
                        MP_DEBUG("bmPowerLineFreq           / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmPowerLineFreq);
                        MP_DEBUG("bmHueAuto                 / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmHueAuto);
                        MP_DEBUG("bmWbtAuto                 / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbtAuto);
                        MP_DEBUG("bmWbcAuto                 / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmWbcAuto);
                        MP_DEBUG("bmDigitalMultiplier       / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmDigitalMultiplier);
                        MP_DEBUG("bmDigitalMultiplierLimit  / %04d", pUsbhDevDes->sUvcProcessUnit.sUnit.bmDigitalMultiplierLimit);
                        MP_DEBUG("--------------------------------");
                        
                    }
//////////////
                    // Extra bData
                    len_of_extra = pUsbExtra->bFunctionLength - USB_DT_EXTRA_SIZE;
                    for(bExtra = 0; bExtra < len_of_extra; bExtra++)
                        MP_DEBUG("bData[%02d]             / %04d / 0x%02x", bExtra + USB_DT_EXTRA_SIZE, *(pUsbExtra->bData+bExtra), *(pUsbExtra->bData+bExtra));
                }

                // Endpoint descriptor
                total_of_endpoint = pUsbIFDesc->bNumEndpoints;
                for(bEPCnt = 0; bEPCnt < total_of_endpoint; bEPCnt++)
                {
                    pUsbEndpoint = (USB_ENDPOINT_DESCRIPTOR *)(pUsbIFDesc->pEndpoint+bEPCnt);
                    MP_DEBUG("===================================");
                    MP_DEBUG("Endpoint Desc. [%02d]   / Dec  / Hex ", bEPCnt);
                    MP_DEBUG("bLength               / %04d / 0x%02x", pUsbEndpoint->bLength, pUsbEndpoint->bLength);
                    MP_DEBUG("bDescriptorType       / %04d / 0x%02x", pUsbEndpoint->bDescriptorType, pUsbEndpoint->bDescriptorType);
                    MP_DEBUG("bEndpointAddress      / %04d / 0x%02x", pUsbEndpoint->bEndpointAddress, pUsbEndpoint->bEndpointAddress);
                    MP_DEBUG("bmAttributes          / %04d / 0x%02x", pUsbEndpoint->bmAttributes, pUsbEndpoint->bmAttributes);
                    MP_DEBUG("wMaxPacketSize        / %04d / 0x%02x", 0xFFFF & byte_swap_of_word(pUsbEndpoint->wMaxPacketSize), 0xFFFF & byte_swap_of_word(pUsbEndpoint->wMaxPacketSize));
                    MP_DEBUG("bInterval             / %04d / 0x%02x", pUsbEndpoint->bInterval, pUsbEndpoint->bInterval);            
                }
            }
        }
        MP_DEBUG("===================================");
    }
    
    isDone = 0;
}

BOOL DumpUsbInterfaceInfo(USB_INTERFACE_NUMBER *pUsbIFNum, BYTE bUsbIFs)
{
    BYTE bCnt, bCount;
    USB_INTERFACE_NUMBER *pUsbIFNumTmp;

    MP_DEBUG("\n");
    for(bCnt = 0; bCnt < bUsbIFs; bCnt++)
    {
        pUsbIFNumTmp= (USB_INTERFACE_NUMBER *)(pUsbIFNum+bCnt);
        MP_DEBUG("=== bInterfaceNumber [%02d] / Total Altsettings : %02d ===", bCnt, pUsbIFNumTmp->bAltsettings);
        for(bCount = 0; bCount < pUsbIFNumTmp->bAltsettings; bCount++)
        {
            MP_DEBUG("Altsetting [%02d] / Total Extra : %02d", bCount, pUsbIFNumTmp->ExtraFuncs[bCount]);
        }
        MP_DEBUG("\n");
    }
}

// Parse Config Descriptor for *pConfigDescriptor
BOOL ParseConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    BYTE *pData = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;
    WORD wTotalLength;
    BYTE num_of_config      = 0; 
    BYTE num_of_interface   = 0;
    BYTE total_of_interface = 0;
    BYTE num_of_alternate   = 0;
    BYTE num_of_endpoint    = 0;
    BYTE num_of_extra       = 0;
    BYTE len_of_extra       = 0;
    BYTE bCnt               = 0;

    USB_INTERFACE_NUMBER     *pUsbIFNum, *pUsbIFNumTmp;
    USB_CONFIG_DESCRIPTOR    *pUsbConfig;
    USB_INTERFACE_DESCRIPTOR *pUsbIFDesc;
    USB_INTERFACE            *pUsbInterface;
    USB_ENDPOINT_DESCRIPTOR  *pUsbEndpoint;
    USB_EXTRA_DESCRIPTOR     *pUsbExtra; // , *pUsbExtraTmp, *pUsbExtraTemp;
    
    wTotalLength = byte_swap_of_word(*((WORD *)(pData+2))); 
    
    MP_DEBUG("GetDescriptor (Configuration) length %d", wTotalLength);   

    // First Pass : Get IF-AlternateSetting & Extra-Function Count
    do {
        switch (*(pData+1)) // check bDescriptorType
        {
            case USB_DT_CONFIG:

                total_of_interface = *(pData+4);
                pUsbIFNum = (USB_INTERFACE_NUMBER *)ker_mem_malloc(total_of_interface*sizeof(USB_INTERFACE_NUMBER), TaskGetId()); // *(pData+4) : bNumInterface

                break;
            
            case USB_DT_INTERFACE:

                pUsbIFNumTmp = (USB_INTERFACE_NUMBER *)(pUsbIFNum+(*(pData+2))); // *(pData+2) : bInterfaceNumber

                num_of_alternate = *(pData+3); // bAlternateSetting
                pUsbIFNumTmp->bAltsettings = num_of_alternate + 1; // +1:Interface descriptor
                pUsbIFNumTmp->ExtraFuncs[num_of_alternate] = 0;

                break;
            
                case USB_DT_CS_INTERFACE:
                case USB_DT_CS_ENDPOINT:
                #if USBOTG_HOST_HID
                case USB_DT_HID:                  // HID
                #endif

                    pUsbIFNumTmp->ExtraFuncs[num_of_alternate] += 1;

                    break;
            
                case USB_DT_ENDPOINT:               // ENDPOINT
                case USB_DT_HUB:                    // HUB 
                case USB_DT_INTERFACE_ASSOCIATION:  // INTERFACE_ASSOCIATION
                    // Nothing
                    break;

                default:
                    mpDebugPrint("--E-- %s FirstPass unknown bDescriptotType : 0x%x !!", __FUNCTION__, *(pData+1));
                    //return FALSE; // ALERT Error Msg and Nothing to Continue by Mark this 
                    break;
                }
                pData += *(pData); // Next Discriptor by before Discriptor bLength
    } while (pData < ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress + wTotalLength));

    DumpUsbInterfaceInfo(pUsbIFNum, total_of_interface);

    // Second Pass : Contruct the descriptor structure
    pData = (BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress;
    
    do 
    {
        switch (*(pData+1)) // check bDescriptorType
        {
            case USB_DT_CONFIG: // CONFIG
                num_of_config = *(pData+5); // bConfigurationValue start from 1
                if(num_of_config > 0)
                    num_of_config -= 1; // Array index start from 0
                else
                    MP_ALERT("\n-E- bConfigurationValue is not correct!!");

                MP_DEBUG("CONFIG %d", num_of_config);                              
                // Write Data to Configuration descriptor
                pUsbConfig = (USB_CONFIG_DESCRIPTOR *)(pUsbhDevDes->pConfigDescriptor+num_of_config);
                memcpy( (BYTE*)pUsbConfig, pData, USB_DT_CONFIG_SIZE);
                // Alloc Interface Descriptor x N
                pUsbConfig->pInterface = (USB_INTERFACE *)ker_mem_malloc(pUsbConfig->bNumInterfaces * sizeof(USB_INTERFACE), TaskGetId());
                MP_DEBUG("[Interface Alloc Address] 0x%x", pUsbConfig->pInterface); // Check free memory after plug-out : address as same as re-plug

                break;
            
            case USB_DT_INTERFACE: // INTERFACE

                // Current Interface descriptor Index
                num_of_interface = *(pData+2); // bInterfaceNumber
                MP_DEBUG("INTERFACE %d", num_of_interface);

                // IF-AlternateSetting & Extra-Function Data
                pUsbIFNumTmp = (USB_INTERFACE_NUMBER *)(pUsbIFNum+num_of_interface);                

                // Write Data to Interface descriptor
                pUsbInterface = (USB_INTERFACE *)(pUsbConfig->pInterface+num_of_interface);
                pUsbInterface->act_altsetting = 0; // Decision by Device 
                pUsbInterface->num_altsetting = *(pData+3);
                pUsbInterface->max_altsetting = pUsbIFNumTmp->bAltsettings;

                // AlternateSetting IF
                if(*(pData+3) == 0) // *(pData+3) : bAlternateSetting
                    pUsbInterface->altsetting = (USB_INTERFACE_DESCRIPTOR *)ker_mem_malloc(pUsbIFNumTmp->bAltsettings * sizeof(USB_INTERFACE_DESCRIPTOR), TaskGetId());
                
                pUsbIFDesc = (USB_INTERFACE_DESCRIPTOR *)(pUsbInterface->altsetting+*(pData+3)); // *(pData+3) : bAlternateSetting
                memcpy((BYTE*)pUsbIFDesc, pData, USB_DT_INTERFACE_SIZE);
                pUsbIFDesc->extra_num = pUsbIFNumTmp->ExtraFuncs[*(pData+3)];

                // Alloc Endpoint/ExtraFunc descriptor x N
                if(pUsbIFDesc->bNumEndpoints > 0)
                {
                    pUsbIFDesc->pEndpoint = (USB_ENDPOINT_DESCRIPTOR  *)ker_mem_malloc(pUsbIFDesc->bNumEndpoints * sizeof(USB_ENDPOINT_DESCRIPTOR), TaskGetId());
                    num_of_endpoint = 0;
                }
                if(pUsbIFDesc->extra_num > 0)
                {
                    pUsbIFDesc->extra = (USB_EXTRA_DESCRIPTOR  *)ker_mem_malloc(pUsbIFDesc->extra_num * sizeof(USB_ENDPOINT_DESCRIPTOR), TaskGetId());
                    num_of_extra = 0;
                }
                
                break;
                
            case USB_DT_CS_INTERFACE:  // CS_INTERFACE
            case USB_DT_CS_ENDPOINT:   // CS_ENDPOINT
            #if USBOTG_HOST_HID
            case USB_DT_HID:                  // HID
            #endif

                // Current CS_Extra descriptor Index
                MP_DEBUG("CS_Extra %d", num_of_extra);

                // Write Extra Function Data[0~2]
                pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbIFDesc->extra+num_of_extra);
                memcpy((BYTE*)pUsbExtra, pData, USB_DT_EXTRA_SIZE);

                // Write Extra Function Data[3~N]
                len_of_extra = pUsbExtra->bFunctionLength - USB_DT_EXTRA_SIZE;
                pUsbExtra->bData = (BYTE *)ker_mem_malloc(len_of_extra, TaskGetId());
                memcpy((BYTE*)pUsbExtra->bData, (pData+USB_DT_EXTRA_SIZE), len_of_extra);

                num_of_extra += 1;
                
                break; 

            case USB_DT_ENDPOINT:  // ENDPOINT

                // Current Endpoint descriptor Index
                MP_DEBUG("ENDPOINT %d", num_of_endpoint);

                // Write Data to Endpoint descriptor
                pUsbEndpoint = (USB_ENDPOINT_DESCRIPTOR *)(pUsbIFDesc->pEndpoint+num_of_endpoint);
                memcpy((BYTE*)pUsbEndpoint, pData, USB_DT_ENDPOINT_SIZE);

                num_of_endpoint += 1;
                
                break; 

            case USB_DT_HUB:                    // HUB
            case USB_DT_INTERFACE_ASSOCIATION:  // INTERFACE_ASSOCIATION        
                // Nothing  // No Record
                break;                

            default:
                MP_ALERT("--E-- %s SecondPass unknown bDescriptotType : 0x%x !!", __FUNCTION__, *(pData+1));
                //return FALSE; // ALERT Error Msg and Nothing to Continue by Mark this 
                break;
        }
        pData += *(pData); // Next Discriptor by before Discriptor bLength
    } while (pData < ((BYTE*)pUsbhDevDes->sSetupPB.dwDataAddress + wTotalLength));

    DumpConfigDescriptorInfo(pUsbhDevDes); // Dump Configurature Descriptor Structure Data

    ker_mem_free(pUsbIFNum); // Free USB_INTERFACE_NUMBER Structure

    return TRUE; // Should return TRUE : Pure Dynamic Configuration Descriptor

}

BOOL SetAppClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
                        PST_APP_CLASS_DESCRIPTOR    pAppClassDes,
                        PST_HUB_CLASS_DESCRIPTOR    pHubClassDes)
{
    PUSB_ENDPOINT_DESCRIPTOR pEpDesc;
    BYTE bEpCnt = 0, i = 0, bUsbDir = 0;

    bEpCnt = GetInterfaceEPs(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);

    for(i = 0; i < bEpCnt; i++)
    {
        pEpDesc = GetEndpointStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx, i);

        // Set Bulk EP of AppClass
        if(pEpDesc->bmAttributes == USB_ENDPOINT_XFER_BULK)
        {
            bUsbDir = pEpDesc->bEndpointAddress & USB_ENDPOINT_DIR_MASK;
        
            if (bUsbDir == USB_DIR_OUT)
            {
                memcpy( (BYTE*)&pAppClassDes->sBulkOutDescriptor,
                        (BYTE*)pEpDesc,
                        USB_DT_ENDPOINT_SIZE);
            }
            else if (bUsbDir == USB_DIR_IN)
            {
                memcpy( (BYTE*)&pAppClassDes->sBulkInDescriptor,
                        (BYTE*)pEpDesc,
                        USB_DT_ENDPOINT_SIZE);
            }
        }

        // Set INT EP of AppClass


        // Set ISO EP of AppClass


    }
}

void InitConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    pUsbhDevDes->pConfigDescriptor = NULL; 
}

BOOL AllocConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    pUsbhDevDes->pConfigDescriptor = (USB_CONFIG_DESCRIPTOR *)ker_mem_malloc(pUsbhDevDes->sDeviceDescriptor.bNumConfigurations * sizeof(USB_CONFIG_DESCRIPTOR), TaskGetId());
    memset ((BYTE*) pUsbhDevDes->pConfigDescriptor, 0, pUsbhDevDes->sDeviceDescriptor.bNumConfigurations * sizeof(USB_CONFIG_DESCRIPTOR));
    pUsbhDevDes->pConfigDescriptor->bLength = USB_DT_CONFIG_SIZE;
    pUsbhDevDes->pConfigDescriptor->wTotalLength = 0;
    pUsbhDevDes->pConfigDescriptor->pInterface = NULL; // Not alloc pInterface
    MP_DEBUG("%s:[Config Alloc Address] 0x%x", __FUNCTION__, pUsbhDevDes->pConfigDescriptor); // Check free memory after plug-out : address as same as re-plug
}

BOOL FreeConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes)
{
    BYTE total_of_config    = 0;
    BYTE total_of_interface = 0;
    BYTE bCFCnt, bIFCnt, bASCnt = 0, bExtraCnt;
    USB_CONFIG_DESCRIPTOR    *pUsbConfig;
    USB_INTERFACE_DESCRIPTOR *pUsbIFDesc;
    USB_INTERFACE            *pUsbInterface;
    USB_EXTRA_DESCRIPTOR     *pUsbExtra;    

    MP_DEBUG("%s:[Config Free Address] 0x%x", __FUNCTION__, pUsbhDevDes->pConfigDescriptor);

    if(pUsbhDevDes->pConfigDescriptor == NULL) // Need to free by checking address
        return FALSE;

    // Configuration descriptor
    total_of_config = pUsbhDevDes->sDeviceDescriptor.bNumConfigurations;
    for(bCFCnt = 0; bCFCnt < total_of_config; bCFCnt++)
    {
        // Configuration descriptor
        pUsbConfig = (USB_CONFIG_DESCRIPTOR *)(pUsbhDevDes->pConfigDescriptor+bCFCnt);
        if(pUsbConfig == NULL)
        {
            MP_ALERT("--W-- %s:pUsbConfig is NULL!", __FUNCTION__);
            pUsbhDevDes->pConfigDescriptor = NULL;
            return FALSE;
        }
        // Interface descriptor
        total_of_interface = pUsbConfig->bNumInterfaces;
        for(bIFCnt = 0; bIFCnt < total_of_interface; bIFCnt++)
        {
            pUsbInterface = (USB_INTERFACE *)(pUsbConfig->pInterface+bIFCnt);
            if(pUsbInterface == NULL)
            {
                MP_ALERT("--W-- %s:pUsbInterface is NULL!", __FUNCTION__);
                pUsbhDevDes->pConfigDescriptor = NULL;
                return FALSE;
            }
            for(bASCnt = 0; bASCnt < pUsbInterface->max_altsetting; bASCnt++)
            {
                pUsbIFDesc = (pUsbInterface->altsetting+bASCnt);
                if(pUsbIFDesc == NULL)
                {
                    MP_ALERT("--W-- %s:pUsbIFDesc is NULL!", __FUNCTION__);
                    pUsbhDevDes->pConfigDescriptor = NULL;
                    return FALSE;
                }
                // Extra descriptor
                if(pUsbIFDesc->extra_num > 0)
                {
                    for(bExtraCnt = 0; bExtraCnt < pUsbIFDesc->extra_num; bExtraCnt++)
                    {
                        pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbIFDesc->extra+bExtraCnt);
                        ker_mem_free(pUsbExtra->bData);
                    }
                    ker_mem_free(pUsbIFDesc->extra);
                }
                
                // Endpoint descriptor
                if(pUsbIFDesc->bNumEndpoints > 0)
                    ker_mem_free(pUsbIFDesc->pEndpoint);
            }

            // Alternate Setting descriptor
            if(pUsbInterface->max_altsetting > 0)
                ker_mem_free(pUsbInterface->altsetting);
        }

        // Interface descriptor
        if(total_of_interface > 0)
            ker_mem_free(pUsbConfig->pInterface);   
    }

    // Configuration descriptor
    if(total_of_config > 0)
        ker_mem_free(pUsbhDevDes->pConfigDescriptor);

    pUsbhDevDes->pConfigDescriptor = NULL;

    return TRUE;
        
}

PUSB_CONFIG_DESCRIPTOR GetConfigStruct(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx)
{
    if(pUsbhDevDes->pConfigDescriptor == NULL) // Need to free by checking address
    {
        MP_ALERT("--E-- %s pConfigDescriptor Not Create!!", __FUNCTION__);
        return (PUSB_CONFIG_DESCRIPTOR)NULL;
    }
    
    if(pUsbhDevDes->sDeviceDescriptor.bNumConfigurations >= ConfigIdx+1)
    {
        return (pUsbhDevDes->pConfigDescriptor+ConfigIdx);
    }
    else
    {
        MP_ALERT("--E-- %s No-Config-Idx: %d", __FUNCTION__, ConfigIdx);
        return NULL;
    }
}

// If no AlternateSetting, please set AlterSettingIdx = 0 and will get Interface Descriptor
PUSB_INTERFACE_DESCRIPTOR GetInterfaceStruct ( PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
                                               BYTE ConfigIdx,
                                               BYTE InterfaceIdx,
                                               BYTE AlterSettingIdx)
{
    USB_CONFIG_DESCRIPTOR    *pUsbConfig;
    USB_INTERFACE            *pUsbInterface;  
    
    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return NULL;


    if(pUsbConfig->bNumInterfaces >= InterfaceIdx+1)
    {
        pUsbInterface = pUsbConfig->pInterface+InterfaceIdx;
        if(pUsbInterface == 0x0) // Not alloc pUsbInterface->altsetting
            return NULL;
        else
            return (pUsbInterface->altsetting+AlterSettingIdx);
    }
    else
    { 
        if(pUsbhDevDes->bDeviceStatus >= USB_STATE_CONFIGURED) // After parse descriptor
            MP_ALERT("--E--%s No-Interface-Idx: %d", __FUNCTION__, InterfaceIdx);
        
        return NULL;
    }
}

PUSB_ENDPOINT_DESCRIPTOR GetEndpointStruct(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;  
    
    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return NULL;

    pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx);

    if(pUsbIFDesc == NULL)
        return NULL;    


    if(pUsbIFDesc->bNumEndpoints >= EndpointIdx+1)
    {
        return (pUsbIFDesc->pEndpoint+EndpointIdx);
        
    }
    else
    {
        MP_ALERT("--E-- %s No-Endpoint-Idx: %d", __FUNCTION__, EndpointIdx);
        return NULL;
    }
}

// Configuration Descriptor Member
WORD GetConfigTotalLength(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;

    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return 0;

    return byte_swap_of_word(pUsbConfig->wTotalLength);
}

BYTE GetConfigLength(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;

    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return 0;

    return pUsbConfig->bLength;
}

BYTE GetConfigConfigurationValue(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;

    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return 0;

    return pUsbConfig->bConfigurationValue;
}

BYTE GetConfigNumInterface(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;

    pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);

    if(pUsbConfig == NULL)
        return 0;

    return pUsbConfig->bNumInterfaces;
}

// Interface Descriptor Member
BYTE GetInterfaceClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx)
{
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;

    pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx);

    if(pUsbIFDesc == NULL)
        return 0;

    return pUsbIFDesc->bInterfaceClass;
}

BYTE GetInterfaceEPs(
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, 
    BYTE ConfigIdx, 
    BYTE InterfaceIdx, 
    BYTE AlterSettingIdx)
{
    USB_CONFIG_DESCRIPTOR       *pUsbConfig;    
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;
    DWORD dwCnt = 0, dwTotal = 0;

    #if 0
    if(AlterSettingIdx == AlterSettingAllIdx) // All AlterSetting
    {
        pUsbConfig = GetConfigStruct(pUsbhDevDes, ConfigIdx);        
        
        if(pUsbConfig == NULL)
            return 0;

        for(dwCnt = 0; dwCnt < pUsbConfig->pInterface->max_altsetting; dwCnt++)
            dwTotal += GetInterfaceEPs(pUsbhDevDes, ConfigIdx, InterfaceIdx, dwCnt);

        return dwTotal;
        
    }
    else
    #endif
    {
        pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx);

        if(pUsbIFDesc == NULL)
            return 0;

        return pUsbIFDesc->bNumEndpoints;
    }
}

BYTE GetInterfaceProtocol(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx)
{
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;

    pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx);

    if(pUsbIFDesc == NULL)
        return 0;

    return pUsbIFDesc->bInterfaceProtocol;
}

BYTE GetInterfaceSubClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx)
{
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;

    pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx);

    if(pUsbIFDesc == NULL)
        return 0;

    return pUsbIFDesc->bInterfaceSubClass;
}

BYTE GetEndpointAttributes(
        PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, 
        BYTE ConfigIdx, 
        BYTE InterfaceIdx, 
        BYTE AlterSettingIdx, 
        BYTE EndpointIdx)
{
    USB_ENDPOINT_DESCRIPTOR    *pEpDesc;

    pEpDesc = GetEndpointStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx, EndpointIdx);

    if(pEpDesc == NULL)
        return 0;

    return pEpDesc->bmAttributes;
}

BYTE GetEndpointAddress(
        PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
        BYTE ConfigIdx,
        BYTE InterfaceIdx,
        BYTE AlterSettingIdx,
        BYTE EndpointIdx)
{
    USB_ENDPOINT_DESCRIPTOR    *pEpDesc;

    pEpDesc = GetEndpointStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx, EndpointIdx);

    if(pEpDesc == NULL)
        return 0;

    return pEpDesc->bEndpointAddress;
}

WORD GetEndpointMaxPacketSize(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx)
{
    USB_ENDPOINT_DESCRIPTOR    *pEpDesc;

    pEpDesc = GetEndpointStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx, EndpointIdx);

    if(pEpDesc == NULL)
        return 0;

    return pEpDesc->wMaxPacketSize;
}

WORD GetEndpointInterval(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx)
{
    USB_ENDPOINT_DESCRIPTOR    *pEpDesc;

    pEpDesc = GetEndpointStruct(pUsbhDevDes, ConfigIdx, InterfaceIdx, AlterSettingIdx, EndpointIdx);

    if(pEpDesc == NULL)
        return 0;

    return pEpDesc->bInterval;
}

#endif // USBOTG_HOST_DESC


#endif

