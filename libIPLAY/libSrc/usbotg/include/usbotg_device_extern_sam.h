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
* Filename      : usbotg_device_extern_sam.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __USBOTG_DEVICE_EXTERN_SAM_H__  
#define __USBOTG_DEVICE_EXTERN_SAM_H__


#include "iplaysysconfig.h"
#include "UtilTypeDef.h"


 #if (SC_USBDEVICE && USBOTG_DEVICE_EXTERN_SAMSUNG)
 
 /**********************************************************************
    Definition for Device Code
**********************************************************************/
#define VENDOR_ID_SAMSUNG       0x04E8  // Samsung's Vendor ID
#define MSDC_PRODUCT_ID_800P  0x2037  // Samsung's MSDC
#define EXT_PRODUCT_ID_800P     0x2038  // Samsung's Side Monitor




/* Length of header frame */
#define FRAME_HEADER_LENTH  12

/* Bit Position of Control Code in the frame*/
#define CONTROL_CODE_POSITION   3

/* For giving command to start decoding */
#define  START_DECODE   20  

/* Reset Control code to a default value */
#define CONTROL_CODE_DEFAULT_VALUE  99

/* USB chunk length */
#define CHUNK_LENGTH    512

/* Default value of index for Disp buffer */
#define DISP_BUFFER_DEFAULT_INDEX   2


 /**********************************************************************
    Functional Macro Definitions 
**********************************************************************/

/* Get length of input buffer */
#define GetLenghtOFFrame(pBuffer) ( (*((pBuffer) + 4))  |  \
	(*((pBuffer) + 5) <<8) 	|  \
	(*((pBuffer) + 6) << 16)|  \
	(*((pBuffer) + 7) << 24)   \
	)


/* Macro to calculate ISP file length from input buffer */
#define GetISPFileLength(pBuffer) ( 	(*((pBuffer) + 8)) 	    | \
        (*((pBuffer) + 9) <<8)     | \
        (*((pBuffer) + 10) << 16)  | \
        (*((pBuffer) + 11) << 24)    \
        )



void SideMonitorIn(void);
void SideMonitorOut(void);
int SideMonitorGetVideoStatus(void);
BOOL SideMonitorChangeMode(BYTE* pbModeStr,BYTE bCurMode, BYTE bNextMode, WHICH_OTG eWhichOtg);
BOOL SideMonitorGetResolution(BYTE* pbResolutionStr);

SDWORD UsbOtgDevExternTaskInit(void);
BYTE eUsbExternDataOut(WORD u16FIFOByteCount, WHICH_OTG eWhichOtg);


#endif // (SC_USBDEVICE && USBOTG_DEVICE_EXTERN_SAMSUNG)
#endif // __USBOTG_DEVICE_EXTERN_SAM_H__

