
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section 
*/
#include "dps_common.h"
#include "mpTrace.h"
#include "..\libsrc\file\include\file.h"


#include <string.h>


#if USBOTG_DEVICE_SIDC

typedef enum
{
	tagOpen,
	tagClose,
	tagOpCl,
	tagMid
} TagType;

typedef struct
{
	TagType tagType;			//open, close, opcl, middle
	char *tagOpen;				// <
	char *tagClose;				// >
	char *param1;				// location of the first letter of the parameter inside the <1st prakets>
} ParamPosition;



static const char xmlHeader[] =
	"<?xml version=\"1.0\"?>\n\r<dps xmlns=\"http://www.cipa.jp/dps/schema/\">\n\r";
const static char xmlOpenInput[] = "<input>";
const static char xmlCloseInput[] = "</input>\n\r";
const static char xmlOpenOutput[] = "<output>";
const static char xmlCloseOutput[] = "</output>\n\r";
const static char xmlClose[] = "</dps>\n\r";

//
const static char xmlOpenConfServ[] = "<configurePrintService>";
const static char xmlCloseConfServ[] = "</configurePrintService>\n\r";
const static char xmlOpenDpsVer[] = "<dpsVersions>";
const static char xmlCloseDpsVer[] = "</dpsVersions>\n\r";
const static char xmlOpenvendName[] = "<vendorName>";
const static char xmlCloseVendName[] = "</vendorName>\n\r";
const static char xmlOpenVendVer[] = "<vendorSpecificVersion>";
const static char xmlCloseVendVer[] = "</vendorSpecificVersion>\n\r";
const static char xmlOpenProdName[] = "<productName>";
const static char xmlCloseProdName[] = "</productName>\n\r";
const static char xmlOpenSerialNum[] = "<serialNo>";
const static char xmlCloseSerialNum[] = "</serialNo>\n\r";

//
const static char xmlOpenGetCapability[] = "<getCapability>";
const static char xmlCloseGetCapability[] = "</getCapability>\n\r";
const static char xmlOpenCapability[] = "<capability>";
const static char xmlCloseCapability[] = "</capability>\n\r";

//const static char xmlOpenQualities[] = "<qualities>";
const static char xmlOpClQualities[] = "<qualities/>\n\r";

//const static char xmlCloseQualities[] = "</qualities>\n";
//const static char xmlOpenPaperSizes[] = "<paperSizes>";
const static char xmlOpClPaperSizes[] = "<paperSizes/>\n\r";

//const static char xmlClosePaperSizes[] = "</paperSizes>\n";
const static char xmlOpenPaperTypes[] = "<paperTypes>";
const static char xmlOpClPaperTypes[] = "<paperTypes/>\n\r";

//const static char xmlClosePaperTypes[] = "</paperTypes>\n";
//const static char xmlOpenFileTypes[] = "<fileTypes>";
const static char xmlOpClFileTypes[] = "<fileTypes/>\n\r";

//const static char xmlCloseFileTypes[] = "<\fileTypes>\n";
//const static char xmlOpenDatePrints[] = "<datePrints>";
const static char xmlOpClDatePrints[] = "<datePrints/>\n\r";

//const static char xmlCloseDatePrints[] = "</datePrints>\n";
//const static char xmlOpenFileNamePrints[] = "<fileNamePrints>";
const static char xmlOpClFileNamePrints[] = "<fileNamePrints/>\n\r";

//const static char xmlCloseFileNamePrints[] = "</fileNamePrints>\n";
//const static char xmlOpenImageOptimizes[] = "<imageOptimizes>";
const static char xmlOpClImageOptimizes[] = "<imageOptimizes/>\n\r";

//const static char xmlCloseImageOptimizes[] = "</imageOptimizes>\n";
const static char xmlOpenLayouts[] = "<layouts>";
//const static char xmlOpClLayouts[] = "<layouts/>\n\r";

//const static char xmlCloseLayouts[] = "</layouts>\n";
//const static char xmlOpenFixedSizes[] = "<fixedSizes>";
const static char xmlOpClFixedSizes[] = "<fixedSizes/>\n\r";

//const static char xmlCloseFixedSizes[] = "</fixedSizes>\n";
//const static char xmlOpenCroppings[] = "<croppings>";
const static char xmlOpClCroppings[] = "<croppings/>\n\r";

//const static char xmlCloseCroppings[] = "</croppings>\n";
//
const static char xmlOpClGetJobStatus[] = "<getJobStatus/>\n\r";
//const static char xmlOpenGetJobStatus[] = "<getJobStatus>";

//
const static char xmlOpClGetDevStatus[] = "<getDeviceStatus/>\n\r";
//const static char xmlOpenGetDevStatus[] = "<getDeviceStatus>";

//
const static char xmlOpenStartJob[] = "<startJob>";
const static char xmlCloseStartJob[] = "</startJob>\n\r";
const static char xmlOpenJobConfig[] = "<jobConfig>";
const static char xmlCloseJobConfig[] = "</jobConfig>\n\r";

//
const static char xmlOpenQuality[] = "<quality>";

//const static char xmlOpClQuality[] = "<quality/>\n";
const static char xmlCloseQuality[] = "</quality>\n\r";
const static char xmlOpenPaperSize[] = "<paperSize>";

//const static char xmlOpClPaperSize[] = "<paperSize/>\n";
const static char xmlClosePaperSize[] = "</paperSize>\n\r";
const static char xmlOpenPaperType[] = "<paperType>";

//const static char xmlOpClPaperType[] = "<paperType/>\n";
const static char xmlClosePaperType[] = "</paperType>\n\r";
const static char xmlOpenFileType[] = "<fileType>";

//const static char xmlOpClFileType[] = "<fileType/>\n";
const static char xmlCloseFileType[] = "</fileType>\n\r";
const static char xmlOpenDatePrint[] = "<datePrint>";

//const static char xmlOpClDatePrint[] = "<datePrint/>\n";
const static char xmlCloseDatePrint[] = "</datePrint>\n\r";
const static char xmlOpenFileNamePrint[] = "<fileNamePrint>";

//const static char xmlOpClFileNamePrint[] = "<fileNamePrint/>\n";
const static char xmlCloseFileNamePrint[] = "</fileNamePrint>\n\r";
const static char xmlOpenImageOptimize[] = "<imageOptimize>";

//const static char xmlOpClImageOptimize[] = "<imageOptimize/>\n";
const static char xmlCloseImageOptimize[] = "</imageOptimize>\n\r";
const static char xmlOpenLayout[] = "<layout>";

//const static char xmlOpClLayout[] = "<layout/>\n";
const static char xmlCloseLayout[] = "</layout>\n\r";
const static char xmlOpenFixedSize[] = "<fixedSize>";

//const static char xmlOpClFixedSize[] = "<fixedSize/>\n";
const static char xmlCloseFixedSize[] = "</fixedSize>\n\r";
const static char xmlOpenCropping[] = "<cropping>";

//const static char xmlOpClCropping[] = "<cropping/>\n";
const static char xmlCloseCropping[] = "</cropping>\n\r";

//
const static char xmlOpenPrintInfo[] = "<printInfo>";
const static char xmlClosePrintInfo[] = "</printInfo>\n\r";
const static char xmlOpenCroppingArea[] = "<croppingArea>";
const static char xmlCloseCroppingArea[] = "</croppingArea>\n\r";
const static char xmlOpenFileID[] = "<fileID>";
const static char xmlCloseFileID[] = "</fileID>\n\r";
const static char xmlOpenFileName[] = "<fileName>";
const static char xmlCloseFileName[] = "</fileName>\n\r";
const static char xmlOpenDate[] = "<date>";
const static char xmlCloseDate[] = "</date>\n\r";
const static char xmlOpenCopies[] = "<copies>";
const static char xmlCloseCopies[] = "</copies>\n\r";
//const static char xmlOpenPrtPID[] = "<prtPID>";
//const static char xmlClosePrtPID[] = "</prtPID>\n\r";
//const static char xmlOpenFilePath[] = "<filePath>";
//const static char xmlCloseFilePath[] = "</filePath>\n\r";
//const static char xmlOpenCopyID[] = "<copyID>";
//const static char xmlCloseCopyID[] = "</copyID>\n\r";

//
const static char xmlOpenAbortJob[] = "<abortJob>";
const static char xmlCloseAbortJob[] = "</abortJob>\n\r";
const static char xmlOpenAbortStyle[] = "<abortStyle>";
const static char xmlCloseAbortStyle[] = "</abortStyle>\n\r";

//
const static char xmlOpClContJob[] = "<continueJob/>\n\r";

//
//
const static char xmlOpenResult[] = "<result>";
const static char xmlCloseResult[] = "</result>\n\r";
const static char xmlOpClNotifyJobStatus[] = "<notifyJobStatus/>\n\r";
const static char xmlOpClNotifyDevStatus[] = "<notifyDeviceStatus/>\n\r";
//const static char xmlOpenGetFileID[] = "<getFileID>";
//const static char xmlCloseGetFileID[] = "</getFileID>\n\r";

//const static char xmlOpenFileID[] = "<fileID>";
//const static char xmlCloseFileID[] = "</fileID>\n";
//

/*
// Static function prototype
*/
static BOOL _isalpha(char c);
static BOOL _isalnum(char c);
static BOOL _dpsParseJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus);
static BOOL _verifyValue(const DWORD val, const DWORD validValues[], int count);
static BOOL _getParamValuePos(char *xmlBuff, char *paramName, char **start, char **end);
static BOOL _getParamShortValue(char *xmlBuff, char *paramName, DWORD * num);
static BOOL _IsNumValidInList(char *start, char *end, DWORD num);
static BOOL _getParamNumericValue(char *xmlBuff, char *paramName, DWORD * num);
static BOOL _parseTag(char *buff, const char *param, ParamPosition * pos);
static BOOL _getResult(char *xmlBuff, Result * result);
static SDWORD _getNumericList(char *start, char *end, DWORD * valList, int max);

// TODO: move these to dps_main 
//ConfPrintServiceHost gPrinterConf;
//DeviceStatus gDevStatus;

/*
// Definition of internal functions
*/
#ifdef SGBOARD
void dpsReqConfPrintService(char *sbuf)
{
	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);
	strcat(sbuf, xmlOpenConfServ);
	strcat(sbuf, xmlOpenDpsVer);
	strcat(sbuf, "1.0");		// dps version
	strcat(sbuf, xmlCloseDpsVer);
	strcat(sbuf, xmlOpenvendName);
	strcat(sbuf, "Samsung Electronics Co., Ltd.");	// vendor name 
	strcat(sbuf, xmlCloseVendName);
	strcat(sbuf, xmlOpenVendVer);
	strcat(sbuf, "01.00");		// optional
	strcat(sbuf, xmlCloseVendVer);
	strcat(sbuf, xmlOpenProdName);
	strcat(sbuf, "LE40M71BX");	// product name 
	strcat(sbuf, xmlCloseProdName);
	strcat(sbuf, xmlOpenSerialNum);
	strcat(sbuf, "1234567890");	// serial number
	strcat(sbuf, xmlCloseSerialNum);
	strcat(sbuf, xmlCloseConfServ);
	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);
}
#else
void dpsReqConfPrintService(char *sbuf)
{

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpenConfServ);

	strcat(sbuf, xmlOpenDpsVer);
	strcat(sbuf, "1.0");		// dps version
	strcat(sbuf, xmlCloseDpsVer);

	strcat(sbuf, xmlOpenvendName);
	strcat(sbuf, "MagicPixel");	// vendor name 
	strcat(sbuf, xmlCloseVendName);

	strcat(sbuf, xmlOpenVendVer);
	strcat(sbuf, "1.0");		// optional
	strcat(sbuf, xmlCloseVendVer);

	strcat(sbuf, xmlOpenProdName);
	strcat(sbuf, "iPlay");		// product name 
	strcat(sbuf, xmlCloseProdName);

	strcat(sbuf, xmlOpenSerialNum);
	strcat(sbuf, "1234567890");	// serial number
	strcat(sbuf, xmlCloseSerialNum);

	strcat(sbuf, xmlCloseConfServ);


	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);

}
#endif

void dpsReqGetCapability(char *sbuf, PaperSize paperSize, DWORD capability)
{
	char str[30];

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpenGetCapability);
	strcat(sbuf, xmlOpenCapability);

	switch (capability)
	{
	case DPS_QUALITIES:
		strcat(sbuf, xmlOpClQualities);
		break;

	case DPS_PAPER_SIZES:
		strcat(sbuf, xmlOpClPaperSizes);
		break;

	case DPS_PAPER_TYPES:
		// may be qualified with paper size
		if (paperSize == noPaperSize)
		{
			strcat(sbuf, xmlOpClPaperTypes);
		}
		else
		{
			mp_sprintf(str, " paperSize=\"%08X\"/>", paperSize);

			strcat(sbuf, xmlOpenPaperTypes);	// <paperTypes>
			strcpy(&sbuf[strlen(sbuf) - 1], str);	// <paperTypes paperSize="%8x"/>
		}
		break;

	case DPS_FILE_TYPES:
		strcat(sbuf, xmlOpClFileTypes);
		break;

	case DPS_DATE_PRINTS:
		strcat(sbuf, xmlOpClDatePrints);
		break;

	case DPS_FILE_NAME_PRINTS:
		strcat(sbuf, xmlOpClFileNamePrints);
		break;

	case DPS_IMAGE_OPTIMIZES:
		strcat(sbuf, xmlOpClImageOptimizes);
		break;

	case DPS_LAYOUTS:
		// layouts must be qualified with paper size
		if (paperSize == noPaperSize)
		{
			paperSize = defPaperSize;
		}

		mp_sprintf(str, " paperSize=\"%08X\"/>", paperSize);

		strcat(sbuf, xmlOpenLayouts);	// <layouts>
		strcpy(&sbuf[strlen(sbuf) - 1], str);	// <paperTypes paperSize="%8x"/>

		break;

	case DPS_FIXED_SIZES:
		strcat(sbuf, xmlOpClFixedSizes);
		break;

	case DPS_CROPPINGS:
		strcat(sbuf, xmlOpClCroppings);
		break;

	default:
		sbuf[0] = '\0';
		return;
	}


	strcat(sbuf, xmlCloseCapability);
	strcat(sbuf, xmlCloseGetCapability);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);
}

void dpsReqGetJobStatus(char *sbuf)
{
	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpClGetJobStatus);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);
}

void dpsReqGetDeviceStatus(char *sbuf)
{
	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpClGetDevStatus);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);

}

void dpsReqStartJob(char *sbuf, DpsJobConfig * jobConf, DpsPrintJobInfo * printInfo,
					DWORD prtInfoCount)
{
	char str[30];
	DWORD dwIndex;
	ST_SEARCH_INFO *pST_SEARCH_INFO;
	BYTE  bName[9];

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpenStartJob);

	if (jobConf != NULL)
	{
		strcat(sbuf, xmlOpenJobConfig);

		// quality
		if (jobConf->quality != noQuality)
		{
			strcat(sbuf, xmlOpenQuality);
			mp_sprintf(str, "%08X", jobConf->quality);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseQuality);
		}

		// paperType
		if (jobConf->paperType != noPaperType)
		{
			strcat(sbuf, xmlOpenPaperType);
			mp_sprintf(str, "%08X", jobConf->paperType);
			strcat(sbuf, str);
			strcat(sbuf, xmlClosePaperType);
		}

		// paperSize
		if (jobConf->paperSize != noPaperSize)
		{
			strcat(sbuf, xmlOpenPaperSize);
			mp_sprintf(str, "%08X", jobConf->paperSize);
			strcat(sbuf, str);
			strcat(sbuf, xmlClosePaperSize);
		}

		//fileType
		if (jobConf->fileType != noFileType)
		{
			strcat(sbuf, xmlOpenFileType);
			mp_sprintf(str, "%08X", jobConf->fileType);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseFileType);
		}

		//datePrint
		if (jobConf->datePrint != noDatePrint)
		{
			strcat(sbuf, xmlOpenDatePrint);
			mp_sprintf(str, "%08X", jobConf->datePrint);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseDatePrint);
		}

		//fileNamePrint
		if (jobConf->fileNamePrt != noNamePrint)
		{
			strcat(sbuf, xmlOpenFileNamePrint);
			mp_sprintf(str, "%08X", jobConf->fileNamePrt);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseFileNamePrint);
		}

		//imageOptimize
		if (jobConf->imageOpt != noImageOpt)
		{
			strcat(sbuf, xmlOpenImageOptimize);
			mp_sprintf(str, "%08X", jobConf->imageOpt);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseImageOptimize);
		}

		//layout
		if (jobConf->layout != noLayout)
		{
			strcat(sbuf, xmlOpenLayout);
			mp_sprintf(str, "%08X", jobConf->layout);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseLayout);

		}
		//fixedSize
		else if (jobConf->fixedSize != noFixedSize)	//fixedSize is execlusive with layout
		{
			strcat(sbuf, xmlOpenFixedSize);
			mp_sprintf(str, "%08X", jobConf->fixedSize);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseFixedSize);
		}

		//cropping
		if (jobConf->cropping != noCropping)
		{
			strcat(sbuf, xmlOpenCropping);
			mp_sprintf(str, "%08X", jobConf->cropping);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseCropping);
		}

		strcat(sbuf, xmlCloseJobConfig);
	}

	for (dwIndex = 0; dwIndex < prtInfoCount; dwIndex++)
	{
		strcat(sbuf, xmlOpenPrintInfo);

		if (jobConf->cropping != noCropping)
		{
			strcat(sbuf, xmlOpenCroppingArea);
			mp_sprintf(str, "%04X %04X %04X %04X", printInfo[dwIndex].CropX,  printInfo[dwIndex].CropY,  printInfo[dwIndex].CropW,  printInfo[dwIndex].CropH);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseCroppingArea);
		}

		strcat(sbuf, xmlOpenFileID);
		mp_sprintf(str, "%08X", printInfo[dwIndex].fileID);
		strcat(sbuf, str);
		strcat(sbuf, xmlCloseFileID);

		//fileNamePrint
		if (jobConf->fileNamePrt != noNamePrint)
		{
        		if (printInfo[dwIndex].bFileNamePrint == TRUE)
        		{
                        // Get 8.3 File Name
                        pST_SEARCH_INFO = (ST_SEARCH_INFO *)FileGetSearchInfo(dwIndex);
                        memset(bName, 0x0, 9);
                        strncpy(bName, pST_SEARCH_INFO->bName, 8);

                         if(pST_SEARCH_INFO->dwLongFdbCount != 0) // Long Name Format
        		        MP_ALERT("--E-- Only support FileName 8.3 Format");
                        
                        strcat(sbuf, xmlOpenFileName);
                        mp_sprintf(str, "%s.%s", bName, pST_SEARCH_INFO->bExt);
                        strcat(sbuf, str);
                        strcat(sbuf, xmlCloseFileName);
        		}
		}                

		if (printInfo[dwIndex].copies != 0)
		{		
			strcat(sbuf, xmlOpenCopies);
			mp_sprintf(str, "%03d", printInfo[dwIndex].copies);
			strcat(sbuf, str);
			strcat(sbuf, xmlCloseCopies);
		}

		if (jobConf->datePrint != noDatePrint)
		{
                if(printInfo[dwIndex].bFileDatePrint == TRUE)
                {
                    //Get Date
                    pST_SEARCH_INFO = (ST_SEARCH_INFO *)FileGetSearchInfo(dwIndex);

                    strcat(sbuf, xmlOpenDate);
                    mp_sprintf(str, "%.4d/%.2d/%.2d", pST_SEARCH_INFO->DateTime.year, pST_SEARCH_INFO->DateTime.month, pST_SEARCH_INFO->DateTime.day);
                    strcat(sbuf, str);
                    strcat(sbuf, xmlCloseDate);
                }
		}

		strcat(sbuf, xmlClosePrintInfo);
	}

	strcat(sbuf, xmlCloseStartJob);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);

}

//void dpsReqAbortJob (char* sbuf, AbortStyle style)
void dpsReqAbortJob(char *sbuf)
{
	char number[10] = "90000000";

//    mp_sprintf(number, "%08X", style);

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpenAbortJob);

	strcat(sbuf, xmlOpenAbortStyle);
	strcat(sbuf, number);		// abort immidiately
	strcat(sbuf, xmlCloseAbortStyle);

	strcat(sbuf, xmlCloseAbortJob);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);

}

void dpsReqContinueJob(char *sbuf)
{

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenInput);

	strcat(sbuf, xmlOpClContJob);

	strcat(sbuf, xmlCloseInput);
	strcat(sbuf, xmlClose);

}

void dpsRespEventNotifyJobStatus(char *sbuf, Result result)
{
	char number[10];

	mp_sprintf(number, "%08X", result);

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenOutput);

	strcat(sbuf, xmlOpenResult);
	strcat(sbuf, number);
	strcat(sbuf, xmlCloseResult);

	strcat(sbuf, xmlOpClNotifyJobStatus);

	strcat(sbuf, xmlCloseOutput);
	strcat(sbuf, xmlClose);
}

void dpsRespEventNotifyDeviceStatus(char *sbuf, Result result)
{
	char number[10];

	mp_sprintf(number, "%08X", result);

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenOutput);

	strcat(sbuf, xmlOpenResult);
	strcat(sbuf, number);
	strcat(sbuf, xmlCloseResult);

	strcat(sbuf, xmlOpClNotifyDevStatus);

	strcat(sbuf, xmlCloseOutput);
	strcat(sbuf, xmlClose);
}

/*
void dpsRespReqGetFileID(char *sbuf, Result result, DWORD id)
{
	char number[10];

	mp_sprintf(number, "%08X", result);

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenOutput);

	strcat(sbuf, xmlOpenResult);
	strcat(sbuf, number);
	strcat(sbuf, xmlCloseResult);

	mp_sprintf(number, "%08X", id);

	strcat(sbuf, xmlOpenGetFileID);
	strcat(sbuf, xmlOpenFileID);
	strcat(sbuf, number);
	strcat(sbuf, xmlCloseFileID);
	strcat(sbuf, xmlCloseGetFileID);

	strcat(sbuf, xmlCloseOutput);
	strcat(sbuf, xmlClose);

}
*/

// in accordance to page 11 for responding to unrecognized operation
void dpsRespNotRecognizedOperation(char *sbuf)
{
	char number[10];

	mp_sprintf(number, "%08X", 0x10030000);

	strcpy(sbuf, xmlHeader);
	strcat(sbuf, xmlOpenOutput);

	strcat(sbuf, xmlOpenResult);
	strcat(sbuf, number);
	strcat(sbuf, xmlCloseResult);

	strcat(sbuf, xmlCloseOutput);
	strcat(sbuf, xmlClose);
}

BOOL dpsParseRespGetJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus,
							  Result * retResult)
{
	BOOL ret;
	Result result;

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	if (ret == FAIL)
	{
		MP_DEBUG1("result failed 1st, result = 0x%x", result);
		return FAIL;
	}

	*retResult = result;

	return _dpsParseJobStatus(xmlBuff, type, jobStatus);
}

BOOL dpsParseEventNotifyJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus)
{
	return _dpsParseJobStatus(xmlBuff, type, jobStatus);
}

BOOL dpsParseRespGetDeviceStatus(char *xmlBuff, DeviceStatus * devStatus, Result * retResult)
{
	Result result;
	BOOL ret;

	ret = _getResult(xmlBuff, &result);
	if (ret == FAIL)
	{
		MP_DEBUG("result failed");
		return FAIL;
	}

	*retResult = result;


	return dpsParseEventNotifyDeviceStatus(xmlBuff, devStatus);
}

BOOL dpsParseEventNotifyDeviceStatus(char *xmlBuff, DeviceStatus * devStatus)
{
	BOOL ret;
	DWORD val;
	static const DWORD servicesValues[3] = { 0x70000000, 0x70010000, 0x70020000 };
	static const DWORD endReasonValues[5] =
		{ 0x71000000, 0x71010000, 0x71020000, 0x71030000, 0x71040000 };
	static const DWORD errStatusValues[3] = { 0x72000000, 0x72010000, 0x72020000 };
	static const DWORD errReasonValues[5] =
		{ 0x73000000, 0x73010000, 0x73020000, 0x73030000, 0x73040000 };
	static const DWORD disconnectValues[2] = { 0x74000000, 0x74010000 };
//  static const DWORD CapabValues[2]      = {0x75000000,0x75010000};
	static const DWORD newJobOKValues[2] = { 0x76000000, 0x76010000 };


	// dpsPrintServiceStatus
	ret = _getParamNumericValue(xmlBuff, "dpsPrintServiceStatus", &(devStatus->printServiceStatus));
	if (ret == FAIL || !_verifyValue(devStatus->printServiceStatus, servicesValues, 3))
	{
		return FAIL;
	}

	// jobEndReason (not mandatory)
	ret = _getParamNumericValue(xmlBuff, "jobEndReason", &(devStatus->jobEndReason));
	if (ret == FAIL || !_verifyValue(devStatus->jobEndReason, endReasonValues, 5))
	{
//      return FAIL;
		devStatus->jobEndReason =
			(devStatus->printServiceStatus == 0x70010000) ? 0x71010000 : 0x71000000;
	}

	// errorStatus
	ret = _getParamNumericValue(xmlBuff, "errorStatus", &(devStatus->errorStatus));
	if (ret == FAIL || !_verifyValue(devStatus->errorStatus, errStatusValues, 3))
	{
		return FAIL;
	}

	// errorReason
	ret = _getParamNumericValue(xmlBuff, "errorReason", &(devStatus->errorReason));
	if (ret == FAIL || !_verifyValue(devStatus->errorReason & 0xFFFF0000U, errReasonValues, 5))	// avoid the minor number
	{
		return FAIL;
	}

	// disconnectEnable   (not mandatory)
	ret = _getParamNumericValue(xmlBuff, "disconnectEnable", &val);
	if (ret == FAIL)
	{
//      return FAIL;
		// if idle, disconnet enable!
		devStatus->disconnectEnable =
			(devStatus->printServiceStatus == 0x70010000) ? 0x74010000 : 0x74000000;
	}
	else if (!_verifyValue(val, disconnectValues, 2))
	{
		return FAIL;
	}
	devStatus->disconnectEnable = (val == 0x74000000) ? FALSE : TRUE;

	// capabilityChanged   (NOT SUPPORTED)

	// newJobOK
	ret = _getParamNumericValue(xmlBuff, "newJobOK", &val);
	if (ret == FAIL || !_verifyValue(val, newJobOKValues, 2))
	{
		return FAIL;
	}
	devStatus->newJobOK = (val == 0x76000000) ? FALSE : TRUE;

	return PASS;

}

BOOL dpsParseRespConfPrintService(char *xmlBuff, ConfPrintServiceHost * conf, Result * retResult)
{

	char *posStart, *posEnd;
	BOOL ret;
	Result result;
	DWORD val, len;

//  float   ver;
	BOOL ver;

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	if (ret == FAIL)
	{
		MP_DEBUG1("result failed 1st, result = 0x%x", result);
		return FAIL;
	}

	*retResult = result;

	// printServiceAvailable
	ret = _getParamValuePos(xmlBuff, "printServiceAvailable", &posStart, &posEnd);
	if (ret != FAIL)
	{
		sscanf(posStart, "%x", (unsigned int *) &val);
		conf->printServiceAvailable = (val == 0x30000000) ? FALSE : TRUE;
	}
	else
	{
		MP_DEBUG("result failed 2nd");
		return FAIL;
	}

	ret = _getParamValuePos(xmlBuff, "dpsVersions", &posStart, &posEnd);
	if (ret != FAIL)
	{
		//  sscanf(posStart, "%f", &ver);
		//  conf->dpsVersion = ( ver == 1.0 ) ? TRUE : FALSE;
		//  sscanf(posStart, "%f", &ver);
		if (strcmp(posStart, "1.0"))
			ver = TRUE;
		else
			ver = FALSE;
		conf->dpsVersion = ver;	//( strcmp(posStart, "1.0") ) ? TRUE : FALSE;
	}

	ret = _getParamValuePos(xmlBuff, "vendorName", &posStart, &posEnd);
	if (ret != FAIL)
	{
		len = posEnd - posStart + 1;
		strncpy(conf->vendName, posStart, len);	// it copies from 0 to len-1  (total len characters)
		conf->vendName[len] = '\0';	// terminate copied path
	}

	ret = _getParamValuePos(xmlBuff, "productName", &posStart, &posEnd);
	if (ret != FAIL)
	{
		len = posEnd - posStart + 1;
		strncpy(conf->productName, posStart, len);	// it copies from 0 to len-1  (total len characters)
		conf->productName[len] = '\0';	// terminate copied path
	}

	ret = _getParamValuePos(xmlBuff, "serialNo", &posStart, &posEnd);
	if (ret != FAIL)
	{
		len = posEnd - posStart + 1;
		strncpy(conf->productName, posStart, len);	// it copies from 0 to len-1  (total len characters)
		conf->productName[len] = '\0';	// terminate copied path
	}

	return DPS_SUCCESS;
}

BOOL dpsParseRespStartJob(char *xmlBuff, Result * retResult)
{
	Result result;
	BOOL ret;
	ParamPosition pos;

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	*retResult = result;

	if (ret != FAIL)
	{
		ret = _parseTag(xmlBuff, "startJob", &pos);
	}

	return ret;
}

BOOL dpsParseRespAbortJob(char *xmlBuff, Result * retResult)
{
	Result result;
	BOOL ret;
	ParamPosition pos;

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	*retResult = result;

	if (ret != FAIL)
	{
		ret = _parseTag(xmlBuff, "abortJob", &pos);
	}

	return ret;
}

BOOL dpsParseRespContinueJob(char *xmlBuff, Result * retResult)
{
	Result result;
	BOOL ret;
	ParamPosition pos;

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	*retResult = result;

	if (ret != FAIL)
	{
		ret = _parseTag(xmlBuff, "continueJob", &pos);
	}

	return ret;
}

BOOL dpsParseRespGetCapability(char *xmlBuff, PaperSize paperSize, DWORD capability,
							   DpsPrinterCapability * printerCap, Result * retResult)
{

	char *posStart, *posEnd;
	Result result;
	BOOL ret;

	//DWORD res;
	SDWORD i, j, count;

    #if 0
	static PaperSize paperSizeOfInterest[MAX_PAPERSIZES] =  { 	
        defPaperSize, paperSizeL, paperSizeL2, paperSizePostcard, paperSizeCard, paperSize100x150,
	    paperSize4x6, paperSize8x10, paperSizeLetter, paperSize11x17, paperSizeA3, paperSizeA4, paperSizeA6,
	    paperSizeB4, paperSizeB5, paperSizeRoll_L, paperSizeRoll_2L };
    #endif        

	// verify result was OK
	ret = _getResult(xmlBuff, &result);
	if (ret == FAIL)
	{
		MP_DEBUG1("result failed 1st, result = 0x%x", result);
		return FAIL;
	}

	*retResult = result;

	// verify it's a result
	switch (capability)
	{
	case DPS_QUALITIES:

		ret = _getParamValuePos(xmlBuff, "qualities", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->qualities, MAX_QUALITIES);

		if (count > 0)
		{
			printerCap->qualitiesCount = count;
		}
		else
		{
			return FAIL;
		}
		break;

	case DPS_PAPER_SIZES:
		//printerCap->paperSizesCount = 0;
		ret = _getParamValuePos(xmlBuff, "paperSizes", &posStart, &posEnd);

        #if 1
		count = _getNumericList(posStart, posEnd, printerCap->paperSizes, MAX_PAPERSIZES);

		if (count > 0)
		{
			printerCap->paperSizesCount = count;

		}
		else
		{
			return FAIL;
		}

        #else
		for (i = 0; i < MAX_PAPERSIZES; i++)
		{
			if (_IsNumValidInList(posStart, posEnd, paperSizeOfInterest[i]) != FALSE)
			{
				printerCap->paperSizes[printerCap->paperSizesCount] = paperSizeOfInterest[i];
				printerCap->paperSizesCount++;
			}
		}
		if (printerCap->paperSizesCount == 0)
		{
			return FAIL;
		}
        #endif
		break;

	case DPS_PAPER_TYPES:
		//This gets the different layouts of the paper size of interest

		// Get the  index
		for (i = 0; i < printerCap->paperSizesCount; i++)
		{
			if (printerCap->paperSizes[i] == paperSize)
			{
				break;
			}
		}
		if (i == printerCap->paperSizesCount)
		{
			return FAIL;
		}
        
		ret = _getParamValuePos(xmlBuff, "paperTypes", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->paperTypes[i], MAX_PAPERTYPES);

		if (count > 0)
		{
			printerCap->paperTypesCount[i] = count;

            #if 0  // ui_GetPrinterCapability do this function 
			// clear the minor code
			for (j = 0; j < count; j++)
			{
				printerCap->paperTypes[i][j] = printerCap->paperTypes[i][j] & 0xFFFF0000U;
			}
            #endif
		}
		else
		{
			return FAIL;
		}
		break;

	case DPS_FILE_TYPES:
		ret = _getParamValuePos(xmlBuff, "fileTypes", &posStart, &posEnd);

        #if 1
		count = _getNumericList(posStart, posEnd, printerCap->fileTypes, MAX_FILETYPES);

		if (count > 0)
		{
			printerCap->fileTypesCount = count;
		}
		else
		{
			return FAIL;
		}
		break;

        #else
		printerCap->fileTypesCount = 0;

		// types of interest are : defFileType, fileTypeJpeg, fileTypeBmp
		if (_IsNumValidInList(posStart, posEnd, defFileType) != FALSE)
		{
			printerCap->fileTypes[printerCap->fileTypesCount] = defFileType;
			printerCap->fileTypesCount++;
		}

		if (_IsNumValidInList(posStart, posEnd, fileTypeJpeg) != FALSE)
		{
			printerCap->fileTypes[printerCap->fileTypesCount] = fileTypeJpeg;
			printerCap->fileTypesCount++;
		}

		if (_IsNumValidInList(posStart, posEnd, fileTypeBmp) != FALSE)
		{
			printerCap->fileTypes[printerCap->fileTypesCount] = fileTypeBmp;
			printerCap->fileTypesCount++;
		}

		if (printerCap->fileTypesCount == 0)
		{
			return FAIL;
		}
		break;
        #endif

	case DPS_DATE_PRINTS:
		ret = _getParamValuePos(xmlBuff, "datePrints", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->datePrints, MAX_DATEPRINTS);

		if (count > 0)
		{
			printerCap->datePrintsCount = count;
		}
		else
		{
			return FAIL;
		}
		break;


	case DPS_FILE_NAME_PRINTS:
		ret = _getParamValuePos(xmlBuff, "fileNamePrints", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->fileNamePrints, MAX_FILENAMEPRINTS);

		if (count > 0)
		{
			printerCap->fileNamePrintsCount = count;
		}
		else
		{
			return FAIL;
		}
		break;


	case DPS_IMAGE_OPTIMIZES:
		ret = _getParamValuePos(xmlBuff, "imageOptimizes", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->imageOptimizes, MAX_OPTIMIZES);

		if (count > 0)
		{
			printerCap->imageOptimizesCount = count;
		}
		else
		{
			return FAIL;
		}
		break;

	case DPS_LAYOUTS:
		//This gets the different layouts of the paper size of interest

		// Get the  index
		for (i = 0; i < printerCap->paperSizesCount; i++)
		{
			if (printerCap->paperSizes[i] == paperSize)
			{
				break;
			}
		}
		if (i == printerCap->paperSizesCount)
		{
			return FAIL;
		}

		ret = _getParamValuePos(xmlBuff, "layouts", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->layouts[i], MAX_LAYOUTSPERSIZE);

		if (count > 0)
		{
			printerCap->layoutsCount[i] = count;
		}
		else
		{
			return FAIL;
		}
		break;

	case DPS_FIXED_SIZES:
		ret = _getParamValuePos(xmlBuff, "fixedSizes", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->fixedSizes, MAX_FIXEDSIZES);

		if (count > 0)
		{
			printerCap->fixedSizesCount = count;
		}
		else
		{
			return FAIL;
		}
		break;

	case DPS_CROPPINGS:
		ret = _getParamValuePos(xmlBuff, "croppings", &posStart, &posEnd);
		count = _getNumericList(posStart, posEnd, printerCap->croppings, MAX_CROPPINGS);

		if (count > 0)
		{
			printerCap->croppingsCount = count;
		}
		else
		{
			return FAIL;
		}
		break;


	}

	return ret;
}

/*
// dpsParseReqGetFileID (DWORD *base, char *filePath)
// base: a number that is recovered from the xml string amd returned to the calling function
// filePath: a string, the calling function has to allocate enough space for it
// returns PASS/FAIL depending if the 2 parameters in the xml string
//         were recovered.
BOOL dpsParseReqGetFileID(char *xmlBuff, DWORD * base, char *filePath)
{
	char *posStart, *posEnd;
	BOOL ret;
	DWORD len;

	// verify it's a getFileID request
	ret = _getParamValuePos(xmlBuff, "getFileID", &posStart, &posEnd);

	// get location of "basePathID"
	if (ret != FAIL)
	{
		ret = _getParamValuePos(xmlBuff, "basePathID", &posStart, &posEnd);
	}
	if (ret != FAIL)
	{
		ret = sscanf(posStart, "%x", (unsigned int *) base);
		if (posEnd - posStart != 7 || ret == 0)
		{
			ret = FAIL;			// no valid ID
		}
	}

	// get location of "filePath"
	if (ret != FAIL)
	{
		ret = _getParamValuePos(xmlBuff, "filePath", &posStart, &posEnd);
	}

	if (ret != FAIL)
	{
		// recover the path string and copy it to base 
		len = posEnd - posStart + 1;
		strncpy(filePath, posStart, len);	// it copies from 0 to len-1  (total len characters)
		filePath[len] = '\0';	// terminate copied path
	}

	return ret;
}
*/
#if 0
BOOL dpsParseReqGetFileID(char *xmlBuff, DWORD * base, char *filePath)
{
	ParamPosition paramPos;
	char *posStart, *posEnd;
	BOOL ret;
	DWORD len;

	// verify it's a getFileID request
	ret = _parseTag(xmlBuff, "getFileID", &paramPos);	// open
	if (ret)
	{
		ret = _parseTag(paramPos.tagClose, "getFileID", &paramPos);	// close
	}

	if (ret == FAIL)
	{
		return FAIL;
	}

	// get location of "basePathID"
	ret = _parseTag(xmlBuff, "basePathID", &paramPos);
	posStart = paramPos.tagClose + 1;
	if (ret)
	{
		ret = _parseTag(paramPos.tagClose, "basePathID", &paramPos);
		posEnd = paramPos.tagOpen - 1;
	}

	if (ret == FAIL)
	{
		return FAIL;
	}

	ret = sscanf(posStart, "%x", base);

	if (posEnd - posStart != 7 || ret == 0)
	{
		return FAIL;			// no valid ID
	}

	// get location of "filePath"
	ret = _parseTag(xmlBuff, "filePath", &paramPos);
	posStart = paramPos.tagClose + 1;
	if (ret)
	{
		ret = _parseTag(paramPos.tagClose, "filePath", &paramPos);
		posEnd = paramPos.tagOpen - 1;
	}

	if (ret == FAIL)
	{
		return FAIL;
	}


	// recover the path string and copy it to base 
	len = posEnd - posStart + 1;
	strncpy(filePath, posStart, len);	// it copies from 0 to len-1  (total len characters)
	filePath[len] = '\0';		// terminate copied path

	return DPS_SUCCESS;
}
#endif //0

/*
// Definition of local functions 
*/
static BOOL _isalpha(char c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return TRUE;

	return FALSE;

}

static BOOL _isalnum(char c)
{

	if (_isalpha(c) || (c >= '0' && c <= '9'))
		return TRUE;

	return FALSE;
}

// this function is used for DPS_GetJobStatus (response) and DPS_NotifyJobStatus (event)
static BOOL _dpsParseJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus)
{
	char *posStart, *posEnd;
	ParamPosition paramPos;
	BOOL ret;
	int num1, num2;

	//DWORD val;

	// Verify: notifyJobStatus (event) OR getJobStatus (response)
	ret = _parseTag(xmlBuff, "notifyJobStatus", &paramPos);
	if (ret == FAIL)
	{
		ret = _parseTag(xmlBuff, "getJobStatus", &paramPos);
	}
	if (ret == FAIL)
	{
		return FAIL;
	}


	ret = _getParamValuePos(xmlBuff, "progress", &posStart, &posEnd);
	if (ret == FAIL)
	{
		return FAIL;
	}
	sscanf(posStart, "%x/%x", &(num1), &(num2));
	jobStatus->progress[0] = num1;
	jobStatus->progress[1] = num2;
	MP_DEBUG2("Print Progress %x/%x", jobStatus->progress[0], jobStatus->progress[1]);

	// --------------------------------------------------------------------
	// no need to return error if any of the following params are missing
	// --------------------------------------------------------------------
	// imagesPrinted
	ret = _getParamShortValue(xmlBuff, "imagesPrinted", &(jobStatus->imagesPrinted));

	if (ret == FAIL)
	{
		// copyID
		ret = _getParamShortValue(xmlBuff, "copyID", &(jobStatus->dpofParams.copyID));
		// prtPID
		ret = _getParamShortValue(xmlBuff, "prtPID", &(jobStatus->dpofParams.prtPID));
		//filePath
		ret = _getParamValuePos(xmlBuff, "filePath", &posStart, &posEnd);
		strncpy(jobStatus->dpofParams.fileName, posStart, posEnd - posStart + 1);	// it copies from 0 to len-1  (total len characters)
		jobStatus->dpofParams.fileName[posEnd - posStart + 1] = '\0';	// terminate copied path

		if (ret == FAIL)
		{
			jobStatus->dpofParamsExist = FALSE;
		}
		else
		{
			jobStatus->dpofParamsExist = TRUE;
		}

	}
	else
	{

		jobStatus->dpofParamsExist = FALSE;
		MP_DEBUG1("imagesPrinted %x", jobStatus->imagesPrinted);

	}

	return DPS_SUCCESS;
}

static BOOL _verifyValue(const DWORD val, const DWORD validValues[], int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (val == validValues[i])
		{
			return TRUE;
		}
	}

	return FALSE;
}

// This function returns pointers (start & end) between which 
// the paramter of the TAG (paramName) does exist.
// returns: PASS/FAIL depending if it finds valid parameter value
static BOOL _getParamValuePos(char *xmlBuff, char *paramName, char **start, char **end)
{
	ParamPosition paramPos;
	BOOL ret = PASS;

	// get location of paramName
	ret = _parseTag(xmlBuff, paramName, &paramPos);
	*start = paramPos.tagClose + 1;
	if (ret != FAIL)
	{
		ret = _parseTag(paramPos.tagClose, paramName, &paramPos);
		*end = paramPos.tagOpen - 1;
	}
	else
	{
		MP_DEBUG("ret parsetag1 = 0\n");
	}

//  MP_DEBUG1("ret parsetag2 = %d\n", ret);

	return ret;
}

static BOOL _getParamShortValue(char *xmlBuff, char *paramName, DWORD * num)
{
	char *posStart, *posEnd, buf[5];
	int i;
	BOOL ret;
	char *ptr;

	ret = _getParamValuePos(xmlBuff, paramName, &posStart, &posEnd);
	if (ret != FAIL)
	{
		// THIS PART IS A BUG WITH sscanf <--- it returns ZERO when string is "000</imagesPrinted>
//      i = sscanf(posStart, "%d", num);
//      if ( posEnd-posStart != 2 || i == 0)       // 8 character Hex number
//      {   
//          MP_DEBUG1("postStart = %s ", posStart);
//          MP_DEBUG1("Failed: short num = %x", num);
//          ret = FAIL;     // no valid result
//      }
		ptr = posStart;
		i = 0;
		while (*ptr >= '0' && *ptr <= '9' && i < 4)
		{
			buf[i] = *ptr;
			ptr++;
			i++;
		}
		buf[i + 1] = '\0';
		if (posEnd - posStart != 2 || i != 3)
		{
			ret = FAIL;
		}
		else
		{
			*num = atoi(buf);
		}
	}

//  MP_DEBUG1("SUCCESS: short num = %x", *num);
	return ret;

}

// returns true : if the param value list contains num
//         false: if the param value list doesn't contain num
static BOOL _IsNumValidInList(char *start, char *end, DWORD num)
{
	char *ptr = start;
	int ret;
	DWORD val;
	char numAscii[9];

	while ((ptr + 7) <= end)
	{
		ret = sscanf(ptr, "%X", (unsigned int *) &val);

		if (val == num)			// is found
		{
			return TRUE;
		}
		mp_sprintf(numAscii, "%X", val);
		ptr = strstr(start, numAscii);
		ptr += 8;
	}
	return FALSE;
}

static BOOL _getParamNumericValue(char *xmlBuff, char *paramName, DWORD * num)
{
	char *posStart, *posEnd;
	int i;
	BOOL ret;

	ret = _getParamValuePos(xmlBuff, paramName, &posStart, &posEnd);
	if (ret != FAIL)
	{
		i = sscanf(posStart, "%X", (unsigned int *) num);
		if (posEnd - posStart != 7 || i == 0)	// 8 character Hex number
		{
			ret = FAIL;			// no valid result
		}
	}
	return ret;

}

// _parseTag : will search for a valid 'param' between < > in the xml string.
// Limitation: if the 1st instance of the param is found to be not between < >, it'll 
//              return with failure and won't try looking for anothe instance.
static BOOL _parseTag(char *buff, const char *param, ParamPosition * pos)
{
	char *ptr1, *ptr2, *buffPtr;

//    char mybuf[50];
	int open_with_slash = -1;
	int close_with_slash = -1;

	// --------- 1st parameter -------------
	buffPtr = buff;

	while (1)					// TODO: set proper termination condition
	{
		ptr1 = strstr(buffPtr, param);

		if (ptr1 == NULL)
		{
			MP_DEBUG("fail 1");
			return FAIL;
		}

		// ARM compiler problem
		// isalpha ('<') returns 0 BUT isalnum('>') retrns 8 !!!!!!!!!!!!
		if (!_isalpha(*(ptr1 - 1)) && !_isalnum(*(ptr1 + strlen(param))))
//      if ( !isalpha( *(ptr1-1)) && !isalpha( *(ptr1+strlen(param)))  && !_isdigit(*(ptr1+strlen(param))) )
//      if ( !_isalpha( *(ptr1-1)) && !_isalnum( *(ptr1+strlen(param))) )
		{
			break;				// it's a param not part of word 
		}
		else
		{
			buffPtr = ptr1 + 1;	// continue searching
		}
	}

	pos->param1 = ptr1;

	// open '<' for 1st parameter
	if (*(ptr1 - 1) == '<')
	{
		open_with_slash = 0;
		pos->tagOpen = ptr1 - 1;
	}
	else if (*(ptr1 - 1) == '/' && *(ptr1 - 2) == '<')
	{
		open_with_slash = 1;
		pos->tagOpen = ptr1 - 2;
	}
	else
	{
		ptr2 = ptr1;
		while (*(--ptr2) != '>')
		{
			if (*ptr2 == '<')
			{
				pos->tagOpen = ptr2;
				break;
			}
		}
		if (*(ptr2) == '>')
		{
			MP_DEBUG("fail 2");
			return FAIL;		// no opening <
		}

	}

	// close '>' for 1st parameter
	ptr1 = ptr1 + strlen(param);

	if (*ptr1 == '>')
	{
		close_with_slash = 0;
		pos->tagClose = ptr1;
	}
	else if (*ptr1 == '/' && *(ptr1 + 1) == '>')
	{
		close_with_slash = 1;
		pos->tagClose = ptr1 + 1;
	}
	else
	{
		ptr2 = ptr1;
		while (*(++ptr2) != '<')
		{
			if (*ptr2 == '>')
			{
				pos->tagClose = ptr2;
				break;
			}
		}
		if (*(ptr2) == '<')
		{
			MP_DEBUG("fail 3");
			return FAIL;		// no closing >
		}
	}

	if (open_with_slash == 1)
	{
		pos->tagType = tagClose;
	}
	else if (close_with_slash == 1)
	{
		pos->tagType = tagOpCl;
	}
	else if (open_with_slash == 0 && close_with_slash == 0)
	{
		pos->tagType = tagOpen;
	}
	else
	{
		pos->tagType = tagMid;
	}

	return DPS_SUCCESS;
}

// As result is inside all responses, this is a function that gets the result value
static BOOL _getResult(char *xmlBuff, Result * result)
{
	char *posStart, *posEnd;
	BOOL ret;
	DWORD res;

	// verify it's a result
	ret = _getParamValuePos(xmlBuff, "result", &posStart, &posEnd);
//  MP_DEBUG1("ret1 = %d", ret);

	if (ret != FAIL)
	{
		ret = sscanf(posStart, "%x", (unsigned int *) &res);
//      MP_DEBUG1("ret2 = %d", ret);
//      MP_DEBUG1("res = %x", res);
		*result = res;
		if (posEnd - posStart != 7 || ret == 0)
		{
			ret = FAIL;			// no valid result
		}
		else
		{
			ret = PASS;
		}
	}

	return ret;
}

// returns -ve: error
//   +ve: number of numeric values returned in valList[]
static SDWORD _getNumericList(char *start, char *end, DWORD * valList, int max)
{
	char *ptr = start;
	int i = 0;
	int ret;
	char numAscii[9];

	while ((ptr + 7) <= end && i < max)
	{
//     MP_DEBUG1("ptr = %s", ptr);
		ret = sscanf(ptr, "%X", (unsigned int *) &valList[i]);
        MP_DEBUG("%d %x", i, valList[i]);
		mp_sprintf(numAscii, "%X", valList[i]);
		ptr = strstr(ptr, numAscii);
		if (ptr == NULL)
		{
			mp_sprintf(numAscii, "%X", valList[i]);
			ptr = strstr(ptr, numAscii);
		}
		ptr += 8;
		i++;
	}
	return i;
}

#endif // SC_USBDEVICE
