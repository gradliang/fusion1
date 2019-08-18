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
* Filename		: usbotg_device_sidc.c
* Programmer(s)	: Cloud Wu
* Created Date	: 2008/11/24 
* Description:  SIDC (Still Image Device Class) 
*               
*               
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
#include "usbotg_api.h"
#include "usbotg_std.h"
#include "usbotg_device_sidc.h"
#include "ptpdps.h"
#include "fs.h"
#include "taskid.h"
#include "ui.h"
#include "..\libsrc\image\include\jpeg.h"

#if USBOTG_DEVICE_SIDC
/*
// Constant declarations
*/
#define NOT_SUPPORT_PC_PTP       1

#define NUM_STORAGEID			        1
//#define NUM_OPERATION_SUPPORTED			0x00000012 
//#define NUM_EVENT_SUPPORTED				0x00000001 
//#define NUM_DEVICE_PROP_CODE_SUPPORTED	0x00000003 
//#define NUM_OBJECT_FORMAT_SUPPORTED		0x00000002

#define USB_STI_CONTAINER_SIZE		12    //not counting paramteter size
#define STI_OBJECT_INFO_SIZE        88    

#define ANALYSE_BUFFER_SIZE			1024*3		// 3 KB
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

//#define CANCEL_REQUEST_DATA_LENGTH          6
//#define GET_DEVICE_STATUS_REQUETS_LENGTH    4

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

#define GET_OBJECT_IN_THE_ROOT		        0xFFFFFFFF
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
//#define MODEL_STRING_LENGTH				32	            
//#define DEVICE_VERSION_STRING_LENGTH	8	            
//#define SERIAL_NUMBER_STRING_LENGTH		32	            
#define INTERNAL_FLASH_STRING_LENGTH	21	            // Internal Flash Memory
#define CF_CARD_STRING_LENGTH			18	            // Compact Flash Card
#define VOLUME_LABLE_STRING_LENGTH		0	            // Internal Flash Memory

#define DIR_PATERN						0x00ff0000
#define ALL_DEVICE_STORAGE_ID			0xFFFFFFFF

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
/*
// Filesystem Type Values
enum {
//	UNDEFINED          = 0x0000,
	GENERIC_FLAT        = 0x0001,
	GENERIC_HIERACHICAL = 0x0002,
	DCF                = 0x0003
};
*/
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


// Generic Container Head Structure: 12 bytes
typedef struct {
	DWORD	containerLength;
	WORD	containerType;
	WORD	code;
	DWORD	transactionID;
} STI_CONTAINER_HEAD, *PSTI_CONTAINER_HEAD; 
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



typedef struct
{
        BYTE   numChars;
        WORD  stringChars[8];
} STR08, *PSTR08;
/*
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

typedef struct
{
        BYTE   numChars;
        WORD  stringChars[32];
} STR32, *PSTR32;

typedef struct {
	DWORD		numElements;
	DWORD*		arrayEntry;
} AUINT32, *PAUINT32; 

// storageID array
typedef struct {
	DWORD		numElements;
	DWORD		arrayEntry[NUM_STORAGEID];
} STORAGE_IDARRAY, *PSTORAGE_IDARRAY; 

// ObjectInfo array
typedef struct {
	DWORD		numElements;
	DWORD*		pObjectInfo;
} OBJECT_INFO_ARRAY, *POBJECT_INFO_ARRAY; 

// ObjectInfo ProtectionStatus Values
enum {
	kNo_Protection = 0x0000,
	kRead_Only = 0x0001
};


/*
// Variable declarations
*/
WORD	gOperationsSupported[NUM_OPERATION_SUPPORTED] = 
		{ 	
            OPERATION_GET_DEVICE_INFO,
			OPERATION_OPEN_SESSION,
			OPERATION_CLOSE_SESSION,
			OPERATION_GET_STORAGE_IDS,
			OPERATION_GET_STORAGE_INFO,
			
			OPERATION_GET_NUM_OBJECTS,
			OPERATION_GET_OBJECT_HANDLES,
			OPERATION_GET_OBJECT_INFO,
			OPERATION_GET_OBJECT,
			OPERATION_GET_THUMB,
			
			OPERATION_DELETE_OBJECT,
			OPERATION_SEND_OBJECT_INFO,
			OPERATION_SEND_OBJECT,
		//	OPERATION_INITIATE_CAPTURE,
		//	OPERATION_FORMAT_STORE,
		//	OPERATION_SET_OBJECT_PROTECTION,
		//	OPERATION_POWERDOWN,
			
			OPERATION_GET_DEVICE_PROP_DESC,
			OPERATION_GET_DEVICE_PROP_VALUE,
			OPERATION_SET_DEVICE_PROP_VALUE,
			OPERATION_RESET_DEVICE_PROP_VALUE,
		//	OPERATION_TERMINATE_OPEN_CAPTURE,

			OPERATION_GET_PARTIAL_OBJECT
		//	OPERATION_INITIATE_OPEN_CAPTURE
		};

WORD	gEventsSupported[NUM_EVENT_SUPPORTED] = 
		{	
            EVENT_REQUEST_OBJECT_TRANSFER
        //    EVENT_OBJECT_ADDED,
		//	EVENT_STORE_FULL,
		//	EVENT_CAPTURE_COMPLETE
		};

WORD	gDevicePropertiesSupported[NUM_DEVICE_PROP_CODE_SUPPORTED] = 
		{
			DEVICE_PROP_BATTERY_LEVEL,
			DEVICE_PROP_IMAGE_SIZE,
		//	DEVICE_PROP_WHITE_BALANCE,
		//	DEVICE_PROP_FOCUS_MODE,
		//	DEVICE_PROP_FLASH_MODE,

			DEVICE_PROP_DATETIME,
		//	DEVICE_PROP_DIGITAL_ZOOM
		};

WORD	gCaptureFormats[NUM_CAPTURE_FORMAT_SUPPORTED] = 
		{
			OBJ_FORMAT_JPEG_EXIF,
            OBJ_FORMAT_MPEG   
		};

WORD	gImageFormats[NUM_IMAGE_FORMAT_SUPPORTED] = 
		{
			OBJ_FORMAT_JPEG_EXIF,
            OBJ_FORMAT_BMP//OBJ_FORMAT_MPEG   
		};

//WORD	gVendorExtensionDesc[VENDOR_EXTENSION_DESC_LENGTH] =
//		{ 0x004D, 0x0041, 0x0047, 0x0049, 0x0043, 0x0050, 0x0049, 0x0058, 0x0045, 0x004C, 0 }; // "MAGICPIXEL "
//WORD	gVendorExtensionDesc[VENDOR_EXTENSION_DESC_LENGTH] =
//		{ 0 };
		
WORD	gStiManufacturerString[MANUFACTURER_STRING_LENGTH] =
		{ 'M', 'a', 'g', 'i', 'c', 'P', 'i', 'x',
          'e', 'l', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }; // "MAGICPIXEL"

WORD	gModelString[MODEL_STRING_LENGTH] =
		{ 'i', 'P', 'l', 'a', 'y', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }; // "iPlay "

WORD	gDeviceVersionString[DEVICE_VERSION_STRING_LENGTH] =
		{ '0', '1', '.', '0', '0', ' ', ' ', 0 }; // "01.00   "

WORD	gSerialNumberString[SERIAL_NUMBER_STRING_LENGTH] =
		{ '1', '2', '3', '4', '5', '6', '7', '8',
          '9', '0', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }; 


STR16	gParentFileName = 
		{ 5, { 0x0044, 0x0043, 0x0049, 0x004D, 0} };	// DCIM

#pragma alignvar(4) 
const  BYTE gDpsExt[]         = "DPS";
const  BYTE gDpsDDiscStr[]    = "DDISCVRY";
const  BYTE gDpsDReqStr[]     = "DREQUEST";
const  BYTE gDpsDRespStr[]    = "DRSPONSE";
const  BYTE gDpsHDiscStr[]    = "HDISCVRY";
const  BYTE gDpsHReqStr[]     = "HREQUEST";
const  BYTE gDpsHRespStr[]    = "HRSPONSE";
// misc DPS/PictBridge values   
#pragma alignvar(4) 
//BYTE *gpGetDeviceStatusRequest;

//BYTE gCancelRequestData[CANCEL_REQUEST_DATA_LENGTH];
//BYTE gGetDeviceStatusRequest[GET_DEVICE_STATUS_REQUETS_LENGTH];

/*
// External Variable declarations
*/

/*
// Macro declarations
*/
#define min(a,b)   (((a) < (b)) ? (a) : (b))


/*
// Static function prototype
*/
static DWORD GetPrintedFileType(WHICH_OTG eWhichOtg);
static BOOL CheckIfSendingData(WHICH_OTG eWhichOtg);
static BOOL IsHostRequest(WHICH_OTG eWhichOtg);
static void SetHostRequest(WHICH_OTG eWhichOtg);
static void ClearHostRequest(WHICH_OTG eWhichOtg);
static BOOL IsDeviceRequest(WHICH_OTG eWhichOtg);
static void SetDeviceRequest(WHICH_OTG eWhichOtg);
static void ClearDeviceRequest(WHICH_OTG eWhichOtg);
static void RemoveDpsState(DWORD dpsState, WHICH_OTG eWhichOtg);
static void SetPrinterCapability(PRINTER_CAPABILITY *pstPrinterCap, WHICH_OTG eWhichOtg);
static PRINTER_CAPABILITY *GetPrinterCapability(WHICH_OTG eWhichOtg);
static void SetDpsStateAndCapa(DWORD dpsState, PRINTER_CAPABILITY *pstPrinterCap, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE DpsProcess (DWORD dpsState, WHICH_OTG eWhichOtg);
static BOOL IsPrinterConnected(WHICH_OTG eWhichOtg);
static DWORD string_len(WORD* str);
static BOOL PtpCheckStorageID(DWORD storageID, WHICH_OTG eWhichOtg);
static BOOL PtpCheckStoreAvailable(DWORD storageID, WHICH_OTG eWhichOtg);
static void PtpGetObjectHeader(DWORD objectHandle, OBJECT_HEADER* header, WHICH_OTG eWhichOtg);
static BOOL PtpCheckValidHandle(DWORD handle, WHICH_OTG eWhichOtg);
static BOOL PtpCheckValidParent(DWORD handle, WHICH_OTG eWhichOtg);
static void PtpGetObjFileName(BYTE** ptr, WHICH_OTG eWhichOtg);
static WORD PtpGetObjectFormat(BYTE* bExt, WHICH_OTG eWhichOtg);
static DWORD PtpAddObjectHandle(DWORD storageID, DWORD parentHandle, WORD handleFormat, WHICH_OTG eWhichOtg);
static void PtpBuildObjectHandles(DWORD storageID, DWORD parentHandle, WHICH_OTG eWhichOtg);
static BOOL PtpGetNumChildren(DWORD parent, DWORD format, WHICH_OTG eWhichOtg);
static void PtpAddWord(BYTE** ptr, WORD val);
static void PtpAddDword(BYTE** ptr, DWORD val);
static void PtpAddString(BYTE** ptr, WORD* str);
static void PtpAddFname(BYTE** ptr, BYTE* fname, BYTE* ext);
static void PtpAddDateTime(BYTE** ptr, WORD date, WORD time);
static BOOL PrepareIFMImageObjectInfo (void);
static BOOL PrepareImageObjectInfo (PUSB_DEVICE_SIDC psUsbDevSidc);
static void PrepareParentObjectInfo (PUSB_DEVICE_SIDC psUsbDevSidc);
static void PrepareDeviceInfo (WHICH_OTG eWhichOtg);
static void FillDeviceInfoData(BYTE* pDeviceInfo, DWORD* pDataSize, WHICH_OTG eWhichOtg);
static void strtrim(char* str, DWORD len);
static TRANSACTION_STATE PtpSendDPSOpReq(BYTE* dbuf, DWORD dcnt, BOOL isEnableEvent, WHICH_OTG eWhichOtg);
static void PtpProcessObjectInfo(PSTI_OBJECT_INFO pobject_info, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static BOOL PtpProcessObject(BYTE** ptr, WHICH_OTG eWhichOtg);
static DWORD OperationCloseSession(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE PtpOpGetPartialObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetDeviceInfo (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationOpenSession (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetNumObjects (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetObjectHandle (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetObjectInfo (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationSendObjectInfo(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationSendObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetThumb(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE OperationGetPartialObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg);
static TRANSACTION_STATE Pima15740CommandProcess (BYTE** hData,
                                            PSTI_CONTAINER pCommand,
                                            PSTI_CONTAINER pResponse,
                                            WHICH_OTG eWhichOtg);
static BOOL PtpPutNextPacket(  BYTE* packet,
                        DWORD numBytes,
                        PSTI_CONTAINER pCommand,
                        PSTI_CONTAINER pResponse,
                        WHICH_OTG eWhichOtg);
static void PtpInit(WHICH_OTG eWhichOtg);
static STREAM* FileOpenForPtp(WORD *pFormat, DWORD ObjectHandle);

/*
// Definition of internal functions
*/

///////////////////////////////////////////////////////////////////////////////
//		eUsbProcessPtp()
//		Description: Process the PTP Command (PIMA15740)
//		input: none
//		output: TRANSACTION_STATE
//      note: have knew the data lenght is 31
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbProcessPtp(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    TRANSACTION_STATE eState = STATE_PTP_RES;
    PSTI_CONTAINER pStiDataBlock;
    DWORD   len = 0;
    WORD    type = 0;
    BOOL    ret = TRUE;
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);

    //ret = vOTGFxRd((BYTE*)(gpStiCommand), u16FIFOByteCount);
    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE*)(psUsbDev->sSidc.psStiCommand),u16FIFOByteCount,eWhichOtg);
    if (ret == FALSE)
        return eState;
    type = BYTE_SWAP_OF_WORD(psUsbDev->sSidc.psStiCommand->containerType);
    if(type != COMMAND_BLOCK)
    {
        MP_ALERT("%s -Not CMD Type- 0x%x", __FUNCTION__, type);
        eState = STATE_PTP_RES;
    }
    else
    {
        eState = Pima15740CommandProcess (	&psUsbDev->sDesc.pbBulkTxRxData,
                                            psUsbDev->sSidc.psStiCommand,
                                            psUsbDev->sSidc.psStiResponse,
                                            eWhichOtg);
        
        if (psUsbDev->sDesc.pbBulkTxRxData != 0)
        {
            psUsbDev->sDesc.pbBulkTxRxData = (BYTE*)((DWORD)psUsbDev->sDesc.pbBulkTxRxData | 0xa0000000);
            pStiDataBlock = (PSTI_CONTAINER) psUsbDev->sDesc.pbBulkTxRxData;
            psUsbDev->sDesc.dwBulkTxRxLength = BYTE_SWAP_OF_DWORD(pStiDataBlock->containerLength);
            //gBulkTxRxLength = BYTE_SWAP_OF_DWORD(pStiDataBlock->containerLength);
            //MP_DEBUG1("len:%d", gBulkTxRxLength);
        }
    }
    return eState;
}

///////////////////////////////////////////////////////////////////////////////
//		eUsbPtpDataOut()
//		Description: Process the data output stage
//		input: none
//		output: TRANSACTION_STATE
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbPtpDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg)
{
    BOOL    ret = TRUE;
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);    
    PUSB_DEVICE_SIDC psDevSidc = &psUsbDev->sSidc;
    //static DWORD total_len = 0;
    
    //ret = vOTGFxRd((BYTE*)((DWORD)gpBulkTxRxData + gBulkTxRxCounter), u16FIFOByteCount);
    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO1,DIRECTION_OUT,(BYTE*)((DWORD)psUsbDev->sDesc.pbBulkTxRxData + psUsbDev->sDesc.dwBulkTxRxCounter),u16FIFOByteCount,eWhichOtg);
    if (ret == FALSE)
        return STATE_PTP_RES;
    if (psUsbDev->sDesc.dwBulkTxRxCounter == 0)
    {
        psDevSidc->dwTotalLen = BYTE_TO_DWORD(	*(psUsbDev->sDesc.pbBulkTxRxData + 3),
                                    *(psUsbDev->sDesc.pbBulkTxRxData + 2),
                                    *(psUsbDev->sDesc.pbBulkTxRxData + 1),
                                    *(psUsbDev->sDesc.pbBulkTxRxData + 0)
                                    );
    }

    psUsbDev->sDesc.dwBulkTxRxCounter += u16FIFOByteCount;
    psDevSidc->dwTotalLen -= u16FIFOByteCount;
    //    fpga_delay();
    if (psDevSidc->dwTotalLen == 0)
    {
        PtpPutNextPacket((BYTE*)(psUsbDev->sDesc.pbBulkTxRxData + USB_STI_CONTAINER_SIZE),
                                 psUsbDev->sDesc.dwBulkTxRxCounter,
                                 psUsbDev->sSidc.psStiCommand,
                                 psUsbDev->sSidc.psStiResponse,
                                 eWhichOtg);
        psUsbDev->sDesc.dwBulkTxRxCounter = 0;
        return STATE_PTP_RES;
    }
    else
        return STATE_PTP_DATA_OUT;
}

///////////////////////////////////////////////////////////////////////////////
//      eUsbPtpDataIn()
//      Description: Process the data intput stage
//      input: none
//      output: TRANSACTION_STATE
///////////////////////////////////////////////////////////////////////////////
//int gTestFlag = 0;
TRANSACTION_STATE eUsbPtpDataIn(WHICH_OTG eWhichOtg)
{
    volatile DWORD u32count = 0;
    TRANSACTION_STATE eState = STATE_PTP_RES;
    DWORD dataSize = 0;
    volatile DWORD bulkTxRxLength = 0;
    DWORD bulkTxRxCounter = 0;
    //static DWORD data_len = 0;
    DWORD cnt = 0;
    BYTE    *pData;
    PST_USB_OTG_DES psUsbOtg    = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);    
    PST_USB_OTG_DEVICE psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);     

    bulkTxRxLength = psUsbDev->sDesc.dwBulkTxRxLength;
    if(bulkTxRxLength != 0)
    {
        pData = (BYTE*)((DWORD)psUsbDev->sDesc.pbBulkTxRxData | 0xa0000000);
        psUsbDev->sSidc.boIsSendingData = TRUE;
        while(bulkTxRxLength > 0) // total size should be less than 256KB
        {
            if (UsbOtgDeviceCheckIdPin(eWhichOtg) == FALSE)
            {
                return STATE_IDLE;
            }
            else
            {
                if(bulkTxRxLength > mUsbOtgEPinMxPtSz(EP1))
                    u32count = mUsbOtgEPinMxPtSz(EP1);
                else
                    u32count = bulkTxRxLength;

                // Send u16FIFOByteCount Bytes data to F0 via DBUS;
                //vMyUsbAPWrToFifo((pData + bulkTxRxCounter), u32count, FIFO_BULK_IN);
                BOOL  ret = TRUE;
                //ret = vOTGFxWr((pData + bulkTxRxCounter), u32count);
                ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN,(pData + bulkTxRxCounter),u32count,eWhichOtg);
                if (ret == FALSE)
                {
                    mpDebugPrint("3Disconnect while reading");
                    break;
                }

                mUsbOtgFIFODone(FIFO_BULK_IN);
                bulkTxRxCounter += u32count;
                bulkTxRxLength -= u32count;
            }

            if ((bulkTxRxCounter % USB_OTG_BUF_SIZE) == 0)
            {
                if (bulkTxRxLength > USB_OTG_BUF_SIZE)
                {
                    dataSize = USB_OTG_BUF_SIZE;
                }
                else
                {
                    dataSize = bulkTxRxLength;
                }
                
                if (psUsbDev->sSidc.boIsFileSending == TRUE)
                {
                    FileRead(psUsbDev->sSidc.psFile, pData, dataSize);
                    psUsbDev->sSidc.dwImageSizeRead += dataSize;
                    if (psUsbDev->sSidc.dwImageSizeRead == psUsbDev->sSidc.sdwImageSize)
                        psUsbDev->sSidc.dwImageSizeRead = 0;
                    else
                        psUsbDev->sSidc.dwImageSizeRead -= dataSize;
                }

                bulkTxRxCounter = 0;                
            }
        } // end of while(bulkTxRxLength > 0)

		MP_DEBUG("bulkTxRxLength While End");
        psUsbDev->sSidc.boIsSendingData = FALSE;
    } // end of if(bulkTxRxLength != 0)

/*
    while((mUsbFIFOEmptyByte0Rd(FIFO_BULK_IN)) == 0 && cnt < 0xffff)
    {
        cnt++;
    };
    
    mUsbFIFODone(FIFO_BULK_IN);
*/    
    if (psUsbDev->sSidc.boIsFileSending == TRUE)
    {
        FileClose(psUsbDev->sSidc.psFile);
        psUsbDev->sSidc.psFile = (STREAM*)NULL;
        psUsbDev->sSidc.boIsFileSending = FALSE;
    }

    return eState;
}

TRANSACTION_STATE eUsbPtpSendResponse(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg        = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);    
    PUSB_DEVICE_SIDC psUsbDevSidc   = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    TRANSACTION_STATE   eState = STATE_PTP_CMD;
    DWORD   response_len = psUsbDevSidc->psStiResponse->containerLength;
    DWORD   dpsState = DPS_IDLE;

    
    memset((BYTE*)psUsbDevSidc->psStiCommand, 0, OPERATION_REQUEST_LENGTH);

    psUsbDevSidc->psStiResponse->containerLength = BYTE_SWAP_OF_DWORD(response_len);
    psUsbDevSidc->psStiResponse->containerType= BYTE_SWAP_OF_WORD(psUsbDevSidc->psStiResponse->containerType);
    psUsbDevSidc->psStiResponse->code = BYTE_SWAP_OF_WORD(psUsbDevSidc->psStiResponse->code);
    if (response_len > USB_STI_CONTAINER_SIZE)
    {
        psUsbDevSidc->psStiResponse->parameter[0] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiResponse->parameter[0]);
        psUsbDevSidc->psStiResponse->parameter[1] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiResponse->parameter[1]);
        psUsbDevSidc->psStiResponse->parameter[2] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiResponse->parameter[2]);
        psUsbDevSidc->psStiResponse->parameter[3] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiResponse->parameter[3]);
        psUsbDevSidc->psStiResponse->parameter[4] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiResponse->parameter[4]);
    }
    BOOL  ret = TRUE;
    //ret = vOTGFxWr((BYTE*)(gpStiResponse),response_len);
    ret = bOTGCxFxWrRd(FOTG200_DMA2FIFO0,DIRECTION_IN,(BYTE*)(psUsbDevSidc->psStiResponse),response_len,eWhichOtg);
    if (ret == FALSE)
    {
        MP_ALERT("--E-- %s Disconnect while reading!", __FUNCTION__);
    }

    mUsbOtgFIFODone(FIFO_BULK_IN);

    dpsState = GetDpsState(eWhichOtg);	
    eState = DpsProcess(dpsState, eWhichOtg);

    return eState;
}

///////////////////////////////////////////////////////////////////////////////
//		eUsbPtpSendEvent()
//		Description: Send out the CSW structure to PC
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
TRANSACTION_STATE eUsbPtpSendEvent(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    DWORD   event_len = psUsbDevSidc->psStiEvent->length;
    TRANSACTION_STATE eState = STATE_PTP_CMD;

    if (psUsbDevSidc->boIsDpsSendEvent == FALSE)
    {
        //return eState;
        return STATE_IDLE;
    }

    psUsbDevSidc->boIsDpsSendEvent = FALSE;
    psUsbDevSidc->psStiEvent->length = BYTE_SWAP_OF_DWORD(event_len);
    psUsbDevSidc->psStiEvent->type = BYTE_SWAP_OF_WORD(psUsbDevSidc->psStiEvent->type);
    psUsbDevSidc->psStiEvent->eventCode = BYTE_SWAP_OF_WORD(psUsbDevSidc->psStiEvent->eventCode);
    if (event_len > USB_STI_CONTAINER_SIZE)
    {
        psUsbDevSidc->psStiEvent->parameter[0] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiEvent->parameter[0]);
        psUsbDevSidc->psStiEvent->parameter[1] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiEvent->parameter[1]);
        psUsbDevSidc->psStiEvent->parameter[2] = BYTE_SWAP_OF_DWORD(psUsbDevSidc->psStiEvent->parameter[2]);
    }


//INTERRUPT   Process
    //vMyUsbAPWrToFifo((BYTE *)(gpStiEvent), event_len, FIFO_INT_IN);
    //vOTGDxFxWr(FOTG200_DMA2FIFO2, (BYTE *) (gpStiEvent), event_len);
    bOTGCxFxWrRd(FOTG200_DMA2FIFO2,DIRECTION_IN,(BYTE *)(psUsbDevSidc->psStiEvent),event_len,eWhichOtg);
    //mUsbFIFODone(FIFO_INT_IN);
    
    //    mUsbFIFODone(FIFO14);
    //	vUsb_F14_In();
    return eState;
}

TRANSACTION_STATE DpsProcessJob(DWORD dpsState, WHICH_OTG eWhichOtg)
{
    DWORD   len = 0;
    BOOL    rt = DPS_FAILURE;
    BOOL    isEnableEvent = TRUE;
    PST_USB_OTG_DEVICE  psUsbDev     = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    PUSB_DEVICE_SIDC    psUsbDevSidc = (PUSB_DEVICE_SIDC)&psUsbDev->sSidc; // For  psUsbDevSidc->sDps.pbXmlBuff as same as other function.  

    if (dpsState & DPS_START_JOB)
    {
        MP_DEBUG("DPS_START_JOB");
        RemoveDpsState(DPS_START_JOB, eWhichOtg);
        isEnableEvent = DpsStartJob(eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        return PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_ABORT_JOB)
    {
        MP_DEBUG("DPS_ABORT_JOB");
        RemoveDpsState(DPS_ABORT_JOB, eWhichOtg);
        isEnableEvent = DpsAbortJob(eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        return PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_CONTINUE_JOB)
    {
        MP_DEBUG("DPS_CONTINUE_JOB");
        RemoveDpsState(DPS_CONTINUE_JOB, eWhichOtg);
        isEnableEvent = DpsContinueJob(eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        return PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else
    {
        return psUsbDev->eUsbTransactionState;
    }
}

void UsbSidcInit(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DEVICE  psUsbDev = (PST_USB_OTG_DEVICE)UsbOtgDevDsGet(eWhichOtg);
    //gpBulkTxRxData = (BYTE*)((DWORD)&gDataBuffer[0] | 0xa0000000);
    psUsbDev->sDesc.pbBulkTxRxData = (BYTE*)((DWORD)UsbOtgBufferGet(eWhichOtg)| 0xa0000000);
    
    psUsbDev->eUsbTransactionState = STATE_PTP_CMD;
    //gpStiCommand = (PSTI_CONTAINER)((DWORD)(&gStiCommand) | 0xa0000000);
    //gpStiResponse = (PSTI_CONTAINER)((DWORD)(&gStiResponse) | 0xa0000000);
    //gpStiEvent = (PSTI_EVENT)((DWORD)(&gStiEvent) | 0xa0000000);
    //gpCancelRequestData = (BYTE*)((DWORD)(&gCancelRequestData[0]) | 0xa0000000);
    //gpGetDeviceStatusRequest = (BYTE*)((DWORD)(&gGetDeviceStatusRequest[0]) | 0xa0000000);
    psUsbDev->sDesc.dwBulkTxRxCounter = 0;
    psUsbDev->sSidc.sdwImageSize = 0;
    psUsbDev->sSidc.dwImageSizeRead = 0;
    psUsbDev->sSidc.boIsFileSending = FALSE;
	psUsbDev->sSidc.boIsSendingData = FALSE;//gIsSendingData = FALSE;
	
    memset((BYTE*)psUsbDev->sSidc.psStiCommand, 0, OPERATION_REQUEST_LENGTH);
    memset((BYTE*)psUsbDev->sSidc.psStiResponse, 0, RESPONSE_LENGTH);
    memset((BYTE*)psUsbDev->sSidc.psStiEvent, 0, EVENT_LENGTH);	
    //memset((BYTE*)psUsbDev->sSidc.pbCancelRequestData, 0, CANCEL_REQUEST_DATA_LENGTH);
    //memset((BYTE*)psUsbDev->sSidc.pbGetDeviceStatusRequest, 0, GET_DEVICE_STATUS_REQUETS_LENGTH);	
    PtpInit(eWhichOtg);
}

void SetDpsState(DWORD dpsState, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    psUsbDevSidc->dwDpsState = dpsState;
}

DWORD GetDpsState(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    return psUsbDevSidc->dwDpsState;
}


void PtpSetOneHandle(BOOL bIsOneHandle, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);  

    psUsbDevSidc->bIsOneHandle = bIsOneHandle;
}

BOOL PtpGetIsOneHandle(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);  

    return psUsbDevSidc->bIsOneHandle;
}

/*
// Definition of local functions 
*/
static DWORD GetPrintedFileType(WHICH_OTG eWhichOtg)
{
    DWORD ret = 0;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
    switch (psUsbDevSidc->wPrintedFileType)
    {
        case OBJ_FORMAT_JPEG_EXIF:
            ret = fileTypeExit_Jpeg;
            break;

        case OBJ_FORMAT_BMP:
            ret = fileTypeBmp;
            break;

        default:
            break;
    }

    return ret;
}

static BOOL CheckIfSendingData(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);     
    return psUsbDevSidc->boIsSendingData;
}

static BOOL IsHostRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);   
    return psUsbDevSidc->boIsHostReq;
}

static void SetHostRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    psUsbDevSidc->boIsHostReq = TRUE;
}

static void ClearHostRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    psUsbDevSidc->boIsHostReq = FALSE;
}

static BOOL IsDeviceRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);      
    return psUsbDevSidc->boIsDeviceReq;
}

static void SetDeviceRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);      
    psUsbDevSidc->boIsDeviceReq = TRUE;
}

static void ClearDeviceRequest(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc;
    
    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    psUsbDevSidc->boIsDeviceReq = FALSE;
}

static void RemoveDpsState(DWORD dpsState, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    psUsbDevSidc->dwDpsState = 0;
    //gDpsState &= ~(dpsState);
}

static void SetPrinterCapability(PRINTER_CAPABILITY *pstPrinterCap, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
	psUsbDevSidc->psPrinterCap = pstPrinterCap;
}

static PRINTER_CAPABILITY *GetPrinterCapability(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
	return psUsbDevSidc->psPrinterCap;
}

static void SetDpsStateAndCapa(DWORD dpsState, PRINTER_CAPABILITY *pstPrinterCap, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    
    SetDpsState(dpsState, eWhichOtg);
	SetPrinterCapability(pstPrinterCap, eWhichOtg);
}

static TRANSACTION_STATE DpsProcess (DWORD dpsState, WHICH_OTG eWhichOtg)
{
    TRANSACTION_STATE eState = STATE_PTP_CMD;
    DWORD   len = 0;
    BOOL    isEnableEvent = TRUE;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);

    if (psUsbDevSidc->boDpsEnabled == FALSE)
        return eState;

    if (dpsState & DPS_CONFIGURE_PRINT_SERVICE)
    {
        MP_DEBUG("DPS_CONFIGURE_PRINT_SERVICE");
        RemoveDpsState(DPS_CONFIGURE_PRINT_SERVICE, eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_GET_CAPABILITY)
    {
        //    MP_DEBUG("DPS_GET_CAPABILITY");
        RemoveDpsState(DPS_GET_CAPABILITY, eWhichOtg);
        DpsGetCapability(eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_GET_DEVICE_STATUS)
    {
        MP_DEBUG("DPS_GET_DEVICE_STATUS");
        RemoveDpsState(DPS_GET_DEVICE_STATUS, eWhichOtg);
        DpsGetDeviceStatus(eWhichOtg);
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_NOTIFY_DEVICE_STATUS)
    {
        MP_DEBUG("DPS_NOTIFY_DEVICE_STATUS");
        RemoveDpsState(DPS_NOTIFY_DEVICE_STATUS, eWhichOtg);

        //    if(IsStartJob())
        //    {
        //        SetDpsState(DPS_START_JOB);
        //    }

        PtpSendDPSOpReq(0, 0, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_NOTIFY_JOB_STATUS)
    {
        MP_DEBUG("DPS_NOTIFY_JOB_STATUS");
        RemoveDpsState(DPS_NOTIFY_JOB_STATUS, eWhichOtg);
        PtpSendDPSOpReq(0, 0, isEnableEvent, eWhichOtg);
    }
    else if (dpsState & DPS_NOT_RECOGNIZED_OPERATION)
    {
        MP_DEBUG("DPS_NOT_RECOGNIZED_OPERATION");
        RemoveDpsState(DPS_NOT_RECOGNIZED_OPERATION, eWhichOtg);
        //isEnableEvent = FALSE;
        PtpSendDPSOpReq(0, 0, isEnableEvent, eWhichOtg);
    }
/*
    else if (dpsState & DPS_START_JOB)
    {
        MP_DEBUG("xx_DPS_START_JOB_xx");
        RemoveDpsState(DPS_START_JOB);
        DpsStartJob();
        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
        eState = PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len);
    }
*/
    //    if(IsStartJob()&&isDone==FALSE)
    //    {
    //        SetDpsState(DPS_START_JOB);
    //        isDone = TRUE;

    //        RemoveDpsState(DPS_START_JOB);
    //        DpsStartJob();
    //        len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
    //        PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len);
    //    }

    return eState;
}

static BOOL IsPrinterConnected(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
    return psUsbDevSidc->boDpsEnabled;
}
/*
void vSidcCancelRequest(WORD *pCnt, BYTE **handle)
{
    *pCnt = CANCEL_REQUEST_DATA_LENGTH;
    *handle = &gCancelRequestData[0];

}

void vSidcGetDeviceStatus(WORD *pCnt, BYTE **handle)
{
    gGetDeviceStatusRequest[0] = LO_BYTE_OF_WORD(GET_DEVICE_STATUS_REQUETS_LENGTH);
    gGetDeviceStatusRequest[1] = HI_BYTE_OF_WORD(GET_DEVICE_STATUS_REQUETS_LENGTH);
    gGetDeviceStatusRequest[2] = LO_BYTE_OF_WORD(RESPONSE_OK);
    gGetDeviceStatusRequest[3] = HI_BYTE_OF_WORD(RESPONSE_OK);
    *pCnt = GET_DEVICE_STATUS_REQUETS_LENGTH;
    *handle = &gGetDeviceStatusRequest[0];
}
*/
/*
// Definition of local functions 
*/


/*
TRANSACTION_STATE eUsbPtpDataIn(void)
{
    WORD u32count;

    if(gBulkTxRxLength > mUsbEPinMxPtSz(EP1))
        u32count = mUsbEPinMxPtSz(EP1);
    else
        u32count = gBulkTxRxLength;
    
    // Send u16FIFOByteCount Bytes data to F0 via DBUS;
    if (gpBulkTxRxData != 0)
    {
        gpBulkTxRxData = (BYTE*)((DWORD)gpBulkTxRxData | 0xa0000000);
        vMyUsbAPWrToFifo((BYTE*)((DWORD)gpBulkTxRxData + gBulkTxRxCounter), u32count, FIFO_BULK_IN);
        gBulkTxRxCounter += u32count;
    }
    //MP_DEBUG1("len:%d", gBulkTxRxLength);
    gBulkTxRxLength -= u32count;
    if (gBulkTxRxLength == 0)
    {
        //MP_DEBUG("_E_");
        gBulkTxRxCounter = 0;
        mUsbFIFODone(FIFO_BULK_IN);
        return STATE_PTP_RES;
    }
    else
    {
        //MP_DEBUG("_C_");
        return STATE_PTP_DATA_IN;
    }
}
*/



//======================================
//string_len
//
//Returns the size of a 0-terminated
//unicode string
//======================================
static DWORD string_len(WORD* str)
{
    DWORD size=0;
    while (*str++ != 0x0000) size++;
    return size;
}



//======================================
//PtpCheckStorageID
//
//Verify a StorageID parameter
//======================================
static BOOL PtpCheckStorageID(DWORD storageID, WHICH_OTG eWhichOtg)
{
    int     i;
    BOOL    found = FALSE;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    for (i = 0; i < (sizeof(psUsbDevSidc->dwPtpStorageIDs) >> 2); i++)
    {
        if (psUsbDevSidc->dwPtpStorageIDs == storageID)
        {
            found = TRUE;
            break;
        }
    }

    return found;
}


//======================================
//PtpCheckStoreAvailable
//
//Check to see if the specified
//store is available
//======================================
static BOOL PtpCheckStoreAvailable(DWORD storageID, WHICH_OTG eWhichOtg)
{
    int     i;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    // we will have some storages
    for (i = 0; i < (sizeof(psUsbDevSidc->dwPtpStorageIDs) >> 2); i++)
    {
        if (storageID != psUsbDevSidc->dwPtpStorageIDs)
        {            
            MP_ALERT("%s -Invalid Store ID- 0x%x", __FUNCTION__, storageID);
            return FALSE;
        }
    }

    return TRUE;
}




//======================================
//PtpGetObjectHeader
//
//Retrieve ObjectHeader Info Based
// on the Specified ObjectHandle
//======================================
static void PtpGetObjectHeader(DWORD objectHandle, OBJECT_HEADER* header, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
    header->physicalStore = psUsbDevSidc->psPtpHandles[objectHandle-1].physicalStore;
    header->logicalStore  = psUsbDevSidc->psPtpHandles[objectHandle-1].logicalStore;
    header->objectFormat  = psUsbDevSidc->psPtpHandles[objectHandle-1].objectFormat;
    header->parentHandle  = psUsbDevSidc->psPtpHandles[objectHandle-1].parentHandle;
}
 

//======================================
// PtpCheckValidHandle
//
//Verify an ObjectHandle parameter
//======================================
static BOOL PtpCheckValidHandle(DWORD handle, WHICH_OTG eWhichOtg)
{
    BOOL rt = FALSE;
    OBJECT_HEADER header;
    WORD    objectFormat;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    // The ROOT is always a valid handle
    if ((handle == 0) || (handle == 0xFFFFFFFF))
    {
        rt = TRUE;
    }

    // DPS virtual file handles are always valid
    else if ((handle == psUsbDevSidc->dwDpsDDiscHdl) ||
            (handle == psUsbDevSidc->dwDpsDReqHdl)  ||
            (handle == psUsbDevSidc->dwDpsDRespHdl) ||
            (handle == psUsbDevSidc->dwDpsHDiscHdl) ||
            (handle == psUsbDevSidc->dwDpsHReqHdl)  ||
            (handle == psUsbDevSidc->dwDpsHRespHdl))
    {
        rt = TRUE;
    }
    else if (handle <= psUsbDevSidc->dwPtpNumHandles)
    {    
        PtpGetObjectHeader(handle, &header, eWhichOtg);
        //Check Object Format 
        objectFormat=header.objectFormat;
        if (objectFormat == OBJ_FORMAT_JPEG_EXIF)
        {
            rt = TRUE;
        }
        else
        {
            MP_ALERT("--E-- %s Not JPEG! Handle:%d", __FUNCTION__, handle);
        }
    }

    return rt;
}




//======================================
//sf _PtpCheckValidParent
//s
//s Verify a parent object
//======================================
static BOOL PtpCheckValidParent(DWORD handle, WHICH_OTG eWhichOtg)
{
    BOOL    rt;
    OBJECT_HEADER header;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    // The ROOT is always a valid parent
    if ((handle == 0) || (handle == 0xFFFFFFFF))
    {
        rt = TRUE;
    }
    else
    {
        PtpGetObjectHeader(handle, &header, eWhichOtg);
        if (header.objectFormat == OBJ_FORMAT_ASSOCIATION)
        {
        rt = TRUE;
        }
        else
        {
        rt = FALSE;
            MP_ALERT("--E-- %s Not Folder! Handle:%d", __FUNCTION__, handle);
        }
    }

    return rt;
}

//======================================
// PtpGetObjFileName
//
// Extract an "8.3" Filename From
// a Unicode String in an ObjectInfo
// Dataset Sent From the Host
//======================================
static void PtpGetObjFileName(BYTE** ptr, WHICH_OTG eWhichOtg)
{
    DWORD i;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    // retrieve the "8.3" file name
    for (i = 0; i < 12; i++){
        psUsbDevSidc->bStrFile[i] = (*ptr)[(i*2)];
        //	MP_DEBUG1("strFile  %c", strFile[i] );
    }
    strtrim(psUsbDevSidc->bStrFile, 12);

}



//======================================
// PtpGetObjectFormat
//
// Return the object "format" based
// on the file type
//======================================
static WORD PtpGetObjectFormat(BYTE* bExt, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);  
    WORD  format;
    BYTE    bCnt;

    if(psUsbDevSidc->bIsOneHandle)
        return OBJ_FORMAT_JPEG_EXIF;

    for(bCnt = 0; bCnt < 4; bCnt++) // Change lower to upper
        if((*(bExt+bCnt) >= 0x61) && (*(bExt+bCnt) <= 0x7a))
            *(bExt+bCnt) -= 0x20;

    if (strncmp(bExt, "AVI", 3) == 0)
    	format = OBJ_FORMAT_AVI;
    else if (strncmp(bExt, "BMP", 3) == 0) // V
    	format = OBJ_FORMAT_BMP;
    else if (strncmp(bExt, "EXE", 3) == 0)
    	format = OBJ_FORMAT_EXECUTABLE;
    else if (strncmp(bExt, "GIF", 3) == 0) // V
    	format = OBJ_FORMAT_GIF;
    else if (strncmp(bExt, "HTM", 3) == 0)
    	format = OBJ_FORMAT_HTML;
    else if (strncmp(bExt, "JPG", 3) == 0)   // V JPG JPEG???
    	format = OBJ_FORMAT_JPEG_EXIF;
    else if (strncmp(bExt, "MPG", 3) == 0)
    	format = OBJ_FORMAT_MPEG;
    else if (strncmp(bExt, "MRK", 3) == 0)
    	format = OBJ_FORMAT_DPOF;
    else if (strncmp(bExt, "PNG", 3) == 0) // V
    	format = OBJ_FORMAT_PNG;
    else if (strncmp(bExt, "TXT", 3) == 0)
    	format = OBJ_FORMAT_TEXT;
    else if (strncmp(bExt, "TIF", 3) == 0) // V TIF TIFF???
    	format = OBJ_FORMAT_TIFF;
    else if (strncmp(bExt, "WAV", 3) == 0)
    	format = OBJ_FORMAT_WAV;
    else if (strncmp(bExt, "XML", 3) == 0)
    	format = OBJ_FORMAT_SCRIPT;
    else if (strncmp(bExt, "DPS", 3) == 0)
        format = OBJ_FORMAT_SCRIPT;
    else
    {
        format = OBJ_FORMAT_UNDEFINED;
        MP_ALERT("--E-- %s add unknow format! extName:%s", __FUNCTION__, bExt);
    }
    
    return format;
}


//======================================
// PtpAddObjectHandle
//
//Add an Object Handle to the List
//======================================
static DWORD PtpAddObjectHandle(DWORD storageID, DWORD parentHandle, WORD handleFormat, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);

    //-------------------------------------------------------------------------
    // Derive an Object Header record from the file entry's attributes and
    //  extension, store it at the end of the Object Handle list, update
    //  the number of handles
    //-------------------------------------------------------------------------
    if (psUsbDevSidc->dwPtpNumHandles < MAX_DEV_NUM_OF_OBJECTS)
    {    
        //storageID = 00010001
        psUsbDevSidc->psPtpHandles[psUsbDevSidc->dwPtpNumHandles].physicalStore = (storageID >> 16) & 0xFF;
        psUsbDevSidc->psPtpHandles[psUsbDevSidc->dwPtpNumHandles].logicalStore  = (storageID >>  0) & 0xFF;
        psUsbDevSidc->psPtpHandles[psUsbDevSidc->dwPtpNumHandles].objectFormat  = handleFormat;
        psUsbDevSidc->psPtpHandles[psUsbDevSidc->dwPtpNumHandles].parentHandle  = parentHandle;
    }
    else
    {
        MP_ALERT("%s -Photo over %d- %d", __FUNCTION__, MAX_DEV_NUM_OF_PHOTOS, psUsbDevSidc->dwPtpNumHandles+1);
    }

    psUsbDevSidc->dwPtpNumHandles++;

    return  psUsbDevSidc->dwPtpNumHandles;
} 

void PtpHandlesAlloc(DWORD dwCnt, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);      
    DWORD   dwObjHdl = 0;
    
    PtpHandlesFree(eWhichOtg);

    if(dwCnt == 0) return;

    dwObjHdl = (dwCnt > MAX_DEV_NUM_OF_PHOTOS)?MAX_DEV_NUM_OF_OBJECTS:dwCnt+MAX_DEV_NUM_OF_DPS;
    MP_DEBUG("-USBOTG%d- %s Ptp Handles:%d", eWhichOtg, __FUNCTION__, dwObjHdl);  

    psUsbDevSidc->psPtpHandles = (POBJECT_HEADER) ker_mem_malloc(dwObjHdl * sizeof(OBJECT_HEADER), UsbOtgDeviceTaskIdGet(eWhichOtg));
    if (psUsbDevSidc->psPtpHandles == 0)
        MP_ALERT("--E-- USBOTG%d %s PtpHandle is not enough memory!", eWhichOtg, __FUNCTION__);    
    else
        MP_DEBUG("-USBOTG%d- PtpHandle alloc 0x%x", eWhichOtg, psUsbDevSidc->psPtpHandles);    
}

void PtpHandlesFree(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);  
    
    if (psUsbDevSidc->psPtpHandles != 0) 
    {
        //MP_DEBUG("-USBOTG%d- %s DPS ObjHdl not Free", eWhichOtg, __FUNCTION__);
        ker_mem_free (psUsbDevSidc->psPtpHandles);    
        psUsbDevSidc->psPtpHandles = 0;
    }
}

static void DumpFileInfo(DWORD dwIndex)
{
    unsigned char Name[512] = {0}, Ext[4] = {0};
    unsigned char u8Name[512] = {0};
    unsigned char ShortName[9] = {0};    
    ST_SEARCH_INFO *pST_SEARCH_INFO = (ST_SEARCH_INFO *)FileGetSearchInfo(dwIndex);

    if(FileBrowserGetFileName(pST_SEARCH_INFO, Name, 512, Ext) == FAIL)
    {
        MP_ALERT("--E-- %s Get File Name Fail !", __FUNCTION__);
        return;
    }

    strncpy(ShortName, pST_SEARCH_INFO->bName, 8);
    mpx_UtilU16ToU08((U08 *) u8Name, (U16 *) Name);

    // 8.3-Format  pST_SEARCH_INFO->dwLongFdbCount = 0
    MP_ALERT("1. 8.3 File Name = %s", ShortName); 
    MP_ALERT("2. 8.3 ExtName = %s", pST_SEARCH_INFO->bExt);
    MP_ALERT("3. 8.3 FileDate = %.4d/%.2d/%.2d", pST_SEARCH_INFO->DateTime.year, pST_SEARCH_INFO->DateTime.month, pST_SEARCH_INFO->DateTime.day);
    // Long-Format  pST_SEARCH_INFO->dwLongFdbCount != 0    
    MP_ALERT("4. Long FileName = %s", u8Name);
    MP_ALERT("5. Long extName = %s", Ext);    

}

static void PtpBuildObjectHandles(DWORD storageID, DWORD parentHandle, WHICH_OTG eWhichOtg)
{
    DWORD   dwCnt;
    DWORD 	fileHdl, dwObjHdl = 0;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);  
    DWORD   dwTotalPhoto =  g_psSystemConfig->sFileBrowser.dwFileListCount[OP_IMAGE_MODE]; // g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile; // This value will change by Current-Mode.
    ST_SEARCH_INFO *pST_SEARCH_INFO;
        
    psUsbDevSidc->dwPtpNumHandles = 0;

    // Check Operation Mode --- IMAGE_MODE
    if(g_psSystemConfig->dwCurrentOpMode != OP_IMAGE_MODE)
    {
        // PictBridge only support IMAGE-MODE, User can't enter other Mode.
        MP_ALERT("--E-- %s Not Image Mode!  CurrentMode:%d", __FUNCTION__, g_psSystemConfig->dwCurrentOpMode);
        MP_ALERT("--E-- PictBridge only support IMAGE-MODE!");

        g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
        FileBrowserResetFileList();
        FileBrowserScanFileList(SEARCH_TYPE);
        dwTotalPhoto =  g_psSystemConfig->sFileBrowser.dwFileListCount[OP_IMAGE_MODE]; 
    }

    // Dump File Information
    //for(dwCnt=0; dwCnt<dwTotalPhoto; dwCnt++)   DumpFileInfo(dwCnt);

    MP_DEBUG("-USBOTG%d- %s Photos:%d", eWhichOtg, __FUNCTION__, dwTotalPhoto);
    if(dwTotalPhoto > MAX_DEV_NUM_OF_PHOTOS && PtpGetIsOneHandle(eWhichOtg) == FALSE)
    {
        MP_ALERT("--W-- %s -Photo over Max:%d- Photo %d ~ %d will skip", __FUNCTION__, MAX_DEV_NUM_OF_PHOTOS, MAX_DEV_NUM_OF_PHOTOS+1, dwTotalPhoto);
        dwTotalPhoto = MAX_DEV_NUM_OF_PHOTOS;
    }

    if(PtpGetIsOneHandle(eWhichOtg))  // One Handle
        dwTotalPhoto = 1;
     
    // Alloc PtpHandles Memory by Photo Count
    PtpHandlesAlloc(dwTotalPhoto, eWhichOtg);

    // Alloc PrintInfo Memory by Photo Count
    DpsPrintInfoAlloc(dwTotalPhoto, eWhichOtg);


    // add handles for the Photo
    for(dwCnt = 0; dwCnt < dwTotalPhoto; dwCnt++)
    {
        pST_SEARCH_INFO = (ST_SEARCH_INFO *)FileGetSearchInfo(dwCnt);
        fileHdl = PtpAddObjectHandle(storageID, 0, PtpGetObjectFormat(pST_SEARCH_INFO->bExt, eWhichOtg), eWhichOtg);	// 3
    }

    // add handles for the DPS virtual files
    psUsbDevSidc->dwDpsDDiscHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 2
    psUsbDevSidc->dwDpsDReqHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 3
    psUsbDevSidc->dwDpsDRespHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 4
    psUsbDevSidc->dwDpsHDiscHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 5
    psUsbDevSidc->dwDpsHReqHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 6
    psUsbDevSidc->dwDpsHRespHdl = PtpAddObjectHandle(storageID, 0, OBJ_FORMAT_SCRIPT, eWhichOtg);	// 7
}


//======================================
//PtpGetNumChildren
//
//Return the number of "child" objects
//associated with the specified "parent"
//& object format
//======================================
static BOOL PtpGetNumChildren(DWORD parent, DWORD format, WHICH_OTG eWhichOtg)
{
    int     i;
    DWORD  numObjects;
    BOOL    is_parent_valid;
    BOOL    is_format_valid;
    OBJECT_HEADER header;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    numObjects = 0;
    for (i = 1; i <= psUsbDevSidc->dwPtpNumHandles; i++)
    {
        // get the object info
        PtpGetObjectHeader(i, &header, eWhichOtg);
        is_parent_valid = FALSE;
        is_format_valid = FALSE;

        // parent object match?
        //  (0 = entire device, 0xFFFFFFFF = ROOT only)
        if ((parent == 0) ||
            (parent == header.parentHandle) ||
            ((parent == 0xFFFFFFFF) && (header.parentHandle == 0)))
        {
            is_parent_valid = TRUE;
        }
        else
        {
            //MP_DEBUG("%s -Parent Object not Match", __FUNCTION__);
        }

        // format match?
        //  (0 = all formats, 0xFFFFFFFF = image formats only)
        if ((format == 0)                ||
            (format == header.objectFormat) ||
            ((format == 0xFFFFFFFF)         &&
             ((header.objectFormat == OBJ_FORMAT_JPEG_EXIF) ||
              (header.objectFormat == OBJ_FORMAT_BMP))))
        {
            is_format_valid = TRUE;
        }
        else
        {
            //MP_DEBUG("%s -Format not Match2", __FUNCTION__);
        }

        if (is_parent_valid && is_format_valid)
        {
            numObjects++;    
        }
    }

    return numObjects;
}


//======================================
//PtpAddWord
//
// Appends a 16-bit value to
// to the specified buffer
//======================================
static void PtpAddWord(BYTE** ptr, WORD val)
{
    (*ptr)[0] = LO_BYTE_OF_WORD(val);
    (*ptr)[1] = HI_BYTE_OF_WORD(val);
    *ptr = (BYTE*)((int)*ptr + 2);
}



//======================================
//PtpAddWord
//
//Appends a 32-bit value to
//to the specified buffer
//======================================
static void PtpAddDword(BYTE** ptr, DWORD val)
{
    (*ptr)[0] = LO_BYTE_OF_DWORD(val);
    (*ptr)[1] = MIDLO_BYTE_OF_DWORD(val);
    (*ptr)[2] = MIDHI_BYTE_OF_DWORD(val);
    (*ptr)[3] = HI_BYTE_OF_DWORD(val);
    *ptr = (BYTE*)((int)*ptr + 4);
}

#if 0
//======================================
//PtpAddShortArray
//
//Append an array of 16-bit values
//to the specified buffer
//======================================
void PtpAddShortArray(BYTE** ptr, WORD* array, DWORD numElements)
{
    int i;
 
    (*ptr)[0] = LO_BYTE_OF_DWORD(numElements);
    (*ptr)[1] = MIDLO_BYTE_OF_DWORD(numElements);
    (*ptr)[2] = MIDHI_BYTE_OF_DWORD(numElements);
    (*ptr)[3] = HI_BYTE_OF_DWORD(numElements);
    *ptr = (BYTE*)((int)*ptr + 4);
    for (i=0; i < numElements; i++)
    {
        (*ptr)[i*2+0] = LO_BYTE_OF_WORD(array[i]);
        (*ptr)[i*2+1] = HI_BYTE_OF_WORD(array[i]);
    }
    *ptr = (BYTE*)((int)*ptr + 2*numElements);
}
#endif

//======================================
//PtpAddString
//
//Appends a Unicode string
//to the specified buffer
//======================================
static void PtpAddString(BYTE** ptr, WORD* str)
{
    WORD i,len;

    if (str == 0)
    {
        len = 0;
    }
    else
    {
        len = string_len(str)+1;
    }

    (*ptr)[0] = LO_BYTE_OF_WORD(len);
    for (i = 0; i < len; i++)
    {
        (*ptr)[i*2+1] = LO_BYTE_OF_WORD(str[i]);
        (*ptr)[i*2+2] = HI_BYTE_OF_WORD(str[i]);
    }
    
    *ptr = (BYTE*)((int)*ptr + 1 + 2*len);
}


//======================================
//PtpAddFname
//
//Appends a file name to the
//specified buffer
//======================================
static void PtpAddFname(BYTE** ptr, BYTE* fname, BYTE* ext)
{
    DWORD i, j;

    for (i = 0; i < 8; i++)
    {
        if (fname[i] == ' ')
        {
            break;
        }
        (*ptr)[i*2+1] = fname[i];
        (*ptr)[i*2+2] = 0x00;
    }
    
    if (ext[0] != ' ')
    {
        (*ptr)[i*2+1] = '.';
        (*ptr)[i*2+2] = 0x00;
        i++;

        for (j = 0; j < 3; j++, i++)
        {
            if (ext[j] == 0)
            {
                break;
            }
            (*ptr)[i*2+1] = ext[j];
            (*ptr)[i*2+2] = 0x00;
        }
    }
    
    (*ptr)[i*2+1] = 0x00;
    (*ptr)[i*2+2] = 0x00;
    i++;

    (*ptr)[0] = LO_BYTE_OF_DWORD(i);
    *ptr = (BYTE*)((int)*ptr + 1 + (2 * i));
}


  
//======================================
//sf PtpAddDateTime
//s
//s Appends a
//======================================
static void PtpAddDateTime(BYTE** ptr, WORD date, WORD time)
{
    int mo,d,y,h,m,s;

    mo =  ((date >> 5)  & 0x0F);
    d  =  ((date >> 0)  & 0x1F);
    y  = (((date >> 9)  & 0x7F) + 1980);
    h  =  ((time >> 11) & 0x1F);
    m  =  ((time >> 5)  & 0x3F);
    s  =  ((time        & 0x1F) * 2);

    (*ptr)[32] = 0x00;
    (*ptr)[31] = 0x00;
    (*ptr)[30] = 0x00;
    (*ptr)[29] = '0' + (s % 10);    s /= 10;
    (*ptr)[28] = 0x00;
    (*ptr)[27] = '0' + (s % 10);
    (*ptr)[26] = 0x00;
    (*ptr)[25] = '0' + (m % 10);    m /= 10;
    (*ptr)[24] = 0x00;
    (*ptr)[23] = '0' + (m % 10);
    (*ptr)[22] = 0x00;
    (*ptr)[21] = '0' + (h % 10);    h /= 10;
    (*ptr)[20] = 0x00;
    (*ptr)[19] = '0' + (h % 10);
    (*ptr)[18] = 0x00;
    (*ptr)[17] = 'T';
    (*ptr)[16] = 0x00;
    (*ptr)[15] = '0' + (d % 10);    d /= 10;
    (*ptr)[14] = 0x00;
    (*ptr)[13] = '0' + (d % 10);
    (*ptr)[12] = 0x00;
    (*ptr)[11] = '0' + (mo % 10);   mo /= 10;
    (*ptr)[10] = 0x00;
    (*ptr)[9]  = '0' + (mo % 10);
    (*ptr)[8]  = 0x00;
    (*ptr)[7]  = '0' + (y % 10);    y /= 10;
    (*ptr)[6]  = 0x00;
    (*ptr)[5]  = '0' + (y % 10);    y /= 10;
    (*ptr)[4]  = 0x00;
    (*ptr)[3]  = '0' + (y % 10);    y /= 10;
    (*ptr)[2]  = 0x00;
    (*ptr)[1]  = '0' + (y % 10);
    (*ptr)[0]  = 16;
    *ptr = (BYTE*)((int)*ptr + 33);
}


static BOOL PrepareIFMImageObjectInfo (void)
{
	WORD   numOfPicture = 0;
	WORD   i;
	BOOL			err = TRUE;
 	POBJECT_FILE_INFO	pImgFileInfo = 0;
 	
	numOfPicture = 1;//GetTotalPictureCount();
	pImgFileInfo = 0;//gImageFileInfoPtr;
	for (i = 0; i < numOfPicture; i++ ) {
		pImgFileInfo->objectHandle = i+1;	// cannot equal to be zero
		pImgFileInfo->dirIndex = 0;
		pImgFileInfo->fileIndex = i;
		pImgFileInfo++;
	}

	return err;
}


static BOOL PrepareImageObjectInfo (PUSB_DEVICE_SIDC psUsbDevSidc)
{
	BOOL	err;
	
	psUsbDevSidc->boIsImgObjectHandleReady = TRUE;
	err = PrepareIFMImageObjectInfo();

	return err;
}


static void PrepareParentObjectInfo (PUSB_DEVICE_SIDC psUsbDevSidc)
{
	WORD    i;

	psUsbDevSidc->sStiParentObjectInfo.storageID        = psUsbDevSidc->dwStorageID;
	psUsbDevSidc->sStiParentObjectInfo.objectFormatCode = OBJ_FORMAT_ASSOCIATION;
	psUsbDevSidc->sStiParentObjectInfo.protectionStatus = kNo_Protection;
	psUsbDevSidc->sStiParentObjectInfo.objectCompSize   = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbFormat      = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbCompSize    = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbPixWidth    = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbPixHeight   = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbPixWidth    = 0;
	psUsbDevSidc->sStiParentObjectInfo.thumbPixHeight   = 0;
	psUsbDevSidc->sStiParentObjectInfo.imageBitDepth    = 0;
	psUsbDevSidc->sStiParentObjectInfo.parentObject     = 0;
	psUsbDevSidc->sStiParentObjectInfo.associationType  = GENERIC_FOLDER;
	psUsbDevSidc->sStiParentObjectInfo.associationDesc  = 0;
	psUsbDevSidc->sStiParentObjectInfo.seqNumber        = 0;
	
	psUsbDevSidc->sStiParentObjectInfo.filename.numChars = psUsbDevSidc->sParentFileName.numChars;
	for (i = 0; i < psUsbDevSidc->sParentFileName.numChars; i++)
		psUsbDevSidc->sStiParentObjectInfo.filename.stringChars[i] = psUsbDevSidc->sParentFileName.stringChars[i];
		
	psUsbDevSidc->sStiParentObjectInfo.captureDate.numChars = 0;	
	psUsbDevSidc->sStiParentObjectInfo.modificationDate.numChars = 0;	
	psUsbDevSidc->sStiParentObjectInfo.keyWords.numChars = 0;

	psUsbDevSidc->dwRootObjectHandle = (DWORD)&psUsbDevSidc->sStiParentObjectInfo;
	
//	gRootObjectInfoArray.numElements = kNumOfRoot;
//	gRootObjectInfoArray.pObjectInfo = (pUINT32) &gStiParentObjectInfo; // remeber to dispose
}

/*
BOOL PrepareDirObjectInfo (void)
{
	WORD    numOfDir = 0;
	WORD    i;//, j;
	WORD				firstDirIdx = GetFirstDirIndex();
	WORD				dirIndex = firstDirIdx;
//	pUINT32				pObjectInfo = 0;

 	POBJECT_FILE_INFO		pDirInfo = gDirInfoPtr;

	numOfDir = (WORD) GetDCFDirCount();	// directory object setting
	gIsDirObjectHandleReady = TRUE;
	for (i = 0, dirIndex = firstDirIdx; i < numOfDir; i++) {
		pDirInfo->objectHandle = (DWORD) ((DWORD) i | DIR_PATERN);	// kDirPatern = 0xff00
		pDirInfo->dirIndex = i;
		pDirInfo++;
		dirIndex = DT[dirIndex].Next;
		showmeplus("PrepareDirObjectInfo:pDirInfo->objectHandle:\0", (DWORD) pDirInfo->objectHandle);
	}
	
	return TRUE;
}
*/

static void PrepareDeviceInfo (WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    PSTI_DEVICE_INFO    pDeviceInfo;
	WORD                i = 0;

    pDeviceInfo = &psUsbDevSidc->sStiDeviceInfo;

	if (psUsbDevSidc->dwRootObjectHandle == 0)
		PrepareParentObjectInfo(psUsbDevSidc);

//	if (gIsDirObjectHandleReady == FALSE)
	//	PrepareDirObjectInfo();

	if (psUsbDevSidc->boIsImgObjectHandleReady== 0)//gIsImgObjectHandleReady == FALSE)
		PrepareImageObjectInfo(psUsbDevSidc);

	
	pDeviceInfo->standardVersion    = STANDARD_VERSION;
	pDeviceInfo->vendorExtensionID  = VENDOR_EXTENSION_ID;
	pDeviceInfo->vendorExtensionVer = VENDOR_EXTENSION_VERSION;
    
	pDeviceInfo->vendorExtensionDesc.numChars = VENDOR_EXTENSION_DESC_LENGTH;
//	pDeviceInfo->vendorExtensionDesc.numChars = VENDOR_EXTENSION_DESC_LENGTH;
//	for (i = 0; i < VENDOR_EXTENSION_DESC_LENGTH; i++)
//		pDeviceInfo->vendorExtensionDesc.stringChars[i] = gVendorExtensionDesc[i];

	// Function Mode
	pDeviceInfo->functionMode = FUNCTIONAL_MODE;

	// Operations
	pDeviceInfo->operations.numElements = NUM_OPERATION_SUPPORTED;
	for (i = 0; i < NUM_OPERATION_SUPPORTED; i++)
		pDeviceInfo->operations.arrayEntry[i] = gOperationsSupported[i];

	// Events
	pDeviceInfo->events.numElements = NUM_EVENT_SUPPORTED;
	for (i = 0; i < NUM_EVENT_SUPPORTED; i++)
		pDeviceInfo->events.arrayEntry[i] = gEventsSupported[i];

	// Device Properties
	pDeviceInfo->deviceProperties.numElements = NUM_DEVICE_PROP_CODE_SUPPORTED;
	for (i = 0; i < NUM_DEVICE_PROP_CODE_SUPPORTED; i++)
		pDeviceInfo->deviceProperties.arrayEntry[i] = gDevicePropertiesSupported[i];

	// Capture Formats
	pDeviceInfo->captureFormats.numElements = NUM_CAPTURE_FORMAT_SUPPORTED;
	for (i = 0; i < NUM_CAPTURE_FORMAT_SUPPORTED; i++)
		pDeviceInfo->captureFormats.arrayEntry[i] = gCaptureFormats[i];

	// Image Formats
	pDeviceInfo->imageFormats.numElements = NUM_IMAGE_FORMAT_SUPPORTED;
	for (i = 0; i < NUM_IMAGE_FORMAT_SUPPORTED; i++)
		pDeviceInfo->imageFormats.arrayEntry[i] = gImageFormats[i];

	// manufacture
	pDeviceInfo->manufacture.numChars = MANUFACTURER_STRING_LENGTH;
	for (i = 0; i < MANUFACTURER_STRING_LENGTH; i++)
		pDeviceInfo->manufacture.stringChars[i] = gStiManufacturerString[i];

	// Model
	pDeviceInfo->model.numChars = MODEL_STRING_LENGTH;
	for (i = 0; i < MODEL_STRING_LENGTH; i++)
		pDeviceInfo->model.stringChars[i] = gModelString[i];

	// DeviceVersion
	pDeviceInfo->deviceVersion.numChars = DEVICE_VERSION_STRING_LENGTH;
	for (i = 0; i < DEVICE_VERSION_STRING_LENGTH; i++)
		pDeviceInfo->deviceVersion.stringChars[i] = gDeviceVersionString[i];

	// Serial Number
	pDeviceInfo->serialNumber.numChars = SERIAL_NUMBER_STRING_LENGTH;
	for (i = 0; i < SERIAL_NUMBER_STRING_LENGTH; i++)
		pDeviceInfo->serialNumber.stringChars[i] = gSerialNumberString[i];
}

static void FillDeviceInfoData(BYTE* pDeviceInfo, DWORD* pDataSize, WHICH_OTG eWhichOtg)
{
	WORD		i, j, index;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
//	PrepareDeviceInfo();
	*pDeviceInfo     = LO_BYTE_OF_WORD	    (psUsbDevSidc->sStiDeviceInfo.standardVersion);
	*(pDeviceInfo+1) = HI_BYTE_OF_WORD	    (psUsbDevSidc->sStiDeviceInfo.standardVersion);
	*(pDeviceInfo+2) = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.vendorExtensionID);
	*(pDeviceInfo+3) = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.vendorExtensionID);
	*(pDeviceInfo+4) = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.vendorExtensionID);
	*(pDeviceInfo+5) = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.vendorExtensionID);
	*(pDeviceInfo+6) = LO_BYTE_OF_WORD		(psUsbDevSidc->sStiDeviceInfo.vendorExtensionVer);
	*(pDeviceInfo+7) = HI_BYTE_OF_WORD		(psUsbDevSidc->sStiDeviceInfo.vendorExtensionVer);
	*(pDeviceInfo+8) = psUsbDevSidc->sStiDeviceInfo.vendorExtensionDesc.numChars;
    j = 9;
//	for (i = 0, j = 9; i < gStiDeviceInfo.vendorExtensionDesc.numChars; i++, j+=2)
//    {
//		*(pDeviceInfo+j) = 		LO_BYTE_OF_WORD(gStiDeviceInfo.vendorExtensionDesc.stringChars[i]);
//		*(pDeviceInfo+j+1) = 	HI_BYTE_OF_WORD(gStiDeviceInfo.vendorExtensionDesc.stringChars[i]);
//	}

	index = j;
	*(pDeviceInfo+index)    = LO_BYTE_OF_WORD		(psUsbDevSidc->sStiDeviceInfo.functionMode);
	*(pDeviceInfo+index+1)  = HI_BYTE_OF_WORD		(psUsbDevSidc->sStiDeviceInfo.functionMode);
	*(pDeviceInfo+index+2)  = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.operations.numElements);
	*(pDeviceInfo+index+3)  = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.operations.numElements);
	*(pDeviceInfo+index+4)  = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.operations.numElements);
	*(pDeviceInfo+index+5)  = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.operations.numElements);
	for (i = 0, j = index+6; i < psUsbDevSidc->sStiDeviceInfo.operations.numElements; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.operations.arrayEntry[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.operations.arrayEntry[i]);
	}
	
	index = j;
	*(pDeviceInfo+index)    = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.events.numElements);
	*(pDeviceInfo+index+1)  = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.events.numElements);
	*(pDeviceInfo+index+2)  = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.events.numElements);
	*(pDeviceInfo+index+3)  = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.events.numElements);
	for (i = 0, j = index+4; i < psUsbDevSidc->sStiDeviceInfo.events.numElements; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.events.arrayEntry[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.events.arrayEntry[i]);
	}
	
	index = j;
	*(pDeviceInfo+index)    = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.deviceProperties.numElements);
	*(pDeviceInfo+index+1)  = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.deviceProperties.numElements);
	*(pDeviceInfo+index+2)  = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.deviceProperties.numElements);
	*(pDeviceInfo+index+3)  = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.deviceProperties.numElements);
	for (i = 0, j = index+4; i < psUsbDevSidc->sStiDeviceInfo.deviceProperties.numElements; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.deviceProperties.arrayEntry[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.deviceProperties.arrayEntry[i]);
	}

	index = j;
	*(pDeviceInfo+index)    = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.captureFormats.numElements);
	*(pDeviceInfo+index+1)  = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.captureFormats.numElements);
	*(pDeviceInfo+index+2)  = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.captureFormats.numElements);
	*(pDeviceInfo+index+3)  = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.captureFormats.numElements);
//	j+=4;
	for (i = 0, j = index+4; i < psUsbDevSidc->sStiDeviceInfo.captureFormats.numElements; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.captureFormats.arrayEntry[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.captureFormats.arrayEntry[i]);
	}

	index = j;
	*(pDeviceInfo+index)    = LO_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.imageFormats.numElements);
	*(pDeviceInfo+index+1)  = MIDLO_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.imageFormats.numElements);
	*(pDeviceInfo+index+2)  = MIDHI_BYTE_OF_DWORD	(psUsbDevSidc->sStiDeviceInfo.imageFormats.numElements);
	*(pDeviceInfo+index+3)  = HI_BYTE_OF_DWORD	    (psUsbDevSidc->sStiDeviceInfo.imageFormats.numElements);
	for (i = 0, j = index+4; i < psUsbDevSidc->sStiDeviceInfo.imageFormats.numElements; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.imageFormats.arrayEntry[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.imageFormats.arrayEntry[i]);
	}
	
	index = j;
	*(pDeviceInfo+index)    = psUsbDevSidc->sStiDeviceInfo.manufacture.numChars;
	for (i = 0, j = index+1; i < psUsbDevSidc->sStiDeviceInfo.manufacture.numChars; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.manufacture.stringChars[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.manufacture.stringChars[i]);
	}
	
	index = j;
	*(pDeviceInfo+index)    = psUsbDevSidc->sStiDeviceInfo.model.numChars;
	for (i = 0, j = index+1; i < psUsbDevSidc->sStiDeviceInfo.model.numChars; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.model.stringChars[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.model.stringChars[i]);
	}

	index = j;
	*(pDeviceInfo+index)    = psUsbDevSidc->sStiDeviceInfo.deviceVersion.numChars;
	for (i = 0, j = index+1; i < psUsbDevSidc->sStiDeviceInfo.deviceVersion.numChars; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.deviceVersion.stringChars[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.deviceVersion.stringChars[i]);
	}

 	index = j;
	*(pDeviceInfo+index)    = psUsbDevSidc->sStiDeviceInfo.serialNumber.numChars;
	for (i = 0, j = index+1; i < psUsbDevSidc->sStiDeviceInfo.serialNumber.numChars; i++, j+=2)
    {
		*(pDeviceInfo+j)    = LO_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.serialNumber.stringChars[i]);
		*(pDeviceInfo+j+1)  = HI_BYTE_OF_WORD	(psUsbDevSidc->sStiDeviceInfo.serialNumber.stringChars[i]);
	}

	*pDataSize = j;

}






//======================================
//sf strtrim
//s
//s "Trims" trailing spaces from
//s the end of a string
//======================================
static void strtrim(char* str, DWORD len)
{
	DWORD i;

    for (i = 0; i < len; i++)
    {
		if ((str[i] == ' ') || (str[i] == 0))
        {
			break;
		}
    }
	str[i] = 0;
}

//-----------------------------------------------------------------------------
//of PTPSendDPSOpReq
//o
//o This function will initiate a DPS operation Request or Response.
//o
//o Inputs:
//o     BYTE* data buffer
//o     DWORD data count
//o     BOOL   request (TRUE, otherwise response
//o
//o Return Value:
//o     None
//-----------------------------------------------------------------------------
static TRANSACTION_STATE PtpSendDPSOpReq(BYTE* dbuf, DWORD dcnt, BOOL isEnableEvent, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    TRANSACTION_STATE   eState = STATE_PTP_INT_IN;

    if (psUsbDevSidc->dwSessionID == 0)
    {
         return FAIL;
    }

    //MP_DEBUG1("PtpSendDPSOpReq:%d", gDpsCurrHdl);
    // "DRSPONSE.DPS" operation?
    if ((psUsbDevSidc->dwDpsRespLen > 0) && (dbuf == 0) && (dcnt == 0))
    {
		MP_DEBUG("1");
		psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsDRespHdl;
    }
    else if ((dbuf == 0) && (dcnt == 0))
    {
        MP_DEBUG("2");
    }

    // if not, must be a "DREQUEST.DPS" operation
    else
    {
		MP_DEBUG("3");
        memcpy(&psUsbDevSidc->pbDpsReqBuf[0], &dbuf[0], dcnt);
        psUsbDevSidc->dwDpsReqLen  = dcnt;
        psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsDReqHdl;
    }

    psUsbDevSidc->psStiEvent->length          = 0x10;
    psUsbDevSidc->psStiEvent->type            = EVENT_BLOCK;
    psUsbDevSidc->psStiEvent->eventCode       = EVENT_REQUEST_OBJECT_TRANSFER;
    psUsbDevSidc->psStiEvent->transactionID   = 0xFFFFFFFF;
    psUsbDevSidc->psStiEvent->parameter[0]    = psUsbDevSidc->dwDpsCurrHdl;
//    UsbSendPtpEvent();

    //gDpsCurrHdl = 0;
    //if (isEnableEvent == FALSE)
    if (IsHostRequest(eWhichOtg) && IsDeviceRequest(eWhichOtg))
    {
        UartOutText("Got-it!!");
        psUsbDevSidc->boIsDpsSendEvent = FALSE;
    }
    else
    {
        psUsbDevSidc->boIsDpsSendEvent = TRUE;
    }
    
    return eState;
}


//======================================
//sf _PtpProcessObjectInfo
//s
//s Process a Received ObjectInfo
//s Dataset
//======================================
static void PtpProcessObjectInfo(PSTI_OBJECT_INFO pobject_info, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
//	static BOOL active = FALSE;
	WORD	obj_format_code = 0;
	WORD	obj_protect_sts = 0;
	DWORD	obj_compress_size = 0;
	DWORD	obj_parent_obj = 0;
//	BYTE*	pStrFileName = 0;
//	DWORD	offset = 0;
	BYTE*	ptr;

	// retrieve/save the required Object Information
	obj_format_code = BYTE_SWAP_OF_WORD(pobject_info->objectFormatCode);
	obj_protect_sts = BYTE_SWAP_OF_WORD(pobject_info->protectionStatus);
	obj_compress_size = BYTE_SWAP_OF_DWORD(pobject_info->objectCompSize);
	obj_parent_obj = BYTE_SWAP_OF_DWORD(pobject_info->parentObject);
//    PtpGetObjFileName((BYTE**)&pobject_info->filename.stringChars);    // file name

//	pobject_info->seqNumber = 0x33343536;

	ptr = (BYTE*) ((BYTE*)pobject_info + (sizeof(pobject_info->storageID)\
								+ sizeof(pobject_info->objectFormatCode)\
								+ sizeof(pobject_info->protectionStatus)\
								+ sizeof(pobject_info->objectCompSize)\
								+ sizeof(pobject_info->thumbFormat)\
								+ sizeof(pobject_info->thumbCompSize)\
								+ sizeof(pobject_info->thumbPixWidth)
								+ sizeof(pobject_info->thumbPixHeight)\
								+ sizeof(pobject_info->imagePixWidth)\
								+ sizeof(pobject_info->imagePixHeight)\
								+ sizeof(pobject_info->imageBitDepth)\
								+ sizeof(pobject_info->parentObject)\
								+ sizeof(pobject_info->associationType)\
								+ sizeof(pobject_info->associationDesc)\
								+ sizeof(pobject_info->seqNumber)\
								+ sizeof(pobject_info->filename.numChars)));
	
    PtpGetObjFileName((BYTE**) &ptr, eWhichOtg);    // file name
//	pStrFileName = pobject_info->filename.stringChars;
//    memcpy(pStrFileName, &pobject_info->filename.stringChars[0], CONFIG_LENGTH);
//DEBUG4("obj_format_code %x, objProtStat %x, objCmpSize %x, objParObj %x  ", obj_format_code, objProtStat, objCmpSize, objParObj);


    // check to see if this is a valid DPS
    // "script" request/response file
    if (obj_format_code != OBJ_FORMAT_SCRIPT)
    {
        //DBGSTR("-I-  _PtpProcessObjectInfo - Not a DPS Script File\r\n");
        return;
    }
    if (strncmp(&psUsbDevSidc->bStrFile[9], gDpsExt, 3) != 0)
    {
        //DBGSTR("-I-  _PtpProcessObjectInfo - Not a DPS File Ext\r\n");
        return;
    }

    // host discovery, command, or response virtual file?
    if (strncmp(psUsbDevSidc->bStrFile, gDpsHDiscStr, 8) == 0)
    {
     //   gDpsCurrHdl = 0;
        SetDpsState(DPS_CONFIGURE_PRINT_SERVICE, eWhichOtg);
     //   geUsbTranStateTemp = geUsbTransactionState;
     //   geUsbTransactionState   = STATE_PTP_INT_IN;
        psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsDReqHdl;
        //	gTest_HDiscovery = TRUE;
        //DBGSTR("-I-  _PtpProcessObjectInfo - DPS Host Discovery\r\n");
    }
    else if (strncmp(psUsbDevSidc->bStrFile, gDpsHReqStr, 8) == 0)
    {
        psUsbDevSidc->dwDpsCurrHdl  = psUsbDevSidc->dwDpsHReqHdl;
		pResponse->parameter[2]     = psUsbDevSidc->dwDpsDRespHdl;
        SetHostRequest(eWhichOtg);
        //gpStiEvent->parameter[0]    = gDpsDRespHdl;
    
    //    if (IsGetCapAlready() == FALSE) // predict the GetCap coming, but not.., the host has requested.
    //    {
    //        SetOpStatusNoScheduled();
    //    }
        //DBGSTR("-I-  _PtpProcessObjectInfo - DPS Host Request\r\n");
    }
    else if (strncmp(psUsbDevSidc->bStrFile, gDpsHRespStr, 8) == 0)
    {
        psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsHRespHdl;
        ClearDeviceRequest(eWhichOtg);
        //DBGSTR("-I-  _PtpProcessObjectInfo - DPS Host Response\r\n");
    }
    else
    {
        psUsbDevSidc->dwDpsCurrHdl = 0;
        //DBGSTR("-I-  _PtpProcessObjectInfo - DPS Host Unknown\r\n");
	}
}

//======================================
//PtpProcessObject
//
// Process a Received Object
// (required only for DPS virtual files)
//======================================
static BOOL PtpProcessObject(BYTE** ptr, WHICH_OTG eWhichOtg)
{
    DWORD rv;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

    if (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHReqHdl)
    {
        rv = DPSPostEvent(*ptr, psUsbDevSidc->pbDpsRespBuf, eWhichOtg);
        //DBGSTR("-I-  DPSPostEvent...");
        if (rv > 0)
        {

            psUsbDevSidc->dwDpsRespLen = rv;
         //   PtpSendDPSOpReq(NULL, NULL);//??? Calvin
            //DBGSTR("...Resp > 0\r\n");
        }
        else
        {
            //DBGSTR("...Resp = 0\r\n");
        }
    }
    else if (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHRespHdl)
    {
        DPSPostResponse(*ptr, PTP_DPS_MAX_XFR, eWhichOtg);            
        //DBGSTR("-I-  _PtpProcessObject - DPS Host Response\r\n");
    }
    else
    {
        //DBGSTR("-I-  _PtpProcessObject - DPS Error\r\n");
    }

    // operation completed...
    //gDpsCurrHdl = NULL;

    return TRUE;
}



//------------------------------------------------------------------------------
//s  PtpOpGetPartialObject
//s
//s  This function is called when the PTP_OC_GET_PARTIAL_OBJECT operation is received.
//s    
//s  OperationCode:          0x101B
//s  Operation Parameter1:   ObjectHandle
//s  Operation Parameter2:   Offset in bytes
//s  Operation Parameter3:   Maximum number of bytes to obtain
//s
//s  Data:                  DataObject
//s  Data Direction:        R -> I
//s
//s  ResponseCode Options:   PTP_RC_OK, 
//s                          PTP_RC_SESSION_NOT_OPEN,
//s                          PTP_RC_INVALID_TRANSACTIONID, 
//s                          PTP_RC_OPERATION_NOT_SUPPORTED, 
//s                          PTP_RC_PARAMETER_NOT_SUPPORTED,
//s                          PTP_RC_INVALID_OBJECT_HANDLE, 
//s                          PTP_RC_INVALID_OBJECT_FORMAT_CODE,
//s                          PTP_RC_STORE_NOT_AVAILABLE,
//s                          PTP_RC_DEV_BUSY,
//s                          PTP_RC_INVALID_PARAMETER
//s  Response Parameter1:    Actual number of bytes sent
//s  Response Parameter2:    None
//s  Response Parameter3:    None
//s
//s  Description: 
//s      Retrieves a partial object from the device. This operation is optional, and may
//s      be used in place of the GetObject operation for devices that support this alternative. If
//s      supported, this operation should be generic, and therefore useable with all types of data
//s      objects present on the device, including both images and non-image data objects, and
//s      should be preceded (although not necessarily immediately) by a GetObjectInfo operation
//s      that uses the same ObjectHandle. For this operation, the size fields in the ObjectInfo
//s      represent maximum size as opposed to actual size. This operation is not necessary for
//s      objects of type Association, as objects of this type are fully qualified by their ObjectInfo
//s      dataset.
//s
//s      The operation behaves exactly like GetObject, except that the second and third
//s      parameters hold the offset in bytes and the number of bytes to obtain starting from the
//s      offset, respectively. If the portion of the object that is desired is from the offset to the
//s      end, the third parameter may be set to 0xFFFFFFFF. The first response parameter
//s      should contain the actual number of bytes of the object sent, not including any wrappers
//s      or overhead structures.
//s  
//------------------------------------------------------------------------------
static TRANSACTION_STATE PtpOpGetPartialObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    OBJECT_HEADER	header;
    DWORD          storageID;
    TRANSACTION_STATE eState = STATE_PTP_DATA_IN;

    PSTI_CONTAINER_HEAD	pDataContainerHead;
    DWORD   objhandle;
    DWORD   len = 0;
    DWORD   dataSize = 0;
    DWORD   imgSize = 0;
    DWORD   bufIdx;
    BYTE    *pDataIdx = (BYTE*)*hData;
    DWORD   byte_cnt= 0;
    int     retVal = 0;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg); 
    BOOL bFileOpenRst = TRUE;
    //DRIVE   drv;
    //BYTE    *pImgBuff;
   // BYTE    *pImgData;
    
//    MP_DEBUG("!!!PtpOpGetPartialObject: check later");

    pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData);

    pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
    pDataContainerHead->code            = pCommand->code;
    pDataContainerHead->transactionID   = pCommand->transactionID;
    pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE);
    *hData += USB_STI_CONTAINER_SIZE;
    objhandle = BYTE_SWAP_OF_DWORD(pCommand->parameter[0]);

    PtpGetObjectHeader(objhandle, &header, eWhichOtg);
    storageID = (header.physicalStore << 16) | (header.logicalStore);


    //-------------------------------------------------------------------------
    // Check if session is open
    //-------------------------------------------------------------------------
    if (psUsbDevSidc->dwSessionID == 0)
    {
        pResponse->code  = RESPONSE_SESSION_NOT_OPEN;
    }

    //-------------------------------------------------------------------------
    // Check for valid transaction ID
    //-------------------------------------------------------------------------
    else if ( BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID + 1 )
    {
        pResponse->code = RESPONSE_INVALID_TRANSACTION_ID;
    }

    //-------------------------------------------------------------------------
    //  Verify ObjectHandle parameter
    //-------------------------------------------------------------------------
    else if (PtpCheckValidHandle(objhandle, eWhichOtg) == FALSE)
    {
       pResponse->code = RESPONSE_INVALID_OBJECT_HANDLE;
    }

    //-------------------------------------------------------------------------
    // Verify Store is available
    //-------------------------------------------------------------------------
    else if (PtpCheckStoreAvailable(storageID, eWhichOtg) == FALSE)
    {
        pResponse->code = RESPONSE_STORE_NOT_AVAILABLE;
    }

    //-------------------------------------------------------------------------
    // Verify unused parameters
    //-------------------------------------------------------------------------
    else if ((pCommand->parameter[3] != 0) || 
             (pCommand->parameter[4] != 0))
    {
        pResponse->code = RESPONSE_PARAMETER_NOT_SUPPORTED;
    }
	
    //-------------------------------------------------------------------------
    //  If all parameters are ok, prepare the ObjectInfo dataset for the
    //  Data Phase
    //-------------------------------------------------------------------------
    else
    {   
        WORD format;

        pResponse->containerLength  = USB_STI_CONTAINER_SIZE + 4;
        pResponse->containerType    = RESPONSE_BLOCK;
        pResponse->code             = RESPONSE_OK;
        pResponse->transactionID    = pCommand->transactionID;
        //pResponse->parameter[0]     = BYTE_SWAP_OF_DWORD(pCommand->parameter[2]);        

        if (psUsbDevSidc->psFile == (STREAM*)NULL )  // First open file
        {
            MP_DEBUG("FileOpenForPtp");
            psUsbDevSidc->psFile = FileOpenForPtp(&format, objhandle);
            if (psUsbDevSidc->psFile == (STREAM*)NULL)
            {
                MP_ALERT("--E-- %s -open file fail- ObjHdl:%d", __FUNCTION__, objhandle);
                bFileOpenRst = FALSE;
            }
        }        

        if(SystemCardPresentCheck(DriveCurIdGet()) == FALSE || bFileOpenRst == FALSE) // Whether Card Out or Open File Fail
        {
            MP_ALERT("--E-- %s -No storage- ObjHdl:%d", __FUNCTION__, objhandle);
            FileClose(psUsbDevSidc->psFile);
            psUsbDevSidc->boIsFileSending = FALSE;
            pResponse->code = RESPONSE_INVALID_OBJECT_HANDLE;
            pResponse->parameter[0] = 0; 
        }
        else
        {
            bufIdx = BYTE_SWAP_OF_DWORD(pCommand->parameter[1]);
            if(bufIdx == 0)    // GetObject /GetPartialObject Offset from 0
            		psUsbDevSidc->sdwImageSize = FileSizeGet(psUsbDevSidc->psFile);   //Get Image Size	
            		
            psUsbDevSidc->dwImageSizeRead = bufIdx;            
          
            imgSize = BYTE_SWAP_OF_DWORD(pCommand->parameter[2]);
            imgSize = (imgSize >= (psUsbDevSidc->sdwImageSize - psUsbDevSidc->dwImageSizeRead))?
                      (psUsbDevSidc->sdwImageSize - psUsbDevSidc->dwImageSizeRead):BYTE_SWAP_OF_DWORD(pCommand->parameter[2]); 
            pResponse->parameter[0] = imgSize; 
            
            if (imgSize == 0xffffffff)
            {
                imgSize = FileSizeGet(psUsbDevSidc->psFile);
            }
            else
            {
                retVal = Seek(psUsbDevSidc->psFile, bufIdx);
                if (retVal != 0)
                {
                    MP_ALERT("--E-- %s -seek file fail-ObjHdl:%d", __FUNCTION__, objhandle);
                    //BREAK_POINT();
                }
            }

            if (imgSize >= USB_OTG_BUF_SIZE)
            {
                dataSize = USB_OTG_BUF_SIZE - USB_STI_CONTAINER_SIZE;
            }
            else
            {
                dataSize = imgSize;
            }
            
            //MP_DEBUG("FileRead_1");
            byte_cnt = FileRead(psUsbDevSidc->psFile, (BYTE*)(*hData), dataSize);
            psUsbDevSidc->dwImageSizeRead += dataSize;
            if (psUsbDevSidc->dwImageSizeRead == psUsbDevSidc->sdwImageSize)
                psUsbDevSidc->dwImageSizeRead = 0;
            else
                psUsbDevSidc->dwImageSizeRead -= dataSize;
            
            if (byte_cnt != dataSize)
            {
                MP_ALERT("--E-- %s -read file fail-ObjHdl:%d", __FUNCTION__, objhandle);
            }
            
            //gImageSizeRead = dataSize;
            len = USB_STI_CONTAINER_SIZE + pResponse->parameter[0];
            pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(len);
            //pResponse->parameter[0]             = len;
            psUsbDevSidc->boIsFileSending = TRUE;

            //FileClose(ghFile);
        }

        *hData = pDataIdx;
}    

    //DBGSTR("-I-  _PtpOpGetPartialObject - OK\r\n");
    return eState;
}

static TRANSACTION_STATE OperationGetDeviceInfo (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD				responseCode = RESPONSE_OK;
	DWORD				deviceInfoSize = 0;
    
	BYTE*				pDeviceInfo;
	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_DATA_IN;
        
//    *hData = gpStiDataContainer;
    pDeviceInfo = (BYTE*)((DWORD)(*hData) + USB_STI_CONTAINER_SIZE);
	FillDeviceInfoData ( pDeviceInfo, &deviceInfoSize, eWhichOtg);
    pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData); // BYTE_SWAP_OF_DWORD
	pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE + deviceInfoSize);
	pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
	pDataContainerHead->code            = pCommand->code;
	pDataContainerHead->transactionID   = pCommand->transactionID;
//    gTransactionID = pCommand->transactionID;
    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
	pResponse->transactionID    = pCommand->transactionID;
//	showmeplus("OperationGetDeviceInfo:pContainerHead:");
//	showme((pINT8) pContainerHead, len, kSkip);

	return eState;
}


static TRANSACTION_STATE OperationOpenSession (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD				responseCode = RESPONSE_OK;
//	DWORD				deviceInfoSize = 0;
    
	DWORD			    sessionID;
//	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_RES;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);

//    *hData = 0; //gpStiDataContainer;
	sessionID = BYTE_SWAP_OF_DWORD(pCommand->parameter[0]);

	if (sessionID == 0)
    {
		responseCode = RESPONSE_INVALID_PARAMETER;
	}
    else if ( psUsbDevSidc->dwSessionID == sessionID )
	{
		responseCode = RESPONSE_SESSION_ALREADY_OPEN;
	}
    else if ( psUsbDevSidc->dwSessionID == 0 )
    {
		psUsbDevSidc->dwSessionID = sessionID;
	}
    else
    {	// USB STI not support multi session
		responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
	}

    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
	pResponse->transactionID    = pCommand->transactionID;
    
	return eState;
}

static DWORD OperationCloseSession(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD	responseCode = RESPONSE_OK;
    TRANSACTION_STATE eState = STATE_PTP_RES;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
//    *hData = gpStiDataContainer;
    if (psUsbDevSidc->dwSessionID == 0)
    { // Check if session is open
        responseCode = RESPONSE_SESSION_NOT_OPEN;
        MP_ALERT("%s -Not Open-", __FUNCTION__);
    }
    else if ( BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID + 1 )
    { // Check for valid transaction ID
        responseCode = RESPONSE_INVALID_TRANSACTION_ID;
        MP_ALERT("%s -PTP_RC_INVALID_TRANSACTIONID-", __FUNCTION__);
    }
    else if ((pCommand->parameter[0] != 0) || 
             (pCommand->parameter[1] != 0) || 
             (pCommand->parameter[2] != 0) || 
             (pCommand->parameter[3] != 0) || 
             (pCommand->parameter[4] != 0))
    { // Verify unused parameters
        responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
        MP_ALERT("%s -Parameter Not Supported-", __FUNCTION__);
    }
    else
    { //  If all parameters are ok, close the current session
        responseCode = RESPONSE_OK;
        psUsbDevSidc->dwTransactionID = 0;
        psUsbDevSidc->dwSessionID = 0;
    }

    //DBGSTR("-I-  _PtpOpCloseSession - OK\r\n");

    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
	pResponse->transactionID    = pCommand->transactionID;
    PtpInit(eWhichOtg); // for PictBridge illeagal testing  
	return eState;
}



static TRANSACTION_STATE OperationGetNumObjects (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD				responseCode = RESPONSE_OK;
//	DWORD				deviceInfoSize = 0;
    
//	DWORD			    sessionID;
//	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_RES;
    DWORD  obj_format_code = 0;
    DWORD   habdle_parent;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
//    DWORD  num_of_obj = 0;//gPtpNumHandles;
    
	// only one store is available; if supports muti-storage, it needs to modify
	pCommand->parameter[0] = DEVICE_STORAGE_ID;

//    *hData = gpStiDataContainer;
	psUsbDevSidc->dwStorageID = pCommand->parameter[0];
    obj_format_code =  BYTE_SWAP_OF_DWORD(pCommand->parameter[1]);
	if (psUsbDevSidc->dwSessionID == 0) 
    { // Check if session is open
		responseCode = RESPONSE_SESSION_NOT_OPEN;
	}
    else if ( BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID + 1 )
	{ // Check for valid transaction ID
		responseCode = RESPONSE_INVALID_TRANSACTION_ID;
	}
    else if (PtpCheckStorageID(psUsbDevSidc->dwStorageID, eWhichOtg) == FALSE)
    { // Verify StorageID parameter
		responseCode = RESPONSE_INVALID_STORAGE_ID;
    }
    else if (PtpCheckStoreAvailable(psUsbDevSidc->dwStorageID, eWhichOtg) == FALSE)
    { // Verify Store is available
		responseCode = RESPONSE_STORE_NOT_AVAILABLE;
    }
    else if ((obj_format_code != 0)                   &&
             (obj_format_code != OBJ_FORMAT_ASSOCIATION) &&
             (obj_format_code != OBJ_FORMAT_SCRIPT)      &&
             (obj_format_code != OBJ_FORMAT_BMP)        &&
             (obj_format_code != OBJ_FORMAT_JPEG_EXIF)   && 
             (obj_format_code != 0xFFFFFFFF))
    { // Verify Format Type
		responseCode = RESPONSE_INVALID_OBJECT_FORMAT_CODE;
	}
    else if ((pCommand->parameter[3] != 0) ||   
             (pCommand->parameter[4] != 0))
    { // Verify unused parameters
		responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
	}
    else
    { // Verify Parent Handle
        habdle_parent = BYTE_SWAP_OF_DWORD(pCommand->parameter[2]);
        if ((habdle_parent == 0) && (obj_format_code == 0))
        {
            responseCode = RESPONSE_OK;
            pResponse->parameter[0]   = psUsbDevSidc->dwPtpNumHandles;
        }
        else
        {
            
            if (PtpCheckValidHandle(habdle_parent, eWhichOtg) == FALSE) 
            {   // Make sure the parent handle is valid
                responseCode = RESPONSE_INVALID_OBJECT_HANDLE;
            }            
            else if (PtpCheckValidParent(habdle_parent, eWhichOtg) == FALSE) 
            {   // Make sure the parent handle is actually a parent
                responseCode = RESPONSE_INVALID_PARENT_OBJECT;
            }
            else    // Search through all handles and add up the number of
            {       // objects who have a matching parent & object format
                pResponse->parameter[0]    = PtpGetNumChildren(habdle_parent, obj_format_code, eWhichOtg);
                responseCode= RESPONSE_OK;
            }
        }
    }

    pResponse->containerLength  = USB_STI_CONTAINER_SIZE + 4;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
	pResponse->transactionID    = pCommand->transactionID;
//    pResponse->parameter[0]     = num_of_obj;
    
	return eState;
}


static TRANSACTION_STATE OperationGetObjectHandle (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD				responseCode = RESPONSE_OK;
//	DWORD				deviceInfoSize = 0;
    
//	DWORD			    sessionID;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_RES;
    DWORD  obj_format_code = 0;
    DWORD   handle_parent;
    DWORD  num_of_obj = psUsbDevSidc->dwPtpNumHandles;
    OBJECT_HEADER header;
    int i=0;
    BOOL is_parent_valid, is_format_valid;
	BYTE* pDataIdx = (BYTE*)*hData;
    
	// only one store is available; if supports muti-storage, it needs to modify
	pCommand->parameter[0] = DEVICE_STORAGE_ID;

//    *hData = gpStiDataContainer;
	psUsbDevSidc->dwStorageID = pCommand->parameter[0];
    obj_format_code =  BYTE_SWAP_OF_DWORD(pCommand->parameter[1]);
	if (psUsbDevSidc->dwSessionID == 0) 
    {   // Check if session is open
		responseCode = RESPONSE_SESSION_NOT_OPEN;
	}
    else if ( BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID + 1 )
	{   // Check for valid transaction ID
		responseCode = RESPONSE_INVALID_TRANSACTION_ID;
	}
    else if (PtpCheckStorageID(psUsbDevSidc->dwStorageID, eWhichOtg) == FALSE)
    {   // Verify StorageID parameter
		responseCode = RESPONSE_INVALID_STORAGE_ID;
    }
    else if (PtpCheckStoreAvailable(psUsbDevSidc->dwStorageID, eWhichOtg) == FALSE)
    {   // Verify Store is available
		responseCode = RESPONSE_STORE_NOT_AVAILABLE;
    }
    else if ((obj_format_code != 0)                   &&
             (obj_format_code != OBJ_FORMAT_ASSOCIATION) &&
             (obj_format_code != OBJ_FORMAT_SCRIPT)      &&
             (obj_format_code != OBJ_FORMAT_BMP)        &&
             (obj_format_code != OBJ_FORMAT_JPEG_EXIF)   && 
             (obj_format_code != 0xFFFFFFFF))
    {   // Verify Format Type
		responseCode = RESPONSE_INVALID_OBJECT_FORMAT_CODE;
	}
    else if ((pCommand->parameter[3] != 0) ||   
             (pCommand->parameter[4] != 0))
    {   // Verify unused parameters
		responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
	}
    else
    {   // Verify Parent Handle
        handle_parent = BYTE_SWAP_OF_DWORD(pCommand->parameter[2]);
        eState = STATE_PTP_DATA_IN;
        if ((handle_parent == 0) && (obj_format_code == 0))
        {
            responseCode = RESPONSE_OK; 
//            *hData = gpStiDataContainer;
            pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData);
	        pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE + 4 + 4*num_of_obj);
        	pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
        	pDataContainerHead->code            = pCommand->code;
        	pDataContainerHead->transactionID   = pCommand->transactionID;

            *hData += USB_STI_CONTAINER_SIZE;
            PtpAddDword(hData, num_of_obj);
            for (i = 1; i <= psUsbDevSidc->dwPtpNumHandles; i++)
            {
                PtpGetObjectHeader(i, &header, eWhichOtg);
                PtpAddDword(hData, i);
            }
        }        
        else    // If a parent and/or an object format was specified we will have
        {       // to search through all of the handles and count the right ones            
            if (PtpCheckValidHandle(handle_parent, eWhichOtg) == FALSE)
            {   // Make sure the parent handle is valid
                responseCode = RESPONSE_INVALID_OBJECT_HANDLE;
            }
            
            if (PtpCheckValidParent(handle_parent, eWhichOtg) == FALSE)
            {   // Make sure the parent handle is actually a parent
                responseCode = RESPONSE_INVALID_PARENT_OBJECT;
            }
            else
            {                
                responseCode = RESPONSE_OK; // default the response                
                num_of_obj  = PtpGetNumChildren(handle_parent, obj_format_code, eWhichOtg); // acquire the handle info			
                MP_DEBUG("-USBOTG%d- %s Format %x Cnt %d", eWhichOtg, __FUNCTION__, obj_format_code, num_of_obj);
           //     *hData = gpStiDataContainer;
                pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData);
    	        pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE + 4 + 4*num_of_obj);
            	pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
            	pDataContainerHead->code            = pCommand->code;
            	pDataContainerHead->transactionID   = pCommand->transactionID;
                *hData += USB_STI_CONTAINER_SIZE;
                PtpAddDword(hData, num_of_obj); // add the requested handles
                
                for (i = 1; i <= psUsbDevSidc->dwPtpNumHandles; i++)
                {                    
                    PtpGetObjectHeader(i, &header, eWhichOtg); // get the object info
                    is_parent_valid = FALSE;
                    is_format_valid = FALSE;

                    // parent object match?
                    //  (0 = entire device, 0xFFFFFFFF = ROOT only)
                    if ((handle_parent == 0) ||
                        (handle_parent == header.parentHandle) ||
                        ((handle_parent == 0xFFFFFFFF) && (header.parentHandle == 0)))
                    {
                        is_parent_valid = TRUE;
                    }

                    // format match?
                    //  (0 = all formats, 0xFFFFFFFF = image formats only)
                    if ((obj_format_code == 0)               ||
                        (obj_format_code == header.objectFormat)||
                        ((obj_format_code == 0xFFFFFFFF)        &&
                         ((header.objectFormat == OBJ_FORMAT_JPEG_EXIF) ||
                          (header.objectFormat == OBJ_FORMAT_BMP))))
                    {
                        is_format_valid = TRUE;
                    }

                    if (is_parent_valid && is_format_valid)
                    {
                        PtpAddDword(hData, i);
                    }
                }
            }
        }
    }

	*hData = pDataIdx;
    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
	pResponse->transactionID    = pCommand->transactionID;
    
	return eState;
}



static TRANSACTION_STATE OperationGetObjectInfo (BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{  
	WORD	responseCode = RESPONSE_OK;
//	DWORD	deviceInfoSize = 0;
	DWORD	storageID;
//	DWORD	thumbSize;
//	DWORD	thumbWidth;
//	DWORD	thumbHeight;
//	DWORD	imageWidth;
//	DWORD	imageHeight;
//	DWORD	imageBitDepth;
	DWORD	parentObject;
//	DWORD	associationDesc;
//	DWORD	sequenceNum;
//	DWORD	objectFormat;
//	DWORD	thumbFormat;
//	DWORD	associationType;
   
//	DWORD			    sessionID;
	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_RES;
//    DWORD  obj_format_code = 0;
//    DWORD   handle_parent;
//    DWORD  num_of_obj = gPtpNumHandles;
    OBJECT_HEADER header;
//    int i=0;
//    BOOL is_parent_valid, is_format_valid;
//    PSTI_OBJECT_INFO    pObjInfo;
    STI_OBJECT_INFO    objInfo;
	BYTE*	pDataIdx = (BYTE*)*hData;
	DWORD	objhandle;
	DWORD	len;
    WORD    date;
    WORD    time;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
        
//    *hData = gpStiDataContainer;
    pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData);
//    pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE + STI_OBJECT_INFO_SIZE);
	pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
	pDataContainerHead->code            = pCommand->code;
	pDataContainerHead->transactionID   = pCommand->transactionID;
    *hData += USB_STI_CONTAINER_SIZE;
	objhandle = BYTE_SWAP_OF_DWORD(pCommand->parameter[0]);
    psUsbDevSidc->dwDpsCurrHdl = objhandle;
    if ((objhandle == psUsbDevSidc->dwDpsDDiscHdl) ||
        (objhandle == psUsbDevSidc->dwDpsDReqHdl)  ||
        (objhandle == psUsbDevSidc->dwDpsDRespHdl))
    { // info request for a DPS virtual object?
        // flag the DPS status as busy
   //     gDpsCurrHdl = objhandle;
        if (objhandle == psUsbDevSidc->dwDpsDReqHdl)
            SetDeviceRequest(eWhichOtg);

        if (objhandle == psUsbDevSidc->dwDpsDRespHdl)
            ClearHostRequest(eWhichOtg);

        // default the transfer values, allocate a buffer
        responseCode = RESPONSE_OK;
        eState       = STATE_PTP_DATA_IN;

        // build the dataset
        //  ("ObjectCompressedSize" must be  set to
        //   the actual length of the request/response)
        objInfo.storageID         = DEVICE_STORAGE_ID;
        objInfo.objectFormatCode  = OBJ_FORMAT_SCRIPT;
        objInfo.protectionStatus  = 0;
    //    objInfo.objectCompSize    = 0;
        if (objhandle == psUsbDevSidc->dwDpsDReqHdl)
        {
            objInfo.objectCompSize = psUsbDevSidc->dwDpsReqLen;
        }
        else
        {
            objInfo.objectCompSize = psUsbDevSidc->dwDpsRespLen;
        }
        objInfo.thumbFormat       = 0;
        objInfo.thumbCompSize     = 0;
        objInfo.thumbPixWidth     = 0;
        objInfo.thumbPixHeight    = 0;
        objInfo.imagePixWidth     = 0;
        objInfo.imagePixHeight    = 0;
        objInfo.imageBitDepth     = 0;
        objInfo.parentObject      = 0;
        objInfo.associationType   = 0;
        objInfo.associationDesc   = 0;
        objInfo.seqNumber         = 0;

		PtpAddDword	(hData, objInfo.storageID);
		PtpAddWord	(hData, objInfo.objectFormatCode);
		PtpAddWord	(hData, objInfo.protectionStatus);
		PtpAddDword	(hData, objInfo.objectCompSize);
		PtpAddWord	(hData, objInfo.thumbFormat);
		PtpAddDword	(hData, objInfo.thumbCompSize);
		PtpAddDword	(hData, objInfo.thumbPixWidth);
		PtpAddDword	(hData, objInfo.thumbPixHeight);
		PtpAddDword	(hData, objInfo.imagePixWidth);
		PtpAddDword	(hData, objInfo.imagePixHeight);
		PtpAddDword	(hData, objInfo.imageBitDepth);
		PtpAddDword	(hData, objInfo.parentObject);
		PtpAddWord	(hData, objInfo.associationType);
		PtpAddDword	(hData, objInfo.associationDesc);
		PtpAddDword	(hData, objInfo.seqNumber);
		
        if (objhandle == psUsbDevSidc->dwDpsDDiscHdl)
        {
            PtpAddFname(hData, (BYTE*)gDpsDDiscStr, (BYTE*)gDpsExt);
        }
        else if (objhandle == psUsbDevSidc->dwDpsDReqHdl)
        {
            PtpAddFname(hData, (BYTE*)gDpsDReqStr, (BYTE*)gDpsExt);
        }
        else
        {
            PtpAddFname(hData, (BYTE*)gDpsDRespStr, (BYTE*)gDpsExt);
        }

		PtpAddString(hData, 0);
		PtpAddString(hData, 0);
		PtpAddString(hData, 0);

		len = ((DWORD)*hData & 0x0000FFFF) - ((DWORD)pDataIdx & 0x0000FFFF);
		pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(len);
        objInfo.objectCompSize = len;

		*hData = pDataIdx;
	    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
	    pResponse->containerType    = RESPONSE_BLOCK;
	    pResponse->code             = responseCode;
		pResponse->transactionID    = pCommand->transactionID;
        
        return eState;
    }//DDiscHdl.dps  DReqHdl.dps  DRespHdl.dps
//    else
//    { // Make Object Info packet (R->I) for other type(Image etc...)
        // add later
//        MP_DEBUG("Other format");
//		__asm("break 100");
//    }
#if 1 // process real file
//========================================================== 
// Make Object Info packet (R->I) for other type(Image etc...)       Calvin 0723 
//==========================================================

    ClearNewJobOk(eWhichOtg);
    // if not perform preliminary setup
    PtpGetObjectHeader(objhandle, &header, eWhichOtg);
    storageID    = (header.physicalStore << 16) | (header.logicalStore);
    parentObject = header.parentHandle;
//    DCFGetFilename(request->Parameter1, filename);

    //-------------------------------------------------------------------------
    // Check if session is open
    //-------------------------------------------------------------------------
    if (psUsbDevSidc->dwSessionID == 0)
    {
        responseCode  = RESPONSE_SESSION_NOT_OPEN;
        MP_ALERT("%s -Session Not Open-", __FUNCTION__);
    }

    //-------------------------------------------------------------------------
    // Check for valid transaction ID
    //-------------------------------------------------------------------------
    else if ( BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID + 1 )
    {
        responseCode = RESPONSE_INVALID_TRANSACTION_ID;
     //   DBGERR1("-E- _PtpOpGetObjectInfo - Transaction [%x]", pCommand->transactionID);
    }

    //-------------------------------------------------------------------------
    //  Verify StorageID parameter
    //-------------------------------------------------------------------------
    else if (PtpCheckValidHandle(objhandle, eWhichOtg) == FALSE)
    {
        responseCode = RESPONSE_INVALID_OBJECT_HANDLE;
      //  DBGERR1("-E- _PtpOpGetObjectInfo - Invalid ObjectHandle [%x]", objhandle);

    }

    //-------------------------------------------------------------------------
    // Verify Store is available
    //-------------------------------------------------------------------------
    else if (PtpCheckStoreAvailable(storageID, eWhichOtg) == FALSE)
    {
        responseCode = RESPONSE_STORE_NOT_AVAILABLE;
     //   DBGERR1("-E- _PtpOpGetObjectInfo - Invalid StorageID [%x]", storageID);
    }

    //-------------------------------------------------------------------------
    // Verify unused parameters
    //-------------------------------------------------------------------------
    else if ((pCommand->parameter[1] != 0) || 
	         (pCommand->parameter[2] != 0) || 
    	     (pCommand->parameter[3] != 0) || 
        	 (pCommand->parameter[4] != 0))
    {
        responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
        MP_ALERT("%s -Invalid 0 Parameter-", __FUNCTION__);
    }
    
    //-------------------------------------------------------------------------
    //  If all parameters are ok, prepare the ObjectInfo dataset for the
    //  Data Phase
    //-------------------------------------------------------------------------
    else
    {
        STREAM *hFile;
        DRIVE drv;
        WORD   format = 0;
        
        // setup defaults & control variables
        responseCode = RESPONSE_OK;
        eState       = STATE_PTP_DATA_IN;
        
        /*
            0  ConLen          0x9E000000  158
            4  ConType             0x0200  Data
            6  Code                0x0810  GetObjectInfo
            8  TransID         0x8B000000  139
           12  StorageID       0x01000100  0x10001
           16  ObjectFormat        0x0138  EXIF/JPEG OBJ_FORMAT_JPEG_EXIF
           18  ProtectStat         0x0000  No Protection
           20  ObjCompSize     0xE5ED0000  0xEDE5
           24  ThumbFormat         0x0838  JFIF
           26  ThumbCompSize   0x00000000  0
           30  ThumbPixWidth   0xF9001400  1310969
           34  ThumbPixHeight  0xFFFFFFFF  -1
           38  ImagePixWidth   0xFFFFFFFF  -1
           42  ImagePixHeight  0x01000000  1
           46  ImageBitDepth   0x00000000  0x00000000
           50  ParentObject    0x00000000  0
           54  AssocType           0x0000  Undefined
           56  AssocDesc       0x00000000  Undefined
           60  SeqNum          0x00000000  0x0000
           64  StrLenFile            0x0D  26
           65  Filename            0x4900  IMAG0001.JPG
           91  StrLenCapDate         0x10  32
           92  CaptureDate     0x32003000  November 08, 2004 at 17:05:24
          124  StrLenModDate         0x10  32
          125  ModDate         0x32003000  November 08, 2004 at 17:05:24
          157  Keywords              0x00  Undefined

        */
        
        hFile = FileOpenForPtp(&format, objhandle);
        if (hFile == (STREAM*)NULL)
        {
            MP_ALERT("--E-- %s -open file fail- ObjHdl:%d", __FUNCTION__, objhandle);
            responseCode = RESPONSE_INVALID_OBJECT_HANDLE;
            //return STATE_PTP_RES;
            
            psUsbDevSidc->sdwImageSize = 0;
            psUsbDevSidc->bIsThumbnail = FALSE;           
        }    
        else
        {
            psUsbDevSidc->sdwImageSize = FileSizeGet(hFile);

            //Check Thumb exist 
            psUsbDevSidc->bIsThumbnail = Check_Thumb_exist(hFile, &psUsbDevSidc->sThumbnailInfo); 
        }
        FileClose(hFile);

        //if (IsPrinterSupportFileType(format) == FALSE)
        //{  // stop!!
        //    SetDpsState(DPS_ABORT_JOB);
        //    SystemSetErrMsg(ERR_PICT_FILE_TYPE_NOT_SUPPORT);
        //    EventSet(UI_EVENT, EVENT_ERROR);
        //}
        psUsbDevSidc->wPrintedFileType = format;
        
        // build the dataset
        objInfo.storageID           = DEVICE_STORAGE_ID;
        objInfo.objectFormatCode    = format;//OBJ_FORMAT_JPEG_EXIF;
        objInfo.protectionStatus    = 0;
        objInfo.objectCompSize      = psUsbDevSidc->sdwImageSize;//TEST_JPEG_SIZE;
        objInfo.thumbFormat         = OBJ_FORMAT_JFIF;//0x3808;
        objInfo.thumbCompSize       = psUsbDevSidc->sThumbnailInfo.End - psUsbDevSidc->sThumbnailInfo.Start + 1; 
        objInfo.thumbPixWidth       = psUsbDevSidc->sThumbnailInfo.Width;   //0xa0;
        objInfo.thumbPixHeight      = psUsbDevSidc->sThumbnailInfo.Height;  //0x78;
        objInfo.imagePixWidth       = 0;//320;//640;//
        objInfo.imagePixHeight      = 0;//240;//480;//
        objInfo.imageBitDepth       = 24;//24;
        objInfo.parentObject        = 0;
        objInfo.associationType     = 0;
        objInfo.associationDesc     = 0;
        objInfo.seqNumber           = 0;
        date = 0x32fa; // 2005,07,26
        time = 0x48dd; // 09:06:59
        
    //    objInfo = (PSTI_OBJECT_INFO) (*hData);

        PtpAddDword	(hData, objInfo.storageID);
        PtpAddWord	(hData, objInfo.objectFormatCode);
        PtpAddWord	(hData, objInfo.protectionStatus);
        PtpAddDword	(hData, objInfo.objectCompSize);
        PtpAddWord	(hData, objInfo.thumbFormat);
        PtpAddDword	(hData, objInfo.thumbCompSize);
        PtpAddDword	(hData, objInfo.thumbPixWidth);
        PtpAddDword	(hData, objInfo.thumbPixHeight);
        PtpAddDword	(hData, objInfo.imagePixWidth);
        PtpAddDword	(hData, objInfo.imagePixHeight);
        PtpAddDword	(hData, objInfo.imageBitDepth);
        PtpAddDword	(hData, objInfo.parentObject);
        PtpAddWord	(hData, objInfo.associationType);
        PtpAddDword	(hData, objInfo.associationDesc);
        PtpAddDword	(hData, objInfo.seqNumber);
        PtpAddFname    (hData, "IMAG0001", "JPG");
        PtpAddDateTime (hData, date, time);
        PtpAddDateTime (hData, date, time);    
        PtpAddString   (hData, 0);

        len = ((DWORD)*hData & 0x0000FFFF) - ((DWORD)pDataIdx & 0x0000FFFF);
        pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(len);
   //     objInfo.objectCompSize = len;

        *hData = pDataIdx;
    }    
#endif // 0

    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->code             = responseCode;
    pResponse->transactionID    = pCommand->transactionID;
   
    return eState;
}

static TRANSACTION_STATE OperationSendObjectInfo(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    TRANSACTION_STATE eState = STATE_PTP_DATA_OUT;
    WORD             responseCode = RESPONSE_OK;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    
//    *hData = gpStiDataContainer;
    pResponse->containerType    = RESPONSE_BLOCK;
    pResponse->transactionID    = pCommand->transactionID;
    if ((pCommand->parameter[0] == 0) && (pCommand->parameter[1] == 0))
    { // is this info for a virtual DPS request/response object?
        pResponse->parameter[0] = 0;
        pResponse->parameter[1] = 0;

        if (psUsbDevSidc->boDpsEnabled == FALSE)
        {
            psUsbDevSidc->dwDpsCurrHdl  = psUsbDevSidc->dwDpsHDiscHdl;
            psUsbDevSidc->boDpsEnabled  = TRUE;
            pResponse->parameter[2] = psUsbDevSidc->dwDpsHDiscHdl;
        }
        else
        {
            psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsHReqHdl;
            pResponse->parameter[2] = psUsbDevSidc->dwDpsHReqHdl;
        }

        psUsbDevSidc->bPtpDataSource = PTP_DATA_TO_BUFFER;
        pResponse->containerLength  = USB_STI_CONTAINER_SIZE + 3 * sizeof(DWORD);
        pResponse->code             = responseCode;
        return eState;
    }
    
    pCommand->parameter[0] = DEVICE_STORAGE_ID; // default to the only available store    
    responseCode = RESPONSE_OK; // default the response block to "OK"
    if (psUsbDevSidc->dwSessionID == 0)
    { // Check if session is open
        responseCode  = RESPONSE_SESSION_NOT_OPEN;
    }
    else if (BYTE_SWAP_OF_DWORD(pCommand->transactionID) != psUsbDevSidc->dwTransactionID+1)
    { // Check for valid transaction ID
        responseCode = RESPONSE_INVALID_TRANSACTION_ID;
    }
    else if ((pCommand->parameter[2] != 0) || 
             (pCommand->parameter[3] != 0) || 
             (pCommand->parameter[4] != 0))
    { // Verify unused parameters
        responseCode = RESPONSE_PARAMETER_NOT_SUPPORTED;
    }
    else if (pCommand->parameter[0] != 0)
    { //  Verify StorageID/ObjectHandle parameters (if necessary)        
        if (PtpCheckStorageID(pCommand->parameter[0], eWhichOtg) == FALSE)
        { // is the specified store available & accessible?
            responseCode = RESPONSE_STORE_NOT_AVAILABLE;
        }
        
        if (pCommand->parameter[1] == 0xFFFFFFFF)
        { // 0xFFFFFFFF is the code for ROOT.
            pCommand->parameter[1] = 0;
        }
        
        if (!PtpCheckValidParent(pCommand->parameter[1], eWhichOtg))
        { // is the specified parent location valid?
            responseCode = RESPONSE_INVALID_PARENT_OBJECT;
        }
    }

    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
    pResponse->code             = responseCode;

    return eState;
}

static TRANSACTION_STATE OperationSendObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    WORD	responseCode = RESPONSE_OK;
    TRANSACTION_STATE eState = STATE_PTP_DATA_OUT;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    

//    *hData = gpStiDataContainer;
    pResponse->containerType    = RESPONSE_BLOCK;
	pResponse->transactionID    = pCommand->transactionID;
    // is this a virtual DPS object?
    if ((psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHReqHdl) || (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHRespHdl))
    {
        // setup the transfer variables
        pResponse->parameter[0] = 0;
        pResponse->parameter[1] = 0;
        pResponse->parameter[2] = 0;
        psUsbDevSidc->bPtpDataSource = PTP_DATA_TO_BUFFER;

//      DBGSTR("-I-  _PtpOpSendObject - DPS\r\n");
        pResponse->containerLength  = USB_STI_CONTAINER_SIZE + 3 * sizeof(DWORD);
        pResponse->code             = responseCode;
        return eState;
    }

#if 0 // modify later	
    if (gSessionID == 0)
    { // Check if session is open
        pResponse->code= RESPONSE_SESSION_NOT_OPEN;
        MP_DEBUG("-E- _PtpOpSendObject - Session Not Open");
    }
    else if (pCommand->TransactionID != gTransactionID+1)
    { // Check for valid transaction ID
        pResponse->code = RESPONSE_INVALID_TRANSACTION_ID;
        MP_DEBUG("-E- _PtpOpSendObject - Invalid TID");
    }
    else if ((pCommand->parameter[0] != 0) || 
             (pCommand->parameter[1] != 0) || 
             (pCommand->parameter[2] != 0) || 
             (pCommand->parameter[3] != 0) || 
             (pCommand->parameter[4] != 0))
    { // Verify unused parameters
        pResponse->code = RESPONSE_PARAMETER_NOT_SUPPORTED;
        MP_DEBUG("-E- _PtpOpSendObject - Parameter Not Supported");
    }
    else 
    { // If OK, set up for the data phase
        pResponse->code = RESPONSE_OK;

        // make sure previously open file is closed
        if (objFPtr)
        {
            mt_fclose(objFPtr);
            objFPtr = 0;
        }

        // data transfer required?
        if (objCmpSize == 0)
        {
            PtpDataSource = PTP_DATA_NONE;
            PtpDataIndex  = 0;
            PtpDataSize   = 0;
            dataType      = PTP_DATA_NONE;
        }

        // if so, setup transfer info
        else
        {
            PtpDataSource = PTP_DATA_TO_FILE;
            PtpDataIndex  = 0;
            PtpDataSize   = objCmpSize;
            dataType      = PTP_DATA_I_TO_R;

            // setup the path, append the file name,
            // make sure the file pointer is 0
            strcpy(strPath, (char*)&sDPOF_ROOT_DIR);  //error
            strncat(strPath, "\\", 2);
            strncat(strPath, (char*)strFile, 13);

            // signify activity to user
            //USBSlaveFlashLED();//Calvin
        }
    }
#endif
    //DBGSTR("-I-  _PtpOpSendObject - OK\r\n");
    return eState;
}



static TRANSACTION_STATE OperationGetObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
	WORD	responseCode = RESPONSE_OK;
	DWORD	data_size = 0;
	BYTE*	pDataIdx = (BYTE*)*hData;
	BYTE*	pDataIdxTmp = 0;
//    DWORD			i;
    
	PSTI_CONTAINER_HEAD	pDataContainerHead;
    TRANSACTION_STATE eState = STATE_PTP_DATA_IN;

    STI_CONTAINER   modifiedRequest;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
	
    //MP_DEBUG("%x %x %x",psUsbDevSidc->dwDpsCurrHdl, psUsbDevSidc->dwDpsDReqHdl, psUsbDevSidc->dwDpsDRespHdl);

    // is this a virtual DPS object impossible?
    if ((psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHReqHdl) || (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHRespHdl))
    {
        MP_ALERT("--E-- USBOTG%d %s Current Handle(%d) Error!", eWhichOtg, __FUNCTION__, psUsbDevSidc->dwDpsCurrHdl);

        if(psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsHReqHdl)  // HP Photosmart 325 Warm-boot error / Cold-boot ok
            psUsbDevSidc->dwDpsCurrHdl = psUsbDevSidc->dwDpsDReqHdl;
    }    

    // is this a virtual DPS object?
    if ((psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsDReqHdl) || (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsDRespHdl))
    {
        // default the transfer values, allocate a buffer
//		*hData = gpStiDataContainer;
		pDataContainerHead = (PSTI_CONTAINER_HEAD) (*hData);
		pDataContainerHead->containerType   = BYTE_SWAP_OF_WORD(DATA_BLOCK);
		pDataContainerHead->code            = pCommand->code;
		pDataContainerHead->transactionID   = pCommand->transactionID;
        if (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsDReqHdl)
        {
            data_size = psUsbDevSidc->dwDpsReqLen;
            psUsbDevSidc->dwDpsReqLen   = 0;
            //DBGSTR("-I-  _PtpOpGetObject - DReq\r\n");
        }
        else
        {
            data_size = psUsbDevSidc->dwDpsRespLen;
            psUsbDevSidc->dwDpsRespLen  = 0;
           /* 
            if(IsStartJob())
            {
                SetDpsState(DPS_START_JOB);
            }
            else
            {
                SetDpsState(DPS_GET_CAPABILITY);
            }
            */
            //DBGSTR("-I-  _PtpOpGetObject - DResp\r\n");
        }
		
		pDataContainerHead->containerLength = BYTE_SWAP_OF_DWORD(USB_STI_CONTAINER_SIZE + data_size);
	    *hData += USB_STI_CONTAINER_SIZE;
		
        // transfer the buffer contents
		pDataIdxTmp = (BYTE*)*hData;
		if (psUsbDevSidc->dwDpsCurrHdl == psUsbDevSidc->dwDpsDReqHdl)
		{
			memcpy(pDataIdxTmp, &psUsbDevSidc->pbDpsReqBuf[0], data_size);
		}
		else
		{      
			memcpy(pDataIdxTmp, &psUsbDevSidc->pbDpsRespBuf[0], data_size);
		}

		*hData = pDataIdx;
	    pResponse->containerLength  = USB_STI_CONTAINER_SIZE;
	    pResponse->containerType    = RESPONSE_BLOCK;
	    pResponse->code             = responseCode;
		pResponse->transactionID    = pCommand->transactionID;
        //DBGSTR("-I-  _PtpOpGetObject - DPS\r\n");
        return eState;
    }

    // construct a modified operation request
    memcpy((BYTE*)&modifiedRequest, (BYTE*)pCommand, sizeof(STI_CONTAINER));
    modifiedRequest.parameter[1] = 0;
    modifiedRequest.parameter[2] = 0xFFFFFFFF;

    // signify activity to user
    //Calvin//USBSlaveFlashLED();

    return  PtpOpGetPartialObject(hData, &modifiedRequest, pResponse, eWhichOtg);    
}

static TRANSACTION_STATE OperationGetThumb(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    STI_CONTAINER   modifiedRequest;
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);        
    DWORD dwThumbOffset =  psUsbDevSidc->sThumbnailInfo.Start; ;
    DWORD dwThumbSize = psUsbDevSidc->sThumbnailInfo.End - psUsbDevSidc->sThumbnailInfo.Start + 1;

    // construct a modified operation request
    memcpy((BYTE*)&modifiedRequest, (BYTE*)pCommand, sizeof(STI_CONTAINER));

    modifiedRequest.parameter[1] = BYTE_TO_DWORD(dwThumbOffset&0xFF, (dwThumbOffset>>8)&0xFF, (dwThumbOffset>>16)&0xFF, (dwThumbOffset>>24)&0xFF);
    modifiedRequest.parameter[2] = BYTE_TO_DWORD(dwThumbSize&0xFF, (dwThumbSize>>8)&0xFF, (dwThumbSize>>16)&0xFF, (dwThumbSize>>24)&0xFF);
    psUsbDevSidc->sdwImageSize =  dwThumbSize;  

    //-------------------------------------------------------------------------
    // Verify Thumbnail
    //-------------------------------------------------------------------------
    if(psUsbDevSidc->bIsThumbnail)
    {
        MP_ALERT("%s -No Thumbnail-", __FUNCTION__);
        pResponse->code = RESPONSE_NO_THUMBNAIL_PRESENT;
        return STATE_PTP_DATA_IN;
    }     

    return  PtpOpGetPartialObject(hData, &modifiedRequest, pResponse, eWhichOtg);    
}

static TRANSACTION_STATE OperationGetPartialObject(BYTE** hData, PSTI_CONTAINER pCommand, PSTI_CONTAINER pResponse, WHICH_OTG eWhichOtg)
{
    return  PtpOpGetPartialObject(hData, pCommand, pResponse, eWhichOtg);    
}


static TRANSACTION_STATE Pima15740CommandProcess (BYTE** hData,
                                            PSTI_CONTAINER pCommand,
                                            PSTI_CONTAINER pResponse,
                                            WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);   
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);    
    TRANSACTION_STATE   eState = STATE_IDLE;
    WORD                op_code = BYTE_SWAP_OF_WORD(pCommand->code);

    MP_DEBUG("-USBOTG%d- PIMA 0x%x", eWhichOtg, op_code);
    
    switch (op_code)
    {
        case OPERATION_GET_DEVICE_INFO:
            // MP_DEBUG("1001");
            eState = OperationGetDeviceInfo(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_OPEN_SESSION:
            // MP_DEBUG("1002");
            eState = OperationOpenSession(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_CLOSE_SESSION:
            // MP_DEBUG("1003");
            eState = OperationCloseSession(hData, pCommand, pResponse, eWhichOtg);
        break;

        //case OPERATION_GET_STORAGE_IDS:
            // MP_DEBUG("1004");
            // response_code = OperationGetStorageIDs(hData, pCommand, pResponse);
        //break;

        //case OPERATION_GET_STORAGE_INFO:
            // MP_DEBUG("1005");
            // response_code = OperationGetStorageInfo(hData, pCommand, pResponse);
        //break;

        case OPERATION_GET_NUM_OBJECTS:
            // MP_DEBUG("1006");
            eState = OperationGetNumObjects(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_GET_OBJECT_HANDLES:
            // MP_DEBUG("1007");
            eState = OperationGetObjectHandle(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_GET_OBJECT_INFO:
            // MP_DEBUG("1008");
            eState = OperationGetObjectInfo(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_GET_OBJECT:
            // MP_DEBUG("1009");
            eState = OperationGetObject(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_GET_THUMB:
            // MP_DEBUG("100A");
            eState = OperationGetThumb(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_SEND_OBJECT_INFO:
            // MP_DEBUG("100C");
            eState = OperationSendObjectInfo(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_SEND_OBJECT:
            // MP_DEBUG("100D");
            eState = OperationSendObject(hData, pCommand, pResponse, eWhichOtg);
        break;

        case OPERATION_GET_PARTIAL_OBJECT:
            //MP_DEBUG("101B");
            eState = OperationGetPartialObject(hData, pCommand, pResponse, eWhichOtg);
        break;

        default:
            #if NOT_SUPPORT_PC_PTP
            mUsbOtgUnPLGSet();
            MP_ALERT("-E- %s Should Connect with Printer, not PC", __FUNCTION__);
            EventSet(UI_EVENT, EVENT_PRINT_ERROR);
            SystemSetErrMsg(ERR_PICT_DISCONNECTED);
            psUsbDevSidc->boDpsEnabled  = FALSE;
            #else
            MP_ALERT("%s -invalid opcode- 0x%x", __FUNCTION__, op_code);
            #endif
        break;
    }

    psUsbDevSidc->dwTransactionID = BYTE_SWAP_OF_DWORD(pCommand->transactionID);
    // pWord((DWORD) eState);
    return eState;
}


//-----------------------------------------------------------------------------
//of PtpPutNextPacket
//o
//o This function will read numBytes of data from packet and process it
//o according to the currently active Data I->R transaction.
//o
//o Inputs:
//o     BYTE*  packet   - Pointer to buffer to read data from.
//o     DWORD  numBytes - Maximum number of bytes to read from packet.
//o
//o Return Value:
//o     BOOL    TRUE (more bytes needed) or FALSE (no more data expected)
//o
//-----------------------------------------------------------------------------
static BOOL PtpPutNextPacket(  BYTE* packet,
                        DWORD numBytes,
                        PSTI_CONTAINER pCommand,
                        PSTI_CONTAINER pResponse,
                        WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC	psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);    
    BYTE* 	ptr;
	WORD	opCode = BYTE_SWAP_OF_WORD(pCommand->code);
	BOOL 	ret = FALSE;
//	DWORD	len = 0;

	// default the response code to "General Error"
	//  in case the data transfer terminates early
	pResponse->code = RESPONSE_GENERAL_ERROR;

/*
	// saving to a file?
    if (gPtpDataSource == PTP_DATA_TO_FILE)
    {
		// transfer the latest data?
        if (numBytes > 0)
        {
			// file not open?
            if (objFPtr == 0)
            {
				// open the specified file
				// (if the file exists, the "w" option
				//  overwrites, otherwise it is created)
                //objFPtr = DosFileOpen(strPath, "w");
                objFPtr = mt_fopen(strPath,  "w");  //error
                if (objFPtr == 0)
                {
			        gpStiResponse->code = RESPONSE_GENERAL_ERROR;
                    MP_DEBUG1("-E- PtpPutNextPacket - Cannot Open File [%s]", strPath);
				}
			}

			// if the file is open, write the data packet
            if (objFPtr != 0)
            {
				// write the packet to the file
                //if (DosFileWrite(packet, 1, numBytes, objFPtr) != numBytes)
                if (mt_fwrite(packet, 1,numBytes,objFPtr ) != numBytes)
                {
                    mt_fclose(objFPtr);
                    objFPtr = 0;
			        gpStiResponse->code = RESPONSE_GENERAL_ERROR;
                    MP_DEBUG("-E- PtpPutNextPacket - File Write Error");
				}

				// if successful, go wait for the next packet...
                else
                {
					// signify activity to user
                    //Calvin//USBSlaveFlashLED();

					return TRUE;
				}
			}
		}

		// if done, force a close of the file & a
		// rebuild of the object handle database
        else
        {
            mt_fclose(objFPtr);
            objFPtr = 0;
            PtpBuildObjectHandles(DEVICE_STORAGE_ID, 0);
			gpStiResponse->code = RESPONSE_OK;
            //DBGSTR("-I-  PtpPutNextPacket - FC\r\n");
		}
	}
	// saving to the buffer?
    else 
*/
	if (psUsbDevSidc->bPtpDataSource == PTP_DATA_TO_BUFFER)
	{
		// default the response code & data pointer
		pResponse->code = RESPONSE_OK;
		ptr = packet;

		// last command received?
		switch (opCode)
		{
			// process the received data
			//  (container "wrapper" removed in STIBulkOut())
			case OPERATION_SEND_OBJECT_INFO:
				PtpProcessObjectInfo((PSTI_OBJECT_INFO) ptr, pResponse, eWhichOtg);
				break;

			case OPERATION_SEND_OBJECT:
				ret = PtpProcessObject(&ptr, eWhichOtg);
				break;
			/* modify later
			case OPERATION_SET_DEVICE_PROP_VALUE:
				switch (gPtpPropToSet)
				{
					case DEVICE_PROP_DATETIME:
						PtpSetDateTime(&ptr);
						break;
					default:
						gpStiResponse->code = RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
						MP_DEBUG("-E- PtpPutNextPacket - Invalid Property");
				}
				break;
			*/		
			default:
				MP_ALERT("%s -Invalid Operation- 0x%x", __FUNCTION__, opCode);
				pResponse->code = RESPONSE_OPERATION_NOT_SUPPORTED;
				break;
		}
	}

	// invalid data source
    else
    {
        pResponse->code = RESPONSE_GENERAL_ERROR;
        MP_ALERT("%s -Invalid Data Source-", __FUNCTION__);
	}

	// reset for the next command
	psUsbDevSidc->bPtpDataSource = PTP_DATA_IDLE;
//	PtpDataIndex  = 0;
//	PtpDataSize   = 0;
//	PtpPropToSet  = 0;

	// transfer completed/terminated, no more data expected
    return ret;
}


static void PtpInit(WHICH_OTG eWhichOtg)
{
    PUSB_DEVICE_SIDC psUsbDevSidc;

    psUsbDevSidc = (PUSB_DEVICE_SIDC)UsbOtgDevSidcGet(eWhichOtg);
    
    psUsbDevSidc->dwSessionID = 0;
    psUsbDevSidc->dwTransactionID = 0;
    psUsbDevSidc->dwPtpNumHandles = 0;

    DpsCapabilityInit(eWhichOtg);
    
    // general purpose info storage
	PrepareDeviceInfo(eWhichOtg);

    // data transfer/control variables

    // DPS/PictBridge info
    psUsbDevSidc->boDpsEnabled = FALSE;
    psUsbDevSidc->dwDpsCurrHdl  = 0;
    psUsbDevSidc->dwDpsDDiscHdl = 0;
    psUsbDevSidc->dwDpsDReqHdl  = 0;
    psUsbDevSidc->dwDpsDRespHdl = 0;
    psUsbDevSidc->dwDpsHDiscHdl = 0;
    psUsbDevSidc->dwDpsHReqHdl  = 0;
    psUsbDevSidc->dwDpsHRespHdl = 0;
    psUsbDevSidc->dwDpsReqLen   = 0;
    psUsbDevSidc->dwDpsRespLen  = 0;
    psUsbDevSidc->bPtpDataSource    = PTP_DATA_IDLE;
    psUsbDevSidc->boIsHostReq = FALSE;
    psUsbDevSidc->boIsDeviceReq = FALSE;
    psUsbDevSidc->boIsSendingData = FALSE;
    // build the object handle database
    PtpBuildObjectHandles(DEVICE_STORAGE_ID, 0, eWhichOtg);

// Begin:PBridge Test
    DpsConfPrintService(eWhichOtg);

	psUsbDevSidc->sTestPrintCapa.copies = 1;
	psUsbDevSidc->sTestPrintCapa.layout_border = defLayout; //0x57FF0000; 
	
	SetPrinterCapability(&psUsbDevSidc->sTestPrintCapa, eWhichOtg);

//	gDps_state_test = 0;
//    psUsbDevSidc->sDps.pbXmlBuff = (BYTE*)((DWORD)(&gaXmlBuff[0]) | 0xa0000000);
//	PictureBridgeTest_state_0();
//gaXmlBuff[255] = 0;
//	len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
//    DpWord(len);
// End:PBridge Test
}

static STREAM* FileOpenForPtp(WORD *pFormat, DWORD ObjectHandle)
{
    ST_SYSTEM_CONFIG *psSysConfig;
    ST_SEARCH_INFO *pSearchInfo; 
    psSysConfig = g_psSystemConfig;
    
    if (psSysConfig->sFileBrowser.dwImgAndMovCurIndex >= g_psSystemConfig->sFileBrowser.dwFileListCount[OP_IMAGE_MODE])  // >= psSysConfig->sFileBrowser.dwImgAndMovTotalFile)
    {
        return (STREAM*)0;
    }

#if DYNAMIC_FILE_LIST	//080917
	pSearchInfo = FileGetCurImgSearchInfo();	//Mason 20080317
#else
    pSearchInfo = &psSysConfig->sFileBrowser.sImgAndMovFileList[psSysConfig->sFileBrowser.dwImgAndMovCurIndex];
#endif

    if ((pSearchInfo->bExt[0] == 'J') && (pSearchInfo->bExt[1] == 'P') && (pSearchInfo->bExt[2] == 'G'))
    {
        *pFormat = OBJ_FORMAT_JPEG_EXIF;
    }
    else if ((pSearchInfo->bExt[0] == 'B') && (pSearchInfo->bExt[1] == 'M') && (pSearchInfo->bExt[2] == 'P'))
    {
        *pFormat = OBJ_FORMAT_BMP;
    }
    else
    {
        *pFormat = OBJ_FORMAT_JPEG_EXIF;
    }

    return FileListOpen(DriveGet(DriveCurIdGet()), pSearchInfo);
}

#if 0
void DpsTest(void)
{
//	static BOOL	done = 0;
	DWORD	len = 0;
	BOOL	ret = FALSE;
//	static int pre_state = 0;
	
/*
	if (geUsbTransactionState == STATE_PTP_CMD && gDpsEnabled && done == 0)
	{
		gDps_state_test = 1;
		done = 1;
	}

	if (gDps_state_test == 0)
	{
		return;
	}
	else if (gDps_state_test == pre_state)
	{
		return;
	}
	
//	if (gDps_state_test == 2)
//	{
//        MP_DEBUG("DpsTest 2");
//	}
	
	pre_state = gDps_state_test;
*/
	ret = PictureBridgeTest();
	if (gTest_StartJob_Process)
	{
		register GPIO *sGpio;

		sGpio = (GPIO *)(GPIO_BASE);
		while(sGpio->Gpdat1 & 0x0001);
		
		len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
		geUsbTransactionState = PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len);
	}
	else if (ret == TRUE)
 	{
		if (gTest_DResponse)
		{
			geUsbTransactionState = PtpSendDPSOpReq(0, 0);
		}
		else
		{
			len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
			geUsbTransactionState = PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len);
		}
 	}
	else
	{
		if (gTest_DResponse)
		{
			geUsbTransactionState = PtpSendDPSOpReq(0, 0);
		}		
	}
/*		
	if (gpStiCommand->code == 0x0c10)
	{
		DWORD len = 0;
		if (done == 0)
		{
			done = 1;
			MP_DEBUG("prepare send event");
			len = strlen((char *) psUsbDevSidc->sDps.pbXmlBuff);
			geUsbTransactionState = PtpSendDPSOpReq(psUsbDevSidc->sDps.pbXmlBuff, len); 
		}
	}
	else if (gpStiCommand->code == 0x0310)
	{
		done = 0;
	}
*/		
}
#endif //0
#endif   // SC_USBDEVICE

