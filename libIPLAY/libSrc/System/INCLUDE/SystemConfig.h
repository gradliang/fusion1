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
* Filename      : SystemConfig.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/
#ifndef __SYSTEM_CFG_H
#define __SYSTEM_CFG_H

/*
// Include section
*/
#include "iplaysysconfig.h"
#include "..\..\display\include\displayStructure.h"
#include "devio.h"
#include "fs.h"

///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////


// Using SYSTEM_EVENT_ID
#define MMCP_DMA_DONE               BIT0
#define PWDC_WAKEUP_FINISHED        BIT1

// physical device ID
#define DRIVE_PHY_DEV_ID            E_DEVICE_ID
#define MAX_USB_HOST_NUM            (DEV_USBOTG1_HOST_PTP - DEV_NULL)



// for System Status bit define
#define SYS_STATUS_IDLE     0x00
#define SYS_STATUS_ERROR    0x01
#define SYS_STATUS_BOOTUP   0x02
#define SYS_STATUS_INIT     0x04
#define SYS_STATUS_COPY     0x08
#define SYS_STATUS_DELETE   0x10
#define SYS_STATUS_PRINT    0x20


#define VOLUME_DEGREE       0 //16

///////////////////////////////////////////////////////////////////////////
//
//  ENUM defines
//
///////////////////////////////////////////////////////////////////////////
typedef enum{  // Change sequence for file list access reason
    OP_IMAGE_MODE = 0,
    OP_AUDIO_MODE,
    OP_MOVIE_MODE,
    OP_EBOOK_MODE,
    OP_FILE_MODE,
    OP_CLOCK_MODE,
    OP_SETUP_MODE,
    OP_MAX_FILE_LIST_MODE,

    /////////////////////////////////////
    OP_STORAGE_MODE = OP_MAX_FILE_LIST_MODE,
    OP_USBD_MSDC_MODE,
    OP_IDLE_MODE,
    OP_NETWARE_MODE,

    OP_TOTAL_MODE  /* note: OP_TOTAL_MODE must be the last item in this enumeration list ! */
} E_SYSTEM_OP_MODE;


#define MAX_FILE_LIST_MODE_NUMBER        OP_MAX_FILE_LIST_MODE


// for dwFileType
enum{
    FILE_OP_TYPE_UNKNOWN = 0,
    FILE_OP_TYPE_IMAGE,
    FILE_OP_TYPE_AUDIO,
    FILE_OP_TYPE_VIDEO,
    FILE_OP_TYPE_EBOOK,
    FILE_OP_TYPE_FOLDER,
} E_FILE_OP_TYPE;



enum {
    EXT_END = 0x23232323,
    EXT_ALL = 0x2A2A2A2A,

// photo
    EXT_JPG = 0x4A50472E,
    EXT_BMP = 0X424D502E,
    EXT_JPEG = 0x4A50452E,
    EXT_GIF = 0X4749462E,
    EXT_PNG = 0X504E472E,
    EXT_TIFF = 0X5449462E,

#if MPO    
	EXT_MPO = 0X4D504F2E,
#endif	

#if (SONY_DCF_ENABLE)
    EXT_JPE = EXT_JPEG,
    EXT_ARW = 0X4152572E,
    EXT_SRF = 0x5352462E,
    EXT_SR2 = 0x5352322E,
    EXT_JFI = 0x4A46492E,
    EXT_THM = 0x54484D2E,
#endif

// audio
    EXT_MP3 = 0x4D50332E,
    EXT_WMA = 0x574D412E,
    EXT_OGG = 0x4f47472E,
    EXT_AAC = 0x4141432E,
    EXT_M4A = 0x4D34412E,
    EXT_AC3 = 0x4143332E,
    EXT_WAV = 0x5741562E,
    EXT_RA  = 0x5241202E,
    EXT_RM  = 0x524D202E,
    EXT_RAM = 0x52414D2E,
    EXT_AMR = 0x414D522E,
// video
    EXT_MPG = 0x4D50472E,
    EXT_DAT = 0x4441542E,
    EXT_VOB = 0x564F422E,
    EXT_AVI = 0x4156492E,
    EXT_MOV = 0X4D4F562E,
    EXT_QT  = 0x51542E,
    EXT_WMV = 0x574D562E,
    EXT_ASF = 0x4153462E,
    EXT_MP4 = 0x4D50342E,
    EXT_3GP = 0X3347502E,
    EXT_MPE = 0X4D50452E,
    EXT_FLV = 0x464C562E,
    EXT_TS  = 0x5453202E,
    EXT_MTS	= 0x4D54532E,
    EXT_M2TS= 0x4D32542E,	//should be M2TS, but performs as M2T
    EXT_MKV = 0x4D4B562E,
	EXT_h264= 0x3236342E, 
	EXT_h263 =0x3236332E,

//Ebook file
    EXT_DOC = 0x444F432E,
    EXT_TXT = 0x5458542E,
    EXT_XML = 0x584D4C2E,
    EXT_RTF = 0x5254462E,
    EXT_PDF = 0x5044462E,
    EXT_EPUB = 0x4550552E, //EPU
    EXT_PGM = 0x50474D2E,  // PGM file output from PDF
    EXT_PPM = 0x50504D2E,  // PPM file output
    EXT_LRF = 0x4C52462E,
    EXT_PDB = 0x5044422E,

EXT_FNT = 0x464E542E,   // font data file
EXT_TAB = 0x5441422E,   // font tab file

};


///////////////////////////////////////////////////////////////////////////
//
//  Structure declarations
//
///////////////////////////////////////////////////////////////////////////
struct ST_IMAGE_PLAYER_TAG
{
    DWORD dwRotateInitFlag;
    DWORD dwRotateDirection;
    DWORD dwZoomInitFlag;
    DWORD dwZoomIndex;
    DWORD dwCurrentX;
    DWORD dwCurrentY;
    DWORD dwCurrentWidth;
    DWORD dwCurrentHeight;
};

struct ST_AUDIO_PLAYER_TAG
{
    DWORD dwPlayIndex;
    ST_SEARCH_INFO *pCurPlaySearchInfo;
};

struct ST_VIDEO_PLAYER_TAG
{
    DWORD dwPlayerStatus;
    DWORD dwMovieTime;
    BOOL isFullScreen;
};

struct ST_GAMMA_TAG
{
    DWORD dwIduYGamma[16][4];
};

struct ST_USER_SETTING_TAG
{

};

struct ST_STORAGE_TAG
{
    DWORD dwPrevStorageId;
    DWORD dwCurStorageId;
    DWORD dwStorageRenewCounter[MAX_DRIVE_NUM];
};

struct ST_FILE_BROWSER_TAG
{
    DWORD dwImgExtArray[3 + GIF + PNG + TIFF];//jpg, bmp, gif, png, tiff + end
    DWORD dwMovieExtArray[10];
    //the maximum format files
    DWORD dwFileExtArray[12 + GIF + PNG + WMA_ENABLE + OGG_ENABLE +
                         (2 * AAC_SW_AUDIO) +
                         AC3_AUDIO_ENABLE + (3 * RAM_AUDIO_ENABLE) + AMR_ENABLE+
                         (10*EREADER_ENABLE)];
    DWORD dwAudioExtArray[2 + WMA_ENABLE + OGG_ENABLE +
                          (2 * AAC_SW_AUDIO) +
                          AC3_AUDIO_ENABLE + (3 * RAM_AUDIO_ENABLE) + AMR_ENABLE];
#if EREADER_ENABLE
    DWORD dwEBookExtArry[10];//total have 10
#endif

    //Joint_file_list
    BYTE bFileType[MAX_FILE_LIST_MODE_NUMBER];
    DWORD pdwExtArray[MAX_FILE_LIST_MODE_NUMBER];
    DWORD dwFileListIndex[MAX_FILE_LIST_MODE_NUMBER];
    DWORD dwFileListCount[MAX_FILE_LIST_MODE_NUMBER];
    DWORD dwFileListAddress[MAX_FILE_LIST_MODE_NUMBER];

#if EREADER_ENABLE
    DWORD dwEbookCurIndex;
    DWORD dwEbookTotalFile;
    ST_SEARCH_INFO *sEbookFileList;
#endif

    DWORD dwImgAndMovCurIndex;
    DWORD dwImgAndMovTotalFile;
    ST_SEARCH_INFO *sImgAndMovFileList;   //Joint_file_list for temp compatible to old program

    DWORD dwAudioCurIndex;
    DWORD dwAudioTotalFile;
    ST_SEARCH_INFO *sAudioFileList;   //Joint_file_list for temp compatible to old program
    DRIVE *pCurDrive;

    DWORD dwSearchFileCount;

#if 1//((BT_PROFILE_TYPE & BT_FTP_SERVER) == BT_FTP_SERVER)//rick for BT FTP
    DWORD dwValid;
#endif
    ST_SEARCH_INFO sSearchFileList[FILE_LIST_SIZE];  //06.26.2006 Athena Joint_file_list
#if NETWARE_ENABLE
    char  reserved[528 * 1000];
#endif
};

typedef struct{
    BYTE bTCONId;
    BYTE bLowTCONId;
    BYTE bPanelId;
    BYTE rev0;
    WORD wInnerWidth;
    WORD wInnerHeight;
    DWORD dwXPGTag;
}ST_SCREEN_TABLE;

struct ST_SCREEN_TAG
{
    WORD wInnerWidth;
    WORD wInnerHeight;
    ST_TCON *pstCurTCON;
    ST_PANEL *pstPanel;
    struct ST_GAMMA_TAG sGamma;
    BYTE bContrast;
    BYTE bBrightness;
    BYTE bSaturation;
    BYTE bHue;
    WORD bScreenIndex;
    WORD reserved;
};

struct ST_SYSTEM_CONFIG_TAG
{
    DWORD dwErrorCode;
    DWORD dwCurrentOpMode;

    BYTE bSystemStatus;
    BYTE bImagePlayerStatus;
    BYTE bAudioPlayerStatus;
    BYTE bVideoPlayerStatus;

    struct ST_SCREEN_TAG        sScreenSetting;

    struct ST_IMAGE_PLAYER_TAG  sImagePlayer;
    struct ST_AUDIO_PLAYER_TAG  sAudioPlayer;
    struct ST_VIDEO_PLAYER_TAG  sVideoPlayer;
    struct ST_USER_SETTING_TAG  sUserSetting;
    struct ST_STORAGE_TAG       sStorage;
    struct ST_FILE_BROWSER_TAG  sFileBrowser;
};

// Clock Source Select 1 structure
typedef struct _CLOCK_SOURCE_1_
{
    DWORD   bfMcardCks:4;       // bits 28-31
    DWORD   bfReserved1:1;      // bit  27
    DWORD   bfCduCks:3;         // bits 24-26
    DWORD   bfReserved2:1;      // bit  23
    DWORD   bfMemCks:3;         // bits 20-22
    DWORD   bfCpuCks:4;         // bits 16-19
    DWORD   bfSdaCks:2;         // bits 14-15
    DWORD   bfIdu2Cks:3;        // bits 11-13
    DWORD   bfTvCks:3;          // bits 8-10
    DWORD   bfIduCks:4;         // bits 4-7
    DWORD   bfVdaCks:1;        // bit  3
    DWORD   bfScaCks:3;         // bits 0-2
} CLKSS1, *PCLKSS1;

// Clock Source Select 2 structure
typedef struct _CLOCK_SOURCE_2_
{
    DWORD   bfReserved1:1;      // bit  31
    DWORD   bfIntsram2Cks:1;    // bit  30
    DWORD   bfIntsram1Cks:2;    // bits 28-29
    DWORD   bfReserved2:2;      // bits 26-27
    DWORD   bfIntsram0Cks:2;    // bits 24-25
    DWORD   bfIrCks:2;          // bits 22-23
    DWORD   bfI2cmCks:3;        // bits 19-21
    DWORD   bfUsbhbCks:3;       // bits 16-18
    DWORD   bfReserved3:3;      // bits 13-15
    DWORD   bfUsbdCks:1;        // bit  12
    DWORD   bfReserved4:2;      // bits 10-11
    DWORD   bfUsbhCks:2;        // bits 8-9
    DWORD   bfReserved5:1;      // bit  7
    DWORD   bfMpvCks:3;         // bits 4-6
    DWORD   bfReserved6:1;      // bit  3
    DWORD   bfMpaCks:3;         // bits 0-2
} CLKSS2, *PCLKSS2;

typedef struct ST_STORAGE_TAG       ST_STORAGE;
typedef struct ST_AUDIO_PLAYER_TAG  ST_AUDIO_PLAYER;
typedef struct ST_IMAGE_PLAYER_TAG  ST_IMAGE_PLAYER;
typedef struct ST_VIDEO_PLAYER_TAG  ST_VIDEO_PLAYER;
typedef struct ST_FILE_BROWSER_TAG  ST_FILE_BROWSER;
typedef struct ST_SYSTEM_CONFIG_TAG ST_SYSTEM_CONFIG;

typedef struct stImageEffect {
    BOOL Edge_Enable;
    BYTE SelEdgeMode;
    BYTE EdgeGain;
    BOOL Blur_Enable;
    BYTE BlurGain;
    BYTE Sketch_Enable;
    BOOL SketchMode;
    BOOL BW_Enable;

    BOOL Noise_Enable;
	BYTE NoiseGain;
    BOOL Bilevel_Enable;
	BYTE BilevelGain;
	BOOL Bilevel_cbralsoGain;
	BOOL Reverse_Enable;

    BOOL CM_Enable;
    BYTE CM_Cb_Mode;
    BYTE CM_Cr_Mode;

	BOOL Y_Offset_Enable;
	BYTE Y_Offset;
	BOOL Y_Gain_Enable;
	BYTE Y_Gain;

	BOOL CbCrOffset_Enable;
	BYTE Cb_Offset;
	BYTE Cr_Offset;

	BOOL SEHG_Enable;
	BYTE SEHG_Offset;
	BYTE SEHG_Gain;
	BOOL Crayon_Enable;
	BOOL IpuGamma_Enable;
	BOOL IpuGamma_Step;
	unsigned char *IpuGammaValue[34];
} IMAGEEFFECT;



///////////////////////////////////////////////////////////////////////////
//
//  Extern variables
//
///////////////////////////////////////////////////////////////////////////
extern ST_SYSTEM_CONFIG *g_psSystemConfig;
extern ST_IMGWIN *g_pMovieScreenWin;

extern ST_SCREEN_TABLE g_mstScreenTable[SCREEN_TABLE_TOTAL_NUM];
extern BYTE g_bVolumeIndex;
extern PCLKSS1 g_psClkSs1;
extern PCLKSS2 g_psClkSs2;

//extern volatile BYTE g_bNCTable;         // 0:None   1:GB2312 table  2:SHIFT_JIS

extern BOOL boScalerBusy, boMPABusy, boXpgBusy, boMovieTotalTime;
extern BYTE g_bXpgStatus;
extern BYTE VolumeSequence[VOLUME_DEGREE + 1];

///////////////////////////////////////////////////////////////////////////
//
//  Extern functions
//
///////////////////////////////////////////////////////////////////////////
extern void SystemConfigInit(void);
//extern void SystemConfigReInit(ST_SCREEN_TABLE *pstTVBOXTable);
extern void SystemSetStatus(BYTE bStatus);
extern void SystemClearStatus(BYTE bStatus);
extern BYTE SystemGetStatus(BYTE bStatus);
extern void SystemClearErrMsg(void);
extern void SystemSetErrMsg (DWORD dwErrorCode);
extern void SystemSetErrEvent(DWORD dwErrorCode);
extern void SystemChkChipVer(void);
extern BYTE *GetSystemErrMsg (DWORD dwErrorCode);
extern DRIVE_PHY_DEV_ID  DriveIndex2PhyDevID(E_DRIVE_INDEX_ID idx);
extern E_DRIVE_INDEX_ID  DriveName2DrvIndexID(const char *name);
extern char              *DriveIndex2DrvName(E_DRIVE_INDEX_ID idx);
char                     *DeviceIndex2DrvName(DRIVE_PHY_DEV_ID phyIdx);

#if (CHIP_VER_MSB != CHIP_VER_615)
void SystemPwdcEnable(void *pwdcPrepareCallBackFunc, void *pwdcWakeupCallBackFunc);
void *SystemPwdcPrepqreCallbackGet(void);
void *SystemPwdcWakeupCallbackGet(void);
BOOL SystemPwdcStausGet(void);
#endif

DWORD SystemPanelPixelClockGet(void);
void SystemPanelSizeGet(WORD *width, WORD *height);
void McardDevInfoStore();
void McardDevInfoLoad(E_DRIVE_INDEX_ID drv_id);

void SystemCardProtectCtrlMode(BOOL enable);
void SystemCardDetectCtrlMode(BOOL enable);
DWORD SystemCardCtrlModeGet(void);
void SystemCardStatusPollingEnable(void);

#endif  //__SYSTEM_CFG_H

