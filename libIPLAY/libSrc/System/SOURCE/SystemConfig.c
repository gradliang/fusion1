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
* Filename      : SystemConfig.c
* Programmer(s) : TY Miao
* Created       : TY Miao
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
#include "mpTrace.h"

#include "global612.h"
#include "devio.h"
#include "taskid.h"
#include "flagdefine.h"
#include "peripheral.h"
#include "bios.h"
#include "ui.h"
#include "display.h"


///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////
BYTE g_bXpgStatus = XPG_MODE_NULL;

//system structure
static ST_SYSTEM_CONFIG stSystemConfig;
ST_SYSTEM_CONFIG *g_psSystemConfig = (ST_SYSTEM_CONFIG *) &stSystemConfig;

ST_IMGWIN *g_pMovieScreenWin;

//=============================================
//volatile BYTE g_bNCTable = LANGUAGE_S_CHINESE;        // 0:None   1:GB2312 table  2:SHIFT_JIS

#if (AUDIO_DAC == DAC_AK4387)
BYTE VolumeSequence[VOLUME_DEGREE + 1] = { 0x00,
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0xff,
};
#elif (AUDIO_DAC == DAC_WS8956)
BYTE VolumeSequence[VOLUME_DEGREE + 1] = { 0x2f,
    0x34, 0x39, 0x3e, 0x43, 0x48, 0x4d, 0x52, 0x57,
    0x5c, 0x61, 0x66, 0x6b, 0x70, 0x75, 0x7a, 0x7f,
};
#elif VOLUME_DEGREE
BYTE VolumeSequence[VOLUME_DEGREE + 1] = {0};
#endif

typedef struct _Drive_ID_Map_S
{
    const char        *drv_name;    //predefined drive name string for each Drive[] table entry
    E_DRIVE_INDEX_ID  drv_IndexID;  //used for Drive[] table entry index
    DRIVE_PHY_DEV_ID  drv_PhyDevID; //used for Mcard driver layer
} ST_DRIVE_ID_MAP;


static ST_DRIVE_ID_MAP Drive_ID_Map[] =
{
    /* note: the left column in this table must match the drive index ID enumeration in drive.h */
    {"NULL_DRIVE",          NULL_DRIVE,         DEV_NULL},
    {"USB_HOST_ID1",        USB_HOST_ID1,       DEV_USB_HOST_ID1}, //how about USB disk multiple partitions ???
    {"USB_HOST_ID2",        USB_HOST_ID2,       DEV_USB_HOST_ID2},
    {"USB_HOST_ID3",        USB_HOST_ID3,       DEV_USB_HOST_ID3},
    {"USB_HOST_ID4",        USB_HOST_ID4,       DEV_USB_HOST_ID4},
    {"USB_HOST_PTP",        USB_HOST_PTP,       DEV_USB_HOST_PTP},
    {"USBOTG1_HOST_ID1",    USBOTG1_HOST_ID1,   DEV_USBOTG1_HOST_ID1}, //for USBOTG1_HOST_ID1
    {"USBOTG1_HOST_ID2",    USBOTG1_HOST_ID2,   DEV_USBOTG1_HOST_ID2}, //for USBOTG1_HOST_ID2
    {"USBOTG1_HOST_ID3",    USBOTG1_HOST_ID3,   DEV_USBOTG1_HOST_ID3}, //for USBOTG1_HOST_ID3
    {"USBOTG1_HOST_ID4",    USBOTG1_HOST_ID4,   DEV_USBOTG1_HOST_ID4}, //for USBOTG1_HOST_ID4
    {"USBOTG1_HOST_PTP",    USBOTG1_HOST_PTP,   DEV_USBOTG1_HOST_PTP}, //for USBOTG1_PTP
    {"NAND_ISP",            NAND_ISP,           DEV_NAND_ISP},
    {"NAND_PART1",          NAND_PART1,         DEV_NAND}, //for NAND partition 1
    {"NAND_PART2",          NAND_PART2,         DEV_NAND}, //for NAND partition 2
    {"NAND_PART3",          NAND_PART3,         DEV_NAND}, //for NAND partition 3
    {"NAND_PART4",          NAND_PART4,         DEV_NAND}, //for NAND partition 4
    {"SM",                  SM,                 DEV_SM},
    {"XD",                  XD,                 DEV_XD},
    {"MS",                  MS,                 DEV_MS},
    {"SD_MMC_PART1",        SD_MMC_PART1,       DEV_SD_MMC},
    {"SD_MMC_PART2",        SD_MMC_PART2,       DEV_SD_MMC},
    {"SD_MMC_PART3",        SD_MMC_PART3,       DEV_SD_MMC},
    {"SD2",                 SD2,                DEV_SD2},
    {"CF",                  CF,                 DEV_CF},
    {"HD",                  HD,                 DEV_HD}, //ATA HDD disk partition 1
    {"HD2",                 HD2,                DEV_HD}, //ATA HDD disk partition 2
    {"HD3",                 HD3,                DEV_HD}, //ATA HDD disk partition 3
    {"HD4",                 HD4,                DEV_HD}, //ATA HDD disk partition 4
    {"SPI_FLASH_PART1",     SPI_FLASH_PART1,    DEV_SPI_FLASH}, //for SPI Flash partition 1
    {"SPI_FLASH_PART2",     SPI_FLASH_PART2,    DEV_SPI_FLASH}, //for SPI Flash partition 2
    {"SPI_FLASH_ISP",       SPI_FLASH_ISP,      DEV_SPI_FLASH_ISP},
    {"SDIO",                SDIO,               DEV_USB_WIFI_DEVICE},
    {"USB_WIFI_DEVICE",     USB_WIFI_DEVICE,    DEV_USB_WIFI_DEVICE},
    {"USB_PPP",             USB_PPP,            DEV_USB_WIFI_DEVICE},
    {"CF_ETHERNET_DEVICE",  CF_ETHERNET_DEVICE, DEV_CF_ETHERNET_DEVICE},
    {"USB_ETHERNET_DEVICE",  USB_ETHERNET_DEVICE, DEV_USB_ETHERNET_DEVICE},
    {"USB_WEBCAM_DEVICE",  USB_WEBCAM_DEVICE, 	DEV_USB_WEBCAM},		
    {"MAX_DRIVE",           MAX_DRIVE,          MAX_DEVICE_DRV}
};



///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Static function prototype
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Definition of external functions
//
///////////////////////////////////////////////////////////////////////////

E_DRIVE_INDEX_ID PhyDevID2DriveIndex(DRIVE_PHY_DEV_ID devId)
{
    int i;

    for (i = 0; i < sizeof(Drive_ID_Map)/sizeof(ST_DRIVE_ID_MAP); i++)
    {
        if (devId == Drive_ID_Map[i].drv_PhyDevID)
            return Drive_ID_Map[i].drv_IndexID;
    }

    return NULL_DRIVE;
}



DRIVE_PHY_DEV_ID DriveIndex2PhyDevID(E_DRIVE_INDEX_ID idx)
{
    int i;

    for (i = 0; i < sizeof(Drive_ID_Map)/sizeof(ST_DRIVE_ID_MAP); i++)
    {
        if (idx == Drive_ID_Map[i].drv_IndexID)
            return Drive_ID_Map[i].drv_PhyDevID;
    }

    return DEV_NULL;
}



E_DRIVE_INDEX_ID DriveName2DrvIndexID(const char *name)
{
    int i;

    for (i = 0; i < sizeof(Drive_ID_Map)/sizeof(ST_DRIVE_ID_MAP); i++)
    {
        /* here, we use char case-sensitive string compare */
        if (StringCompare08((BYTE *) name, (BYTE *) Drive_ID_Map[i].drv_name) == E_COMPARE_EQUAL)
            return Drive_ID_Map[i].drv_IndexID;
    }

    return NULL_DRIVE;
}



char *DriveIndex2DrvName(E_DRIVE_INDEX_ID idx)
{
    int i;

    for (i = 0; i < sizeof(Drive_ID_Map)/sizeof(ST_DRIVE_ID_MAP); i++)
    {
        if (idx == Drive_ID_Map[i].drv_IndexID)
            return (char *) Drive_ID_Map[i].drv_name;
    }

    return (char *) Drive_ID_Map[0].drv_name; //"NULL_DRIVE"
}



char *DeviceIndex2DrvName(DRIVE_PHY_DEV_ID phyIdx)
{
    int i;

    for (i = 0; i < sizeof(Drive_ID_Map)/sizeof(ST_DRIVE_ID_MAP); i++)
    {
        if (phyIdx == Drive_ID_Map[i].drv_PhyDevID)
            return (char *) Drive_ID_Map[i].drv_name;
    }

    return (char *) Drive_ID_Map[0].drv_name; //"NULL_DRIVE"
}


#if 0
void SystemGammaTableInit(void)
{
    register struct ST_GAMMA_TAG *psGamma = &g_psSystemConfig->sScreenSetting.sGamma;

    psGamma->dwIduYGamma[0][0] = (DWORD)67108864U; //0x0400 0000
    psGamma->dwIduYGamma[0][1] = (DWORD)807671822U; //0x3024 180E
    psGamma->dwIduYGamma[0][2] = (DWORD)1616136252U; //0x6054 483C
    psGamma->dwIduYGamma[0][3] = (DWORD)2491840620U; //0x9486 786C

    psGamma->dwIduYGamma[1][0] = (DWORD)100663296U; //0x0600 0000
    psGamma->dwIduYGamma[1][1] = (DWORD)874978064U; //0x3427 1B10
    psGamma->dwIduYGamma[1][2] = (DWORD)1717128512U; //0x6659 4D40
    psGamma->dwIduYGamma[1][3] = (DWORD)2626518898U; //0x9C8D 7F72

    psGamma->dwIduYGamma[2][0] = (DWORD)134217728U; //0x0800 0000
    psGamma->dwIduYGamma[2][1] = (DWORD)942349843U; //0x382E 1E13
    psGamma->dwIduYGamma[2][2] = (DWORD)1818186309U; //0x6C5F 5254
    psGamma->dwIduYGamma[2][3] = (DWORD)2761262713U; //0xA495 5F69

    psGamma->dwIduYGamma[3][0] = (DWORD)167837696U; //0xA010 0000
    psGamma->dwIduYGamma[3][1] = (DWORD)1009656085U; //0x3C2E 2115
    psGamma->dwIduYGamma[3][2] = (DWORD)1919178569U; //0x7264 5749
    psGamma->dwIduYGamma[3][3] = (DWORD)2895940991U; //0xAC9C 8D7F

    psGamma->dwIduYGamma[4][0] = (DWORD)201457664U; //0xC020 0000
    psGamma->dwIduYGamma[4][1] = (DWORD)1077027864U; //0x4032 2418
    psGamma->dwIduYGamma[4][2] = (DWORD)2020236366U; //0x786A 5C4E
    psGamma->dwIduYGamma[4][3] = (DWORD)3030684806U; //0xB4A49486

    psGamma->dwIduYGamma[5][0] = (DWORD)235077632U;   //0xE030 0000
    psGamma->dwIduYGamma[5][1] = (DWORD)1144334106U; //0x4435 271A
    psGamma->dwIduYGamma[5][2] = (DWORD)2121228626U; //0x7E6F 6152
    psGamma->dwIduYGamma[5][3] = (DWORD)3165363084U; //0xBCAB 9B8C

    psGamma->dwIduYGamma[6][0] = (DWORD)268763136U; //0x1005 0000
    psGamma->dwIduYGamma[6][1] = (DWORD)1211705885U; //0x4839 2A1D
    psGamma->dwIduYGamma[6][2] = (DWORD)2222286423U; //0x8475 6657
    psGamma->dwIduYGamma[6][3] = (DWORD)3300106899U; //0xC4B3 A293

    psGamma->dwIduYGamma[7][0] = (DWORD)302383360U; //0x1206 0100
    psGamma->dwIduYGamma[7][1] = (DWORD)1279012127U; //0x4C3C 2D1F
    psGamma->dwIduYGamma[7][2] = (DWORD)2323278683U; //0x8A7A 6B5B
    psGamma->dwIduYGamma[7][3] = (DWORD)3434785177U; //0xCCBA A999

    psGamma->dwIduYGamma[8][0] = (DWORD)336069120U; //0x1408 0200
    psGamma->dwIduYGamma[8][1] = (DWORD)1346383906U; //0x5040 3022
    psGamma->dwIduYGamma[8][2] = (DWORD)2424336480U; //0x9080 7060
    psGamma->dwIduYGamma[8][3] = (DWORD)3569528992U; //0xD4C2 B0A0

    psGamma->dwIduYGamma[9][0] = (DWORD)369689344U; //0x1609 0300
    psGamma->dwIduYGamma[9][1] = (DWORD)1413690148U; //0x5443 3324
    psGamma->dwIduYGamma[9][2] = (DWORD)2525328740U; //0x9685 7564
    psGamma->dwIduYGamma[9][3] = (DWORD)3704207270U;   //0xDCC9 B7A6

    psGamma->dwIduYGamma[10][0] = (DWORD)403375105U; //0x180B 0401
    psGamma->dwIduYGamma[10][1] = (DWORD)1481061927U; //0x5847 3627
    psGamma->dwIduYGamma[10][2] = (DWORD)2626386537U; //0x9C8B 7A69
    psGamma->dwIduYGamma[10][3] = (DWORD)3838951085U; //0xE4D1 BEAD

    psGamma->dwIduYGamma[11][0] = (DWORD)436995329; //0x1A0C 0501
    psGamma->dwIduYGamma[11][1] = (DWORD)1548368169U; //0x5C4A 3929
    psGamma->dwIduYGamma[11][2] = (DWORD)2727378797U; //0xA290 7A6D
    psGamma->dwIduYGamma[11][3] = (DWORD)3940074931U; //0xEAD8 C5B3

    psGamma->dwIduYGamma[12][0] = (DWORD)470681090; //0x1C0E 0602
    psGamma->dwIduYGamma[12][1] = (DWORD)1615739948U; //0x604E 3C2C
    psGamma->dwIduYGamma[12][2] = (DWORD)2828436594U; //0xa896 8472
    psGamma->dwIduYGamma[12][3] = (DWORD)4041264314U; //0xF0e0 CCBA

    psGamma->dwIduYGamma[13][0] = (DWORD)504301314; //0x1E0F 0702
    psGamma->dwIduYGamma[13][1] = (DWORD)1683046190U; //0x6451 3F2E
    psGamma->dwIduYGamma[13][2] = (DWORD)2929428854U; //0xAE9B 8976
    psGamma->dwIduYGamma[13][3] = (DWORD)4041724864U; //0xF0E7D3C0

    psGamma->dwIduYGamma[14][0] = (DWORD)537987075; //0x2011 0803
    psGamma->dwIduYGamma[14][1] = (DWORD)1750417969U; //0x6855 4231
    psGamma->dwIduYGamma[14][2] = (DWORD)3030486651U; //0xB4A1 8E7B
    psGamma->dwIduYGamma[14][3] = (DWORD)4042250951U; //0x1A5c 36C7

    psGamma->dwIduYGamma[15][0] = (DWORD)571607299; //0x2212 0903
    psGamma->dwIduYGamma[15][1] = (DWORD)1817724211U; //0x6C58 4533
    psGamma->dwIduYGamma[15][2] = (DWORD)3131478911U; //0xBAA6 937F
    psGamma->dwIduYGamma[15][3] = (DWORD)4042252749U; //0xF0EF E1CD
}
#endif


void SystemConfigInit(void)
{
    struct ST_FILE_BROWSER_TAG *psFileBrowser;
    DWORD i;
    ST_SCREEN_TABLE *pstScreenTable = &g_mstScreenTable[0];

    psFileBrowser = &g_psSystemConfig->sFileBrowser;

    // clean whole stSystemConfig
    memset((U08 *) g_psSystemConfig, 0, sizeof(ST_SYSTEM_CONFIG));

    Idu_TableSearch(pstScreenTable->bTCONId, pstScreenTable->bPanelId, pstScreenTable->wInnerWidth, pstScreenTable->wInnerHeight);
    SystemMemoryMapInit();
#if (SC_USBDEVICE)
    SystemDriveLunInfoInit();
#endif

    //system parameter
    g_psSystemConfig->dwCurrentOpMode = OP_STORAGE_MODE;
    g_psSystemConfig->dwErrorCode = NO_ERR;

    //sotrage parameter
    g_psSystemConfig->sStorage.dwPrevStorageId = 0;

    for (i = 0; i < MAX_DRIVE_NUM ; i++)
    {
        g_psSystemConfig->sStorage.dwStorageRenewCounter[i] = 0;
    }

    //image ext
    psFileBrowser->dwImgExtArray[0] = EXT_JPG;  //JPG.Photo
    psFileBrowser->dwImgExtArray[1] = EXT_BMP;  //BMP.
    psFileBrowser->dwImgExtArray[2] = EXT_END;  //####

    //audio ext
    i = 0;
#if (MP3_SW_AUDIO | MP3_MAD_AUDIO | MP3_HW_CODEC)
    psFileBrowser->dwAudioExtArray[i++] = EXT_MP3;      //MP3.Audio
#endif

#if (WMA_ENABLE)
    psFileBrowser->dwAudioExtArray[i++] = EXT_WMA;      //WMA.
#endif

#if (OGG_ENABLE)
    psFileBrowser->dwAudioExtArray[i++] = EXT_OGG;      //OGG.
#endif

#if (AAC_SW_AUDIO )
    psFileBrowser->dwAudioExtArray[i++] = EXT_AAC;      //AAC.
    psFileBrowser->dwAudioExtArray[i++] = EXT_M4A;      //M4A.
#endif

#if (AC3_AUDIO_ENABLE)
    psFileBrowser->dwAudioExtArray[i++] = EXT_AC3;      //AC3.
#endif

#if (RAM_AUDIO_ENABLE)
    psFileBrowser->dwAudioExtArray[i++] = EXT_RA;       //RA.
    psFileBrowser->dwAudioExtArray[i++] = EXT_RM;       //RM.
    psFileBrowser->dwAudioExtArray[i++] = EXT_RAM;      //RAM.
#endif

#if (AMR_ENABLE)
    psFileBrowser->dwAudioExtArray[i++] = EXT_AMR;      //AMR
#endif

    psFileBrowser->dwAudioExtArray[i] = EXT_END;        //####

    //movie ext
    psFileBrowser->dwMovieExtArray[0] = EXT_MPG;        //MPG.Video
    psFileBrowser->dwMovieExtArray[1] = EXT_AVI;        //AVI.
    psFileBrowser->dwMovieExtArray[2] = EXT_MOV;        //MOV.
    psFileBrowser->dwMovieExtArray[3] = EXT_ASF;        //ASF.
    psFileBrowser->dwMovieExtArray[4] = EXT_MP4;        //MP4.
    psFileBrowser->dwMovieExtArray[5] = EXT_3GP;        // 3GP.
    psFileBrowser->dwMovieExtArray[6] = EXT_END;        //####
    psFileBrowser->dwMovieExtArray[7] = EXT_DAT;        //MPG.Video

    /* scan all files for OP_FILE_MODE */
    psFileBrowser->dwFileExtArray[0] = EXT_ALL;         //scan all files
    psFileBrowser->dwFileExtArray[1] = EXT_END;         //####

    psFileBrowser->pdwExtArray[OP_IMAGE_MODE] = (DWORD)&psFileBrowser->dwImgExtArray[0];
    psFileBrowser->pdwExtArray[OP_AUDIO_MODE] = (DWORD)&psFileBrowser->dwAudioExtArray[0];
    psFileBrowser->pdwExtArray[OP_MOVIE_MODE] = (DWORD)&psFileBrowser->dwMovieExtArray[0];
    psFileBrowser->pdwExtArray[OP_FILE_MODE ] = (DWORD)&psFileBrowser->dwFileExtArray[0];

    psFileBrowser->bFileType[OP_IMAGE_MODE] = FILE_OP_TYPE_IMAGE;
    psFileBrowser->bFileType[OP_AUDIO_MODE] = FILE_OP_TYPE_AUDIO;
    psFileBrowser->bFileType[OP_MOVIE_MODE] = FILE_OP_TYPE_VIDEO;
    psFileBrowser->bFileType[OP_FILE_MODE ] = FILE_OP_TYPE_UNKNOWN;

    //SystemGammaTableInit();
//    g_bNCTable = 0;
}



//------------------------------------------------------------------------------------
// check system operation status
//------------------------------------------------------------------------------------
void SystemSetStatus(BYTE bStatus)
{
    g_psSystemConfig->bSystemStatus |= bStatus;
}


void SystemClearStatus(BYTE bStatus)
{
    g_psSystemConfig->bSystemStatus &= ~bStatus;
}



BYTE SystemGetStatus(BYTE bStatus)
{
    return (g_psSystemConfig->bSystemStatus & bStatus);
}



//------------------------------------------------------------------------------------
void SystemClearErrMsg(void)
{
    g_psSystemConfig->dwErrorCode = 0;
}



void SystemSetErrMsg(DWORD dwErrorCode)
{
#if 0
    g_psSystemConfig->dwErrorCode |= dwErrorCode;
#else
    if (g_psSystemConfig->dwErrorCode == 0)
        g_psSystemConfig->dwErrorCode = dwErrorCode;
#endif
    MP_ALERT("\r\n-E-0x%x", dwErrorCode);
}



void SystemSetErrEvent(DWORD dwErrorCode)
{
    if (g_psSystemConfig->dwErrorCode == 0)
    {
        g_psSystemConfig->dwErrorCode = dwErrorCode;        // temp use one error at same time
    }

    MP_ALERT("\r\n-E-%d", dwErrorCode);
    EventSet(UI_EVENT, EVENT_ERROR);
}



DWORD SystemPanelPixelClockGet(void)
{
    switch (g_psSystemConfig->sScreenSetting.pstCurTCON->bBusType)
    {
    case DISP_TYPE_LVDS_SINGLE_666:
    case DISP_TYPE_LVDS_SINGLE_888:
        return (g_psSystemConfig->sScreenSetting.pstCurTCON->wIduClock * 100000 / 7);
        break;
    case DISP_TYPE_LVDS_DOUBLE_666:
    case DISP_TYPE_LVDS_DOUBLE_888:
        return (g_psSystemConfig->sScreenSetting.pstCurTCON->wIduClock * 200000 / 7);
        break;
    default:
        return (g_psSystemConfig->sScreenSetting.pstCurTCON->wIduClock * 100000);
        break;
    }
}



void SystemPanelSizeGet(WORD *width, WORD *height)
{
    if (width)
        *width = g_psSystemConfig->sScreenSetting.pstPanel->wWidthInPix;

    if (height)
        *height = g_psSystemConfig->sScreenSetting.pstPanel->wHeightInPix;
}


