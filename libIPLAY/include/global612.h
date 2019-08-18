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
* Filename      : global612.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __GLOBAL612_H
#define __GLOBAL612_H

/*
// Include section
*/
#include "flagdefine.h"
#include "BitsDefine.h"

//PLEASE run config.bat at local path like MP612_DPF\config.bat
// to copy local iplaysysconfig.h to libIPLAY
#include "iplaysysconfig.h"

#include "utiltypedef.h"
#include "utilregfile.h"

#include "bios.h"
#include "fs.h"
#include "system.h"
#include "os.h"
#include "usbotg.h"
#include "util.h"
#include "Nand_config.h"

/*
// Constant declarations
*/
/*

/*
***************************************************************************
*                               defines
***************************************************************************
*/
//for dwErrorCode use
#define NO_ERR                  0x00000000
#define ERR_NO_STORAGE          0x10000001
#define ERR_FILE_SYSTEM         0x10000002
#define ERR_JPG_SOUR_OVERFLOW   0x10000004
#define ERR_JPG_DECODE          0x10000008
#define ERR_IMAGE_DECODER       0x10000010
#define ERR_REMOVE_CURDEV       0x10000020
#define ERR_SYSTEM_TIMEOUT      0x10000040  // watchdog timeout
#define ERR_FILE_FORMAT         0x10000080
#define ERR_WRITE_PROTECT       0x10000100
#define ERR_DISK_FULL           0x10000200
#define ERR_MEM_MALLOC          0x10000400 //chasing

#define ERR_VIDEO_INVALID       0x20000001      //Invalid File Format
#define ERR_VIDEO_DECOMPRESSOR  0x20000002      //No Suitable Decompressor
#define ERR_VIDEO_FORMAT        0x20000004      //The Format Is Not Supported
#define ERR_VIDEO_OPENFILE      0x20000008      //Cannot Open File
#define ERR_VIDEO_ERROR         0x20000010      //Media File Error
#define ERR_VIDEO_STREAM        0x20000020      //stream initialize error
#define ERR_VIDEO_DEMUX	        0x20000040      //no demux found
#define ERR_VIDEO_DEMUX_OPEN	0x20000080      //demux open failed
#define ERR_VIDEO_CODEC	        0x20000100      //no video codec
#define ERR_VIDEO_OUT           0x20000200      //video out failed
#define ERR_AUDIO_CODEC	        0x20000400      //no audio codec
#define ERR_AUDIO_OUT           0x20000800      //audio out failed
#define ERR_BITRATE             0x20001000
#define ERR_RESOLUTION          0x20002000
#define ERR_AVSYNC              0x20004000
#define ERR_AV_STYSTEM          0x20008000
#define ERR_NOTSUPPORT_OVER720P 0x20010000
#define ERR_Video_MEMORY_NOTENOUGH 0x20020000  //on2 decode is not enough

#define ERR_PICT_FATAL_ERROR    0x40000001      // Fatal Error
#define ERR_PICT_WARNING_ERROR  0x40000002      // Waring Error
#define ERR_PICT_PAPER          0x40000004      // Paper Error
#define ERR_PICT_PAPER_EMPTY    0x40000008      // Paper Empty Error
#define ERR_PICT_PAPER_JAM      0x40000010      // Paper Jam Error
#define ERR_PICT_INK            0x40000020      // Ink Error
#define ERR_PICT_INK_EMPTY      0x40000040      // Ink Empty Error
#define ERR_PICT_HARDWARE       0x40000080      // Hardware Error
#define ERR_PICT_HW_COVER_OPEN  0x40000100      // Hardware Cover OpenError
#define ERR_PICT_FILE           0x40000200      // File Operation Error
#define ERR_PICT_FILE_DECODE_ERROR  0x40000400  // File Decode Error
#define ERR_PICT_DISCONNECTED       0x40000800  // Disconnected Error

#define ERR_NO_SECOND_STORAGE   0x80000001
#define ERR_There_is_no_file_set_for_background_music   0x80000002
#define ERR_There_is_no_music_file  0x80000004
#define ERR_Choose_files_for_background_music   0x80000008
#define ERR_No_file_in_this_mode    0x80000010

#define ERR_USBH_DETECT_ERROR               0x80000020      // USB Host detect the device fail
#define ERR_USBOTG0_HOST_DETECT_ERROR       0x80000020      // USBOTG0 Host detect the device fail
#define ERR_USBOTG1_HOST_DETECT_ERROR      0x80000040      // USBOTG1 Host detect the device fail
#define ERR_USER_CANCEL_COPY 0x80000080
#define ERR_Print_is_canceled 0x800000c0


// for image size requirement
#define IMAGE_MIN_WIDTH         16
#define IMAGE_MIN_HEIGHT        12

/*
#if (CHIP_VER == CHIP_VER_615)
#define DLL_CTRL_96M_72M        0x00151515
#else
#define DLL_CTRL_96M_72M        0x00151515
#endif
#define DLL_CTRL_108M           0x80121212
*/

#define FIX_POINT(a)        ((a) << 8)
#define FIX_POINT_R(a)      ((a) >> 8)
#define FIX_POINT_D(a)      ((a) * 1000)

//-------------------------------------------------
// video
//-------------------------------------------------
#define VIDEO_ON                    (VIDEO_ENABLE || MJPEG_ENABLE)

//For SS using
#define BRIGHTNESS_DEFAULT_VALUE	  		8 		//default value initial NorFlash setting table (range 0-100)
#define VOLUME_DEFAULT_VALUE				8


#define XPG_LIST_FONT_COLOR         	0
#define XPG_LIST_TIME_FONT_COLOR    	3

#define FONT_NORMAL_COLOR				0 	// Dark gray
#define FONT_SETTINGS_NORMAL_COLOR		1 	// Light gray
#define FONT_DEV_DISABLE_COLOR			3	// Gray
#define FONT_HIGHLIGHT_COLOR			5 	// White
#define FONT_GRAY125_COLOR				6	// Gray in SMF
#define FONT_SELECTED_COLOR				11	// Light blue
#define FONT_GRAY110_COLOR				12
#define FONT_GRAY149_COLOR				FONT_DEV_DISABLE_COLOR
#define FONT_WHITEGRAY_COLOR			13	// 0xcccccc -> changed
#define FONT_TIME_COLOR					14	// 0x25,0x25,0x25

/*
***************************************************************************
*                               Enum defines
***************************************************************************
*/
enum {
    INTSRAM0CKS_MEM_CLK     = 0,
    INTSRAM0CKS_CPU_CLK     = 1,
    INTSRAM0CKS_MPA_CLK     = 2,
    INTSRAM0CKS_ALWAYS_LOW  = 3,
};



enum {
    INTSRAM1CKS_MEM_CLK     = 0,
    INTSRAM1CKS_CPU_CLK     = 1,
    INTSRAM1CKS_MPA_CLK     = 2,
    INTSRAM1CKS_ALWAYS_LOW  = 3,
};



enum {
    INTSRAM2CKS_CPU_CLK     = 0,
    INTSRAM2CKS_MEM_CLK     = 1,
};



enum AO_AV_TYPE
{
    NULL_TYPE=0,
    AO_TYPE=1,
    AV_TYPE=2,
};

enum{  //for NoneXPGKeyMap
    SETUP_KEYMAP_ZERO = 0,
    SETUP_KEYMAP_DISPLAY,
    SETUP_KEYMAP_CHROMA,
    SETUP_KEYMAP_COLORMANAGEMENT,
    SETUP_KEYMAP_SHARP,
    SETUP_KEYMAP_BLUR,
    SETUP_KEYMAP_SKETCH,
    SETUP_KEYMAP_BW,
    SETUP_KEYMAP_SEBIAN,
    SETUP_KEYMAP_CM,
    SETUP_KEYMAP_REDEYESREMOVE,
    SETUP_KEYMAP_FACEDETECTION,
    SETUP_KEYMAP_DYNAMICLIGHTING,
    SETUP_KEYMAP_RTC,
    SETUP_KEYMAP_ALARM, // Jonny add 20090601
    SETUP_KEYMAP_DIGITRTC, // Jonny add 20100510
#if(BT_XPG_UI == ENABLE)
// ******************Morris BT test***************
    SETUP_KEYMAP_BT_MYDEV,
// ******************tset TB sirroM***************
#endif

#if DIALOG_BOX == ENABLE
    SETUP_KEYMAP_PASSKEY,
    SETUP_KEYMAP_MENUITEM,
    SETUP_KEYMAP_NULL,
#endif

    SETUP_KEYMAP_TOTAL,
};


/*
***************************************************************************
*                               Structure declarations
***************************************************************************
*/

typedef struct
{
   STREAM *dec_fp;
   int file_size;
   int fending;
   int frame;
   int ch;
   int srate;
   int bitrate;
   int frame_size;
   int total_time;
   int play_time;
   int *pcm4_buf[2];
   int *pcm4_buf2[2][2];//wma use
   int *sdram_buf;
   char *sdram_buf2;//ogg use
   int frame_num;
   int blockalign;// wav use it
   unsigned int waveoffset;// wav use it
} Audio_dec;



typedef struct{
    WORD wImgHeight;
    WORD wImgWidth;
    WORD wVideoType;
    WORD wImgType;
} ST_SOF0;


/*
***************************************************************************
*                               extern variables
***************************************************************************
*/
//Global612.c
extern AIU *g_psAiu;
extern BIU *g_psBiu;
extern CLOCK *g_psClock;
extern DMA *g_psDma;
extern CHANNEL *g_psDmaAiu;
extern GPIO *g_psGpio;

#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
extern IPU *g_psIpu;
extern CDU *g_psCDU;
#endif

extern ST_SEARCH_INFO_Slideshow *g_pstSlideShowIndexBuffer;

extern DWORD g_bAniFlag;
extern ST_IMGWIN g_sMovieWin;

/*
***************************************************************************
*                               Macro declarations
***************************************************************************
*/
#define mClrClkSs1()        (g_psClock->Clkss1      = 0)
#define mClrScaCks()        (g_psClkSs1->bfScaCks   = 0)
#define mClrVdaCks()        (g_psClkSs1->bfVdaCks   = 0)
#define mClrIduCks()        (g_psClkSs1->bfIduCks   = 0)
#define mClrTvCks()         (g_psClkSs1->bfTvCks    = 0)
#define mClrIdu2Cks()       (g_psClkSs1->bfIdu2Cks  = 0)
#define mClrSdaCks()        (g_psClkSs1->bfSdaCks   = 0)
#define mClrCpuCks()        (g_psClkSs1->bfCpuCks   = 0)
#define mClrMemCks()        (g_psClkSs1->bfMemCks   = 0)
#define mClrCduCks()        (g_psClkSs1->bfCduCks   = 0)
#define mClrMcardCks()      (g_psClkSs1->bfMcardCks = 0)

#define mGetScaCks()        (g_psClkSs1->bfScaCks)
#define mGetVdaCks()        (g_psClkSs1->bfVdaCks)
#define mGetIduCks()        (g_psClkSs1->bfIduCks)
#define mGetTvCks()         (g_psClkSs1->bfTvCks)
#define mGetIdu2Cks()       (g_psClkSs1->bfIdu2Cks)
#define mGetSdaCks()        (g_psClkSs1->bfSdaCks)
#define mGetCpuCks()        (g_psClkSs1->bfCpuCks)
#define mGetMemCks()        (g_psClkSs1->bfMemCks)
#define mGetCduCks()        (g_psClkSs1->bfCduCks)
#define mGetMcardCks()      (g_psClkSs1->bfMcardCks)

#define mSetScaCks(cs)      (g_psClkSs1->bfScaCks   = (cs))
#define mSetVdaCks(cs)      (g_psClkSs1->bfVdaCks   = (cs))
#define mSetIduCks(cs)      (g_psClkSs1->bfIduCks   = (cs))
#define mSetTvCks(cs)       (g_psClkSs1->bfTvCks    = (cs))
#define mSetIdu2Cks(cs)     (g_psClkSs1->bfIdu2Cks  = (cs))
#define mSetSdaCks(cs)      (g_psClkSs1->bfSdaCks   = (cs))
#define mSetCpuCks(cs)      (g_psClkSs1->bfCpuCks   = (cs))
#define mSetMemCks(cs)      (g_psClkSs1->bfMemCks   = (cs))
#define mSetCduCks(cs)      (g_psClkSs1->bfCduCks   = (cs))
#define mSetMcardCks(cs)    (g_psClkSs1->bfMcardCks = (cs))

#define mClrClkSs2()        (g_psClock->Clkss2          = 0)
#define mClrMpaCks()        (g_psClkSs2->bfMpaCks       = 0)
#define mClrMpvCks()        (g_psClkSs2->bfMpvCks       = 0)
#define mClrUsbhCks()       (g_psClkSs2->bfUsbhCks      = 0)
#define mClrUsbhbCks()      (g_psClkSs2->bfUsbhbCks     = 0)
#define mClrI2cmCks()       (g_psClkSs2->bfI2cmCks      = 0)
#define mClrIrCks()         (g_psClkSs2->bfIrCks        = 0)
#define mClrIntsram0Cks()   (g_psClkSs2->bfIntsram0Cks  = 0)
#define mClrIntsram1Cks()   (g_psClkSs2->bfIntsram1Cks  = 0)
#define mClrIntsram2Cks()   (g_psClkSs2->bfIntsram2Cks  = 0)

#define mGetMpaCks()        (g_psClkSs2->bfMpaCks)
#define mGetMpvCks()        (g_psClkSs2->bfMpvCks)
#define mGetUsbhCks()       (g_psClkSs2->bfUsbhCks)
#define mGetUsbhbCks()      (g_psClkSs2->bfUsbhbCks)
#define mGetI2cmCks()       (g_psClkSs2->bfI2cmCks)
#define mGetIrCks()         (g_psClkSs2->bfIrCks)
#define mGetIntsram0Cks()   (g_psClkSs2->bfIntsram0Cks)
#define mGetIntsram1Cks()   (g_psClkSs2->bfIntsram1Cks)
#define mGetIntsram2Cks()   (g_psClkSs2->bfIntsram2Cks)

#define mSetMpaCks(cs)      (g_psClkSs2->bfMpaCks       = cs)
#define mSetMpvCks(cs)      (g_psClkSs2->bfMpvCks       = cs)
#define mSetUsbhCks(cs)     (g_psClkSs2->bfUsbhCks      = cs)
#define mSetDtdCks(cs)      (g_psClock->Clkss2 = (g_psClock->Clkss2 & 0xfffff0ff) | (cs<<8))
#define mSetDtdGlbVar(cs)   (DtdClkSetting = (cs<<8))
#define mSetLedCks(cs)      (g_psClock->Clkss2 = (g_psClock->Clkss2 & 0xfff8ffff) | DtdClkSetting | (cs<<16))
#define mSetUsbhbCks(cs)    (g_psClkSs2->bfUsbhbCks     = cs)
#define mSetI2cmCks(cs)     (g_psClkSs2->bfI2cmCks      = cs)
#define mSetIrCks(cs)       (g_psClkSs2->bfIrCks        = cs)
#define mSetIntsram0Cks(cs) (g_psClkSs2->bfIntsram0Cks  = cs)
#define mSetIntsram1Cks(cs) (g_psClkSs2->bfIntsram1Cks  = cs)
#define mSetIntsram2Cks(cs) (g_psClkSs2->bfIntsram2Cks  = cs)

#define mDisableUsbdCks()   (g_psClkSs2->bfUsbdCks = 0)
#define mEnableUsbdCks()    (g_psClkSs2->bfUsbdCks = 1)

/*
***************************************************************************
*                               extern functions
***************************************************************************
 */
typedef BOOL (*EVENT_POLLING_CALLBACK)(void);

//Global612.c
extern void ScalerClockReduce();
extern void ScalerClockReset();
extern DWORD GetRelativeMs(void);

void SystemCardEventSet(DWORD cardIn, DRIVE_PHY_DEV_ID phyDevId);
void SystemCardFatalErrorEventSet(DRIVE_PHY_DEV_ID phyDevId);
BYTE SystemCardEvent2DrvIdGet(DWORD dwEvent);
BYTE SystemCardOutCheck(E_DRIVE_INDEX_ID driveIndex);
BOOL SystemCardPlugInCheck(E_DRIVE_INDEX_ID driveIndex);
BOOL SystemCardPresentCheck(E_DRIVE_INDEX_ID driveIndex);
WORD SystemCardSubtypeGet(E_DRIVE_INDEX_ID driveIndex);

SWORD SystemDeviceInit(E_DRIVE_INDEX_ID driveIndex);
SWORD SystemDeviceRawFormat(E_DRIVE_INDEX_ID driveIndex, BYTE deepVerify);
SWORD SystemDeviceRemove(BYTE driveIndex);
BYTE  SystemGetFlagReadOnly(BYTE driveIndex);

#if NAND_DUMPAP
SWORD SystemDeviceRawPageRead(E_DRIVE_INDEX_ID driveIndex, BYTE *buf);
SWORD SystemDeviceRawPageWrite(E_DRIVE_INDEX_ID driveIndex, BYTE *buf);
#endif

#if (SC_USBDEVICE)
BOOL SystemCheckUsbdPlugIn(void);
void  SystemDriveLunInfoInit(void);
BYTE  SystemDriveLunNumGet(E_DRIVE_INDEX_ID driveIndex);
BYTE  SystemDriveIdGetByLun(BYTE lun);
BYTE  SystemMaxLunGet(void);
DWORD SystemDriveTotalSectorGetByLun(BYTE lun);
DWORD SystemDriveSectorSizeGetByLun(BYTE lun);
BOOL  SystemDriveLunNumSet(E_DRIVE_INDEX_ID driveId, BYTE lun);
SDWORD SystemDriveReadByLun(BYTE lun, BYTE *bufPtr, DWORD lbaAddr, DWORD sectorSize);
SDWORD SystemDriveWriteByLun(BYTE lun, BYTE *bufPtr, DWORD lbaAddr, DWORD sectorSize);
void   SystemDeviceLunInfoUpdate(void);
SDWORD SystemSysDriveLunEnable(BOOL enable);
void SystemDriveLunInfoChange(E_DRIVE_INDEX_ID driveIndex);
SBYTE SystemUsbdDetectEnable(void);
void SystemUsbdDetectDisable(void);
SDWORD SystemToolDriveLunInfoChange(BOOL enable);
SDWORD SystemToolDriveLunEnable(BOOL enable);
#endif

#if SC_USBHOST
void UsbPwdcHandler(WHICH_OTG eWhichOtg, BOOL bHostSuspend);
#endif

SDWORD SystemPollingEventIdSet(BYTE newEventId);
SDWORD SystemPollingEventHandlerRegister(DWORD eventBitField, EVENT_POLLING_CALLBACK funPtr);
extern BOOL Polling_Event(void);
extern int PollByte(BYTE *polling_data, BYTE target_value, DWORD us);
extern int PollWord(WORD *polling_data, WORD target_value, DWORD us);
extern int PollDWord(DWORD *polling_data, DWORD target_value, DWORD us);
//extern int Poll(int (*polling_func) (void *), void *func_data, DWORD us);
//extern int BusyPollByte(BYTE *polling_data, BYTE target_value, DWORD us);
//extern int BusyPollWord(WORD *polling_data, WORD target_value, DWORD us);
//extern int BusyPollDWord(DWORD *polling_data, DWORD target_value, DWORD us);
//extern int BusyPoll(int (*polling_func) (void *), void *func_data, DWORD us);
//extern int _PollRegister8(volatile BYTE *polling_data, BYTE target_value, BYTE register_mask, DWORD us);
//extern int _PollRegister16(volatile WORD *polling_data, WORD target_value, WORD register_mask, DWORD us);
//extern int _BusyPollRegister8(volatile BYTE *polling_data, BYTE target_value, BYTE register_mask, DWORD us);
//extern int _BusyPollRegister16(volatile WORD *polling_data, WORD target_value, WORD register_mask, DWORD us);
extern int _PollRegister32(volatile DWORD *polling_data, DWORD target_value, DWORD register_mask, DWORD us);
extern int _BusyPollRegister32(volatile DWORD *polling_data, DWORD target_value, DWORD register_mask, DWORD us);

/*
#define PollRegister8(polling_register, target_value, register_mask, us)            _PollRegister8( &(polling_register), (target_value), (register_mask), (us) )
#define PollRegister16(polling_register, target_value, register_mask, us)           _PollRegister16( &(polling_register), (target_value), (register_mask), (us) )
#define BusyPollRegister8(polling_register, target_value, register_mask, us)        _BusyPollRegister8( &(polling_register), (target_value), (register_mask), (us) )
#define BusyPollRegister16(polling_register, target_value, register_mask, us)       _BusyPollRegister16( &(polling_register), (target_value), (register_mask), (us) )
*/
#define PollRegister32(polling_register, target_value, register_mask, us)           _PollRegister32( &(polling_register), (target_value), (register_mask), (us) )
#define BusyPollRegister32(polling_register, target_value, register_mask, us)       _BusyPollRegister32( &(polling_register), (target_value), (register_mask), (us) )

#if (BLUETOOTH == 1)
int IspReadBTDevice(BYTE *trg, SWORD size, SWORD *fp);
int IspWriteBTDevice(BYTE *data, WORD len);
#endif

// Wrapper
#define GetCurMs2               GetSysTime
#define GetCurMs                GetSysTime
#define GetElapsedMs2           SystemGetElapsedTime
#define Mcard_GetMaxLun         SystemMaxLunGet

void SystemExceptionInit(void);

/*
static inline BYTE SystemCheckSlideShow()
{
    return (g_bAniFlag & ANI_SLIDE);
}
*/

typedef struct
{
    const BYTE *ptrName;
    DWORD u32Function;    ///< 32-bits kmodule api data
} ST_KMODULEAPI;

#if (Make_TESTCONSOLE == 1)
#define MPX_KMODAPI_SET(name)\
        static ST_KMODULEAPI TempValuable_##name __attribute__ ((section(".kmodapi"))) = {(const BYTE *)&(#name), (DWORD)(name)}
#else
#define MPX_KMODAPI_SET(name)
#endif

/*Select the quality of JPEG encode*/
// Standard QT =  TABLE[ JPEG_ENCODE_TABLE_NUM ]*/
/*Total number of the encode table is (JPEG_ENCODE_TABLE_NUM+1)*/
#define   JPEG_ENCODE_QUALITY_SELECT      0
//#define   JPEG_ENCODE_QUALITY_SELECT      1
#if JPEG_ENCODE_QUALITY_SELECT
  #define ENCODE_NORMAL					    6  //7
  #define ENCODE_BETTER 					3  // 4
  #define ENCODE_BEST						1
  #define JPEG_ENCODE_TABLE_NUM   ENCODE_NORMAL
#endif

#define swap16(a)	(((a & 0xff) << 8)|((a & 0xff00) >> 8))
#define swap32(a)	(((a & 0xff) << 24)|(((a & 0xff00)>>8)<<16)|(((a & 0xff0000)>>16)<<8)|((a & 0xff000000)>>24))

#if 1
int isUTF8Style1(BYTE b1);
int isUTF8Style2(BYTE b1, BYTE b2);
int isUTF8Style3(BYTE b1, BYTE b2, BYTE b3);
int isUTF8Style4(BYTE b1, BYTE b2, BYTE b3, BYTE b4);
int isUTF8Style5(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5);
int isUTF8Style6(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6);
unsigned UTF8ToDCharS1(BYTE b1);
unsigned UTF8ToDCharS2(BYTE b1, BYTE b2);
unsigned UTF8ToDCharS3(BYTE b1, BYTE b2, BYTE b3);
unsigned UTF8ToDCharS4(BYTE b1, BYTE b2, BYTE b3, BYTE b4);
unsigned UTF8ToDCharS5(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5);
unsigned UTF8ToDCharS6(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6); 
void U8BuffToU32Char(const unsigned char* pbUtf8Buff, int* pUtf8len, unsigned* pU32Char, int* pValid);
void U32CharToU8Buff(unsigned u32char, unsigned char* pbUtf8Buff, int* pUtf8len, int* pValid);

#endif

#endif  //__GLOBAL612_H

