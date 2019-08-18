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
* Filename      : Camcorder_Func.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "TaskId.h"
#include "ui.h"
#include "peripheral.h"
#include "ui_timer.h"
#include "../../ui/include/uiGpio.h"
#include "Api_main.h"
#include "Camcorder_Func.h"
#include "Camera_Func.h"
#include "UI_FileSystem.h"
#include "record.h"
#include "PhotoFrame.h"
#include "../../../libiPlay/libsrc/sensor/include/sensor.h"
#include "xpgFunc.h"

#define memset          mmcp_memset
#define memcpy          mmcp_memcpy

#if (SENSOR_ENABLE == ENABLE)

static WORD videoFileNamePreFix[] = {'D', 'V', '_', 0};     // at least 3 char
static WORD videoFileNameFolderPath[] = {'/', 'V', 'I', 'D', 'E', 'O', '/', 0};
static BYTE videoExtFileName[] = {'A', 'V', 'I', 0};

static void forceDoFatScan(void)
{
    DriveFreeSizeGet(DriveGet(DriveCurIdGet()));
}

void Camcorder_VideoRecordingPreviewStart(E_CAMCORDER_TIMING recordTiming,record_argument *p)
{
    //record_argument p;
    WORD recordWidth, recordHeight;

    MP_DEBUG("%s: recordTiming = %d", __FUNCTION__, recordTiming);
    switch (recordTiming)
    {
    default:
        recordTiming = CAMCORDER_RESOLUTION_CIF;
        //SetupCamcorderTimingSet(recordTiming);
    case CAMCORDER_RESOLUTION_CIF:
        p->resolution = RESOLUTION_CIF_30;
        p->fps = 30;
        p->image_size = SIZE_CIF_352x240;
        recordWidth = 352;
        recordHeight = 240;
        MP_DEBUG("Resolution is CIF@30");
        break;

    case CAMCORDER_RESOLUTION_480P:
        p->resolution = RESOLUTION_480P_30;
        p->fps = 24;
        p->image_size = SIZE_480P_720x480;
        recordWidth = 720;
        recordHeight = 480;
        MP_DEBUG("Resolution is D1@30");
        break;

    case CAMCORDER_RESOLUTION_800x480:
        p->resolution = RESOLUTION_480P_30;
        p->fps = 20;
        p->image_size = SIZE_SVGA_800x480;
        recordWidth = 800;
        recordHeight = 480;
        MP_DEBUG("Resolution is SVGA@20");
        break;

    case CAMCORDER_RESOLUTION_SVGA:
        p->resolution = RESOLUTION_800x600;
        p->fps = 20;
        p->image_size = SIZE_SVGA_800x600;
        recordWidth = 800;
        recordHeight = 600;
        MP_DEBUG("Resolution is SVGA@20");
        break;

    case CAMCORDER_RESOLUTION_VGA:
        p->resolution = RESOLUTION_VGA_30;
        p->fps = 24;
        p->image_size = SIZE_VGA_640x480;
        recordWidth = 640;
        recordHeight = 480;
        MP_DEBUG("Resolution is VGA@30");
        break;

    case CAMCORDER_RESOLUTION_VGA_24:
        p->resolution = RESOLUTION_VGA_24;
        p->fps = 24;
        p->image_size = SIZE_VGA_640x480;
        recordWidth = 640;
        recordHeight = 480;
        MP_DEBUG("Resolution is VGA@24");
        break;

    case CAMCORDER_RESOLUTION_VGA_25:
        p->resolution = RESOLUTION_VGA_25;
        p->fps = 25;
        p->image_size = SIZE_VGA_640x480;
        recordWidth = 640;
        recordHeight = 480;
        MP_DEBUG("Resolution is VGA@25");
        break;

    case CAMCORDER_RESOLUTION_XGA:
        p->resolution = RESOLUTION_XGA_30;
        p->fps = 30;
        p->image_size = SIZE_XGA_1024x768;
        recordWidth = 1024;
        recordHeight = 768;
        MP_DEBUG("Resolution is XGA@30");
        break;

    case CAMCORDER_RESOLUTION_720P:
        p->image_size = SIZE_720P_1280x720;
        recordWidth = 1280;
        recordHeight = 720;

        if (Clock_MemFreqGet() >= 144000000)
        {
            p->resolution = RESOLUTION_720P_24;
            p->fps = 24;
            MP_DEBUG("Resolution is 720P@24");
        }
        else
        {
            p->resolution = RESOLUTION_720P_16;
            p->fps = 16;
            MP_DEBUG("Resolution is 720P@16");
        }
        break;
    }

    p->handle = NULL;
    p->movie_length = 100;
    p->quantification = r_midlle;
#if RECORD_AUDIO
    p->RecordingAudio= r_open;//0:Open 1:Close
#else
    p->RecordingAudio= r_close;//0:Open 1:Close
#endif

    return p; //(SDWORD) Api_VideoRecordingPreviewStart(&p);
}

SDWORD Camcorder_PreviewStart(E_CAMCORDER_TIMING recordTiming)
{
    record_argument p;

    MP_DEBUG("%s: recordTiming = %d", __FUNCTION__, recordTiming);
    Camcorder_VideoRecordingPreviewStart(recordTiming,&p);
    //Camcorder_TimeStampSet(recordTiming);
    Api_VideoRecordingPreviewStart(&p); // must put after Camcorder_TimeStampSet()
    forceDoFatScan();
}


SDWORD Camcorder_PreviewStop(void)
{
    SDWORD retVal;

    MP_DEBUG("%s -", __FUNCTION__);

    retVal = Api_VideoRecordingPreviewStop();

    return retVal;
}

#if (RECYCLING==ENABLE)
SDWORD CheckReleaseForFreeSize()
{
	DWORD diskSize,retry=FILE_LIST_SIZE;

    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

	while (retry && diskSize < MIN_FREE_DISK_SPACE*120/100)
	{
		if (FAIL == UI_FileSystem_EarliestFileRemove(videoFileNameFolderPath, videoFileNamePreFix, videoExtFileName))
		{
			MP_ALERT("--E-- Could not auto-remove old video file !!!");
			MP_ALERT("--E-- Cange another card or remove unused file at other folder !!!");
			return FAIL;
		}

	    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
	    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;
		retry--;
	}

}
#endif

SDWORD Camcorder_RecordStart(BOOL infiniteRecordMode)
{
    STREAM *handle;
    DWORD serialNumber;
    DWORD diskSize;
    record_argument p;
    MP_DEBUG("%s -", __FUNCTION__);
#if NAND_ENABLE
    Ui_TimerProcRemove(McardDevInfoStore);
#endif //NAND_ENABLE

#if (RECYCLING==ENABLE)

    if (infiniteRecordMode == ENABLE)       // when card full, it will automatically remove file
    {
			CheckReleaseForFreeSize();
    }
#endif	
    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;
	if (diskSize < MIN_FREE_DISK_SPACE)
	{
		MP_ALERT("--E-- %s: Disk free space small than %d MB, remain %d MB", __FUNCTION__, MIN_FREE_DISK_SPACE, diskSize);
#if OSD_ENABLE
#ifdef OSDICON_CARD_FULL
		Show_Status_Icon(OSDICON_CARD_FULL);
		Timer_Clear_Action_Icon(3000);
#endif
#endif
		return FAIL;
	}



    Api_SetRecordVolume(g_bVolumeIndex);

    handle = UI_FileSystem_AutoNameFileCreate(videoFileNameFolderPath, videoFileNamePreFix, videoExtFileName);
    //handle = UI_FileSystem_DateTimeNameFileCreate(videoFileNameFolderPath, videoExtFileName,MODE_INDEX_CAMCORDER);
    p.handle = handle;
    p.driveIndex = DriveCurIdGet();
#if RECORD_AUDIO
    p.RecordingAudio= 0;//0:Open 1:Close
#else
    p.RecordingAudio= 1;//0:Open 1:Close
#endif
    return (SDWORD) Api_VideoRecording(&p);
}

SDWORD Camcorder_RecordStop(void)
{
    MP_DEBUG("%s -", __FUNCTION__);
#if NAND_ENABLE
        McardDevInfoStore();
#endif //NAND_ENABLE
    return (SDWORD) Api_StopVideoRecording();
}

SDWORD Camcorder_RecordPause(void)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return (SDWORD) Api_VideoRecordingPause();
}

SDWORD Camcorder_RecordResume(void)
{
    MP_DEBUG("%s -", __FUNCTION__);
    return (SDWORD) Api_VideoRecordingResume();
}

#if RECORD_AUDIO
static WORD audioFileNamePreFix[] = {'A', 'U', '_', 0};     // at least 3 char
static WORD audioFileNameFolderPath[] = {'/', 'A', 'U', 'D', 'I', 'O', '/', 0};
static BYTE audioExtFileName[] = {'W', 'A', 'V', 0};

SDWORD Audio_RecordStart(BOOL infiniteRecordMode)
{
    STREAM *handle;
    DWORD diskSize;
    DWORD serialNumber;
    record_argument p;

    MP_DEBUG("%s -", __FUNCTION__);

#if NAND_ENABLE
    Ui_TimerProcRemove(McardDevInfoStore);
#endif //NAND_ENABLE

    diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
    diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

    if (infiniteRecordMode == ENABLE)       // when card full, it will automatically remove file
    {
        while (diskSize < MIN_FREE_DISK_SPACE)
        {
            if (FAIL == UI_FileSystem_EarliestFileRemove(videoFileNameFolderPath, videoFileNamePreFix, videoExtFileName))
            {
                MP_ALERT("--E-- Could not auto-remove old video file !!!");
                MP_ALERT("--E-- Cange another card or remove unused file at other folder !!!");

                return FAIL;
            }

            diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
            diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;
        }
    }
    else
    {
        if (diskSize < MIN_FREE_DISK_SPACE)
        {
            MP_ALERT("--E-- %s: Disk free space small than %d MB, remain %d MB", __FUNCTION__, MIN_FREE_DISK_SPACE, diskSize);

            return FAIL;
        }
    }

    Api_SetRecordVolume(g_bVolumeIndex);
    handle = UI_FileSystem_AutoNameFileCreate(audioFileNameFolderPath, audioFileNamePreFix, audioExtFileName);
    p.handle = handle;
	p.driveIndex = DriveCurIdGet();
//    Clock_CpuClockSelSet(CPUCKS_PLL2_DIV_2);

    return (SDWORD) Api_AudioRecording(&p);
}

SDWORD Audio_RecordStop(void)
{
    MP_DEBUG("%s -", __FUNCTION__);

#if NAND_ENABLE
    McardDevInfoStore();
#endif //NAND_ENABLE
//    Clock_CpuClockSelSet(CPUCKS_PLL2);

    return (SDWORD) Api_StopAudioRecording();
}

#endif

#if USBOTG_WEB_CAM
STREAM * GetRecordFileHandle()
{
    STREAM *handle = UI_FileSystem_AutoNameFileCreate(videoFileNameFolderPath, videoFileNamePreFix, videoExtFileName);

	return handle;
}
#endif

#endif //#if (SENSOR_ENABLE == ENABLE)

