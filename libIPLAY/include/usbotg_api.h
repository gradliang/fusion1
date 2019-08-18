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
* Filename      : usbotg_api.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef _USBOTG_API_H_
#define _USBOTG_API_H_

///
///@mainpage iPlay USBOTG API
///
///Three groups are included.
///
///USB Global Control
///- Initialize and function set
///
///USB Host
///- Control
///- MSDC
///- SIDC (PTP)
///- CDC (3.5G)
///- Audio and Video Class (Webcam)
///
///USB Peripheral
///- Control
///- SIDC (Picture Bridge)
///- CDC



/*
// Include section
*/
#include "UtilRegFile.h"


// for control
///
///@defgroup  USB_Gloable_Control USB Init
///
///When system startup, USB system could be initialed after system clock, timer, OS, Memory
///management module ready. It should be initialed before system start USB transacting.
///
///USB OTG initialize and function set shows as below,
/// - Api_UsbInit
/// - Api_UsbFunctionSet
///

///
///@defgroup    USBOTG_INIT_API   Initialize and function set
///@ingroup     USB_Gloable_Control
///These API Functions.

#if 0 // move to FlagDefine.h
///
///@ingroup USBOTG_INIT_API
///@brief   Enumeration constants of which one of usb otg.
typedef enum {
    USBOTG0,            ///< for USBOTG0
    USBOTG1,            ///< for USBOTG1
    USBOTG_NONE,        ///< it's initial value and total number of item of WHICH_OTG
}WHICH_OTG;
#endif

///
///@ingroup USBOTG_INIT_API
///@brief   Enumeration constants of function for USB.
typedef enum {
    OTG_FUNC,           ///< OTG function including host and device.
    HOST_ONLY_FUNC,     ///< host function only NOT including device.
    DEVICE_ONLY_FUNC,   ///< device function only NOT including host.
    NONE_FUNC,          ///< it's initial value and total number of item of USB_OTG_FUNC
}USB_OTG_FUNC;

typedef enum _UVC_PROCESSING_CONTROLS_BITMAP_
{
    PROCESSING_BRIGHTNESS                       = 0,
    PROCESSING_CONTRAST                         = 1,
    PROCESSING_HUE                              = 2,
    PROCESSING_SATURATION                       = 3,
    PROCESSING_SHARPNESS                        = 4,
    PROCESSING_GAMMA                            = 5,
    PROCESSING_WHITE_BALANCE_TEMPERATURE        = 6,
    PROCESSING_WHITE_BALANCE_COMPONENT          = 7,
    PROCESSING_BACKLIGHT_COMPENSATON            = 8,
    PROCESSING_GAIN                             = 9,
    PROCESSING_POWER_LINE_FREQUENCY             = 10, 
    PROCESSING_HUE_AUTO                         = 11,
    PROCESSING_WHITE_BALANCE_TEMPERATURE_AUTO   = 12, 
    PROCESSING_WHITE_BALANCE_COMPONENT_AUTO     = 13,
    PROCESSING_DIGITAL_MULTIPLIER               = 14, 
    PROCESSING_DIGITAL_MULTIPLIER_LIMIT         = 15,
    PROCESSING_ANALOG_VIDEO_STANDARD            = 16,
    PROCESSING_ANALOG_VIDEO_LOCK_STATUS         = 17,
    PROCESSING_RESERVED                         = 18,
} UVC_PROCESSING_CONTROLS;

// from UVC spec 
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



///
///@ingroup USBH_WEBCAM_VIDEO_FORMAT
///@brief   Enumeration video format for USB host webcam.
typedef enum {
    USING_NONE     = VS_UNDEFINED,
    USING_MPEG2_TS = VS_FORMAT_MPEG2TS,
    USING_MJPEG    = VS_FORMAT_MJPEG,
    USING_YUV      = VS_FORMAT_UNCOMPRESSED,
} USBH_WEBCAM_VIDEO_FORMAT;

#define MAX_NUM_FRAME_INDEX          8
// Host UVC frame resolution
typedef struct _UVC_FRAME_RESOLUTION_
{
    DWORD  wWidth;
    DWORD  wHigh;
    DWORD  wFrameIndex;
    DWORD  wReserved;
} USBH_UVC_FRAME_RESOLUTION, *PUSBH_UVC_FRAME_RESOLUTION;

// Host UVC format information
typedef struct _USBH_UVC_FORMAT_INFORMATION_
{
    USBH_WEBCAM_VIDEO_FORMAT    eFormat;
    DWORD                       dwFormatIndex;
    DWORD                       dwTotalFrameNumber;
    USBH_UVC_FRAME_RESOLUTION   sFrameResolution[MAX_NUM_FRAME_INDEX];
} USBH_UVC_FORMAT_INFORMATION, *PUSBH_UVC_FORMAT_INFORMATION;

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
    UVC_BIT_MAP_CCC         sInfo;
    WORD                    wStatus;
    BYTE                    bControlSelector;
    WORD                    wMin;
    WORD                    wMax;
    WORD                    wRes;
    WORD                    wDef;
} UVC_GET_REQUEST_OF_PROCESSING, *PUVC_GET_REQUEST_OF_PROCESSING;

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

typedef struct _UVC_PROCESSING_UNIT_
{
    UVC_BM_PROCESSING_UNIT  sUnit; 
    BYTE                    bUnitID;
    BYTE                    bControlUnits;
} UVC_PROCESSING_UNIT, *PUVC_PROCESSING_UNIT;

///
///@ingroup USBOTG_INIT_API
///@brief   Enumeration constants of error code for USB.
enum _USB_ERROR_CODE_
{
    USBOTG_NO_ERROR             =  0,   ///< No error
    USBOTG_NO_MEMORY            = -1,   ///< allocat memory failed
    USBOTG_NOT_INIT_YET         = -2,   ///< USB did not initial yet
    USBOTG_PARAMETER_ERROR      = -3,   ///< the parameter is not allowed
    USBOTG_MESSAGE_CREATE_ERROR = -4,   ///< message create failed
    USBOTG_EVENT_CREATE_ERROR   = -5,   ///< event create failed
    USBOTG_MAILBOX_CREATE_ERROR = -6,   ///< mailbox create failed
    USBOTG_TASK_CREATE_ERROR    = -7,   ///< task create failed
    USBOTG_TASK_STARTUP_ERROR   = -8,   ///< startup task failed
    USBOTG_UNKNOW_ERROR         = -10,  ///< unknow error
    USBOTG_ISOC_QUEUE_FULL      = -11,	///< isoc data queue full
    USBOTG_ISOC_QUEUE_EMPTY 	= -12,	///< isoc data queue empty
    USBOTG_MAILBOX_SEND_ERROR   = -13,  ///< mailbox send failed
    USBOTG_DEVICE_NOT_EXIST     = -14,  ///< device not exist
    USBOTG_DEVICE_NOT_READY     = -15,  ///< device not ready
    USBOTG_DEVICE_IS_BUSY       = -16,  ///< device is busy
    USBOTG_DEVICE_NOT_START_YET = -17,  ///< device not start yet
};

typedef struct _USBH_UVC_STILL_IMAGE_
{
    BYTE    bFormatIndex;
    BYTE    bFrameIndex;
    BYTE    bCompressionIndex;
    DWORD   dwMaxVideoFrameSize;
    DWORD   dwMaxPayloadTransferSize;
} UVC_STILL_IMAGE, *PSUVC_STILL_IMAGE;



///
///@ingroup USBOTG_INIT_API
///@brief   initial the data structure of USB OTG0/OTG1.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark  assign initial value for each item of USBOTG DS.
///
///
void Api_UsbInit(WHICH_OTG eWhichOtg);

///
///@ingroup USBOTG_INIT_API
///@brief   assign function for USB OTG0/OTG1 by this API function which will allocate
///          memory and assign validated value for each item in USB OTG DS (Data Structure).
///
///@param   sUsbOtg_0_Func  assign function for USBOTG0
///@param   sUsbOtg_1_Func  assign function for USBOTG1
/// - OTG_FUNC
/// - HOST_ONLY_FUNC
/// - DEVICE_ONLY_FUNC
/// - NONE_FUNC
///
///@retval  USBOTG_NO_ERROR         no error.
///@retval  USBOTG_NO_MEMORY        allocate memory failed.
///@retval  USBOTG_NOT_INIT_YET     the related items in DS are not initial.
///@retval  USBOTG_PARAMETER_ERROR  the parameter is not defined.
///
///@remark  allocate the related memory blocks by ker_mem_malloc.
///
///
SDWORD Api_UsbFunctionSet (USB_OTG_FUNC sUsbOtg_0_Func, USB_OTG_FUNC sUsbOtg_1_Func);

#if (SC_USBHOST==ENABLE)
//USB Host
///
///@defgroup  USB_Host USB Host
///
///iPlay USB Host supported many classes. For stardard classes, support Mass Storage
/// Device Class (MSDC) for all pen-drives and digital camera; Still Image Device Class
/// (SIDC) for PTP device like as Canon DC; Video and Audio Class for webcam which
///have implemented Sonix's device; Wireless Controller Device Class for Blue Tooth.
///For vendor class, support Wifi and some BT dongle.

#if (USBOTG_HOST_USBIF || USBOTG_HOST_EYE)
// Host Test mode
///
///@defgroup    USBH_TEST_MODE USB-IF Test
///@ingroup     USB_Host
///This is for USB-IF Embedded Host Electrical Test.
///
///
///@ingroup USBH_TEST_MODE
///@brief   In test mode, it's able to test eye-diagram
///         Call this function and change diagram-mode in order.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
///
///@remark  While system into the host test mode, will not back to normal mode.
///
void Api_UsbIfTestMode(WHICH_OTG eWhichOtg);
#endif
// Host MSDC
///
///@defgroup    USBH_MSDC Mass Storage Device Class
///@ingroup     USB_Host
///Mass Storage Device Class (MSDC) is for all kind of storages like as pen-drive, digital camera ect..
///
///The API functions show as below,
/// - Api_UsbhStorageDeviceInit
/// - Api_UsbhStorageDeviceRemove

///
///@defgroup    USBH_MSDC_API   Device init and remove
///@ingroup     USBH_MSDC
///For App to initial and cleart the variable in MCARD for USB mass storage
///


///@ingroup USBH_MSDC_API
///@brief   update the status to present for USB mass storage card in
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  MCARD_CMD_PASS  always pass.
///
///@remark  This function init the internal variables of Mcard
///
SWORD Api_UsbhStorageDeviceInit (BYTE bMcardID);

///@ingroup USBH_MSDC_API
///@brief   clear the status for USB mass storage card out
///
///@param   bMcardID    ID number of the Mcard device driver
///
///@retval  MCARD_CMD_PASS  always pass.
///
///@remark  This function clears the internal variables of Mcard
///
SWORD Api_UsbhStorageDeviceRemove (BYTE bMcardID);

#if (USBOTG_WEB_CAM==ENABLE)
///
///@defgroup    WEB_CAM Webcam includes Audio and Video Class
///@ingroup     USB_Host
///This implements device class of video and audio for web cam device.
///
///The API functions show as below,
/// - Api_UsbhWebCamSetBrightness
/// - Api_UsbhWebCamSetContrast
/// - Api_UsbhWebCamSetGamma
/// - Api_UsbhWebCamVideoGetTotalResolutions
/// - Api_UsbhWebCamVideoGetResolutionByIndex
/// - Api_UsbhWebCamVideoGetCurResolution
/// - Api_UsbhWebCamVideoGetCurResolutionIndex
/// - Api_UsbhWebCamVideoSetResolutionByIndex
/// - Api_UsbhWebCamStartPlay
/// - Api_UsbhWebCamStopPlay
///

///@defgroup    WEB_CAM_DEVICE_SYS   The state of Webcam
///@ingroup     WEB_CAM
///@brief   there are four states.
typedef enum {
    WEB_CAM_STATE_NOT_EXIST,
    WEB_CAM_STATE_NOT_READY,
    WEB_CAM_STATE_INIT_READY,
    WEB_CAM_STATE_BUSY,
} WEBCAM_STATE;
/*
///@defgroup    WEB_CAM_DEVICE_SYS   The API functions for Webcam
///@ingroup     WEB_CAM
///These API Functions.

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Set web cam device brightness (0 ~ 7).
///
///@param   brightness  brightness value (0 ~ 7)
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark  Set web cam device brightness.
///
void Api_UsbhWebCamSetBrightness(WORD brightness, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Set web cam device contrast (0 ~ 255).
///
///@param   contrast   contrast value (0 ~ 255)
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark  Set web cam device contrast.
///
void Api_UsbhWebCamSetContrast(WORD contrast, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Set web cam device gamma (0 ~ 255).
///
///@param   gamma      gamma value (0 ~ 255)
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark  Set web cam device gamma.
///
void Api_UsbhWebCamSetGamma(WORD gamma, WHICH_OTG eWhichOtg);
*/
///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device total resolutions supported.
///
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  N the total number of resolution setting that web cam supports.
/// - the Sonix webcam return the total number of resolution setting 4.
///
///@remark  The total number of resolution setting is depended on webcam.
///
DWORD Api_UsbhWebCamVideoGetTotalResolutions(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device resolution by index.
///
///@param   index       index of resolution (1 ~ 4)
/// - 1     640 x 480
/// - 2     320 x 240
/// - 3     176 x 144
/// - 4     160 x 120
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  0x028001E0      640 x 480
///@retval  0x014000F0      320 x 240
///@retval  0x00B00090      176 x 144
///@retval  0x00A00078      160 x 120
/// - Return resolution by index number. (width.height: high word is width, low word is height)
///
///@remark  Get web cam device resolution by index.
///
DWORD Api_UsbhWebCamVideoGetResolutionByIndex(DWORD index, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get web cam device current resolution.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  0x028001E0      640 x 480
///@retval  0x014000F0      320 x 240
///@retval  0x00B00090      176 x 144
///@retval  0x00A00078      160 x 120
/// - Return resolution by index number. (width.height: high word is width, low word is height)
///
///@remark  Get web cam device current resolution.
///
DWORD Api_UsbhWebCamVideoGetCurResolution(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Get index of web cam device current resolution.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval   1      640 x 480
///@retval   2      320 x 240
///@retval   3      176 x 144
///@retval   4      160 x 120
/// - Return index of resolution (1 ~ 4).
///
///@remark  Get index of web cam device current resolution.
///
DWORD Api_UsbhWebCamVideoGetCurResolutionIndex(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Set web cam device resolution by index.
///
///@param   index       index of resolution

///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
//
///@remark  Set web cam device resolution by index.
///
void Api_UsbhWebCamVideoSetResolutionByIndex(DWORD index, WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Start web cam device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  PASS    the WEB_CAM_AUDIO_TASK and WEB_CAM_VIDEO_TASK have started up.
///@retval  FAIL    the related memory block allocated fail.
///
///@remark  Start web cam device.
///
int Api_UsbhWebCamStartPlay(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   Stop web cam device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  PASS    the related tasks are terminated and memory block is relaesed.
///
///@remark  Stop web cam device.
///
int Api_UsbhWebCamStopPlay(WHICH_OTG eWhichOtg);


///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   check wethether connected with WebCam device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE  Connected with WebCam device.
///@retval  FAIL   Disconnect with WebCam device.
///
///@remark  Stop web cam device.
///
BOOL Api_UsbhCheckIfConnectedWebCam(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   assign function pointer for parsind data coming from isoc ep.
///
///@param   void*   assign function pointer
///
///@remark  assign function pointer for parsind data coming from isoc ep.
///
void Api_UsbhRegistForIsoc(void *pFunctionPointer);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   web cam start.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  USBOTG_NO_ERROR             no error and web cam starts, or start already.
///@retval  USBOTG_DEVICE_NOT_EXIST     device not exist.
///@retval  USBOTG_MAILBOX_SEND_ERROR   mail to usb host task fail 
///
///@remark  Start web cam device.
///
SDWORD Api_UsbhWebCamStart(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   web cam stop.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  USBOTG_NO_ERROR             no error and web cam stop, or stop already.
///@retval  USBOTG_DEVICE_NOT_EXIST     device not exist.
///@retval  USBOTG_MAILBOX_SEND_ERROR   mail to usb host task fail 
///
///@remark  Stop web cam device.
///
SDWORD Api_UsbhWebCamStop(WHICH_OTG eWhichOtg);

///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   web cam video fromat setting.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///@param   eVideoFormat   assign USING_MPEG2_TS/USING_MJPEG
/// - USING_MPEG2_TS
/// - USING_MJPEG
///
///@retval  USBOTG_NO_ERROR            no error and web cam stop, or stop already.
///@retval  USBOTG_PARAMETER_ERROR     the assigned vedio format does not support.
///
///@remark  select video fromat for webcam.
///
SDWORD Api_UsbhWebCamVedioFormat(WHICH_OTG eWhichOtg, 
                                 USBH_WEBCAM_VIDEO_FORMAT eVideoFormat,
                                 USBH_UVC_FRAME_RESOLUTION sFrameResolution);


///
///@ingroup WEB_CAM_DEVICE_SYS
///@brief   get the state of webcam.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  WEB_CAM_STATE_NOT_EXIST     device not exist.
///@retval  WEB_CAM_STATE_NOT_READY     device is not ready to play.
///@retval  WEB_CAM_STATE_INIT_READY    device is ready to play after init.
///@retval  WEB_CAM_STATE_STOP_READY    device is ready to play after stop.
///@retval  WEB_CAM_STATE_BUSY          device is playing.
///
///@remark  Stop web cam device.
///
WEBCAM_STATE Api_UsbhGetWebCamState(WHICH_OTG eWhichOtg);

PUSBH_UVC_FORMAT_INFORMATION Api_UsbhWebCamVideoGetForamtInfo(WHICH_OTG eWhichOtg);


#endif //(USBOTG_WEB_CAM==ENABLE)

#if USBOTG_HOST_HID

///@ingroup USBH_HID_API
///@brief   enumeration for USB HID Device.
typedef enum {
    USB_HOST_HID_NONE	= 0,     ///< not defined
    USB_HOST_HID_KEYBOARD,       ///< Keyboard
    USB_HOST_HID_MOUSE,          ///< Mouse
}  USB_HOST_HID_DEVICE;

///@ingroup USBH_HID_API
///@brief   Get key value from HID class device.
///
///@param  none
///
///@retval  Key value.
///
///@remark  none
///
DWORD Api_UsbhHidGetKey(WHICH_OTG eWhichOtg);

///@ingroup USBH_HID_API
///@brief   Set bKey to key value.
///
///@param   bKey   Key value
///
///@retval  none
///
///@remark  none
///
void Api_UsbhHidSetKey(DWORD bKey, WHICH_OTG eWhichOtg);

///
///@ingroup USBH_HID_API
///@brief   check wethether connected with HID device(Keyboard/Mouse).
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE  Connected with HID device(Keyboard/Mouse).
///@retval  FAIL   Disconnect with HID device(Keyboard/Mouse).
///
///@remark
///
BOOL Api_UsbhHidCheckIfConnectedKeyboard(WHICH_OTG eWhichOtg);
BOOL Api_UsbhHidCheckIfConnectedMouse(WHICH_OTG eWhichOtg);
///
///@ingroup USBH_HID_API
///@brief   Get HID device is Keyboard or Mouse.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  0  Nothing
///@retval  1  Keyboard.
///@retval  2  Mouse
///
///@remark
///
USB_HOST_HID_DEVICE Api_UsbhHidGetWhichDevice(WHICH_OTG eWhichOtg);

///@ingroup USBH_HID_API
///@brief   Get which OTG connected with HID in first device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval USBOTG0 means connected PC with USB OTG 0.
///@retval USBOTG1 means connected PC with USB OTG 1.
///@retval USBOTG_NONE means no PC connected.
///
///@remark  it calls Api_UsbdCheckIfConnectedPc with each OTG.
///
WHICH_OTG Api_UsbhHidGetWhichOtg(BYTE bHidDevice);

#endif

#endif //(SC_USBHOST==ENABLE)



#if (SC_USBDEVICE==ENABLE)
///
///@defgroup  USB_Peripheral USB Peripheral
///
///iPlay USB device supported Mass Storage Device Class (MSDC) for card reader application,
///Still Image Device Class (SIDC) for Picture Bridge and Communication Device Class (CDC)
///for USB COM port.
///

//Device control
///
///@defgroup    USBD_CTRL  Control for USB Peripheral
///@ingroup     USB_Peripheral
///
/// The API functions show as below,
/// - Api_UsbdInit
/// - Api_UsbdFinal
/// - Api_UsbdSetMode
/// - Api_UsbdGetMode
/// - Api_UsbdCheckIfConnectedPc
/// - Api_UsbdGetWhichOtgConnectedPc
/// - Api_UsbdCheckIfConnectedPrinter
/// - Api_UsbdGetWhichOtgConnectedPrinter
/// - Api_UsbdGetDetectPlugFlag
/// - Api_UsbdDpsGetPrinterName

#include "..\..\libIPLAY\libsrc\usbotg\include\usbotg_ctrl.h"
///
///@defgroup    USBD_CTRL_API   Init and final USBD and get the device class mode
///@ingroup     USBD_CTRL
///These API Functions.

///@ingroup USBD_CTRL_API
///@brief   Initial the software and hardware related setting.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///@remark  this function should be called after plug-in event occurs.
///
void Api_UsbdInit(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Initial the software for next plug-ing and set unplug bit.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///@remark  this function should be called after plug-out event occurs.
///
void Api_UsbdFinal(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   enumeration for USB device class.
typedef enum {
    USB_MODE_NONE	= 0,    ///< not defined
    USB_AP_MSDC_MODE,       ///< Mass Storage Device Class
    USB_AP_SIDC_MODE,       ///< Still Image Device Class
    USB_AP_CDC_MODE,        ///< Communication Device Class
    USB_AP_EXTERN_MODE,     ///< Vendor Device Class for SideMonitor
    USB_AP_VENDOR_MODE,     ///< Vendor Device Class
    USB_AP_UAVC_MODE,		///< UVC/UAC Device Class
}  USB_DEV_CLASS_MODE;


///@ingroup USBD_CTRL_API
///@brief   Set the USB Application Class for USB device.
///
///@param   mode    USB application mode. There are four modes MSDC,
/// SIDC, CDC and Vendor classes.
/// - USB_MODE_NONE
/// - USB_AP_MSDC_MODE
/// - USB_AP_SIDC_MODE
/// - USB_AP_CDC_MODE
/// - USB_AP_VENDOR_MODE
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///
///@remark  Application should set the USB mode before transacting.
///
void Api_UsbdSetMode(USB_DEV_CLASS_MODE mode, WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Get the USB Application Class for USB device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  USB_MODE_NONE       not defined yet.
///@retval  USB_AP_MSDC_MODE    Mass Storage Device Class for being card reader on PC.
///@retval  USB_AP_SIDC_MODE    Still Image Device Class for Picture Bridge.
///@retval  USB_AP_CDC_MODE     Communication Device Class for debugging like as UART.
///@retval  USB_AP_VENDOR_MODE  Vendor Device Class for other application by specific protocol.
///
///@remark  Application is able know what the USB mode is by calling this function.
///
USB_DEV_CLASS_MODE Api_UsbdGetMode(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Check if connects device.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE means connected device.
///@retval  FALSE means disconnect or the parameter is not acceptable.
///
///@remark  it checks the ID pin if connected.
///
BOOL Api_UsbdCheckIfConnectedDevice(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Get which OTG connected device
///
///@retval USBOTG0 means connected device with USB OTG 0.
///@retval USBOTG1 means connected device with USB OTG 1.
///@retval USBOTG_NONE means no device connected.
///
///@remark  it calls Api_UsbdCheckIfConnectedDevice with each OTG.
///
WHICH_OTG Api_UsbdGetWhichOtgConnectedDevice(void);

///@ingroup USBD_CTRL_API
///@brief   Check if connects PC.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE means connected printer with MSDC protocol.
///@retval  FALSE means disconnect or the parameter is not acceptable.
///
///@remark  it checks the ID pin if connect to host and initiated with MSDC protocol.
///
BOOL Api_UsbdCheckIfConnectedPc(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Get which OTG connected PC.
///
///@retval USBOTG0 means connected PC with USB OTG 0.
///@retval USBOTG1 means connected PC with USB OTG 1.
///@retval USBOTG_NONE means no PC connected.
///
///@remark  it calls Api_UsbdCheckIfConnectedPc with each OTG.
///
WHICH_OTG Api_UsbdGetWhichOtgConnectedPc(void);

///@ingroup USBD_CTRL_API
///@brief   Check if connects Printer.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE means connected printer with SIDC protocol.
///@retval  FALSE means disconnect or the parameter is not acceptable.
///
///@remark  it checks the ID pin if connect to host and initiated with SIDC protocol.
///
BOOL Api_UsbdCheckIfConnectedPrinter(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Get which OTG connected Printer.
///
///@retval USBOTG0 means connected printer with USB OTG 0.
///@retval USBOTG1 means connected printer with USB OTG 1.
///@retval USBOTG_NONE means no printer connected.
///
///@remark  it calls Api_UsbdCheckIfConnectedPrinter with each OTG.
///
WHICH_OTG Api_UsbdGetWhichOtgConnectedPrinter(void);

#if ( USBOTG_DEVICE_EXTERN || USBOTG_DEVICE_EXTERN_SAMSUNG )
//@ingroup USBD_CTRL_API
///@brief   Check if connects PC for Side Monitor.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval  TRUE means connected printer with Vendor protocol.
///@retval  FALSE means disconnect or the parameter is not acceptable.
///
///@remark  it checks the ID pin if connect to Device and initiated with Vendor protocol.
///
BOOL Api_UsbdCheckIfConnectedSideMonitor(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Get which OTG connected PC for Side Monitor.
///
///@retval USBOTG0 means connected printer with USB OTG 0.
///@retval USBOTG1 means connected printer with USB OTG 1.
///@retval USBOTG_NONE means no printer connected.
///
///@remark  it calls Api_UsbdCheckIfConnectedSideMonitor with each OTG.
///
WHICH_OTG Api_UsbdGetWhichOtgConnectedSideMonitor(void);
#endif

///@ingroup USBD_CTRL_API
///@brief   Get the plug flag to represent plug-in/plug-out.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1

///@retval 0 means plug-out.
///@retval 1 means plug-in.
///
///@remark   this falg is assigned in UsbOtgDeviceDetect being called periodical in timer func.
BYTE Api_UsbdGetDetectPlugFlag (WHICH_OTG eWhichOtg);


///@ingroup USBD_CTRL_API
///@brief   enumeration for USB device current state from Chapter 9 of USB 2.0 SPEC.
typedef enum {
    USB_DEV_STATE_NONE  = 0,      ///< not defined
    USB_DEV_STATE_ATTACHED,     ///< Attached to Host
    USB_DEV_STATE_POWERED,       ///< Powered from Host
    USB_DEV_STATE_DEFAULT,        ///< Default Address after Reset Signal from Host
    USB_DEV_STATE_ADDRESS,        ///< Set Address Finish
    USB_DEV_STATE_CONFIGURED,  ///< Configured from Host
    USB_DEV_STATE_SUSPENDED,     ///< Suspended
    USB_DEV_STATE_MAX                  ///< Max State to check
}  USB_DEV_CUR_STATE;


///@ingroup USBD_CTRL_API
///@brief   Get the USB device current state.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval USB device current state.
///
///@remark  None.
///
USB_DEV_CUR_STATE Api_UsbdGetCurrentStat(WHICH_OTG eWhichOtg);

///@ingroup USBD_CTRL_API
///@brief   Set the USB device current state.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///
///@remark  None.
///
void Api_UsbdSetCurrentStat(WHICH_OTG eWhichOtg, USB_DEV_CUR_STATE enUsbState);

///
///@defgroup    PictBridge Picture Bridge
///@ingroup     USB_Peripheral
///  For Picture Bridge compatible devices.
///
///  Picture Bridge API function is as below,
///  - Api_UsbdDpsGetPrinterCapability
///  - Api_UsbdDpsAddImage
///  - Api_UsbdDpsSetPrintJobConfig
///  - Api_UsbdDpsSetPrintAction
///  - Api_UsbdDpsGetDeviceStatus


///
///@ingroup PictBridge_API
///@brief   Get the printer name.
///
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@retval the pointer of BYTE for the printer product name
///
BYTE* Api_UsbdDpsGetPrinterName(WHICH_OTG eWhichOtg);


///
///@defgroup    PictBridge_API   Picture Bridge API Function
///@ingroup     PictBridge
///These API Functions.

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For printer capability
typedef enum {
    DPS_PARAM_QUALITY,          ///< Quality capability. Refer to ePrinterCapabilityQuality enum.
    DPS_PARAM_PAPER_SIZE,       ///< Paper Size capability. Refer to ePrinterCapabilityPaperSize enum.
    DPS_PARAM_PAPER_TYPE,       ///< Paper Type capability. Refer to ePrinterCapabilityPaperType enum.
    DPS_PARAM_FILE_TYPE,        ///< File Type capability. Refer to ePrinterCapabilityFileType enum.
    DPS_PARAM_DATE_PRINT,       ///< Date Print capability. Refer to ePrinterCapabilityDatePrint enum.
    DPS_PARAM_FILENAME_PRINT,   ///< Filename Print capability. Refer to ePrinterCapabilityFileNamePrint enum.
    DPS_PARAM_IMAGE_OPTIMIZE,   ///< Image Optimize capability. Refer to ePrinterCapabilityImageOptimize enum.
    DPS_PARAM_LAYOUT,           ///< Layout capability. Refer to ePrinterCapabilityLayout enum.
    DPS_PARAM_FIXED_SIZE,       ///< Fixed Size capability. Refer to ePrinterCapabilityFixedSize enum.
    DPS_PARAM_CROPPING,         ///< Cropping capability. Refer to ePrinterCapabilityCropping enum.
    DPS_PARAM_PRINTERCAP_MAX    ///< Max enum value.
} ePrinterCapability;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_QUALITY
typedef enum {
    DPS_Quality_noQuality,	///< No Quality.
    DPS_Quality_Default,    ///< Default Quality by Printer.
    DPS_Quality_Normal,     ///< Normal Quality.
    DPS_Quality_Draft,      ///< Draft Quality.
    DPS_Quality_Fine,       ///< Fine Quality.
    DPS_Quality_MAX         ///< Max enum value.
}ePrinterCapabilityQuality;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_PAPER_SIZE
typedef enum {
	DPS_PaperSize_noPaperSize,              ///< No Paper Size.
	DPS_PaperSize_defPaperSize,             ///< Default Paper Size by Printer.
	DPS_PaperSize_L,                        ///< L  Size.
	DPS_PaperSize_L2,                       ///< L2  Size.
	DPS_PaperSize_Postcard,                 ///< Postcard  Size.
	DPS_PaperSize_Card,                     ///< Card  Size.
	DPS_PaperSize_100x150,                  ///< 100x150  Size.
	DPS_PaperSize_4x6,                      ///< 4x6  Size.
	DPS_PaperSize_8x10,                     ///< 8x10  Size.
	DPS_PaperSize_Letter,                   ///< Letter  Size.
	DPS_PaperSize_11x17,                    ///< 11x17  Size.
	//511n0000 An (n=0~9) (Note2)                    // (n=0~9) (Note2)
	DPS_PaperSize_A0,                       ///< A0  Size.
	DPS_PaperSize_A1,                       ///< A1  Size.
	DPS_PaperSize_A2,                       ///< A2  Size.
	DPS_PaperSize_A3,                       ///< A3  Size.
	DPS_PaperSize_A4,                       ///< A4  Size.
	DPS_PaperSize_A5,                       ///< A5 Size.
	DPS_PaperSize_A6,                       ///< A6 Size.
	DPS_PaperSize_A7,                       ///< A7 Size.
	DPS_PaperSize_A8,                       ///< A8 Size.
	DPS_PaperSize_A9,                       ///< A9  Size.
	//512m0000 Bm (m=0~9) (Note3)	             // (m=0~9) (Note3)
	DPS_PaperSize_B0,                       ///< B0  Size.
	DPS_PaperSize_B1,                       ///< B1  Size.
	DPS_PaperSize_B2,                       ///< B2 Size.
	DPS_PaperSize_B3,                       ///< B3 Size.
	DPS_PaperSize_B4,                       ///< B4 Size.
	DPS_PaperSize_B5,                       ///< B5 Size.
	DPS_PaperSize_B6,                       ///< B6 Size.
	DPS_PaperSize_B7,                       ///< B7 Size.
	DPS_PaperSize_B8,                       ///< B8 Size.
	DPS_PaperSize_B9,                       ///< B9 Size.
	DPS_PaperSize_Roll_L,                   ///< Roll_L Size.
	DPS_PaperSize_Roll_2L,                  ///< Roll_2L Size.
    DPS_PaperSize_MAX                       ///< Max enum value.
}ePrinterCapabilityPaperSize;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_PAPER_TYPE
typedef enum {
    DPS_PaperType_noPaperType,      ///< No Paper Type.
    DPS_PaperType_defPaperType,     ///< Default Paper Type by Printer.
    DPS_PaperType_PlainPaperType,   ///< Plain Paper Type.
    DPS_PaperType_PhotoPaperType,   ///< Photo Paper Type.
    DPS_PaperType_FastPaperType,    ///< Fast Paper Type.
    DPS_PaperType_MAX               ///< Max enum value.
}ePrinterCapabilityPaperType;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_FILE_TYPE
typedef enum {
    DPS_FileType_noFileType,        ///< No format.
    DPS_FileType_defFileType,       ///< Default format by Printer.
    DPS_FileType_Exit_Jpeg,         ///< Exit_Jpeg format.
    DPS_FileType_Exit,              ///< Exit format.
    DPS_FileType_JPEG,              ///< JPEG format.
    DPS_FileType_TIFF_EP,           ///< TIFF_EP format.
    DPS_FileType_FlashPix,          ///< FlashPix format.
    DPS_FileType_Bmp,               ///< Bmp format.
    DPS_FileType_CIFF,              ///< CIFF format.
    DPS_FileType_GIF,               ///< GIF format.
    DPS_FileType_JFIF,              ///< JFIF format.
    DPS_FileType_PCD,               ///< PCD format.
    DPS_FileType_PICT,              ///< PICT format.
    DPS_FileType_PNG,               ///< PNG format.
    DPS_FileType_TIFF,              ///< TIFF format.
    DPS_FileType_TIFF_IT,           ///< TIFF_IT format.
    DPS_FileType_JP2,               ///< JP2 format.
    DPS_FileType_JPX,               ///< JPX format.
    DPS_FileType_Undefined,         ///< Undefined format.
    DPS_FileType_Association,       ///< Association format (Folder).
    DPS_FileType_Script,            ///< Script format.
    DPS_FileType_Executable,        ///< Executable format.
    DPS_FileType_Text,              ///< Text format.
    DPS_FileType_HTML,              ///< HTML format.
    DPS_FileType_XHTML,             ///< XHTML format.
    DPS_FileType_DPOF,              ///< DPOF format.
    DPS_FileType_AIFF,              ///< AIFF format.
    DPS_FileType_WAV,               ///< WAV format.
    DPS_FileType_MP3,               ///< MP3 format.
    DPS_FileType_AVI,               ///< AVI format.
    DPS_FileType_MPEG,              ///< MPEG format.
    DPS_FileType_ASF,               ///< ASF format.
    DPS_FileType_MAX                ///< Max enum value.
}ePrinterCapabilityFileType;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_DATE_PRINT
typedef enum {
    DPS_DatePrint_noDatePrint,  ///< No print date.
    DPS_DatePrint_defDatePrint, ///< Default print date by Printer.
    DPS_DatePrint_dateOff,      ///< Turn off Printing date.
    DPS_DatePrint_dateOn,       ///< Turn on Printing date.
    DPS_DatePrint_MAX           ///< Max enum value.
}ePrinterCapabilityDatePrint;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_FILENAME_PRINT
typedef enum {
    DPS_FileNamePrint_noNamePrint,  ///< No print name.
    DPS_FileNamePrint_defFNamePrt,  ///< Default print name by Printer.
    DPS_FileNamePrint_filePrtOff,   ///< Turn off Printing name.
    DPS_FileNamePrint_filePrtON,    ///< Turn on Printing name.
    DPS_FileNamePrint_MAX           ///< Max enum value.
}ePrinterCapabilityFileNamePrint;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_IMAGE_OPTIMIZE
typedef enum {
    DPS_ImageOptimize_noImageOpt,   ///< No image optimize.
    DPS_ImageOptimize_defImgOpt,    ///< Default image optimize by Printer.
    DPS_ImageOptimize_optOff,       ///< Turn off image optimize.
    DPS_ImageOptimize_optOn,        ///< Turn on image optimize.
    DPS_ImageOptimize_MAX           ///< Max enum value.
}ePrinterCapabilityImageOptimize;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_LAYOUT
typedef enum {
    DPS_Layout_noLayout,        ///< No layout.
    DPS_Layout_defLayout,       ///< Default layout by Printer.
    DPS_Layout_oneUp,           ///< 1 layout.
    DPS_Layout_twoUp,           ///< 2 layout.
    DPS_Layout_threeUp,         ///< 3 layout.
    DPS_Layout_fourUp,          ///< 4 layout.
    DPS_Layout_fiveUp,          ///< 5 layout.
    DPS_Layout_sixUp,           ///< 6 layout.
    DPS_Layout_sevenUp,         ///< 7 layout.
    DPS_Layout_eightUp,         ///< 8 layout.
    DPS_Layout_indexLayout,     ///< Index Print.
    DPS_Layout_oneFullBleed,    ///< Borderless.
    DPS_Layout_MAX              ///< Max enum value.
}ePrinterCapabilityLayout;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_FIXED_SIZE
typedef enum {
    DPS_FixedSize_noFixedSize,  ///< No fixed size.
    DPS_FixedSize_defFixedSize, ///< Default fixed size by Printer.
    DPS_FixedSize_size2x3,      ///< Fixed size 2x3.
    DPS_FixedSize_size3x5,      ///< Fixed size 3x5.
    DPS_FixedSize_size4x6,      ///< Fixed size 4x6.
    DPS_FixedSize_size5x7,      ///< Fixed size 5x7.
    DPS_FixedSize_size8x10,     ///< Fixed size 8x10.
    DPS_FixedSize_MAX           ///< Max enum value.
}ePrinterCapabilityFixedSize;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_PARAM_CROPPING
typedef enum {
    DPS_Cropping_noCropping,    ///< Max enum value.
    DPS_Cropping_defCropping,   ///< Default crop by Printer.
    DPS_Cropping_cropOff,       ///< Turn off cropping.
    DPS_Cropping_cropOn,        ///< Turn on cropping.
    DPS_Cropping_MAX            ///< Max enum value.
}ePrinterCapabilityCropping;


///
///@ingroup PictBridge_API
///@brief   Get the printer capability by this function.
///
///@param   type            Printer capability type.
/// - Refer to ePrinterCapability enum.
///@param   pTypeVaue   Get printer capability array that you selected printer capability type and know what printer settings.
/// - Refer to ePrinterCapabilityQuality enum for DPS_PARAM_QUALITY type.
/// - Refer to ePrinterCapabilityPaperSize enum for DPS_PARAM_PAPER_SIZE type.
/// - Refer to ePrinterCapabilityPaperType enum for DPS_PARAM_PAPER_TYPE type.
/// - Refer to ePrinterCapabilityFileType enum for DPS_PARAM_FILE_TYPE type.
/// - Refer to ePrinterCapabilityDatePrint enum for DPS_PARAM_DATE_PRINT type.
/// - Refer to ePrinterCapabilityFileNamePrint enum for DPS_PARAM_FILENAME_PRINT type.
/// - Refer to ePrinterCapabilityImageOptimize enum for DPS_PARAM_IMAGE_OPTIMIZE type.
/// - Refer to ePrinterCapabilityLayout enum for DPS_PARAM_LAYOUT type.
/// - Refer to ePrinterCapabilityFixedSize enum for DPS_PARAM_FIXED_SIZE type.
/// - Refer to ePrinterCapabilityCropping enum for DPS_PARAM_CROPPING type.
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark None
void Api_UsbdDpsGetPrinterCapability(ePrinterCapability type, BYTE* pTypeVaue, WHICH_OTG eWhichOtg);
///
///@ingroup PictBridge_API
///@brief   Select the printed-image you want by this function.
///
///@param   fileID              The FileID is the image index start from 0.
///@param   copies              Image copies.
///@param   bFileDatePrint    Whether print photo with File Date.
/// - Note: Need to call Api_UsbdDpsSetPrintJobConfig(DPS_PARAM_DATE_PRINT, DPS_DatePrint_dateOn);
///@param   bFileNamePrint   Whether print photo with File Name.
/// - Note: Need to call Api_UsbdDpsSetPrintJobConfig(DPS_PARAM_FILENAME_PRINT, DPS_FileNamePrint_filePrtON);
///@param   cropX               The printed picture with crop format.
///@param   cropY               The printed picture with crop format.
///@param   cropW               The printed picture with crop format.
///@param   cropH               The printed picture with crop format.
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark  If fileID is not photo format, please don't call this function by UI.
void Api_UsbdDpsAddImage (  DWORD fileID,
                    DWORD copies,
                    BOOL bFileDatePrint,
                    BOOL bFileNamePrint,
                    DWORD cropX,
                    DWORD cropY,
                    DWORD cropW,
                    DWORD cropH,
                    WHICH_OTG eWhichOtg);
///
///@ingroup PictBridge_API
///@brief   Set the printer capability by this function.
///
///@param   JobConfigId        Printer capability type.
/// - Refer to ePrinterCapability enum.
///@param   JobConfigValue    Set printer capability that you selected printer capability type.
/// - Refer to ePrinterCapabilityQuality enum for DPS_PARAM_QUALITY type.
/// - Refer to ePrinterCapabilityPaperSize enum for DPS_PARAM_PAPER_SIZE type.
/// - Refer to ePrinterCapabilityPaperType enum for DPS_PARAM_PAPER_TYPE type.
/// - Refer to ePrinterCapabilityFileType enum for DPS_PARAM_FILE_TYPE type.
/// - Refer to ePrinterCapabilityDatePrint enum for DPS_PARAM_DATE_PRINT type.
/// - Refer to ePrinterCapabilityFileNamePrint enum for DPS_PARAM_FILENAME_PRINT type.
/// - Refer to ePrinterCapabilityImageOptimize enum for DPS_PARAM_IMAGE_OPTIMIZE type.
/// - Refer to ePrinterCapabilityLayout enum for DPS_PARAM_LAYOUT type.
/// - Refer to ePrinterCapabilityFixedSize enum for DPS_PARAM_FIXED_SIZE type.
/// - Refer to ePrinterCapabilityCropping enum for DPS_PARAM_CROPPING type.
///@param   eWhichOtg   assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark None
void Api_UsbdDpsSetPrintJobConfig(ePrinterCapability JobConfigId, DWORD JobConfigValue, WHICH_OTG eWhichOtg);


///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For printer's action.
typedef enum {
    DPS_PARAM_START_JOB,        ///< Start print job.
    DPS_PARAM_ABORT_JOB,        ///< Abort print job.
    DPS_PARAM_CONTINUE_JOB,     ///< Continue print job.
    DPS_PARAM_INIT_JOB,         ///< Initiate print job.
    DPS_PARAM_PRINTACTION_MAX   ///< Max enum value.
} ePrintAction;

///
///@ingroup PictBridge_API
///@brief   Set printer action.
///
///@param   act    set printer action as below.
/// - Refer to ePrintAction enum.
///@param   eWhichOtg  assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark None
void Api_UsbdDpsSetPrintAction(ePrintAction act, WHICH_OTG eWhichOtg);


///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For printer's status.
typedef enum {
    DPS_DevStat_PrintStatus,    ///< Print status. Refer to ePrintingStatus enum.
    DPS_DevStat_JobEnd,         ///< The result of job end. Refer to eJobEnd enum.
    DPS_DevStat_ErrStatus,      ///< Error status. Refer to eErrorStatus enum.
    DPS_DevStat_ErrReason,      ///< Error reason. Refer to eErrorReason enum.
    DPS_DevStat_DisconEn,       ///< Whether disconnect with Printer. Refer to eDisconEn enum.
    DPS_DevStat_CapabilityChg,  ///< Printer capability had changed. Refer to eCapabilityChg enum.
    DPS_DevStat_NewJobOK,       ///< Whether new print job. Refer to eNewJobOK enum.
    DPS_DevStat_MAX             ///< Max enum value.
} eDeviceStatus;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_PrintStatus.
typedef enum
{
    DPS_PrintStatus_Printing,   ///< Printing.
    DPS_PrintStatus_Idle,       ///< Idle.
    DPS_PrintStatus_Paused,     ///< Paused.
    DPS_PrintStatus_MAX         ///< Max enum value.
} ePrintingStatus;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_JobEnd.
typedef enum {
    DPS_JobEnd_NotEnded,                ///< Print job have not ended.
    DPS_JobEnd_Ended,                   ///< Print job have ended.
    DPS_JobEnd_AbortJob_Immediate,      ///< Print job have ended by aborting job immediately.
    DPS_JobEnd_AbortJob_NoImmediate,    ///< Print job have ended by aborting job.
    DPS_JobEnd_OtherReason,             ///< Print job have ended by other reason.
    DPS_JobEnd_MAX                      ///< Max enum value.
} eJobEnd;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_ErrStatus.
typedef enum {
    DPS_ErrStatus_NoError,      ///< No error.
    DPS_ErrStatus_Warning,      ///< Warning.
    DPS_ErrStatus_FatalError,   ///< Fatal error.
    DPS_ErrStatus_MAX           ///< Max enum value.
} eErrorStatus;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_ErrReason.
typedef enum {
    DPS_ErrReason_NoReason,     ///< No reason.
    DPS_ErrReason_PaperError,   ///< Paper error.
    DPS_ErrReason_InkError,     ///< Ink error.
    DPS_ErrReason_HWError,      ///< Hardware error.
    DPS_ErrReason_FileError,    ///< File error.
    DPS_ErrReason_MAX           ///< Max enum value.
} eErrorReason;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_DisconEn.
typedef enum {
    DPS_DisconEn_FALSE, ///< Printer can't disconnect.
    DPS_DisconEn_TRUE,  ///< Printer can disconnect.
    DPS_DisconEn_MAX    ///< Max enum value.
} eDisconEn;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_CapabilityChg.
typedef enum {
    DPS_CapabilityChg_FALSE,    ///< Printer capability have not changed.
    DPS_CapabilityChg_TRUE,     ///< Printer capability have changed.
    DPS_CapabilityChg_MAX       ///< Max enum value.
} eCapabilityChg;

///@ingroup PictBridge_API
///@brief   Enumeration for DPS.
///@remark  For DPS_DevStat_NewJobOK.
typedef enum {
    DPS_newJobOK_FALSE, ///< Printer can't new print job.
    DPS_newJobOK_TRUE,  ///< Printer can new print job.
    DPS_newJobOK_MAX    ///< Max enum value.
} eNewJobOK;


///
///@ingroup PictBridge_API
///@brief   Get printer status.
///
///@param   *DeviceStatus    Get printer status array including PrintStatus, JobEnd, ErrStatus, ErrReason, DisconEn, CapabilityChg, NewJobOK.
/// - Array index refer to eDeviceStatus enum.
/// - Array value
///    - DeviceStatus[DPS_DevStat_PrintStatus]:Refer to ePrintingStatus enum.
///    - DeviceStatus[DPS_DevStat_JobEnd]:Refer to eJobEnd enum.
///    - DeviceStatus[DPS_DevStat_ErrStatus]:Refer to eErrorStatus enum.
///    - DeviceStatus[DPS_DevStat_ErrReason]:Refer to eErrorReason enum.
///    - DeviceStatus[DPS_DevStat_DisconEn]:Refer to eDisconEn enum.
///    - DeviceStatus[DPS_DevStat_CapabilityChg]:Refer to eCapabilityChg enum.
///    - DeviceStatus[DPS_DevStat_NewJobOK]:Refer to eNewJobOK enum.
///    refer to eDeviceStatus enum.
///@param   eWhichOtg         assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark    None
///
void Api_UsbdDpsGetDeviceStatus(DWORD* DeviceStatus, WHICH_OTG eWhichOtg);


///
///@ingroup PictBridge_API
///@brief   Print current photo for the last time fileID of Api_UsbdDpsAddImage.
///
///@param   bCurPhoto         print current photo.
/// - TRUE
/// - FALSE
///@param   eWhichOtg         assign USBOTG0/USBOTG1
/// - USBOTG0
/// - USBOTG1
///
///@remark    None
///
void Api_UsbdDpsCurPhotoPrint(BOOL bIsCurPhotoPrint, WHICH_OTG eWhichOtg);


///
///@ingroup USBD_CTRL_API
///@brief   Register USB Device Protection call-back function.
///
///@param   *VendorPasswordSet     Set user password.
///@param   *VendorPasswordCmp   Compare user password.
///@param   *VendorPatitionSwitch    Change other partition.
///
///@remark    None
///
void Api_UsbdProtectCBFunc(BOOL (*VendorPasswordSet) (BYTE* pbString, DWORD dwLen),
                                                        BOOL (*VendorPasswordCmp) (BYTE* pbString, DWORD dwLen),
                                                        BOOL (*VendorPatitionSwitch) (BYTE bPartition));

#endif //(SC_USBDEVICE==ENABLE)

#endif /* End of _USBOTG_API_H_ */



