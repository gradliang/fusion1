#ifndef  _DPS_COMMON__H_
#define  _DPS_COMMON__H_

#include "UtilTypeDef.h"
//#include "utilregfile.h" //temp_remove
#include "iplaysysconfig.h"
#include "flagdefine.h"

//#if (SC_USBDEVICE)
#define SEND_BUFF_SIZE		1100
#define DCF_FILE_SIZE		64		// TODO: Check
#define DPS_VERSION			1.0


#define DPS_FAILURE     FAIL
#define DPS_SUCCESS     TRUE


#define DPS_QUALITIES			1
#define DPS_PAPER_SIZES			2
#define DPS_PAPER_TYPES			3
#define DPS_FILE_TYPES			4
#define DPS_DATE_PRINTS			5
#define DPS_FILE_NAME_PRINTS	6
#define DPS_IMAGE_OPTIMIZES		7
#define DPS_LAYOUTS				8
#define DPS_FIXED_SIZES			9
#define DPS_CROPPINGS			10

//// constants used in XML
typedef enum { 
	noPaperSize		= 0x00000000,
	defPaperSize    = 0x51000000,
	paperSizeL      = 0x51010000,
	paperSizeL2     = 0x51020000,
	paperSizePostcard     = 0x51030000,	
	paperSizeCard   = 0x51040000,
	paperSize100x150  = 0x51050000, 
	paperSize4x6    = 0x51060000,
	paperSize8x10   = 0x51070000,
	paperSizeLetter = 0x51080000,
	paperSize11x17   = 0x510A0000,
	//511n0000 An (n=0~9) (Note2)
	paperSizeA0     = 0x51100000,	
	paperSizeA1     = 0x51110000,
	paperSizeA2     = 0x51120000, 
	paperSizeA3     = 0x51130000,	
	paperSizeA4     = 0x51140000,
	paperSizeA5     = 0x51150000, 
	paperSizeA6     = 0x51160000,	
	paperSizeA7     = 0x51170000,
	paperSizeA8     = 0x51180000, 	
	paperSizeA9     = 0x51190000, 	
	//512m0000 Bm (m=0~9) (Note3)	
	paperSizeB0     = 0x51200000,	
	paperSizeB1     = 0x51210000,
	paperSizeB2     = 0x51220000, 
	paperSizeB3     = 0x51230000,	
	paperSizeB4     = 0x51240000,
	paperSizeB5     = 0x51250000, 
	paperSizeB6     = 0x51260000,	
	paperSizeB7     = 0x51270000,
	paperSizeB8     = 0x51280000, 	
	paperSizeB9     = 0x51290000,	
	paperSizeRoll_L = 0x51810000,
	paperSizeRoll_2L= 0x51820000	
} PaperSize;

typedef enum { 
	noQuality		= 0x00000000,
	defQuality 	= 0x50000000,
	normalQuality	= 0x50010000, 
	draftQuality 	= 0x50020000, 
	fineQuality  	= 0x50030000 
} Quality;

typedef enum { 
	noPaperType	  = 0x00000000,
   defPaperType	  = 0x52000000,
   PlainPaperType = 0x52010000,
   PhotoPaperType = 0x52020000, 
   FastPaperType = 0x52030000
} PaperType;

typedef enum { 
	noFileType	        = 0x00000000,
	defFileType	        = 0x53000000,
	fileTypeExit_Jpeg 	= 0x53010000,
	fileTypeExit        = 0x53020000,	
	fileTypeJPEG     	= 0x53030000,
	fileTypeTIFF_EP    	= 0x53040000,
	fileTypeFlashPix 	= 0x53050000,	
	fileTypeBmp 	    = 0x53060000,
	fileTypeCIFF 	    = 0x53070000,
	fileTypeGIF 	    = 0x53080000,
	fileTypeJFIF 	    = 0x53090000,
	fileTypePCD 	    = 0x530A0000,
	fileTypePICT 	    = 0x530B0000,
	fileTypePNG 	    = 0x530C0000,
    fileTypeTIFF 	    = 0x530D0000,
    fileTypeTIFF_IT	    = 0x530E0000,
    fileTypeJP2 	    = 0x530F0000,
	fileTypeJPX         = 0x53100000,
	fileTypeUndefined 	= 0x53110000,
	fileTypeAssociation = 0x53120000,	
	fileTypeScript     	= 0x53130000,
	fileTypeExecutable  = 0x53140000,
	fileTypeText        = 0x53150000,	
	fileTypeHTML 	    = 0x53160000,
	fileTypeXHTML 	    = 0x53170000,
	fileTypeDPOF 	    = 0x53180000,
	fileTypeAIFF 	    = 0x53190000,
	fileTypeWAV 	    = 0x531A0000,
	fileTypeMP3 	    = 0x531B0000,
	fileTypeAVI 	    = 0x531C0000,
    fileTypeMPEG 	    = 0x531D0000,
    fileTypeASF	        = 0x531E0000
} FileType;

typedef enum { 
	noDatePrint		= 0x00000000,
	defDatePrint	= 0x54000000,
	dateOff			= 0x54010000,
	dateOn			= 0x54020000 
} DatePrint;

typedef enum { 
	noNamePrint	= 0x00000000,
	defFNamePrt	= 0x55000000,
	filePrtOff	= 0x55010000,
	filePrtON 	= 0x55020000 
} FileNamePrint;

typedef enum { 
	noImageOpt	= 0x00000000,
	defImgOpt	= 0x56000000,
	optOff		= 0x56010000,
	optOn 	 	= 0x56020000 
} ImageOptimize;

typedef enum { 
	noLayout		= 0x00000000,
	defLayout		= 0x57000000,
	//570n0000 n-Up (n=0~250) 
	oneUp			= 0x57010000,
	twoUp			= 0x57020000,
	threeUp			= 0x57030000,
	fourUp			= 0x57040000,
	fiveUp			= 0x57050000,
	sixUp			= 0x57060000,
	sevenUp			= 0x57070000,
	eightUp			= 0x57080000,
	indexLayout		= 0x57FE0000,  /* Index Print */
	oneFullBleed	= 0x57FF0000   /* Borderless */
} Layout;

typedef enum { 
	noFixedSize		= 0x00000000,
	defFixedSize	= 0x58000000,
	size2x3			= 0x58010000,
	size3x5			= 0x58020000,
	size4x6			= 0x58030000,
	size5x7			= 0x58040000,
	size8x10		= 0x58050000 
} FixedSize;

typedef enum { 
	noCropping	= 0x00000000,
	defCropping	= 0x59000000,
	cropOff		= 0x59010000,
	cropOn	 	= 0x59020000 
} Cropping;


typedef enum { 
	okResult		= 0x10000000,
	notExecuted		= 0x10010000,
	notSupported	= 0x10020000,
	notRecognized 	= 0x10030000 
} Result;
//dpsPrintServiceStatus
typedef enum { 
    PrintStatusPrinting = 0x70000000, 
    PrintStatusIdle        = 0x70010000,
    PrintStatusPaused = 0x70020000 
} enPrintStatus;

//jobEndReason
typedef enum { 
    JobEndNotEnded = 0x71000000,
    JobEndEnded =  0x71010000,
    JobEndAbortJobImmediate =  0x71020000,
    JobEndAbortJobNoImmediate =  0x71030000,
    JobEndOtherReason = 0x71040000   
} enJobEnd;

//errorStatus 
typedef enum { 
    ErrStatusNoError = 0x72000000,
    ErrStatusWarning =  0x72010000,    /* recoverable error */
    ErrStatusFatalError = 0x72020000  /* unrecoverable error */
} enErrStatus;

//errorReason
typedef enum { 
    ErrReasonNoReason = 0x73000000,
    ErrReasonPaperError =  0x73010000,
    ErrReasonInkError =  0x73020000,
    ErrReasonHWError =  0x73030000,
    ErrReasonFileError =  0x73040000, 
} enErrReason;

//disconnectEnable
typedef enum { 
    DisconEnFALSE = 0x74000000,
    DisconEnTRUE =  0x74010000 
} enDisconEn;

//capabilityChanged
typedef enum { 
    CapabilityChgFALSE = 0x75000000,
    CapabilityChgTRUE = 0x75010000
} enCapabilityChg;

//newJobOK
typedef enum { 
    newJobOKFALSE = 0x76000000,
    newJobOKTRUE = 0x76010000
} enNewJobOK;


#define MAX_QUALITIES       4
#define MAX_PAPERSIZES		34  // Max 34
#define MAX_PAPERTYPES		14  // Max: 4 x 33 (Minor code) = 132
#define MAX_FILETYPES		31  // Max 31
#define MAX_DATEPRINTS		3
#define MAX_FILENAMEPRINTS	3
#define MAX_OPTIMIZES		3
#define MAX_LAYOUTSPERSIZE	12  // Max 253
#define MAX_FIXEDSIZES		19 
#define MAX_CROPPINGS		3 

typedef struct{
	DWORD	qualities[MAX_QUALITIES];
	DWORD	qualitiesCount;
	DWORD  paperSizes[MAX_PAPERSIZES];
	DWORD	paperSizesCount;
    #if 1
	DWORD  paperTypes[MAX_PAPERSIZES][MAX_PAPERTYPES]; // first index by the paperSize
	DWORD	paperTypesCount[MAX_PAPERSIZES];    
    #else
	DWORD  paperTypes[MAX_PAPERTYPES];
	DWORD	paperTypesCount;
    #endif
	DWORD  fileTypes[MAX_FILETYPES];
	DWORD	fileTypesCount;
	DWORD  datePrints[MAX_DATEPRINTS];
	DWORD	datePrintsCount;
	DWORD  fileNamePrints[MAX_FILENAMEPRINTS];
	DWORD	fileNamePrintsCount;
	DWORD  imageOptimizes[MAX_OPTIMIZES];
	DWORD	imageOptimizesCount;
	DWORD  layouts[MAX_PAPERSIZES][MAX_LAYOUTSPERSIZE];			   // first index by the paperSize
	DWORD	layoutsCount[MAX_PAPERSIZES];
	DWORD  fixedSizes[MAX_FIXEDSIZES];
	DWORD	fixedSizesCount;
	DWORD  croppings[MAX_CROPPINGS];
	DWORD	croppingsCount;

	SDWORD	timeStamp;
} DpsPrinterCapability;


typedef struct{
	Quality			quality;
	PaperType		paperType;
	PaperSize		paperSize;
	FileType		fileType;
	DatePrint		datePrint;
	FileNamePrint	fileNamePrt;
	ImageOptimize	imageOpt;
	Layout			layout;
	FixedSize		fixedSize;
	Cropping		cropping;
} DpsJobConfig;

typedef struct{
    DWORD   fileID;
    BOOL  bFileNamePrint; // char    fileName[DCF_FILE_SIZE];
    DWORD   copies;
    BOOL  bFileDatePrint; // char    fileDate[12];
    BOOL    Crop;
    DWORD   CropX;
    DWORD   CropY;
    DWORD   CropW;
    DWORD   CropH;
} DpsPrintJobInfo;

// respose of the printer
typedef struct{
	BOOL	printServiceAvailable;
	BOOL	dpsVersion;			// true if 1.0 is supported
	char	vendName[64];
//		vendSpecVer;
	char	productName[64];
	char	serialNum[64];
} ConfPrintServiceHost;

// Events from printer
//DeviceStatus
typedef struct{
	SDWORD	timeStamp;
	DWORD	printServiceStatus;
	DWORD	jobEndReason;
	DWORD	errorStatus;
	DWORD	errorReason;
	BOOL	disconnectEnable;
    BOOL	capabilityChanged;
	BOOL	newJobOK;
} DeviceStatus;

typedef struct{
	SDWORD	timeStamp;
	BOOL	dpofParamsExist;
	WORD	progress[2];
	DWORD	imagesPrinted;		// execlusive with dpofParams
	struct dpofParams_{
		DWORD	prtPID;
		DWORD	copyID;
		char	fileName[DCF_FILE_SIZE];
	}dpofParams;
} JobStatus;

/*
// Function prototype 
*/
void dpsReqConfPrintService(char *sbuf);
void dpsReqGetCapability(char *sbuf, PaperSize paperSize, DWORD capability);
void dpsReqGetJobStatus(char *sbuf);
void dpsReqGetDeviceStatus(char *sbuf);
void dpsReqStartJob(char *sbuf, DpsJobConfig * jobConf, DpsPrintJobInfo * printInfo,
					DWORD prtInfoCount);
void dpsReqAbortJob(char *sbuf);
void dpsReqContinueJob(char *sbuf);
void dpsRespEventNotifyJobStatus(char *sbuf, Result result);
void dpsRespEventNotifyDeviceStatus(char *sbuf, Result result);
void dpsRespNotRecognizedOperation(char *sbuf);
BOOL dpsParseRespGetJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus,
							  Result * retResult);
BOOL dpsParseEventNotifyJobStatus(char *xmlBuff, FileType type, JobStatus * jobStatus);
BOOL dpsParseRespGetDeviceStatus(char *xmlBuff, DeviceStatus * devStatus, Result * retResult);
BOOL dpsParseEventNotifyDeviceStatus(char *xmlBuff, DeviceStatus * devStatus);
BOOL dpsParseRespConfPrintService(char *xmlBuff, ConfPrintServiceHost * conf, Result * retResult);
BOOL dpsParseRespStartJob(char *xmlBuff, Result * retResult);
BOOL dpsParseRespAbortJob(char *xmlBuff, Result * retResult);
BOOL dpsParseRespContinueJob(char *xmlBuff, Result * retResult);
BOOL dpsParseRespGetCapability(char *xmlBuff, PaperSize paperSize, DWORD capability,
							   DpsPrinterCapability * printerCap, Result * retResult);

//#endif // SC_USBDEVICE

#endif	//_DPS_COMMON__H_

