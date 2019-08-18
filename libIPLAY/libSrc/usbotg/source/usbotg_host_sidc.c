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
* Filename      : usbotg_host_sidc.c
* Programmer(s) : Morse Chen
* Created       : 2008-06-25
* Descriptions  :
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
#include "Usbotg_host_sidc.h"
#include "Usbotg_host_setup.h"

#include "taskid.h"
#include "os.h"
#include "ui.h"

#if (SC_USBHOST==ENABLE)
/*
// Constant declarations
*/
#define MAX_READ_BYTE_CNT               (Host20_SIDC_BULK_RX_BUFFER_SIZE-USB_STI_CONTAINER_SIZE) //0x20000 
#define NUM_STORAGEID			        1
//#define NUM_OPERATION_SUPPORTED			0x00000012 
//#define NUM_EVENT_SUPPORTED				0x00000001 
//#define NUM_DEVICE_PROP_CODE_SUPPORTED	0x00000003 
//#define NUM_OBJECT_FORMAT_SUPPORTED		0x00000002

#define STI_OBJECT_INFO_SIZE        88    

#define STI_BUFFER_SIZE			1024

#define STI_CONTAINER_BUFFER_SIZE	1024*800		// 700 kb //0x100000	// 1 MB
#define NUMBER_OF_DIRECTORY_OBJECT	100
#define NUMBER_OF_CHILD_OBJECT		1000
#define NUMBER_OF_IMAGE_OBJECT		1000


//#define kUSBInterruptPipe 		USB_FIFO_EP2_IN
#define PHYSICAL_STORAGEID		0x00010000
#define IMAGE_BIT_DEPTH			24


//
// PIMA 15740 Definition
// Data Length for Opration, Response and Event
//
#define OPERATION_REQUEST_LENGTH		30
#define RESPONSE_LENGTH                 30
#define EVENT_LENGTH				    26



//
// PIMA 15740 Definition
// functional mode
//

enum {
    STANDARD_MODE	= 0x0000,
    SLEEP_MODE		= 0x0001
};

//
// PIMA 15740 Definition
//version and string Length for device info
//

//#define NULL						0x0000
#define ISINVALID					-1
#define NOTDEFINED					-1
#define UNUSED						0xFFFFFFFF

#define NUM_OF_OBJECTS_OF_ALL_FORMATS		0xFFFFFFFF
#define FROM_OFFSET_TO_END					0xFFFFFFFF
#define DELETE_ALLOBJECTS					0xFFFFFFFF


#define PARAMETER_IS_UNUSED				0x00000000
#define NUMOFROOT						0x00000001

//#define NOT_SUPPORT					    0x00000000
#define NO_NIMAGE						0x00000000
#define DEFAULT_STORAGE					0x00000000

#define STI_GENERIC_CONTAINER_LENGTH	32
#define STANDARD_VERSION				0x0064          // 100
#define VENDOR_EXTENSION_ID				0x00000000      //0	// not support
#define VENDOR_EXTENSION_VERSION		0x0000          // 100	// v 1.00
#define VENDOR_EXTENSION_DESC_LENGTH	0	        // not support
#define FUNCTIONAL_MODE					STANDARD_MODE
#define MANUFACTURER_STRING_LENGTH		32	            
#define MODEL_STRING_LENGTH				32	            
#define DEVICE_VERSION_STRING_LENGTH	8	            
#define SERIAL_NUMBER_STRING_LENGTH		32	            
#define INTERNAL_FLASH_STRING_LENGTH	21	            // Internal Flash Memory
#define CF_CARD_STRING_LENGTH			18	            // Compact Flash Card
#define VOLUME_LABLE_STRING_LENGTH		0	            // Internal Flash Memory

#define DIR_PATERN						0x00ff0000
#define DEVICE_STORAGE_ID				0x00010001
#define ALL_DEVICE_STORAGE_ID			0xFFFFFFFF

#define MAX_NUM_OF_OBJECTS          1000
#define PTP_DPS_MAX_XFR             8*1024   //for 100 files(XML) (PTP_DPS_MAX_XFR = DPS_BUFF_SIZE //DPS_main.c) Calvin 2004.09.02
//
// Still Image Capture Device Definition
// Class-Specific Requests
//

enum {
	STI_HOST_TO_DEVICE		= 0x21,
	STI_DEVICE_TO_HOST		= 0xA1
};


//
// Still Image Capture Device Definition
// Container Type
//

enum {
	COMMAND_BLOCK		= 0x0001,
	DATA_BLOCK			= 0x0002,
	RESPONSE_BLOCK		= 0x0003,
	EVENT_BLOCK		    = 0x0004 
};


//
// PIMA 15740 Definition
// Event Code
//

enum {
	EVENT_UNDEFINED				    = 0x4000,
	EVENT_CANCEL_TRANSACTION		= 0x4001, 
	EVENT_OBJECT_ADDED			    = 0x4002, 
	EVENT_OBJECT_REMOVED			= 0x4003, 
	EVENT_STORE_ADDED			    = 0x4004, 
	EVENT_STORE_REMOVED			    = 0x4005, 
	EVENT_DEVICE_PROP_CHANGED		= 0x4006, 
	EVENT_OBJECT_INFO_CHANGED		= 0x4007, 
	EVENT_DEVICE_INFO_CHANGED		= 0x4008, 
	EVENT_REQUEST_OBJECT_TRANSFER	= 0x4009, 
	EVENT_STORE_FULL				= 0x400a, 
	EVENT_DEVICE_RESET			    = 0x400b, 
	EVENT_STORAGE_INFO_CHANGED	    = 0x400c, 
	EVENT_CAPTURE_COMPLETE		    = 0x400d, 
	EVENT_UNREPORTED_STATUS		    = 0x400e
};

//
// PIMA 15740 Definition
// ObjectFormat Code
//
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
//
// PIMA 15740 Definition
// DeviceProp Code
//

enum {
	DEVICE_PROP_UNDEFINED					= 0x5000,	
	DEVICE_PROP_BATTERY_LEVEL			    = 0x5001,  	// Get
	DEVICE_PROP_FUNCTIONA_MODE				= 0x5002,	 
	DEVICE_PROP_IMAGE_SIZE					= 0x5003,	// Get 
	DEVICE_PROP_COMPRESSION_SETTING			= 0x5004,	
	DEVICE_PROP_WHITE_BALANCE				= 0x5005,	// Get / Set
	DEVICE_PROP_RGB_GAIN					= 0x5006,	
	DEVICE_PROP_F_NUMBER					= 0x5007,	
	DEVICE_PROP_FOCAL_LENGTH				= 0x5008,	
	DEVICE_PROP_FOCUS_DISTANCE				= 0x5009,	
	DEVICE_PROP_FOCUS_MODE					= 0x500a,	// Get / Set
	DEVICE_PROP_EXPOSURE_METERING_MODE		= 0x500b,	
	DEVICE_PROP_FLASH_MODE					= 0x500c,	// Get / Set
	DEVICE_PROP_EXPOSURE_TIME				= 0x500d,	
	DEVICE_PROP_EXPOSURE_PROGRAMMODE		= 0x500e,	
	DEVICE_PROP_EXPOSURE_INDEX				= 0x500f,	
	DEVICE_PROP_EXPOSURE_BIAS_COMPENSATION	= 0x5010,	
	DEVICE_PROP_DATETIME					= 0x5011,	// Get / Set
	DEVICE_PROP_CAPTURETIME					= 0x5012,	
	DEVICE_PROP_STILL_CAPTURE_MODE			= 0x5013,	
	DEVICE_PROP_CONTRAST					= 0x5014,  	
	DEVICE_PROP_SHARPNESS					= 0x5015,  	
	DEVICE_PROP_DIGITAL_ZOOM				= 0x5016,  	// Get / Set
	DEVICE_PROP_EFFECT_MODE					= 0x5017,  	
	DEVICE_PROP_BURST_NUMBER				= 0x5018,  	
	DEVICE_PROP_BURST_INTERVAL				= 0x5019,  	
	DEVICE_PROP_TIMELAPSE_NUMBER			= 0x501a,	
	DEVICE_PROP_TIMELAPSE_INTERVAL			= 0x501b,  	
	DEVICE_PROP_FOCUS_METERING_MODE			= 0x501c,  	
	DEVICE_PROP_UPLOAD_URL					= 0x501d,  	
	DEVICE_PROP_ARTIST						= 0x501e,  	
	DEVICE_PROP_COPY_RIGHT_INFO				= 0x501f	
};

// Storage Type
enum {
	UNDEFINED      = 0x0000,
	FIXED_ROM       = 0x0001,
	REMOVABLE_ROM   = 0x0002,
	FIXE_DRAM       = 0x0003,
	REMOVABLE_RAM   = 0x0004
};


// StorageInfo AccessCapability Values
enum {
	READ_WRITE                      = 0x0000,
	READ_ONLY_WITHOUT_OBJECT_DELETION  = 0x0001,
	READ_ONLY_WITH_OBJECT_DELETION     = 0x0002
};

// Association Types
enum {
	NOT_AN_ASSOCIATION 	= 0x0000,
	GENERIC_FOLDER 		= 0x0001,
	ALBUM 				= 0x0002,
	TIME_SEQUENCE 		= 0x0003,
	HORIZONTAL_PANORAMIC = 0x0004,
	VERTICAL_PANORAMIC 	= 0x0005,
	TWOD_PANORAMIC 		= 0x0006,
	ANCILLARY_DATA 		= 0x0007
};


// Data Source
enum {
	PTP_DATA_FROM_BUFFER	= 0,
	PTP_DATA_FROM_FILE		= 1,
	PTP_DATA_TO_BUFFER		= 2,
	PTP_DATA_TO_FILE		= 3,
	PTP_DATA_IDLE			= 4,
};



/*
// Structure declarations
*/
//
// PIMA 15740 Definition
// Data Types Definition
//

//
// Still Image Capture Device Definition
// Data Structure Definition
//

/*
// Event Structure: 24 bytes; refer to usb_stil_imag.pdf chapter 7.3.1 p29
typedef struct {
	DWORD	length;
	WORD	type;
	WORD	eventCode;
//	DWORD	sessionID;
	DWORD	transactionID;
	DWORD	parameter[3];
} STI_EVENT, *PSTI_EVENT; 
*/
// Image File Info Structure record the object handle, directory index and file index
// to easy to get the image data from our filesystem 
typedef struct {
	DWORD	objectHandle;
	WORD	dirIndex;
	WORD	fileIndex;
} OBJECT_FILE_INFO, *POBJECT_FILE_INFO; 


/*
typedef struct
{
        BYTE   numChars;
        WORD  stringChars[8];
} STR08, *PSTR08;

typedef struct
{
        BYTE   numChars;
        BYTE  stringChars[32];
} STR16, *PSTR16;
*/
typedef struct
{
        BYTE   numChars;
        WORD  stringChars[24];
} STR24, *PSTR24;
/*
typedef struct
{
        BYTE   numChars;
        WORD  stringChars[32];
} STR32, *PSTR32;
*/




typedef struct {
	DWORD		numElements;
	DWORD*		arrayEntry;
} AUINT32, *PAUINT32; 

// storageID array
typedef struct {
	DWORD		numElements;
	DWORD		arrayEntry[NUM_STORAGEID];
} STORAGE_IDARRAY, *PSTORAGE_IDARRAY; 
/*
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

// ObjectInfo array
typedef struct {
	DWORD		numElements;
	DWORD*		pObjectInfo;
} OBJECT_INFO_ARRAY, *POBJECT_INFO_ARRAY; 
*/

// ObjectInfo ProtectionStatus Values
enum {
	kNo_Protection = 0x0000,
	kRead_Only = 0x0001
};

/*
//
// PIMA 15740 Definition
// DeviceInfo Dataset
//

typedef struct {
	WORD						standardVersion;
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
//
// PIMA 15740 Definition
// StorageInfo Dataset
//


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
	STR24		captureDate;
	STR24		modificationDate;
	STR00		keyWords;
}STI_DIR_OBJECT_INFO, *PSTI_DIR_OBJECT_INFO;
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
// Variable declarations
*/
BOOL UsbOtgHostGetSidcScanObjects(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	return psSidc->boScanObjects;//sbUsbOtgHostSidcScanObjects;
}

BOOL UsbOtgHostGetSidcPartialObject(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	return psSidc->boGetPartialObject;//sbUsbOtgHostSidcGetPartialObject;
}

void UsbOtgHostSetSidcScanObjects(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	psSidc->boScanObjects = TRUE;//sbUsbOtgHostSidcScanObjects = TRUE;
}

void UsbOtgHostSetSidcPartialObject(DWORD dwBuffer, DWORD dwCount, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	psSidc->boGetPartialObject = TRUE;
	psSidc->dwBuffer = dwBuffer;
	psSidc->dwCount = dwCount;
}

void UsbOtgHostClrSidcScanObjects(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	psSidc->boScanObjects = FALSE;
}

void UsbOtgHostClrSidcPartialObject(BOOL copy, WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;
    PUSB_HOST_EHCI pEhci;

    pEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	psSidc->boGetPartialObject = FALSE;
	if (psSidc->dwCount != 0 && psSidc->dwBuffer != 0 && copy)
		memcpy((BYTE*)psSidc->dwBuffer, (((BYTE*)pEhci->dwHostSidcRxBufferAddress)+USB_STI_CONTAINER_SIZE), psSidc->dwCount);
	psSidc->dwCount = 0;
	psSidc->dwBuffer = 0;
}

BOOL UsbOtgHostGetSidcObject(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	return psSidc->boGetObject;
}

void UsbOtgHostSetSidcObject(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	psSidc->boGetObject = TRUE;
}

void UsbOtgHostClrSidcObject(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	psSidc->boGetObject = FALSE;
}

BOOL UsbOtgHostGetSidcObjectInfo(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	return psSidc->boGetObjectInfo;
}

void UsbOtgHostSetSidcObjectInfo(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	psSidc->boGetObjectInfo = TRUE;
}

void UsbOtgHostClrSidcObjectInfo(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_SIDC psSidc;

    psSidc = (PUSB_HOST_SIDC) UsbOtgHostSidcDsGet(eWhichOtg);
	UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
	psSidc->boGetObjectInfo = FALSE;
}

void UsbOtgHostSidcDevicePlugOut(WHICH_OTG eWhichOtg)
{
    PUSB_HOST_EHCI psEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    BYTE bTxDoneEventId = UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg);

	if (UsbOtgHostGetSidcPartialObject(eWhichOtg))
	{
		UsbOtgHostClrSidcPartialObject(FALSE, eWhichOtg);
	}
	if (UsbOtgHostGetSidcScanObjects(eWhichOtg))
	{
		UsbOtgHostClrSidcScanObjects(eWhichOtg);
		EventSet(bTxDoneEventId, EVENT_MSDC_TRANSACTION_FAILED);
	}
	if (UsbOtgHostGetSidcObject(eWhichOtg))
	{
		UsbOtgHostClrSidcObject(eWhichOtg);
		EventSet(bTxDoneEventId, EVENT_MSDC_TRANSACTION_FAILED);
	}
	if (UsbOtgHostGetSidcObjectInfo(eWhichOtg))
	{
		UsbOtgHostClrSidcObjectInfo(eWhichOtg);
		EventSet(bTxDoneEventId, EVENT_MSDC_TRANSACTION_FAILED);
	}
	if (psEhci->dwHostSidcGetObjecRxBufferAddress)
	{
		ext_mem_free((void *)psEhci->dwHostSidcGetObjecRxBufferAddress);
		psEhci->dwHostSidcGetObjecRxBufferAddress = 0;
	}
}

/*
// Macro declarations
*/

/*
// Static function prototype
*/
static void StiCmdBuilder(  PST_APP_CLASS_DESCRIPTOR pAppSidc,
                            WORD    opCode,
                            DWORD*  pData_in_length,
                            DWORD*  pData_out_length,
                            DWORD*  pData_addr,
                            DWORD*  pResp_Data_in_length,
                            WHICH_OTG eWhichOtg);

SDWORD UsbhStorage_PtpScanObject(DWORD dwExtArrayAddr, DWORD dwRecordElementAddr, ST_SEARCH_INFO* psSearchInfo, DWORD dwMaxElement, WHICH_OTG eWhichOtg)
{
    PUSBH_PTP_MAIL_TAG	pSendMailDrv;
    BYTE				mail_id;
    SDWORD				osSts;
	DWORD				dwEvent;

	MP_DEBUG("UsbhStorage_PtpScanObject");

	if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
		return USB_HOST_DEVICE_PLUG_OUT;

    pSendMailDrv = (PUSBH_PTP_MAIL_TAG) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
    memset(pSendMailDrv, 0, sizeof(PUSBH_PTP_MAIL_TAG));

    pSendMailDrv->psSearchInfo              = psSearchInfo;
    pSendMailDrv->dwMaxElement              = dwMaxElement;
    pSendMailDrv->dwBuffer                  = dwExtArrayAddr;
    pSendMailDrv->dwRecordElementAddr       = dwRecordElementAddr;
    pSendMailDrv->wCmd                      = FOR_USBH_PTP_SCAN_OBJECT_NOT_MCARD_CMD;
    pSendMailDrv->wStateMachine             = SIDC_SCAN_OBJECTS_SM;
    pSendMailDrv->wCurrentExecutionState    = SIDC_SO_START_STATE;
    pSendMailDrv->wMCardId                  = eWhichOtg ? DEV_USBOTG1_HOST_PTP : DEV_USB_HOST_PTP;
    
    osSts = MailboxSend( UsbOtgHostMailBoxIdGet(eWhichOtg),
                        ((BYTE *)pSendMailDrv),
                        sizeof(USBH_PTP_MAIL_TAG),
                        &mail_id);
	//MailTrack(mail_id);
	if (osSts != OS_STATUS_OK)
	{
		MP_DEBUG1("UsbhStorage_PtpScanObject: MailboxSend Failed %d", osSts);
		return osSts;
	}

	UsbOtgHostSetSidcScanObjects(eWhichOtg);
	osSts = EventWaitWithTO (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg),
                                0x7fffffff,
                                OS_EVENT_OR,
                                &dwEvent,
                                180000);
	
    if (osSts != OS_STATUS_OK)
    {
		MP_DEBUG1("UsbhStorage_PtpScanObject: EventWaitWithTO error %d", osSts);
        if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
        { // plug out
            SetHostFinalizedFlag(eWhichOtg);
            osSts = USB_HOST_DEVICE_PLUG_OUT;            
        }
        else
        { // reset device
            SetHostToResetDeviceFlag(eWhichOtg);
			osSts = USB_UNKNOW_ERROR; 
        }
    }
    return osSts;
}

SWORD UsbOtgHostSidcCommand ( PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes,
                        PST_APP_CLASS_DESCRIPTOR    pAppSidc,
                        WORD                        opCode,
						WHICH_OTG                   eWhichOtg)
{
    WORD    err = USB_NO_ERROR;
    WORD    response_code = 0;
#if USBOTG_HOST_DESC
    if (GetInterfaceProtocol(pUsbhDevDes, pUsbhDevDes->bDeviceConfigIdx, pUsbhDevDes->bDeviceInterfaceIdx, AlterSettingDefaultIdx)
		        == PIMA_15740_BULK_ONLY_PROTOCOL)
#else    
    if (pUsbhDevDes->sInterfaceDescriptor[pUsbhDevDes->bDeviceInterfaceIdx].bInterfaceProtocol 
				== PIMA_15740_BULK_ONLY_PROTOCOL)
#endif
    {
		StiCmdBuilder(  pAppSidc,
						opCode,
						&pAppSidc->dDataInLen,
						&pAppSidc->dDataOutLen,
						&pAppSidc->dDataBuffer,
						&pAppSidc->sStiResponse.containerLength,
						eWhichOtg
						);

		pAppSidc->dwBulkOnlyState   = BULKONLY_SIDC_OP_REQUEST_STATE;
		pUsbhDevDes->psAppClass     = pAppSidc;
		EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
    }

    return err;
}

SWORD UsbOtgHostSidcGetPartialObject(DWORD                      dwIdx,
                                    DWORD                       dwCount,
                                    DWORD                       dwBuffer,
                                    DWORD                       dwObjectHandle,
                                    WHICH_OTG                   eWhichOtg)
{
    ST_MCARD_MAIL	*pSendMailDrv;
	DWORD			dwEvent, index = 0;
	SWORD			rtn = USB_NO_ERROR;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pAppSidc;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pAppSidc = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
	MP_DEBUG("UsbOtgHostSidcGetPartialObject: len = %x idx = %x dwObjectHandle = %x", dwCount, dwIdx, dwObjectHandle);

	pAppSidc->dObjectHandlesIdx = MAX_NUM_OF_OBJECT_HANDLES;
	while ((pAppSidc->dObjectHandles[index] != 0) && (index < MAX_NUM_OF_OBJECT_HANDLES))
	{
		if (pAppSidc->dObjectHandles[index] == dwObjectHandle)
		{
			pAppSidc->dObjectHandlesIdx = index;
			break;
		}
		++index;
	}
	if (pAppSidc->dObjectHandlesIdx == MAX_NUM_OF_OBJECT_HANDLES)
	{
		rtn = USB_UNKNOW_ERROR;
		return rtn;
	}

    pAppSidc->dDataInLen        = dwCount;
    pAppSidc->dDataOutLen       = 0;
    pAppSidc->dDataBuffer       = (DWORD)(dwBuffer|0xa0000000);
    pAppSidc->dLba              = dwIdx;

	pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
	pSendMailDrv->wStateMachine = SIDC_GET_PARTIAL_OBJECTS_SM;
	pSendMailDrv->wCurrentExecutionState = SIDC_GP_GET_PARTIAL_OBJECT_DONE_STATE;

	rtn = UsbOtgHostSidcCommand(pUsbhDevDes, pAppSidc, OPERATION_GET_PARTIAL_OBJECT, eWhichOtg);

	if (rtn != USB_NO_ERROR)
	{
		MP_DEBUG1("UsbOtgHostSidcGetPartialObject: err %x" , rtn);
	}

	rtn = EventWaitWithTO (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg),
                                0x7fffffff,
                                OS_EVENT_OR,
                                &dwEvent,
                                2000);

    if (rtn != OS_STATUS_OK)
    {
		MP_DEBUG1("UsbOtgHostSidcGetPartialObject: EventWaitWithTO error %d", rtn);
        if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
        { // plug out
            rtn = USB_HOST_DEVICE_PLUG_OUT;            
        }
        else
        { // reset device
            SetHostToResetDeviceFlag(eWhichOtg);
			rtn = USB_UNKNOW_ERROR; 
        }
    }

    return rtn;
}

SWORD UsbOtgHostSidcGetObject(	DWORD                       dwCount,
								DWORD                       dwBuffer,
								DWORD                       dwObjectHandle,
								WHICH_OTG                   eWhichOtg)
{
    ST_MCARD_MAIL	*pSendMailDrv;
	DWORD			dwEvent, index = 0;
	SWORD			rtn = USB_NO_ERROR;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pAppSidc;

    
    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pAppSidc = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
	MP_DEBUG("UsbOtgHostSidcGetObject: len = %x dwObjectHandle = %x dwBuffer = %x", dwCount, dwObjectHandle, dwBuffer);

	pAppSidc->dObjectHandlesIdx = MAX_NUM_OF_OBJECT_HANDLES;
	while ((pAppSidc->dObjectHandles[index] != 0) && (index < MAX_NUM_OF_OBJECT_HANDLES))
	{
		if (pAppSidc->dObjectHandles[index] == dwObjectHandle)
		{
			pAppSidc->dObjectHandlesIdx = index;
			break;
		}
		++index;
	}
	if (pAppSidc->dObjectHandlesIdx == MAX_NUM_OF_OBJECT_HANDLES)
	{
		rtn = USB_UNKNOW_ERROR;
		return rtn;
	}

    pAppSidc->dDataInLen        = dwCount;
    pAppSidc->dDataOutLen       = 0;
    pAppSidc->dDataBuffer       = (DWORD)(dwBuffer|0xa0000000);

	pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
	pSendMailDrv->wStateMachine = SIDC_GET_OBJECT_SM;
	pSendMailDrv->wCurrentExecutionState = SIDC_GO_GET_OBJECT_DONE_STATE;

	rtn = UsbOtgHostSidcCommand(pUsbhDevDes, pAppSidc, OPERATION_GET_OBJECT, eWhichOtg);

	if (rtn != USB_NO_ERROR)
	{
		MP_DEBUG1("UsbOtgHostSidcGetObject: err %x" , rtn);
	}

	rtn = EventWaitWithTO (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg),
                                0x7fffffff,
                                OS_EVENT_OR,
                                &dwEvent,
                                10000);

    if (rtn != OS_STATUS_OK)
    {
		MP_DEBUG1("UsbOtgHostSidcGetObject: EventWaitWithTO error %d", rtn);
        if (UsbOtgHostConnectStatusGet(eWhichOtg)==0)
        { // plug out
            rtn = USB_HOST_DEVICE_PLUG_OUT;            
        }
        else
        { // reset device
            SetHostToResetDeviceFlag(eWhichOtg);
			rtn = USB_UNKNOW_ERROR; 
        }
    }

    return rtn;
}

SWORD UsbOtgHostSidcGetObjectInfo(	DWORD                       dwBuffer,
									DWORD                       dwObjectHandle,
									WHICH_OTG                   eWhichOtg)
{
    ST_MCARD_MAIL	*pSendMailDrv;
	SWORD			rtn = USB_NO_ERROR;
	DWORD			dwEvent;
	DWORD			index = 0;
    PST_USBH_DEVICE_DESCRIPTOR  pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR    pAppSidc;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);
    pAppSidc = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);

	MP_DEBUG("UsbOtgHostSidcGetObjectInfo: dwObjectHandle = %x", dwObjectHandle);

	pAppSidc->dObjectHandlesIdx = MAX_NUM_OF_OBJECT_HANDLES;
	while ((pAppSidc->dObjectHandles[index] != 0) && (index < MAX_NUM_OF_OBJECT_HANDLES))
	{
		if (pAppSidc->dObjectHandles[index] == dwObjectHandle)
		{
			pAppSidc->dObjectHandlesIdx = index;
			break;
		}
		++index;
	}
	if (pAppSidc->dObjectHandlesIdx == MAX_NUM_OF_OBJECT_HANDLES)
	{
		rtn = USB_UNKNOW_ERROR;
		return rtn;
	}
	pAppSidc->dDataBuffer       = (DWORD)(dwBuffer|0xa0000000);

	pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
	pSendMailDrv->wStateMachine = SIDC_GET_OBJECT_INFO_SM;
	pSendMailDrv->wCurrentExecutionState = SIDC_GOINFO_GET_OBJECT_INFO_DONE_STATE;

	rtn = UsbOtgHostSidcCommand(pUsbhDevDes, pAppSidc, OPERATION_GET_OBJECT_INFO, eWhichOtg);

	if (rtn != USB_NO_ERROR)
	{
		MP_DEBUG1("UsbOtgHostSidcGetObjectInfo: err %x" , rtn);
	}

	rtn = EventWaitWithTO (UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg),
                                0x7fffffff,
                                OS_EVENT_OR,
                                &dwEvent,
                                5000);

    if (rtn != OS_STATUS_OK)
    {
		MP_DEBUG1("UsbOtgHostSidcGetObjectInfo: EventWaitWithTO error %d", rtn);
        if (UsbOtgHostConnectStatusGet(eWhichOtg)==0)
        { // plug out
            rtn = USB_HOST_DEVICE_PLUG_OUT;            
        }
        else
        { // reset device
            SetHostToResetDeviceFlag(eWhichOtg);
			rtn = USB_UNKNOW_ERROR; 
        }
    }

    return rtn;
}

SWORD UsbOtgHostSidcReadData (	DWORD                       dwIdx,
								DWORD                       dwCount,
								DWORD                       dwBuffer,
								DWORD                       dwObjectHandle,
								WHICH_OTG                   eWhichOtg)
{
	DWORD dwEvent;
	SWORD   sts = 0;
    PUSB_HOST_EHCI              pEhci;
    PST_APP_CLASS_DESCRIPTOR    pAppSidc;

    pEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    pAppSidc = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);
	MP_DEBUG("UsbOtgHostSidcReadData: Object Handle = %x", dwObjectHandle);
	if (CheckIfUsbhReadyToReadWrite(eWhichOtg) == FALSE || GetUsbhCardPresentFlag((eWhichOtg ? (DEV_USBOTG1_HOST_PTP - 1):(DEV_USB_HOST_PTP-1)), eWhichOtg) == FALSE) 
		return FAIL;

	if (pAppSidc->dOPUsed == OPERATION_GET_PARTIAL_OBJECT)
	{
		MP_DEBUG("UsbOtgHostSidcReadData: offset = %x count = %x", dwIdx, dwCount);

		while (dwCount > 0)
		{
			if (UsbOtgHostConnectStatusGet(eWhichOtg) == 0)
				return USB_HOST_DEVICE_PLUG_OUT;

			if (dwCount > MAX_READ_BYTE_CNT)
			{
				UsbOtgHostSetSidcPartialObject(dwBuffer, MAX_READ_BYTE_CNT, eWhichOtg);
				sts = UsbOtgHostSidcGetPartialObject (	dwIdx,
    													MAX_READ_BYTE_CNT,
    													pEhci->dwHostSidcRxBufferAddress,
    													dwObjectHandle,
    													eWhichOtg);
													//pUsbhDevDes,
													//pAppSidc);
				dwCount -= MAX_READ_BYTE_CNT;
				dwBuffer += MAX_READ_BYTE_CNT;
				dwIdx += MAX_READ_BYTE_CNT;
			}
			else
			{
				UsbOtgHostSetSidcPartialObject(dwBuffer, dwCount, eWhichOtg);
				sts = UsbOtgHostSidcGetPartialObject (	dwIdx,
    													dwCount,
    													pEhci->dwHostSidcRxBufferAddress,
    													dwObjectHandle,
                                                        eWhichOtg);
                                                    //pUsbhDevDes,
                                                    //pAppSidc);
				dwCount = 0;
			}
		}
	}
	else if (pAppSidc->dOPUsed == OPERATION_GET_OBJECT)
	{

		if (dwIdx == 0)
		{
			unsigned char *pc = NULL;
			DWORD fileSize = 0;

            if (pEhci->dwHostSidcGetObjecRxBufferAddress)
				ext_mem_free((void *)pEhci->dwHostSidcGetObjecRxBufferAddress);
                
			UsbOtgHostSetSidcObjectInfo(eWhichOtg);
			UsbOtgHostSidcGetObjectInfo(pEhci->dwHostSidcRxBufferAddress, dwObjectHandle, eWhichOtg);//pUsbhDevDes, pAppSidc);
			pc = ((char *)pEhci->dwHostSidcRxBufferAddress) + 20;
			fileSize = pc[0] + (pc[1] << 8) + (pc[2] << 16) + (pc[3] << 24);
			pEhci->dwHostSidcGetObjecRxBufferAddress = ext_mem_malloc(fileSize + 24*1024 + USB_STI_CONTAINER_SIZE);
			if (pEhci->dwHostSidcGetObjecRxBufferAddress == NULL)
			{
				mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: OPERATION_GET_OBJECT size too large");
			}
			else
			{
				UsbOtgHostSetSidcObject(eWhichOtg);
				sts = UsbOtgHostSidcGetObject(	fileSize,
											UsbOtgHostGetSidcGetObjectRxBufferAddress(eWhichOtg),
											dwObjectHandle,
                                            eWhichOtg);
			}
		}
        if (pEhci->dwHostSidcGetObjecRxBufferAddress)
		{
			MP_DEBUG("UsbOtgHostSidcReadData: memcpy offset = %x count = %x", dwIdx, dwCount);
			memcpy((BYTE*)dwBuffer, ((BYTE*)UsbOtgHostGetSidcGetObjectRxBufferAddress(eWhichOtg))+dwIdx+USB_STI_CONTAINER_SIZE, dwCount);
		}
	}
	else mpDebugPrint("UsbOtgHostSidcReadData: operation not supported");

    return sts;
}
/*
// Definition of local functions
*/
static void StiCmdBuilder(  PST_APP_CLASS_DESCRIPTOR pAppSidc,
                            WORD    opCode,
                            DWORD*  pData_in_length,
                            DWORD*  pData_out_length,
                            DWORD*  pData_addr,
                            DWORD*  pResp_Data_in_length,
                            WHICH_OTG eWhichOtg)
{
    BYTE    how_many_paramters = 0;
    BYTE    resp_how_many_paramters = 0;

	PUSB_HOST_SIDC psUsbHostSidc = (PUSB_HOST_SIDC)UsbOtgHostSidcDsGet(eWhichOtg);
	PUSB_HOST_EHCI pEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);
    PST_APP_CLASS_DESCRIPTOR pSidcDes = (PST_APP_CLASS_DESCRIPTOR)UsbOtgHostDevAppDescGet(eWhichOtg);

    if (opCode != OPERATION_GET_DEVICE_INFO && opCode != OPERATION_OPEN_SESSION)
        psUsbHostSidc->dwTransactionId++;
    
    memset((BYTE*)&pAppSidc->sStiCommand, 0, sizeof(STI_CONTAINER));
    memset((BYTE*)&pAppSidc->sStiResponse, 0, sizeof(STI_CONTAINER));
    pAppSidc->sStiCommand.containerType  = byte_swap_of_word(COMMAND_BLOCK);
    pAppSidc->sStiCommand.transactionID  = byte_swap_of_dword(psUsbHostSidc->dwTransactionId);
    pAppSidc->sStiCommand.code           = byte_swap_of_word(opCode);

	switch (opCode)
	{
		case OPERATION_GET_DEVICE_INFO:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_DEVICE_INFO");
        	break;

        case OPERATION_OPEN_SESSION:
			MP_DEBUG("StiCmdBuilder: OPERATION_OPEN_SESSION");
        	break;

        case OPERATION_GET_STORAGE_IDS:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_STORAGE_IDS");
        	break;

        case OPERATION_GET_STORAGE_INFO:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_STORAGE_INFO");
        	break;

        case OPERATION_GET_NUM_OBJECTS:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_NUM_OBJECTS");
        	break;

        
        case OPERATION_GET_OBJECT_HANDLES:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_OBJECT_HANDLES");
        	break;

        case OPERATION_GET_OBJECT_INFO:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_OBJECT_INFO");
	        break;

        case OPERATION_GET_PARTIAL_OBJECT:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_PARTIAL_OBJECT");
	        break;
        case OPERATION_GET_OBJECT:
			MP_DEBUG("StiCmdBuilder: OPERATION_GET_OBJECT");
	        break;
		default:
			MP_DEBUG("StiCmdBuilder: default");
			break;
    }

    switch (opCode)
    {
        case OPERATION_GET_DEVICE_INFO:
        {
            *pData_out_length   = 0;
            *pData_in_length    = STI_BUFFER_SIZE;
            *pData_addr         = (DWORD) pEhci->dwHostSidcRxBufferAddress;
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            how_many_paramters  = 0;
            resp_how_many_paramters = 0;
        }
        break;

        case OPERATION_OPEN_SESSION:
        {
            DWORD session_id = 1;
            *pData_out_length       = 0;
            *pData_in_length        = 0;
            how_many_paramters      = 1;
            resp_how_many_paramters = 0;
            pAppSidc->sStiCommand.parameter[0] = byte_swap_of_dword(session_id);
        }
        break;

        case OPERATION_GET_STORAGE_IDS:
        {
            *pData_out_length   = 0;
            *pData_in_length    = STI_BUFFER_SIZE;
            *pData_addr         = (DWORD) pEhci->dwHostSidcRxBufferAddress;
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            how_many_paramters  = 0;
            resp_how_many_paramters = 0;
        }
        break;

        case OPERATION_GET_STORAGE_INFO:
        {
            *pData_out_length       = 0;
            *pData_in_length        = STI_BUFFER_SIZE;
            *pData_addr             = (DWORD) pEhci->dwHostSidcRxBufferAddress;
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            how_many_paramters      = 1;
            resp_how_many_paramters = 0;
            pAppSidc->sStiCommand.parameter[0] = byte_swap_of_dword(pAppSidc->dSorageIds[pAppSidc->dSorageIdsIdx]);
        }
        break;

        case OPERATION_GET_NUM_OBJECTS:
        {
            *pData_out_length       = 0;
            *pData_in_length        = 0;
            how_many_paramters      = 3;
            resp_how_many_paramters = 1;
            pAppSidc->sStiCommand.parameter[0] = ALL_DEVICE_STORAGE_ID;// it's for Cannon IX70//byte_swap_of_dword(pAppSidc->dSorageIds[pAppSidc->dSorageIdsIdx]);
            pAppSidc->sStiCommand.parameter[1] = PARAMETER_IS_UNUSED;
            pAppSidc->sStiCommand.parameter[2] = PARAMETER_IS_UNUSED;// it's for Cannon IX70//byte_swap_of_dword(pAppSidc->dFolderObjectHandles[pAppSidc->dFolderObjectHandlesIdx]);
        }
        break;

        
        case OPERATION_GET_OBJECT_HANDLES:
        {
            *pData_out_length       = 0;
            *pData_in_length        = sizeof(DWORD) * (MAX_NUM_OF_OBJECT_HANDLES+4);
            *pData_addr             = (DWORD) pEhci->dwHostSidcRxBufferAddress;
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            how_many_paramters      = 3;
            resp_how_many_paramters = 0;
            pAppSidc->sStiCommand.parameter[0] = ALL_DEVICE_STORAGE_ID;// it's for Cannon IX70//byte_swap_of_dword(pAppSidc->dSorageIds[pAppSidc->dSorageIdsIdx]);
            pAppSidc->sStiCommand.parameter[1] = PARAMETER_IS_UNUSED;
            pAppSidc->sStiCommand.parameter[2] = PARAMETER_IS_UNUSED;// it's for Cannon IX70//byte_swap_of_dword(pAppSidc->dFolderObjectHandles[pAppSidc->dFolderObjectHandlesIdx]);
        }
        break;

        case OPERATION_GET_OBJECT_INFO:
        {
            *pData_out_length       = 0;
            *pData_in_length        = STI_BUFFER_SIZE;
            *pData_addr             = (DWORD) pEhci->dwHostSidcRxBufferAddress;
            memset((BYTE*)(*pData_addr), 0, *pData_in_length);
            how_many_paramters      = 1;
            resp_how_many_paramters = 0;
            pAppSidc->sStiCommand.parameter[0] = byte_swap_of_dword(pAppSidc->dObjectHandles[pAppSidc->dObjectHandlesIdx]);
        }
        break;

        case OPERATION_GET_PARTIAL_OBJECT:
        {
            DWORD offset = pAppSidc->dLba;
            *pData_out_length   = 0;
            how_many_paramters  = 3;
            resp_how_many_paramters = 1;
            pAppSidc->sStiCommand.parameter[0] = byte_swap_of_dword(pAppSidc->dObjectHandles[pAppSidc->dObjectHandlesIdx]);
            pAppSidc->sStiCommand.parameter[1] = byte_swap_of_dword(offset);
            pAppSidc->sStiCommand.parameter[2] = byte_swap_of_dword(*pData_in_length);
            //gtestg = 1;
        }
        break;
        case OPERATION_GET_OBJECT:
        {
            *pData_out_length   = 0;
            how_many_paramters  = 1;
            resp_how_many_paramters = 0;
            pAppSidc->sStiCommand.parameter[0] = byte_swap_of_dword(pAppSidc->dObjectHandles[pAppSidc->dObjectHandlesIdx]);
        }
        break;
    }
    pAppSidc->sStiCommand.containerLength = byte_swap_of_dword((USB_STI_CONTAINER_SIZE + (sizeof(DWORD) * how_many_paramters)));
	MP_DEBUG("sStiCommand.containerLength %d", byte_swap_of_dword(pAppSidc->sStiCommand.containerLength));
    *pResp_Data_in_length = byte_swap_of_dword((USB_STI_CONTAINER_SIZE + (sizeof(DWORD) * resp_how_many_paramters)));
}

#define MAX_DATA_PAGE           5
#define ONE_PAGE_BYTE_CNT       0x1000  // 4 k
#define ONE_TD_MAX_BYTE_CNT     (ONE_PAGE_BYTE_CNT * MAX_DATA_PAGE) // 20 k
static void UsbOtgSidcBulkOnlyDataInProcessfor4KAlignmentOnly(WHICH_OTG eWhichOtg)
{
    DWORD pbDataPage[MAX_DATA_PAGE]; 
	DWORD pbDataBuf = 0;
    DWORD wTotalLengthRemain = 0, wLastTDByteCnt = 0;
    DWORD i = 0;
    BYTE  dir_flag = 0;
    BYTE  qHD_array_number = 0;
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

	if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_SIDC_DATA_IN_STATE)
	{
		dir_flag = OTGH_DIR_IN;
		qHD_array_number = pUsbhDevDes->psAppClass->bBulkInQHDArrayNum;
		wTotalLengthRemain = pUsbhDevDes->psAppClass->dDataInLen + USB_STI_CONTAINER_SIZE;

		UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)pUsbhDevDes->psAppClass->dDataBuffer,
                                        (DWORD)((DWORD)pUsbhDevDes->psAppClass->dDataBuffer+wTotalLengthRemain),
                                        eWhichOtg);
	}
	else if (pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_SIDC_DATA_OUT_STATE)
	{
		MP_DEBUG("UsbOtgSidcBulkOnlyDataInProcessfor4KAlignmentOnly: support data in only");
		return;
	}

    pbDataBuf = pUsbhDevDes->psAppClass->dDataBuffer;
	if ((pbDataBuf & 0xfff) != 0)
	{
		MP_DEBUG("UsbOtgSidcBulkOnlyDataInProcessfor4KAlignmentOnly: buffer not in 4K-Alignment");
		return;
	}

	if (wTotalLengthRemain > ONE_TD_MAX_BYTE_CNT)
	{
		while (wTotalLengthRemain > 0)
		{
			for (i = 0; i < MAX_DATA_PAGE; i++)
				pbDataPage[i] = 0;
			i = 0;

			if (wTotalLengthRemain > ONE_TD_MAX_BYTE_CNT)
			{
				pbDataPage[0] = pbDataBuf;
				pbDataPage[1] = pbDataPage[0] + ONE_PAGE_BYTE_CNT;
				pbDataPage[2] = pbDataPage[1] + ONE_PAGE_BYTE_CNT;
				pbDataPage[3] = pbDataPage[2] + ONE_PAGE_BYTE_CNT;
				pbDataPage[4] = pbDataPage[3] + ONE_PAGE_BYTE_CNT;
				wTotalLengthRemain -= ONE_TD_MAX_BYTE_CNT;
				pbDataBuf += ONE_TD_MAX_BYTE_CNT;

				flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
														ONE_TD_MAX_BYTE_CNT, 
														&(pbDataPage[0]), 
														0, 
														dir_flag,
														FALSE,
														eWhichOtg);
			}
			else
			{
				for (i = 0; i < MAX_DATA_PAGE; i++)
				{
					pbDataPage[i] = pbDataBuf;
					pbDataBuf += ONE_PAGE_BYTE_CNT;
					wLastTDByteCnt += ONE_PAGE_BYTE_CNT;
				}
				wTotalLengthRemain = 0;
				flib_Host20_Issue_Bulk_Active_Multi_TD ( qHD_array_number,
														wLastTDByteCnt, 
														&(pbDataPage[0]), 
														0, 
														dir_flag,
														TRUE,
														eWhichOtg);
        	}
		}
	}
	else
	{
		for (i = 0; i < MAX_DATA_PAGE; i++)
		{
			pbDataPage[i] = pbDataBuf;
			pbDataBuf += ONE_PAGE_BYTE_CNT;
			wLastTDByteCnt += ONE_PAGE_BYTE_CNT;
		}
		wTotalLengthRemain = 0;

		flib_Host20_Issue_Bulk_Active ( qHD_array_number,
										wLastTDByteCnt, 
										&(pbDataPage[0]), 
										0, 
										dir_flag,
										eWhichOtg);
	}
}

void UsbOtgHostSidcBulkOnlyActive (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;

    pUsbhDevDes = (PST_USBH_DEVICE_DESCRIPTOR)UsbOtgHostDevDescGet(eWhichOtg);

    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState)
    {
        case BULKONLY_SIDC_OP_REQUEST_STATE:
        {
            MP_DEBUG("UsbOtgHostSidcBulkOnlyActive: request");
			Host20_BufferPointerArray_Structure aTemp;
			UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)(&(pUsbhDevDes->psAppClass->sStiCommand.containerLength)),
												(DWORD)((DWORD)(&(pUsbhDevDes->psAppClass->sStiCommand.containerLength))+
												byte_swap_of_dword(pUsbhDevDes->psAppClass->sStiCommand.containerLength)),
												eWhichOtg);
            aTemp.BufferPointerArray[0] = (DWORD)(&(pUsbhDevDes->psAppClass->sStiCommand.containerLength));
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum,
                                            byte_swap_of_dword(pUsbhDevDes->psAppClass->sStiCommand.containerLength), 
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_OUT,
                                            eWhichOtg);
        }
        break;
        case BULKONLY_SIDC_DATA_IN_STATE:
        case BULKONLY_SIDC_DATA_OUT_STATE:
        {
			MP_DEBUG("UsbOtgHostSidcBulkOnlyActive: data %s",
				pUsbhDevDes->psAppClass->dwBulkOnlyState == BULKONLY_SIDC_DATA_IN_STATE ? "in" : "out");
            UsbOtgSidcBulkOnlyDataInProcessfor4KAlignmentOnly(eWhichOtg);
        }
        break;

        case BULKONLY_SIDC_RESPONSE_STATE:
        {
            MP_DEBUG("UsbOtgHostSidcBulkOnlyActive: response");
            Host20_BufferPointerArray_Structure aTemp;
			UsbOtgHostSetSwapBuffer2RangeEnable((DWORD)(&(pUsbhDevDes->psAppClass->sStiResponse.containerLength)),
												(DWORD)((DWORD)(&(pUsbhDevDes->psAppClass->sStiResponse.containerLength))+
												byte_swap_of_dword(pUsbhDevDes->psAppClass->sStiResponse.containerLength)),
												eWhichOtg);
            aTemp.BufferPointerArray[0] = (DWORD)(&(pUsbhDevDes->psAppClass->sStiResponse.containerLength));
            aTemp.BufferPointerArray[1] = 0;	
            aTemp.BufferPointerArray[2] = 0;	
            aTemp.BufferPointerArray[3] = 0;
            aTemp.BufferPointerArray[4] = 0;		
            flib_Host20_Issue_Bulk_Active ( pUsbhDevDes->psAppClass->bBulkInQHDArrayNum,
                                            byte_swap_of_dword(pUsbhDevDes->psAppClass->sStiResponse.containerLength), 
                                            (&aTemp.BufferPointerArray[0]), 
                                            0, 
                                            OTGH_DIR_IN,
                                            eWhichOtg);
        }
        break;

        default:
        {
			mpDebugPrint("UsbOtgHostSidcBulkOnlyActive: default");
        }
        break;
    }
}
void UsbOtgHostSidcBulkOnlyIoc (WHICH_OTG eWhichOtg)
{
    PST_USBH_DEVICE_DESCRIPTOR pUsbhDevDes;
    PST_APP_CLASS_DESCRIPTOR   pAppSidc;
	PUSB_HOST_EHCI pEhci = (PUSB_HOST_EHCI)UsbOtgHostEhciDsGet(eWhichOtg);

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
    
    pAppSidc = pUsbhDevDes->psAppClass;    
    switch (pUsbhDevDes->psAppClass->dwBulkOnlyState)
    {
        case BULKONLY_SIDC_OP_REQUEST_STATE:
        {
            MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: request done");
            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkOutqHD[pUsbhDevDes->psAppClass->bBulkOutQHDArrayNum], eWhichOtg);
			UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
			if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
				mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_OP_REQUEST_STATE err = %x", pUsbhDevDes->bQHStatus);
                pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SIDC_RESPONSE_STATE;
                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature ( pUsbhDevDes->psAppClass->sBulkOutDescriptor.bEndpointAddress,
                                              eWhichOtg);
					if (pUsbhDevDes->psAppClass->dDataBuffer != NULL)
						free1((void*)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);
					return;
                }
            }
            else
            {
                if (pUsbhDevDes->psAppClass->dDataInLen > 0)
                {
	                MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_DATA_IN_STATE");
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SIDC_DATA_IN_STATE;
                }
                else if(pUsbhDevDes->psAppClass->dDataOutLen > 0)
                {
                	MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_DATA_OUT_STATE");
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SIDC_DATA_OUT_STATE;
                }
                else
                {
                	MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_RESPONSE_STATE");
                    pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SIDC_RESPONSE_STATE;
                }
            }

            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }
        break;

        case BULKONLY_SIDC_DATA_IN_STATE:
        {
			MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: data in done");

            flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
			UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);
			pUsbhDevDes->psAppClass->dwBulkOnlyState = BULKONLY_SIDC_RESPONSE_STATE;
            if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
            {
				mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_DATA_IN_STATE err = %x", pUsbhDevDes->bQHStatus);

                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                            eWhichOtg);
					if (pUsbhDevDes->psAppClass->dDataBuffer != NULL)
						free1((void*)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);
					return;
                }
            }
            
            EventSet(UsbOtgHostDriverEventIdGet(eWhichOtg), EVENT_EHCI_ACTIVE_BULK);
        }
        break;
        
        case BULKONLY_SIDC_DATA_OUT_STATE:
        {
        }
        break;

        case BULKONLY_SIDC_RESPONSE_STATE:
        {
		    ST_MCARD_MAIL	*pSendMailDrv;
			SDWORD  osSts;

            MP_DEBUG("UsbOtgHostSidcBulkOnlyIoc: response done");
			flib_Host20_Send_qTD_Done(pUsbhDevDes->hstBulkInqHD[pUsbhDevDes->psAppClass->bBulkInQHDArrayNum], eWhichOtg);
			UsbOtgHostSetSwapBuffer2RangeDisable(eWhichOtg);

			if (pUsbhDevDes->psAppClass->dDataBuffer != NULL)
				free1((void*)pUsbhDevDes->psAppClass->dDataBuffer, eWhichOtg);

			if (pUsbhDevDes->bQHStatus != USB_NO_ERROR)
			{
				mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: BULKONLY_SIDC_RESPONSE_STATE err = %x", pUsbhDevDes->bQHStatus);

                if (pUsbhDevDes->bQHStatus & HOST20_qTD_STATUS_Halted)
                {
                    SWORD err = 0;
                    pUsbhDevDes->psAppClass->swBulkOnlyError = USB_STALL_ERROR;
                    err = SetupClearFeature(pUsbhDevDes->psAppClass->sBulkInDescriptor.bEndpointAddress,
                                            eWhichOtg);
					return;
                }
            }

			pSendMailDrv = (ST_MCARD_MAIL*) GetUsbhMailEnvelope(USBOTG_HOST_FOR_PTP_SENDER, eWhichOtg);
			if ((pSendMailDrv->wStateMachine == SIDC_INIT_SM) &&
					((pSendMailDrv->wCurrentExecutionState == SIDC_INIT_GET_DEVICE_INFO_DONE_STATE) ||
					 (pSendMailDrv->wCurrentExecutionState == SIDC_INIT_OPEN_SESSION_DONE_STATE)))
			{

				if (pSendMailDrv->wCurrentExecutionState == SIDC_INIT_GET_DEVICE_INFO_DONE_STATE)
				{
					unsigned char *pc = ((char *)pEhci->dwHostSidcRxBufferAddress);
					DWORD totalOP;
					DWORD index;

					pc += (2*pc[20] + 23);
					totalOP = pc[0] + (pc[1] << 8) + (pc[2] << 16) + (pc[3] << 24);
					pAppSidc->dOPUsed = 0;
					pc += 4;

					for (index = 0; index < totalOP; ++index, pc += 2)
					{
						if ((pc[0] + (pc[1] << 8)) == OPERATION_GET_PARTIAL_OBJECT)
							pAppSidc->dOPUsed = OPERATION_GET_PARTIAL_OBJECT;
						if ((pAppSidc->dOPUsed == 0) && ((pc[0] + (pc[1] << 8)) == OPERATION_GET_OBJECT))
							pAppSidc->dOPUsed = OPERATION_GET_OBJECT;
					}
				}

				osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv, eWhichOtg);
				if (osSts != OS_STATUS_OK)
				{
					mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: SendMailToUsbOtgHostClassTask failed!!");
				}
			}
			else if ((pSendMailDrv->wStateMachine == SIDC_SCAN_OBJECTS_SM) &&
					((pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_STORAGE_ID_DONE_STATE) ||
					 (pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_STORAGE_INFO_DONE_STATE) ||
					 (pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_NUM_OF_OBJECT_DONE_STATE) ||
					 (pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_OBJECT_HANDLES_DONE_STATE) ||
					 (pSendMailDrv->wCurrentExecutionState == SIDC_SO_GET_OBJECT_INFO_DONE_STATE)))
            {
				osSts = SendMailToUsbOtgHostClassTask((DWORD)pSendMailDrv,eWhichOtg);
				if (osSts != OS_STATUS_OK)
				{
					mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: SendMailToUsbOtgHostClassTask failed!!");
				}
			}
			else if ((pSendMailDrv->wStateMachine == SIDC_GET_PARTIAL_OBJECTS_SM) &&
					(pSendMailDrv->wCurrentExecutionState == SIDC_GP_GET_PARTIAL_OBJECT_DONE_STATE))
            {
				EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
				UsbOtgHostClrSidcPartialObject(TRUE, eWhichOtg);
			}
			else if ((pSendMailDrv->wStateMachine == SIDC_GET_OBJECT_SM) &&
					(pSendMailDrv->wCurrentExecutionState == SIDC_GO_GET_OBJECT_DONE_STATE))
            {
				EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
				UsbOtgHostClrSidcObject(eWhichOtg);
			}
			else if ((pSendMailDrv->wStateMachine == SIDC_GET_OBJECT_INFO_SM) &&
					(pSendMailDrv->wCurrentExecutionState == SIDC_GOINFO_GET_OBJECT_INFO_DONE_STATE))
            {
				EventSet(UsbOtgHostMsdcTxDoneEventIdGet(eWhichOtg), EVENT_MSDC_TRANSACTION_PASSED);
				UsbOtgHostClrSidcObjectInfo(eWhichOtg);
			}
        }
        break;


        default:
        {
			mpDebugPrint("UsbOtgHostSidcBulkOnlyIoc: default");
        }
        break;
    }
}

#endif // SC_USBHOST


