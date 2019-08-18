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
* Filename		: usbotg_device_sidc.h
* Programmer(s)	: Cloud Wu
* Created Date	: 2008/11/24 
* Description:  SIDC (Still Image Device Class) 
*               
*               
******************************************************************************** 
*/

#ifndef __USBOTG_DEVICE_SIDC_H__
#define __USBOTG_DEVICE_SIDC_H__

#include "UtilTypeDef.h"
#include "usbotg_std.h"
#include "usbotg_device.h"
#include "usbotg_api.h"

//#define NUM_OPERATION_SUPPORTED			0x00000012 
//#define NUM_EVENT_SUPPORTED				0x00000001 
//#define NUM_DEVICE_PROP_CODE_SUPPORTED	0x00000003 
//#define NUM_OBJECT_FORMAT_SUPPORTED		0x00000002
#define MODEL_STRING_LENGTH				32	            
#define DEVICE_VERSION_STRING_LENGTH	8	            
#define SERIAL_NUMBER_STRING_LENGTH		32

#define MAX_DEV_NUM_OF_PHOTOS               500 // <Watch> if increase this, please confirm that DPF have enough Kernel Memory to alloc. If DPF don't have enough memory, it will alert message. // PHOTOx500 + DPSx6   // No single object for WinXP in the future.
#define MAX_DEV_NUM_OF_DPS                      6 // PHOTOx500 + DPSx6
#define MAX_DEV_NUM_OF_OBJECTS              MAX_DEV_NUM_OF_PHOTOS +  MAX_DEV_NUM_OF_DPS // PHOTOx500 + DPSx6

#define PTP_DPS_MAX_XFR             8*1024   //for 100 files(XML) (PTP_DPS_MAX_XFR = DPS_BUFF_SIZE //DPS_main.c) Calvin 2004.09.02
#define DEVICE_STORAGE_ID				0x00010001

//#define CANCEL_REQUEST_DATA_LENGTH          6
//#define GET_DEVICE_STATUS_REQUETS_LENGTH    4
/*
enum {
	NUM_CAPTURE_FORMAT_SUPPORTED	= NUM_OBJECT_FORMAT_SUPPORTED,
	NUM_IMAGE_FORMAT_SUPPORTED	= NUM_OBJECT_FORMAT_SUPPORTED,
//	NO_NIMAGEOBJECT				= 0x3000,
//	ASSOCIATION					= 0x3001,
//	OBJECT_FORMAT_MPEG			= 0x300B,
//	OBJECT_FORMAT_EXIF_JPEG		= 0x3801
};
*/
typedef struct {
	SDWORD		hiLong;
	SDWORD		loLong;
} UINT64,  *PUINT64; 

/*
typedef struct
{
        BYTE   numChars;
} STR00, *PSTR00;

typedef struct
{
        BYTE   numChars;
        BYTE  stringChars[32];
} STR16, *PSTR16;

// operation code array
typedef struct {
	DWORD		numElements;
	WORD		arrayEntry[NUM_OPERATION_SUPPORTED];
} OPERATIONS_SUPPORTED, *POPERATIONS_SUPPORTED; 

// event code array
typedef struct {
	DWORD		numElements;
	WORD		arrayEntry[NUM_EVENT_SUPPORTED];
} EVENTS_SUPPORTED, *PEVENTS_SUPPORTED; 

// device properties code array
typedef struct {
	DWORD		numElements;
	WORD		arrayEntry[NUM_DEVICE_PROP_CODE_SUPPORTED];
} DEVICE_PROPERTIES_SUPPORTED, *PDEVICE_PROPERTIES_SUPPORTED; 

// object format code array
typedef struct {
	DWORD		numElements;
	WORD		arrayEntry[NUM_OBJECT_FORMAT_SUPPORTED];
} CAPTURE_FORMATS, *PCAPTURE_FORMATS; 

// object format code array
typedef struct {
	DWORD		numElements;
	WORD		arrayEntry[NUM_OBJECT_FORMAT_SUPPORTED];
} IMAGE_FORMATS, *PIMAGE_FORMATS; 


// Generic Container Structure: 32 bytes
typedef struct {
	DWORD	containerLength;
	WORD	containerType;
	WORD	code;
	DWORD	transactionID;
	DWORD	parameter[5];
} STI_CONTAINER, *PSTI_CONTAINER; 
*/

// Event Structure: 24 bytes; refer to usb_stil_imag.pdf chapter 7.3.1 p29
typedef struct {
	DWORD	length;
	WORD	type;
	WORD	eventCode;
//	DWORD	sessionID;
	DWORD	transactionID;
	DWORD	parameter[3];
} STI_EVENT, *PSTI_EVENT; 
/*
//
// PIMA 15740 Definition
// DeviceInfo Dataset
//
typedef struct {
	WORD						standardVersion;
//	DWORD						standardVersion;
	DWORD						vendorExtensionID;
	WORD						vendorExtensionVer;
	STR00						vendorExtensionDesc;
	WORD						functionMode;
	OPERATIONS_SUPPORTED        operations;
	EVENTS_SUPPORTED            events;
	DEVICE_PROPERTIES_SUPPORTED	deviceProperties;
	CAPTURE_FORMATS				captureFormats;
	IMAGE_FORMATS				imageFormats;
	STR16						manufacture;
	STR16						model;
	STR16						deviceVersion;
	STR16						serialNumber;
	WORD						reserved;
} STI_DEVICE_INFO, *PSTI_DEVICE_INFO; 
*/
//
// PIMA 15740 Definition
// StorageInfo Dataset
//
typedef struct {
	WORD		storageType;
	WORD		filesystemType;
	WORD		accessCapability;
	UINT64		maxCapability;
	UINT64		freeSpaceInBytes;
	DWORD		freeSpaceInImages;
	STR00		storageDescription;
	STR00		volumeLable;
	WORD		reserved;
} STI_STORAGE_INFO, *PSTI_STORAGE_INFO;
/*
//
// PIMA 15740 Definition
// ObjectInfo Dataset
//

typedef struct {
	DWORD		storageID;
	WORD		objectFormatCode;
	WORD		protectionStatus;
	DWORD		objectCompSize;
	WORD		thumbFormat;
	DWORD		thumbCompSize;
	DWORD		thumbPixWidth;
	DWORD		thumbPixHeight;
	DWORD		imagePixWidth;
	DWORD		imagePixHeight;
	DWORD		imageBitDepth;
	DWORD		parentObject;
	WORD		associationType;
	DWORD		associationDesc;
	DWORD		seqNumber;
	STR16		filename;
	STR00		captureDate;
	STR00		modificationDate;
	STR00		keyWords;
}STI_OBJECT_INFO, *PSTI_OBJECT_INFO;
*/

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



//#if SC_USBDEVICE

typedef struct {
	
   DWORD copies;	
   DWORD layout_border;

} PRINTER_CAPABILITY;






//
// Still Image Capture Device Definition
// Class-Specific Requests
//

enum {
	CANCEL_REQUEST				= 0x64,
	GET_EXTENDED_EVENT_DATA	    = 0x65,
	DEVICE_RESET_REQUEST		= 0x66,
	GET_DEVICE_STATUS			= 0x67
};



// DPS States
enum {
	DPS_IDLE	                = 0x0000,
	DPS_CONFIGURE_PRINT_SERVICE = 0x0010,
	DPS_GET_CAPABILITY          = 0x0020,
	DPS_GET_JOB_STATUS          = 0x0040,
	DPS_GET_DEVICE_STATUS       = 0x0080,
	DPS_START_JOB               = 0x0100,
	DPS_ABORT_JOB               = 0x0200,
	DPS_CONTINUE_JOB            = 0x0400,
	DPS_NOTIFY_JOB_STATUS       = 0x0800,
	DPS_NOTIFY_DEVICE_STATUS    = 0x1000,
    DPS_NOT_RECOGNIZED_OPERATION   = 0x2000,
};


TRANSACTION_STATE eUsbProcessPtp(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);
TRANSACTION_STATE eUsbPtpDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);
TRANSACTION_STATE eUsbPtpDataIn(WHICH_OTG eWhichOtg);
TRANSACTION_STATE eUsbPtpSendResponse(WHICH_OTG eWhichOtg);
TRANSACTION_STATE eUsbPtpSendEvent(WHICH_OTG eWhichOtg);
TRANSACTION_STATE DpsProcessJob(DWORD dpsState, WHICH_OTG eWhichOtg);
void UsbSidcInit(WHICH_OTG eWhichOtg);
void SetDpsState(DWORD dpsState, WHICH_OTG eWhichOtg);
DWORD GetDpsState(WHICH_OTG eWhichOtg);
void PtpHandlesAlloc(DWORD dwCnt, WHICH_OTG eWhichOtg);
void PtpHandlesFree(WHICH_OTG eWhichOtg);

//void vSidcCancelRequest(WORD *pCnt, BYTE **handle);
//void vSidcGetDeviceStatus(WORD *pCnt, BYTE **handle);

//#endif  // SC_USBDEVICE

#endif // USBOTG_DEVICE_SIDC_H

