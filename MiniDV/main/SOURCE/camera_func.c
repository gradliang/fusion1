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
* Filename      : Camera_Func.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

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
#include "Camera_func.h"
#include "UI_FileSystem.h"
#include "../../../libiPlay/libsrc/sensor/include/sensor.h"
#include "PhotoFrame.h"
#include "xpgFunc.h"

#if (SENSOR_ENABLE == ENABLE)

#if CAMCODERRECORD_OLDMODE //frank lin 20110209
static WORD cameraFileNameFolderPath[] = {'/', 'D', 'C', 'I', 'M', '/', 0};
static BYTE cameraExtFileName[] = {'J', 'P', 'G', 0};

SDWORD Camera_PreviewStart(E_CAMERA_RESOLUTION resolution)
{
    BYTE captureSizeId;

    MP_DEBUG("%s -", __FUNCTION__);
#if PHOTO_FRAME
    Camera_PhotoFrameSetup(TRUE);
    Camera_PhotoFrameSetup(FALSE);
#endif
    switch (resolution)
    {
    default:
        //SetupCameraCaptureSizeSet(CAMERA_RESOLUTION_640x480);
    case CAMERA_RESOLUTION_640x480:
        SetImageSize(SIZE_VGA_640x480);
        break;

    case CAMERA_RESOLUTION_1024x768:
        SetImageSize(SIZE_XGA_1024x768);
        break;

    case CAMERA_RESOLUTION_1280x1024:
        SetImageSize(SIZE_SXGA_1280x1024);
        break;

    case CAMERA_RESOLUTION_1600x1200:
        SetImageSize(SIZE_2M_1600x1200);
        break;

    case CAMERA_RESOLUTION_2048x1536:
        SetImageSize(SIZE_QXGA_2048x1536);
        break;

    case CAMERA_RESOLUTION_2560x1920:
        SetImageSize(SIZE_5M_2560x1920);
        break;
    }

    SetSensorInterfaceMode(MODE_CAPTURE);

  //set 720x480 JPEG resolution
  SetImageSize(SIZE_480P_720x480);

  return API_Sensor_Initial();
 


}

SDWORD Camera_PreviewStop()
{
    MP_DEBUG("%s -", __FUNCTION__);
    SetSensorOverlayEnable(DISABLE); //add by Frank Lin's requirement
     API_Sensor_Stop();
#if PHOTO_FRAME
    PhotoFrameRelease(FALSE);
    PhotoFrameRelease(TRUE);
#endif
    return PASS;
}

SDWORD Camera_Capture(E_CAMERA_RESOLUTION resolution)
{
    STREAM *handle;
    DWORD diskSize;
    DWORD serialNumber;
    BYTE captureSizeId;

    MP_DEBUG("%s -", __FUNCTION__);

#if SAVE_FILE_TO_SPI
    if (g_boSaveFileToSPI == 0)
#endif
    {
        if (SystemGetFlagReadOnly(DriveCurIdGet()))
    	{
            MP_ALERT("--E-- %s: Write protected !!", __FUNCTION__);
            TaskSleep(4000);
            return FAIL;
    	}
    }

    switch (resolution)
    {
    case CAMERA_RESOLUTION_640x480:
        captureSizeId = SIZE_VGA_640x480;
        MP_DEBUG("Capture size 640x480");
        break;

    case CAMERA_RESOLUTION_1024x768:
        captureSizeId = SIZE_XGA_1024x768;
        MP_DEBUG("Capture size 1024x768");
        break;

    case CAMERA_RESOLUTION_1280x1024:
        captureSizeId = SIZE_SXGA_1280x1024;
        MP_DEBUG("Capture size 1280x1024");
        break;

    case CAMERA_RESOLUTION_1600x1200:
        captureSizeId = SIZE_2M_1600x1200;
        MP_DEBUG("Capture size 1600x1200");
        break;

    case CAMERA_RESOLUTION_2048x1536:
        captureSizeId = SIZE_QXGA_2048x1536;
        MP_DEBUG("Capture size 2048x1536");
        break;

    case CAMERA_RESOLUTION_2560x1920:
        captureSizeId = SIZE_5M_2560x1920;
        MP_DEBUG("Capture size 2560x1920");
        break;
    }
        //SetupCameraCaptureSizeSet(CAMERA_RESOLUTION_640x480);

#if SAVE_FILE_TO_SPI
    if (g_boSaveFileToSPI)
    {
        mpDebugPrint("Capture photo save to SPI flash directly.");
    }
	else
#endif
    {
        diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
        diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

        if (diskSize < MIN_FREE_DISK_SPACE)
        {
            MP_ALERT("--E-- %s: Disk free space small than %dMB, remain %dMB", __FUNCTION__, MIN_FREE_DISK_SPACE, diskSize);
         	TaskSleep(4000);
            return FAIL;
        }
		handle = CreateFileByTime(cameraFileNameFolderPath,cameraExtFileName);
    }
    
#if 1 //set 720x480 jpeg 
    captureSizeId = SIZE_480P_720x480;
#endif

    if (handle 
#if SAVE_FILE_TO_SPI
			|| g_boSaveFileToSPI
#endif
			)
    {
        MP_DEBUG("--- captureSizeId = %d ---", captureSizeId);
        if (API_Sensor_Capture(handle, captureSizeId) != PASS)
        {
#if SAVE_FILE_TO_SPI
            if (g_boSaveFileToSPI==0)
#endif
                DeleteFile(handle);
            MP_ALERT("--E-- %s: Capture error !!!");
            return FAIL;
        }
        return PASS;
    }

    return FAIL;
}
#endif

STREAM *GetNewCaptureHandle()
{
    STREAM *handle;
    DWORD diskSize;

	  diskSize = DriveFreeSizeGet(DriveGet(DriveCurIdGet())) / 1024;  // Sector
      diskSize = diskSize * DriveSetcorSizeGet(DriveGet(DriveCurIdGet())) / 1024;

      if (diskSize < MIN_FREE_DISK_SPACE)
      {
          MP_ALERT("--E-- %s: Disk free space small than %dMB, remain %dMB", __FUNCTION__, MIN_FREE_DISK_SPACE, diskSize);
			handle=NULL;
         	TaskSleep(4000);
      }
		else
		{			handle = CreateFileByTime("/DCIM/","JPG");

		}
      return handle;

}



#endif //#if (SENSOR_ENABLE == ENABLE)

