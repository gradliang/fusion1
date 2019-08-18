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
* Filename		: usbotg_std.h
* Programmer(s)	: Joe Luo (JL) (based on Faraday's sample code)
* Created Date	: 2009/10/21 
* Description:  : put all the common data structure for host and device here
******************************************************************************** 
*/
#ifndef __USBOTG_STD_H__
#define __USBOTG_STD_H__ 
#include "utiltypedef.h"

// TEST
#define USBOTG_HOST_INTERRUPT_TX_TEST 0    

/////////////////////////////////////////////////
// USB Spec Chapter 9 Copy from Linux
/////////////////////////////////////////////////
#ifndef __LINUX_USB_CH9_H
#define USB_DIR_OUT     0x00			/* to device */
#define USB_DIR_IN      0x80		/* to host */
#endif
/*
 * USB types, the second of three bRequestType fields
 */
#define USB_TYPE_MASK       (0x03 << 5)
#define USB_TYPE_STANDARD   (0x00 << 5)
#define USB_TYPE_CLASS      (0x01 << 5)
#define USB_TYPE_VENDOR     (0x02 << 5)
#define USB_TYPE_RESERVED   (0x03 << 5)

#define USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define USB_ENDPOINT_XFER_CONTROL	0
#define USB_ENDPOINT_XFER_ISOC		1
#define USB_ENDPOINT_XFER_BULK		2
#define USB_ENDPOINT_XFER_INT		3

#define USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK		0x80

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

//OTG
#define USB_OTG_PERIPHERAL      1

//CDC
#define CANCEL_REQUEST_DATA_LENGTH          6
#define GET_DEVICE_STATUS_REQUETS_LENGTH    4

//SIDC
#define NUM_OPERATION_SUPPORTED			0x00000012 
#define NUM_EVENT_SUPPORTED				0x00000001 
#define NUM_DEVICE_PROP_CODE_SUPPORTED	0x00000003 
#define NUM_OBJECT_FORMAT_SUPPORTED		0x00000002
#define GET_OBJECT_IN_THE_ROOT		    0xFFFFFFFF

enum {
	NUM_CAPTURE_FORMAT_SUPPORTED	= NUM_OBJECT_FORMAT_SUPPORTED,
	NUM_IMAGE_FORMAT_SUPPORTED	= NUM_OBJECT_FORMAT_SUPPORTED,
//	NO_NIMAGEOBJECT				= 0x3000,
//	ASSOCIATION					= 0x3001,
//	OBJECT_FORMAT_MPEG			= 0x300B,
//	OBJECT_FORMAT_EXIF_JPEG		= 0x3801
};

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


/*
// Structure declarations
*/

typedef struct 
{
    BYTE    physicalStore;
    BYTE    logicalStore;
    WORD    objectFormat;
    DWORD   parentHandle;
} OBJECT_HEADER, *POBJECT_HEADER;


// Filesystem Type Values
enum {
//	UNDEFINED          = 0x0000,
	GENERIC_FLAT        = 0x0001,
	GENERIC_HIERACHICAL = 0x0002,
	DCF                 = 0x0003
};



//MSDC CBW and CSW for Host and Device
/////////////////////////////////////////////////
// USB Bulk Only Protocol
/////////////////////////////////////////////////
enum 
{
    SCSI_TEST_UNIT_READY                = 0x00,
    SCSI_REQUEST_SENSE                  = 0x03,
    SCSI_FORMAT_UNIT                    = 0x04,
    
    SCSI_INQUIRY                        = 0x12,
    SCSI_MODE_SENSE_6                   = 0x1A,
    SCSI_START_STOP_UNIT                = 0x1B,
    SCSI_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C,
    SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E,
    
    SCSI_READ_FORMAT_CAPACITIES         = 0x23,
    SCSI_READ_CAPACITY                  = 0x25,
    SCSI_READ_10                        = 0x28,
    SCSI_READ_12                        = 0xA8,
    SCSI_WRITE_10                       = 0x2A,
    SCSI_WRITE_12                       = 0xAA,
    SCSI_SEEK_10                        = 0x2B,
    SCSI_WRITE_AND_VERIFY_10            = 0x2E,
    
    SCSI_VERIFY_10                      = 0x2F,
    SCSI_MODE_SELECT_10                 = 0x55,
    SCSI_MODE_SENSE_10                  = 0x5A,

    SCSI_VENDOR_PROTECTION_CMD  =  0xF1,
    SCSI_VENDOR_TECO_CMD              =  0xF2,
};

typedef struct {
    BYTE   bErrcode;                    // error code                       
                                        // byte7 : valid                    
                                        //   (1 : SCSI2)                    
                                        //   (0 : Vendor specific)          
                                        // byte6-0 : error code             
                                        //  (0x70 : current error)          
                                        //  (0x71 : specific command error) 
    BYTE   bSegNo;                      // segment No.                      
    BYTE   bSenseKey;                   // byte5 : ILI                      
                                        // byte3-0 : sense key              
    BYTE   bInfo[4];                    // infomation                       
    BYTE   bAddSenseLen;                // added sense data length          
    BYTE   bCmdInfo[4];                 // command specific infomation      
    BYTE   bAsc;                        // ASC                              
    BYTE   bAscq;                       // ASCQ                             
    BYTE   bFru;                        // FRU                              
    BYTE   bSnsKeyInfo[3];              // sense key specific infomation    
    BYTE   bReserved[2];
} SENSE_DATA, *PSENSE_DATA;

#define VEN_ID_LEN              8               // Vendor ID Length         
#define PRDCT_ID_LEN            16              // Product ID Length        
#define PRDCT_REV_LEN           4               // Product LOT Length       

typedef struct {
    BYTE   bQlfrDevtype;                // Status of Logical Unit & Device Type
    BYTE   bRmb;                        // Removable or not                 
    BYTE   bStandard;                   // ISO & ECMA & ANSI                
    BYTE   bSupDatType;                 // Supported Functions &            
                                        // Response Data Type               
    BYTE   bAddDataLen;                 // Length of added data             
    BYTE   bReserved1;                  // Reserved area                    
    BYTE   bReserved2;                  // Reserved area                    
    BYTE   bSupport;                    // Supported Functions              
    BYTE   bVendorID[VEN_ID_LEN];       // Vendor ID                        
    BYTE   bPrdctID[PRDCT_ID_LEN];      // Product ID                       
    BYTE   bPrdctRev[PRDCT_REV_LEN];    // product Lot                      
} STANDARD_INQUIRY, *PSTANDARD_INQUIRY;


typedef struct {                            // Little endian(31 bytes)
    volatile DWORD  u32Signature;           // 'USBC'
    volatile DWORD  u32Tag;                 // A Command Block Tag sent by the host.
    volatile DWORD  u32DataTransferLength;  // The number of bytes of data 
    volatile BYTE   u8Flags;                // Bit0~5 : Reserved-the host shall set these bits to zero.
                                            // Bit6   : Obsolete.The host shall set this bit to zero.
                                            // Bit7   : Direction - '0'= Data-Out from host to the device.
                                            //                      '1'= Data-In from the device to the host.
    volatile BYTE   u8LUN;                  // The device Logical Unit Number to which the command block is being send.
                                            // Bits 0-3: LUN, 4-7: Reserved
    volatile BYTE	u8CBLength;             // The valid length of the CBWCB in bytes(0x01 through 0x10)
                                            // Bits 0-4: CDB Length, 5-7: Reserved
    volatile BYTE	u8CB[16];               // Command status(byte16~30)
    BYTE            bReserved;
} CBW, *PCBW; 

typedef struct {                        // Little endian(13 bytes)
    volatile DWORD  u32Signature;       // 'USBS'
    volatile DWORD  u32Tag;             // The value received in the SBWTag of the associated CBW.
    volatile DWORD  u32DataResidue;     // Data-Out : Report the difference between the amount of data expected 
                                        // Data_In  : and the amount of data processed by the device.
                                        // Normal : 0x00000000
    volatile BYTE	u8Status;           // '0x00' - Command Passed("good status")
                                        // '0x01' - Command Failed
                                        // '0x02' - Phase Error
                                        // '0x03','0x04' - Reserved(Obsolete)
                                        // '0x05~0xff' - Reserved
    BYTE            bReserved[3];
} CSW, *PCSW; 



#endif //__USBOTG_STD_H__ 

