
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "global612.h"
#include "mpTrace.h"
#include "dps_main.h"
#include "dps_common.h" 
#include "taskid.h"
#include "ui.h"
//#include "dps_api.h"

#include <string.h>

#if USBOTG_DEVICE_SIDC
//#define MAX_FILE_TO_PRINT   30
#define DPS_TIMEOUT                 3000	// 30 seconds
#define DPS_DEVICE_STATUS_EXPIRY    1000	// 10 seconds
#define DPS_JOB_STATUS_EXPIRY       3000	// 90 seconds

//static BOOL   gSendFlag = TRUE;

// TOTDO: Fix header
//DWORD PtpGetFileID(char *filePath);

/*
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
*/


/*
void SetOpStatusNoScheduled(void)
{
	psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
}

void SetOpStatusWaitingResp(void)
{
	psDps->stDscStatus.opStatus = OPSTATUS_WAITING_RESP;
}
*/

/***********************************************

           PictBridge UI Function for UI  (START)

************************************************/


/*        User Value & DPS Value Mapping Table   */
Quality tabPrinterCapabilityQuality[DPS_Quality_MAX] =
{
    noQuality	,
    defQuality, 
    normalQuality,
    draftQuality, 
    fineQuality
};

PaperSize tabPrinterCapabilityPaperSize[DPS_PaperSize_MAX] =
{
	noPaperSize,
	defPaperSize,
	paperSizeL,
	paperSizeL2,
	paperSizePostcard,	
	paperSizeCard,
	paperSize100x150, 
	paperSize4x6,
	paperSize8x10,
	paperSizeLetter,
	paperSize11x17,
	//511n0000 An (n=0~9) (Note2)
	paperSizeA0,	
	paperSizeA1,
	paperSizeA2, 
	paperSizeA3,	
	paperSizeA4,
	paperSizeA5, 
	paperSizeA6,	
	paperSizeA7,
	paperSizeA8, 	
	paperSizeA9, 	
	//512m0000 Bm (m=0~9) (Note3)	
	paperSizeB0,	
	paperSizeB1,
	paperSizeB2, 
	paperSizeB3,	
	paperSizeB4,
	paperSizeB5, 
	paperSizeB6,	
	paperSizeB7,
	paperSizeB8, 	
	paperSizeB9,	
	paperSizeRoll_L,
	paperSizeRoll_2L
};

PaperType tabPrinterCapabilityPaperType[DPS_PaperType_MAX] =
{
    noPaperType,
    defPaperType,
    PlainPaperType,
    PhotoPaperType,
    FastPaperType
};

FileType tabPrinterCapabilityFileType[DPS_FileType_MAX] =
{
    noFileType,
    defFileType,
    fileTypeExit_Jpeg,
    fileTypeExit,
    fileTypeJPEG,
    fileTypeTIFF_EP,
    fileTypeFlashPix,
    fileTypeBmp,
    fileTypeCIFF,
    fileTypeGIF,
    fileTypeJFIF,
    fileTypePCD,
    fileTypePICT,
    fileTypePNG,
    fileTypeTIFF,
    fileTypeTIFF_IT,
    fileTypeJP2,
    fileTypeJPX,
    fileTypeUndefined,
    fileTypeAssociation,
    fileTypeScript,
    fileTypeExecutable,
    fileTypeText,
    fileTypeHTML,
    fileTypeXHTML,
    fileTypeDPOF,
    fileTypeAIFF,
    fileTypeWAV,
    fileTypeMP3,
    fileTypeAVI,
    fileTypeMPEG,
    fileTypeASF
};

DatePrint tabPrinterCapabilityDatePrint[DPS_DatePrint_MAX] =
{
	noDatePrint,
	defDatePrint,
	dateOff,
	dateOn
};

FileNamePrint tabPrinterCapabilityFileNamePrint[DPS_FileNamePrint_MAX] =
{
	noNamePrint,
	defFNamePrt,
	filePrtOff,
	filePrtON
};

ImageOptimize tabPrinterCapabilityImageOptimize[DPS_ImageOptimize_MAX] =
{
	noImageOpt,
	defImgOpt,
	optOff,
	optOn 
};

Layout tabPrinterCapabilityLayout[DPS_Layout_MAX] =
{
	noLayout,
	defLayout,
	oneUp,
	twoUp,
	threeUp,
	fourUp,
	fiveUp,
	sixUp,
	sevenUp,
	eightUp,
	indexLayout,  // Index Print
	oneFullBleed  // Full-Borderless
};

FixedSize tabPrinterCapabilityFixedSize[DPS_FixedSize_MAX] =
{
	noFixedSize,
	defFixedSize,
	size2x3,
	size3x5,
	size4x6,
	size5x7,
	size8x10 
};

Cropping tabPrinterCapabilityCropping[DPS_Cropping_MAX] =
{
	noCropping,
	defCropping,
	cropOff,
	cropOn
};

ePrintAction tabPrinterPrintAction[DPS_PARAM_PRINTACTION_MAX] =
{
	DPS_START_JOB,
	DPS_ABORT_JOB,
	DPS_CONTINUE_JOB
	//,DPS_INIT_JOB    /* Not in DPS */
};

//dpsPrintServiceStatus
enPrintStatus tabPrintStatus[DPS_PrintStatus_MAX] =
{
    PrintStatusPrinting, 
    PrintStatusIdle,
    PrintStatusPaused 
} ;

//jobEndReason
enJobEnd tabJobEnd[DPS_JobEnd_MAX] =
{
    JobEndNotEnded,
    JobEndEnded,
    JobEndAbortJobImmediate,
    JobEndAbortJobNoImmediate,
    JobEndOtherReason   
} ;

//errorStatus 
enErrStatus tabErrStatus[DPS_ErrStatus_MAX] =
{
    ErrStatusNoError,
    ErrStatusWarning,    /* recoverable error */
    ErrStatusFatalError  /* unrecoverable error */
} ;

//errorReason
enErrReason tabErrReason[DPS_ErrReason_MAX] = 
{
    ErrReasonNoReason,
    ErrReasonPaperError,
    ErrReasonInkError,
    ErrReasonHWError,
    ErrReasonFileError
} ;

//disconnectEnable
enDisconEn tabDisconEn[DPS_DisconEn_MAX] =
{
    DisconEnFALSE,
    DisconEnTRUE 
} ;

//capabilityChanged
enCapabilityChg tabCapabilityChg[DPS_CapabilityChg_MAX] =
{
    CapabilityChgFALSE,
    CapabilityChgTRUE
} ;

//newJobOK
enNewJobOK tabNewJobOK[DPS_newJobOK_MAX] =
{
    newJobOKFALSE,
    newJobOKTRUE
} ;

/*
// Static function prototype
*/
static BYTE GetDpsParam(DWORD* tabPrinterCapability, DWORD Count, DWORD index);
static void DpsInitJob(WHICH_OTG eWhichOtg);  // Clean setting before printing
static BOOL IsJobStarted(WHICH_OTG eWhichOtg);
static BOOL IsJobFinished(WHICH_OTG eWhichOtg);
static BOOL _DpsSendOperation(BYTE * buf_out, WHICH_OTG eWhichOtg);
static DWORD CheckDpsError(DpsPrinterStatus dpsPrinterStatus, WHICH_OTG eWhichOtg);
static void _DpsFillPrinterSettings(DpsPrinterSettings * printSettings,
									DpsPrinterCapability * capabilities);
static void DpsGetPrinterStatus(DpsPrinterStatus * pDpsPrinterStatus, WHICH_OTG eWhichOtg);
static void DpsGetJobStatus(DpsPrintJobStatus * pDpsPrintJobStatus);

/*
// Definition of internal functions
*/
BYTE* Api_UsbdDpsGetPrinterName(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    

    return psDps->stPrinterStatus.hostConfPrintService.productName;
}

void Api_UsbdDpsGetPrinterCapability(ePrinterCapability type, BYTE* typeVaue, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	int i,j,m,k,rep;

    MP_DEBUG("-USBOTG%d %s Type:%d", eWhichOtg, __FUNCTION__, type);
    
	switch (type)
	{
            	case DPS_PARAM_QUALITY:
                    typeVaue[0] = psDps->stDpsPrinterCapability.qualitiesCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.qualitiesCount; i++)
                    {      
		                typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityQuality, DPS_Quality_MAX, psDps->stDpsPrinterCapability.qualities[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("QUALITY ERR: Out of range");
                        MP_DEBUG("QUALITY %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.qualities[i-1]);
                    }
            		break;

            	case DPS_PARAM_PAPER_SIZE:
                    typeVaue[0] = psDps->stDpsPrinterCapability.paperSizesCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.paperSizesCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityPaperSize, DPS_PaperSize_MAX, psDps->stDpsPrinterCapability.paperSizes[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("PAPER_SIZE ERR: Out of range");
                        MP_DEBUG("PAPER_SIZE %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.paperSizes[i-1]);
            		}
            		break;

            	case DPS_PARAM_PAPER_TYPE:
                    //Pre-set Only one PaperSize
                    // clear the minor code and clear duplicated master code
        			m = 0;
                    psDps->stDpsPrinterCapability.paperTypes[0][m] = 0;
        			for (j = 0; j < psDps->stDpsPrinterCapability.paperTypesCount[0]; j++)
        			{
                        rep = TRUE;
                        for(k = 0; k <= m; k++)
                        {          
                            if(psDps->stDpsPrinterCapability.paperTypes[0][k] == (psDps->stDpsPrinterCapability.paperTypes[0][j] & 0xFFFF0000U))
                            {
                                rep = FALSE;
                                break;
                            }
                        }
                        if(rep == TRUE)
                        {
                            psDps->stDpsPrinterCapability.paperTypes[0][m] = (psDps->stDpsPrinterCapability.paperTypes[0][j] & 0xFFFF0000U);
                            m++;
                        }  
        			}

                    typeVaue[0] = m;

                    MP_DEBUG("cnt %d",m);

            		for (i = 1; i <= m; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityPaperType, DPS_PaperType_MAX, psDps->stDpsPrinterCapability.paperTypes[0][i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("PAPER_TYPE ERR: Out of range");
                        MP_DEBUG("PAPER_TYPE %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.paperTypes[0][i-1]);
            		}
            		break;

            	case DPS_PARAM_FILE_TYPE:
                    typeVaue[0] = psDps->stDpsPrinterCapability.fileTypesCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.fileTypesCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityFileType, DPS_FileType_MAX, psDps->stDpsPrinterCapability.fileTypes[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("FILE_TYPE ERR: Out of range");
                        MP_DEBUG("FILE_TYPE %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.fileTypes[i-1]);
            		}
            		break;


            	case DPS_PARAM_DATE_PRINT:
                    typeVaue[0] = psDps->stDpsPrinterCapability.datePrintsCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.datePrintsCount; i++)
            		{
        				typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityDatePrint, DPS_DatePrint_MAX, psDps->stDpsPrinterCapability.datePrints[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("DATE_PRINT ERR: Out of range");
                        MP_DEBUG("DATE_PRINT %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.datePrints[i-1]);
            		}
            		break;


            	case DPS_PARAM_FILENAME_PRINT:
                    typeVaue[0] = psDps->stDpsPrinterCapability.fileNamePrintsCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.fileNamePrintsCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityFileNamePrint, DPS_FileNamePrint_MAX, psDps->stDpsPrinterCapability.fileNamePrints[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("FILENAME_PRINT ERR: Out of range");
                        MP_DEBUG("FILENAME_PRINT %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.fileNamePrints[i-1]);
            		}
            		break;


            	case DPS_PARAM_IMAGE_OPTIMIZE:
                    typeVaue[0] = psDps->stDpsPrinterCapability.imageOptimizesCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.imageOptimizesCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityImageOptimize, DPS_ImageOptimize_MAX, psDps->stDpsPrinterCapability.imageOptimizes[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("IMAGE_OPTIMIZE ERR: Out of range");
                        MP_DEBUG("IMAGE_OPTIMIZE %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.imageOptimizes[i-1]);
            		}
            		break;

            	case DPS_PARAM_LAYOUT:
			        //Pre-set Only one PaperSize
                    typeVaue[0] = psDps->stDpsPrinterCapability.layoutsCount[0];
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.layoutsCount[0]; i++)
            		{	
                        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityLayout, DPS_Layout_MAX, psDps->stDpsPrinterCapability.layouts[0][i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("LAYOUT ERR: Out of range");
            			MP_DEBUG("LAYOUT %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.layouts[0][i-1]);
					}
					break;

            	case DPS_PARAM_FIXED_SIZE:
                    typeVaue[0] = psDps->stDpsPrinterCapability.fixedSizesCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.fixedSizesCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityFixedSize, DPS_FixedSize_MAX, psDps->stDpsPrinterCapability.fixedSizes[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("FIXED_SIZE ERR: Out of range");
                        MP_DEBUG("FIXED_SIZE %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.fixedSizes[i-1]);
            		}
            		break;

            	case DPS_PARAM_CROPPING:
                    typeVaue[0] = psDps->stDpsPrinterCapability.croppingsCount;
		
            		for (i = 1; i <= psDps->stDpsPrinterCapability.croppingsCount; i++)
            		{
				        typeVaue[i] =GetDpsParam((DWORD*)tabPrinterCapabilityCropping, DPS_Cropping_MAX, psDps->stDpsPrinterCapability.croppings[i-1]);
                        if(typeVaue[i] == 0xFF) MP_DEBUG("CROPPING ERR: Out of range");
                        MP_DEBUG("CROPPING %d/%d %d %x", i, typeVaue[0], typeVaue[i], psDps->stDpsPrinterCapability.croppings[i-1]);
            		}
            		break;
	}
}

/*   FileID follow File-Index start from 0 */
void Api_UsbdDpsAddImage(DWORD FileID, DWORD Copies, BOOL bDatePrint, BOOL bFileNamePrint, DWORD CropX, DWORD CropY, DWORD CropW, DWORD CropH, WHICH_OTG eWhichOtg)
{	
    PPICT_BRIDGE_DPS psDps      = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);

    MP_DEBUG("-USBOTG%d %s ID %d Copies %d", eWhichOtg, __FUNCTION__, FileID, Copies);
    
    if(FileID > MAX_FILE_PRINT  && PtpGetIsOneHandle(eWhichOtg) == FALSE)
    {
        MP_ALERT("%s over %d pics", __FUNCTION__, MAX_FILE_PRINT);
        return;
    }

    if(PtpGetIsOneHandle(eWhichOtg)) // Alway .fileID = 1 when PTP one handle 
    {
        psDps->dwFilePrintCount = 0;
        FileID = 0;
    }

    psDps->pstPrintInfo[psDps->dwFilePrintCount].fileID = FileID + 1; /* FileID follow File-Index start from 0 */
    psDps->pstPrintInfo[psDps->dwFilePrintCount].copies = Copies;
    psDps->pstPrintInfo[psDps->dwFilePrintCount].bFileNamePrint = bFileNamePrint; /* FileID follow File-Index start from 0 */
    psDps->pstPrintInfo[psDps->dwFilePrintCount].bFileDatePrint = bDatePrint;


    if((CropX != 0) ||  (CropY != 0) ||  (CropW != 0) || ( CropH != 0))
    {
        psDps->pstPrintInfo[psDps->dwFilePrintCount].Crop = TRUE;
        psDps->pstPrintInfo[psDps->dwFilePrintCount].CropX = CropX;
        psDps->pstPrintInfo[psDps->dwFilePrintCount].CropY = CropY;
        psDps->pstPrintInfo[psDps->dwFilePrintCount].CropW = CropW;
        psDps->pstPrintInfo[psDps->dwFilePrintCount].CropH = CropH;    
    }

    psDps->dwFilePrintCount += 1;    
    //psDps->stPrintJobInfo.printJobInfoCount = psDps->dwFilePrintCount;
}

void Api_UsbdDpsSetPrintJobConfig(ePrinterCapability JobConfigId, DWORD JobConfigValue, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    
	switch (JobConfigId)
	{
            	case DPS_PARAM_QUALITY:
                            psDps->stPrintJobConfig.quality = tabPrinterCapabilityQuality[JobConfigValue] ;
                            break;

            	case DPS_PARAM_PAPER_SIZE:
                            psDps->stPrintJobConfig.paperSize = tabPrinterCapabilityPaperSize[JobConfigValue] ;
                            break;

            	case DPS_PARAM_PAPER_TYPE:
                            psDps->stPrintJobConfig.paperType = tabPrinterCapabilityPaperType[JobConfigValue] ;
                            break;

            	case DPS_PARAM_FILE_TYPE:
                            psDps->stPrintJobConfig.fileType = tabPrinterCapabilityFileType[JobConfigValue] ;
                            break;

            	case DPS_PARAM_DATE_PRINT:
                            psDps->stPrintJobConfig.datePrint = tabPrinterCapabilityDatePrint[JobConfigValue] ;
                            break;

            	case DPS_PARAM_FILENAME_PRINT:
                            psDps->stPrintJobConfig.fileNamePrt = tabPrinterCapabilityFileNamePrint[JobConfigValue] ;
                            break;

            	case DPS_PARAM_IMAGE_OPTIMIZE:
                            psDps->stPrintJobConfig.imageOpt = tabPrinterCapabilityImageOptimize[JobConfigValue] ;
                            break;

            	case DPS_PARAM_LAYOUT:
                            psDps->stPrintJobConfig.layout = tabPrinterCapabilityLayout[JobConfigValue] ;
                            break;

            	case DPS_PARAM_FIXED_SIZE:
                            psDps->stPrintJobConfig.fixedSize = tabPrinterCapabilityFixedSize[JobConfigValue] ;
                            break;

            	case DPS_PARAM_CROPPING:
                            psDps->stPrintJobConfig.cropping = tabPrinterCapabilityCropping[JobConfigValue] ;
                            break;
	}
}

void Api_UsbdDpsSetPrintAction(ePrintAction act, WHICH_OTG eWhichOtg)
{
    
    MP_DEBUG("-USBOTG%d %s Act:%x", eWhichOtg, __FUNCTION__, act);

    // Clean the other JobConfig & PrintInfo by UI, 
    if(act == DPS_PARAM_INIT_JOB)
    {
    	DpsInitJob(eWhichOtg); // Clean setting
    }
    // otherwise it will keep the pre-setting 
    //DPS_INIT_JOB,  // need to implement to init job calvin
    else
    {
        if(act < DPS_PARAM_PRINTACTION_MAX)
            SetDpsState(tabPrinterPrintAction[act], eWhichOtg);    
        else
            MP_ALERT("-USBOTG%d %s Para Act Err:%x", eWhichOtg, __FUNCTION__, act);
    }
}

//void UsbdPrintExitWithError() call this function
void Api_UsbdDpsGetDeviceStatus(DWORD* DeviceStatus, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);

    MP_DEBUG("-USBOTG%d- %s", eWhichOtg, __FUNCTION__);

	//dpsPrintServiceStatus
	DeviceStatus[DPS_DevStat_PrintStatus] = GetDpsParam((DWORD*)tabPrintStatus, DPS_PrintStatus_MAX, psDps->stPrinterStatus.hostDeviceStatus.printServiceStatus);
    MP_DEBUG("DPS_DevStat_PrintStatus %d", DeviceStatus[DPS_DevStat_PrintStatus]);
	//jobEndReason
	DeviceStatus[DPS_DevStat_JobEnd] = GetDpsParam((DWORD*)tabJobEnd, DPS_JobEnd_MAX, psDps->stPrinterStatus.hostDeviceStatus.jobEndReason);
    MP_DEBUG("DPS_DevStat_JobEnd %d", DeviceStatus[DPS_DevStat_JobEnd]);
	//errorStatus
	DeviceStatus[DPS_DevStat_ErrStatus] = GetDpsParam((DWORD*)tabErrStatus, DPS_ErrStatus_MAX, psDps->stPrinterStatus.hostDeviceStatus.errorStatus);
    MP_DEBUG("DPS_DevStat_ErrStatus %d", DeviceStatus[DPS_DevStat_ErrStatus]);
	//errorReason
	DeviceStatus[DPS_DevStat_ErrReason] = GetDpsParam((DWORD*)tabErrReason, DPS_ErrReason_MAX, psDps->stPrinterStatus.hostDeviceStatus.errorReason);
    MP_DEBUG("DPS_DevStat_ErrReason %d", DeviceStatus[DPS_DevStat_ErrReason]);
	//disconnectEnable
	DeviceStatus[DPS_DevStat_DisconEn] =  psDps->stPrinterStatus.hostDeviceStatus.disconnectEnable;
    MP_DEBUG("DPS_DevStat_DisconEn %d", DeviceStatus[DPS_DevStat_DisconEn]);
	//capabilityChanged
	DeviceStatus[DPS_DevStat_CapabilityChg] =  psDps->stPrinterStatus.hostDeviceStatus.capabilityChanged;
    MP_DEBUG("DPS_DevStat_CapabilityChg %d", DeviceStatus[DPS_DevStat_CapabilityChg]);    
	//newJobOK
	DeviceStatus[DPS_DevStat_NewJobOK] =  psDps->stPrinterStatus.hostDeviceStatus.newJobOK;
    MP_DEBUG("DPS_DevStat_NewJobOK %d", DeviceStatus[DPS_DevStat_NewJobOK]);
}


// 
void Api_UsbdDpsCurPhotoPrint(BOOL bIsCurPhotoPrint, WHICH_OTG eWhichOtg)
{
    PtpSetOneHandle(bIsCurPhotoPrint, eWhichOtg);
}


void ClearNewJobOk(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	psDps->boIsNewJobOk = FALSE;
}

void DpsConfPrintService(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    
	psDps->stDscStatus.opType = OP_CONF_PRINT_SERVICE;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;
	_DpsSendOperation(psDps->pbXmlBuff, eWhichOtg);
	//Fill Printer Settings 
	_DpsFillPrinterSettings(&psDps->stPrinterSettings, &psDps->stDpsPrinterCapability);
	psDps->stDscStatus.opResult = okResult;
}

void DpsGetCapability(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	//static DWORD layoutForPaperSizeIndex = 0;
	//static DWORD paperTypeForPaperSizeIndex = 0;
    
	if (psDps->stDscStatus.opResult == okResult)
		//if (1) // keep going to get capability
	{
		//Turn next printer capability   Calvin 2004.08.10
		switch (psDps->stRequestCapability)
		{
#ifdef SGBOARD
		case 0:
			MP_DEBUG("Test_state_1:0");
			psDps->stRequestCapability = DPS_FILE_TYPES;
			break;
#else
		case 0:
			MP_DEBUG("Test_state_1:0");
			psDps->stRequestCapability = DPS_QUALITIES;
			break;

		case DPS_QUALITIES:
			MP_DEBUG("DPS_QUALITIES");
			psDps->stRequestCapability = DPS_PAPER_SIZES;
			break;

        #if 0
		case DPS_PAPER_SIZES:
			MP_DEBUG("DPS_PAPER_SIZES");
			psDps->stRequestCapability = DPS_PAPER_TYPES;
			break;

		case DPS_PAPER_TYPES:
			MP_DEBUG("DPS_PAPER_TYPES");
			psDps->stRequestCapability = DPS_FILE_TYPES;
			break;
        #else

		case DPS_PAPER_SIZES:
			MP_DEBUG("DPS_PAPER_SIZES");
			psDps->stRequestCapability = DPS_PAPER_TYPES;
			psDps->dwPaperTypeForPaperSizeIdx = 0;
			psDps->stRequestCapabilityPaperSize = psDps->stDpsPrinterCapability.paperSizes[0];
			psDps->dwPaperTypeForPaperSizeIdx++;
			break;

		case DPS_PAPER_TYPES:
			MP_DEBUG("DPS_PAPER_TYPES");
			if (psDps->dwPaperTypeForPaperSizeIdx == psDps->stDpsPrinterCapability.paperSizesCount)
			{
				psDps->stRequestCapability = DPS_FILE_TYPES;
			}
			else
			{
				// keep getting paperType for all paperSizes
				//psDps->stRequestCapability = DPS_LAYOUTS;    
				psDps->stRequestCapabilityPaperSize =
					psDps->stDpsPrinterCapability.paperSizes[psDps->dwPaperTypeForPaperSizeIdx];
				psDps->dwPaperTypeForPaperSizeIdx++;
			}
			break;
        #endif
        
		case DPS_FILE_TYPES:
			MP_DEBUG("DPS_FILE_TYPES");
			psDps->stRequestCapability = DPS_DATE_PRINTS;
			break;

		case DPS_DATE_PRINTS:
			MP_DEBUG("DPS_DATE_PRINTS");
			psDps->stRequestCapability = DPS_FILE_NAME_PRINTS;
			break;

		case DPS_FILE_NAME_PRINTS:
			MP_DEBUG("DPS_FILE_NAME_PRINTS");
			psDps->stRequestCapability = DPS_IMAGE_OPTIMIZES;
			break;

		case DPS_IMAGE_OPTIMIZES:
			MP_DEBUG("DPS_IMAGE_OPTIMIZES");
			psDps->stRequestCapability = DPS_LAYOUTS;
			psDps->dwLayoutForPaperSizeIdx = 0;
			psDps->stRequestCapabilityPaperSize = psDps->stDpsPrinterCapability.paperSizes[0];
			psDps->dwLayoutForPaperSizeIdx++;
			break;

		case DPS_LAYOUTS:
			MP_DEBUG("DPS_LAYOUTS");
			if (psDps->dwLayoutForPaperSizeIdx == psDps->stDpsPrinterCapability.paperSizesCount)
			{
				psDps->stRequestCapability = DPS_FIXED_SIZES;
			}
			else
			{
				// keep getting layouts for all paperSizes
				//psDps->stRequestCapability = DPS_LAYOUTS;    
				psDps->stRequestCapabilityPaperSize =
					psDps->stDpsPrinterCapability.paperSizes[psDps->dwLayoutForPaperSizeIdx];
				psDps->dwLayoutForPaperSizeIdx++;
			}
			break;

		case DPS_FIXED_SIZES:
			MP_DEBUG("DPS_FIXED_SIZES");
			psDps->stRequestCapability = DPS_CROPPINGS;
			break;

//              case DPS_CROPPINGS:   // end
			// TODO: get papertypes per paper size here?!
//                  break;
#endif // #if SGBOARD
		}

		psDps->stDscStatus.opType = OP_GET_CAPABILITY;
		psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;
		_DpsSendOperation(psDps->pbXmlBuff, eWhichOtg);
		//Fill Printer Settings 
		_DpsFillPrinterSettings(&psDps->stPrinterSettings, &psDps->stDpsPrinterCapability);
	}
	else
	{
		DpsConfPrintService(eWhichOtg);
	}

}

void DpsGetDeviceStatus(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    
//  MP_DEBUG("PictureBridgeTest_state_2");
	if (psDps->stDscStatus.opResult == okResult)
	{
#ifdef SGBOARD
		if (psDps->stRequestCapability == DPS_FILE_TYPES)
#else
		if (psDps->stRequestCapability == DPS_CROPPINGS)
#endif
		{
			//  MP_DEBUG("DPS_CROPPINGS");
			psDps->stDscStatus.opType = OP_GET_DEV_STATUS;
			psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

/* // check later // JL, 05252005
             // record time stamp
            psDps->stDpsPrinterCapability.timeStamp = SystemGetTimeStamp();
*/
			_DpsSendOperation(psDps->pbXmlBuff, eWhichOtg);
			//Fill Printer Settings 
			_DpsFillPrinterSettings(&psDps->stPrinterSettings, &psDps->stDpsPrinterCapability);
		}
		else
		{
			DpsGetCapability(eWhichOtg);
		}
	}
	else
	{
		DpsConfPrintService(eWhichOtg);
	}
}

void DpsCapabilityInit(WHICH_OTG eWhichOtg) // Next PictBridge Tool Stage for capability during PTP Init
{
    PPICT_BRIDGE_DPS psDps;

    psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    
    psDps->stRequestCapability = 0;
    psDps->stRequestCapabilityPaperSize = noPaperSize;

    psDps->dwLayoutForPaperSizeIdx = 0;
    psDps->dwPaperTypeForPaperSizeIdx = 0;
    psDps->boIsPrinting = FALSE;
    psDps->boSend = 1;
    psDps->dwDpsPaperTypeLoopCnt = 0;
    psDps->dwDpsLayoutLoopCnt = 0;
}

BOOL DpsStartJob(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	int i = 0;
	BOOL rt;

	

	psDps->boIsStartJob = TRUE;
#if 0
	psDps->stPrintJobInfo.jobConf.datePrint = dateOff;	//_GetDatePrint(jobSettings->datePrint); //Calvin 2004.08.05
	psDps->stPrintJobInfo.jobConf.fileNamePrt = noNamePrint;
	psDps->stPrintJobInfo.jobConf.imageOpt = noImageOpt;
	psDps->stPrintJobInfo.jobConf.fileType = GetPrintedFileType();	//exifJpeg;     //dpof
	psDps->stPrintJobInfo.jobConf.cropping = noCropping;

	psDps->stPrintJobInfo.jobConf.quality = noQuality;	//_GetQuality(jobSettings->quality);    //noQuality;
	psDps->stPrintJobInfo.jobConf.paperType = noPaperType;	//_GetPaperType(jobSettings->paperTypes);   //noPaperType;
	psDps->stPrintJobInfo.jobConf.paperSize = noPaperSize;	//_GetPaperSize(jobSettings->paperSize);    //noPaperSize;
	//check MenuData(LayOut)
	psDps->stPrintJobInfo.jobConf.layout = noLayout; // defLayout; // oneFullBleed;	//_GetLayout(jobSettings, jobSettings->paperSize);      //index;
	psDps->stPrintJobInfo.jobConf.fixedSize = noFixedSize;	//_GetFixedSize(jobSettings->fixedPrintSize);   //noFixedSize;

	psDps->stPrintJobInfo.printJobInfoCount = 1;


	psDps->pstPrintInfo.copies = 1;		//files[i].copies ;
	psDps->pstPrintInfo.fileID = 1;		//files[i].fileID; //Calvin 2004.07.22
	psDps->pstPrintInfo.fileName[0] = '\0';	//No file print option calvin 2004.08.06
	memset(psDps->pstPrintInfo.fileDate, 0, 12);
	psDps->stPrintJobInfo.pPrintJobInfo = &psDps->pstPrintInfo;
#endif

	psDps->stDscStatus.opType = OP_START_JOB;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

	rt = DPS_SUCCESS;
	if (_DpsSendOperation(NULL, eWhichOtg) == FAIL)
	{
		rt = DPS_FAILURE;
	}
	if (psDps->stDscStatus.opResult == okResult && rt != DPS_FAILURE)
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job accepted OK");
		psDps->stPrintJobInfo.jobStatus.timeStamp = 0;	// to force updating the job status
		rt = DPS_SUCCESS;
	}
	else
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job send failed");
		rt = DPS_FAILURE;
	}
	return rt;
/*
<?xml version="1.0"?>
<dps xmlns="http://www.cipa.jp/dps/schema/">
<input>
<startJob>
<jobConfig>
<paperType>52000000</paperType>
<paperSize>51000000</paperSize>
<fileType>53010000</fileType>
<fixedSize>58000000</fixedSize>
</jobConfig>
<printInfo>
<fileID>00000001</fileID>
<copies>001</copies>
</printInfo>
</startJob>
</input>
</dps>
*/

/*
<?xml version="1.0"?>
..<dps xmlns="http://www.cipa.jp/dps/schema/">
..<input>
  <startJob>
  <jobConfig>
  <paperType>52000000</paperType>
..<paperSize>51000000</paperSize>
..<fileType>53010000</fileType>
..<datePrint>54010000</datePrint>
..<layout>57000000</layout>
..</jobConfig>
..<printInfo>
..<fileID>00000001</fileID>
..<copies>D</copies>
..<date></date>
..</printInfo>
..</startJob>
..</input>
..</dps>..
*/

}

//extern int gDps_state_test;
//---------------------------------------------------------------------------
//of  DPSPostResponse
//o
//o This function posts a DPS Response FROM the printer TO the DSC.
//o    This function should be used by the USB slave when it receives such Response.
//o    DPS Response is the response of the printer to a DPS Operation sent by the DSC.
//o
//o Input:
//o     buf_in: pointer to the buffer that contains the DPS response (XML string)
//o     len   : the length of the input buffer.
//o
//o Return Value:
//o     BOOL: 
//o       TRUE: if the printer is waiting such response type and the response if proprly fomatted.
//o       FALSE: Otherwise
//---------------------------------------------------------------------------
BOOL DPSPostResponse(BYTE * buf_in, DWORD len, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	BOOL ret = PASS;
	char *pos;
    //static int dwDpsPaperTypeLoopCnt, dwDpsLayoutLoopCnt = 0;    

	pos = strstr((char *) buf_in, "</dps>");
	*(pos + 6) = '\0';

	//MP_DEBUG1("Response received:\n%s", (char *)buf_in);
	//MP_DEBUG("Response received:\n");

	if (psDps->stDscStatus.opType == OP_ABORT_JOB)
		psDps->stDscStatus.opStatus = OPSTATUS_WAITING_RESP;

	// 1 - check if response is waited
	if (psDps->stDscStatus.opStatus != OPSTATUS_WAITING_RESP)
	{
		MP_DEBUG1("DPSPostResponse:%d", psDps->stDscStatus.opStatus);
		return FALSE;
	}

	// 2 - verify response is OK
	switch (psDps->stDscStatus.opType)
	{
	case OP_CONF_PRINT_SERVICE:
        psDps->dwDpsPaperTypeLoopCnt = psDps->dwDpsLayoutLoopCnt = 0;
		ret =
			dpsParseRespConfPrintService((char *) buf_in, &psDps->stPrinterStatus.hostConfPrintService,
										 &psDps->stDscStatus.opResult);
		if (ret == DPS_SUCCESS)
		{
			SetDpsState(DPS_GET_CAPABILITY, eWhichOtg);	// for letting Cannon Selphy printer work
		}
		// gDps_state_test = 2;
		//SEBUG1("Send:\n\r%s", (char *)buf_in); //don't delete Calvin 2004.07.22
		break;

	case OP_GET_CAPABILITY:
		ret =
			dpsParseRespGetCapability((char *) buf_in, psDps->stRequestCapabilityPaperSize,
									  psDps->stRequestCapability, &psDps->stDpsPrinterCapability,
									  &psDps->stDscStatus.opResult);
        #if 1
        //DEBUG MESSAGE
        switch(psDps->stRequestCapability)
        {
        	case DPS_QUALITIES:
                MP_DEBUG("DPS_QUALITIES Cnt %d", psDps->stDpsPrinterCapability.qualitiesCount);
        		break;

        	case DPS_PAPER_SIZES:
                MP_DEBUG("DPS_PAPER_SIZES Cnt %d", psDps->stDpsPrinterCapability.paperSizesCount);
        		break;

        	case DPS_PAPER_TYPES:
                MP_DEBUG("DPS_PAPER_TYPES idx %d Cnt %d", psDps->dwDpsPaperTypeLoopCnt, psDps->stDpsPrinterCapability.paperTypesCount[psDps->dwDpsPaperTypeLoopCnt]);
                psDps->dwDpsPaperTypeLoopCnt++;                
        		break;

        	case DPS_FILE_TYPES:
                MP_DEBUG("DPS_FILE_TYPES Cnt %d", psDps->stDpsPrinterCapability.fileTypesCount);
        		break;


        	case DPS_DATE_PRINTS:
                MP_DEBUG("DPS_DATE_PRINTS Cnt %d", psDps->stDpsPrinterCapability.datePrintsCount);
        		break;


        	case DPS_FILE_NAME_PRINTS:
                MP_DEBUG("DPS_FILE_NAME_PRINTS Cnt %d", psDps->stDpsPrinterCapability.fileNamePrintsCount);
        		break;


        	case DPS_IMAGE_OPTIMIZES:
                MP_DEBUG("DPS_IMAGE_OPTIMIZES Cnt %d", psDps->stDpsPrinterCapability.imageOptimizesCount);
        		break;

        	case DPS_LAYOUTS:
                MP_DEBUG("DPS_LAYOUTS idx %d Cnt %d", psDps->dwDpsLayoutLoopCnt, psDps->stDpsPrinterCapability.layoutsCount[psDps->dwDpsLayoutLoopCnt]);
                psDps->dwDpsLayoutLoopCnt++;
        		break;

        	case DPS_FIXED_SIZES:
                MP_DEBUG("DPS_FIXED_SIZES Cnt %d", psDps->stDpsPrinterCapability.fixedSizesCount);
        		break;

        	case DPS_CROPPINGS:
                MP_DEBUG("DPS_CROPPINGS Cnt %d", psDps->stDpsPrinterCapability.croppingsCount);
        		break;
        }
        #endif
        
#ifdef SGBOARD
		if (psDps->stRequestCapability == DPS_FILE_TYPES)
#else
		if (psDps->stRequestCapability == DPS_CROPPINGS)
#endif
		{
			SetDpsState(DPS_GET_DEVICE_STATUS, eWhichOtg);
			//   psDps->boIsGetCapAlready = TRUE;
		}
		else
		{
			SetDpsState(DPS_GET_CAPABILITY, eWhichOtg);
		}
		//_displayCapability( psDps->stRequestCapabilityPaperSize, psDps->stRequestCapability, &psDps->stDpsPrinterCapability);
		//SEBUG1("Send:\n\r%s", (char *)buf_in); //don't delete Calvin 2004.07.22
		break;

	case OP_GET_JOB_STATUS:
		//ret = dpsParseJobStatus ((char *)buf_in, printJobInfo.jobConf.fileType, &psDps->stPrintJobInfo.jobStatus);
		ret =
			dpsParseRespGetJobStatus((char *) buf_in, psDps->stPrintJobInfo.jobConf.fileType,
									 &psDps->stPrintJobInfo.jobStatus, &psDps->stDscStatus.opResult);
/* // check later // JL, 05252005
            if ( ret != FAIL && psDps->stDscStatus.opResult == okResult )
            {
                psDps->stPrintJobInfo.jobStatus.timeStamp = SystemGetTimeStamp();
            }
*/
		break;

	case OP_GET_DEV_STATUS:
		{
			psDps->boIsGetCapAlready = TRUE;
			//ret = dpsParseEventNotifyDeviceStatus ((char *)buf_in, &psDps->stPrinterStatus.hostDeviceStatus);
			ret =
				dpsParseRespGetDeviceStatus((char *) buf_in, &psDps->stPrinterStatus.hostDeviceStatus,
											&psDps->stDscStatus.opResult);
			//SerialSendString((char *)buf_in);
			/* // check later // JL, 05252005
			   if ( ret != FAIL && psDps->stDscStatus.opResult == okResult )
			   {
			   printerStatus.hostDeviceStatus.timeStamp = SystemGetTimeStamp();
			   }
			 */
		}
		break;

	case OP_START_JOB:
		ret = dpsParseRespStartJob((char *) buf_in, &psDps->stDscStatus.opResult);
		break;

	case OP_ABORT_JOB:
		ret = dpsParseRespAbortJob((char *) buf_in, &psDps->stDscStatus.opResult);
		if (ret == DPS_SUCCESS)
		{
            EventSet(UI_EVENT, EVENT_PRINT_FINISH);
		}
		break;

	case OP_CONTINUE_JOB:
		ret = dpsParseRespContinueJob((char *) buf_in, &psDps->stDscStatus.opResult);
		break;

	default:
		MP_DEBUG("ERROR: Unexpected operation!\n");
		return FALSE;
	}

	if (ret == FAIL)
	{
		MP_DEBUG("response Failed");
		psDps->stDscStatus.opStatus = OPSTATUS_GOT_RESP;
		return FAIL;
	}

	// 3 - update flags that keep track of the changes
	// ONLY IF the needed response is received
	psDps->stDscStatus.opStatus = OPSTATUS_GOT_RESP;

	return TRUE;
}

//---------------------------------------------------------------------------
//of   DpsAbortJob
//o
//o This function aborts the current print job 
//o
//o Input:
//o     none
//o
//o Return Value:
//o     DPS_SUCCESS: if the abort operation succeed to submit and 
//o              positive response is received from the printer.
//o     FAILURE: response from the printer is not OK or the response timed out.
//---------------------------------------------------------------------------
BOOL DpsAbortJob(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
//    SDWORD ts;
	BOOL rt;

/* // check later // JL, 05252005
    // Wait for DPS to be ready, then start task.   
    ts = SystemGetTimeStamp();
    while ( psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED )
    {
        if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
        {
			//MP_DEBUG("Timed out");
            return FAILURE;
        }
    }
*/
	psDps->stDscStatus.opType = OP_ABORT_JOB;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

	rt = DPS_SUCCESS;
	if (_DpsSendOperation(NULL, eWhichOtg) == FAIL)
	{
		rt = DPS_FAILURE;
	}

	if (psDps->stDscStatus.opResult == okResult && rt != DPS_FAILURE)
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		MP_DEBUG("job aborted OK");

		rt = DPS_SUCCESS;
	}
	else
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		MP_DEBUG("job abort failed");
		rt = DPS_FAILURE;
	}

	return rt;
}

//---------------------------------------------------------------------------
//of   DpsContinueJob
//o
//o This function aborts the current print job 
//o
//o Input:
//o     none
//o
//o Return Value:
//o     DPS_SUCCESS: if the abort operation succeed to submit and 
//o              positive response is received from the printer.
//o     FAILURE: response from the printer is not OK or the response timed out.
//---------------------------------------------------------------------------
BOOL DpsContinueJob(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
//    SDWORD ts;
	BOOL rt;

/* // check later // JL, 05252005
    // Wait for DPS to be ready, then start task.   
    ts = SystemGetTimeStamp();
    while ( psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED )
    {
        if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
        {
			//MP_DEBUG("Timed out");
            return FAILURE;
        }
    }
*/
	psDps->stDscStatus.opType = OP_CONTINUE_JOB;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

	rt = DPS_SUCCESS;
	if (_DpsSendOperation(NULL, eWhichOtg) == FAIL)
	{
		rt = DPS_FAILURE;
	}

	if (psDps->stDscStatus.opResult == okResult && rt != DPS_FAILURE)
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		MP_DEBUG("job continue OK");

		rt = DPS_SUCCESS;
	}
	else
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		MP_DEBUG("job continue failed");
		rt = DPS_FAILURE;
	}

	return rt;
}

//---------------------------------------------------------------------------
//of  DPSPostEvent
//o
//o This function posts a DPS event (asynchrnous) FROM the printer TO the DSC.
//o    This function should be used by the USB slave when it receives such event.
//o
//o Input:
//o     buf_in: pointer to the buffer that contains the DPS event (XML string)
//o     buf_out: pointer to the buffer that the DSC will fill its XML string response of the event.
//o              USB slave will pass the string in buf_out as the DPS response of the event.
//o Return Value:
//o     DWORD: length of the data to be sent in buf_out.
//o             It can be ZERO if there is no data to be sent
//---------------------------------------------------------------------------
DWORD DPSPostEvent(BYTE * buf_in, BYTE * buf_out, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	BOOL ret;
	DWORD len = 0;
	char *pos;

	//static BOOL isPrinting = FALSE;
	//static BOOL send = 1;

	pos = strstr((char *) buf_in, "</dps>");
	*(pos + 6) = '\0';

	//MP_DEBUG1("Event received:\n%s", (char *)buf_in);
	//MP_DEBUG("Event received\n");

	//- check if event is valid now (no response is expected)
	if (psDps->stDscStatus.opStatus == OPSTATUS_WAITING_RESP)
	{
		//MP_DEBUG1("now waiting a response %d", psDps->stDscStatus.opStatus );
		return 0;
	}
	//- decode event and get the data from the event
	if (strstr((char *) buf_in, "notifyDeviceStatus") != NULL)
	{
		SetDpsState(DPS_NOTIFY_DEVICE_STATUS, eWhichOtg);
		ret = dpsParseEventNotifyDeviceStatus((char *) buf_in, &psDps->stPrinterStatus.hostDeviceStatus);
		if (ret != FAIL)
		{
			DpsPrinterStatus dpsPrinterStatus;
			DWORD errCode = NO_ERR;

			dpsRespEventNotifyDeviceStatus((char *) buf_out, okResult);
			if (psDps->stPrinterStatus.hostDeviceStatus.newJobOK == TRUE)
			{
				psDps->boIsNewJobOk = TRUE;
                            EventSet(UI_EVENT, EVENT_PRINT_INIT);
			}

			DpsGetPrinterStatus(&dpsPrinterStatus, eWhichOtg);
			if (dpsPrinterStatus.status == DPS_PrintStatus_Printing)
			{
				psDps->boIsPrinting = TRUE;
				EventSet(UI_EVENT, EVENT_PRINT_PRINTING);
			}
			else if (dpsPrinterStatus.status == DPS_PrintStatus_Idle)
			{
				if (psDps->boIsPrinting == TRUE)
				{
					psDps->boIsPrinting = FALSE;
					if (dpsPrinterStatus.jobEnded != DPS_JobEnd_AbortJob_Immediate)
					{
                                        psDps->boIsJobFinished = TRUE;
                                        psDps->boIsStartJob = FALSE;
                                        EventSet(UI_EVENT, EVENT_PRINT_FINISH);
					}
				}
			}
            
			if (dpsPrinterStatus.error != DPS_ErrStatus_NoError ||
				dpsPrinterStatus.errorReason != DPS_ErrReason_NoReason)
			{
				errCode = CheckDpsError(dpsPrinterStatus, eWhichOtg);
				if (errCode != NO_ERR)
				{
					//SetDpsState(DPS_ABORT_JOB);
                                    EventSet(UI_EVENT, EVENT_PRINT_ERROR);
					SystemSetErrMsg(errCode);
				}
			}

			/* // check later // JL, 05252005
			   psDps->stPrinterStatus.hostDeviceStatus.timeStamp = SystemGetTimeStamp();
			   SerialSendString("D");
			 */
			// // //tmp
			psDps->boSend = !psDps->boSend;
			//            if (psDps->boSend && SystemGetElapsedTime(psDps->stPrinterStatus.hostDeviceStatus.timeStamp)< 200)  // not older than 2 seconds
			//csh            if (psDps->boSend)
			//csh                return 0;
			// // //
			//SerialSendString("D");
		}
		else
		{
			dpsRespEventNotifyDeviceStatus((char *) buf_out, notSupported);
		}
	}
	else if (strstr((char *) buf_in, "notifyJobStatus") != NULL)
	{
		//      ret = dpsParseJobStatus ((char *)buf_in, printJobInfo.jobConf.fileType, &psDps->stPrintJobInfo.jobStatus);
		ret =
			dpsParseEventNotifyJobStatus((char *) buf_in, psDps->stPrintJobInfo.jobConf.fileType,
										 &psDps->stPrintJobInfo.jobStatus);
		if (psDps->stPrintJobInfo.jobStatus.imagesPrinted == psDps->stPrintJobInfo.jobStatus.progress[0])
		{
			;					//psDps->boIsJobFinished = TRUE;
			//EventSet(UI_EVENT, EVENT_PRINT);
		}
		//tmp
		psDps->boSend = 0;

		if (ret != FAIL)
		{
			psDps->boIsPrinting = TRUE; // For EPSON-TX610FW printer didn't send PRINTING message.
			dpsRespEventNotifyJobStatus((char *) buf_out, okResult);

/* // check later // JL, 05252005
            SerialSendString("J");
            psDps->stPrintJobInfo.jobStatus.timeStamp =  SystemGetTimeStamp();
*/
		}
		else
		{
			dpsRespEventNotifyJobStatus((char *) buf_out, notRecognized);
		}

		SetDpsState(DPS_NOTIFY_JOB_STATUS, eWhichOtg);
	}
	else
	{
		MP_DEBUG("Bogus event!");
		dpsRespNotRecognizedOperation((char *) buf_out);
		SetDpsState(DPS_NOT_RECOGNIZED_OPERATION, eWhichOtg);
//        return 0;
	}

//    MP_DEBUG1("Event Reply:\n%s", (char *)buf_out);
	//MP_DEBUG("Event Reply sent");
	len = strlen((char *) buf_out);
	return len;

}


/*
// Definition of local functions 
*/
static BYTE GetDpsParam(DWORD* tabPrinterCapability, DWORD Count, DWORD index)
{
    int i =0;

    for(i=0 ; i<Count; i++)
    {
        //MP_DEBUG("idx %d %x = %x", i, tabPrinterCapability[i], index);

        if((index & 0xFFFF0000) == tabPrinterCapability[i])  // Get Major ,No Minor
            return i;

    }
    return 0xFF;
}

static void DpsInitJob(WHICH_OTG eWhichOtg)  // Clean setting before printing
{
    PPICT_BRIDGE_DPS psDps  = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
    psDps->dwFilePrintCount = 0;
}


/*
// this is API function for UI, pls do not delete
DpsPrinterCapability *ui_getPrinterCapability()
{

	return &psDps->stDpsPrinterCapability;

}
*/

static BOOL IsJobStarted(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	BOOL isStartJob = psDps->boIsStartJob;

	if (psDps->boIsStartJob == TRUE)
	{
		psDps->boIsStartJob = FALSE;
	}

	return isStartJob;
}

static BOOL IsJobFinished(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	BOOL isJobFinished = psDps->boIsJobFinished;

	if (psDps->boIsJobFinished == TRUE)
	{
		psDps->boIsJobFinished = FALSE;
	}

	return isJobFinished;
}

/*
BOOL IsGetCapAlready(void)
{
	BOOL isGetCapAlready = psDps->boIsGetCapAlready;

	return isGetCapAlready;
}

BOOL IsStartJob(void)
{
	BOOL isStartJob = FALSE;

	if (psDps->boIsNewJobOk && psDps->boIsGetCapAlready)
	{
		isStartJob = TRUE;
	}

	return isStartJob;
}
*/

/*
BOOL IsPrinterSupportFileType(WORD format)
{
	BOOL ret = FALSE;
	WORD i = 0;

	switch (format)
	{
	case OBJ_FORMAT_JPEG_EXIF:
		for (i = 0; i < psDps->stDpsPrinterCapability.fileTypesCount; i++)
		{
			ret = (psDps->stDpsPrinterCapability.fileTypes[i] == fileTypeJpeg) ? TRUE : FALSE;
			if (ret == TRUE)
				break;
		}
		break;
	case OBJ_FORMAT_BMP:
		for (i = 0; i < psDps->stDpsPrinterCapability.fileTypesCount; i++)
		{
			ret = (psDps->stDpsPrinterCapability.fileTypes[i] == fileTypeBmp) ? TRUE : FALSE;
			if (ret == TRUE)
				break;
		}
		break;
	default:
		break;
	}

	return ret;
}


void _displayCapability(PaperSize paperSize, DWORD capability, DpsPrinterCapability * printerCap)
{
	int i, j;

	switch (capability)
	{
	case DPS_QUALITIES:
		//MP_DEBUG1("DPS_QUALITIES = %d ", printerCap->qualitiesCount);

		for (i = 0; i < printerCap->qualitiesCount; i++)
			//MP_DEBUG1("%X",printerCap->qualities[i]);

			break;

	case DPS_PAPER_SIZES:
		//MP_DEBUG1("DPS_PAPER_SIZES = %d ", printerCap->paperSizesCount);

		for (i = 0; i < printerCap->paperSizesCount; i++)
			//MP_DEBUG1("%X",printerCap->paperSizes[i]);

			break;

	case DPS_PAPER_TYPES:
		//MP_DEBUG1("DPS_PAPER_TYPES = %d ", printerCap->paperTypesCount);

		for (i = 0; i < printerCap->paperTypesCount; i++)
			//MP_DEBUG1("%X",printerCap->paperTypes[i]);

			break;

	case DPS_FILE_TYPES:
		//MP_DEBUG1("DPS_FILE_TYPES = %d ", printerCap->fileTypesCount);

		for (i = 0; i < printerCap->fileTypesCount; i++)
			//MP_DEBUG1("%X",printerCap->fileTypes[i]);

			break;


	case DPS_DATE_PRINTS:
		//MP_DEBUG1("DPS_DATE_PRINTS = %d ", printerCap->datePrintsCount);

		for (i = 0; i < printerCap->datePrintsCount; i++)
			//MP_DEBUG1("%X",printerCap->datePrints[i]);

			break;


	case DPS_FILE_NAME_PRINTS:
		//MP_DEBUG1("DPS_FILE_NAME_PRINTS = %d ", printerCap->fileNamePrintsCount);

		for (i = 0; i < printerCap->fileNamePrintsCount; i++)
			//MP_DEBUG1("%X",printerCap->fileNamePrints[i]);

			break;


	case DPS_IMAGE_OPTIMIZES:
		//MP_DEBUG1("DPS_IMAGE_OPTIMIZES = %d ", printerCap->imageOptimizesCount);

		for (i = 0; i < printerCap->imageOptimizesCount; i++)
			//MP_DEBUG1("%X",printerCap->imageOptimizes[i]);

			break;

	case DPS_LAYOUTS:
		for (j = 0; j < printerCap->paperSizesCount; j++)
		{
			if (printerCap->paperSizes[j] == paperSize)
			{
				break;
			}
		}
		//MP_DEBUG3("For paperSize = %x  -- index(%d), DPS_LAYOUTS = %d ", paperSize, j, printerCap->layoutsCount[j]);

		for (i = 0; i < printerCap->layoutsCount[j]; i++)
			//MP_DEBUG1("%X",printerCap->layouts[j][i]);

			break;

	case DPS_FIXED_SIZES:
		//MP_DEBUG1("DPS_FIXED_SIZES = %d ", printerCap->fixedSizesCount);

		for (i = 0; i < printerCap->fixedSizesCount; i++)
			//MP_DEBUG1("%X",printerCap->fixedSizes[i]);

			break;

	case DPS_CROPPINGS:
		//MP_DEBUG1("DPS_CROPPINGS = %d ", printerCap->croppingsCount);

		for (i = 0; i < printerCap->croppingsCount; i++)
			//MP_DEBUG1("%X",printerCap->croppings[i]);

			break;
	}
}
*/

//-----------------------------------------------------------------------
////////////////////////// USB interface ////////////////////////////////
//-----------------------------------------------------------------------

// USB  periodically calls this function.
// retrieves an operation (when available) FROM the DPS and send it TO the printer (buf_out)
// return:  FALSE: FAILED
//      TRUE: Successful
static BOOL _DpsSendOperation(BYTE * buf_out, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	//periodic
	//_DPSCapabilityUpdate(0);
//  SDWORD  ts;

	if (buf_out == NULL)
	{
		buf_out = psDps->pbXmlBuff;
	}

	if (psDps->stDscStatus.opStatus != OPSTATUS_SCHEDULED)
		return DPS_SUCCESS;		// nothing to do

//MP_DEBUG1("psDps->stDscStatus.opType %d", psDps->stDscStatus.opType); //don't display any large message  Calvin 2004.07.19

	switch (psDps->stDscStatus.opType)
	{
	case OP_CONF_PRINT_SERVICE:	// 0
		dpsReqConfPrintService((char *) buf_out);
		break;

	case OP_GET_CAPABILITY:	// 1
		dpsReqGetCapability((char *) buf_out, psDps->stRequestCapabilityPaperSize, psDps->stRequestCapability);
		break;

	case OP_GET_JOB_STATUS:	// 2
		dpsReqGetJobStatus((char *) buf_out);
		break;

	case OP_GET_DEV_STATUS:	// 3
		dpsReqGetDeviceStatus((char *) buf_out);
		break;

	case OP_START_JOB:			// 4
		//dpsReqStartJob((char *) buf_out, &printJobInfo.jobConf, psDps->stPrintJobInfo.pPrintJobInfo, printJobInfo.printJobInfoCount);		
		dpsReqStartJob((char *) buf_out, &psDps->stPrintJobConfig, psDps->pstPrintInfo, psDps->dwFilePrintCount);
		break;

	case OP_ABORT_JOB:			// 5
		dpsReqAbortJob((char *) buf_out);
		break;

	case OP_CONTINUE_JOB:		// 6
		dpsReqContinueJob((char *) buf_out);
		break;

	default:
		//  TODO: error msg
		return FAIL;
	}

	//  MP_DEBUG1("Send:\n\r%s", (char *)buf_out);
	psDps->stDscStatus.opStatus = OPSTATUS_WAITING_RESP;
	//return strlen((char *) buf_out ); 


	// Now send the request thru PTP
//  PtpSendDPSOpReq(psDps->pbXmlBuff, strlen((char *) psDps->pbXmlBuff) ); 

/* // check later // JL, 05252005
	// wait (blocking) for the response to arrive
	ts = SystemGetTimeStamp();
	while ( psDps->stDscStatus.opStatus != OPSTATUS_GOT_RESP && gSendFlag != FALSE )
	{
		if ( SystemGetElapsedTime(ts) > DPS_TIMEOUT )	// time out
		{
			MP_DEBUG("send op and receive resposnse -- FAIILED");
			psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
			return FAIL;
		}
	}
*/
	//MP_DEBUG("send op and receive resposnse -- compelete");
//  psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;

	return DPS_SUCCESS;
}


// Follow 10.7.1 errorReason Minor Code
static DWORD CheckDpsError(DpsPrinterStatus dpsPrinterStatus, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
    DWORD errCode = NO_ERR;

    if (dpsPrinterStatus.error == DPS_ErrStatus_FatalError)
    {
        errCode = ERR_PICT_FATAL_ERROR;
    }
    else if (dpsPrinterStatus.error == DPS_ErrStatus_Warning)
    {
        errCode = ERR_PICT_WARNING_ERROR;
    }

    if (dpsPrinterStatus.errorReason == DPS_ErrReason_PaperError)
    {
        errCode = ERR_PICT_PAPER;
        switch (psDps->stPrinterStatus.hostDeviceStatus.errorReason & 0x0000FFFF)
        {
            case 0x0100:			// Paper is empty
                errCode = ERR_PICT_PAPER_EMPTY;
                break;
            case 0x0500:			// Paper jam
                errCode = ERR_PICT_PAPER_JAM;
                break;
            case 0x0400:			// Paper media error
                ;//errCode = ERR_PICT_PAPER_MEDIA_ERROR;
                break;
            case 0x0700:			// Paper type/size error
                ;//errCode = ERR_PICT_PAPER_TYE_SIZE_ERROR;
                break;
            default:				// others error related to paper
                errCode = ERR_PICT_PAPER;
                break;
        }
    }
    else if (dpsPrinterStatus.errorReason == DPS_ErrReason_InkError)
    {
        errCode = ERR_PICT_INK;
        switch (psDps->stPrinterStatus.hostDeviceStatus.errorReason & 0x0000FFFF)
        {
            case 0x0100:			// ink is empty
                errCode = ERR_PICT_INK_EMPTY;
                break;
            case 0x0000:			// ink related error
                //if (errCode == ERR_PICT_FATAL_ERROR)
                //errCode = ERR_PICT_INK_FAIL_SEARCH_MARKER;
                //else if (errCode == ERR_PICT_WARNING_ERROR)
                //errCode = ERR_PICT_INK_RIBBON_ERROR;
                break;
            default:				// others error related to ink
                errCode = ERR_PICT_INK;
                break;
        }
    }
    else if (dpsPrinterStatus.errorReason == DPS_ErrReason_HWError)
    {
        errCode = ERR_PICT_HARDWARE;
        switch (psDps->stPrinterStatus.hostDeviceStatus.errorReason & 0x0000FFFF)
        {
            case 0x0100:			// Cover open
                ;//errCode = ERR_PICT_FATAL_ERROR;
                break;
            case 0x0600:			// Cover open
                errCode = ERR_PICT_HW_COVER_OPEN;
                break;
            default:				// others error related to hardware
                errCode = ERR_PICT_HARDWARE;
                break;
        }
    }
    else if (dpsPrinterStatus.errorReason == DPS_ErrReason_FileError)
    {
        errCode = ERR_PICT_FILE;
        switch (psDps->stPrinterStatus.hostDeviceStatus.errorReason & 0x0000FFFF)
        {
            case 0x0200:			// File decode error (picture format of specified image cannot be decoded)
                errCode = ERR_PICT_FILE_DECODE_ERROR;
                break;
            default:				// others error related to hardware
                errCode = ERR_PICT_FILE;
                break;
        }
    }

    if(errCode != NO_ERR)
        MP_ALERT("-USBOTG%d- %s 0x%x", eWhichOtg, __FUNCTION__, errCode);

    return errCode;
}




///-------------------------------------------------------------------//////
////    This function is called periodically
///-------------------------------------------------------------------//////
// repeat: non-zero means start from the begining
// return: FAILED       : unexpected system error
//         COMPLETED    : The request to update capabilities are complete
//         PROGRESSING  : last request in progress
//
#if 0
PrinterStatus _DPSCapabilityUpdate(DWORD repeat)
{
	static int state = 0;		//0;
	static DWORD layoutForPaperSizeIndex = 0;

	if (psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED)
	{
//         if ( state == 6 )
//             return COMPLETED;
//         else
		return PROGRESSING;
	}

	// send configure print service
	switch (state)
	{
	case 0:
		psDps->stDscStatus.opType = OP_CONF_PRINT_SERVICE;
		psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;
		state += 1;
		break;

	case 1:
		// was response received?
		if (psDps->stDscStatus.opResult == okResult)
		{
			state += 1;
		}
		else
		{
			state -= 1;
		}
		break;

	case 2:
		//Turn next printer capability   Calvin 2004.08.10
		switch (psDps->stRequestCapability)
		{
		case 0:
			psDps->stRequestCapability = DPS_QUALITIES;
			break;

		case DPS_QUALITIES:
			psDps->stRequestCapability = DPS_PAPER_SIZES;
			break;

		case DPS_PAPER_SIZES:
			psDps->stRequestCapability = DPS_PAPER_TYPES;
			break;

		case DPS_PAPER_TYPES:
			psDps->stRequestCapability = DPS_FILE_TYPES;
			break;

		case DPS_FILE_TYPES:
			psDps->stRequestCapability = DPS_DATE_PRINTS;
			break;

		case DPS_DATE_PRINTS:
			psDps->stRequestCapability = DPS_FILE_NAME_PRINTS;
			break;

		case DPS_FILE_NAME_PRINTS:
			psDps->stRequestCapability = DPS_IMAGE_OPTIMIZES;
			break;

		case DPS_IMAGE_OPTIMIZES:
			psDps->stRequestCapability = DPS_LAYOUTS;
			psDps->dwLayoutForPaperSizeIdx = 0;
			psDps->stRequestCapabilityPaperSize = psDps->stDpsPrinterCapability.paperSizes[0];
			psDps->dwLayoutForPaperSizeIdx++;
			break;

		case DPS_LAYOUTS:
			if (psDps->dwLayoutForPaperSizeIdx == psDps->stDpsPrinterCapability.paperSizesCount)
			{
				psDps->stRequestCapability = DPS_FIXED_SIZES;
			}
			else
			{
				// keep getting layouts for all paperSizes
				//psDps->stRequestCapability = DPS_LAYOUTS;    
				psDps->stRequestCapabilityPaperSize =
					psDps->stDpsPrinterCapability.paperSizes[psDps->dwLayoutForPaperSizeIdx];
				psDps->dwLayoutForPaperSizeIdx++;
			}
			break;

		case DPS_FIXED_SIZES:
			psDps->stRequestCapability = DPS_CROPPINGS;
			break;

//              case DPS_CROPPINGS:   // end
			// TODO: get papertypes per paper size here?!
//                  break;

		}
		psDps->stDscStatus.opType = OP_GET_CAPABILITY;
		psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;
		state += 1;

		break;

	case 3:
		//Richard 0809
		//MP_DEBUG2("psDps->stDscStatus.opResult=%X   psDps->stRequestCapability=%X",psDps->stDscStatus.opResult ,psDps->stRequestCapability);
		// was response received?
		if (psDps->stDscStatus.opResult == okResult)
		{

			if (psDps->stRequestCapability == DPS_CROPPINGS)
			{
				state += 1;

/* // check later // JL, 05252005
                     // record time stamp
                    psDps->stDpsPrinterCapability.timeStamp = SystemGetTimeStamp();
*/
			}
			else
			{
				state -= 1;
			}
		}
		else
		{
			state -= 1;
		}

		break;

	case 4:
		psDps->stDscStatus.opType = OP_GET_DEV_STATUS;
		psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;
		state += 1;

		break;

	case 5:
		if (psDps->stDscStatus.opResult == okResult)
		{
/* // check later // JL, 05252005
                 // record time stamp
                 psDps->stPrinterStatus.hostDeviceStatus.timeStamp = SystemGetTimeStamp();
*/
			state += 1;
		}
		else					// not OK result
		{
			state -= 1;
		}

		break;

	case 6:
		// reset state
		state = 0;
		return COMPLETED;		// return

	}

	return PROGRESSING;

}
#endif

////// local interfaces used to fill the API structures
static void _DpsFillPrinterSettings(DpsPrinterSettings * printSettings,
									DpsPrinterCapability * capabilities)
{
#if 0
	DWORD i, j;
	PrintLayoutType *printLayout;


	// fixed print sizes
	printSettings->fixedPrintSize.print2x3 = 0;
	printSettings->fixedPrintSize.print3x5 = 0;
	printSettings->fixedPrintSize.print4x6 = 0;
	printSettings->fixedPrintSize.print5x7 = 0;
	printSettings->fixedPrintSize.print8x10 = 0;
	printSettings->fixedPrintSize.fixedDefault = 1;

	for (i = 0; i < capabilities->fixedSizesCount; i++)
	{
		switch (capabilities->fixedSizes[i])
		{
		case size2x3:
			printSettings->fixedPrintSize.print2x3 = 1;
			break;

		case size3x5:
			printSettings->fixedPrintSize.print3x5 = 1;
			break;

		case size4x6:
			printSettings->fixedPrintSize.print4x6 = 1;
			break;

		case size5x7:
			printSettings->fixedPrintSize.print5x7 = 1;
			break;

		case size8x10:
			printSettings->fixedPrintSize.print8x10 = 1;
			break;
		}
	}

	// paper sizes & layouts
	printSettings->paperSize.paper4x6 = 0;
	printSettings->paperSize.paper8x10 = 0;
	printSettings->paperSize.paperA4 = 0;
	printSettings->paperSize.paperA6 = 0;
	printSettings->paperSize.paperCard = 0;
	printSettings->paperSize.paperDefault = 1;
	printSettings->paperSize.paperLetter = 0;

	for (i = 0; i < capabilities->paperSizesCount; i++)
	{
		switch (capabilities->paperSizes[i])
		{
		case paperSizeCard:
			printSettings->paperSize.paperCard = 1;
			printLayout = &(printSettings->printLayoutForCard);
			break;

		case paperSize4x6:
			printSettings->paperSize.paper4x6 = 1;
			printLayout = &(printSettings->printLayoutFor4x6);
			break;

		case paperSize8x10:
			printSettings->paperSize.paper8x10 = 1;
			printLayout = &(printSettings->printLayoutFor8x10);
			break;

		case paperSizeLetter:
			printSettings->paperSize.paperLetter = 1;
			printLayout = &(printSettings->printLayoutForLetter);
			break;

		case paperSizeA4:
			printSettings->paperSize.paperA4 = 1;
			printLayout = &(printSettings->printLayoutForA4);
			break;

		case paperSizeA6:
			printSettings->paperSize.paperA6 = 1;
			printLayout = &(printSettings->printLayoutForA6);
			break;

		case defPaperSize:
			printSettings->paperSize.paperDefault = 1;
			printLayout = &(printSettings->printLayoutForDefault);
			break;

		default:
			printLayout = NULL;
			break;

		}

		if (printLayout != NULL)
		{
			printLayout->print1FullBleed = 0;
			printLayout->print1Up = 0;
			printLayout->print2Up = 0;
			printLayout->print3Up = 0;
			printLayout->print4Up = 0;
			printLayout->print5Up = 0;
			printLayout->print6Up = 0;
			printLayout->print7Up = 0;
			printLayout->print8Up = 0;
			printLayout->printIndex = 0;
			printLayout->printDefault = 1;

			for (j = 0; j < capabilities->layoutsCount[i]; j++)
			{

				switch (capabilities->layouts[i][j])
				{
				case oneUp:
					printLayout->print1Up = 1;
					break;

				case twoUp:
					printLayout->print2Up = 1;
					break;

				case threeUp:
					printLayout->print3Up = 1;
					break;

				case fourUp:
					printLayout->print4Up = 1;
					break;

				case fiveUp:
					printLayout->print5Up = 1;
					break;

				case sixUp:
					printLayout->print6Up = 1;
					break;

				case sevenUp:
					printLayout->print7Up = 1;
					break;

				case indexLayout:
					printLayout->printIndex = 1;
					break;

				case oneFullBleed:
					printLayout->print1FullBleed = 1;
					break;

				case defLayout:
					printLayout->printDefault = 1;
					break;

				}
			}
		}
	}


	//  PaperTypeType       paperTypes
	printSettings->paperTypes.typeDefault = 1;
	printSettings->paperTypes.typePhoto = 0;
	printSettings->paperTypes.typePlain = 0;

	for (i = 0; i < capabilities->paperTypesCount; i++)
	{
		switch (capabilities->paperTypes[i])
		{
		case PlainPaperType:
			printSettings->paperTypes.typePlain = 1;	//Calvin 2004.08.06
			break;

		case PhotoPaperType:
			printSettings->paperTypes.typePhoto = 1;	//Calvin 2004.08.06
			break;
		}

	}

	//  QualityType         quality
	printSettings->quality.qualityDefault = 1;
	printSettings->quality.qualityDraft = 0;
	printSettings->quality.qualityFine = 0;
	printSettings->quality.qualityNormal = 0;

	for (i = 0; i < capabilities->qualitiesCount; i++)
	{
		switch (capabilities->qualities[i])
		{
		case normalQuality:
			printSettings->quality.qualityNormal = 1;
			break;

		case draftQuality:
			printSettings->quality.qualityDraft = 1;
			break;

		case fineQuality:
			printSettings->quality.qualityFine = 1;
			break;
		}
	}

	//  dateprintType         dateprint
	printSettings->datePrint.datePrintDefault = 1;
	printSettings->datePrint.datePrintOff = 0;
	printSettings->datePrint.datePrintOn = 0;

	for (i = 0; i < capabilities->datePrintsCount; i++)
	{
		switch (capabilities->datePrints[i])
		{
		case defDatePrint:
			printSettings->datePrint.datePrintDefault = 1;
			break;

		case dateOff:
			printSettings->datePrint.datePrintOff = 1;
			break;

		case dateOn:
			printSettings->datePrint.datePrintOn = 1;
			break;
		}
	}
#endif
}
#if 0
DWORD _GetQuality(QualityType quality)
{

	if (quality.qualityDraft)
	{
		return draftQuality;
	}
	else if (quality.qualityNormal)
	{
		return normalQuality;
	}
	else if (quality.qualityFine)
	{
		return fineQuality;
	}

	return defQuality;
}

DWORD _GetPaperType(PaperTypeType paperType)
{
	if (paperType.typePhoto)
	{
		return PhotoPaperType;
	}
	else if (paperType.typePlain)
	{
		return PlainPaperType;
	}
	else						// default or nothing is set
	{
		return defPaperType;
	}
}


DWORD _GetPaperSize(PaperSizeType paperSize)
{
	if (paperSize.paper4x6)
	{
		return paperSize4x6;
	}
	else if (paperSize.paper8x10)
	{
		return paperSize8x10;
	}
	else if (paperSize.paperA4)
	{
		return paperSizeA4;
	}
	else if (paperSize.paperA6)
	{
		return paperSizeA6;
	}
	else if (paperSize.paperCard)
	{
		return paperSizeCard;
	}
	else if (paperSize.paperLetter)
	{
		return paperSizeLetter;
	}
	else
	{
		return defPaperSize;
	}
}

//if selected papar size have lay-out ,it will return lay-out(default: Borderless<oneFullBleed>)  Calvin 2004.08.05
DWORD _GetLayout(DpsPrinterSettings * settings, PaperSizeType paperSize)
{
	PrintLayoutType printLayout;

	if (paperSize.paperLetter)
	{
		printLayout = settings->printLayoutForLetter;
	}
	else if (paperSize.paper4x6)
	{
		printLayout = settings->printLayoutFor4x6;
	}
	else if (paperSize.paper8x10)
	{
		printLayout = settings->printLayoutFor8x10;
	}
	else if (paperSize.paperA4)
	{
		printLayout = settings->printLayoutForA4;
	}
	else if (paperSize.paperA6)
	{
		printLayout = settings->printLayoutForA6;
	}
	else if (paperSize.paperCard)
	{
		printLayout = settings->printLayoutForCard;
	}
	else						// paperSize.paperDefault or anything else
	{
		printLayout = settings->printLayoutForDefault;
	}

	// Layout
	if (printLayout.print1FullBleed)
	{
		return oneFullBleed;
	}
	else if (printLayout.printDefault)
	{
		return defLayout;
	}
	else if (printLayout.print1Up)
	{
		return oneUp;
	}
	else if (printLayout.print2Up)
	{
		return twoUp;
	}
	else if (printLayout.print3Up)
	{
		return threeUp;
	}
	else if (printLayout.print4Up)
	{
		return fourUp;
	}
	else if (printLayout.print5Up)
	{
		return fiveUp;
	}
	else if (printLayout.print6Up)
	{
		return sixUp;
	}
	else if (printLayout.print7Up)
	{
		return sevenUp;
	}
	else if (printLayout.print8Up)
	{
		return eightUp;
	}
	else if (printLayout.printIndex)
	{
		return indexLayout;
	}
	else
	{
		return noLayout;
	}
}


DWORD _GetFixedSize(FixedPrintSizeType fixedSize)
{
	if (fixedSize.fixedDefault)
	{
		return defFixedSize;
	}
	else if (fixedSize.print2x3)
	{
		return size2x3;
	}
	else if (fixedSize.print3x5)
	{
		return size3x5;
	}
	else if (fixedSize.print4x6)
	{
		return size4x6;
	}
	else if (fixedSize.print5x7)
	{
		return size5x7;
	}
	else if (fixedSize.print8x10)
	{
		return size8x10;
	}
	else if (fixedSize.print4x6)
	{
		return size4x6;
	}

	return noFixedSize;
}

//Date Print    Calvin 2004.08.05
DWORD _GetDatePrint(DatePrintType dateprint)
{
	if (dateprint.datePrintDefault)
	{
		return defDatePrint;
	}
	else if (dateprint.datePrintOff)
	{
		return dateOff;
	}
	else if (dateprint.datePrintOn)
	{
		return dateOn;
	}

	return noDatePrint;
}


// Second Test Error via USB 
//----------------------------------------------------------------------------------------------
// ---------------- interfaces used by the UI to drive the printing process --------------------
//----------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------
//of  DpsGetPrintSettings
//o
//o This function returns the printer Capabilities annd the 
//o
//o Input:
//o     settings: pointer to DpsPrinterSettings that will be filled with the printer Capabilities
//o
//o Return Value:
//o     DPS_SUCCESS: if the process of getting the printer Capabilities was successful
//o     FAILURE: otherwise
//---------------------------------------------------------------------------
BOOL DpsGetPrintSettings(DpsPrinterSettings * settings)
{
//    SDWORD ts;
	BOOL rt = DPS_SUCCESS;

//check later // JL, 05252005//    ts = SystemGetTimeStamp();

	// This is the first thing to be called
	psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;

	//Get Printer Capability
	while (_DPSCapabilityUpdate(0) == PROGRESSING)
	{
		if (_DpsSendOperation(psDps->pbXmlBuff) == FAIL)
		{
			MP_DEBUG("DpsGetPrintSettings FAILURE");
			rt = FAILURE;
			break;
		}

		/* // check later // JL, 05252005
		   // 30 second timeout
		   if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
		   {
		   PrintErrorMessage(); //Error message wait for installing DPS driver for Test Tool Calvin 2004.08.11
		   MP_DEBUG("time-out");
		   rt = FAILURE;
		   break;
		   }
		 */
	}

	//Fill Printer Settings 
	_DpsFillPrinterSettings(settings, &psDps->stDpsPrinterCapability);

#if 0							//MP_DEBUG MESSAGE    don't delete
	MP_DEBUG1("paperSize				= %08x", *((DWORD *) (&(settings->paperSize))));
	MP_DEBUG1("fixedPrintSize			= %08x", *((DWORD *) (&(settings->fixedPrintSize))));
	MP_DEBUG1("paperTypes			= %08x", *((DWORD *) (&(settings->paperTypes))));
	MP_DEBUG1("quality				= %08x", *((DWORD *) (&(settings->quality))));
	MP_DEBUG1("printLayoutFor4x6		= %08x", *((DWORD *) (&(settings->printLayoutFor4x6))));
	MP_DEBUG1("printLayoutFor8x10		= %08x", *((DWORD *) (&(settings->printLayoutFor8x10))));
	MP_DEBUG1("printLayoutForA4		= %08x", *((DWORD *) (&(settings->printLayoutForA4))));
	MP_DEBUG1("printLayoutForA6		= %08x", *((DWORD *) (&(settings->printLayoutForA6))));
	MP_DEBUG1("printLayoutForCard		= %08x", *((DWORD *) (&(settings->printLayoutForCard))));
	MP_DEBUG1("printLayoutForDefault	= %08x", *((DWORD *) (&(settings->printLayoutForDefault))));
	MP_DEBUG1("printLayoutForLetter		= %08x", *((DWORD *) (&(settings->printLayoutForLetter))));
	MP_DEBUG1("datePrint				= %08x", *((DWORD *) (&(settings->datePrint))));
#endif

	return rt;
}
#endif

//---------------------------------------------------------------------------
//of  DpsGetPrinterStatus
//o
//o This function reports the printer status
//o
//o Input:
//o     none
//o
//o Return Value:
//o     DpsPrinterStatus: If the available printer status is expired it'll request a status from the printer
//---------------------------------------------------------------------------
static void DpsGetPrinterStatus(DpsPrinterStatus * pDpsPrinterStatus, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);    
	//DpsPrinterStatus   printStatus;
	//  SDWORD ts;
	//  BOOL sendStatus;

//SEBUG3("0x%.8x 0x%.8x 0x%.8x",psDps->stPrinterStatus.hostDeviceStatus.jobEndReason, printerStatus.hostDeviceStatus.printServiceStatus,printerStatus.hostDeviceStatus.errorStatus);

	pDpsPrinterStatus->sessionStatus = TRUE;
/* // check later // JL, 05252005
    // get a new status if needed
    if ( SystemGetElapsedTime( psDps->stPrinterStatus.hostDeviceStatus.timeStamp ) > DPS_DEVICE_STATUS_EXPIRY )
    {
        // Wait for DPS to be ready, then start task.   
        ts = SystemGetTimeStamp();
        while ( psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED )
        {
            if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
            {
                printStatus.error = DPS_ErrStatus_FatalError;
                MP_DEBUG("2");
                return printStatus;	   // return system error
            }
        }
        psDps->stDscStatus.opType    = OP_GET_DEV_STATUS;
        psDps->stDscStatus.opStatus  = OPSTATUS_SCHEDULED;

        if ( (sendStatus=_DpsSendOperation(NULL)) == FAIL || psDps->stDscStatus.opResult != okResult)
        {
            printStatus.error = DPS_ErrStatus_FatalError;

            // if PTP session is closed, report it
            if ( sendStatus == FAIL && gSendFlag == FALSE)
            {
                printStatus.sessionStatus = FALSE;
            }
            return printStatus; // return system error
        }

        //MP_DEBUG("Got device status OK");
    }
*/

	// error status
	switch (psDps->stPrinterStatus.hostDeviceStatus.errorStatus)
	{
	case 0x72000000:			// no error
		pDpsPrinterStatus->error = DPS_ErrStatus_NoError;
		break;
	case 0x72010000:			// warning
		pDpsPrinterStatus->error = DPS_ErrStatus_Warning;
		break;
	case 0x72020000:			// fatal error
		pDpsPrinterStatus->error = DPS_ErrStatus_FatalError;
		break;
	}

	// error reason
	switch (psDps->stPrinterStatus.hostDeviceStatus.errorReason & 0xFFFF0000)
	{
	case 0x73000000:			// no reason
		pDpsPrinterStatus->errorReason = DPS_ErrReason_NoReason;
		break;
	case 0x73010000:			// paper related error
		pDpsPrinterStatus->errorReason = DPS_ErrReason_PaperError;
		break;
	case 0x73020000:			// ink related error                                    
		pDpsPrinterStatus->errorReason = DPS_ErrReason_InkError;
		break;
	case 0x73030000:			// hardware related error                
		pDpsPrinterStatus->errorReason = DPS_ErrReason_HWError;
		break;
	case 0x73040000:			// file related error                
		pDpsPrinterStatus->errorReason = DPS_ErrReason_FileError;
		break;
	}

	// status
	switch (psDps->stPrinterStatus.hostDeviceStatus.printServiceStatus)
	{
	case 0x70000000:			// printing
		pDpsPrinterStatus->status = DPS_PrintStatus_Printing;
		break;
	case 0x70010000:			// idle
		pDpsPrinterStatus->status = DPS_PrintStatus_Idle;
		break;
	case 0x70020000:			// pause
		pDpsPrinterStatus->status = DPS_PrintStatus_Paused;
		break;
	}

	// jobEnded
	if (psDps->stPrinterStatus.hostDeviceStatus.jobEndReason == 0x71000000)
	{
		pDpsPrinterStatus->jobEnded = DPS_JobEnd_NotEnded;
	}
	else if (psDps->stPrinterStatus.hostDeviceStatus.jobEndReason == 0x71020000
			 || psDps->stPrinterStatus.hostDeviceStatus.jobEndReason == 0x71030000)
	{
		pDpsPrinterStatus->jobEnded = DPS_JobEnd_AbortJob_Immediate;
	}
	else
	{
		pDpsPrinterStatus->jobEnded = DPS_JobEnd_Ended;
	}

	// errorReason
	//pDpsPrinterStatus->errorReason  = psDps->stPrinterStatus.hostDeviceStatus.errorReason;

	//MP_DEBUG3("0x%.8x 0x%.8x 0x%.8x",psDps->stPrinterStatus.hostDeviceStatus.jobEndReason, printerStatus.hostDeviceStatus.printServiceStatus,printerStatus.hostDeviceStatus.errorStatus);

	return;						//  printStatus;
}
/*
//---------------------------------------------------------------------------
//of  DpsGetJobStatus
//o
//o This function reports the progress of the submitted print job
//o
//o Input:
//o     none
//o
//o Return Value:
//o     DpsPrintJobStatus: the 'isValid' field needs to be TRUE if such information is avalable
//---------------------------------------------------------------------------
void DpsGetJobStatus(DpsPrintJobStatus * pDpsPrintJobStatus)
{
	//DpsPrintJobStatus jobStatus;

	// wait for events (don't make a request)
//check later// JL, 05252005//  if ( SystemGetElapsedTime( psDps->stPrintJobInfo.jobStatus.timeStamp )  < DPS_JOB_STATUS_EXPIRY )
	if (1)
	{
		pDpsPrintJobStatus->currImage = psDps->stPrintJobInfo.jobStatus.progress[0];

		pDpsPrintJobStatus->totalImage = psDps->stPrintJobInfo.jobStatus.progress[1];

		pDpsPrintJobStatus->isValid = TRUE;
	}
	else
	{
		pDpsPrintJobStatus->isValid = FALSE;
	}
	return;						//   jobStatus;
}
*/

//
//  PictureBridge Test
//









#if 0
//---------------------------------------------------------------------------
//of  DpsStartPrintJob
//o
//o This function is used to submit a print job to a PictBridge Printer. This should happen after getting
//       the printer's Capability.
//o
//o Input:
//o     jobSettings : pointer to the DpsPrinterSettings of the JOB. This structure should be filled properly to
//o                   match the job but not to set anything the is not supported by the PRINTER's DpsPrinterSettings.
//o     files       : Pointer to the DpsPrintFile array of the files to be printed.
//o     count       : is the number of valid elements in files.
//o
//o Return Value:
//o     BOOL:
//o         TRUE: if the job was submitted and accepted by the printer.
//o         FALSE: otherwise
//---------------------------------------------------------------------------
BOOL DpsStartPrintJob(DpsPrinterSettings * jobSettings, DpsPrintFile * files, DWORD count)
{
	//static DpsPrintJobInfo  printInfo[MAX_FILE_TO_PRINT];       // structure per file
	//static DpsPrintJobInfo*  printInfo;
	static DpsPrintJobInfo printInfo;
	int i = 0;

//  SDWORD ts;
	BOOL rt;

// JL, 05252005:only supports one picture only, so far
	count = 1;

	// filling printJobInfo.jobConf
	psDps->stPrintJobInfo.jobConf.datePrint = _GetDatePrint(jobSettings->datePrint);	//Calvin 2004.08.05
	psDps->stPrintJobInfo.jobConf.fileNamePrt = noNamePrint;
	psDps->stPrintJobInfo.jobConf.imageOpt = noImageOpt;
	psDps->stPrintJobInfo.jobConf.fileType = exifJpeg;	//dpof
	psDps->stPrintJobInfo.jobConf.cropping = noCropping;

	psDps->stPrintJobInfo.jobConf.quality = noQuality;	//_GetQuality(jobSettings->quality);    //noQuality;
	psDps->stPrintJobInfo.jobConf.paperType = _GetPaperType(jobSettings->paperTypes);	//noPaperType;
	psDps->stPrintJobInfo.jobConf.paperSize = _GetPaperSize(jobSettings->paperSize);	//noPaperSize;
	//check MenuData(LayOut)
	psDps->stPrintJobInfo.jobConf.layout = _GetLayout(jobSettings, jobSettings->paperSize);	//index;
	//  psDps->stPrintJobInfo.jobConf.fixedSize  = defFixedSize;         //size2x3
	psDps->stPrintJobInfo.jobConf.fixedSize = _GetFixedSize(jobSettings->fixedPrintSize);	//noFixedSize;

	// psDps->stPrintJobInfo.printJobInfoCount
	psDps->stPrintJobInfo.printJobInfoCount = count;

#if 0							//Message
	MP_DEBUG1("datePrint   %.8x", psDps->stPrintJobInfo.jobConf.datePrint);
	MP_DEBUG1("paperType %.8x", psDps->stPrintJobInfo.jobConf.paperType);
	MP_DEBUG1("paperSize  %.8x", psDps->stPrintJobInfo.jobConf.paperSize);
	MP_DEBUG1("layout        %.8x", psDps->stPrintJobInfo.jobConf.layout);
	MP_DEBUG1("fixedSize    %.8x\n\n", psDps->stPrintJobInfo.jobConf.fixedSize);
#endif
/* check later//JL, 05252005:only supports one picture only, so far
	//alloc memory  Calvin
	printInfo = (DpsPrintJobInfo*)SystemAllocateMemory(count*sizeof(DpsPrintJobInfo));
	// psDps->stPrintJobInfo.printJobInfo
	for (i=0; i<count; i++)
	{
		printInfo[i].copies = files[i].copies ;
		//printInfo[i].fileID = PtpGetFileID(files[i].fileName); //mark Calvin 2004.07.22
		printInfo[i].fileID = files[i].fileID; //Calvin 2004.07.22
		printInfo[i].fileName[i]    = '\0'; //No file print option calvin 2004.08.06
		strcpy(printInfo[i].fileDate,files[i].fileDate);
	}
*/
	printInfo.copies = files[i].copies;
	//printInfo[i].fileID = PtpGetFileID(files[i].fileName); //mark Calvin 2004.07.22
	printInfo.fileID = files[i].fileID;	//Calvin 2004.07.22
	printInfo.fileName[i] = '\0';	//No file print option calvin 2004.08.06
	strcpy(printInfo.fileDate, files[i].fileDate);
	psDps->stPrintJobInfo.pPrintJobInfo = &printInfo;

/* // check later // JL, 05252005
	// Wait for DPS to be ready, then start task.   
	ts = SystemGetTimeStamp();


	while ( psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED )
	{
	    if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
	    {
			MP_DEBUG("Timed out");
			return FAILURE;
	    }
	}
*/
	psDps->stDscStatus.opType = OP_START_JOB;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

	rt = DPS_SUCCESS;
	if (_DpsSendOperation(NULL) == FAIL)
	{
		rt = FAILURE;
	}
	if (psDps->stDscStatus.opResult == okResult && rt != FAILURE)
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job accepted OK");
		psDps->stPrintJobInfo.jobStatus.timeStamp = 0;	// to force updating the job status
		rt = DPS_SUCCESS;
	}
	else
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job send failed");
		rt = FAILURE;
	}
	return rt;
}

//---------------------------------------------------------------------------
//of  Test_DpsStartPrintJob
//o
//o This function is used to submit a print job to a PictBridge Printer. This should happen after getting
//       the printer's Capability.
//o
//o Input:
//o     jobSettings : pointer to the DpsPrinterSettings of the JOB. This structure should be filled properly to
//o                   match the job but not to set anything the is not supported by the PRINTER's DpsPrinterSettings.
//o     files       : Pointer to the DpsPrintFile array of the files to be printed.
//o     count       : is the number of valid elements in files.
//o
//o Return Value:
//o     BOOL:
//o         TRUE: if the job was submitted and accepted by the printer.
//o         FALSE: otherwise
//---------------------------------------------------------------------------
BOOL Test_DpsStartPrintJob(void)
{
	//static DpsPrintJobInfo  printInfo[MAX_FILE_TO_PRINT];       // structure per file
	//static DpsPrintJobInfo*  printInfo;
	static DpsPrintJobInfo printInfo;
	int i = 0;

//  SDWORD ts;
	BOOL rt;

// JL, 05252005:only supports one picture only, so far
//  count = 1;

	// filling printJobInfo.jobConf
	psDps->stPrintJobInfo.jobConf.datePrint = dateOff;	//_GetDatePrint(jobSettings->datePrint); //Calvin 2004.08.05
	psDps->stPrintJobInfo.jobConf.fileNamePrt = noNamePrint;
	psDps->stPrintJobInfo.jobConf.imageOpt = noImageOpt;
	psDps->stPrintJobInfo.jobConf.fileType = exifJpeg;	//dpof
	psDps->stPrintJobInfo.jobConf.cropping = noCropping;

	psDps->stPrintJobInfo.jobConf.quality = noQuality;	//_GetQuality(jobSettings->quality);    //noQuality;
	psDps->stPrintJobInfo.jobConf.paperType = defPaperType;	//_GetPaperType(jobSettings->paperTypes);   //noPaperType;
	psDps->stPrintJobInfo.jobConf.paperSize = defPaperSize;	//_GetPaperSize(jobSettings->paperSize);    //noPaperSize;
	//check MenuData(LayOut)
	psDps->stPrintJobInfo.jobConf.layout = defLayout;	//_GetLayout(jobSettings, jobSettings->paperSize);      //index;
	//  psDps->stPrintJobInfo.jobConf.fixedSize  = defFixedSize;         //size2x3
	psDps->stPrintJobInfo.jobConf.fixedSize = noFixedSize;	//_GetFixedSize(jobSettings->fixedPrintSize);   //noFixedSize;

	// psDps->stPrintJobInfo.printJobInfoCount
	psDps->stPrintJobInfo.printJobInfoCount = 1;

#if 0							//Message
	MP_DEBUG1("datePrint   %.8x", psDps->stPrintJobInfo.jobConf.datePrint);
	MP_DEBUG1("paperType %.8x", psDps->stPrintJobInfo.jobConf.paperType);
	MP_DEBUG1("paperSize  %.8x", psDps->stPrintJobInfo.jobConf.paperSize);
	MP_DEBUG1("layout        %.8x", psDps->stPrintJobInfo.jobConf.layout);
	MP_DEBUG1("fixedSize    %.8x\n\n", psDps->stPrintJobInfo.jobConf.fixedSize);
#endif
/* check later//JL, 05252005:only supports one picture only, so far
	//alloc memory  Calvin
	printInfo = (DpsPrintJobInfo*)SystemAllocateMemory(count*sizeof(DpsPrintJobInfo));
	// psDps->stPrintJobInfo.printJobInfo
	for (i=0; i<count; i++)
	{
		printInfo[i].copies = files[i].copies ;
		//printInfo[i].fileID = PtpGetFileID(files[i].fileName); //mark Calvin 2004.07.22
		printInfo[i].fileID = files[i].fileID; //Calvin 2004.07.22
		printInfo[i].fileName[i]    = '\0'; //No file print option calvin 2004.08.06
		strcpy(printInfo[i].fileDate,files[i].fileDate);
	}
*/
	printInfo.copies = 0;		//files[i].copies ;
	//printInfo[i].fileID = PtpGetFileID(files[i].fileName); //mark Calvin 2004.07.22
	printInfo.fileID = 0;		//files[i].fileID; //Calvin 2004.07.22
	printInfo.fileName[i] = '\0';	//No file print option calvin 2004.08.06
	//  strcpy(printInfo.fileDate,files[i].fileDate);
	memset(printInfo.fileDate, 0, 12);
	psDps->stPrintJobInfo.pPrintJobInfo = &printInfo;

/* // check later // JL, 05252005
	// Wait for DPS to be ready, then start task.   
	ts = SystemGetTimeStamp();


	while ( psDps->stDscStatus.opStatus != OPSTATUS_NO_SCHEDULED )
	{
	    if (SystemGetElapsedTime(ts) > DPS_TIMEOUT)
	    {
			MP_DEBUG("Timed out");
			return FAILURE;
	    }
	}
*/
	psDps->stDscStatus.opType = OP_START_JOB;
	psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

	rt = DPS_SUCCESS;
	if (_DpsSendOperation(NULL) == FAIL)
	{
		rt = FAILURE;
	}
	if (psDps->stDscStatus.opResult == okResult && rt != FAILURE)
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job accepted OK");
		psDps->stPrintJobInfo.jobStatus.timeStamp = 0;	// to force updating the job status
		rt = DPS_SUCCESS;
	}
	else
	{
		psDps->stDscStatus.opStatus = OPSTATUS_NO_SCHEDULED;
		//MP_DEBUG("job send failed");
		rt = FAILURE;
	}
	return rt;
}
void PictureBridgeTest_state_2(void)
{
//  MP_DEBUG("PictureBridgeTest_state_2");
	if (psDps->stDscStatus.opResult == okResult)
	{
		if (psDps->stRequestCapability == DPS_CROPPINGS)
		{
			MP_DEBUG("DPS_CROPPINGS");
			psDps->stDscStatus.opType = OP_GET_DEV_STATUS;
			psDps->stDscStatus.opStatus = OPSTATUS_SCHEDULED;

/* // check later // JL, 05252005
             // record time stamp
            psDps->stDpsPrinterCapability.timeStamp = SystemGetTimeStamp();
*/
			_DpsSendOperation(psDps->pbXmlBuff);
			//Fill Printer Settings 
			_DpsFillPrinterSettings(&psDps->stPrinterSettings, &psDps->stDpsPrinterCapability);
		}
		else
		{
			PictureBridgeTest_state_1();
		}
	}
	else
	{
		PictureBridgeTest_state_1();
	}
}

BOOL PictureBridgeTest_state_3(void)
{
	BOOL ret = FALSE;

	if (psDps->stDscStatus.opResult == okResult)
	{
		_DpsSendOperation(psDps->pbXmlBuff);
		//Fill Printer Settings 
		_DpsFillPrinterSettings(&psDps->stPrinterSettings, &psDps->stDpsPrinterCapability);
	}
	else
	{
		PictureBridgeTest_state_2();
		ret = TRUE;
	}

	return ret;
}

extern BOOL gTest_StartJob_Process;
BOOL PictureBridgeTest_state_4(void)
{
	BOOL ret = FALSE;

	gTest_StartJob_Process = TRUE;
	return ret;
}

BOOL PictureBridgeTest(void)
{
	BOOL ret = FALSE;

	switch (psDps->stDscStatus.opType)
	{
	case OP_CONF_PRINT_SERVICE:
		MP_DEBUG("OP_CONF_PRINT_SERVICE");
		PictureBridgeTest_state_1();
		ret = TRUE;
		break;

	case OP_GET_CAPABILITY:
		MP_DEBUG("OP_GET_CAPABILITY");
		PictureBridgeTest_state_2();
		ret = TRUE;
		break;

	case OP_GET_JOB_STATUS:
		MP_DEBUG("OP_GET_JOB_STATUS");
		break;
	case OP_GET_DEV_STATUS:
		MP_DEBUG("OP_GET_DEV_STATUS");
		ret = PictureBridgeTest_state_3();
		gTest_StartJob_Process = TRUE;
		//ret = TRUE;
		break;
	case OP_START_JOB:
		MP_DEBUG("OP_START_JOB");
		//  PictureBridgeTest_state_4();
		ret = TRUE;
		break;
	case OP_ABORT_JOB:
		MP_DEBUG("OP_ABORT_JOB");
		break;
	case OP_CONTINUE_JOB:
		MP_DEBUG("OP_CONTINUE_JOB");
		break;

	default:
		MP_DEBUG("Nothing");
		break;
	}

	return ret;
}
#endif // 0

void DpsPrintInfoAlloc(DWORD dwCnt, WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps      = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    DWORD   dwInfos = 0;     

    DpsPrintInfoFree(eWhichOtg);

    if(dwCnt == 0) return;

    dwInfos = (dwCnt > MAX_DEV_NUM_OF_PHOTOS)?MAX_DEV_NUM_OF_PHOTOS:dwCnt;
    MP_DEBUG("-USBOTG%d- %s Print Info:%d", eWhichOtg, __FUNCTION__, dwInfos);  

    psDps->pstPrintInfo = (DpsPrintJobInfo*) ker_mem_malloc(sizeof(DpsPrintJobInfo)*dwInfos, UsbOtgDeviceTaskIdGet(eWhichOtg));    
    if (psDps->pstPrintInfo == 0)
        MP_ALERT("--E-- USBOTG%d %s PrintInfo is not enough memory", eWhichOtg, __FUNCTION__); 
    else
        MP_DEBUG("-USBOTG%d- PrintInfo alloc 0x%x", eWhichOtg, psDps->pstPrintInfo);       

}

void DpsPrintInfoFree(WHICH_OTG eWhichOtg)
{
    PPICT_BRIDGE_DPS psDps      = (PPICT_BRIDGE_DPS)UsbOtgDevSidcPtpDpsGet(eWhichOtg);
    DWORD   dwInfos = 0;

    if (psDps->pstPrintInfo != 0) 
    {
        //MP_DEBUG("-USBOTG%d- %s pstPrintInfo is not Free", eWhichOtg, __FUNCTION__);
        ker_mem_free (psDps->pstPrintInfo);    
        psDps->pstPrintInfo = 0;
    }
}

#endif // SC_USBDEVICE

