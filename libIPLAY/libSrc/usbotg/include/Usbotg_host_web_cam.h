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
* 
* Filename		: usbotg_host_web_cam.h
* Programmer(s)	: Morse Chen
* Created Date	: 2008-12-09
* Description	: 
******************************************************************************** 
*/
#ifndef __USBOTG_HOST_WEB_CAM_H__
#define __USBOTG_HOST_WEB_CAM_H__
/*
// Include section 
*/
#include "utiltypedef.h"


#if 1//((SC_USBHOST==ENABLE) && (USBOTG_WEB_CAM==ENABLE))

//////////////////////////////////////////
//          USB Video Class 1.1         //
// Appendix A. Video Device Class Codes //
//////////////////////////////////////////
enum _VIDEO_INTERFACE_SUBCLASS_CODES_
{
    SC_UNDEFINED                    = 0x00,
    SC_VIDEOCONTROL                 = 0x01,
    SC_VIDEOSTREAMING               = 0x02,
    SC_VIDEO_INTERFACE_COLLECTION   = 0x03 
};

enum _VIDEO_CLASS_SPECIFIC_DESCRIPTOR_TYPES_
{
    CS_UNDEFINED        = 0x20, 
    CS_DEVICE           = 0x21,
    CS_CONFIGURATION    = 0x22, 
    CS_STRING           = 0x23, 
    CS_INTERFACE        = 0x24, 
    CS_ENDPOINT         = 0x25, 
};

enum _VIDEO_CLASS_SPECIFIC_VC_INTERFACE_DESCRIPTOR_SUBTYPES_
{
    VC_DESCRIPTOR_UNDEFINED = 0x00, 
    VC_HEADER               = 0x01, 
    VC_INPUT_TERMINAL       = 0x02, 
    VC_OUTPUT_TERMINAL      = 0x03, 
    VC_SELECTOR_UNIT        = 0x04, 
    VC_PROCESSING_UNIT      = 0x05, 
    VC_EXTENSION_UNIT       = 0x06,
};

/* move to usbotg_api.h
enum _VIDEO_CLASS_SPECIFIC_VS_INTERFACE_DESCRIPTOR_SUBTYPES_
{
    VS_UNDEFINED            = 0x00,
    VS_INPUT_HEADER         = 0x01,
    VS_OUTPUT_HEADER        = 0x02,
    VS_STILL_IMAGE_FRAME    = 0x03,
    VS_FORMAT_UNCOMPRESSED  = 0x04,
    VS_FRAME_UNCOMPRESSED   = 0x05,
    VS_FORMAT_MJPEG         = 0x06,
    VS_FRAME_MJPEG          = 0x07,
    //Reserved                0x08 
    //Reserved                0x09 
    VS_FORMAT_MPEG2TS       = 0x0A, 
    //Reserved                0x0B 
    VS_FORMAT_DV            = 0x0C, 
    VS_COLORFORMAT          = 0x0D,
    //Reserved                0x0E 
    //Reserved                0x0F 
    VS_FORMAT_FRAME_BASED   = 0x10,
    VS_FRAME_FRAME_BASED    = 0x11,
    VS_FORMAT_STREAM_BASED  = 0x12,
};
*/
enum _VIDEO_CLASS_SPECIFIC_ENDPOINT_DESCRIPTOR_SUBTYPES_
{
    EP_UNDEFINED    = 0x00, 
    EP_GENERAL      = 0x01, 
    EP_ENDPOINT     = 0x02, 
    EP_INTERRUPT    = 0x03,
};

enum _VIDEO_CLASS_SPECIFIC_REQUEST_CODES_
{
    RC_UNDEFINED    = 0x00,
    SET_CUR         = 0x01,
    GET_CUR         = 0x81,
    GET_MIN         = 0x82,
    GET_MAX         = 0x83,
    GET_RES         = 0x84,
    GET_LEN         = 0x85,
    GET_INFO        = 0x86,
    GET_DEF         = 0x87,
};

enum _VIDEOCONTROL_INTERFACE_CONTROL_SELECTORS_
{
    VC_CONTROL_UNDEFINED            = 0x00, 
    VC_VIDEO_POWER_MODE_CONTROL     = 0x01, 
    VC_REQUEST_ERROR_CODE_CONTROL   = 0x02, 
    //Reserved                        0x03 
};

enum _TERMINAL_CONTROL_SELECTORS_
{
    TE_CONTROL_UNDEFINED = 0x00,
};

enum _SELECTOR_UNIT_CONTROL_SELECTORS_
{
    SU_CONTROL_UNDEFINED    = 0x00,
    SU_INPUT_SELECT_CONTROL = 0x01,
};

enum _CAMERA_TERMINAL_CONTROL_SELECTORS_
{
    CT_CONTROL_UNDEFINED                = 0x00, 
    CT_SCANNING_MODE_CONTROL            = 0x01, 
    CT_AE_MODE_CONTROL                  = 0x02,
    CT_AE_PRIORITY_CONTROL              = 0x03,
    CT_EXPOSURE_TIME_ABSOLUTE_CONTROL   = 0x04,
    CT_EXPOSURE_TIME_RELATIVE_CONTROL   = 0x05,
    CT_FOCUS_ABSOLUTE_CONTROL           = 0x06,
    CT_FOCUS_RELATIVE_CONTROL           = 0x07,
    CT_FOCUS_AUTO_CONTROL               = 0x08,
    CT_IRIS_ABSOLUTE_CONTROL            = 0x09,
    CT_IRIS_RELATIVE_CONTROL            = 0x0A,
    CT_ZOOM_ABSOLUTE_CONTROL            = 0x0B,
    CT_ZOOM_RELATIVE_CONTROL            = 0x0C,
    CT_PANTILT_ABSOLUTE_CONTROL         = 0x0D,
    CT_PANTILT_RELATIVE_CONTROL         = 0x0E,
    CT_ROLL_ABSOLUTE_CONTROL            = 0x0F,
    CT_ROLL_RELATIVE_CONTROL            = 0x10,
    CT_PRIVACY_CONTROL                  = 0x11,
};

enum _PROCESSING_UNIT_CONTROL_SELECTORS_ 
{
    PU_CONTROL_UNDEFINED                        = 0x00,
    PU_BACKLIGHT_COMPENSATION_CONTROL           = 0x01,
    PU_BRIGHTNESS_CONTROL                       = 0x02,
    PU_CONTRAST_CONTROL                         = 0x03,
    PU_GAIN_CONTROL                             = 0x04,
    PU_POWER_LINE_FREQUENCY_CONTROL             = 0x05,
    PU_HUE_CONTROL                              = 0x06,
    PU_SATURATION_CONTROL                       = 0x07,
    PU_SHARPNESS_CONTROL                        = 0x08,
    PU_GAMMA_CONTROL                            = 0x09,
    PU_WHITE_BALANCE_TEMPERATURE_CONTROL        = 0x0A,
    PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL   = 0x0B,
    PU_WHITE_BALANCE_COMPONENT_CONTROL          = 0x0C,
    PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL     = 0x0D,
    PU_DIGITAL_MULTIPLIER_CONTROL               = 0x0E,
    PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL         = 0x0F,
    PU_HUE_AUTO_CONTROL                         = 0x10,
    PU_ANALOG_VIDEO_STANDARD_CONTROL            = 0x11,
    PU_ANALOG_LOCK_STATUS_CONTROL               = 0x12,
};

enum _EXTENSION_UNIT_CONTROL_SELECTORS_ 
{
    XU_CONTROL_UNDEFINED = 0x00,
};

enum _VIDEOSTREAMING_INTERFACE_CONTROL_SELECTORS_  
{
    VS_CONTROL_UNDEFINED            = 0x00,
    VS_PROBE_CONTROL                = 0x01,
    VS_COMMIT_CONTROL               = 0x02,
    VS_STILL_PROBE_CONTROL          = 0x03,
    VS_STILL_COMMIT_CONTROL         = 0x04,
    VS_STILL_IMAGE_TRIGGER_CONTROL  = 0x05,
    VS_STREAM_ERROR_CODE_CONTROL    = 0x06,
    VS_GENERATE_KEY_FRAME_CONTROL   = 0x07,
    VS_UPDATE_FRAME_SEGMENT_CONTROL = 0x08,
    VS_SYNCH_DELAY_CONTROL          = 0x09,
};

/***Device Class Codes***/
typedef enum    //Interface Class Code
{
    CC_AUDIO = 0x01,
    CC_VIDEO = 0x0e,
} INF_CLASS_CODE;


/*
typedef struct{
	DWORD BufferBeginAdd;
	DWORD BufferSize;
	BYTE BufferNumber;
} WebCamAudioBuffer;
*/

typedef unsigned char		UINT8;
typedef unsigned long       UINT32;
typedef struct _BIT_MAP_HINT_
{
    WORD   Reserved1:3;       // bits 5-7
    
    WORD   wCompWindowSize:1;   // bit  4
    WORD   wCompQuality:1;      // bit  3
    WORD   wPFrameRate:1;       // bit  2
    WORD   wKeyFrameRate:1;     // bit  1
    WORD   dwFrameInterval:1;   // bit  0
    
    WORD   Reserved:8;        // bits 8-15
} UVC_BM_HINT, *PUVC_BM_HINT;

typedef struct _VIDEO_PROBE_COMMIT_CONTROLS_
{
    UVC_BM_HINT bmHint;
    BYTE        bFormatIndex;
    BYTE        bFrameIndex;
    DWORD       dwFrameInterval;
    WORD        wKeyFrameRate;
    WORD        wPFrameRate;
    WORD        wCompQuality;
    WORD        wCompWindowSize;
    WORD        wDelay;
    DWORD       dwMaxVideoFrameSize;
    DWORD       dwMaxPayloadTransferSize;
} UVC_PROBE_COMMIT_CONTROLS, *PUVC_PROBE_COMMIT_CONTROLS;

/* move to usbotg_host.h
typedef struct _BIT_MAP_PROCESSING_UNIT_
{
    WORD   bmWbc:1;                 // bit  7 // White Balance Component
    WORD   bmWbt:1;                 // bit  6 // White Balance Temperature
    WORD   bmGamma:1;               // bit  5
    WORD   bmSharpness:1;           // bit  4
    WORD   bmSaturation:1;          // bit  3
    WORD   bmHue:1;                 // bit  2
    WORD   bmContrast:1;            // bit  1
    WORD   bmBrightness:1;          // bit  0
    
    WORD   bmDigitalMultiplierLimit:1;  // bit  15
    WORD   bmDigitalMultiplier:1;       // bit  14
    WORD   bmWbcAuto:1;                 // bit  13
    WORD   bmWbtAuto:1;                 // bit  12
    WORD   bmHueAuto:1;                 // bit  11
    WORD   bmPowerLineFreq:1;           // bit  10
    WORD   bmGain:1;                    // bit  9
    WORD   bmBacklightCompensation:1;   // bit  8
} UVC_BM_PROCESSING_UNIT, *PUVC_BM_PROCESSING_UNIT;

typedef struct _UVC_BIT_MAP_CCC_ // Containing Capabilities of the Control
{
    BYTE   bmReserved:3;      // bits 5-7
    BYTE   bmAsynchronous:1;  // bit  4
    BYTE   bmAutoupdate:1;    // bit  3
    BYTE   bmDisable:1;       // bit  2
    BYTE   bmSetValue:1;      // bit  1
    BYTE   bmGetValue:1;      // bit  0
} UVC_BIT_MAP_CCC, *PUVC_BIT_MAP_CCC;

typedef struct _UVC_GET_REQUEST_OF_PROCESSING_UNIT_
{
    UVC_BM_PROCESSING_UNIT  sUnit; 
    UVC_BIT_MAP_CCC         sInfo;
    BYTE                    bMin;
    BYTE                    bMax;
    BYTE                    bRes;
    BYTE                    bDef;
    BYTE                    bReserved;
} UVC_GET_REQUEST_OF_PROCESSING, *PUVC_GET_REQUEST_OF_PROCESSING;
*/


#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Initialize USB host driver for video class and audio class. 
///
///@param   none
///
///@retval  none
/// 
///@remark  The function initialize USB host driver for video class and audio class.
///
#endif
extern void  webCamPeriodicFrameListInit (WHICH_OTG eWhichOtg);

#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Inform the iPlay BabyMonitor system that the web cam is plugged out. 
///
///@param   none
///
///@retval  none
/// 
///@remark  The function inform the iPlay BabyMonitor system that the web cam is plugged out. 
///
#endif
extern void webCamDeInit(WHICH_OTG eWhichOtg);

#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Free USB host driver for video class and audio class. 
///
///@param   none
///
///@retval  none
/// 
///@remark  The function free USB host driver for video class and audio class.
///
#endif
extern void webCamSWDeInit(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Initialize the web cam device and startup USB host driver for video 
///         class and audio class. 
///
///@param   none
///
///@retval  none
/// 
///@remark  The function initialize the web cam device and startup USB host driver for video 
///         class and audio class. 
///
#endif
extern void webCamStart(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Stop the web cam device and stop USB host driver for video 
///         class and audio class. 
///
///@param   none
///
///@retval  none
/// 
///@remark  Stop the web cam device and stop USB host driver for video 
///         class and audio class. 
///
#endif
extern void webCamStop(WHICH_OTG eWhichOtg);
extern void webCamStateMachine(ST_MCARD_DEVS *pUsbh, BYTE bMcardTransferID, WHICH_OTG eWhichOtg);
extern DWORD webCamVideoGetStreamBuffer(DWORD dwBuffer, WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get the end point number of web cam device video class. 
///
///@param   none
///
///@retval  DWORD   The end point number of web cam device video class.
/// 
///@remark  Get the end point number of web cam device video class. 
///
#endif
extern DWORD webCamVideoGetStreamInterfaceEndPointNumber(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get the end point number of web cam device audio class. 
///
///@param   none
///
///@retval  DWORD   The end point number of web cam device audio class.
/// 
///@remark  Get the end point number of web cam device audio class. 
///
#endif
extern DWORD webCamAudioGetStreamInterfaceEndPointNumber(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get max packet size of web cam device video class end point. 
///
///@param   none
///
///@retval  DWORD   The max packet size of web cam device video class end point.
/// 
///@remark  Get max packet size of web cam device video class end point. 
///
#endif
extern DWORD webCamVideoGetStreamInterfaceEndPointMaxPacketSize(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get max packet size of web cam device audio class end point. 
///
///@param   none
///
///@retval  DWORD   The max packet size of web cam device audio class end point.
/// 
///@remark  Get max packet size of web cam device audio class end point. 
///
#endif
extern DWORD webCamAudioGetStreamInterfaceEndPointMaxPacketSize(WHICH_OTG eWhichOtg);

extern void webCamResetParam(WHICH_OTG eWhichOtg);
//extern DWORD webCamParseInterfaceForBestIsoEp(WHICH_OTG eWhichOtg);
extern void webCamGetAudioVideoInfo(BYTE bInterfaceClass, BYTE bInteraceNumber, WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Send command to enable or disable web cam device video interface. 
///
///@param   pUsbhDevDes   pointer to ST_USBH_DEVICE_DESCRIPTOR data structure.
///@param   action        TRUE if enable; otherwise FALSE.
///
///@retval  BOOL   0 if no error; otherwise error code.
/// 
///@remark  Send command to enable or disable web cam device video interface. 
///
#endif
extern SWORD webCamVideoSetupSetInterface(WORD wInterfaceNumber, WORD wAlternateSetting, WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Send command to enable or disable web cam device audio interface. 
///
///@param   pUsbhDevDes   pointer to ST_USBH_DEVICE_DESCRIPTOR data structure.
///@param   action        TRUE if enable; otherwise FALSE.
///
///@retval  BOOL   0 if no error; otherwise error code.
/// 
///@remark  Send command to enable or disable web cam device audio interface. 
///
#endif
extern SWORD webCamAudioSetupSetInterface(BOOL action, WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Send command to setup web cam device video/audio interface parameter. 
///
///@param   pUsbhDevDes   pointer to ST_USBH_DEVICE_DESCRIPTOR data structure
///@param   cmdData       pointer to command data buffer
///@param   type          type value of command
///@param   bRequest      request value of command
///@param   wValue        value of command
///@param   wIndex        index value of command
///@param   wLength       length of command
///
///@retval  BOOL   0 if no error; otherwise error code.
/// 
///@remark  Send command to setup web cam device video/audio interface parameter. 
///
#endif
extern SWORD webCamSetupSetCmd(BYTE *cmdData, BYTE bRequest, WORD wValue, WORD wIndex, WORD wLength, WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio data format.
///
///@param   none
///
///@retval  DWORD   The audio data format of web cam device.
/// 
///@remark  Get web cam device audio data format.
///
#endif
//extern DWORD webCamAudioGetType(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio frequency.
///
///@param   none
///
///@retval  DWORD   The audio frequency of web cam device.
/// 
///@remark  Get web cam device audio frequency.
///
#endif
//extern DWORD webCamAudioGetFreqRate(WHICH_OTG eWhichOtg);
#if 0
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio sample size.
///
///@param   none
///
///@retval  DWORD   The audio sample size of web cam device.
/// 
///@remark  Get web cam device audio sample size.
///
#endif
//extern DWORD webCamAudioGetSampleSize(WHICH_OTG eWhichOtg);

//int WebCamGetVideoStatus(WHICH_OTG eWhichOtg);
//BOOL WebCamGetAudioStatus(BYTE index, WHICH_OTG eWhichOtg);
//void WebCamClearAudioStatus(BYTE index, WHICH_OTG eWhichOtg);
//DWORD WebCamGetAudioStreamLength(BYTE index, WHICH_OTG eWhichOtg);
//DWORD WebCamGetAudioSampleSize(WHICH_OTG eWhichOtg);
//DWORD WebCamGetAudioFreqRate(WHICH_OTG eWhichOtg);
//BOOL WebCamGetAudioBuffer(WebCamAudioBuffer *pAudioBuffer, WHICH_OTG eWhichOtg);

/********************************************************************
#include "..\..\libIPLAY\libsrc\mcard\include\mcard.h"
///
///@defgroup    WEB_CAM Webcam includes Audio and Video Class
///@ingroup     USB_Host
///This implements device class of video and audio for web cam device. 
///
///The API functions show as below,
/// - WebCamGetVideoStatus
/// - WebCamGetAudioStatus
/// - WebCamGetAudioStatus
/// - WebCamClearAudioStatus
/// - WebCamGetAudioStreamLength
/// - WebCamGetAudioSampleSize
/// - WebCamGetAudioFreqRate
/// - WebCamGetAudioBuffer

///
///@defgroup    WEB_CAM_DEVICE_SYS   The API functions for Webcam
///@ingroup     WEB_CAM
///These API Functions.

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device video stream buffer status.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval   Return 0, if data in video stream buffer is ready.
///@retval   Return 1, if data in video stream buffer is not ready.
///
///@remark  Get web cam device video stream buffer status.
///
int WebCamGetVideoStatus(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio stream buffer status.
///
///@param   index   the index of audio stream buffer
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval    Return 0, if data in audio stream buffer is ready.
///@retval    Return 1, if data in audio stream buffer is not ready.
/// 
///@remark  Get web cam device audio stream buffer status.
///
BOOL WebCamGetAudioStatus(BYTE index, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Clear web cam device audio stream buffer status.
///
///@param   index   the index of audio stream buffer
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval  none
/// 
///@remark  Clear web cam device audio stream buffer status.
///
void WebCamClearAudioStatus(BYTE index, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio stream buffer length.
///
///@param   index   the index of audio stream buffer
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval  Return number of bytes in audio stream buffer.
/// 
///@remark  Get web cam device audio stream buffer length.
///
DWORD WebCamGetAudioStreamLength(BYTE index, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio sample size.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval  Return web cam audio sample size.
/// 
///@remark  Get web cam device audio sample size.
///
DWORD WebCamGetAudioSampleSize(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio frequency.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// -USBOTG0
/// -USBOTG1
/// -USBOTG_NONE
///
///@retval  Return web cam audio frequency.
/// 
///@remark  Get web cam device audio frequency.
///
DWORD WebCamGetAudioFreqRate(WHICH_OTG eWhichOtg);


#include "..\..\libIPLAY\libsrc\usbotg\include\Usbotg_host_web_cam.h"
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device audio buffer information.
///
///@param   pAudioBuffer pointer to WebCamAudioBuffer structure
///
///@retval  Return TRUE, if data in pAudioBuffer is valid.
///@retval  Return FALSE, if data in pAudioBuffer is not valid.
/// 
///@remark  Get web cam device audio buffer information.
///
BOOL WebCamGetAudioBuffer(WebCamAudioBuffer *pAudioBuffer, WHICH_OTG eWhichOtg);
********************************************************************/

//extern DWORD WebCamGetAudioType(WHICH_OTG eWhichOtg);
//extern DWORD webCamAudioGetStreamBuffer(DWORD dwBuffer, WHICH_OTG eWhichOtg);
extern void dumpiTD(volatile iTD_Structure *piTD);
//extern void webCamUsbOtgHostIssueIso(UINT32 wEndPt,UINT32 wMaxPacketSize,UINT32 wSize,UINT32 *pwBufferArray,UINT32 wOffset,UINT8 bDirection,UINT8 bMult, WHICH_OTG eWhichOtg);

#endif // SC_USBOTG

#endif // __USBOTG_HOST_WEB_CAM_H__


