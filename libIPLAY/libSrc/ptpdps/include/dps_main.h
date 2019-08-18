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
* Filename		: Dps_main.h
* Programmer(s)	: 
* Created Date	: 
* Description   : 
******************************************************************************** 
*/
#ifndef  _DPS_MAIN__H_
#define  _DPS_MAIN__H_

#include "UtilTypeDef.h"
#include "iplaysysconfig.h"
#include "usbotg_api.h"

//#if (SC_USBDEVICE==ENABLE)
#define DPS_BUFF_SIZE      1024//8*1024//for 100 files(XML) (DPS_BUFF_SIZE = PTP_DPS_MAX_XFR //Ptp_main.c)Calvin 2004.09.02

 
/***********************************************

           Picture Bridge UI Parameter for UI  (START)

************************************************/
#define MAX_FILE_PRINT  MAX_DEV_NUM_OF_PHOTOS
#define PrinterCapMax   34  // by #define MAX_PAPERSIZES  34  // Max 34

#if 0 // Move to usbotg_api.h
//void ui_getPrinterCapability(ePrinterCapability type, BYTE* typeVaue)
typedef enum { 
    DPS_PARAM_QUALITY,
    DPS_PARAM_PAPER_SIZE,
    DPS_PARAM_PAPER_TYPE,
    DPS_PARAM_FILE_TYPE,
    DPS_PARAM_DATE_PRINT,
    DPS_PARAM_FILENAME_PRINT,
    DPS_PARAM_IMAGE_OPTIMIZE,
    DPS_PARAM_LAYOUT,
    DPS_PARAM_FIXED_SIZE,
    DPS_PARAM_CROPPING,
    DPS_PARAM_PRINTERCAP_MAX
} ePrinterCapability;

//DPS_PARAM_QUALITY
typedef enum { 
    DPS_Quality_noQuality,
    DPS_Quality_Default,
    DPS_Quality_Normal,
    DPS_Quality_Draft,
    DPS_Quality_Fine,
    DPS_Quality_MAX
}ePrinterCapabilityQuality;

//DPS_PARAM_PAPER_SIZE,
typedef enum { 
    DPS_PaperSize_noPaperSize,
    DPS_PaperSize_defPaperSize,
	DPS_PaperSize_L,
	DPS_PaperSize_L2,
	DPS_PaperSize_Postcard,	
	DPS_PaperSize_Card,
	DPS_PaperSize_100x150, 
	DPS_PaperSize_4x6,
	DPS_PaperSize_8x10,
	DPS_PaperSize_Letter,
	DPS_PaperSize_11x17,
	//511n0000 An (n=0~9) (Note2)
	DPS_PaperSize_A0,	
	DPS_PaperSize_A1,
	DPS_PaperSize_A2, 
	DPS_PaperSize_A3,	
	DPS_PaperSize_A4,
	DPS_PaperSize_A5, 
	DPS_PaperSize_A6,	
	DPS_PaperSize_A7,
	DPS_PaperSize_A8, 	
	DPS_PaperSize_A9, 	
	//512m0000 Bm (m=0~9) (Note3)	
	DPS_PaperSize_B0,	
	DPS_PaperSize_B1,
	DPS_PaperSize_B2, 
	DPS_PaperSize_B3,	
	DPS_PaperSize_B4,
	DPS_PaperSize_B5, 
	DPS_PaperSize_B6,	
	DPS_PaperSize_B7,
	DPS_PaperSize_B8, 	
	DPS_PaperSize_B9,	
	DPS_PaperSize_Roll_L,
	DPS_PaperSize_Roll_2L, 
    DPS_PaperSize_MAX
}ePrinterCapabilityPaperSize;

//DPS_PARAM_PAPER_TYPE,
typedef enum { 
    DPS_PaperType_noPaperType,
    DPS_PaperType_defPaperType,
    DPS_PaperType_PlainPaperType,
    DPS_PaperType_PhotoPaperType,
    DPS_PaperType_FastPaperType,
    DPS_PaperType_MAX
}ePrinterCapabilityPaperType;

//DPS_PARAM_FILE_TYPE,
typedef enum { 
    DPS_FileType_noFileType,
    DPS_FileType_defFileType,
    DPS_FileType_Exit_Jpeg,
    DPS_FileType_Exit,
    DPS_FileType_JPEG,
    DPS_FileType_TIFF_EP,
    DPS_FileType_FlashPix,
    DPS_FileType_Bmp,
    DPS_FileType_CIFF,
    DPS_FileType_GIF,
    DPS_FileType_JFIF,
    DPS_FileType_PCD,
    DPS_FileType_PICT,
    DPS_FileType_PNG,
    DPS_FileType_TIFF,
    DPS_FileType_TIFF_IT,
    DPS_FileType_JP2,
    DPS_FileType_JPX,
    DPS_FileType_Undefined,
    DPS_FileType_Association,
    DPS_FileType_Script,
    DPS_FileType_Executable,
    DPS_FileType_Text,
    DPS_FileType_HTML,
    DPS_FileType_XHTML,
    DPS_FileType_DPOF,
    DPS_FileType_AIFF,
    DPS_FileType_WAV,
    DPS_FileType_MP3,
    DPS_FileType_AVI,
    DPS_FileType_MPEG,
    DPS_FileType_ASF,
    DPS_FileType_MAX
}ePrinterCapabilityFileType;

//DPS_PARAM_DATE_PRINT,
typedef enum { 
    DPS_DatePrint_noDatePrint,
    DPS_DatePrint_defDatePrint,
    DPS_DatePrint_dateOff,
    DPS_DatePrint_dateOn,
    DPS_DatePrint_MAX
}ePrinterCapabilityDatePrint;

//DPS_PARAM_FILENAME_PRINT,
typedef enum { 
    DPS_FileNamePrint_noNamePrint,
    DPS_FileNamePrint_defFNamePrt,
    DPS_FileNamePrint_filePrtOff,
    DPS_FileNamePrint_filePrtON,
    DPS_FileNamePrint_MAX
}ePrinterCapabilityFileNamePrint;

//DPS_PARAM_IMAGE_OPTIMIZE,
typedef enum { 
    DPS_ImageOptimize_noImageOpt,
    DPS_ImageOptimize_defImgOpt,
    DPS_ImageOptimize_optOff,
    DPS_ImageOptimize_optOn, 
    DPS_ImageOptimize_MAX
}ePrinterCapabilityImageOptimize;

//DPS_PARAM_LAYOUT,
typedef enum { 
    DPS_Layout_noLayout,
    DPS_Layout_defLayout,
    DPS_Layout_oneUp,
    DPS_Layout_twoUp,
    DPS_Layout_threeUp,
    DPS_Layout_fourUp,
    DPS_Layout_fiveUp,
    DPS_Layout_sixUp,
    DPS_Layout_sevenUp,
    DPS_Layout_eightUp,
    DPS_Layout_indexLayout,    /* Index Print */
    DPS_Layout_oneFullBleed,   /* Borderless */
    DPS_Layout_MAX
}ePrinterCapabilityLayout;

//DPS_PARAM_FIXED_SIZE,
typedef enum { 
    DPS_FixedSize_noFixedSize,
    DPS_FixedSize_defFixedSize,
    DPS_FixedSize_size2x3,
    DPS_FixedSize_size3x5,
    DPS_FixedSize_size4x6,
    DPS_FixedSize_size5x7,
    DPS_FixedSize_size8x10,  
    DPS_FixedSize_MAX
}ePrinterCapabilityFixedSize;

//DPS_PARAM_CROPPING
typedef enum { 
    DPS_Cropping_noCropping,
    DPS_Cropping_defCropping,
    DPS_Cropping_cropOff,
    DPS_Cropping_cropOn,
    DPS_Cropping_MAX
}ePrinterCapabilityCropping;
#endif


//void ui_AddImage( )
typedef struct{
    DWORD   FileID;
    DWORD   Copies;
    BOOL    DatePrint;
    BOOL    FileNamePrint;
    BOOL    Crop;
    DWORD   CropX;
    DWORD   CropY;
    DWORD   CropW;
    DWORD   CropH;    
} PrintJobSetting;

#if 0 // Move to usbotg_api.h
//void ui_printAction(ePrintAction act)
typedef enum { 
    DPS_PARAM_START_JOB,
    DPS_PARAM_ABORT_JOB,
    DPS_PARAM_CONTINUE_JOB,
    DPS_PARAM_INIT_JOB,    /* Not in DPS */
    DPS_PARAM_PRINTACTION_MAX    
} ePrintAction;

//void ui_GetDeviceStatus(DWORD* DeviceStatus)
typedef enum { 
    DPS_DevStat_PrintStatus,
    DPS_DevStat_JobEnd,
    DPS_DevStat_ErrStatus,    
    DPS_DevStat_ErrReason,
    DPS_DevStat_DisconEn,
    DPS_DevStat_CapabilityChg,       
    DPS_DevStat_NewJobOK,  
    DPS_DevStat_MAX    
} eDeviceStatus;
#endif
#if 0 //unused JL, 20091209
typedef enum
{
    FAILED,
    COMPLETED,
    PROGRESSING
} PrinterStatus;
#endif
#if 0 // Move to usbotg_api.h
//dpsPrintServiceStatus
typedef enum
{
    DPS_PrintStatus_Printing,
    DPS_PrintStatus_Idle,
    DPS_PrintStatus_Paused,
    DPS_PrintStatus_MAX
} PrintingStatus;

//Job End
typedef enum { 
    DPS_JobEnd_NotEnded,
    DPS_JobEnd_Ended,
    DPS_JobEnd_AbortJob_Immediate,
    DPS_JobEnd_AbortJob_NoImmediate,
    DPS_JobEnd_OtherReason,	
    DPS_JobEnd_MAX        
} JobEnd;

//errorStatus 
typedef enum { 
    DPS_ErrStatus_NoError,
    DPS_ErrStatus_Warning,
    DPS_ErrStatus_FatalError,
    DPS_ErrStatus_MAX     
} ErrorStatus;

//errorReason
typedef enum { 
    DPS_ErrReason_NoReason,
    DPS_ErrReason_PaperError,
    DPS_ErrReason_InkError,
    DPS_ErrReason_HWError,
    DPS_ErrReason_FileError,
    DPS_ErrReason_MAX       
} ErrorReason;

//disconnectEnable
typedef enum { 
    DPS_DisconEn_FALSE,
    DPS_DisconEn_TRUE,
    DPS_DisconEn_MAX    
} eDisconEn;

//capabilityChanged
typedef enum { 
    DPS_CapabilityChg_FALSE,
    DPS_CapabilityChg_TRUE,
    DPS_CapabilityChg_MAX    
} eCapabilityChg;

//newJobOK
typedef enum { 
    DPS_newJobOK_FALSE,
    DPS_newJobOK_TRUE,
    DPS_newJobOK_MAX    
} eNewJobOK;
#endif

// variable holds the type of last data sent
typedef enum
{
	OP_CONF_PRINT_SERVICE,
	OP_GET_CAPABILITY,
	OP_GET_JOB_STATUS,
	OP_GET_DEV_STATUS,
	OP_START_JOB,
	OP_ABORT_JOB,
	OP_CONTINUE_JOB
} LastOpType;


typedef enum
{
	OPSTATUS_SCHEDULED,			// when there is a an operation waiting to be sent by xml
	OPSTATUS_NO_SCHEDULED,		// Nothing is scheduled to be sent
	OPSTATUS_WAITING_RESP,		// operation was sent and waiting response
	OPSTATUS_GOT_RESP			// Got response ... it waits till the sending mechanisms ack the response
} LastOpStatus;

/***********************************************

           Picture Bridge UI Function for UI  (END)

************************************************/

typedef struct
{
   unsigned paperDefault: 1;
   unsigned paperLetter: 1;
   unsigned paperCard: 1;
   unsigned paper4x6: 1;
   unsigned paper8x10: 1;
   unsigned paperA4: 1;
   unsigned paperA6: 1;

} PaperSizeType;


typedef struct
{
   unsigned fixedDefault: 1;

   unsigned print2x3: 1;
   unsigned print3x5: 1;
   unsigned print4x6: 1;
   unsigned print5x7: 1;
   unsigned print8x10: 1;
} FixedPrintSizeType;

typedef struct
{
   unsigned printDefault:1;

   unsigned print1Up: 1;
   unsigned print2Up: 1;
   unsigned print3Up: 1;
   unsigned print4Up: 1;
   unsigned print5Up: 1;
   unsigned print6Up: 1;
   unsigned print7Up: 1;
   unsigned print8Up: 1;
   unsigned printIndex: 1;
   unsigned print1FullBleed: 1;
} PrintLayoutType;

typedef struct
{
   unsigned typeDefault: 1;
   unsigned typePlain: 1;
   unsigned typePhoto: 1;
} PaperTypeType;

typedef struct
{
   unsigned qualityDefault: 1;
   unsigned qualityNormal: 1;
   unsigned qualityDraft: 1;
   unsigned qualityFine: 1;
} QualityType;

//Calvin 2004.08.05
typedef struct
{
   unsigned datePrintDefault: 1;
   unsigned datePrintOff: 1;
   unsigned datePrintOn: 1;
} DatePrintType;



typedef struct
{
	PaperSizeType		paperSize;

	FixedPrintSizeType		fixedPrintSize;

	PrintLayoutType		printLayoutForDefault;
	PrintLayoutType		printLayoutForLetter;
	PrintLayoutType		printLayoutForCard;
	PrintLayoutType		printLayoutFor4x6;
	PrintLayoutType		printLayoutFor8x10;
	PrintLayoutType		printLayoutForA4;
	PrintLayoutType		printLayoutForA6;

	PaperTypeType		paperTypes;

	QualityType			quality;
	
	DatePrintType			datePrint; //Calvin 2004.08.05
} DpsPrinterSettings;

#if SC_USBDEVICE
typedef struct
{
    eJobEnd          jobEnded;
    ePrintingStatus  status;         // 0: idle, 1: printing, 2: paused
    eErrorStatus     error;          // 0: no error, 1: warning, 2: fatal error
    eErrorReason     errorReason;
    BOOL             sessionStatus;  // 0: disconnected, 1: connected
} DpsPrinterStatus;
#endif

typedef struct
{
    BOOL    isValid;    // FALSE if the data unreliable
    WORD    currImage;
    WORD    totalImage;   //  progCurrent / progTotal

} DpsPrintJobStatus;




/***********************************************

           Picture Bridge UI Parameter for UI  (START)

************************************************/
// have put in usbotg_api.h

/***********************************************

           Picture Bridge UI Function for UI  (END)

************************************************/
#if SC_USBDEVICE

void ClearNewJobOk(WHICH_OTG eWhichOtg);
void DpsConfPrintService(WHICH_OTG eWhichOtg);
void DpsGetCapability(WHICH_OTG eWhichOtg);
void DpsGetDeviceStatus(WHICH_OTG eWhichOtg);
void DpsCapabilityInit(WHICH_OTG eWhichOtg);
BOOL DpsStartJob(WHICH_OTG eWhichOtg);
BOOL DPSPostResponse(BYTE * buf_in, DWORD len, WHICH_OTG eWhichOtg);
BOOL DpsAbortJob(WHICH_OTG eWhichOtg);
BOOL DpsContinueJob(WHICH_OTG eWhichOtg);
DWORD DPSPostEvent(BYTE * buf_in, BYTE * buf_out, WHICH_OTG eWhichOtg);
void DpsAllocPrintInfo(DWORD dwCnt, WHICH_OTG eWhichOtg);
void DpsPrintInfoFree(WHICH_OTG eWhichOtg);
void DpsPrintInfoAlloc(DWORD dwCnt, WHICH_OTG eWhichOtg);

#endif

//#endif // SC_USBDEVICE

#endif  //	_DPS_MAIN__H_

