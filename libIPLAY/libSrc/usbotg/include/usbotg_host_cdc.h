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
* Filename		: usbotg_host_cdc.h
* Programmer(s)	: Calvin
* Created Date	: 2008/02/18 
* Description        : USB OTG CDC Class
******************************************************************************** 
*/

#ifndef __USBOTG_HOST_CDC_H__
#define __USBOTG_HOST_CDC_H__
/*
// Include section 
*/
#include "utiltypedef.h"
#include "usbotg_api.h"

#if (USBOTG_HOST_CDC == ENABLE)

#define USBOTG_HOST_CDC_HUAWEI_TESTING 0

/////////////////////////////////////////////////
// Constant declarations
/////////////////////////////////////////////////
#define CDC_FUNTION_DATA_MAX 30 


/////////////////////////////////////////////////
// CDC Function Descriptor
/////////////////////////////////////////////////
// Generic Function Descriptor Type (bFuntionLength)
enum 
{
    CDC_DESC_TYPE_CS_INTERFACE          = 0x24,
    CDC_DESC_TYPE_CS_ENDPOINT           = 0x25,
};

// Generic Function Descriptor Sub Type (bDiscriptorSubType)
enum 
{
    CDC_DESC_SUBTYPE_HEADER             = 0x00,  // Header Functional Descriptor
    CDC_DESC_SUBTYPE_CALL_MANAGE        = 0x01,  // Call Management Functional Descriptor
    CDC_DESC_SUBTYPE_ABSTRACT_CONTROL   = 0x02,  // Abstract Control Management Functional Descriptor
    CDC_DESC_SUBTYPE_DIRECT_LINE        = 0x03,  // Direct Line Management Functional Descriptor
    CDC_DESC_SUBTYPE_TELEPHONE_RINGER   = 0x04,  // Telephone Ringer Functional Descriptor
    CDC_DESC_SUBTYPE_TELEPHONE_CALL     = 0x05,  // Telephone Call and Line State Reporting Capabilities Functional Descriptor
    CDC_DESC_SUBTYPE_UNION              = 0x06,  // Union Functional descriptor
    CDC_DESC_SUBTYPE_COUNTRY_SELECTION  = 0x07,  // Country Selection Functional Descriptor
    CDC_DESC_SUBTYPE_TELEPHONE_OP       = 0x08,  // Telephone Operational Modes Functional Descriptor
    CDC_DESC_SUBTYPE_USB_TERMINAL       = 0x09,  // USB Terminal Functional Descriptor
    CDC_DESC_SUBTYPE_NET_CH             = 0x0A,  // Network Channel Terminal Descriptor
    CDC_DESC_SUBTYPE_PROTOCOL_UNIT      = 0x0B,  // Protocol Unit Functional Descriptor    
    CDC_DESC_SUBTYPE_EXTENSION_UNIT     = 0x0C,  // Extension Unit Functional Descriptor
    CDC_DESC_SUBTYPE_MULTI_CH           = 0x0D,  // Multi-Channel Management Functional Descriptor
    CDC_DESC_SUBTYPE_CAPI_CONTROL       = 0x0E,  // CAPI Control Management Functional Descriptor
    CDC_DESC_SUBTYPE_ETHERNET_NET       = 0x0F,  // Ethernet Networking Functional Descriptor
    CDC_DESC_SUBTYPE_ATM_NET            = 0x10,  // ATM Networking Functional Descriptor
    CDC_DESC_SUBTYPE_MOBILE_DIRECT      = 0x12,  // Mobile Direct Line Model Functional Descriptor
    CDC_DESC_SUBTYPE_MDLM_DETAIL        = 0x13,  // MDLM Detail Functional Descriptor
};

// Generic Function Structure: N+3 bytes
typedef struct {
	BYTE	bFuntionLength;
	BYTE	bDiscriptorType;    
	BYTE	bDiscriptorSubType;
	BYTE	bFuntionData[CDC_FUNTION_DATA_MAX];
} CDC_DISCRIPTOR, *PCDC_DISCRIPTOR; 



/////////////////////////////////////////////////
// CDC Class-Specific Request Codes
/////////////////////////////////////////////////
enum 
{
    USB_CDC_INTERFACE_1     = 0x01,
    USB_CDC_INTERFACE_2     = 0x02,
};

// CDC Class Command
enum 
{
    CDC_SEND_ENCAPSULATED_COMMAND               = 0x00,
    CDC_GET_ENCAPSULATED_RESPONSE               = 0x01,
    CDC_SET_COMM_FEATURE                        = 0x02,
    CDC_GET_COMM_FEATURE                        = 0x03,
    CDC_CLEAR_COMM_FEATURE                      = 0x04,
    //RESERVED (future use) 05h-0Fh
    CDC_SET_AUX_LINE_STATE                      = 0x10,
    CDC_SET_HOOK_STATE                          = 0x11,
    CDC_PULSE_SETUP                             = 0x12,
    CDC_SEND_PULSE                              = 0x13,
    CDC_SET_PULSE_TIME                          = 0x14,
    CDC_RING_AUX_JACK                           = 0x15,
    //RESERVED (future use) 16h-1Fh
    CDC_SET_LINE_CODING                         = 0x20,
    CDC_GET_LINE_CODING                         = 0x21,
    CDC_SET_CONTROL_LINE_STATE                  = 0x22,
    CDC_SEND_BREAK                              = 0x23,
    //RESERVED (future use) 24h-2Fh
    CDC_SET_RINGER_PARMS                        = 0x30,
    CDC_GET_RINGER_PARMS                        = 0x31,
    CDC_SET_OPERATION_PARMS                     = 0x32,
    CDC_GET_OPERATION_PARMS                     = 0x33,
    CDC_SET_LINE_PARMS                          = 0x34,
    CDC_GET_LINE_PARMS                          = 0x35,
    CDC_DIAL_DIGITS                             = 0x36,
    CDC_SET_UNIT_PARAMETER                      = 0x37,
    CDC_GET_UNIT_PARAMETER                      = 0x38,
    CDC_CLEAR_UNIT_PARAMETER                    = 0x39,
    CDC_GET_PROFILE                             = 0x3A,
    //RESERVED (future use) 3Bh-3Fh
    CDC_SET_ETHERNET_MULTICAST_FILTERS          = 0x40,
    CDC_SET_ETHERNET_POWER_MANAGEMENT_PATTERN   = 0x41,
    CDC_GET_ETHERNET_POWER_MANAGEMENT_PATTERN   = 0x42,
    CDC_SET_ETHERNET_PACKET_FILTER              = 0x43,
    CDC_GET_ETHERNET_STATISTIC                  = 0x44,
    //RESERVED (future use) 45h-4Fh    
    CDC_VENDOR_CMD_STATE                        = 0xFF,
}; 

// CDC Vendor Command
enum 
{
    CDC_QISDA_VENDOR_CMD_STATE                  = 0x04,  // Qisda Company (Vendor Command)
}; 

// CDC CDC_SET_CONTROL_LINE_STATE wValue
enum 
{
    CDC_NOT_PRESENT                             = 0x00,
    CDC_PRESENT                                 = 0x01,  
    CDC_ACTIVATE_CARRIER                        = 0x02, 
}; 

/////////////////////////////////////////////////
// Function prototype 
/////////////////////////////////////////////////
void UsbOtgHostCdcBulkActive(WHICH_OTG eWhichOtg);
void UsbOtgHostCdcBulkIoc(WHICH_OTG eWhichOtg);
void UsbOtgHostCdcInterruptActive(WHICH_OTG eWhichOtg);
void UsbOtgHostCdcInterruptIoc(WHICH_OTG eWhichOtg);
SDWORD CdcDataOut(BYTE *pbBuffer,DWORD dwlength, WHICH_OTG eWhichOtg);
SDWORD CdcDataIn(BYTE *pbBuffer, DWORD dwlength, WHICH_OTG eWhichOtg);
SDWORD CdcInterruptIn(BYTE *pbBuffer, WHICH_OTG eWhichOtg);
SWORD SetupCdcClassCmd(BYTE bRequest, WORD wValue, WHICH_OTG eWhichOtg);
#endif
#if (USBOTG_HOST_CDC || USBOTG_HOST_DATANG)
void UsbOtgHostCdcAtCmdPlug(WHICH_OTG eWhichOtg, BOOL bPlug);
void UsbOtgHostCdcAtCmdInit(WHICH_OTG eWhichOtg);
#endif
#if USBOTG_HOST_CDC
#if 1
void NetUsb_BulkIoc(WHICH_OTG eWhichOtg);
void NetUsb_InterruptActive(WHICH_OTG eWhichOtg);
#endif
/********************************************************************
///
///@defgroup    Host_CDC Host CDC class for Communication
///@ingroup     USB_Host
///  This USB Host CDC class is used in Communication, such as 3.5G etc...
///
///  USB CDC API function is as below:<br>
///  - CdcDataOut<br>
///  - CdcDataIn<br>
///  - CdcInterruptIn<br>
///
///  USB CDC Active & Ioc function is as below:<br>
///  - UsbOtgHostCdcBulkActive<br>
///  - UsbOtgHostCdcBulkIoc<br>
///  - UsbOtgHostCdcInterruptActive<br>
///  - UsbOtgHostCdcInterruptIoc<br>

///
///@defgroup    Host_CDC_API   Host CDC API Function
///@ingroup     Host_CDC
///These API Functions.

///
///@ingroup Host_CDC_API
///@brief   To send data to another device(3.5G Modem) via USB driver.
///
///@param   *pbBuffer  Sent Data Buffer Pointer
///@param   dwlength   Sent Data Length
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     0      Success
///@retval   -n      Error 
///
///@remark None
SDWORD CdcDataOut(BYTE *pbBuffer,DWORD dwlength, WHICH_OTG eWhichOtg);
///
///@ingroup Host_CDC_API
///@brief   To receive data from another device(3.5G Modem) via USB driver.
///
///@param   *pbBuffer  Received Data Buffer Pointer
///@param   dwlength   Received Data Length
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     0      Success
///@retval   -n      Error 
///
///@remark None
SDWORD CdcDataIn(BYTE *pbBuffer, DWORD dwlength, WHICH_OTG eWhichOtg)
///
///@ingroup Host_CDC_API
///@brief   To get another device(3.5G Modem) status via USB driver.
///
///@param   *pbBuffer  Received Data Buffer Pointer
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     0      Success
///@retval   -n      Error 
///
///@remark None
SDWORD CdcInterruptIn(BYTE *pbBuffer, WHICH_OTG eWhichOtg);



///@defgroup    Host_CDC_ACTIVE_IOC   Host CDC Active & Ioc Function
///@ingroup     Host_CDC
///These Active & Ioc of Bulk & Interrupt.

///
///@ingroup Host_CDC_ACTIVE_IOC
///@brief   Set Bulk Active to send/receive data via Bulk transfer type.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     None
///
///@remark *pUsbhDev is a ST_USBH_DEVICE_DESCRIPTOR structure.
void UsbOtgHostCdcBulkActive(WHICH_OTG eWhichOtg);
///
///@ingroup Host_CDC_ACTIVE_IOC
///@brief   When Bulk EP finish transfering send/receive data, it will be called.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     None
///
///@remark *pUsbhDev is a ST_USBH_DEVICE_DESCRIPTOR structure.
void UsbOtgHostCdcBulkIoc(WHICH_OTG eWhichOtg);
///
///@ingroup Host_CDC_ACTIVE_IOC
///@brief   Set Interrupt Active to receive data via Interrupt transfer type.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     None
///
///@remark *pUsbhDev is a ST_USBH_DEVICE_DESCRIPTOR structure.
void UsbOtgHostCdcInterruptActive(WHICH_OTG eWhichOtg);
///
///@ingroup Host_CDC_ACTIVE_IOC
///@brief   When Interrupt EP finish transfering send/receive data, it will be called.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval     None
///
///@remark *pUsbhDev is a ST_USBH_DEVICE_DESCRIPTOR structure.
void UsbOtgHostCdcInterruptIoc(WHICH_OTG eWhichOtg);
//void UsbOtgHostCdcAtCmdInit(void);
********************************************************************/
#endif // USBOTG_HOST_CDC

#endif /* End of __USBOTG_HOST_CDC_H__ */




