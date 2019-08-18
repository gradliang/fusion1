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
* Filename		: usbotg_host_hid.h
* Programmer(s)	: Calvin
* Created Date	: 2010/10/13 
* Description        : USB OTG HID Class
******************************************************************************** 
*/

#ifndef __USBOTG_HOST_HID_H__
#define __USBOTG_HOST_HID_H__
/*
// Include section 
*/
#include "utiltypedef.h"
#include "usbotg_api.h"

#if (USBOTG_HOST_HID == ENABLE)

/////////////////////////////////////////////////
// Constant declarations
/////////////////////////////////////////////////

/*
 * HID class requests
 */

#define HID_REQ_GET_REPORT		0x01
#define HID_REQ_GET_IDLE		0x02
#define HID_REQ_GET_PROTOCOL		0x03
#define HID_REQ_SET_REPORT		0x09
#define HID_REQ_SET_IDLE		0x0A
#define HID_REQ_SET_PROTOCOL		0x0B

/*
 * HID class descriptor types
 */
 
#define HID_DT_HID                      (USB_TYPE_CLASS | 0x01)
#define HID_DT_REPORT                (USB_TYPE_CLASS | 0x02)
#define HID_DT_PHYSICAL             (USB_TYPE_CLASS | 0x03)

/*
 * HID report types
 */

#define HID_INPUT_REPORT	   1
#define HID_OUTPUT_REPORT	   2
#define HID_FEATURE_REPORT   3

/*
 * HID report descriptor item size (prefix bit 0,1)
 */

#define HID_ITEM_SIZE_ZERO	         0
#define HID_ITEM_SIZE_ONE            1
#define HID_ITEM_SIZE_TWO           2
#define HID_ITEM_SIZE_FOUR          3

/*
 * HID report descriptor item type (prefix bit 2,3)
 */

#define HID_ITEM_TYPE_MAIN	           0
#define HID_ITEM_TYPE_GLOBAL        1
#define HID_ITEM_TYPE_LOCAL          2
#define HID_ITEM_TYPE_RESERVED    3


/////////////////////////////////////////////////
// HID Report Descriptor
/////////////////////////////////////////////////

typedef struct {
    BYTE      bTag:4;
    BYTE      bType:2;
    BYTE      bSize:2;
    BYTE      bData[4];	

} USB_HID_ITEM, *PUSB_HID_ITEM;


typedef struct {
    // Package format
    BYTE bUsagePage;
    BYTE bUsagePageMin;    
    BYTE bUsagePageMax;
    BYTE bLogicalMin;  // Value Range:bLogicalMin ~ bLogicalMax
    BYTE bLogicalMax;
    BYTE bReportSize;  // unit:bits   
    BYTE bReportCount;    
    BYTE bReserve;
    // Field format to calculate
    BYTE  report_offset;        /* bit offset in the report */
    BYTE  report_size;           /* size of this field in the report */
    BYTE  report_count;         /* number of this field in the report */
    BYTE  report_type;          /* (input,output,feature) */
} USB_HID_ALL_PUT, *PUSB_HID_REPORT_ALL_PUT;


typedef struct {
    BYTE  physical;
    BYTE  logical;
    BYTE  application;
    BYTE  bReserve;
    USB_HID_ALL_PUT  *pHidInput;
    USB_HID_ALL_PUT  *pHidOutput;
    BYTE  bMaxInput;
    BYTE  bMaxOutput;
} USB_HID_REPORT_COLLECTION, *PUSB_HID_REPORT_COLLECTION;


// HID Report Descriptor Structure
typedef struct {
    BYTE    bIndex;
    BYTE    bDescriptorType;  // HID_DT_REPORT
    WORD  wDescriptorLength; 
    BYTE    bMaxCollect;
    BYTE    bReserve[3];
    USB_HID_REPORT_COLLECTION *pHidRepCollection;
} USB_HID_REPORT, *PUSB_HID_REPORT;


// HID Descriptor
typedef struct {
    BYTE    bLength;
    BYTE    bDescriptorType;
    WORD  bcdHID;
    BYTE    bCountryCode;
    BYTE    bNumDescriptors;  // total count of pHidReport(bMaxReport) & pHidPhysical(bMaxPhysical)
    BYTE    bMaxReport;
    BYTE    bMaxPhysical;
    USB_HID_REPORT *psHidReport;
    // USB_HID_PHYSICAL * pHidPhysical; // Not use
    BYTE    bInterfaceNumber;
} USB_HID_DESCRIPTOR, *PUSB_HID_DESCRIPTOR;


/////////////////////////////////////////////////
// HID Class-Specific Request Codes
/////////////////////////////////////////////////



/////////////////////////////////////////////////
// Function prototype 
/////////////////////////////////////////////////
SWORD SetupHidClassCmd(BYTE bRequest, WORD wValue, WORD wIndex, WORD wLength, DWORD dwDataAddr, WHICH_OTG eWhichOtg);

void UsbhHidInit(WHICH_OTG eWhichOtg);
void UsbhHidDeInit(WHICH_OTG eWhichOtg);

void UsbhHidInterruptActive(WHICH_OTG eWhichOtg);
void UsbhHidInterruptIoc(WHICH_OTG eWhichOtg);
SWORD UsbhHidOutReport(BYTE bKey, WHICH_OTG eWhichOtg);
void UsbhHidSetupIoc(WHICH_OTG eWhichOtg);

BYTE UsbhHidGetHidDescriptorNum(WHICH_OTG eWhichOtg);

PUSB_HID_DESCRIPTOR UsbhHidGetHidDescriptor(BYTE bHidDescIndex, WHICH_OTG eWhichOtg);
PUSB_HID_REPORT UsbhHidGetHidReport(BYTE bHidDescIndex, BYTE bHidReportIndex, WHICH_OTG eWhichOtg);


#endif // USBOTG_HOST_HID

#endif /* End of __USBOTG_HOST_HID_H__ */




