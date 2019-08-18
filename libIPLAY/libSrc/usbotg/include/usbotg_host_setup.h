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
* Filename		: usbotg_host_setup.h
* Programmer(s)	: Joe Luo (JL)
* Created Date	: 2005/08/26 
* Description	: 
******************************************************************************** 
*/
#ifndef __USBOTG_SETUP_H__
#define __USBOTG_SETUP_H__ 

/*
// Include section 
*/
#include "UtilRegFile.h"

#if (SC_USBHOST==ENABLE)

/*
// Constant declarations
*/
//move to usbotg_std.h
#if 0
/////////////////////////////////////////////////
// USB Spec Chapter 9 Copy from Linux
/////////////////////////////////////////////////
#define USB_DIR_OUT     0x00			/* to device */
#define USB_DIR_IN      0x80		/* to host */
/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK       (0x03 << 5)
#define USB_TYPE_STANDARD   (0x00 << 5)
#define USB_TYPE_CLASS      (0x01 << 5)
#define USB_TYPE_VENDOR     (0x02 << 5)
#define USB_TYPE_RESERVED   (0x03 << 5)
#endif

/*
 * USB recipients, the third of three bRequestType fields
 */
#ifndef __LINUX_USB_CH9_H
#define USB_RECIP_MASK      0x1f
#define USB_RECIP_DEVICE    0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT  0x02
#define USB_RECIP_PORT      0x03
#endif

#if 0 // move to usbotg_std.h
/*
 * Standard requests, for the bRequest field of a SETUP packet.
 *
 * These are qualified by the bRequestType field, so that for example
 * TYPE_CLASS or TYPE_VENDOR specific feature flags could be retrieved
 * by a GET_STATUS request.
 */
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
#define USB_REQ_SET_FEATURE         0x03
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR		0x06
#define USB_REQ_SET_DESCRIPTOR		0x07
#define USB_REQ_GET_CONFIGURATION	0x08
#define USB_REQ_SET_CONFIGURATION	0x09
#define USB_REQ_GET_INTERFACE		0x0A
#define USB_REQ_SET_INTERFACE		0x0B
#define USB_REQ_SYNCH_FRAME         0x0C

#define USB_REQ_MSDC_GET_MAX_LUN    0xFE
#define USB_REQ_MSDC_RESET          0xFF
#endif
/*
 * Hub Class Feature Selector
 */
#define HUB_C_HUB_LOCAL_POWER      0
#define HUB_C_HUB_OVER_CURRENT     1

#define HUB_PORT_CONNECTION        0
#define HUB_PORT_ENABLE            1
#define HUB_PORT_SUSPEND           2
#define HUB_PORT_OVER_CURRENT      3
#define HUB_PORT_RESET             4
#define HUB_PORT_POWER             8
#define HUB_PORT_LOW_SPEED         9
#define HUB_C_PORT_CONNECTION      16
#define HUB_C_PORT_ENABLE          17
#define HUB_C_PORT_SUSPEND         18
#define HUB_C_PORT_OVER_CURRENT    19    
#define HUB_C_PORT_RESET           20

// bDescriptorType
#ifndef __LINUX_USB_CH9_H
#define USB_DT_DEVICE				    0x0001
#define USB_DT_CONFIG				    0x0002
#define USB_DT_STRING				    0x0003
#define USB_DT_INTERFACE			    0x0004
#define USB_DT_ENDPOINT                 0x0005
#define USB_DT_DEVICE_QUALIFIER         0x0006
#define USB_DT_OTHER_SPEED_CONFIG	    0x0007
#define USB_DT_INTERFACE_POWER		    0x0008
#define USB_DT_OTG					    0x0009
#define USB_DT_INTERFACE_ASSOCIATION    0x000B // Web Camera
#define USB_DT_HID                                          0x0021 // HID
#define USB_DT_CS_INTERFACE			    0x0024
#define USB_DT_CS_ENDPOINT              0x0025
#define USB_DT_HUB                      0x0029 // PenDrive with Hub
#endif


#define USB_DT_DEVICE_SIZE		    18
#define USB_DT_CONFIG_SIZE		    9
#define USB_DT_INTERFACE_SIZE		9
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_EXTRA_SIZE           3
#define USB_DT_HUB_DES_SIZE		    9

/*
 * Device and/or Interface Class codes
 * as found in bDeviceClass or bInterfaceClass
 * and defined by www.usb.org documents
 */
#ifndef __LINUX_USB_CH9_H
#define USB_CLASS_PER_INTERFACE	    0	/* for DeviceClass */
#define USB_CLASS_AUDIO			    1
#define USB_CLASS_COMM				2   /* Communications Device Class */
#define USB_CLASS_HID				3
#define USB_CLASS_PHYSICAL			5
#define USB_CLASS_STILL_IMAGE		6
#define USB_CLASS_PRINTER			7
#define USB_CLASS_MASS_STORAGE		8
#define USB_CLASS_HUB				9
#define USB_CLASS_CDC_DATA			0x0a   /* Communications Device Class DATA */
#define USB_CLASS_CSCID             0x0b	/* chip+ smart card */
#define USB_CLASS_CONTENT_SEC		0x0d	/* content security */
#define USB_CLASS_VIDEO				0x0E	/* VIDEO CLASS */
#define USB_CLASS_WIRELESS_BT		0xE0    // rick BT
#define USB_CLASS_APP_SPEC			0xfe
#define USB_CLASS_VENDOR_SPEC		0xff
#endif

/*
 * Endpoints
 */

#if 0 // move to usbotg_std.h
#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80
#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3
#endif
#define USB_DT_ENDPOINT_SIZE		7
#define USB_DT_ENDPOINT_AUDIO_SIZE	9	/* Audio extension */

//#define USB_CLASS_MASS_STORAGE	        8
//#define kHubInterfaceClass	        9
//#define kVendorInterfaceClass	    0xff

// bInterfaceProtocol
#define BULK_ONLY_PROTOCOL          0x50
#define CBI_WITHOUT_INT_PROTOCOL    0x01
#define PIMA_15740_BULK_ONLY_PROTOCOL    0x01
#define CBI_PROTOCOL  0x00
#define COMMON_AT_COMMANDS_PROTOCOL    0x01

#define HID_KEYBOARD_PROTOCOL			0x01
#define HID_MOUSE_PROTOCOL    			0x02



#define USB_DEVICE_ADDRESS	            1 // 4
//#define NUM_OF_TOTAL_TX_BYTES_PER_TD    OHCD_PAGE_SIZE_ASSUMPTION//NUM_OF_TOTAL_TX_BYTES_PER_TD





/*
// Structure declarations
*/


/*
// Function prototype 
*/
// extern SetupCommand for CBI protocol
/*
DWORD SetupCommand( PED                 pEd,
                    WORD                data_in_len,
                    WORD                data_out_len,
                    DWORD               data_addr,
                    USB_CTRL_REQUEST    ctrl_request,
                    DWORD               bufferRounding);

SWORD SetupGetDeviceDescriptor          ( PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes);

SWORD SetupGetConfigurationDescriptor   ( PST_USBH_DEVICE_DESCRIPTOR    pUsbhDevDes,
                                          PST_APP_CLASS_DESCRIPTOR      pAppClassDes,
                                          PST_HUB_CLASS_DESCRIPTOR      pHubClassDes);

SWORD SetupSetConfguration              ( PST_USBH_DEVICE_DESCRIPTOR    pUsbhDevDes,
                                          PST_APP_CLASS_DESCRIPTOR      pAppClassDes,
                                          PST_HUB_CLASS_DESCRIPTOR      pHubClassDes);

SWORD SetupSetAddress(PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes);
SWORD SetupClearFeature (BYTE end_point);

SWORD SetupGetHubDescriptor ( PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes,
                              PUSB_HUB_DESCRIPTOR        pUsbhHubDes);
SWORD SetupGetSatus         ( PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes);
SWORD SetupGetPortSatus     ( PST_HUB_CLASS_DESCRIPTOR   pHubDes,
                              WORD                       port_num);
SWORD SetPortFeature        ( PST_HUB_CLASS_DESCRIPTOR   pHubDes,
                              WORD                       port_num,
                              WORD                       feature_selector);
SWORD ClearPortFeature      ( PST_HUB_CLASS_DESCRIPTOR   pHubDes,
                              WORD                       port_num,
                              WORD                       feature_selector);
*/

#if USBOTG_HOST_DESC // Dynamic Configuration Descriptor

enum _ALTER_SETTING_INDEX_
{
	AlterSettingDefaultIdx      = 0x00,  // If no AlternateSetting, please set AlterSettingIdx = 0 and will get Interface Descriptor
	AlterSettingAllIdx          = 0xFF,        
};

void InitConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes);
BOOL AllocConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes);
BOOL FreeConfigDescriptor(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes);

// Please check return pointer whether it will be NULL. If NULL, please don't access value of structure member.
PUSB_CONFIG_DESCRIPTOR      GetConfigStruct(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx);
PUSB_INTERFACE_DESCRIPTOR   GetInterfaceStruct(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx);
PUSB_ENDPOINT_DESCRIPTOR    GetEndpointStruct(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx);

// Configuration Descriptor Member
WORD    GetConfigTotalLength(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx); // swapped
BYTE    GetConfigLength(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx);
BYTE    GetConfigConfigurationValue(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx);
BYTE    GetConfigNumInterface(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx);

// Interface Descriptor Member
BYTE    GetInterfaceClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx);
BYTE    GetInterfaceEPs(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx);
BYTE    GetInterfaceProtocol(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx);
BYTE    GetInterfaceSubClass(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx);

// Endpoint Descriptor Member
BYTE GetEndpointAttributes(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx);
BYTE GetEndpointAddress(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx);
WORD GetEndpointMaxPacketSize(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx);
WORD GetEndpointInterval(PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes, BYTE ConfigIdx, BYTE InterfaceIdx, BYTE AlterSettingIdx, BYTE EndpointIdx);

#endif // USBOTG_HOST_DESC

void SetupBuilder(
        PUSB_CTRL_REQUEST   pCtrlRequest,
        BYTE                bRequestType,
        BYTE                bRequest,
        WORD                wValue,
        WORD                wIndex,
        WORD                wLength);
SDWORD SetupCommand( 
        PST_USBOTG_SETUP_PB pSetupPB,
        WORD data_in_len,
        WORD data_out_len,
        DWORD data_addr,
        PUSB_CTRL_REQUEST pCtrl_request,
        WHICH_OTG eWhichOtg);
SWORD SetupClearFeature(BYTE end_point, WHICH_OTG eWhichOtg);
SWORD SetupGetDeviceDescriptor(WHICH_OTG eWhichOtg);
SWORD SetupSetAddress(WHICH_OTG eWhichOtg);
SWORD SetupGetConfigurationDescriptor (WHICH_OTG eWhichOtg);
void SetupGetConfigurationDescriptorDone  (WHICH_OTG eWhichOtg);
SWORD SetupSetConfguration (WHICH_OTG eWhichOtg);
SWORD SetupSetInterface (WORD wInterface, WORD wAltInterface, WHICH_OTG eWhichOtg);
SWORD SetupGetHubDescriptor(WHICH_OTG eWhichOtg);
SWORD SetupGetSatus(WHICH_OTG eWhichOtg);
SWORD SetupGetPortSatus (WORD port_num, WHICH_OTG eWhichOtg);
SWORD SetPortFeature (  WORD port_num,
                        WORD feature_selector,
                        WHICH_OTG eWhichOtg);
SWORD ClearPortFeature (    WORD port_num,
                            WORD feature_selector,
                            WHICH_OTG eWhichOtg);
SWORD SetupMsdcReset (WHICH_OTG eWhichOtg);
SWORD SetupGetMaxLun (WHICH_OTG eWhichOtg);


#endif // SC_USBHOST

#endif /* End of __USBOTG_SETUP_H__ */

