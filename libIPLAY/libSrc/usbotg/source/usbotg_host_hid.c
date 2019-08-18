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
* Filename		: usbotg_host_hid.c
* Programmer(s)	: Calvin
* Created Date	: 2008/02/18 
* Description        : USB OTG HID Class
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
#include "usbotg_host_hid.h"
#include "Usbotg_at.h"

#include "taskid.h"
#include "os.h"

#include "ui.h"

#if USBOTG_HOST_HID

#define OutReport 0x200
#define SetReport 0x9

BYTE bOutReportVal;

//------------------------------------------------------------------------------
// TODO - move to application 
static const BYTE usb_kbd_keycode[256] = {  // Map to ASCII
/*   0 */	  0,  0,  0,  0, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108,
/* 16 */	 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,  49,  50,
/* 32 */	  51,  52,  53,  54,  55,  56,  57,  48,  KEY_ENTER,  KEY_EXIT,  127,  9,  32,  45,  61,  91,
/* 48 */	  93,  92,  0,  59,  39,  96,  44,  46,  47,  0,  0,  0,  0,  0,  0,  0,
/* 64 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  KEY_RIGHT,
/* 80 */	  KEY_LEFT,  KEY_DOWN,  KEY_UP,  0,  47,  42,  45,  43,  0,  49,  50,  51,  52,  53,  54,  55,
/* 96 */	  56,  57,  48,  46,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*112 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*128 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*144 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*160 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*176 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*192 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*208 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*224 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
/*240 */	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
/*256 */	 
};

BYTE bCapsLockState = 0;
BYTE bNumLockState = 0;
BYTE bScrollLockState = 0;
void UsbCheckHidKeyState(BYTE bKeyData, BYTE bKeyState, WHICH_OTG eWhichOtg)
{
    //mpDebugPrint("%s: bKeyData = 0x%02X, bKeyState = 0x%X", __func__, bKeyData, bKeyState); // log too
    BYTE b650Code = KEY_NULL;
    if(bKeyData == 0)
    {
        //MP_ALERT("Usb HID bKeyData == 0");
        //return;
    }
    
    // check current keyboard state
    if(bKeyData == 0x39) // CapsLock
    {
        bCapsLockState = bCapsLockState ? 0 : 1;
        mpDebugPrint("bKeyData == 0x39, bCapsLockState = %d", bCapsLockState);
        //return;
    }
    
    if(bKeyData == 0x53) // NumLock
    {
        bNumLockState = bNumLockState ? 0 : 1;
        mpDebugPrint("bKeyData == 0x53, bNumLockState = %d", bNumLockState);
        //return;
    }
    
    if(bKeyData == 0x47) // ScrollLock
    {
        bScrollLockState = bScrollLockState ? 0 : 1;
        mpDebugPrint("bKeyData == 0x47, bScrollLockState = %d", bScrollLockState);
        //return;
    }

	// Enable LED Example Num Lock (BIT0) /Caps Lock (BIT2) / Scroll Lock	(BIT3)
	UsbhHidOutReport((bNumLockState << 0) | (bCapsLockState << 1) | (bScrollLockState << 2), eWhichOtg);
    
    // check flow control code for MP650
    if(bKeyData == 0x28) // ENTER
    {
        b650Code = 0x0E; // Enter key
        //Api_UsbhGetHidSetKey(b650Code);
        //EventSet(UI_EVENT, EVENT_HID_KEY);      
    } 
    
    if(bKeyData == 0x29) // Escape key
    {
        b650Code = 0x07; // EXIT
        //Api_UsbhGetHidSetKey(b650Code);
        //EventSet(UI_EVENT, EVENT_HID_KEY);      
    } 
    
    if(bKeyData == 0x4F) // Right key
    {
        b650Code = KEY_RIGHT;
        Api_UsbhHidSetKey(b650Code, eWhichOtg);
        EventSet(UI_EVENT, EVENT_HID_KEY);
        return;     
    } 
    
    if(bKeyData == 0x50) // Left key
    {
        b650Code = KEY_LEFT;
        Api_UsbhHidSetKey(b650Code, eWhichOtg);
        EventSet(UI_EVENT, EVENT_HID_KEY);
        return;      
    } 
    
    if(bKeyData == 0x51) // Down key
    {
        b650Code = KEY_DOWN;
        Api_UsbhHidSetKey(b650Code, eWhichOtg);
        EventSet(UI_EVENT, EVENT_HID_KEY);
        return;      
    } 
    
    if(bKeyData == 0x52) // Up key
    {
        b650Code = KEY_UP;
        Api_UsbhHidSetKey(b650Code, eWhichOtg);
        EventSet(UI_EVENT, EVENT_HID_KEY);
        return;      
    } 
    
    // Editor usage
    if(bKeyData == 0x2B) // Tab
    {
        // TODO - in editor usage only
        b650Code = KEY_NULL;
    } 
    
    // ASCII A..Z or a..z
    if(bKeyData >= 0x04 && bKeyData <= 0x1D)
    {
        if(bCapsLockState == 1)
            b650Code = 0x41 - 0x04 + bKeyData; // A..Z
        else
            b650Code = 0x61 - 0x04 + bKeyData; // a..z
        //mpDebugPrint("after CapsLockState(%d), b650Code = 0x%X(%d)", bCapsLockState, b650Code, b650Code);
        if(bKeyState & 0x22) // 0x20(right shift) or 0x02(left shift)
        {
            if(b650Code >= 0x41 && b650Code <= 0x5A) // A..Z
                b650Code |= 0x20; // A..Z to a..z
            else
                b650Code &= 0xDF; // a..z to A..Z
            //mpDebugPrint("after shift convert, b650Code = 0x%X(%d)", b650Code, b650Code);
        }
    }
    
    // 1..9 & 0
    if(bKeyData >= 0x1E && bKeyData <= 0x27)
    {
        if(bKeyData == 0x27) // key 0
            b650Code = 0x30;
        else
            b650Code = 0x31 + (bKeyData - 0x1E);   
    }
    
    //  key 
    if(bKeyData >= 0x2D || bKeyData == 0x2E || bKeyData == 0x2F || bKeyData == 0x30 || bKeyData == 0x31
        || bKeyData == 0x33 || bKeyData == 0x34 || bKeyData == 0x36 || bKeyData == 0x37 || bKeyData == 0x38 ) 
        b650Code = bKeyData;
        
    // F1..F12
    if(bKeyData >= 0x3A && bKeyData <= 0x45)
        b650Code = bKeyData;
        
    //   PrintScreen,  PauseBreak, Insert, Delete, Home, End, PageUp, PageDown 
    if(bKeyData >= 0x46 && bKeyData <= 0x52)
        b650Code = bKeyData;
    
    // BackSpace
    if(bKeyData == 0x7F) // Backspace
        b650Code = KEY_NULL;
        
    if(bKeyState & 0x22) // 0x20(right shift) or 0x02(left shift)
    {
        //mpDebugPrint("bKeyState & 0x22");
        if(bKeyData == 0x1E) // key 1
            b650Code = 0x21; // !
        else if(bKeyData == 0x1F) // key 2
            b650Code = 0x40; // @
        else if(bKeyData == 0x20) // key 3
            b650Code = 0x23; // #
        else if(bKeyData == 0x21) // key 4
            b650Code = 0x24; // $
        else if(bKeyData == 0x22) // key 5
            b650Code = 0x25; // %
        else if(bKeyData == 0x23) // key 6
            b650Code = 0x5E; // ^
        else if(bKeyData == 0x24) // key 7
            b650Code = 0x26; // &
        else if(bKeyData == 0x25) // key 8
            b650Code = 0x2A; // *
        else if(bKeyData == 0x26) // key 9
            b650Code = 0x28; // (
        else if(bKeyData == 0x27) // key 0
            b650Code = 0x29; // )
            
        if(bKeyData == 0x2D) // key -
            b650Code = 0x5F; // _
        else if(bKeyData == 0x2E) // key =
            b650Code = 0x2B; // +
        else if(bKeyData == 0x2F) // key [
            b650Code = 0x7B; // {
        else if(bKeyData == 0x30) // key ]
            b650Code = 0x7D; // }
        else if(bKeyData == 0x31) // key 
            b650Code = 0x25; //
        else if(bKeyData == 0x33) // key 
            b650Code = 0x3A; // ^
        else if(bKeyData == 0x34) // key '
            b650Code = 0x22; // "
        else if(bKeyData == 0x36) // key ,
            b650Code = 0x3C; // <
        else if(bKeyData == 0x37) // key .
            b650Code = 0x3E; // >
        else if(bKeyData == 0x38) // key /
            b650Code = 0x3F; // ?
    }
   
    // numeric keypad - NumLock state off
    if(bNumLockState == 0)
    {
             if(bKeyData == 0x54) 
            b650Code = KEY_NULL; // 
        else if(bKeyData == 0x55) 
            b650Code = KEY_NULL; // 
        else if(bKeyData == 0x56) 
            b650Code = KEY_NULL; // 
        else if(bKeyData == 0x57) 
            b650Code = 0x2B; // +
        else if(bKeyData == 0x58) 
            b650Code = 0x0E; // Enter
        else if(bKeyData == 0x59) 
            b650Code = 0x4D; // End
        else if(bKeyData == 0x5A) 
            b650Code = KEY_DOWN; // Down
        else if(bKeyData == 0x5B) 
            b650Code = 0x4E; // PgDn
        else if(bKeyData == 0x5C) 
            b650Code = KEY_LEFT; // Left
        else if(bKeyData == 0x5D) 
            b650Code = KEY_NULL; // 
        else if(bKeyData == 0x5E) 
            b650Code = KEY_RIGHT; // Right
        else if(bKeyData == 0x5F) 
            b650Code = 0x4A; // Home
        else if(bKeyData == 0x60) 
            b650Code = KEY_UP; // Up
        else if(bKeyData == 0x61) 
            b650Code = 0x4B; // PgUp
        else if(bKeyData == 0x62) 
            b650Code = 0x49; // Insert
        else if(bKeyData == 0x63) 
            b650Code = 0x07; // EXIT 0x4C; // Del
    }
    else // bNumLockState == 1
    {
             if(bKeyData == 0x54) 
            b650Code = 0x2F; // /
        else if(bKeyData == 0x55) 
            b650Code = 0x2A; // *
        else if(bKeyData == 0x56) 
            b650Code = 0x2D; // -
        else if(bKeyData == 0x57) 
            b650Code = 0x2B; // +
        else if(bKeyData == 0x58) 
            b650Code = 0x0E; // Enter
        else if(bKeyData == 0x59) 
            b650Code = 0x31; // 1
        else if(bKeyData == 0x5A) 
            b650Code = 0x32; // 2
        else if(bKeyData == 0x5B) 
            b650Code = 0x33; // 3
        else if(bKeyData == 0x5C) 
            b650Code = 0x34; // 4
        else if(bKeyData == 0x5D) 
            b650Code = 0x35; // 5
        else if(bKeyData == 0x5E) 
            b650Code = 0x36; // 6
        else if(bKeyData == 0x5F) 
            b650Code = 0x37; // 7
        else if(bKeyData == 0x60) 
            b650Code = 0x38; // 8
        else if(bKeyData == 0x61) 
            b650Code = 0x39; // 9
        else if(bKeyData == 0x62) 
            b650Code = 0x30; // 0
        else if(bKeyData == 0x63) 
            b650Code = 0x2E; // .
    }
            
    // issue event
    mpDebugPrint("%s: b650Code = 0x%X(%d)", __func__, b650Code, b650Code);
    Api_UsbhHidSetKey(b650Code, eWhichOtg);
    EventSet(UI_EVENT, EVENT_HID_KEY);  
    
}

//------------------------------------------------------------------------------

/*
// Static function prototype
*/
static void UsbhHidSetHidDescriptorData(WHICH_OTG eWhichOtg);
static void UsbhHidSetHidDescriptorDataFromExtra(BYTE bIndex, USB_EXTRA_DESCRIPTOR *pUsbExtra, BYTE bInterfaceNumber, WHICH_OTG eWhichOtg);
static void UsbhHidDumpHidDescriptor(BYTE bHidDescIndex, WHICH_OTG eWhichOtg);


// API ======================================================
BYTE b650MouseKey = 0; 
BYTE b650MouseType = 0; // reserved for extend
BYTE Api_UsbhHidGetMouseKey(WHICH_OTG eWhichOtg)
{
    //mpDebugPrint("%s: b650MouseKey = 0x%08X", __func__, b650MouseKey);
    BYTE b650MouseKeyBackup = 0;
    //b650MouseKeyBackup |= b650MouseType;
    //b650MouseKeyBackup << 8;
    b650MouseKeyBackup |= b650MouseKey;
    b650MouseKey = 0; // reset
    b650MouseType = 0; // reset
    return b650MouseKeyBackup;
}
    
void Api_UsbhHidSetMouseKeyType(DWORD bKey, BYTE bType, WHICH_OTG eWhichOtg)
{
    b650MouseKey = bKey;
    b650MouseType = bType;
    //mpDebugPrint("%s: b650MouseKey = bKey = 0x%08X, bType = %d", __func__, bKey, bType);
}

DWORD Api_UsbhHidGetKey(WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure    

    return pHidClassDes->dwCurKey;
}
    
void Api_UsbhHidSetKey(DWORD bKey, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure    

    pHidClassDes->dwCurKey = bKey;
}

BOOL Api_UsbhHidCheckIfConnectedKeyboard(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if((pUsbhDevDes->bConnectStatus == USB_STATUS_CONNECT) && 
        ((GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) == USB_CLASS_HID) &&
        (Api_UsbhHidGetWhichDevice(eWhichOtg) == HID_KEYBOARD_PROTOCOL))
        return TRUE;
    else
        return FALSE;
}

BOOL Api_UsbhHidCheckIfConnectedMouse(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    if((pUsbhDevDes->bConnectStatus == USB_STATUS_CONNECT) && 
        ((GetInterfaceClass(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)) == USB_CLASS_HID) &&
        (Api_UsbhHidGetWhichDevice(eWhichOtg) == HID_MOUSE_PROTOCOL))
        return TRUE;
    else
        return FALSE;
}

USB_HOST_HID_DEVICE Api_UsbhHidGetWhichDevice(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    return (USB_HOST_HID_DEVICE)GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx);
}

WHICH_OTG Api_UsbhHidGetWhichOtg(BYTE bHidDevice)
{
    BYTE bCnt = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    for(bCnt = 0; bCnt < USBOTG_NONE; bCnt++)
    {
        pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(bCnt);
        if (pUsbhDevDes->bConnectStatus != USB_STATUS_CONNECT)
            continue;
		if(bHidDevice == HID_KEYBOARD_PROTOCOL)
			if(Api_UsbhHidCheckIfConnectedKeyboard(bCnt))
				return (WHICH_OTG)bCnt;
		if(bHidDevice == HID_MOUSE_PROTOCOL)
			if(Api_UsbhHidCheckIfConnectedMouse(bCnt))
				return (WHICH_OTG)bCnt;
    }

    return USBOTG_NONE;
}


// global =====================================================

void UsbhHidInit(WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure

    // Init Hid Decriptor structure and variable
    pHidClassDes->bHidDescriptorNum = 0;
    pHidClassDes->bHidDescriptorCurrent= 0;
    pHidClassDes->psHidDescriptor = 0;    
    
    // How many HID descriptor and Alloc for psHidDescriptor
    pHidClassDes->bHidDescriptorNum = UsbhHidGetHidDescriptorNum(eWhichOtg);
    pHidClassDes->psHidDescriptor = (USB_HID_DESCRIPTOR *)ker_mem_malloc(pHidClassDes->bHidDescriptorNum*sizeof(USB_HID_DESCRIPTOR), TaskGetId());
    MP_DEBUG("[Hid-Descriptor Alloc Address] 0x%x", pHidClassDes->psHidDescriptor);
    if (pHidClassDes->psHidDescriptor == 0)
    {
        MP_ALERT("-USBOTG%d- %s Alloc Hid Decriptor Fail", eWhichOtg, __FUNCTION__);
        return;
    }
    
    // Set up data of psHidDescriptor stucture
    memset(pHidClassDes->psHidDescriptor, 0x00, pHidClassDes->bHidDescriptorNum*sizeof(USB_HID_DESCRIPTOR));
    UsbhHidSetHidDescriptorData(eWhichOtg);

    pHidClassDes->pbBuffer = (BYTE *)ker_mem_malloc(8*sizeof(BYTE), TaskGetId()); // Should after parse Report to get size as 8
    if (pHidClassDes->pbBuffer == 0)
    {
        MP_ALERT("-USBOTG%d- %s Alloc Hid pbBuffer Fail", eWhichOtg, __FUNCTION__);
        return;
    }    
    MP_DEBUG("[Hid-Buffer Alloc Address] 0x%x", pHidClassDes->pbBuffer);

}


void UsbhHidDeInit(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_HID psHid = (PUSB_HOST_HID)UsbOtgHostHidDsGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure

    MP_DEBUG("-USBOTG%d- %s", eWhichOtg, __FUNCTION__);

    // Free alloc-memory in this file
    if(pHidClassDes->psHidDescriptor != NULL)
    {
        if(pHidClassDes->psHidDescriptor->psHidReport  != NULL)
        {
            MP_DEBUG("[Hid-Report Free Address] 0x%x", pHidClassDes->psHidDescriptor->psHidReport);
            ker_mem_free(pHidClassDes->psHidDescriptor->psHidReport);
        }
        MP_DEBUG("[Hid-Descriptor Free Address] 0x%x", pHidClassDes->psHidDescriptor);       
        ker_mem_free(pHidClassDes->psHidDescriptor);
    }

    if(pHidClassDes->pbBuffer != NULL)
    {
        MP_DEBUG("[Hid-Buffer Free Address] 0x%x", pHidClassDes->pbBuffer);
        ker_mem_free(pHidClassDes->pbBuffer);
    }
        
}


/*
//  Control End-Pointer (SETUP)
*/
SWORD SetupHidClassCmd(BYTE bRequest, WORD wValue, WORD wIndex, WORD wLength, DWORD dwDataAddr, WHICH_OTG eWhichOtg)
{
    WORD    data_in_len    = wLength;
    SWORD   err             = USB_NO_ERROR;
    USB_CTRL_REQUEST    requestContainer;    
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    
    switch(bRequest)
    {
        case USB_REQ_GET_DESCRIPTOR:
            SetupBuilder(&requestContainer, USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_INTERFACE, bRequest, wValue, wIndex, data_in_len);
          
            break;

        default:
            MP_DEBUG("HID bRequest is not defined");
            break;
    }

    MP_DEBUG("SetupHidClassCmd 0x%x", bRequest);
    
    err = SetupCommand( &pUsbhDevDes->sSetupPB, data_in_len, 0, dwDataAddr, &requestContainer, eWhichOtg);
    
    if (err != USB_NO_ERROR)
    {
        MP_DEBUG("ERROR:SetupHidClassCmd %x", bRequest);
    }

    return err;
}


void UsbhHidInterruptActive(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure    
    Host20_BufferPointerArray_Structure stBufAry;
    DWORD dwDataLen = 0;
    BYTE  bDataDir = 0;

    pUsbhDevDes->psAppClass->dIntDataBuffer = ((DWORD)pHidClassDes->pbBuffer | 0xa0000000);
    pUsbhDevDes->psAppClass->dwBulkOnlyState = CBI_INTERRUPT_STATE;
    pUsbhDevDes->psAppClass->bIntDataInLen = 8; 

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_INTERRUPT_STATE:
            //MP_DEBUG("HID_INTERRUPT_STATE");
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


void UsbhHidInterruptIoc(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg); 
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure    
    BYTE  bKey;   

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState) // CBI_DATA_IN_STATE, CBI_DATA_OUT_STATE, CBI_INTERRUPT_STATE
    {
        case CBI_INTERRUPT_STATE:
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstInterruptInqHD[pUsbhDevDes->psAppClass->bIntInQHDArrayNum], eWhichOtg);
            break; 

        default:
            MP_DEBUG("Unknow USB Direction %x", pUsbhDevDes->psAppClass->dwBulkOnlyState);
            break;
    }  


    bKey = pHidClassDes->pbBuffer[1];
#if 1

	MP_DEBUG("-I- %s USBOTG%d %s", __FUNCTION__, eWhichOtg, Api_UsbhHidGetWhichDevice(eWhichOtg) == HID_KEYBOARD_PROTOCOL ? "Keyboard":"Mouse");
	if(Api_UsbhHidGetWhichDevice(eWhichOtg) == HID_KEYBOARD_PROTOCOL) // KeyBoard
    	if(bKey != 0) 
        	UsbCheckHidKeyState(bKey, pHidClassDes->pbBuffer[3], eWhichOtg);

	if(Api_UsbhHidGetWhichDevice(eWhichOtg) == HID_MOUSE_PROTOCOL) // Mouse
	{
	   	   // pHidClassDes->pbBuffer[0] = 0x01 - forward(upward), 0xFF - backward(downward)
	   // pHidClassDes->pbBuffer[1] = 0x01 or 0xFF
	   // pHidClassDes->pbBuffer[2] = 0x01 or 0xFF
	   // pHidClassDes->pbBuffer[3] = 0x01 (left key) or 0x02(right key)
	   WORD wVal = 0;
	   wVal = pHidClassDes->pbBuffer[0] + pHidClassDes->pbBuffer[1] + pHidClassDes->pbBuffer[2] + pHidClassDes->pbBuffer[3];
	   //if(pHidClassDes->pbBuffer[3] != 0x00)
	   if(wVal > 0x00)
	   {
	       BYTE index, pbBuffer[8];
	       for(index = 0; index < 8; index++)
	           pbBuffer[index] = pHidClassDes->pbBuffer[index];
	       //mpDebugPrint("HID: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X", 
           // pbBuffer[0], pbBuffer[1], pbBuffer[2], pbBuffer[3], pbBuffer[4], pbBuffer[5], pbBuffer[6], pbBuffer[7]);
            
            BYTE b650Code = 0, bType = 0; 
            if(pHidClassDes->pbBuffer[0] > 0x00 || pHidClassDes->pbBuffer[2] > 0x00 || pHidClassDes->pbBuffer[3] > 0x00)
            {
                if(pHidClassDes->pbBuffer[0] > 0x00) // wheel
                {
                    if(pHidClassDes->pbBuffer[0] == 0x01)
                        b650Code = KEY_UP; 
                    else if(pHidClassDes->pbBuffer[0] == 0xFF)
                        b650Code = KEY_DOWN;
                }
                else
                if(pHidClassDes->pbBuffer[3] > 0x00) // left or right button
                {
                    if(pHidClassDes->pbBuffer[3] == 0x01)
                        b650Code = KEY_ENTER; //0x0E; // KEY_ENTER
                    else if(pHidClassDes->pbBuffer[3] == 0x02)
                        b650Code = KEY_EXIT;
                    else
                        ; //MP_ALERT("%s: get Mouse response - unknown buffer code!", __func__);
                }
                else
                if(pHidClassDes->pbBuffer[2] > 0x00) // 
                {
                    if(pHidClassDes->pbBuffer[2] == 0x01)
                        b650Code = KEY_RIGHT;
                    else if(pHidClassDes->pbBuffer[2] == 0xFF)
                        b650Code = KEY_LEFT;
                    // delay 1 second - wait bouncing done
                    DWORD i, j, count = 4000;
                    for(i=0; i<count; i++)
                        for(j=0; j<count; j++)
                            ;
                }
                                    
                Api_UsbhHidSetMouseKeyType(b650Code, bType, eWhichOtg);
                EventSet(UI_EVENT, EVENT_HID_MOUSE);
            }
        }            
    }
	
#else    
    if(usb_kbd_keycode[bKey] != 0) 
    {
        MP_ALERT("Key:%c", usb_kbd_keycode[bKey]);
        Api_UsbhHidSetKey(usb_kbd_keycode[bKey], eWhichOtg);
        EventSet(UI_EVENT, EVENT_HID_KEY);      
    }
    else
    {
        if(bKey != 0)
            MP_ALERT("KeyCode:%d", bKey);  
    }
    
    if(pHidClassDes->pbBuffer[3] != 0)
        MP_ALERT("KeySpec:%d", pHidClassDes->pbBuffer[3]); 
#endif 
    EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_INTERRUPT);
      
}

SWORD UsbhHidOutReport(BYTE bKey, WHICH_OTG eWhichOtg)
{
	PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
	SDWORD   err = USB_NO_ERROR;
	USB_CTRL_REQUEST	ctrl_request;

	bOutReportVal = bKey;
	SetupBuilder (  &ctrl_request,
	                (BYTE) (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE),
	                (BYTE) SetReport,
	                (WORD) OutReport,
	                (WORD) 0,
	                (WORD) 8);
	err = SetupCommand( &pUsbhDevDes->sSetupPB,
	                    0,
	                    1,
	                    (DWORD)&bOutReportVal,
	                    &ctrl_request,
	                    eWhichOtg);
	if (err != USB_NO_ERROR)
	    MP_DEBUG("UsbhHidOutReport fail err = %x", err);

	return err;
}

void UsbhHidSetupIoc(WHICH_OTG eWhichOtg)
{
	PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
	pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

	MP_DEBUG("UsbhHidSetupIoc");

	if (pUsbhDevDes->bConnectStatus == USB_STATUS_DISCONNECT)// plug out
	{
	    return;
	}
	if (pUsbhDevDes->bCmdTimeoutError == 1)
	{
	    pUsbhDevDes->bCmdTimeoutError = 0;
	    return;
	}

//	SendMailToUsbOtgHostClassTask(GetUsbhMailEnvelope(USBOTG_HOST_DRIVER_TASK_SENDER, eWhichOtg),eWhichOtg); // for next setup command
}

PUSB_HID_DESCRIPTOR UsbhHidGetHidDescriptor(BYTE bHidDescIndex, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure

    if(bHidDescIndex >= pHidClassDes->bHidDescriptorNum)
    {
        MP_ALERT("-E- %s Hid Descriptor Index:%d over Total:%d ", __FUNCTION__, bHidDescIndex, pHidClassDes->bHidDescriptorNum);
        return NULL;
    }
    
    return (pHidClassDes->psHidDescriptor + bHidDescIndex);
}

PUSB_HID_REPORT UsbhHidGetHidReport(BYTE bHidDescIndex, BYTE bHidReportIndex, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure
    PUSB_HID_DESCRIPTOR psHidDescriptor;

    psHidDescriptor = UsbhHidGetHidDescriptor(bHidDescIndex, eWhichOtg);
    if(psHidDescriptor == NULL) return NULL;
    
    if(bHidReportIndex >= psHidDescriptor->bMaxReport) 
    {
        MP_ALERT("-E- %s Hid Report Index:%d over Total:%d ", __FUNCTION__, bHidReportIndex, psHidDescriptor->bMaxReport);        
        return NULL;
    }
    
    return (psHidDescriptor->psHidReport + bHidReportIndex);
}

void UsbhHidParseReport(BYTE* bData, PUSB_HID_REPORT psHidReport, WHICH_OTG eWhichOtg)
{
#if 0
    PUSB_HID_ITEM pstUsbHidItem;
    WORD wLength;

    wLength = psHidReport->wDescriptorLength;

    mpDebugPrint("UsbhHidParseReport 0x%x", bData[0]);

    while(wLength > 0)
    {
        pstUsbHidItem = bData;

        MP_DEBUG("bSize 0x%x", pstUsbHidItem->bSize);
        switch(pstUsbHidItem->bSize)
        {
            case HID_ITEM_SIZE_ZERO:
                break;
            case HID_ITEM_SIZE_ONE:
                break;
            case HID_ITEM_SIZE_TWO:
                break;
            case HID_ITEM_SIZE_FOUR:
                break;                
            default:
                break;                
        }

        
        MP_DEBUG("bType 0x%x", pstUsbHidItem->bType);
        switch(pstUsbHidItem->bType)
        {
            case HID_ITEM_TYPE_MAIN:
                break;
            case HID_ITEM_TYPE_GLOBAL:
                break;
            case HID_ITEM_TYPE_LOCAL:
                break;
            default:
                break;
        }


        MP_DEBUG("bTag 0x%x", pstUsbHidItem->bTag);
    }
#endif
}


BYTE UsbhHidGetHidDescriptorNum(WHICH_OTG eWhichOtg) // Get from GetDescriptor (Configuration)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;    
    USB_EXTRA_DESCRIPTOR *pUsbExtra;
    BYTE    bNumOfInterface = 0, bNumOfHidDescriptor = 0;
    BYTE    bIfCnt, bExtraCnt;

    bNumOfInterface = GetConfigNumInterface(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);

    for(bIfCnt = 0; bIfCnt < bNumOfInterface; bIfCnt++)
    {
        pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bIfCnt, AlterSettingDefaultIdx);

        if( pUsbIFDesc->bInterfaceClass == USB_CLASS_HID )
        {
            for(bExtraCnt = 0; bExtraCnt < pUsbIFDesc->extra_num; bExtraCnt++)
            {
                pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbIFDesc->extra+bExtraCnt);
                if(pUsbExtra->bDescriptorType == HID_DT_HID)
                    bNumOfHidDescriptor++;
            }
        }
    }
    MP_DEBUG("Total HID Descriptor:%d", bNumOfHidDescriptor);
    return bNumOfHidDescriptor;
}



// Local =======================================================



static void UsbhHidSetHidDescriptorData(WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    USB_INTERFACE_DESCRIPTOR    *pUsbIFDesc;    
    USB_EXTRA_DESCRIPTOR *pUsbExtra;
    BYTE    bNumOfInterface = 0, bNumOfHidDescriptor = 0;
    BYTE    bIfCnt, bExtraCnt;

    bNumOfInterface = GetConfigNumInterface(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx);
 
    for(bIfCnt = 0; bIfCnt < bNumOfInterface; bIfCnt++)
    {
        pUsbIFDesc = GetInterfaceStruct(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, bIfCnt, AlterSettingDefaultIdx);

        if(pUsbIFDesc->bInterfaceClass == USB_CLASS_HID)
        {
            for(bExtraCnt = 0; bExtraCnt < pUsbIFDesc->extra_num; bExtraCnt++)
            {
                pUsbExtra = (USB_EXTRA_DESCRIPTOR *)(pUsbIFDesc->extra+bExtraCnt);
                if(pUsbExtra->bDescriptorType == HID_DT_HID)
                {
                    UsbhHidSetHidDescriptorDataFromExtra(bNumOfHidDescriptor, pUsbExtra, bIfCnt, eWhichOtg);
                    bNumOfHidDescriptor++;
                }
            }
        }
    }
}

static void UsbhHidSetHidDescriptorDataFromExtra(BYTE bIndex, USB_EXTRA_DESCRIPTOR *pUsbExtra, BYTE bInterfaceNumber, WHICH_OTG eWhichOtg)
{
    PST_APP_CLASS_DESCRIPTOR    pHidClassDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg); // Get HID structure
    USB_HID_DESCRIPTOR *psHidDescriptor;
    USB_HID_REPORT         *psHidReport;
    BYTE bCnt;
    BYTE bSameLength = 3;

    // HID Decriptor Structure
    psHidDescriptor = UsbhHidGetHidDescriptor(bIndex, eWhichOtg);
    if(psHidDescriptor == NULL) return;
    memcpy(psHidDescriptor, pUsbExtra, bSameLength);
    psHidDescriptor->bcdHID = ((pUsbExtra->bData[0] <<8) | pUsbExtra->bDescriptorSubtype);
    psHidDescriptor->bCountryCode = pUsbExtra->bData[1];
    psHidDescriptor->bNumDescriptors = pUsbExtra->bData[2];
    for(bCnt = 0; bCnt < psHidDescriptor->bNumDescriptors; bCnt++)
    {
        if(pUsbExtra->bData[bSameLength+(bCnt*3)] == HID_DT_REPORT)
            psHidDescriptor->bMaxReport++;
        if(pUsbExtra->bData[bSameLength+(bCnt*3)] == HID_DT_PHYSICAL)
            psHidDescriptor->bMaxPhysical++;        
    }
    psHidDescriptor->bInterfaceNumber = bInterfaceNumber;    
    psHidDescriptor->psHidReport = (USB_HID_REPORT *)ker_mem_malloc(psHidDescriptor->bMaxReport*sizeof(USB_HID_REPORT), TaskGetId());
    if (psHidDescriptor->psHidReport == 0)
    {
        MP_ALERT("-USBOTG%d- %s Alloc Hid Report Fail", eWhichOtg, __FUNCTION__);
        return;
    }
    MP_DEBUG("[Hid-Report Alloc Address] 0x%x", psHidDescriptor->psHidReport);
	
    // HID Report Structure
    for(bCnt = 0; bCnt < psHidDescriptor->bNumDescriptors; bCnt++)
    {
        if(pUsbExtra->bData[bSameLength+(bCnt*3)] == HID_DT_REPORT)
        {
            psHidReport = (psHidDescriptor->psHidReport+bCnt);
            psHidReport->bIndex = bCnt;
            psHidReport->bDescriptorType = HID_DT_REPORT;
            psHidReport->wDescriptorLength = ((pUsbExtra->bData[bSameLength+(bCnt*3)+2] <<8) | pUsbExtra->bData[bSameLength+(bCnt*3)+1]);            
        }
    }    

    // DUMP
    UsbhHidDumpHidDescriptor(bIndex, eWhichOtg);

}

static void UsbhHidDumpHidDescriptor(BYTE bHidDescIndex, WHICH_OTG eWhichOtg)
{
    BYTE bCnt;
    USB_HID_DESCRIPTOR *psHidDescriptor;
    USB_HID_REPORT *psHidReport;

    psHidDescriptor = UsbhHidGetHidDescriptor(bHidDescIndex, eWhichOtg);
    if(psHidDescriptor == NULL) return;

    MP_DEBUG("\n===================================");    
    MP_DEBUG("Dump HID Descriptor[%02d]", bHidDescIndex);
    MP_DEBUG("===================================");  
    MP_DEBUG("bLength                       0x%02x", psHidDescriptor->bLength);
    MP_DEBUG("bDescriptorType               0x%02x", psHidDescriptor->bDescriptorType);
    MP_DEBUG("bcdHID                        0x%04x", psHidDescriptor->bcdHID);    
    MP_DEBUG("bCountryCode                  0x%02x", psHidDescriptor->bCountryCode);
    MP_DEBUG("bNumDescriptors               0x%02x", psHidDescriptor->bNumDescriptors);
    for(bCnt = 0; bCnt < psHidDescriptor->bMaxReport; bCnt++)
    {
        psHidReport = psHidDescriptor->psHidReport;
        MP_DEBUG("bDescriptorType[%02d]           0x%02x", psHidReport->bIndex, psHidReport->bDescriptorType);  
        MP_DEBUG("wDescriptorLength[%02d]         0x%02x", psHidReport->bIndex, psHidReport->wDescriptorLength);  
    }
    MP_DEBUG("===================================");
    // Extra Information
    MP_DEBUG("bMaxReport                    0x%02x", psHidDescriptor->bMaxReport);  
    MP_DEBUG("bMaxPhysical                  0x%02x", psHidDescriptor->bMaxPhysical);
    MP_DEBUG("bInterfaceNumber              0x%02x", psHidDescriptor->bInterfaceNumber);
    MP_DEBUG("===================================\n");    
    
}

#endif //USBOTG_HOST_HID

