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
* Filename		: usbotg_host_sidc.h
* Programmer(s)	: Morse Chen
* Created Date	: 2008-06-25
* Description	: 
******************************************************************************** 
*/
#ifndef __USBH_SIDC_H__
#define __USBH_SIDC_H__
/*
// Include section 
*/
#include "utiltypedef.h"
#include "usbotg_device_sidc.h"
#include "usbotg_api.h"

///
///@mainpage iPlay USB Host Driver System.
///

#if (SC_USBHOST==ENABLE)

///
///@defgroup USB_STILL_DEVICE_CLASS
///
///This implements PIMA 15740 for USB still device class.
///

/*
// Constant declarations
*/

/*
//
// PIMA 15740 Definition
// Operation Code
//

enum {
	OPERATION_GET_DEVICE_INFO		    = 0x1001, 
	OPERATION_OPEN_SESSION			    = 0x1002, 
	OPERATION_CLOSE_SESSION			    = 0x1003, 
	OPERATION_GET_STORAGE_IDS		    = 0x1004, 
	OPERATION_GET_STORAGE_INFO		    = 0x1005, 

	OPERATION_GET_NUM_OBJECTS			= 0x1006, 
	OPERATION_GET_OBJECT_HANDLES		= 0x1007, 
	OPERATION_GET_OBJECT_INFO			= 0x1008, 
	OPERATION_GET_OBJECT				= 0x1009, 
	OPERATION_GET_THUMB					= 0x100a, 

	OPERATION_DELETE_OBJECT				= 0x100b, 
	OPERATION_SEND_OBJECT_INFO			= 0x100c, 
	OPERATION_SEND_OBJECT				= 0x100d, 
	OPERATION_INITIATE_CAPTURE			= 0x100e, 
	OPERATION_FORMAT_STORE				= 0x100f, 

	OPERATION_RESET_DEVICE				= 0x1010, 
	OPERATION_SELFTEST					= 0x1011, 
	OPERATION_SET_OBJECT_PROTECTION		= 0x1012, 
	OPERATION_POWERDOWN				    = 0x1013, 
	OPERATION_GET_DEVICE_PROP_DESC		= 0x1014, 

	OPERATION_GET_DEVICE_PROP_VALUE		= 0x1015, 
	OPERATION_SET_DEVICE_PROP_VALUE		= 0x1016, 
	OPERATION_RESET_DEVICE_PROP_VALUE	= 0x1017, 
	OPERATION_TERMINATE_OPEN_CAPTURE	= 0x1018, 
	OPERATION_MOVE_OBJECT				= 0x1019, 

	OPERATION_COPY_OBJECT				= 0x101a, 
	OPERATION_GET_PARTIAL_OBJECT		= 0x101b, 
	OPERATION_INITIATE_OPEN_CAPTURE		= 0x101c
};

//
// PIMA 15740 Definition
// Response Code
//

enum {
	RESPONSE_UNDEFINED								= 0x2000,
	RESPONSE_OK									    = 0x2001, 
	RESPONSE_GENERAL_ERROR							= 0x2002, 
	RESPONSE_SESSION_NOT_OPEN						= 0x2003, 
	RESPONSE_INVALID_TRANSACTION_ID					= 0x2004, 
	RESPONSE_OPERATION_NOT_SUPPORTED				= 0x2005, 
	RESPONSE_PARAMETER_NOT_SUPPORTED				= 0x2006, 
	RESPONSE_INCOMPLETE_TRANSFER					= 0x2007, 
	RESPONSE_INVALID_STORAGE_ID						= 0x2008, 
	RESPONSE_INVALID_OBJECT_HANDLE					= 0x2009, 
	RESPONSE_DEVICE_PROP_NOT_SUPPORTED				= 0x200a, 
	RESPONSE_INVALID_OBJECT_FORMAT_CODE				= 0x200b, 
	RESPONSE_STORE_FULL							    = 0x200c, 
	RESPONSE_OBJECT_WRITE_PROTECTED					= 0x200d, 
	RESPONSE_STORE_READONLY						    = 0x200e, 
	RESPONSE_ACCESS_DENIED							= 0x200f, 
	RESPONSE_NO_THUMBNAIL_PRESENT					= 0x2010, 
	RESPONSE_SELFTEST_FAILED						= 0x2011, 
	RESPONSE_PARTIAL_DELETION						= 0x2012, 
	RESPONSE_STORE_NOT_AVAILABLE					= 0x2013, 
	RESPONSE_SPECIFICATION_BY_FORMAT_UNSUPPORTED	= 0x2014, 
	RESPONSE_NO_VALID_OBJECT_INFO					= 0x2015, 
	RESPONSE_INVALID_CODE_FORMAT					= 0x2016, 
	RESPONSE_UNKNOWN_VENDOR_CODE					= 0x2017, 
	RESPONSE_CAPTURE_ALREADY_TERMINATED			    = 0x2018, 
	RESPONSE_DEVICE_BUSY							= 0x2019, 
	RESPONSE_INVALID_PARENT_OBJECT					= 0x201a, 
	RESPONSE_INVALID_DEVICE_PROB_FORMAT				= 0x201b, 
	RESPONSE_INVALID_DEVICE_PROP_VALUE				= 0x201c, 
	RESPONSE_INVALID_PARAMETER						= 0x201d, 
	RESPONSE_SESSION_ALREADY_OPEN					= 0x201e, 
	RESPONSE_TRANSACTION_CANCELLED					= 0x201f, 
	RESPONSE_SPECIFICATION_OF_DESTINATION_UNSUPPORTED	= 0x2020 
};
//
// PIMA 15740 Definition
// Object Format Codes
//
enum {
    OBJ_FORMAT_UNDEFINED       = 0x3000,
    OBJ_FORMAT_ASSOCIATION     = 0x3001,
    OBJ_FORMAT_SCRIPT          = 0x3002,
    OBJ_FORMAT_EXECUTABLE      = 0x3003,
    OBJ_FORMAT_TEXT            = 0x3004,
    OBJ_FORMAT_HTML            = 0x3005,
    OBJ_FORMAT_DPOF            = 0x3006,
    OBJ_FORMAT_AIFF            = 0x3007,
    OBJ_FORMAT_WAV             = 0x3008,
    OBJ_FORMAT_MP3             = 0x3009,
    OBJ_FORMAT_AVI             = 0x300A,
    OBJ_FORMAT_MPEG            = 0x300B,
    OBJ_FORMAT_ASF             = 0x300C,
    OBJ_FORMAT_UNDEFINED_IMAGE = 0x3800,
    OBJ_FORMAT_JPEG_EXIF       = 0x3801,
    OBJ_FORMAT_TIFF_EP         = 0x3802,
    OBJ_FORMAT_FLASHPIX        = 0x3803,
    OBJ_FORMAT_BMP             = 0x3804,
    OBJ_FORMAT_CIFF            = 0x3805,
    OBJ_FORMAT_RSVD1           = 0x3806,
    OBJ_FORMAT_GIF             = 0x3807,
    OBJ_FORMAT_JFIF            = 0x3808,
    OBJ_FORMAT_PCD             = 0x3809,
    OBJ_FORMAT_PICT            = 0x380A,
    OBJ_FORMAT_PNG             = 0x380B,
    OBJ_FORMAT_RSVD2           = 0x380C,
    OBJ_FORMAT_TIFF            = 0x380D,
    OBJ_FORMAT_TIFF_IT         = 0x380E,
    OBJ_FORMAT_JP2             = 0x380F,
    OBJ_FORMAT_JPX             = 0x3810
};
// Filesystem Type Values
enum {
//	UNDEFINED          = 0x0000,
	GENERIC_FLAT        = 0x0001,
	GENERIC_HIERACHICAL = 0x0002,
	DCF                 = 0x0003
};
*/

/*
// Structure declarations
*/
/*
typedef struct 
{
    BYTE    physicalStore;
    BYTE    logicalStore;
    WORD    objectFormat;
    DWORD   parentHandle;
    DWORD   entrySector;
    DWORD   entryIndex;	
} OBJECT_HEADER, *POBJECT_HEADER;
*/

/*
// Function prototype 
*/
///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Send command to device. 
///
///@param   pUsbhDevDes    pointer to ST_USBH_DEVICE_DESCRIPTOR structure
///@param   pSidcDes       pointer to ST_APP_CLASS_DESCRIPTOR structure
///@param   opCode         operation code of command
///
///@retval  SWORD          0 if no error; otherwise error code
/// 
///@remark  Send command to device.
///
SWORD UsbOtgHostSidcCommand (	PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
								PST_APP_CLASS_DESCRIPTOR    pSidcDes,
								WORD                        opCode,
								WHICH_OTG                   eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Read data from device. 
///
///@param   dwLba            logical block address
///@param   dwCount          data length
///@param   dwBuffer         data buffer
///@param   dwObjectHandle   object handle
///@param   pUsbhDevDes      pointer to ST_USBH_DEVICE_DESCRIPTOR structure
///@param   pSidcDes         pointer to ST_APP_CLASS_DESCRIPTOR structure
///
///@retval  SWORD            0 if no error; otherwise error code
/// 
///@remark  Read data from device. 
///
SWORD UsbOtgHostSidcReadData (	DWORD                       dwLba,
								DWORD                       dwCount,
								DWORD                       dwBuffer,
								DWORD                       dwObjectHandle,
								WHICH_OTG                   eWhichOtg);

/*
SWORD UsbOtgHostSidcWriteData (   BYTE                        bLun,
                            DWORD                       dwLba,
                            DWORD                       dwCount,
                            DWORD                       dwBuffer,
                            WHICH_OTG                   eWhichOtg)
                            PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
                            PST_APP_CLASS_DESCRIPTOR    pSidcDes);
*/
void UsbOtgHostSidcBulkOnlyActive (WHICH_OTG eWhichOtg);
void UsbOtgHostSidcBulkOnlyIoc    (WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Check whether it is processing scan object command or not. 
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///
///@retval  BOOL    TRUE if it is processing scan object command; otherwise FALSE.
/// 
///@remark  Check whether it is processing scan object command or not. 
///
BOOL UsbOtgHostGetSidcScanObjects(WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Check whether it is processing partial object command or not. 
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///
///@retval  BOOL    TRUE if it is processing partial object command; otherwise FALSE.
/// 
///@remark  Check whether it is processing partial object command or not. 
///
BOOL UsbOtgHostGetSidcPartialObject(WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Set sbUsbOtgHostSidcScanObjects to TRUE if it is processing scan object. 
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///
///@retval  none
/// 
///@remark  Set sbUsbOtgHostSidcScanObjects to TRUE if it is processing scan object. 
///
void UsbOtgHostSetSidcScanObjects(WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Set sbUsbOtgHostSidcGetPartialObject to TRUE if it is processing partial object. 
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///
///@retval  none
/// 
///@remark  Set sbUsbOtgHostSidcGetPartialObject to TRUE if it is processing partial object. 
///
void UsbOtgHostSetSidcPartialObject(DWORD dwBuffer, DWORD dwCount, WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Set sbUsbOtgHostSidcScanObjects to FALSE. 
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///
///@retval  none
/// 
///@remark  Set sbUsbOtgHostSidcScanObjects to FALSE. 
///
void UsbOtgHostClrSidcScanObjects(WHICH_OTG eWhichOtg);

///
///@ingroup USB_STILL_DEVICE_CLASS
///@brief   Set sbUsbOtgHostSidcGetPartialObject to FALSE. 
///
///@param   copy    if it is TRUE then copy data to buffer from FS 
///@param   eWhichOtg   assign USBOTG0/USBOTG1 to operate.
///@retval  none
/// 
///@remark  Set sbUsbOtgHostSidcGetPartialObject to FALSE. 
///
void UsbOtgHostClrSidcPartialObject(BOOL copy,WHICH_OTG eWhichOtg);

BOOL UsbOtgHostGetSidcObject(WHICH_OTG eWhichOtg);
void UsbOtgHostSetSidcObject(WHICH_OTG eWhichOtg);
void UsbOtgHostClrSidcObject(WHICH_OTG eWhichOtg);

BOOL UsbOtgHostGetSidcObjectInfo(WHICH_OTG eWhichOtg);
void UsbOtgHostSetSidcObjectInfo(WHICH_OTG eWhichOtg);
void UsbOtgHostClrSidcObjectInfo(WHICH_OTG eWhichOtg);

extern void UsbOtgHostSidcDevicePlugOut(WHICH_OTG eWhichOtg);


#endif // SC_USBHOST

#endif /* End of __USBH_SIDC_H__ */


