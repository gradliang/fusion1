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
* Filename      : usbotg_device_extern.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __USBOTG_DEVICE_EXTERN_H__  
#define __USBOTG_DEVICE_EXTERN_H__

#include "iplaysysconfig.h"
#include "UtilTypeDef.h"


 #if (SC_USBDEVICE && USBOTG_DEVICE_EXTERN)

// Setup GetDescriptor Vendor Command - Command
#define EXTERN_CHANGE_MODE                   0xC0  // Side Monitor for Change MSDC/SideMonitor Mode
#define EXTERN_CUR_RESOLUTION               0xC1  // Side Monitor for Current Resolution
#define EXTERN_START_STOP                       0xC2  // Side Monitor for Start/Stop SideMonitor



typedef struct _EXT_CHANGE_MODE_ {

    BYTE bExtLen;
    BYTE bExtCurClass;
    BYTE bExtNextClass; 

} EXT_CHANGE_MODE, *PEXT_CHANGE_MODE;

typedef struct _EXT_CUR_RESOLUTION_ {

    BYTE bExtLen;
    BYTE bExtWidthH;
    BYTE bExtWidthL;
    BYTE bExtHeightH;
    BYTE bExtHeightL;    

} EXT_CUR_RESOLUTION, *PEXT_CUR_RESOLUTION;

typedef struct _EXT_START_STOP_ {

    BYTE bExtLen;
    BYTE bExtAct;
    BYTE bReserved; 

} EXT_START_STOP, *PEXT_START_STOP;

typedef enum 
{
    SMR_DEFAULT = 0, // by Panel
    SMR_640x480,
    SMR_800X480,
    SMR_800x600,
    SMR_720x480,
    SMR_720x576,  
    SMR_1024x600,
    SMR_1024x768,
    SMR_1280x800,
    SMR_1366x768,
    SMR_1920x1080,
    SMR_RESOLUTION_MAX    
} SIDE_MON_RESOLUTION;

typedef enum 
{
    SMA_STOP = 0,
    SMA_START,
    SMR_MAX    
} SIDE_MON_ACTION;  


void SideMonitorIn(void);
void SideMonitorOut(void);
int SideMonitorGetVideoStatus(void);
BOOL SideMonitorChangeMode(PEXT_CHANGE_MODE pbModeStr,BYTE bCurMode, BYTE bNextMode, WHICH_OTG eWhichOtg);
BOOL SideMonitorGetResolution(PEXT_CUR_RESOLUTION pbResolutionStr);
BOOL SideMonitorStartStop(PEXT_START_STOP pbActionStr, BYTE bAction);


SDWORD UsbOtgDevExternTaskInit(void);
BYTE eUsbExternDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);


#endif // SC_USBDEVICE
#endif // __USBOTG_DEVICE_EXTERN_H__

