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
* Filename		: usbotg_host_msdc.h
* Programmer(s)	: Joe Luo (JL)
* Created Date	: 2008/04/30 
* Description	: 
******************************************************************************** 
*/
#ifndef __USBOTG_HOST_MSDC_H__
#define __USBOTG_HOST_MSDC_H__
/*
// Include section 
*/
#include "utiltypedef.h"
#include "usbotg_api.h"
//#include "utilregfile.h" //temp_remove

//#if (SC_USBHOST==ENABLE)

/*
// Constant declarations
*/
#define COMMAND_BLOCK_WRAPPER_SIGNATURE	0x55534243//'USBC'
#define BYTE_COUNT_OF_CBW                   31
#define BYTE_COUNT_OF_CSW                   13

#define BYTE_COUNT_OF_SENSE_DATA            18
#define BYTE_COUNT_OF_INQUIRY_DATA          36
#define BYTE_COUNT_OF_FORMAT_CAPACITY_LEN   252//12//
#define BYTE_COUNT_OF_READ_CAPACITY_LEN     8
#define BYTE_COUNT_OF_MODE_SENSE_6_LEN      0xC0//12 //4
#define BYTE_COUNT_OF_MODE_SENSE_10_LEN     12
#define BYTE_COUNT_OF_MODE_SENSE_10_LEN_CBI  8


#define CBI_INTERRUPT_DATA_BLOCK_LEN 0x2

/*
#define VEN_ID_LEN              8               // Vendor ID Length         
#define PRDCT_ID_LEN            16              // Product ID Length        
#define PRDCT_REV_LEN           4               // Product LOT Length       
*/
#define MSDC_DEVICE_WRITE_PROTECT            0x80

#define PARSE_NO_ERROR  0
#define PARSE_ERROR     -1
#define PARSE_ERROR_DEVICE_NOT_SUPPORT     -2

#define CBI_CMD_BYTE_CNT    12
/*
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
};
*/
// Data transfer flags for CommandTaskPB
enum
{
    CBW_FLAGS_DATA_OUT  = 0x00,
    CBW_FLAGS_DATA_IN   = 0x80,
    CBW_FLAGS_NO_DATA   = 0x00,
};

enum
{
    CSW_COMMAND_PASSED	= 0x00,
    CSW_COMMAND_FAILED	= 0x01,
    CSW_PHASE_ERROR	    = 0x02,
};

enum
{
    UNFORMATED_MEDIA	= 1,
    FORMATED_MEDIA	    = 2,
    NO_CARTRIDGE_IN_DRIVE	= 3,
};


//
// Recommanded Sense Keys, ASC and ASCQ for All Commands Errors
//
enum
{
    NO_SENSE           = 0x0,
    RECOVERED_ERROR    = 0x1,
    NOT_READY          = 0x2,
    MEDIUM_ERROR       = 0x3,
    HARDWARE_ERROR     = 0x4,
    ILLEGAL_REQUEST    = 0x5,
    UNIT_ATTENTION     = 0x6,
    DATA_PROTECT       = 0x7,
    BLANK_CHECK        = 0x8,
    ABORTED_COMMAND    = 0xB,
    VOLUME_OVERFLOW    = 0xC,
    MISCOMPARE         = 0xD,
};

enum
{
    USBH_NO_SENSE                                    = 0,
    RECOVERED_DATA_WITH_RETRIES                 = 0x011701,
    RECOVERED_DATA_WITH_ECC                     = 0x011800,
    MEDIUM_NOT_PRESENT                          = 0x023A00,
    LOGICAL_DRIVE_NOT_READY_BECOMING_READY      = 0x020401,
    LOGICAL_DRIVE_NOT_READY_FORMAT_IN_PROGRESS  = 0x020404,
    NO_REFERENCE_POSITION_FOUND                 = 0x020600,
    NO_SEEK_COMPLETE                            = 0x030200,
    WRITE_FAULT                                 = 0x030300,
    ID_CRC_ERROR                                = 0x031000,
    UNRECOVERED_READ_ERROR                      = 0x031100,
    ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD         = 0x031200,
    RECORDED_ENTITY_NOT_FOUND                   = 0x031400,
    INCOMPATIBLE_MEDIUM_INSTALLED               = 0x033000,
    CANNOT_READ_MEDIUM_INCOMPATIBLE_FORMAT      = 0x033002,
    CANNOT_READ_MEDIUM_UNKNOWN_FORMAT           = 0x033001,
    FORMAT_COMMAND_FAILED                       = 0x033101,
    INVALID_COMMAND_OPERATION_CODE              = 0x052000,
    LOGICAL_BLOCK_DDRESS_OUT_OF_RANGE           = 0x052100,
    INVALID_FIELD_IN_COMMAND_PACKET             = 0x052400,
    LOGICAL_UNIT_NOT_SUPPORTED                  = 0x052500,
    INVALID_FIELD_IN_PARAMETER_LIST             = 0x052600,
    MEDIUM_REMOVAL_PREVENTED                    = 0x055302,
    NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED = 0x062800,
    POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED = 0x062900,
    WRITE_PROTECTED_MEDIA                       = 0x072700,
    OVERLAPPED_COMMAND_ATTEMPTED                = 0x0B4E00,
};

enum
{
    //MEDIA_TYPE_AND_WRITE_PROTECT = 0x00,
    READ_WRITE_ERROR_RECOVERY_PAGE          = 0x01,
    FLEXIBLE_DISK_PAGE                      = 0x05,
    REMOVABLE_BLOCK_ACCESS_CAPACITIES_PAGE  = 0x1B,
    TIMER_AND_PROTECT_PAGE                  = 0x1C,
    RETURN_ALL_PAGES                        = 0x3F,
};

/*
// Structure declarations
*/
#if 0
typedef struct {
    BYTE   bErrcode;                    /* error code                       */
                                        /* byte7 : valid                    */
                                        /*   (1 : SCSI2)                    */
                                        /*   (0 : Vendor specific)          */
                                        /* byte6-0 : error code             */
                                        /*  (0x70 : current error)          */
                                        /*  (0x71 : specific command error) */
    BYTE   bSegNo;                      /* segment No.                      */
    BYTE   bSenseKey;                   /* byte5 : ILI                      */
                                        /* byte3-0 : sense key              */
    BYTE   bInfo[4];                    /* infomation                       */
    BYTE   bAddSenseLen;                /* added sense data length          */
    BYTE   bCmdInfo[4];                 /* command specific infomation      */
    BYTE   bAsc;                        /* ASC                              */
    BYTE   bAscq;                       /* ASCQ                             */
    BYTE   bFru;                        /* FRU                              */
    BYTE   bSnsKeyInfo[3];              /* sense key specific infomation    */
    BYTE   bReserved[2];
} SENSE_DATA, *PSENSE_DATA;

/*---- standard Inquiry data ----*/
typedef struct {
    BYTE   bQlfrDevtype;                /* Status of Logical Unit &         */
                                        /*                      Device Type */
    BYTE   bRmb;                        /* Removable or not                 */
    BYTE   bStandard;                   /* ISO & ECMA & ANSI                */
    BYTE   bSupDatType;                 /* Supported Functions &            */
                                        /* Response Data Type               */
    BYTE   bAddDataLen;                 /* Length of added data             */
    BYTE   bReserved1;                  /* Reserved area                    */
    BYTE   bReserved2;                  /* Reserved area                    */
    BYTE   bSupport;                    /* Supported Functions              */
    BYTE   bVendorID[VEN_ID_LEN];       /* Vendor ID                        */
    BYTE   bPrdctID[PRDCT_ID_LEN];      /* Product ID                       */
    BYTE   bPrdctRev[PRDCT_REV_LEN];    /* product Lot                      */
} STANDARD_INQUIRY, *PSTANDARD_INQUIRY;
#endif


typedef struct {
    BYTE    bReserved[3] ;
    BYTE    bCapacityListLength ;
    DWORD   dLastLBA;       // Last logical block address
    BYTE    bDescriptorCode;
    BYTE    bBlockLength[3];    // Block length in bytes(MSB first for command return )
} FORMAT_CAPACITY_DES, *PFORMAT_CAPACITY_DES ;     

typedef struct {
    DWORD   dLastLogicBlockAddr;    // Last logical block address
    DWORD   dBlockLenInByte;        // Block length in bytes(MSB first for command return )
} READ_CAPACITY, *PREAD_CAPACITY ;	     

// Mode Parameter Header
typedef struct {
    BYTE   bModeParaLen;                // Mode parameter length     
    BYTE   bMediaType;                  // Medium type               
    BYTE   bDevPara;                    // Device specific parameter 
    BYTE   bBlkDscrptLen;               // Block descriptor length   
} MODE_SENSE_6_HDR, *PMODE_SENSE_6_HDR;


typedef struct {
    BYTE   bModeParaLen[2];             // Mode parameter length
    BYTE   bMediaType;                  // Medium type
    BYTE   bDevPara;                    // Device specific parameter
    BYTE   bReserved[2];                // Rserved area
    BYTE   bBlkDscrptLen[2];            // Block descriptor length
} MODE_SENSE_10_HDR, *PMODE_SENSE_10_HDR;



/*
// Function prototype 
*/
void OTGH_PT_Bulk_Init(WHICH_OTG eWhichOtg);
void OTGH_PT_Bulk_Close(WHICH_OTG eWhichOtg);
void UsbOtgHostBulkOnlyActive (WHICH_OTG eWhichOtg);
void UsbOtgHostBulkOnlyIoc (WHICH_OTG eWhichOtg);
void UsbOtgHostCbNoIntActive (WHICH_OTG eWhichOtg);
void UsbOtgHostCbNoIntIoc(WHICH_OTG eWhichOtg);
void UsbOtgHostCbWithIntActive (WHICH_OTG eWhichOtg);
void UsbOtgHostCbWithIntIoc(WHICH_OTG eWhichOtg);
void UsbOtgHostBulkOnlySetupIoc (WHICH_OTG eWhichOtg);
void UsbOtgHostCbNoIntSetupIoc (WHICH_OTG eWhichOtg);
void UsbOtgHostCbWithIntSetupIoc (WHICH_OTG eWhichOtg);
SWORD ProcessInquiryData(PSTANDARD_INQUIRY pInquiryBuffer);
SWORD RequestSenseDataProcess (WHICH_OTG eWhichOtg);
SWORD UsbhMsdcCommand( BYTE opCode, WHICH_OTG eWhichOtg);
SWORD UsbhMsdcReadData (    BYTE                        bLun,
                            DWORD                       dwLba,
                            DWORD                       dwCount,
                            DWORD                       dwBuffer,
                            WHICH_OTG                   eWhichOtg);
SWORD UsbhMsdcWriteData(    BYTE                        bLun,
                            DWORD                       dwLba,
                            DWORD                       dwCount,
                            DWORD                       dwBuffer,
                            WHICH_OTG                   eWhichOtg);
#if (USBOTG_HOST_TEST || Make_TESTCONSOLE)
SWORD UsbOtgHostTestFunction (  BYTE                        bLun,
                                DWORD                       dwLba,
                                DWORD                       dwCount,
                                DWORD                       dwBuffer,
                                WHICH_OTG                   eWhichOtg);
#endif
DWORD UsbhCheckReqSenseData(BYTE senseKey, BYTE asc, BYTE ascq);

extern BOOL CheckIfPanasonicDMC_FX8 (WORD wVid, WORD wPid);

//#endif // SC_USBHOST

#endif /* End of __USBOTG_HOST_MSDC_H__ */




