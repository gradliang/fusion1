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
* Filename		: usbotg_device_webcam.c
* Programmer(s)	: Morse
* Created Date	: 2010/05/06 
* Description   : USB Device UVC/UAC Class
******************************************************************************** 
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
#include "typedef.h"
#include "string.h" 
#include "taskid.h"
#include "SysConfig.h"
#include "usbotg_std.h"
#include "usbotg_device.h"
#include "usbotg_device_msdc.h"
#include "usbotg_ctrl.h"
#include "usbotg_device_vac.h"
#include "../../sensor/include/sensor.h"
#if (SC_USBDEVICE && WEB_CAM_DEVICE)

#define PCCAM_STRING  "ENJOY Camera"

/***Video Device Class Descriptor***/
typedef struct
{
    U08 bLength;    //12+n
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U16 bcdUVC;
    U16 wTotalLength;
    U32 dwClockFrequency;
    U08 bInCollection;  //=n
    U08 baInterfaceNr[8];
} ST_VC_INF_HEADER_DESC;

typedef struct
{
    U08 bLength;    //8(+x)
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bTerminalID;
    U16 wTerminalType;
    U08 bAssocTerminal;
    U08 iTerminal;
} ST_VIDEO_IN_TERMINAL_DESC;

typedef struct
{
    U08 bLength;    //9(+x)
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bTerminalID;
    U16 wTerminalType;
    U08 bAssocTerminal;
    U08 bSourceID;
    U08 iTerminal;
} ST_VIDEO_OUT_TERMINAL_DESC;

typedef struct
{
    U08 bLength;    //15+n
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bTerminalID;
    U16 wTerminalType;
    U08 bAssocTerminal;
    U08 iTerminal;
    U16 wObjectiveFocalLengthMin;
    U16 wObjectiveFocalLengthMax;
    U16 wOcularFocalLength;
    U08 bControlSize;   //=n
    U08 bmControls[8];
} ST_CAM_TERMINAL_DESC;

typedef struct
{
    U08 bLength;    //6+p
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bUnitID;
    U08 bNrInPins;  //=p
    U08 baSourceID[8];
    //the last byte is iSelector
} ST_SEL_UNIT_DESC;

typedef struct
{
    U08 bLength;    //9+n or 10+n(UVC 1.1)
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bUnitID;
    U08 bSourceID;
    U16 wMaxMultiplier;
    U08 bControlSize;   //=n
    U08 bmControls[8];
    //the last sencond byte is iProcessing, and
    //the last byte is bmVideoStandards, only present in UVC 1.1 spec.
} ST_PROC_UNIT_DESC;

typedef struct
{
    U08 bLength;    //6+p
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bColorPrimaries;
    U08 bTransferCharacteristics;  //=p
    U08 bMatrixCoefficients;
    //the last byte is iSelector
} ST_SCOLOR_FORMAT_DESC;

typedef struct
{
    U08 bLength;    //24+p+n
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bUnitID;
    U08 guidExtensionCode[16];
    U08 bNumControls;
    U08 bNrInPins;  //=p
    U08 baSourceID[8];
    //bControlSize  //=n
    //bmControls[]
    //iExtension
} ST_EXT_UNIT_DESC;

typedef struct
{
    U08 bLength;    //5
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U16 wMaxTransferSize;
} ST_CS_EP_DESC;

typedef struct
{
    U08 bLength;    //13+(p*n)
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bNumFormats;
    U16 wTotalLength;
    U08 bEndpointAddress;
    U08 bmInfo;
    U08 bTerminalLink;
    U08 bStillCaptureMethod;
    U08 bTriggerSupport;
    U08 bTriggerUsage;
    U08 bControlSize;   //=1
    U08 bmaControls[2];
} ST_VS_INF_IN_HEADER_DESC;

typedef struct
{
    U08 bLength;    //8 or 9+(p*n)(UVC 1.1)
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bNumFormats;
    U16 wTotalLength;
    U08 bEndpointAddress;
    U08 bTerminalLink;
    //U08 bControlSize; //only present in UVC 1.1 spec.
    //U08 *bmaControls; //only present in UVC 1.1 spec.
} ST_VS_INF_OUT_HEADER_DESC;

typedef struct
{
    U08 bLength;    //11
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bFormatIndex;
    U08 bNumFrameDescriptors;
    U08 bmFlags;
    U08 bDefaultFrameIndex;
    U08 bAspectRatioX;
    U08 bAspectRatioY;
    U08 bmInterlaceFlags;
    U08 bCopyProtect;
} ST_MJPEG_FORMAT_DESC;

typedef struct
{
    U08 bLength;    //27
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bFormatIndex;
    U08 bNumFrameDescriptors;
    U08 guidFormat[16];
    U08 bBitsPerPixel;
    U08 bDefaultFrameIndex;
    U08 bAspectRatioX;
    U08 bAspectRatioY;
    U08 bmInterlaceFlags;
    U08 bCopyProtect;
} ST_UNCOMPRESSED_FORMAT_DESC;

typedef struct
{
    U08 bLength;    //34, if bFrameIntervalType is 0
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bFrameIndex;
    U08 bmCapabilities;
    U16 wWidth;
    U16 wHeight;
    U32 dwMinBitRate;
    U32 dwMaxBitRate;
    U32 dwMaxVideoFrameBufferSize;
    U32 dwDefaultFrameInterval;
    U08 bFrameIntervalType;
    U32 dwMinFrameInterval;
    U32 dwMaxFrameInterval;
    U32 dwFrameIntervalStep;
} ST_MJPEG_FRAME_DESC;

typedef struct
{
    U08 bLength;    //34, if bFrameIntervalType is 0
    U08 bDescriptorType;
    U08 bDescriptorSubtype;
    U08 bFrameIndex;
    U08 bmCapabilities;
    U16 wWidth;
    U16 wHeight;
    U32 dwMinBitRate;
    U32 dwMaxBitRate;
    U32 dwMaxVideoFrameBufferSize;
    U32 dwDefaultFrameInterval;
    U08 bFrameIntervalType;
    U32 dwMinFrameInterval;
    U32 dwMaxFrameInterval;
    U32 dwFrameIntervalStep;
} ST_UNCOMPRESSED_FRAME_DESC;

typedef struct
{
    U08 bHeaderLength;
    U08 bmHeaderInfo;
    U16 wSOFConter;
    U32 dwPresentationTime;
    U32 dwClockFrequency;
    U16 wMaxPacketSize;
    U16 wReserve1;
    U32 dwClockTimeInterval;
} ST_VS_INFO;

typedef struct
{
    U08 bLength;    //8 + n
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U16 bcdADC;
    U16 wTotalLength;
    U08 bInCollection;
    U08 baInterfaceNr[1];   //n = 1
} ST_AC_INF_HEADER_DESC;

typedef struct
{
    U08 bLength;    //12
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bTerminalID;
    U16 wTerminalType;
    U08 bAssocTerminal;
    U08 bNrChannels;
    U16 wNrChannelConfig;
    U08 iChannelNames;
    U08 iTerminal;
} ST_AUDIO_IN_TERMINAL_DESC;

typedef struct
{
    U08 bLength;    //9
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bTerminalID;
    U16 wTerminalType;
    U08 bAssocerminal;
    U08 bSourceID;
    U08 iTerminal;
} ST_AUDIO_OUT_TERMINAL_DESC;

typedef struct
{
    U08 bLength;    //9
    U08 bDescriptorType;
    U08 bDescripterSubType;
    U08 bUnitID;
    U08 bSourceID;
    U08 bControlSize;
    U16 bmControls;
    U08 iFeature;
} ST_AUDIO_FEATURE_UNIT_DESC;

typedef struct
{
    U08 bLength;    //7
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bTerminalLink;
    U08 bDelay;
    U16 wFormatTag;
} ST_AS_INF_GENERAL_DESC;

typedef struct
{
    U08 bLength;    //8 + 3*n, n is number of supported sample rate
    U08 bDescriptorType;
    U08 bDescriptorSubType;
    U08 bFormatType;
    U08 bNrChannels;
    U08 bSubFrameSize;
    U08 bBitResolution;
    U08 bSamFreqType;
    U08 tSamFreq[1][3]; // n = 1
} ST_TYPE_I_FORMAT_DESC;


typedef struct
{
    U08 bLength;    //7
    U08 bDescriptorType;
    U08 bDescritptorSubType;
    U08 bmAttributes;
    U08 bLockDelayUnits;
    U16 wLockDelay;
} ST_CS_ISO_AUDIO_EP_DESC;

/***Device Class Codes***/
/* move to usbotg_host_webcam.h
typedef enum    //Interface Class Code
{
    CC_AUDIO = 0x01,
    CC_VIDEO = 0x0e,
} INF_CLASS_CODE;
*/
typedef enum    //Interface SubClass Code
{
    SC_UNDEFINED = 0x00,
    SC_VIDEOCONTROL,
    SC_VIDEOSTREAMING,
    SC_VIDEO_INTERFACE_COLLECTION,

    SC_AUDIOCONTROL = 0x01,
    SC_AUDIOSTREAMING,
    SC_MIDISTREAMING,
} INF_SUBCLASS_CODE;

typedef enum    //Interface Protocal Code
{
    PC_PROTOCAL_UNDEFINED = 0x00
} INF_PROTOCAL_CODE;

typedef enum    //Class-Specific VC Interface Descriptor Subtypes
{
    VC_DESCRIPTOR_UNDEFINED = 0x00,
    VC_HEADER,
    VC_INPUT_TERMINAL,
    VC_OUTPUT_TERMINAL,
    VC_SELECTOR_UNIT,
    VC_PROCESSING_UNIT,
    VC_EXTENSION_UNIT,
} CS_VC_INF_DESC_SUBTYPE;

typedef enum    //Class-Specific VS Interface Descriptor Subtypes
{
    VS_UNDEFINED = 0x00,
    VS_INPUT_HEADER = 0x01,
    VS_OUPUT_HEADER = 0x02,
    VS_STILL_IMAGE_FRAME = 0x03,
    VS_FORMAT_UNCOMPRESSED = 0x04,
    VS_FRAME_UNCOMPRESSED = 0x05,
    VS_FORMAT_MJPEG = 0x06,
    VS_FRAME_MJPEG = 0x07,
} CS_VS_INF_DESC_SUBTYPE;

typedef enum    //Class-Specific AC Interface Descriptor Subtypes
{
    AC_DESCRIPTOR_UNDEFINED = 0x00,
    AC_HEADER,
    AC_INPUT_TERMINAL,
    AC_OUTPUT_TERMINAL,
    AC_MIXER_UNIT,
    AC_SELECTOR_UNIT,
    AC_FEATURE_UNIT,
} CS_AC_INF_DESC_SUBTYPE;

typedef enum    //Class-Specific AS Interface Descriptor Subtypes
{
    AS_UNDEFINED = 0x00,
    AS_GENERAL = 0x01,
    AS_FORMAT_TYPE = 0x02,
    AS_FORMAT_SPECIFIC = 0x03,
} CS_AS_INF_DESC_SUBTYPE;

typedef enum    //Class-Specific Endpoint Descriptor Subtypes
{
    EP_UNDEFINED =0x00,
    EP_GENERAL = 0x01,
    EP_ENDPOINT = 0x02,
    EP_INTERRUPT = 0x03,
} CS_END_DESC_SUBTYPE;

typedef enum    //Class-Specific Request Codes
{
    RC_UNDEFINED = 0x00,
    SET_CUR = 0x01,
    GET_CUR = 0x81,
    SET_MIN = 0x02,
    GET_MIN = 0x82,
    SET_MAX = 0x03,
    GET_MAX = 0x83,
    SET_RES = 0x04,
    GET_RES = 0x84,
    GET_LEN = 0x85,
    GET_INFO = 0x86,
    GET_DEF = 0x87,
    SET_MEM = 0x05,
    GET_MEM = 0x85,
    GET_STAT = 0xff,
} CS_REQUEST_CODE;

typedef enum    //VideoControl Interface Control Selectors
{
    VC_CONTROL_UNDEFINED = 0x00,
    VC_VIDEO_POWER_MODE_CONTROL,
    VC_REQUEST_ERROR_CODE_CONTROL,
} VC_INF_CONTROL_SEL;

typedef enum    //VideoStreaming Interface Control Selectors
{
    VS_CONTROL_UNDEFINED = 0x00,
    VS_PROBE_CONTROL,
    VS_COMMIT_CONTROL,
    VS_STILL_PROBE_CONTROL,
    VS_STILL_COMMIT_CONTROL,
    VS_STILL_IMAGE_TRIGGER_CONTROL,
    VS_STREAM_ERROR_CODE_CONTROL,
    VS_GENERATE_KEY_FRAME_CONTROL,
    VS_UPDATE_FRAME_SEGMENT_CONTROL,
    VS_SYNCH_DELAY_CONTROL,
} VS_INF_CONTROL_SEL;

typedef enum
{
    FU_CONTROL_UNDEFINED = 0,
    MUTE_CONTROL,
    VOLUME_CONTROL,
    BASS_CONTROL,
    MID_CONTROL,
    TREBLE_CONTROL,
    GRAPHIC_EQUALIZER_CONTROL,
    AUTOMATIC_GAIN_CONTROL,
    DELAY_CONTROL,
    BASS_BOOST_CONTROL,
    LOUDNESS_CONTROL,
} AUDIO_FEATURE_CS;

typedef enum
{
    PU_CONTROL_UNDEFINED = 0,
    BACKLIGHT_COMPENSATION_CONTROL,
    BRIGHTNESS_CONTROL,
    CONTRAST_CONTROL,
    GAIN_CONTROL,
    POWER_LINE_FREQUENCY_CONTROL,
    HIU_CONTROL,
    SATURATION_CONTROL,
    SHARPNESS_CONTROL,
    GAMMA_CONTROL,
    WHITE_BALENCE_TEMPERATURE_CONTROL,
    WHITE_BALENCE_TEMPERATURE_AUTO_CONTROL,
    WHITE_BALENCE_COMPONENT_CONTROL,
    WHITE_BALENCE_COMPONENT_AUTO_CONTROL,
    DIGITAL_MULTIPLIER_CONTROL,
    DIGITAL_MULTIPLIER_LIMIT_CONTROL,
    HIU_AUTO_CONTROL,
    ANALOG_VIDEO_STANDARD_CONTROL,
    ANALOG_LOCK_STATUS_CONTROL,
} PROCESSING_FEATURE_CS;

typedef enum
{
    TE_CONTROL_UNDEFINED = 0x00,
    COPY_PROTECT_CONTROL,
} TERMINAL_CS;

typedef enum
{
    EP_CONTROL_UNDEFINED = 0x00,
    SAMPLING_FREQ_CONTROL,
    PITCH_CONTROL,
} EP_CS;

/***Configuration***/
/***************************************************************************/

#define API_VIDEO_PREVIEW_START                  0x00000001
#define API_VIDEO_PREVIEW_STOP                   0x00000002
#define API_VIDEO_RECORD_START                   0x00000003
#define API_VIDEO_RECORD_STOP                    0x00000004
#define API_VIDEO_CLIP_OPEN                      0x00000005
#define API_VIDEO_CLIP_CLOSE                     0x00000006
#define API_VIDEO_PLAY_START                     0x00000007
#define API_VIDEO_PLAY_STOP                      0x00000008
#define API_VIDEO_PROGRESS_SET                   0x00000009
#define API_VIDEO_PROGRESS_GET                   0x0000000a
#define API_VIDEO_FORWARD_SET                    0x0000000b
#define API_VIDEO_IMAGE_WINDOW_REDRAW            0x0000000c
#define API_VIDEO_DIGITAL_ZOOM_SET               0x0000000d
#define API_VIDEO_RECORD_PAUSE                   0x0000000e
#define API_VIDEO_RECORD_RESUME                  0x0000000f
#define API_VIDEO_CON_PREVIEW_START              0x00000010
#define API_VIDEO_CON_PREVIEW_STOP               0x00000011
#define API_VIDEO_CON_TRANSMISSION_START         0x00000012
#define API_VIDEO_CON_TRANSMISSION_STOP          0x00000013
#define API_VIDEO_CON_VISUAL_CODEC_INIT          0x00000014
#define API_VIDEO_CON_VISUAL_ENCODE_TYPE_SET     0x00000015
#define API_VIDEO_CON_VISUAL_DECODE_TYPE_SET     0x00000016
#define API_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE 0x00000017
#define API_VIDEO_CON_VISUAL_CODEC_VOL_SET       0x00000018
#define API_VIDEO_CON_VISUAL_STREAM_FILL         0x00000019
#define API_VIDEO_CON_VISUAL_BUFFER_MEASURE      0x0000001a
#define API_VIDEO_CON_VISUAL_STREAM_DECODE       0x0000001b
#define API_VIDEO_CON_SOUND_ENCODE_TYPE_SET      0x0000001c
#define API_VIDEO_PROGRESS_FRAME_BY_FRAME        0x0000001d
#define API_VIDEO_QUICK_CLIP_OPEN                0x00000020
#define API_VIDEO_VISUAL_FRAME_SET               0x00000021
#define API_VIDEO_VISUAL_FRAME_APPLY             0x00000022
#define API_VIDEO_TIMESTAMP_FONT_SET             0x00000023
#define API_VIDEO_TIMESTAMP_FONT_CLEAN           0x00000024
#define API_VIDEO_TIMESTAMP_REGION_SET           0x00000025
#define API_VIDEO_TIMESTAMP_ENABLE               0x00000026

#define AUDIO_SAMPLERATE_8K     0 // AUDIO_RECORD_8K_SR
#define AUDIO_SAMPLERATE_16K    1 // AUDIO_RECORD_16K_SR
#define AUDIO_SAMPLERATE_24K    2 // AUDIO_RECORD_24K_SR

#define ERR_VIDEO_BASE                          0x80040000
#define ERR_VIDEO_INVALID_PARAMETER             (9 + ERR_VIDEO_BASE)



#define STILL_IMG_SUPPORT       0
#define MICROPHONE_SUPPORT      ENABLE

#define MJPEG_FORMAT_SUPPORT    1
#define YUV_FORMAT_SUPPORT      1

#define VC_INF_PIPE_COUNT       0
#define VS_INF_PIPE_COUNT       1
#if MICROPHONE_SUPPORT == ENABLE
    #define AC_INF_PIPE_COUNT       0
    #define AS_INF_PIPE_COUNT       1
#else
    #define AC_INF_PIPE_COUNT       0
    #define AS_INF_PIPE_COUNT       0
#endif

#define WEBCAM_PIPE_COUNT       (VC_INF_PIPE_COUNT + VS_INF_PIPE_COUNT + AC_INF_PIPE_COUNT + AS_INF_PIPE_COUNT)

#define VIDEO_INF_COUNT         2
#if MICROPHONE_SUPPORT == ENABLE
    #define AUDIO_INF_COUNT         2
#else
    #define AUDIO_INF_COUNT         0
#endif
#define WEBCAM_INF_COUNT        (VIDEO_INF_COUNT + AUDIO_INF_COUNT)

#define FRAME_INTERVAL_30FPS    0x51615
#define FRAME_INTERVAL_15FPS    0xa2c2a
#define FRAME_INTERVAL_10FPS    0xf4240

#define SYS_CLOCK_FREQ      6000000 //6MHz

#define VC_INF_ID           0
#define VS_INF_ID           1
#define AC_INF_ID           2
#define AS_INF_ID           3

#define CAMERA_IT_ID            1
#define PROCESSING_UT_ID        2
#define VIDEO_STREAMING_OT_ID   3

#define MICROPHONE_IT_ID        1
#define FEATURE_UT_ID           2
#define AUDIO_STREAMING_OT_ID   3

#define MIN_MIC_VOLUME  0x0
#define MAX_MIC_VOLUME  0x1680

#define MIN_BRIGHTNESS_VOLUME  0x1
#define MAX_BRIGHTNESS_VOLUME  0xa

#define MIN_CONTRAST_VOLUME  0x0
#define MAX_CONTRAST_VOLUME  0xa

/***************************************************************************/
#define MODE_PREVIEW 0
#define MODE_CAPTURE 1
#define MODE_RECORD  2
#define MODE_PCCAM   3

//Descriptor length
#define VC_INF_HEADER_DESC_LEN      0xd
#define VC_CAM_TERMINAL_DESC_LEN    0x12
#define VC_PROC_UNIT_DESC_LEN       0xb
#define VC_OUT_TERMINAL_DESC_LEN    0x9
#define VS_IN_HEADER_DESC_LEN       0xd //not include bmaControls field
#define VS_MJPEG_FORMAT_DESC_LEN    0xb
#define VS_MJPEG_FRAME_DESC_LEN     0x26

#define VS_UNCOMPRESSED_FORMAT_DESC_LEN    0x1b
#define VS_UNCOMPRESSED_FRAME_DESC_LEN     0x26
#define VS_UNCOMPRESSED_COLOR_FORMAT_DESC_LEN     0x06

#define AC_INF_HEADER_DESC_LEN      0x9
#define AC_IN_TERMINAL_DESC_LEN     0xc
#define AC_FEATURE_UNIT_DESC_LEN    0x9
#define AC_OUT_TERMINAL_DESC_LEN    0x9
#define AS_INF_GEN_DESC_LEN         0x7
#define AS_TYPE_I_FORMAT_DESC_LEN   0xb
#define AS_CS_EP_DESC_LEN           0x7

//micro utility
#define DESC_FILL_BYTE(addr, value)     (*addr++ = (U08)value);
#define DESC_FILL_WORD(addr, value)     (*addr++ = (U08)(value >> 8));  \
                                        (*addr++ = (U08)value);
#define DESC_FILL_DWORD(addr, value)    (*addr++ = (U08)(value >> 24)); \
                                        (*addr++ = (U08)(value >> 16)); \
                                        (*addr++ = (U08)(value >> 8));  \
                                        (*addr++ = (U08)value);



#define FRAME_FIELDS 10
static U32 mjpegFrameType[][FRAME_FIELDS] =
{
    //{
        //wWidth
        //wHeight
        //dwMinBitRate
        //dwMaxBitRate
        //dwMaxVideoFrameBufferSize
        //dwDefaultFrameInterval
        //bFrameIntervalType
        //dwMinFrameInterval
        //dwMaxFrameInterval
        //dwFrameIntervalStep
    //}
    {
        0x500,                  //1280
        0x2d0,                  //720
        0x1770000*3,               //12288000, 12 multiple compression, 30fps
        0x8ca0000*3,              //36864000, 4 multiple compression, 30fps
        0x96000*3,                //153600, 4 multiple compression
        FRAME_INTERVAL_30FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
    {
        0x280,                  //640
        0x1e0,                  //480
        0x1770000,               //12288000, 12 multiple compression, 30fps
        0x8ca0000,              //36864000, 4 multiple compression, 30fps
        0x96000,                //153600, 4 multiple compression
        FRAME_INTERVAL_30FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
    {
        0x140,                  //320
        0xf0,                   //240
        0x5dc000,               //3072000, 12 multiple compression, 30fps
        0x2328000,               //9216000, 4 multiple compression, 30fps
        0x25800,                 //38400, 4 multiple compression
        FRAME_INTERVAL_30FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
    {
        0xb0,                   //176
        0x90,                   //144
        0x1ef00,                //
        0xb9a00,                //
        0xc600,                 //
        FRAME_INTERVAL_30FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
};

static U32 uncompressedFrameType[][FRAME_FIELDS] =
{
    //{
        //wWidth
        //wHeight
        //dwMinBitRate
        //dwMaxBitRate
        //dwMaxVideoFrameBufferSize
        //dwDefaultFrameInterval
        //bFrameIntervalType
        //dwMinFrameInterval
        //dwMaxFrameInterval
        //dwFrameIntervalStep
    //}
    {
        640,					//640
        480,					//480
        147456000,              //147456000 = 640*480 * 2byte * 30fps * 8bit
        147456000,              //147456000 = 640*480 * 2byte * 30fps * 8bit
        614400,					//614400 = 640*480 * 2byte
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
    {
        0x140,                  //320
        0xf0,                   //240
        0x5dc000,               //3072000, 12 multiple compression, 30fps
        0x2328000,               //9216000, 4 multiple compression, 30fps
        0x25800,                 //38400, 4 multiple compression
        FRAME_INTERVAL_30FPS/*FRAME_INTERVAL_15FPS*/,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS/*FRAME_INTERVAL_10FPS*/,   //1000000,10fps
    },
    {
        0xb0,                   //176
        0x90,                   //144
        0x1ef000,                //
        0x5cd000,                //
        0xc600,                 //
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        3,                      //Discrete frame interval
        FRAME_INTERVAL_30FPS,   //333333, 30fps
        FRAME_INTERVAL_15FPS,   //666666, 15fps
        FRAME_INTERVAL_10FPS,   //1000000,10fps
    },
};

static WHICH_OTG webCamWhichOtg = USBOTG_NONE;

static U08 compressionFormatGUID[16] =
    {0x59,0x55,0x59,0x32,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71};  //YUY2
//    {0x4e,0x56,0x31,0x32,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71};  //NV12

#define VS_INF_ALT_FIELDS 3
static U32 vsInfAlt[][VS_INF_ALT_FIELDS] =
{
/////NumPerFrame, MaxPacketSize, Interval
    {1,           512,           4   },
    //{3,           1020,          1   },
    //{2,           512,           1   },
    //{3,           512,           1   },
    //{1,           1024,          1   },
    {2,           1024,          1   },
    //{3,           1024,          1   },
};
#define AS_EP_PKSIZE    80



typedef struct
{
    U08 u08PipeType;
    U08 u08PipeNum;
} ST_PIPE_INF;


typedef struct
{
    U16 bmHint;
    U08 bFormatIndex;
    U08 bFrameIndex;
    U32 dwFrameInterval;
    U16 wKeyFrameRate;
    U16 wPFrameRate;
    U16 wCompQuality;
    U16 wCompWindowSize;
    U16 wDelay;
    U32 dwMaxVideoFrameSize;
    U32 dwMaxPayloadTransferSize;
    //U32 dwClockFrequency; //only present in UVC 1.1 spec.
    //U08 bmFramingInfo;    //only present in UVC 1.1 spec.
    //U08 bPreferedVersion; //only present in UVC 1.1 spec.
    //U08 bMinVersion;      //only present in UVC 1.1 spec.
    //U08 bMaxVersion;      //only present in UVC 1.1 spec.
} ST_VIDEO_PROBE_COMMIT;

static BOOL supportMJPEGFormat;
static BOOL supportYUVFormat;
static U32 supportFormatNumber;
static U32 (*firstFormat)[FRAME_FIELDS];
static U32 (*secondFormat)[FRAME_FIELDS];
static U32 firstFormatFrameNum;
static U32 secondFormatFrameNum;

static ST_VIDEO_PROBE_COMMIT stVPC_Cur;
static ST_VIDEO_PROBE_COMMIT stVPC_Min;
static ST_VIDEO_PROBE_COMMIT stVPC_Max;
static ST_VIDEO_PROBE_COMMIT stVPC_Def;
static ST_VIDEO_PROBE_COMMIT stVPC_Res;

static ST_VS_INFO vsInfo;

static BOOL boolVsStart = FALSE;
static BOOL boolAsStart = FALSE;
static BOOL boolMute = FALSE;

static U08 webCamVsTaskId;
static U08 webCamAsTaskId;
static U08 webCamVsMsgId;
static U08 webCamAsMsgId;
//static BOOL FuncStart;

static U32 vsTransferBufSize;
static U08 *vsTransferBuf;

static U16 currVolumeValue;
static U16 currMicVolume;

static U16 currBrightnessVolume;
static U16 currContrastVolume;
static U16 currHiuVolume;
static U16 currSaturationVolume;
static U16 currSharpnessVolume;
static U16 currGammaVolume;

static void vsStart(void *para);
static void vsPause();
static S32 vsDataGet(U08* encBuf, U32 encSize, U32 enable);
static void asStart(void *para);
static void asPause();
static S32 asDataGet(U08* encBuf, U32 encSize, U32 enable);

#define VIDEO_CON_VISUAL_NONE_TYPE  0
#define VIDEO_CON_VISUAL_H263_TYPE  34
#define VIDEO_CON_VISUAL_MP4_TYPE   100
#define VIDEO_CON_VISUAL_JPG_TYPE   101

#define VIDEO_CON_SOUND_NONO_TYPE   0
#define VIDEO_CON_SOUND_PCMA_TYPE   8
#define VIDEO_CON_SOUND_G729_TYPE   18
#define VIDEO_CON_SOUND_AMR_TYPE    110
#define VIDEO_CON_SOUND_AAC_TYPE    111

#define VIDEO_CON_GRAPHIC_WINDOW 0
#define VIDEO_CON_IMAGE_WINDOW   1

#define VIDEO_QUALITY_FINE      0
#define VIDEO_QUALITY_NORMAL    1
#define VIDEO_QUALITY_ECONOMIC  2

#define VIDEO_CON_I_FRAME_TYPE 0
#define VIDEO_CON_P_FRAME_TYPE 1

#define VIDEO_CON_0_COUNT_SKIP_FRAME    0
#define VIDEO_CON_2_COUNT_SKIP_FRAME    2
#define VIDEO_CON_3_COUNT_SKIP_FRAME    3
#define VIDEO_CON_4_COUNT_SKIP_FRAME    4
#define VIDEO_CON_5_COUNT_SKIP_FRAME    5
#define VIDEO_CON_6_COUNT_SKIP_FRAME    6
#define VIDEO_CON_7_COUNT_SKIP_FRAME    7
#define VIDEO_CON_8_COUNT_SKIP_FRAME    8
#define VIDEO_CON_9_COUNT_SKIP_FRAME    9
#define VIDEO_CON_10_COUNT_SKIP_FRAME   10
#define VIDEO_CON_11_COUNT_SKIP_FRAME   11
#define VIDEO_CON_12_COUNT_SKIP_FRAME   12
#define VIDEO_CON_13_COUNT_SKIP_FRAME   13
#define VIDEO_CON_14_COUNT_SKIP_FRAME   14
#define VIDEO_CON_15_COUNT_SKIP_FRAME   15
#define VIDEO_CON_16_COUNT_SKIP_FRAME   16
#define VIDEO_CON_17_COUNT_SKIP_FRAME   17
#define VIDEO_CON_18_COUNT_SKIP_FRAME   18
#define VIDEO_CON_19_COUNT_SKIP_FRAME   19
#define VIDEO_CON_20_COUNT_SKIP_FRAME   20

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//typedef unsigned long U32;
//typedef unsigned int  U16;
//typedef unsigned char U08;
typedef U32 FILE_HANDLE;

#define VIDEO_RESOLUTION_352x288 0
#define VIDEO_RESOLUTION_320x240 1
#define VIDEO_RESOLUTION_176x144 2
#define VIDEO_RESOLUTION_640x480 3
#define VIDEO_RESOLUTION_736x480 4
#define VIDEO_RESOLUTION_736x576 5
#define VIDEO_RESOLUTION_1280x720 6


#define BYTE_SWAP_OF_U16(x)	(U16)(((U16)(x) >> 8) | ((U16)(x) << 8))
#define BYTE_SWAP_OF_U32(x) (((U32)(x) << 24) | (((U32)(x) & 0x0000ff00) << 8) | (((U32)(x) & 0x00ff0000) >> 8) | ((U32)(x) >> 24))

#define MIN_VOLUMN_VALUE     0
#define MAX_VOLUMN_VALUE     100


static ST_PIPE_INF webCamPipes[WEBCAM_PIPE_COUNT];

#define VS_ISO_IN_PIPE  webCamPipes[0]
#define AS_ISO_IN_PIPE  webCamPipes[1]
//#define VC_INT_IN_PIPE  webCamPipes[2]

#define USBD_FUNC_INF_DEPEND    0x8000
#define USBD_MODE_PCCAM     (0x02 | USBD_FUNC_INF_DEPEND)

#define USBD_FUNC_UID_PCCAM     (USBD_MODE_PCCAM    );


#define GET_INTFACE_STR 0x02
#define GET_INTFACE_CLASS 0x03
#define GET_INTFACE_SUBCLASS 0x04
#define GET_INTFACE_PROTOCAL 0x05

#define END_DECRP_LEN       0x07
#define INF_DECRP_LEN       0x09
#define CONFIG_DECRP_LEN    0x09
#define DEV_DECRP_LEN       0x12
#define DEVQ_DECRP_LEN      0x0A
#define INF_ASSO_DESC_LEN   0x08

#define INF_PRODUCT_STR_IDX 4

#define MIC_INPUT            0x20
#define MIN_GAIN_VALUE     0
#define MAX_GAIN_VALUE     100

typedef struct
{
    U08 u08bLength;
	U08 u08bDescriptorType;	
	U16	u16bcdUSB;
	U08	u08bDeviceClass;
	U08	u08bDeviceSubClass;
	U08	u08bDeviceProtocol;
	U08	u08bMaxPacketSize0;
	U16	u16idVendor;
	U16	u16idProduct;
	U16	u16bcdDevice;
	U08 u08iManufacturer;
	U08 u08iProduct;
	U08	u08iSerialNumber;
	U08	u08bNumConfigurations;
} ST_DEV_DESC;

#define VID 0x0851  //Macronix International
#define PID_INDEP       0x1542
#define PID_PCCAM       0x1543
#define PID_PCCAM_MIC   0x1544
#define PID_CDC_COM     0x1540

typedef struct
{
    U08 U08bLength;
    U08 U08bDescriptorType;
    U16 U16wTotalLength;
    U08 U08bNumInterfaces;
    U08 U08bConfigurationValue;
    U08 U08iConfiguration;
    U08 U08bmAttributes;
    U08 U08MaxPower;
} ST_CFG_DESC;

typedef struct
{
    U08 U08bLength;
    U08 U08bDescriptorType;
    U08 U08bInterfaceNumber;
    U08 U08bAlternateSetting;
    U08 U08bNumEndpoints;
    U08 U08bInterfaceClass;
    U08 U08bInterfaceSubClass;
    U08 U08bInterfaceProtocol;
    U08 U08iInterface;
} ST_INF_DESC;

typedef struct
{
    U08 U08bLength;
    U08 U08bDescriptorType;
    U08 U08bEndpointAddress;
    U08 U08bmAttributes;
    U16 U16wMaxPacketSize;
    U08 U08bInterval;
} ST_EP_DESC;

typedef struct
{
    U08 u08bLength;
    U08 u08bDescriptorType;
    U08 u08bFirstInterface;
    U08 u08bInterfaceCount;
    U08 u08bFunctionClass;
    U08 u08bFunctionSubClass;
    U08 u08bFunctionProtocal;
    U08 u08iFunction;
} ST_INF_ASSO_DESC;

typedef enum
{
    DEVICE = 1,
    CONFIGURATION,
    STRING,
    INTERFACE,
    ENDPOINT,
    DEVICE_QUALIFIER,
    OTHER_SPEED_CONFIGURATION,
    INTERFACE_POWER,
    OTG,
    DEBUG,
    INTERFACE_ASSOCIATION
} DECRP_TYPE;


//endpoint type
#define EP_TYPE_CONTROL         0x00
#define EP_TYPE_ISO_IN          0x81
#define EP_TYPE_BULK_IN         0x82
#define EP_TYPE_INTERRUPT_IN    0x83
#define EP_TYPE_ISO_OUT         0x01
#define EP_TYPE_BULK_OUT        0x02
#define EP_TYPE_INTERRUPT_OUT   0x03

//event
#define EVENT_BUS_RESET             0x1
#define EVENT_DATA_COMEIN           0x2
#define EVENT_DATA_COMEIN_NAK       0x4
#define EVENT_DATA_SEND_DONE        0x8
#define EVENT_DATA_RECV_DONE        0x10
#define EVENT_SOFT_DISCON_CON       0x20
#define EVENT_SUSPEND               0x40
#define EVENT_DISCONNECT            0x80

typedef enum    //Class-Specific Descriptor Types
{
    CS_UNDEFINED = 0x20,
    CS_DEVICE,
    CS_CONFIGURATION,
    CS_STRING,
    CS_INTERFACE,
    CS_ENDPOINT,
} CS_DECRP_TYPE;

typedef struct
{
    U32 Uid;
    void (*FuncInit)();                 //1. create the task    2. initiliz resources
    void (*FuncDeInit)();               //1. destory object
    BOOL (*FuncStart)();                //1. startup the task   2. start the pipes
    void (*FuncStop)();                 //1. disarm the task    2. stop the pipes
    
    U08 (*FuncPipeCntGet)();
    U08 (*FuncInfCntGet)();
    U08 (*FuncPipeTypeGet)(U08);
    void (*FuncPipeNumSet)(U08, U08);
    U32 (*FuncInfoGet)(U08);

    BOOL (*FuncClassReq)(U08* psetupPacket, U32* len,U08** addr, void (**CBfn) (void *, U08*), void **CBpara);
    BOOL (*FuncVendorReq)(U08* psetupPacket, U32* len,U08** addr, void (**CBfn) (void *, U08*), void **CBpara);

    BOOL (*FuncDevDescGet)(U32 * len, U08 ** addr);
    BOOL (*FuncConfDescGet)(U08 index, U32 * len, U08 ** addr, BOOL otherspeed);
    BOOL (*FuncInfSet)(U16 infNum, U16 altNum, void (**CBfn) (void *, U08*), void **CBpara);
} ST_USBD_FUNC;


typedef enum
{
    IDU_GRAPHIC_WINDOW  = 0,
    IDU_IMAGE_WINDOW    = 1,
}ENUM_IDU_WINDOW_TYPE;

#define AUDIO_SAMPLERATE_8K     0 // AUDIO_RECORD_8K_SR
#define AUDIO_SAMPLERATE_16K    1 // AUDIO_RECORD_16K_SR
#define AUDIO_SAMPLERATE_24K    2 // AUDIO_RECORD_24K_SR





S32 mpx_VideoConTransmissionStart(BOOL soundEnable)
{
    UNION_VIDEO_API_MESSAGE message;

    message.VideoConTransmissionStartMsg.u32ActionId = API_VIDEO_CON_TRANSMISSION_START;
    message.VideoConTransmissionStartMsg.boolSoundEnable = soundEnable;

    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConTransmissionStartMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    return message.VideoConTransmissionStartRetMsg.s32Response;
}




S32 mpx_VideoConTransmissionStop()
{
    UNION_VIDEO_API_MESSAGE message;
    S32 status;
    BYTE mail_id;

    message.VideoConTransmissionStopMsg.u32ActionId = API_VIDEO_CON_TRANSMISSION_STOP;
    status = MailboxSend( (U08)VideoActionMsgIdGet(),
                        (U08 *)&message,
                        sizeof(message.VideoConTransmissionStopMsg),
                        &mail_id);
	if (status != OS_STATUS_OK)
	{
		MP_DEBUG("%s: MailboxSend Failed %d", __FUNCTION__, status);
		return status;
	}
    //mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConTransmissionStopMsg));


#if REMOVE



		MailboxReceive(MCARD_MAIL_ID, &bMcardMailId);
		if (MailGetBufferStart(bMcardMailId, (DWORD *)&McMail) == OS_STATUS_OK)
		{
			ST_MCARD_DEV *McDevs = Mcard_GetDevice(McMail->wMCardId);
			if (McDevs->CommandProcess == NULL)
			{
				MP_ALERT("Device Task: no command process for card%d", McMail->wMCardId);
			}
			else
			{
				#if PERFORMANCE_METER
				DWORD tm = GetSysTime()>>2;
				#endif
				McDevs->sMcardRMail = McMail;
				McDevs->CommandProcess(McDevs);
				#if PERFORMANCE_METER
				McardStatistics(McMail, tm);
				#endif
			}
			if (McDevs->swStatus || McMail->swStatus)
				mpDebugPrint("Card%d: commad %d processed failure!(%x/%x)", McMail->wMCardId, McMail->wCmd, McDevs->swStatus, McMail->swStatus);
		}
		MailRelease(bMcardMailId);




#endif






	mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    return message.VideoConTransmissionStopRetMsg.s32Response;
}



static BOOL SensorModuleStatus = FALSE;

S32 mpx_VideoConPreviewStop()
{
    UNION_VIDEO_API_MESSAGE message;

    message.VideoConPreviewStopMsg.u32ActionId = API_VIDEO_CON_PREVIEW_STOP;
    mpx_MessageSend((U08)VideoActionMsgIdGet(), (U08 *)&message, sizeof(message.VideoConPreviewStopMsg));
    mpx_MessageReceive((U08)VideoReturnMsgIdGet(), (U08 *)&message);

    SensorModulePowerOff();

    SensorModuleStatus = FALSE;

    return message.VideoConPreviewStopRetMsg.s32Response;
}



void FreeUsbdBuf(U08 * buf)
{
#if REMOVE
#ifdef NEW_USB_BUF
    U08 i;

    mpx_SemaphoreWait(UsbdIntBufSemphoreId);
    for(i = 0 ; i < MAX_UBUF; i ++)
    {
        if(UsbdIntBuf[i] == buf && buf)
        {
            bufnum--;
            UsbdIntBuf[i] = 0;            
            mpx_Free(buf);           
            //DPrintf("bufm = %x, %x", bufnum, buf);
            mpx_SemaphoreRelease(UsbdIntBufSemphoreId);
            
            IntDisable();
            if(waitUsbdBufTaskId)
            {
                U08 tskId = waitUsbdBufTaskId;
                waitUsbdBufTaskId = 0;
                mpx_TaskWakeup(tskId);
            }
            else
                IntEnable();
            
            return;
        }
    }
    mpx_SemaphoreRelease(UsbdIntBufSemphoreId);
#else
    U08 i;
    for(i = 0 ; i < MAX_UBUF; i ++)
    {
        if(UsbdIntBuf[i] == buf && buf)
        {
            mpx_Free(buf);
            UsbdIntBuf[i] = 0;
            mpx_SemaphoreRelease(UsbdIntBufSemphoreId);            
            bufnum--;
            //DPrintf("bufm = %x, %x", bufnum, buf);
            return;
        }
        
    }
    
    //DPrintf("no buf found");
    
#endif
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////



static void nothing(const U08 * format, ...)
{

}



void UsbWebCamInit(void)
{
mpDebugPrint("%s", __FUNCTION__);
    VS_ISO_IN_PIPE.u08PipeType = EP_TYPE_ISO_IN;
    VS_ISO_IN_PIPE.u08PipeNum = 1; // video ep1
#if MICROPHONE_SUPPORT == ENABLE
    AS_ISO_IN_PIPE.u08PipeType = EP_TYPE_ISO_IN;
    AS_ISO_IN_PIPE.u08PipeNum = 3; // audio ep3
#endif

#if MJPEG_FORMAT_SUPPORT == 1
    firstFormat = mjpegFrameType;
#else
    firstFormat = uncompressedFrameType;
#endif

    stVPC_Def.bmHint = 0x0;
    stVPC_Def.bFormatIndex = 0x1;
    stVPC_Def.bFrameIndex = 0x1;
    stVPC_Def.dwFrameInterval = firstFormat[stVPC_Def.bFrameIndex-1][5];
    stVPC_Def.dwMaxVideoFrameSize = firstFormat[stVPC_Def.bFrameIndex-1][4];
    stVPC_Def.wKeyFrameRate = 0x0;
    stVPC_Def.wPFrameRate = 0x0;
    stVPC_Def.wCompQuality = 0x0;
    stVPC_Def.wCompWindowSize = 0x0;
    stVPC_Def.wDelay = 0x0;
    stVPC_Def.dwMaxPayloadTransferSize = vsInfAlt[sizeof(vsInfAlt)/(VS_INF_ALT_FIELDS<<2) - 1][1];//VIDEO_MAX_PAYLOADE_SIZE;

    memcpy(&stVPC_Cur, &stVPC_Def, sizeof(ST_VIDEO_PROBE_COMMIT));

    memcpy(&stVPC_Min, &stVPC_Def, sizeof(ST_VIDEO_PROBE_COMMIT));

    memcpy(&stVPC_Max, &stVPC_Def, sizeof(ST_VIDEO_PROBE_COMMIT));
    stVPC_Max.bFormatIndex = supportFormatNumber;
    stVPC_Max.bFrameIndex = firstFormatFrameNum;
    stVPC_Max.dwMaxVideoFrameSize =
        firstFormat[stVPC_Max.bFrameIndex-1][0] * firstFormat[stVPC_Max.bFrameIndex-1][1];

    memset(&stVPC_Res, 0, sizeof(ST_VIDEO_PROBE_COMMIT));
    stVPC_Res.bFormatIndex = 0x1;
    stVPC_Res.bFrameIndex = 0x1;

    currVolumeValue = (MAX_VOLUMN_VALUE - MIN_VOLUMN_VALUE)/2;
    currMicVolume = (MAX_MIC_VOLUME - MIN_MIC_VOLUME)/2;

	SsSysCfgBrightnessSet(5);
	SsSysCfgContrastSet(5);
    currBrightnessVolume = (U16)SsSysCfgBrightnessGet();
    currContrastVolume = (U16)SsSysCfgContrastGet();
    IpuSetBrightnessLevelInf(currBrightnessVolume);
    IpuSetContrastLevelInf(currContrastVolume);
}



static BOOL cameraControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
    DpString("Not support camera control, return stall");
    UsbdPipeStallSet(0);
    return FALSE;
}



static void audioMuteSet(void *para, U08 *data)
{
    boolMute = data[0]&0x1;
    MP_DEBUG("Mute: %2x", *data);
}



static void audioVolumeSet(void *para, U08 *data)
{
    currMicVolume = UtilByteSwap16((U16 *)data);

    currVolumeValue = (currMicVolume * (MAX_VOLUMN_VALUE - MIN_VOLUMN_VALUE))/(MAX_MIC_VOLUME - MIN_MIC_VOLUME);
    if(currVolumeValue > MAX_VOLUMN_VALUE)
        currVolumeValue = 1;
    mpx_AudioCodecVolumnSet(MIC_INPUT, currVolumeValue);
    MP_DEBUG("Set Volume: %4x, %3d", currMicVolume, currVolumeValue);
}



static BOOL audioFeatureControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
	static U08 dataBuf[512];
    BOOL ret = TRUE;
    U32 retLen = 0;
    S16 volume;

    *addr = &dataBuf[0];//GetUsbdBuf(512);
    switch(control)
    {
        case MUTE_CONTROL:
            switch(req)
            {
                case GET_CUR:
                    **addr = boolMute;
                    retLen = 1;
                    break;
                case SET_CUR:
                    *CBfn = audioMuteSet;
                    retLen = 1;
                    break;
                default:
                    ret = FALSE;
            }
            break;
        case VOLUME_CONTROL:
            switch(req)
            {
                case GET_CUR:
                    volume = (currMicVolume >> 8) + (currMicVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:
                    volume = ((S16)MIN_MIC_VOLUME >> 8) + ((S16)MIN_MIC_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:
                    volume = ((S16)MAX_MIC_VOLUME >> 8) + ((S16)MAX_MIC_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:
                    *((U16 *)*addr) = 0x8001;
                    retLen = 2;
                    break;
                case SET_CUR:
                    *CBfn =  audioVolumeSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;
        case BASS_BOOST_CONTROL:
            switch(req)
            {
                case GET_CUR:
                    **addr = 0;
                    retLen = 1;
                    break;
                case SET_CUR:
                    *CBfn =  audioVolumeSet;
                    retLen = 1;
                    break;
                default:
                    ret = FALSE;
            }
            break;
        default:
            ret = FALSE;
    }

    *len = (*len > retLen)?retLen:*len;
	mpDebugPrint("%s ret %x", __FUNCTION__, ret);
    return ret;
}


static void processingBrightnessSet(void *para, U08 *data)
{
    currBrightnessVolume = UtilByteSwap16((U16 *)data);

    IpuSetBrightnessLevelInf(currBrightnessVolume);
    mpDebugPrint("Set Brightness Level: %3d", currBrightnessVolume);
}

static void processingContrastSet(void *para, U08 *data)
{
    currContrastVolume = UtilByteSwap16((U16 *)data);

    IpuSetContrastLevelInf(currContrastVolume);
    mpDebugPrint("Set Contrast Level: %3d", currContrastVolume);
}


static BOOL processingUnitControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
	static U08 dataBuf[512];
    BOOL ret = TRUE;
    U32 retLen = 0;
    S16 volume;

	mpDebugPrint("processingUnitControl");

    *addr = &dataBuf[0];//GetUsbdBuf(512);
    switch(control)
    {
        case BRIGHTNESS_CONTROL:
			mpDebugPrint("brightness");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currBrightnessVolume >> 8) + (currBrightnessVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_BRIGHTNESS_VOLUME >> 8) + ((S16)MIN_BRIGHTNESS_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_BRIGHTNESS_VOLUME >> 8) + ((S16)MAX_BRIGHTNESS_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingBrightnessSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

        case CONTRAST_CONTROL:
			mpDebugPrint("contrast");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currContrastVolume >> 8) + (currContrastVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_CONTRAST_VOLUME >> 8) + ((S16)MIN_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_CONTRAST_VOLUME >> 8) + ((S16)MAX_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingContrastSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

		case HIU_CONTROL:
			mpDebugPrint("HIU_CONTROL");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currHiuVolume >> 8) + (currHiuVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_CONTRAST_VOLUME >> 8) + ((S16)MIN_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_CONTRAST_VOLUME >> 8) + ((S16)MAX_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingContrastSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

		case SATURATION_CONTROL:
			mpDebugPrint("SATURATION_CONTROL");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currSaturationVolume >> 8) + (currSaturationVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_CONTRAST_VOLUME >> 8) + ((S16)MIN_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_CONTRAST_VOLUME >> 8) + ((S16)MAX_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingContrastSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

		case SHARPNESS_CONTROL:
			mpDebugPrint("SHARPNESS_CONTROL");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currSharpnessVolume >> 8) + (currSharpnessVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_CONTRAST_VOLUME >> 8) + ((S16)MIN_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_CONTRAST_VOLUME >> 8) + ((S16)MAX_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingContrastSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

		case GAMMA_CONTROL:
			mpDebugPrint("GAMMA_CONTROL");
            switch(req)
            {
                case GET_INFO:mpDebugPrint("GET_INFO");
                    volume = (0x03 << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 1;
                    break;
                case GET_CUR:mpDebugPrint("GET_CUR");
                    volume = (currGammaVolume >> 8) + (currGammaVolume << 8);
                    *((U16 *)*addr) = volume ;
                    retLen = 2;
                    break;
                case GET_MIN:mpDebugPrint("GET_MIN");
                    volume = ((S16)MIN_CONTRAST_VOLUME >> 8) + ((S16)MIN_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_MAX:mpDebugPrint("GET_MAX");
                    volume = ((S16)MAX_CONTRAST_VOLUME >> 8) + ((S16)MAX_CONTRAST_VOLUME << 8);
                    *((U16 *)*addr) = volume;
                    retLen = 2;
                    break;
                case GET_RES:mpDebugPrint("GET_RES");
                    *((U16 *)*addr) = 0x0100;
                    retLen = 2;
                    break;
                case GET_DEF:mpDebugPrint("GET_DEF");
                    *((U16 *)*addr) = (5 << 8);
                    retLen = 2;
                    break;
                case SET_CUR:mpDebugPrint("SET_CUR");
                    *CBfn =  processingContrastSet;
                    retLen = 2;
                    break;
                default:
                    ret = FALSE;
            }
            break;

        default:
			mpDebugPrint("%x", control);
            ret = FALSE;
    }

    *len = (*len > retLen)?retLen:*len;
    return ret;
}

static BOOL vcIntEpControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
    BOOL ret = FALSE;
    return ret;
}



static void vpcCpy(U08 *buf, ST_VIDEO_PROBE_COMMIT *pVPC)
{
    U32 dwValue;
    U16 wValue;

    DESC_FILL_WORD(buf, pVPC->bmHint);
    DESC_FILL_BYTE(buf, pVPC->bFormatIndex);
    DESC_FILL_BYTE(buf, pVPC->bFrameIndex);
    dwValue = BYTE_SWAP_OF_U32(pVPC->dwFrameInterval);
    DESC_FILL_DWORD(buf, dwValue);
    wValue = BYTE_SWAP_OF_U16(pVPC->wKeyFrameRate);
    DESC_FILL_WORD(buf, wValue);
    wValue = BYTE_SWAP_OF_U16(pVPC->wPFrameRate);
    DESC_FILL_WORD(buf, wValue);
    wValue = BYTE_SWAP_OF_U16(pVPC->wCompQuality);
    DESC_FILL_WORD(buf, wValue);
    wValue = BYTE_SWAP_OF_U16(pVPC->wCompWindowSize);
    DESC_FILL_WORD(buf, wValue);
    wValue = BYTE_SWAP_OF_U16(pVPC->wDelay);
    DESC_FILL_WORD(buf, wValue);
    dwValue = BYTE_SWAP_OF_U32(pVPC->dwMaxVideoFrameSize);
    DESC_FILL_DWORD(buf, dwValue);
    dwValue = BYTE_SWAP_OF_U32(pVPC->dwMaxPayloadTransferSize);
    DESC_FILL_DWORD(buf, dwValue);
}



static void vpcGet(U08 *buf, ST_VIDEO_PROBE_COMMIT *pVPC)
{
    pVPC->bmHint = buf[0] | (buf[1] << 8);
    pVPC->bFormatIndex = buf[2];
    pVPC->bFrameIndex = buf[3];
    pVPC->dwFrameInterval = buf[4] | (buf[5] << 8) | (buf[6] << 16) | (buf[7] << 24);
    pVPC->wKeyFrameRate = buf[8] | (buf[9] << 8);
    pVPC->wPFrameRate = buf[10] | (buf[11] << 8);
    pVPC->wCompQuality = buf[12] | (buf[13] << 8);
    pVPC->wCompWindowSize = buf[14] | (buf[15] << 8);
    pVPC->wDelay = buf[16] | (buf[17] << 8);
    pVPC->dwMaxVideoFrameSize = buf[18] | (buf[19] << 8) | (buf[20] << 16) | (buf[21] << 24);
    pVPC->dwMaxPayloadTransferSize = buf[22] | (buf[23] << 8) | (buf[24] << 16) | (buf[25] << 24);
}

static void vsNegotiation(void *para, U08 *data)
{
    ST_VIDEO_PROBE_COMMIT negoVpc;
    U32 frameCount;

    vpcGet(data, &negoVpc);
    mpDebugPrint("Negotiation***********************");
    mpDebugPrint("bmHint=%4x", negoVpc.bmHint);
    mpDebugPrint("bFormatIndex=%2x", negoVpc.bFormatIndex);
    mpDebugPrint("bFrameIndex=%2x", negoVpc.bFrameIndex);
    mpDebugPrint("dwFrameInterval=%x", negoVpc.dwFrameInterval);
    mpDebugPrint("wKeyFrameRate=%4x", negoVpc.wKeyFrameRate);
    mpDebugPrint("wPFrameRate=%4x", negoVpc.wPFrameRate);
    mpDebugPrint("wCompQuality=%4x", negoVpc.wCompQuality);
    mpDebugPrint("wCompWindowSize=%4x", negoVpc.wCompWindowSize);
    mpDebugPrint("wDelay=%4x", negoVpc.wDelay);
    mpDebugPrint("dwMaxVideoFrameSize=%x", negoVpc.dwMaxVideoFrameSize);
    mpDebugPrint("dwMaxPayloadTransferSize=%x", negoVpc.dwMaxPayloadTransferSize);

	SetSensorInterfaceMode(MODE_PCCAM);

	stVPC_Cur.bmHint = BYTE_SWAP_OF_U16(negoVpc.bmHint);
    stVPC_Cur.bFormatIndex = negoVpc.bFormatIndex;
    stVPC_Cur.bFrameIndex = negoVpc.bFrameIndex;
    stVPC_Cur.dwMaxPayloadTransferSize = 2048;
    if(supportMJPEGFormat)
    {
	    firstFormat = mjpegFrameType;
        if(stVPC_Cur.bFormatIndex == 1)
        {
            if(firstFormatFrameNum >= negoVpc.bFrameIndex && negoVpc.bFrameIndex)
            {
                stVPC_Cur.bFrameIndex = negoVpc.bFrameIndex;
                stVPC_Cur.dwFrameInterval = firstFormat[stVPC_Cur.bFrameIndex-1][5];
                stVPC_Cur.dwMaxVideoFrameSize = firstFormat[stVPC_Cur.bFrameIndex-1][4];
				if (stVPC_Cur.bFrameIndex==1)
					SetImageSize(SIZE_720P_1280x720);
				else if (stVPC_Cur.bFrameIndex==2)
					SetImageSize(SIZE_VGA_640x480);
				else if (stVPC_Cur.bFrameIndex==3)
					SetImageSize(SIZE_QVGA_320x240);
				else if (stVPC_Cur.bFrameIndex==4)
					SetImageSize(SIZE_QCIF_176x144);

				API_SensorUsb_DisableShareBuf();
            }

            MP_DEBUG("\nMJPEG frame%1d, Frame size: %3d x %3d", stVPC_Cur.bFrameIndex,
                firstFormat[stVPC_Cur.bFrameIndex-1][0], firstFormat[stVPC_Cur.bFrameIndex-1][1]);
        }
        else
        {
   	        secondFormat = uncompressedFrameType;
            if(secondFormatFrameNum >= negoVpc.bFrameIndex && negoVpc.bFrameIndex)
            {
                stVPC_Cur.bFrameIndex = negoVpc.bFrameIndex;
                stVPC_Cur.dwFrameInterval = secondFormat[stVPC_Cur.bFrameIndex-1][5];
                stVPC_Cur.dwMaxVideoFrameSize = secondFormat[stVPC_Cur.bFrameIndex-1][4];
				//API_Sensor_ForceToYUYV();
				API_SensorUsb_EnableShareBuf();
				if (stVPC_Cur.bFrameIndex==1)
					SetImageSize(SIZE_VGA_640x480);
				else if (stVPC_Cur.bFrameIndex==2)
					SetImageSize(SIZE_QVGA_320x240);
				else if (stVPC_Cur.bFrameIndex==3)
					SetImageSize(SIZE_QCIF_176x144);
            }

            mpDebugPrint("\nUncompressed frame%1d, Frame size: %3d x %3d", stVPC_Cur.bFrameIndex,
                secondFormat[stVPC_Cur.bFrameIndex-1][0], secondFormat[stVPC_Cur.bFrameIndex-1][1]);
        }
    }
    else
    {
    	firstFormat = uncompressedFrameType;
		if(firstFormatFrameNum >= negoVpc.bFrameIndex && negoVpc.bFrameIndex)
        {
            stVPC_Cur.bFrameIndex = negoVpc.bFrameIndex;
            stVPC_Cur.dwFrameInterval = firstFormat[stVPC_Cur.bFrameIndex-1][5];
            stVPC_Cur.dwMaxVideoFrameSize = firstFormat[stVPC_Cur.bFrameIndex-1][4];
			//API_Sensor_ForceToYUYV();
			API_SensorUsb_EnableShareBuf();
			if (stVPC_Cur.bFrameIndex==1)
				SetImageSize(SIZE_VGA_640x480);
			else if (stVPC_Cur.bFrameIndex==2)
				SetImageSize(SIZE_QVGA_320x240);
			else if (stVPC_Cur.bFrameIndex==3)
				SetImageSize(SIZE_QCIF_176x144);
        }

        MP_DEBUG("\nUncompressed frame%1d, Frame size: %3d x %3d", stVPC_Cur.bFrameIndex,
            firstFormat[stVPC_Cur.bFrameIndex-1][0], firstFormat[stVPC_Cur.bFrameIndex-1][1]);
    }

	SetSensorOverlayEnable(DISABLE);
	//SetMultiBufFlag(1);
    mpDebugPrint("***********************************");
}



static BOOL vsInfControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
	static U08 dataBuf[512];
    BOOL ret = TRUE;
    U32 retLen = 0;

    *addr = &dataBuf[0];//GetUsbdBuf(512);
    switch(control)
    {
        case VS_PROBE_CONTROL:
            switch(req)
            {
                case GET_CUR:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    vpcCpy(*addr, &stVPC_Cur);
                    break;
                case GET_MIN:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    vpcCpy(*addr, &stVPC_Min);
                    break;
                 case GET_MAX:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    vpcCpy(*addr, &stVPC_Max);
                    break;
                case GET_RES:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    vpcCpy(*addr, &stVPC_Res);
                    break;
                case GET_LEN:
                    **addr = sizeof(ST_VIDEO_PROBE_COMMIT);
                    retLen = 1;
                    break;
                case GET_INFO:
                    **addr = 0x3;
                    retLen = 1;
                    break;
                case SET_CUR:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    *CBfn = vsNegotiation;
                    break;
                default:
                    ret = FALSE;
            }
            break;

        case VS_COMMIT_CONTROL:
            switch(req)
            {
                case GET_CUR:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    vpcCpy(*addr, &stVPC_Cur);
                    break;
                case GET_MIN:
                case GET_MAX:
                case GET_RES:
                case GET_DEF:
                    retLen = 0;
                    break;
                case GET_LEN:
                    **addr = sizeof(ST_VIDEO_PROBE_COMMIT);
                    retLen = 1;
                    break;
                case GET_INFO:
                    **addr = 0x3;
                    retLen = 1;
                    break;
                case SET_CUR:
                    retLen = sizeof(ST_VIDEO_PROBE_COMMIT);
                    *CBfn = vsNegotiation;
                    break;
                default:
                    ret = FALSE;
            }
            break;

        default:
            ret = FALSE;
    }

    *len = (*len > retLen)?retLen:*len;
    return ret;
}

static BOOL asIsoEpControl(U08 req, U08 control, U32 *len, U08 **addr, void (**CBfn) (void *, U08*), void **CBpara)
{
    BOOL ret = FALSE;
    return ret;
}



#ifdef TRANSFER_DEBUG
static U08 timerId;
static U08 *fileBuf;
static U32 fileSize;
static S32 vsDataGetTest(U08* encBuf, U32 encSize, U32 encType)
{

}
#endif

static void videoConfStart(U16 width, U16 height, BOOL audioEnable)
{
    S32 status = NO_ERR;
    ST_IP_IMAGE_POINT stPos;
    ST_IP_IMAGE_SIZE stSize;
    U32 resolution = 0xffffffff;

    MP_DEBUG("VS Resolution = %3d x %3d", width, height);
    if(width == 640 && height == 480)
        resolution = VIDEO_RESOLUTION_640x480;
    else if(width == 352 && height == 288)
        resolution = VIDEO_RESOLUTION_352x288;
    else if(width == 320 && height == 240)
        resolution = VIDEO_RESOLUTION_320x240;
    else if(width == 176 && height == 144)
        resolution = VIDEO_RESOLUTION_176x144;
    else if(width == 736 && height == 480)
        resolution = VIDEO_RESOLUTION_736x480;
    else if(width == 736 && height == 576)
        resolution = VIDEO_RESOLUTION_736x576;
    else
    {
        MP_DEBUG("Not supported resolution");
        return;
    }

#ifdef TRANSFER_DEBUG
#if 1
    if( mpx_VideoFunctionsStartup() || mpx_AudioFunctionsStartup() )
    {
        MP_DEBUG("[Failure]");
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    mpx_GdcPanelVisualSizeGet(IDU_IMAGE_WINDOW, &stSize.u16H, &stSize.u16V);

    stPos.u16H = 0;
    stPos.u16V = 0;

    mpx_SysCfgImageWindowSet(stPos, stSize);

    status = mpx_VideoConPreviewStart();

    ///////////////////////////////////////////////////////////////////////////
    MP_DEBUG("Video Camera Preview ....");
    if ( status != NO_ERR )
        MP_DEBUG("Failure [%x]", status);
    else
    {
        U08 codecType, encQuality, encResolution, decResolution;

        MP_DEBUG("[WEBCAM] Visual Codec Init...");
        status = mpx_VideoConVisualCodecInit(&codecType, &encQuality, &encResolution, &decResolution);
        if ( status != NO_ERR )
            MP_DEBUG("Failure [%x]", status);
        else
            MP_DEBUG("Success");
        MP_DEBUG("CodecType: %2x, Enc Quality: %2x, Enc Reso: %2x, Dec Reso: %2x", codecType, encQuality, encResolution, decResolution);

//        status = mpx_VideoConVisualEncodedTypeSet(VIDEO_CON_VISUAL_MP4_TYPE,
        status = mpx_VideoConVisualEncodedTypeSet(VIDEO_CON_VISUAL_JPG_TYPE,
                                                  VIDEO_CON_IMAGE_WINDOW,
                                                  VIDEO_QUALITY_ECONOMIC,
                                                  resolution,
                                                  VIDEO_CON_P_FRAME_TYPE,
                                                  VIDEO_CON_0_COUNT_SKIP_FRAME,
                                                  FALSE,
                                                  FALSE,
                                                  vsDataGetTest);

        MP_DEBUG("[WEBCAM] Start visual transmission...");
        status = mpx_VideoConTransmissionStart(TRUE);
        if ( status != NO_ERR )
            MP_DEBUG("Failure [%x]", status);
        else
            MP_DEBUG("Success");
    }
#endif
    timerId = mpx_TimerCreate(vsDataGet, OS_TIMER_NORMAL, 1, 30);
    mpx_TimerStart(timerId, 0xffffffff);
#else
    MP_DEBUG("## WebCam Video Streaming Start ... ");
    if( mpx_VideoFunctionsStartup() || mpx_AudioFunctionsStartup() )
    {
        MP_DEBUG("[Failure]");
        return;
    }

    ///////////////////////////////////////////////////////////////////////////
    mpx_GdcPanelVisualSizeGet(IDU_IMAGE_WINDOW, &stSize.u16H, &stSize.u16V);

    stPos.u16H = 0;
    stPos.u16V = 0;

    mpx_SysCfgImageWindowSet(stPos, stSize);

    status = mpx_VideoConPreviewStart();

    ///////////////////////////////////////////////////////////////////////////
    MP_DEBUG("Video Camera Preview ....");
    if ( status != NO_ERR )
        mpDebugPrint("Failure [%x]", status);
    else
    {
        U08 codecType, encQuality, encResolution, decResolution;

        MP_DEBUG("[WEBCAM] Visual Codec Init...");
        status = mpx_VideoConVisualCodecInit(&codecType, &encQuality, &encResolution, &decResolution);
        if ( status != NO_ERR )
            mpDebugPrint("Failure [%x]", status);
        else
            MP_DEBUG("Success");
        MP_DEBUG("CodecType: %2x, Enc Quality: %2x, Enc Reso: %2x, Dec Reso: %2x", codecType, encQuality, encResolution, decResolution);

//        status = mpx_VideoConVisualEncodedTypeSet(VIDEO_CON_VISUAL_MP4_TYPE,
        status = mpx_VideoConVisualEncodedTypeSet(VIDEO_CON_VISUAL_JPG_TYPE,
                                                  VIDEO_CON_IMAGE_WINDOW,
                                                  VIDEO_QUALITY_ECONOMIC,
                                                  resolution,
                                                  VIDEO_CON_P_FRAME_TYPE,
                                                  VIDEO_CON_0_COUNT_SKIP_FRAME,
                                                  FALSE,
                                                  FALSE,
                                                  vsDataGet);

#if MICROPHONE_SUPPORT == ENABLE
        if(audioEnable)
        {
            AudioCodecInit(0);

            mpx_AudioCodecInit();
            mpx_AudioCodecAdcEnable(8000);
            mpx_AudioCodecVolumnSet(MIC_INPUT, MAX_GAIN_VALUE);
            MP_DEBUG("[WEBCAM] Sound Encoded Type Set...");
            status += mpx_VideoConSoundEncodedTypeSet(VIDEO_CON_SOUND_PCMA_TYPE, AUDIO_SAMPLERATE_8K, asDataGet);
            if ( status != NO_ERR )
                mpDebugPrint("Failure [%x]", status);
            else
                MP_DEBUG("Success");
        }
#endif
        MP_DEBUG("[WEBCAM] Start visual transmission...");
        status = mpx_VideoConTransmissionStart(audioEnable);
        if ( status != NO_ERR )
            mpDebugPrint("Failure [%x]", status);
        else
            MP_DEBUG("Success");
    }
#endif
}

static void videoConfStop()
{
    mpx_VideoConTransmissionStop();
    mpx_VideoConPreviewStop();
}

static S32 vsDataGet(U08* encBuf, U32 encSize, U32 encType)
{
    ST_PIPE_MESSAGE pmsg;
#if 0
    static U32 prevTime = 0;
    U32 currTime;

    currTime = mpx_SystemTickerGet() * 4;
    currTime += (HwTimerCurValueGet(0) * HwTimerResolutionGet(0)) / 1000;
    DPrintf("V,%4d", currTime - prevTime);
    prevTime = currTime;
#endif

    pmsg.data = encBuf;
    pmsg.u32Len = encSize;
    pmsg.u32Event = encType;

    #ifdef TRANSFER_DEBUG
    {
        pmsg.data = fileBuf;
        pmsg.u32Len = fileSize;
    }
    #endif

    //mpx_TaskPriorityChange(webCamVsTaskId, ISR_TASK_MAX_PRI);
    //mpx_MessageSend(webCamVsMsgId, (U08 *)&pmsg, sizeof(ST_PIPE_MESSAGE));
    if(boolVsStart)
        mpx_MessageDrop(webCamVsMsgId, (U08 *)&pmsg, sizeof(ST_PIPE_MESSAGE));


    return 0;
}



static void vsStart(void *para)
{
    U16 width, height;
    S32 status;

    if(boolVsStart == FALSE)
    {
        MP_DEBUG("WebCam VS tsk id=%2d", webCamVsTaskId);

    	status = TaskStartup(webCamVsTaskId);
    	if (status != USBOTG_NO_ERROR)
    	{
    	    MP_ALERT("USB Device %s Task Id %d Startup fail %d", __FUNCTION__, webCamVsTaskId, status);
    	    return;
   		}
    }
    else
    {
        MP_DEBUG("Video stream has started already");
        return;
    }

    width = *(U32 *)para;
    height = *(U32 *)(para + 4);
    if(boolAsStart) //audio stream has started, to stop and restart it.
        videoConfStop();
    videoConfStart(width, height, TRUE);

    boolVsStart = TRUE;
}



static void vsPause()
{
    ST_PIPE_MESSAGE pmsg;

    if(boolVsStart && !boolAsStart) //only vs work
    {
#ifdef TRANSFER_DEBUG
        mpx_TimerStop(timerId);
        mpx_TimerDestroy(timerId);
#endif
        videoConfStop();
    }
    else if(!boolVsStart)
        return;

    if(vsTransferBuf)
    {
        FreeUsbdBuf(vsTransferBuf);
        vsTransferBuf = 0;
    }
    //REMOVE mpx_TaskDisarm(webCamVsTaskId);
    //REMOVE while(mpx_MessageGrab(webCamVsMsgId, (U08 *)&pmsg) != OS_STATUS_POLLING_FAILURE)
            ;
    boolVsStart = FALSE;
}



static S32 asDataGet(U08* encBuf, U32 encSize, U32 enable)
{
    ST_PIPE_MESSAGE pmsg;
    pmsg.data = encBuf;
    pmsg.u32Len = encSize;
    pmsg.u32Event = enable;

    if(boolAsStart && !boolMute)
        mpx_MessageDrop(webCamAsMsgId, (U08 *)&pmsg, sizeof(ST_PIPE_MESSAGE));


    return 0;
}

static void asStart(void *para)
{
    S32 status;

    if(boolAsStart == FALSE)
    {
        MP_DEBUG("WebCam AS tsk id=%2d", webCamAsTaskId);

    	status = TaskStartup(webCamAsTaskId);
    	if (status != USBOTG_NO_ERROR)
    	{
    	    MP_ALERT("USB Device %s Task Id %d Startup fail %d", __FUNCTION__, webCamAsTaskId, status);
    	    return;
   		}
    }
    else
    {
        MP_DEBUG("Audio stream has started already");
        return;
    }

    if(!boolVsStart)    //only microphone function
        videoConfStart(176, 144, TRUE);

    mpx_AudioCodecVolumnSet(MIC_INPUT, currVolumeValue);
    boolAsStart = TRUE;
}



static void asPause()
{
    ST_PIPE_MESSAGE pmsg;

    if(boolAsStart && !boolVsStart) //only as work
        videoConfStop();
    else if(!boolAsStart)
        return;

    //REMOVE mpx_TaskDisarm(webCamAsTaskId);
    //REMOVE while(mpx_MessageGrab(webCamAsMsgId, (U08 *)&pmsg) != OS_STATUS_POLLING_FAILURE)
            ;
    boolAsStart = FALSE;
}
#if MICROPHONE_SUPPORT == ENABLE
#define NUM_AUDIO_BUF 2
#define AUDIO_BUF_SIZE 1024
static int usbAudioIndex;
static int audioIndex;
static int usbAudioBufPos=-1;
static BOOL bAudioInit=FALSE;
struct audioBuf
{
	DWORD len;
	BYTE  *data;
};
static struct audioBuf audioStream[NUM_AUDIO_BUF];
DWORD writepcm1(BYTE *buf, DWORD len, BYTE hciSize)
{
	if (len > AUDIO_BUF_SIZE) {
		mpDebugPrint("%s error len", __func__);
		return -1;
	}

	if (audioStream[audioIndex].len > 0) {
		mpDebugPrint("af");
		audioIndex = (usbAudioIndex+1) == NUM_AUDIO_BUF ? 0 : (usbAudioIndex+1);
		audioStream[audioIndex].len = 0;
	}
	if (audioStream[audioIndex].len == 0) {
	   	audioStream[audioIndex].data = buf;
	    audioStream[audioIndex].len = len;
		audioIndex = (++audioIndex) == NUM_AUDIO_BUF ? 0 : audioIndex;
	}
}

static BYTE remainTime=0;
DWORD audioDataGet(BYTE **buf, int len)
{
#if 1
	if (usbAudioBufPos==0)
		remainTime=32;
	--remainTime;
	if (audioStream[usbAudioIndex  ? 0 : 1].len == AUDIO_BUF_SIZE) {
		//if (remainTime>=8)
			remainTime |= 0x80;
		if ((audioStream[usbAudioIndex].len - usbAudioBufPos) <= 64)
			len = audioStream[usbAudioIndex].len - usbAudioBufPos;
		else if (remainTime&0x80)
			len=72;
	}
#endif
	if (audioStream[usbAudioIndex].len > usbAudioBufPos+len) {
		*buf = &audioStream[usbAudioIndex].data[usbAudioBufPos];
		usbAudioBufPos+=len;
	} else if (audioStream[usbAudioIndex].len > 0) {
		*buf = &audioStream[usbAudioIndex].data[usbAudioBufPos];
		len=audioStream[usbAudioIndex].len-usbAudioBufPos;
		audioStream[usbAudioIndex].len=0;
		usbAudioBufPos=0;
		usbAudioIndex = (++usbAudioIndex == NUM_AUDIO_BUF) ? 0 : usbAudioIndex;
	} else {
		mpDebugPrint("ae");
		len = 0;
	}
	return len;
}
#endif
//-----------------------------------------------------------------------------------------
static unsigned int getSensorJpgFrameData(unsigned char *buf)
{
	DWORD SensorEvent;
	DWORD IMG_size=0;

extern int SensorInputWidth;
extern int SensorInputHeight;
#if SENSOR_WIN_NUM
extern BYTE *pbSensorWinBuffer;
extern ST_IMGWIN SensorInWin[SENSOR_WIN_NUM];
#endif
    DWORD IPW_Shift = 0;

	ST_IMGWIN win ;
	
	if (EventWaitWithTO(SENSOR_IPW_FRAME_END_EVENT_ID, BIT3, OS_EVENT_OR, &SensorEvent, 8000)==OS_STATUS_TIMEOUT)
		return 0;
#if SENSOR_WIN_NUM
	IPU *ipu = (IPU *) IPU_BASE;
	DWORD GetReadyBuf;
  	memset(&win, 0, sizeof(ST_IMGWIN)); 

		if(API_SensorUsb_GetShareBuf_status() == 1)
		{
			IPW_Shift = 3;
		}


	GetReadyBuf = API_PCCAM_GetReadyBufCnt();


	MP_DEBUG2("%s: GetReadyBuf= %d", __FUNCTION__, GetReadyBuf);
	
  	win.pdwStart= SensorInWin[GetReadyBuf].pdwStart+IPW_Shift;

	win.dwOffset= SensorInWin[GetReadyBuf].wWidth<<1;
	win.wHeight = SensorInWin[GetReadyBuf].wHeight;
	win.wWidth  = SensorInWin[GetReadyBuf].wWidth;

	IMG_size=ImageFile_Encode_Img2Jpeg(buf, &win/*&win->pdwStart*/);
#endif
    MP_DEBUG2("%s: IMG_size = %d", __func__, IMG_size);
    Api_Set_PCCAM_SwitchBuf_flag();

	return IMG_size;
}
static int sensorBufIdx=0;
static struct SENSOR_SHARE_BUF *sensorShareBuf;
static unsigned int getSensorYuvFrameData(unsigned char **buf, DWORD *idx)
{
	DWORD IMG_size;
	DWORD sensorBufNum = API_SensorUsbUsedBufNum();
	DWORD IPW_Shift = 0;

	if (stVPC_Cur.bFrameIndex==1)
		IMG_size=640*480*2;
	else if (stVPC_Cur.bFrameIndex==2)
		IMG_size=320*240*2;
	else if (stVPC_Cur.bFrameIndex==3)
		IMG_size=176*144*2;

		if(API_SensorUsb_GetShareBuf_status() == 1)
		{
			IPW_Shift = 3;
		}

	if (API_SensorUsb_GetEmptyFlag(sensorBufIdx))
		return 0;

	sensorShareBuf = API_SensorUsbGetShareBufInfo(sensorBufIdx);
	*buf = sensorShareBuf->Buf;
	*idx = sensorBufIdx;
	sensorBufIdx = (++sensorBufIdx == sensorBufNum) ? 0 : sensorBufIdx; 
	return IMG_size;
}

#define NUM_SENSOR_FRAME 2
#define SENSOR_FRAME_SIZE 128*1024
struct sensorFrame 
{
	DWORD empty;
	DWORD sensorBufIdx;
	DWORD len;
	BYTE  *data;
	BYTE dataBuf[SENSOR_FRAME_SIZE+12];
};
static struct sensorFrame sensorFrameData[NUM_SENSOR_FRAME];
static int sensorFrameIndex = -1;
static BOOL videoIfEnable=FALSE;
static void prepareVideoDataGet(void)
{
	if (!videoIfEnable) {
		TaskSleep(500);
		return;
	}

	if (sensorFrameIndex == -1) {
		sensorFrameData[0].empty = TRUE;
		sensorFrameData[1].empty = TRUE;
		sensorFrameIndex = 0;
	}
	if (!sensorFrameData[sensorFrameIndex].empty) {
		TaskSleep(5);
		return;
	}
	if (stVPC_Cur.bFormatIndex == 1) {
		sensorFrameData[sensorFrameIndex].len = getSensorJpgFrameData(&sensorFrameData[sensorFrameIndex].dataBuf[12]);
		sensorFrameData[sensorFrameIndex].data = &sensorFrameData[sensorFrameIndex].dataBuf[0];
		sensorFrameData[sensorFrameIndex].sensorBufIdx = 0xff;
	}
	else if (stVPC_Cur.bFormatIndex == 2)
		sensorFrameData[sensorFrameIndex].len = getSensorYuvFrameData(&sensorFrameData[sensorFrameIndex].data, &sensorFrameData[sensorFrameIndex].sensorBufIdx);
	else sensorFrameData[sensorFrameIndex].len = 0;

	sensorFrameData[sensorFrameIndex].empty = (sensorFrameData[sensorFrameIndex].len == 0) ? TRUE : FALSE;
	if (!sensorFrameData[sensorFrameIndex].empty)
		sensorFrameIndex = (++sensorFrameIndex == NUM_SENSOR_FRAME) ? 0 : sensorFrameIndex;
}

void UsbdWebCamVsTask()
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(0);

    while(1) {
		//mpDebugPrint("1a0 %x 1a8 %x 1ac %x 160 %x 168 %x", 
		//	wFOTGPeri_Port(0x1a0),
		//	wFOTGPeri_Port(0x1a8),
		//	wFOTGPeri_Port(0x1ac),
		//	wFOTGPeri_Port(0x160),
		//	wFOTGPeri_Port(0x168));

		if (webCamWhichOtg == USBOTG_NONE)
			TaskSleep(500);
		else if (Api_UsbdGetCurrentStat(webCamWhichOtg) == USB_DEV_STATE_CONFIGURED) {
			prepareVideoDataGet();
		}
		else {
			TaskSleep(500);
		}
    }
}

void UsbdWebCamAsTask()
{
    ST_PIPE_MESSAGE pmsg;
    U08 *EncBuf;
    U32 EncSize;
    U16 pkSize;;

    while(1)
    {
        mpx_MessageReceive(webCamAsMsgId, (U08 *)&pmsg);

        EncBuf = pmsg.data;
        EncSize = pmsg.u32Len;
        /*
        pkSize = AS_EP_PKSIZE;
        pmsg.data = GetUsbdBuf(pkSize);
        memcpy(pmsg.data, EncBuf, EncSize);
        */

        UsbdPipeWrite(AS_ISO_IN_PIPE.u08PipeNum, &pmsg);

        //FreeUsbdBuf(pmsg.data);
    }
    //REMOVE mpx_TaskSleep();
}



U32 UsbdWebCamFuncInfoGet(U08 getinf)
{
    switch (getinf)
    {
        case GET_INTFACE_STR:
            return (U32) &PCCAM_STRING;

        default:
            DpString("No Dcrp Info Provided.");
    }

    return 0;
}



U08 UsbdWebCamFuncPipeCntGet()
{
    return WEBCAM_PIPE_COUNT;
    /*if(infNum == VC_INF_ID)
        return VC_INF_PIPE_COUNT;
    else if(infNum == VS_INF_ID)
        return VS_INF_PIPE_COUNT;*/
}



U08 UsbdWebCamFuncInfCntGet()
{
    return WEBCAM_INF_COUNT;
}



U08 UsbdWebCamFuncPipeTypeGet(U08 idx)
{
    return webCamPipes[idx].u08PipeType;
}



void UsbdWebCamFuncPipeNumSet(U08 idx, U08 num)
{
    webCamPipes[idx].u08PipeNum = num;
}



static BOOL bUsbdWebCamFuncInit=FALSE;
int UsbdWebCamFuncInit()
{
    S32 status;

	if (bUsbdWebCamFuncInit) return 0;
	bUsbdWebCamFuncInit = TRUE;

    webCamVsTaskId = UVC_TASK;
    status = TaskCreate(UVC_TASK, UsbdWebCamVsTask, DRIVER_PRIORITY, 0x1000);
    if (status != USBOTG_NO_ERROR)
    {
        MP_ALERT("USB Device UVC %s create fail %d", __FUNCTION__, status);
        return USBOTG_TASK_CREATE_ERROR;
    }
	TaskStartup(UVC_TASK);

    webCamVsMsgId = UVC_MAIL_ID;
    status = MailboxCreate(UVC_MAIL_ID, OS_ATTR_PRIORITY);
    if (status != USBOTG_NO_ERROR)
    {
        MP_ALERT("USB Device %s mail box Id %d create fail %d", __FUNCTION__, UVC_MAIL_ID, status);
        return USBOTG_MAILBOX_CREATE_ERROR;
    }

#if MICROPHONE_SUPPORT == ENABLE
    webCamAsTaskId = UAC_TASK;
    status = TaskCreate(UAC_TASK, UsbdWebCamAsTask, DRIVER_PRIORITY, 0x1000);
    if (status != USBOTG_NO_ERROR)
    {
        MP_ALERT("USB Device UAC %s create fail %d", __FUNCTION__, status);
        return USBOTG_TASK_CREATE_ERROR;
    }

    webCamAsMsgId = UAC_MAIL_ID;
    status = MailboxCreate(UAC_MAIL_ID, OS_ATTR_PRIORITY);
    if (status != USBOTG_NO_ERROR)
    {
        MP_ALERT("USB Device %s mail box Id %d create fail %d", __FUNCTION__, UAC_MAIL_ID, status);
        return USBOTG_MAILBOX_CREATE_ERROR;
    }
#endif

    UsbWebCamInit();

    boolVsStart = FALSE;
    boolAsStart = FALSE;
}



void UsbdWebCamFuncDeInit()
{
    //REMOVE mpx_TaskDestroy(webCamVsTaskId);
    webCamVsTaskId = 0;
    mpx_MessageDestroy(webCamVsMsgId);
    webCamVsMsgId = 0;
#if MICROPHONE_SUPPORT == ENABLE
    mpx_TaskYield(webCamAsTaskId);
    webCamAsTaskId = 0;
    mpx_MessageDestroy(webCamAsMsgId);
    webCamAsMsgId = 0;
#endif
}



BOOL UsbdWebCamFuncStart()
{
    return TRUE;
}



void UsbdWebCamFuncStop()
{
    if(boolVsStart)
        vsPause();
    UsbdPipeStop(VS_ISO_IN_PIPE.u08PipeNum);
#if MICROPHONE_SUPPORT == ENABLE
    if(boolAsStart)
        asPause();
    UsbdPipeStop(AS_ISO_IN_PIPE.u08PipeNum);
#endif
}


BOOL UsbdWebCamFunctionClassReq(WHICH_OTG eWhichOtg)
//BOOL UsbdWebCamFunctionClassReq(U08 * psetupPacket, U32 * len, U08 ** addr, void (**CBfn) (void *, U08*), void **CBpara)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
    PUSB_DEVICE_DESC psDevDesc = &psDev->sDesc;
	U32 len = psDev->psControlCmd->Length;
	U08 *addr;
	void (*CBfn) (void *, U08*);
	void *CBpara;
    BOOL ok = FALSE;

    U08 reqType = psDev->psControlCmd->Object;
    U08 req = psDev->psControlCmd->Request;
    U08 control = (U08)((psDev->psControlCmd->Value >> 8) & 0xff);
    U08 entityId = (U08)((psDev->psControlCmd->Index >> 8) & 0xff);
    U08 inf = (U08)(psDev->psControlCmd->Index & 0xff);

#if 0
	mpDebugPrint("------------------------");
	mpDebugPrint("reqType  %x", reqType);
	mpDebugPrint("req      %x", req);
	mpDebugPrint("control  %x", control);
	mpDebugPrint("entityId %x", entityId);
	mpDebugPrint("inf      %x", inf);
	mpDebugPrint("len      %x", len);
#endif

    if(reqType == 1)    //Inferface
    {
        if(entityId)    //control to entity
        {
            if(inf == VC_INF_ID || inf == VS_INF_ID)    //Video
            {
                switch(entityId)
                {
                    case CAMERA_IT_ID:
                        ok = cameraControl(req, control, &len, &addr, &CBfn, &CBpara);
                        break;

                    case PROCESSING_UT_ID:
                        ok = processingUnitControl(req, control, &len, &addr, &CBfn, &CBpara);
                        break;

                    default:
						ok = FALSE;
                        mpDebugPrint("Not supported entity=%2d control of Video, return stall", entityId);
                        UsbdPipeStallSet(0);
                        break;
                }
            }
            else    //Audio
            {
                switch(entityId)
                {
                    case FEATURE_UT_ID:
                        ok = audioFeatureControl(req, control, &len, &addr, &CBfn, &CBpara);
                        break;
                    default:
						ok = FALSE;
                        mpDebugPrint("Not supported entity=%2d control of Audio, return stall", entityId);
                        UsbdPipeStallSet(0);
                        break;
                }
            }
        }
        else            //control to interface
        {
            if(inf == VS_INF_ID)
                ok = vsInfControl(req, control, &len, &addr, &CBfn, &CBpara);
            else
                MP_DEBUG("Not supported interface=%2d control, return stall", inf);
        }
    }
    else if(reqType == 2)   //endpoint
    {
        if(inf == AS_ISO_IN_PIPE.u08PipeNum)
            ok = asIsoEpControl(req, control, &len, &addr, &CBfn, &CBpara);
        else
            MP_DEBUG("Not supported interface=%2d control, return stall", inf);
    }


    if(FALSE == ok)
    {
        MP_DEBUG("WebCam ep0 stall");
        UsbdPipeStallSet(0);
    }
	if (req == SET_CUR)
	{
		bOTGCxFxWrRd(FOTG200_DMA2CxFIFO, DIRECTION_OUT, addr, len, eWhichOtg);
		if (CBfn)
	        CBfn(NULL, addr);
	}
	else
		bOTGCxFxWrRd(FOTG200_DMA2CxFIFO, DIRECTION_IN, addr, len, eWhichOtg);

    return ok;
}



BOOL UsbdWebCamFunctionVendorReq(U08 * psetupPacket, U32 * len, U08 ** addr, void (**CBfn) (void *, U08*), void **CBpara)
{
    DpString("No Vendor Req exists in WebCam.");
    return FALSE;
}



BOOL UsbdWebCamFuncDevDecrpGet(U16 * len, U08 ** addr)
{
    static ST_DEV_DESC DevDesc;
    if(!(*len))
        return FALSE;

    *len = (*len > DEV_DECRP_LEN) ? DEV_DECRP_LEN : *len;
    //DevDesc = (ST_DEV_DESC *) GetUsbdBuf(512);

    DevDesc.u08bLength = DEV_DECRP_LEN;
    DevDesc.u08bDescriptorType = DEVICE;
    DevDesc.u16bcdUSB = 0x0002;
    DevDesc.u08bDeviceClass = 0xef;    //Miscellaneous
    DevDesc.u08bDeviceSubClass = 0x02; //Common class
    DevDesc.u08bDeviceProtocol = 0x01; //Interface Association Descriptor
    DevDesc.u08bMaxPacketSize0 = 0x40; //need to get hw information
    DevDesc.u16idVendor = (VID >> 8) + ((VID & 0xff) << 8);
#if MICROPHONE_SUPPORT == ENABLE
    DevDesc.u16idProduct = (PID_PCCAM_MIC >> 8) + ((PID_PCCAM_MIC & 0xff) << 8);
#else
    DevDesc.u16idProduct = (PID_PCCAM>> 8) + ((PID_PCCAM & 0xff) << 8);
#endif
    DevDesc.u16bcdDevice = 0x0200;     //version 0.2
    DevDesc.u08iManufacturer = 0x01;   //str idx = 1
    DevDesc.u08iProduct = 0x02;        //str idx = 2
    DevDesc.u08iSerialNumber = 0x03;   //str idx = 3
    DevDesc.u08bNumConfigurations = 1;

    *addr = (U08 *) &DevDesc;

    supportMJPEGFormat = MJPEG_FORMAT_SUPPORT;
    supportYUVFormat = YUV_FORMAT_SUPPORT;
    //REMOVE if(!UsbdPipeFrameNumGet(0, 0))   //if full speed mode, only support MJPEG format
         //supportYUVFormat = 0;
    //if(!supportMJPEGFormat && !supportYUVFormat)
    //    return FALSE;
    if(supportMJPEGFormat && supportYUVFormat)
    {
        firstFormat = mjpegFrameType;
        secondFormat = uncompressedFrameType;
        firstFormatFrameNum = sizeof(mjpegFrameType)/(FRAME_FIELDS<<2);
        secondFormatFrameNum = sizeof(uncompressedFrameType)/(FRAME_FIELDS<<2);
    }
    else if(supportMJPEGFormat)
    {
        firstFormat = mjpegFrameType;
        secondFormat = mjpegFrameType;
        firstFormatFrameNum = sizeof(mjpegFrameType)/(FRAME_FIELDS<<2);
        secondFormatFrameNum = firstFormatFrameNum;
    }
    else
    {
        firstFormat = uncompressedFrameType;
        secondFormat = uncompressedFrameType;
        firstFormatFrameNum = sizeof(uncompressedFrameType)/(FRAME_FIELDS<<2);
        secondFormatFrameNum = firstFormatFrameNum;
    }
    supportFormatNumber = supportMJPEGFormat + supportYUVFormat;

    return TRUE;
}



BOOL UsbdWebCamFuncConfigDecrpGet(U08 index, U32 * len, U08 ** addr, BOOL otherspeed)
{
	static U08 dataBuf[512];
    U16 totlength;
    U16 wValue;
    U32 dwValue;
    U32 i;
    U08 *adr;

    ST_CFG_DESC cfg_desc;
    ST_INF_DESC inf_desc;
    ST_EP_DESC ep_desc;
    ST_INF_ASSO_DESC ia_desc;
    ST_VC_INF_HEADER_DESC vc_header_desc;
    ST_CAM_TERMINAL_DESC cam_it_desc;
    ST_PROC_UNIT_DESC proc_ut_desc;
	ST_SCOLOR_FORMAT_DESC color_format_desc;
    ST_VIDEO_OUT_TERMINAL_DESC usb_video_ot_desc;
    ST_VS_INF_IN_HEADER_DESC vs_in_header_desc;
    ST_MJPEG_FORMAT_DESC mjpeg_format_desc;
    ST_MJPEG_FRAME_DESC mjpeg_frame_desc;
    ST_UNCOMPRESSED_FORMAT_DESC uncompressed_format_desc;
    ST_UNCOMPRESSED_FRAME_DESC uncompressed_frame_desc;
    ST_AC_INF_HEADER_DESC ac_inf_header_desc;
    ST_AUDIO_IN_TERMINAL_DESC usb_audio_it_desc;
    ST_AUDIO_OUT_TERMINAL_DESC usb_audio_ot_desc;
    ST_AUDIO_FEATURE_UNIT_DESC usb_audio_feature_desc;
    ST_AS_INF_GENERAL_DESC as_inf_general_desc;
    ST_TYPE_I_FORMAT_DESC type_i_format_desc;
    ST_CS_ISO_AUDIO_EP_DESC cs_iso_audio_ep_desc;


	mpDebugPrint("1 len %x", *len);

    if(!(*len) || index)
    {
        return FALSE;
    }

    totlength = CONFIG_DECRP_LEN + INF_ASSO_DESC_LEN;
    //Video Control descriptors
    totlength += INF_DECRP_LEN;
    totlength += VC_INF_HEADER_DESC_LEN;
    totlength += VC_CAM_TERMINAL_DESC_LEN;
    totlength += VC_PROC_UNIT_DESC_LEN;
    totlength += VC_OUT_TERMINAL_DESC_LEN;
    //Video Stream descriptors
    totlength += INF_DECRP_LEN; //Alt. set 0
    totlength += VS_IN_HEADER_DESC_LEN; //not include bmaControls field
    totlength += supportFormatNumber;    //bmaControls of Input Header Descriptor
    if(supportMJPEGFormat)
    {
        totlength += VS_MJPEG_FORMAT_DESC_LEN;
        totlength += VS_MJPEG_FRAME_DESC_LEN * sizeof(mjpegFrameType)/(FRAME_FIELDS<<2);
    }
    if(supportYUVFormat)
    {
        totlength += VS_UNCOMPRESSED_FORMAT_DESC_LEN;
        totlength += VS_UNCOMPRESSED_FRAME_DESC_LEN * sizeof(uncompressedFrameType)/(FRAME_FIELDS<<2);
		//totlength += VS_UNCOMPRESSED_COLOR_FORMAT_DESC_LEN;
    }
    totlength += INF_DECRP_LEN * sizeof(vsInfAlt)/(VS_INF_ALT_FIELDS<<2);
    totlength += END_DECRP_LEN * sizeof(vsInfAlt)/(VS_INF_ALT_FIELDS<<2);
#if MICROPHONE_SUPPORT == ENABLE
    totlength += INF_ASSO_DESC_LEN;
    //Audio Control descriptors
    totlength += INF_DECRP_LEN;
    totlength += AC_INF_HEADER_DESC_LEN;
    totlength += AC_IN_TERMINAL_DESC_LEN;
    totlength += AC_FEATURE_UNIT_DESC_LEN;
    totlength += AC_OUT_TERMINAL_DESC_LEN;
    //Audio Stream descriptors
    totlength += INF_DECRP_LEN; //Alt. set 0
    totlength += INF_DECRP_LEN; //Alt. set 1
    totlength += AS_INF_GEN_DESC_LEN;
    totlength += AS_TYPE_I_FORMAT_DESC_LEN;
    totlength += (END_DECRP_LEN + 2);
    totlength += AS_CS_EP_DESC_LEN;
#endif


    *addr = adr = &dataBuf[0];//GetUsbdBuf(512);
    memset(adr, 0, 512);

    //Configuration descriptor
    cfg_desc.U08bLength = CONFIG_DECRP_LEN;
    cfg_desc.U08bDescriptorType = CONFIGURATION;
    cfg_desc.U16wTotalLength = BYTE_SWAP_OF_U16(totlength);
    cfg_desc.U08bNumInterfaces = WEBCAM_INF_COUNT;
    cfg_desc.U08bConfigurationValue = 0x01;
    cfg_desc.U08iConfiguration = 0x00;
    cfg_desc.U08bmAttributes = 0x80;
    cfg_desc.U08MaxPower = 0x10;        //max-power 2*x
    DESC_FILL_BYTE(adr, cfg_desc.U08bLength);
    DESC_FILL_BYTE(adr, cfg_desc.U08bDescriptorType);
    DESC_FILL_WORD(adr, cfg_desc.U16wTotalLength);
    DESC_FILL_BYTE(adr, cfg_desc.U08bNumInterfaces);
    DESC_FILL_BYTE(adr, cfg_desc.U08bConfigurationValue);
    DESC_FILL_BYTE(adr, cfg_desc.U08iConfiguration);
    DESC_FILL_BYTE(adr, cfg_desc.U08bmAttributes);
    DESC_FILL_BYTE(adr, cfg_desc.U08MaxPower);

    //IAD
    ia_desc.u08bLength = INF_ASSO_DESC_LEN;
    ia_desc.u08bDescriptorType = INTERFACE_ASSOCIATION;
    ia_desc.u08bFirstInterface = VC_INF_ID;
    ia_desc.u08bInterfaceCount = VIDEO_INF_COUNT;
    ia_desc.u08bFunctionClass = CC_VIDEO;
    ia_desc.u08bFunctionSubClass = SC_VIDEO_INTERFACE_COLLECTION;
    ia_desc.u08bFunctionProtocal = PC_PROTOCAL_UNDEFINED;
    ia_desc.u08iFunction = INF_PRODUCT_STR_IDX;
    DESC_FILL_BYTE(adr, ia_desc.u08bLength);
    DESC_FILL_BYTE(adr, ia_desc.u08bDescriptorType);
    DESC_FILL_BYTE(adr, ia_desc.u08bFirstInterface);
    DESC_FILL_BYTE(adr, ia_desc.u08bInterfaceCount);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionClass);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionSubClass);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionProtocal);
    DESC_FILL_BYTE(adr, ia_desc.u08iFunction);

    //Standard VC Interface Descriptor
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = VC_INF_ID;
    inf_desc.U08bAlternateSetting = 0;
    inf_desc.U08bNumEndpoints = (STILL_IMG_SUPPORT)?1:0;
    inf_desc.U08bInterfaceClass = CC_VIDEO;
    inf_desc.U08bInterfaceSubClass = SC_VIDEOCONTROL;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = INF_PRODUCT_STR_IDX;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Class-specific VC Interface Descriptor
    wValue = VC_INF_HEADER_DESC_LEN + VC_CAM_TERMINAL_DESC_LEN + VC_PROC_UNIT_DESC_LEN + VC_OUT_TERMINAL_DESC_LEN;
    dwValue = SYS_CLOCK_FREQ;
    vc_header_desc.bLength = VC_INF_HEADER_DESC_LEN;
    vc_header_desc.bDescriptorType = CS_INTERFACE;
    vc_header_desc.bDescriptorSubtype = VC_HEADER;
    vc_header_desc.bcdUVC = 0x0001;    //0x0100 -> UVC 1.0
    vc_header_desc.wTotalLength = BYTE_SWAP_OF_U16(wValue);
    vc_header_desc.dwClockFrequency = BYTE_SWAP_OF_U32(dwValue);
    vc_header_desc.bInCollection = 1;
    vc_header_desc.baInterfaceNr[0] = 1;
    DESC_FILL_BYTE(adr, vc_header_desc.bLength);
    DESC_FILL_BYTE(adr, vc_header_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, vc_header_desc.bDescriptorSubtype);
    DESC_FILL_WORD(adr, vc_header_desc.bcdUVC);
    DESC_FILL_WORD(adr, vc_header_desc.wTotalLength);
    DESC_FILL_DWORD(adr, vc_header_desc.dwClockFrequency);
    DESC_FILL_BYTE(adr, vc_header_desc.bInCollection);
    DESC_FILL_BYTE(adr, vc_header_desc.baInterfaceNr[0]);

    //Camera Terminal Descriptor
    cam_it_desc.bLength = VC_CAM_TERMINAL_DESC_LEN;
    cam_it_desc.bDescriptorType = CS_INTERFACE;
    cam_it_desc.bDescriptorSubtype = VC_INPUT_TERMINAL;
    cam_it_desc.bTerminalID = CAMERA_IT_ID;
    cam_it_desc.wTerminalType = 0x0102;        //ITT_CAMERA type, ref. UVC_1.0 B.2
    cam_it_desc.bAssocTerminal = 0;
    cam_it_desc.iTerminal = 0;
    cam_it_desc.wObjectiveFocalLengthMin = 0;  //No optical zoom supported
    cam_it_desc.wObjectiveFocalLengthMax = 0;  //No optical zoom supported
    cam_it_desc.wOcularFocalLength = 0;        //No optical zoom supported
    cam_it_desc.bControlSize = 0x03;           //bmControls is 2 bytes
    cam_it_desc.bmControls[0] = 0;             //No controls are supported
    cam_it_desc.bmControls[1] = 0;             //No controls are supported
    cam_it_desc.bmControls[2] = 0;             //No controls are supported
    DESC_FILL_BYTE(adr, cam_it_desc.bLength);
    DESC_FILL_BYTE(adr, cam_it_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, cam_it_desc.bDescriptorSubtype);
    DESC_FILL_BYTE(adr, cam_it_desc.bTerminalID);
    DESC_FILL_WORD(adr, cam_it_desc.wTerminalType);
    DESC_FILL_BYTE(adr, cam_it_desc.bAssocTerminal);
    DESC_FILL_BYTE(adr, cam_it_desc.iTerminal);
    DESC_FILL_WORD(adr, cam_it_desc.wObjectiveFocalLengthMin);
    DESC_FILL_WORD(adr, cam_it_desc.wObjectiveFocalLengthMax);
    DESC_FILL_WORD(adr, cam_it_desc.wOcularFocalLength);
    DESC_FILL_BYTE(adr, cam_it_desc.bControlSize);
    DESC_FILL_BYTE(adr, cam_it_desc.bmControls[0]);
    DESC_FILL_BYTE(adr, cam_it_desc.bmControls[1]);
    DESC_FILL_BYTE(adr, cam_it_desc.bmControls[2]);

    //Output Terminal Descriptor
    usb_video_ot_desc.bLength = VC_OUT_TERMINAL_DESC_LEN;
    usb_video_ot_desc.bDescriptorType = CS_INTERFACE;
    usb_video_ot_desc.bDescriptorSubtype = VC_OUTPUT_TERMINAL;
    usb_video_ot_desc.bTerminalID = VIDEO_STREAMING_OT_ID;
    usb_video_ot_desc.wTerminalType = 0x0101;        //TT_STREAMING type, ref. UVC_1.0 B.2
    usb_video_ot_desc.bAssocTerminal = 0;
    usb_video_ot_desc.bSourceID = PROCESSING_UT_ID;
    usb_video_ot_desc.iTerminal = 0;
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bLength);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bDescriptorSubtype);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bTerminalID);
    DESC_FILL_WORD(adr, usb_video_ot_desc.wTerminalType);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bAssocTerminal);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.bSourceID);
    DESC_FILL_BYTE(adr, usb_video_ot_desc.iTerminal);

    //Processing Unit Descriptor
    proc_ut_desc.bLength = VC_PROC_UNIT_DESC_LEN;
    proc_ut_desc.bDescriptorType= CS_INTERFACE;
    proc_ut_desc.bDescriptorSubtype = VC_PROCESSING_UNIT;
    proc_ut_desc.bUnitID = PROCESSING_UT_ID;
    proc_ut_desc.bSourceID = CAMERA_IT_ID;
    proc_ut_desc.wMaxMultiplier = 0;
    proc_ut_desc.bControlSize = 2;
    proc_ut_desc.bmControls[0] = 0x3f;
    proc_ut_desc.bmControls[1] = 0;
    proc_ut_desc.bmControls[2] = 0; //iProcessing
    DESC_FILL_BYTE(adr, proc_ut_desc.bLength);
    DESC_FILL_BYTE(adr, proc_ut_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, proc_ut_desc.bDescriptorSubtype);
    DESC_FILL_BYTE(adr, proc_ut_desc.bUnitID);
    DESC_FILL_BYTE(adr, proc_ut_desc.bSourceID);
    DESC_FILL_WORD(adr, proc_ut_desc.wMaxMultiplier);
    DESC_FILL_BYTE(adr, proc_ut_desc.bControlSize);
    DESC_FILL_BYTE(adr, proc_ut_desc.bmControls[0]);
    DESC_FILL_BYTE(adr, proc_ut_desc.bmControls[1]);
    DESC_FILL_BYTE(adr, proc_ut_desc.bmControls[2]);

    //Standard VS Interface Descriptor
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = VS_INF_ID;
    inf_desc.U08bAlternateSetting = 0;
    inf_desc.U08bNumEndpoints = 0;     //no bandwidth used
    inf_desc.U08bInterfaceClass = CC_VIDEO;
    inf_desc.U08bInterfaceSubClass = SC_VIDEOSTREAMING;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = 0;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Class-specifc VS Input Header Descriptor
    wValue = VS_IN_HEADER_DESC_LEN;
    wValue += supportFormatNumber;   //bmaControls of Input Header Descriptor
    if(supportMJPEGFormat)
    {
        wValue += VS_MJPEG_FORMAT_DESC_LEN;
        wValue += VS_MJPEG_FRAME_DESC_LEN * sizeof(mjpegFrameType)/(FRAME_FIELDS<<2);
    }
    if(supportYUVFormat)
    {
        wValue += VS_UNCOMPRESSED_FORMAT_DESC_LEN;
        wValue += VS_UNCOMPRESSED_FRAME_DESC_LEN * sizeof(uncompressedFrameType)/(FRAME_FIELDS<<2);
    }
    vs_in_header_desc.bLength = VS_IN_HEADER_DESC_LEN + supportFormatNumber; //add bmaControls size
    vs_in_header_desc.bDescriptorType = CS_INTERFACE;
    vs_in_header_desc.bDescriptorSubtype = VS_INPUT_HEADER;
    vs_in_header_desc.bNumFormats = (supportMJPEGFormat + supportYUVFormat);
    vs_in_header_desc.wTotalLength = BYTE_SWAP_OF_U16(wValue);
    vs_in_header_desc.bEndpointAddress = VS_ISO_IN_PIPE.u08PipeNum | 0x80;
    vs_in_header_desc.bmInfo = 0;      //No dynamic format change supported
    vs_in_header_desc.bTerminalLink = usb_video_ot_desc.bTerminalID;
    vs_in_header_desc.bStillCaptureMethod = STILL_IMG_SUPPORT;
    vs_in_header_desc.bTriggerSupport = 0; //Hardware trigger is not suppurted
    vs_in_header_desc.bTriggerUsage = 0;
    vs_in_header_desc.bControlSize = 1;
    vs_in_header_desc.bmaControls[0] = 0;  //No VideoStreaming specific control are supported
    vs_in_header_desc.bmaControls[1] = 0;
    DESC_FILL_BYTE(adr, vs_in_header_desc.bLength);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bDescriptorSubtype);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bNumFormats);
    DESC_FILL_WORD(adr, vs_in_header_desc.wTotalLength);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bEndpointAddress);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bmInfo);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bTerminalLink);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bStillCaptureMethod);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bTriggerSupport);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bTriggerUsage);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bControlSize);
    DESC_FILL_BYTE(adr, vs_in_header_desc.bmaControls[0]);
    if((supportMJPEGFormat + supportYUVFormat) == 2)
        DESC_FILL_BYTE(adr, vs_in_header_desc.bmaControls[1]);

    if(supportMJPEGFormat)
    {
        //Class-specifc VS Format Descriptor(MJPEG)
        mjpeg_format_desc.bLength = VS_MJPEG_FORMAT_DESC_LEN;
        mjpeg_format_desc.bDescriptorType = CS_INTERFACE;
        mjpeg_format_desc.bDescriptorSubType = VS_FORMAT_MJPEG;
        mjpeg_format_desc.bFormatIndex = 1;
        mjpeg_format_desc.bNumFrameDescriptors = sizeof(mjpegFrameType)/(FRAME_FIELDS<<2);
        mjpeg_format_desc.bmFlags = 1;     //Uses fixed size samples
        mjpeg_format_desc.bDefaultFrameIndex = 1;  //Default frame index is 1
        mjpeg_format_desc.bAspectRatioX = 0;   //Non-interlaced stream - not required
        mjpeg_format_desc.bAspectRatioY = 0;   //Non-interlaced stream - not required
        mjpeg_format_desc.bmInterlaceFlags = 0;//Non-interlaced stream
        mjpeg_format_desc.bCopyProtect = 0;
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bLength);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bDescriptorType);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bDescriptorSubType);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bFormatIndex);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bNumFrameDescriptors);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bmFlags);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bDefaultFrameIndex);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bAspectRatioX);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bAspectRatioY);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bmInterlaceFlags);
        DESC_FILL_BYTE(adr, mjpeg_format_desc.bCopyProtect);

        //Class-specifc VS Frame Descriptor(MJPEG)
        for(i=0; i<mjpeg_format_desc.bNumFrameDescriptors; i++)
        {
            mjpeg_frame_desc.bLength = VS_MJPEG_FRAME_DESC_LEN;
            mjpeg_frame_desc.bDescriptorType = CS_INTERFACE;
            mjpeg_frame_desc.bDescriptorSubtype = VS_FRAME_MJPEG;
            mjpeg_frame_desc.bFrameIndex = i+1;
            mjpeg_frame_desc.bmCapabilities = (STILL_IMG_SUPPORT == 1)?1:0;
            wValue = mjpegFrameType[i][0];
            mjpeg_frame_desc.wWidth = BYTE_SWAP_OF_U16(wValue);
            wValue = mjpegFrameType[i][1];
            mjpeg_frame_desc.wHeight = BYTE_SWAP_OF_U16(wValue);
            dwValue = mjpegFrameType[i][2];
            mjpeg_frame_desc.dwMinBitRate = BYTE_SWAP_OF_U32(dwValue);
            dwValue = mjpegFrameType[i][3];
            mjpeg_frame_desc.dwMaxBitRate = BYTE_SWAP_OF_U32(dwValue);
            dwValue = mjpegFrameType[i][4];
            mjpeg_frame_desc.dwMaxVideoFrameBufferSize = BYTE_SWAP_OF_U32(dwValue);
            dwValue = mjpegFrameType[i][5];
            mjpeg_frame_desc.dwDefaultFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            mjpeg_frame_desc.bFrameIntervalType = mjpegFrameType[i][6];
            dwValue = mjpegFrameType[i][7];
            mjpeg_frame_desc.dwMinFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            dwValue = mjpegFrameType[i][8];
            mjpeg_frame_desc.dwMaxFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            dwValue = mjpegFrameType[i][9];
            mjpeg_frame_desc.dwFrameIntervalStep = BYTE_SWAP_OF_U32(dwValue);

            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bLength);
            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bDescriptorType);
            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bDescriptorSubtype);
            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bFrameIndex);
            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bmCapabilities);
            DESC_FILL_WORD(adr, mjpeg_frame_desc.wWidth);
            DESC_FILL_WORD(adr, mjpeg_frame_desc.wHeight);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwMinBitRate);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwMaxBitRate);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwMaxVideoFrameBufferSize);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwDefaultFrameInterval);
            DESC_FILL_BYTE(adr, mjpeg_frame_desc.bFrameIntervalType);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwMinFrameInterval);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwMaxFrameInterval);
            DESC_FILL_DWORD(adr, mjpeg_frame_desc.dwFrameIntervalStep);
        }
    }

    if(supportYUVFormat)
    {
        //Class-specifc VS Format Descriptor(UNCOMPRESSED)
        uncompressed_format_desc.bLength = VS_UNCOMPRESSED_FORMAT_DESC_LEN;
        uncompressed_format_desc.bDescriptorType = CS_INTERFACE;
        uncompressed_format_desc.bDescriptorSubType = VS_FORMAT_UNCOMPRESSED;
        uncompressed_format_desc.bFormatIndex = 2;
        uncompressed_format_desc.bNumFrameDescriptors = sizeof(uncompressedFrameType)/(FRAME_FIELDS<<2);
        for(i=0; i<16; i++)
            uncompressed_format_desc.guidFormat[i] = compressionFormatGUID[i];
        uncompressed_format_desc.bBitsPerPixel = 16;
        uncompressed_format_desc.bDefaultFrameIndex = 1;    //Default frame index is 1
        uncompressed_format_desc.bAspectRatioX = 0;   //Non-interlaced stream - not required
        uncompressed_format_desc.bAspectRatioY = 0;   //Non-interlaced stream - not required
        uncompressed_format_desc.bmInterlaceFlags = 0;//Non-interlaced stream
        uncompressed_format_desc.bCopyProtect = 0;
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bLength);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bDescriptorType);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bDescriptorSubType);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bFormatIndex);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bNumFrameDescriptors);
        for(i=0; i<16; i++)
            DESC_FILL_BYTE(adr, uncompressed_format_desc.guidFormat[i]);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bBitsPerPixel);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bDefaultFrameIndex);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bAspectRatioX);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bAspectRatioY);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bmInterlaceFlags);
        DESC_FILL_BYTE(adr, uncompressed_format_desc.bCopyProtect);

        //Class-specifc VS Frame Descriptor(UNCOMPRESSED)
        for(i=0; i<uncompressed_format_desc.bNumFrameDescriptors; i++)
        {
            uncompressed_frame_desc.bLength = VS_UNCOMPRESSED_FRAME_DESC_LEN;
            uncompressed_frame_desc.bDescriptorType = CS_INTERFACE;
            uncompressed_frame_desc.bDescriptorSubtype = VS_FRAME_UNCOMPRESSED;
            uncompressed_frame_desc.bFrameIndex = i+1;
            uncompressed_frame_desc.bmCapabilities = (STILL_IMG_SUPPORT == 1)?1:0;
            wValue = uncompressedFrameType[i][0];
            uncompressed_frame_desc.wWidth = BYTE_SWAP_OF_U16(wValue);
            wValue = uncompressedFrameType[i][1];
            uncompressed_frame_desc.wHeight = BYTE_SWAP_OF_U16(wValue);
            dwValue = uncompressedFrameType[i][2];
            uncompressed_frame_desc.dwMinBitRate = BYTE_SWAP_OF_U32(dwValue);
            dwValue = uncompressedFrameType[i][3];
            uncompressed_frame_desc.dwMaxBitRate = BYTE_SWAP_OF_U32(dwValue);
            dwValue = uncompressedFrameType[i][4];
            uncompressed_frame_desc.dwMaxVideoFrameBufferSize = BYTE_SWAP_OF_U32(dwValue);
            dwValue = uncompressedFrameType[i][5];
            uncompressed_frame_desc.dwDefaultFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            uncompressed_frame_desc.bFrameIntervalType = uncompressedFrameType[i][6];
            dwValue = uncompressedFrameType[i][7];
            uncompressed_frame_desc.dwMinFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            dwValue = uncompressedFrameType[i][8];
            uncompressed_frame_desc.dwMaxFrameInterval = BYTE_SWAP_OF_U32(dwValue);
            dwValue = uncompressedFrameType[i][9];
            uncompressed_frame_desc.dwFrameIntervalStep = BYTE_SWAP_OF_U32(dwValue);

            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bLength);
            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bDescriptorType);
            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bDescriptorSubtype);
            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bFrameIndex);
            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bmCapabilities);
            DESC_FILL_WORD(adr, uncompressed_frame_desc.wWidth);
            DESC_FILL_WORD(adr, uncompressed_frame_desc.wHeight);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwMinBitRate);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwMaxBitRate);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwMaxVideoFrameBufferSize);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwDefaultFrameInterval);
            DESC_FILL_BYTE(adr, uncompressed_frame_desc.bFrameIntervalType);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwMinFrameInterval);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwMaxFrameInterval);
            DESC_FILL_DWORD(adr, uncompressed_frame_desc.dwFrameIntervalStep);
        }
#if 0
		//Processing colorformat Descriptor
		color_format_desc.bLength = 6;
		color_format_desc.bDescriptorType= CS_INTERFACE;
		color_format_desc.bDescriptorSubtype = 13;
		color_format_desc.bColorPrimaries= 3;
		color_format_desc.bTransferCharacteristics= 3;
		color_format_desc.bMatrixCoefficients= 3;
		DESC_FILL_BYTE(adr, color_format_desc.bLength);
		DESC_FILL_BYTE(adr, color_format_desc.bDescriptorType);
		DESC_FILL_BYTE(adr, color_format_desc.bDescriptorSubtype);
		DESC_FILL_BYTE(adr, color_format_desc.bColorPrimaries);
		DESC_FILL_BYTE(adr, color_format_desc.bTransferCharacteristics);
		DESC_FILL_BYTE(adr, color_format_desc.bMatrixCoefficients);
#endif
	}

    //Operational Alternate 1
    //Standard VS Interface Descriptor
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = VS_INF_ID;
    inf_desc.U08bAlternateSetting = 1;
    inf_desc.U08bNumEndpoints = 1;     //isochronous endpoint
    inf_desc.U08bInterfaceClass = CC_VIDEO;
    inf_desc.U08bInterfaceSubClass = SC_VIDEOSTREAMING;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = 0;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Standard VS Isochronous Video Data Endpoint Descriptor
    wValue = vsInfAlt[0][1] | ((vsInfAlt[0][0] - 1) << 11);
    ep_desc.U08bLength = END_DECRP_LEN;
    ep_desc.U08bDescriptorType = ENDPOINT;
    ep_desc.U08bEndpointAddress = VS_ISO_IN_PIPE.u08PipeNum | 0x80;
    ep_desc.U08bmAttributes = 0x05;     //Isochronous, Asynchronization
    ep_desc.U16wMaxPacketSize = BYTE_SWAP_OF_U16(wValue);
    ep_desc.U08bInterval = vsInfAlt[0][2];
    DESC_FILL_BYTE(adr, ep_desc.U08bLength);
    DESC_FILL_BYTE(adr, ep_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, ep_desc.U08bEndpointAddress);
    DESC_FILL_BYTE(adr, ep_desc.U08bmAttributes);
    DESC_FILL_WORD(adr, ep_desc.U16wMaxPacketSize);
    DESC_FILL_BYTE(adr, ep_desc.U08bInterval);
    for(i=1; i<sizeof(vsInfAlt)/(VS_INF_ALT_FIELDS<<2); i++)
    {
        //Standard VS Interface Descriptor
        inf_desc.U08bAlternateSetting = i+1;
        DESC_FILL_BYTE(adr, inf_desc.U08bLength);
        DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
        DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
        DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
        DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
        DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
        DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
        DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
        DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

        //Standard VS Isochronous Video Data Endpoint Descriptor
        wValue = vsInfAlt[i][1] | ((vsInfAlt[i][0] - 1) << 11);
        ep_desc.U16wMaxPacketSize = BYTE_SWAP_OF_U16(wValue);
        ep_desc.U08bInterval = vsInfAlt[i][2];
        DESC_FILL_BYTE(adr, ep_desc.U08bLength);
        DESC_FILL_BYTE(adr, ep_desc.U08bDescriptorType);
        DESC_FILL_BYTE(adr, ep_desc.U08bEndpointAddress);
        DESC_FILL_BYTE(adr, ep_desc.U08bmAttributes);
        DESC_FILL_WORD(adr, ep_desc.U16wMaxPacketSize);
        DESC_FILL_BYTE(adr, ep_desc.U08bInterval);
    }
#if MICROPHONE_SUPPORT == ENABLE
    //IAD
    ia_desc.u08bLength = INF_ASSO_DESC_LEN;
    ia_desc.u08bDescriptorType = INTERFACE_ASSOCIATION;
    ia_desc.u08bFirstInterface = AC_INF_ID;
    ia_desc.u08bInterfaceCount = AUDIO_INF_COUNT;
    ia_desc.u08bFunctionClass = CC_AUDIO;
    ia_desc.u08bFunctionSubClass = SC_AUDIOSTREAMING;
    ia_desc.u08bFunctionProtocal = PC_PROTOCAL_UNDEFINED;
    ia_desc.u08iFunction = INF_PRODUCT_STR_IDX;
    DESC_FILL_BYTE(adr, ia_desc.u08bLength);
    DESC_FILL_BYTE(adr, ia_desc.u08bDescriptorType);
    DESC_FILL_BYTE(adr, ia_desc.u08bFirstInterface);
    DESC_FILL_BYTE(adr, ia_desc.u08bInterfaceCount);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionClass);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionSubClass);
    DESC_FILL_BYTE(adr, ia_desc.u08bFunctionProtocal);
    DESC_FILL_BYTE(adr, ia_desc.u08iFunction);

    //Standard AC Interface Descriptor
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = AC_INF_ID;
    inf_desc.U08bAlternateSetting = 0;
    inf_desc.U08bNumEndpoints = 0;     //isochronous endpoint
    inf_desc.U08bInterfaceClass = CC_AUDIO;
    inf_desc.U08bInterfaceSubClass = SC_AUDIOCONTROL;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = INF_PRODUCT_STR_IDX;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Class-specific AC interface Descriptor
    wValue = AC_INF_HEADER_DESC_LEN + AC_IN_TERMINAL_DESC_LEN +
        AC_FEATURE_UNIT_DESC_LEN + AC_OUT_TERMINAL_DESC_LEN;
    ac_inf_header_desc.bLength= AC_INF_HEADER_DESC_LEN;
    ac_inf_header_desc.bDescriptorType = CS_INTERFACE;
    ac_inf_header_desc.bDescriptorSubType = AC_HEADER;
    ac_inf_header_desc.bcdADC = 0001;   //1.0
    ac_inf_header_desc.wTotalLength = BYTE_SWAP_OF_U16(wValue);
    ac_inf_header_desc.bInCollection = 1;
    ac_inf_header_desc.baInterfaceNr[0] = AS_INF_ID;
    DESC_FILL_BYTE(adr, ac_inf_header_desc.bLength);
    DESC_FILL_BYTE(adr, ac_inf_header_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, ac_inf_header_desc.bDescriptorSubType);
    DESC_FILL_WORD(adr, ac_inf_header_desc.bcdADC);
    DESC_FILL_WORD(adr, ac_inf_header_desc.wTotalLength);
    DESC_FILL_BYTE(adr, ac_inf_header_desc.bInCollection);
    DESC_FILL_BYTE(adr, ac_inf_header_desc.baInterfaceNr[0]);

    //Audio Input Terminal Descriptor
    usb_audio_it_desc.bLength = AC_IN_TERMINAL_DESC_LEN;
    usb_audio_it_desc.bDescriptorType = CS_INTERFACE;
    usb_audio_it_desc.bDescriptorSubType = AC_INPUT_TERMINAL;
    usb_audio_it_desc.bTerminalID = MICROPHONE_IT_ID;
    usb_audio_it_desc.wTerminalType = 0x0102;   //Microphone, Ref. USB Audio Terminal Types(termt10.pdf)
    usb_audio_it_desc.bAssocTerminal = 0;
    usb_audio_it_desc.bNrChannels = 2;  //one channel
    usb_audio_it_desc.wNrChannelConfig = BIT8|BIT9; //Mono sets no position bits
    usb_audio_it_desc.iChannelNames = 0;    //unused
    usb_audio_it_desc.iTerminal = 0;        //unused
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bLength);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bDescriptorSubType);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bTerminalID);
    DESC_FILL_WORD(adr, usb_audio_it_desc.wTerminalType);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bAssocTerminal);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.bNrChannels);
    DESC_FILL_WORD(adr, usb_audio_it_desc.wNrChannelConfig);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.iChannelNames);
    DESC_FILL_BYTE(adr, usb_audio_it_desc.iTerminal);

    //Audio Feature Unit Descritpor
    usb_audio_feature_desc.bLength = AC_FEATURE_UNIT_DESC_LEN;
    usb_audio_feature_desc.bDescriptorType = CS_INTERFACE;
    usb_audio_feature_desc.bDescripterSubType= AC_FEATURE_UNIT;
    usb_audio_feature_desc.bUnitID = FEATURE_UT_ID;
    usb_audio_feature_desc.bSourceID = MICROPHONE_IT_ID;
    usb_audio_feature_desc.bControlSize = 2;
    usb_audio_feature_desc.bmControls = 0x0300; //Support Mute, Volum
    usb_audio_feature_desc.iFeature = 0;
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bLength);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bDescripterSubType);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bUnitID);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bSourceID);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.bControlSize);
    DESC_FILL_WORD(adr, usb_audio_feature_desc.bmControls);
    DESC_FILL_BYTE(adr, usb_audio_feature_desc.iFeature);

    //Audio Output Terminal Descritpor
    usb_audio_ot_desc.bLength = AC_OUT_TERMINAL_DESC_LEN;
    usb_audio_ot_desc.bDescriptorType = CS_INTERFACE;
    usb_audio_ot_desc.bDescriptorSubType = AC_OUTPUT_TERMINAL;
    usb_audio_ot_desc.bTerminalID = AUDIO_STREAMING_OT_ID;
    usb_audio_ot_desc.wTerminalType = 0x0101;   //USB streaming, Ref. USB Audio Terminal Types(termt10.pdf)
    usb_audio_ot_desc.bAssocerminal = 0;
    usb_audio_ot_desc.bSourceID = FEATURE_UT_ID;
    usb_audio_ot_desc.iTerminal = 0;
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bLength);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bDescriptorSubType);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bTerminalID);
    DESC_FILL_WORD(adr, usb_audio_ot_desc.wTerminalType);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bAssocerminal);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.bSourceID);
    DESC_FILL_BYTE(adr, usb_audio_ot_desc.iTerminal);

    //Standard AS Interface Descriptor, Operational Alternate 0
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = AS_INF_ID;
    inf_desc.U08bAlternateSetting = 0;
    inf_desc.U08bNumEndpoints = 0;     //isochronous endpoint
    inf_desc.U08bInterfaceClass = CC_AUDIO;
    inf_desc.U08bInterfaceSubClass = SC_AUDIOSTREAMING;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = 0;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Standard AS Interface Descriptor, Operational Alternate 1
    inf_desc.U08bLength = INF_DECRP_LEN;
    inf_desc.U08bDescriptorType = INTERFACE;
    inf_desc.U08bInterfaceNumber = AS_INF_ID;
    inf_desc.U08bAlternateSetting = 1;
    inf_desc.U08bNumEndpoints = 1;     //isochronous endpoint
    inf_desc.U08bInterfaceClass = CC_AUDIO;
    inf_desc.U08bInterfaceSubClass = SC_AUDIOSTREAMING;
    inf_desc.U08bInterfaceProtocol = PC_PROTOCAL_UNDEFINED;
    inf_desc.U08iInterface = 0;
    DESC_FILL_BYTE(adr, inf_desc.U08bLength);
    DESC_FILL_BYTE(adr, inf_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceNumber);
    DESC_FILL_BYTE(adr, inf_desc.U08bAlternateSetting);
    DESC_FILL_BYTE(adr, inf_desc.U08bNumEndpoints);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceSubClass);
    DESC_FILL_BYTE(adr, inf_desc.U08bInterfaceProtocol);
    DESC_FILL_BYTE(adr, inf_desc.U08iInterface);

    //Class-specific AS General Interface Descriptor
    as_inf_general_desc.bLength = AS_INF_GEN_DESC_LEN;
    as_inf_general_desc.bDescriptorType = CS_INTERFACE;
    as_inf_general_desc.bDescriptorSubType = AS_GENERAL;
    as_inf_general_desc.bTerminalLink = AUDIO_STREAMING_OT_ID;
    as_inf_general_desc.bDelay = 1; //Interface delay
    as_inf_general_desc.wFormatTag = 0x0100;  //PCM, Ref Audio Data formats(frmts10.pdf)
    DESC_FILL_BYTE(adr, as_inf_general_desc.bLength);
    DESC_FILL_BYTE(adr, as_inf_general_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, as_inf_general_desc.bDescriptorSubType);
    DESC_FILL_BYTE(adr, as_inf_general_desc.bTerminalLink);
    DESC_FILL_BYTE(adr, as_inf_general_desc.bDelay);
    DESC_FILL_WORD(adr, as_inf_general_desc.wFormatTag);

    //Type I Format Type Descriptor
    type_i_format_desc.bLength = AS_TYPE_I_FORMAT_DESC_LEN;
    type_i_format_desc.bDescriptorType = CS_INTERFACE;
    type_i_format_desc.bDescriptorSubType = AS_FORMAT_TYPE;
    type_i_format_desc.bFormatType = 0x01;  //FORMAT_TYPE_I
    type_i_format_desc.bNrChannels = 0x02;  //One channel
    type_i_format_desc.bSubFrameSize = 0x02;    //Tow bytes per audio subframe
    type_i_format_desc.bBitResolution = 0x10;//0x10;//0x10;   //16 bits per sample
    type_i_format_desc.bSamFreqType = 0x01; //One frequency supported
    type_i_format_desc.tSamFreq[0][0] = 0x80;   //16000Hz
    type_i_format_desc.tSamFreq[0][1] = 0x3e;
    type_i_format_desc.tSamFreq[0][2] = 0x00;
    DESC_FILL_BYTE(adr, type_i_format_desc.bLength);
    DESC_FILL_BYTE(adr, type_i_format_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, type_i_format_desc.bDescriptorSubType);
    DESC_FILL_BYTE(adr, type_i_format_desc.bFormatType);
    DESC_FILL_BYTE(adr, type_i_format_desc.bNrChannels);
    DESC_FILL_BYTE(adr, type_i_format_desc.bSubFrameSize);
    DESC_FILL_BYTE(adr, type_i_format_desc.bBitResolution);
    DESC_FILL_BYTE(adr, type_i_format_desc.bSamFreqType);
    DESC_FILL_BYTE(adr, type_i_format_desc.tSamFreq[0][0]);
    DESC_FILL_BYTE(adr, type_i_format_desc.tSamFreq[0][1]);
    DESC_FILL_BYTE(adr, type_i_format_desc.tSamFreq[0][2]);

    //Standard Endpoint Descriptor
    wValue = AS_EP_PKSIZE;    //packet size = 64 byte
    ep_desc.U08bLength = END_DECRP_LEN + 2;
    ep_desc.U08bDescriptorType = ENDPOINT;
    ep_desc.U08bEndpointAddress = AS_ISO_IN_PIPE.u08PipeNum | 0x80;
    ep_desc.U08bmAttributes = 0x05;     //Isochronous, No Synchronization
    ep_desc.U16wMaxPacketSize = BYTE_SWAP_OF_U16(wValue);
    ep_desc.U08bInterval = 0x4;
    DESC_FILL_BYTE(adr, ep_desc.U08bLength);
    DESC_FILL_BYTE(adr, ep_desc.U08bDescriptorType);
    DESC_FILL_BYTE(adr, ep_desc.U08bEndpointAddress);
    DESC_FILL_BYTE(adr, ep_desc.U08bmAttributes);
    DESC_FILL_WORD(adr, ep_desc.U16wMaxPacketSize);
    DESC_FILL_BYTE(adr, ep_desc.U08bInterval);
    DESC_FILL_BYTE(adr, 0); //bReflash
    DESC_FILL_BYTE(adr, 0); //bSynchAddress

    //Class-specific Isoc. Audio Data Endpoint Descriptor
    cs_iso_audio_ep_desc.bLength = AS_CS_EP_DESC_LEN;
    cs_iso_audio_ep_desc.bDescriptorType = CS_ENDPOINT;
    cs_iso_audio_ep_desc.bDescritptorSubType = AS_GENERAL;
    cs_iso_audio_ep_desc.bmAttributes = 0;  //No sampling frequency control, no pitch control, no packet padding.
    cs_iso_audio_ep_desc.bLockDelayUnits = 0;   //Unused
    cs_iso_audio_ep_desc.wLockDelay = 0;        //Unused
    DESC_FILL_BYTE(adr, cs_iso_audio_ep_desc.bLength);
    DESC_FILL_BYTE(adr, cs_iso_audio_ep_desc.bDescriptorType);
    DESC_FILL_BYTE(adr, cs_iso_audio_ep_desc.bDescritptorSubType);
    DESC_FILL_BYTE(adr, cs_iso_audio_ep_desc.bmAttributes);
    DESC_FILL_BYTE(adr, cs_iso_audio_ep_desc.bLockDelayUnits);
    DESC_FILL_WORD(adr, cs_iso_audio_ep_desc.wLockDelay);
#endif

    *len = (*len > totlength) ? totlength : *len;

    return TRUE;
}
////////////////////////////////////////////////////////////////////////

static BYTE *usbFrameData;
static BYTE uvcHeader[12];
static WORD toggle = 1;
static WORD usbSensorFrameIndex=0;
static int usbFrameIndex=-1;
static DWORD bufPos;
static DWORD bufTotalLen;
static DWORD MAXLEN;
static DWORD ERR_BIT_ENABLE=1;
#define SAVE_VIDEO 0
#if SAVE_VIDEO
static DRIVE *sDrv = 0;
static STREAM *shandle = 0;
#endif

DWORD jpgDataGet(BYTE **buf, DWORD length)
{
	DWORD len;

	if ((usbFrameIndex == -1) && (sensorFrameData[usbSensorFrameIndex].empty)) {
		usbFrameData = &uvcHeader[0];
		usbFrameData[0] = 12;
		usbFrameData[1] = (BIT2|BIT3);
		if (ERR_BIT_ENABLE)
			usbFrameData[1]|=BIT6;

		usbFrameData[1] |= (BIT7);
		if (toggle)
			usbFrameData[1] |= BIT0;
		*buf = &usbFrameData[0];
		return 12;	
	} else if (usbFrameIndex == -1) {
#if SAVE_VIDEO
		if (sDrv == 0)
			sDrv=DriveGet(SD_MMC);

		CreateFile(sDrv, "yuv", "yu");
		shandle=FileOpen(sDrv);
#endif
		usbFrameData = &uvcHeader[0];
		usbFrameData[0] = 12;
		usbFrameData[1] = (BIT2|BIT3);
		if (ERR_BIT_ENABLE)
			usbFrameData[1]|=BIT6;

		bufPos = 0;
		bufTotalLen = sensorFrameData[usbSensorFrameIndex].len;
		usbFrameIndex = 0;
		usbFrameData[1] |= (BIT7);
		if (toggle)
			usbFrameData[1] |= BIT0;
		*buf = &usbFrameData[0];
		if (ERR_BIT_ENABLE) {
			usbFrameData[1] &= (~BIT6);
			usbFrameData[1] |= (BIT1);
			toggle = toggle ? 0 : 1;
			ERR_BIT_ENABLE=0;
		}
		return 12;	
	}
	MAXLEN=length-12;
	if ((bufPos+MAXLEN) < bufTotalLen) {
		usbFrameData = &sensorFrameData[usbSensorFrameIndex].data[bufPos];
		usbFrameData[0] = 12;
		usbFrameData[1] = (BIT2|BIT3);
		if (ERR_BIT_ENABLE)
			usbFrameData[1]|=BIT6;

		len = MAXLEN+12;
		bufPos+=MAXLEN;
		usbFrameData[1] &= ~BIT1;

		if (toggle)
			usbFrameData[1] |= BIT0;
#if SAVE_VIDEO
		FileWrite(shandle, &usbFrameData[12], len-12);
#endif

	}
	else {
		usbFrameData = &sensorFrameData[usbSensorFrameIndex].data[bufPos];
		usbFrameData[0] = 12;
		usbFrameData[1] = (BIT2|BIT3);
		if (ERR_BIT_ENABLE)
			usbFrameData[1]|=BIT6;
		len = bufTotalLen-bufPos+12;
		sensorFrameData[usbSensorFrameIndex].empty = TRUE;
		sensorFrameData[usbSensorFrameIndex].len = 0;
		if (sensorFrameData[sensorFrameIndex].sensorBufIdx != 0xff)
			API_SensorUsb_SetEmptyFlag( sensorFrameData[usbSensorFrameIndex].sensorBufIdx);
		usbSensorFrameIndex = (++usbSensorFrameIndex == NUM_SENSOR_FRAME) ? 0 : usbSensorFrameIndex;
		usbFrameIndex = -1;
		usbFrameData[1] |= BIT1;

		if (toggle)
			usbFrameData[1] |= BIT0;
		toggle = toggle ? 0 : 1;
#if SAVE_VIDEO
		FileWrite(shandle, &usbFrameData[12], len-12);
		FileClose(shandle);
#endif
	}
	usbFrameData[1] |= (BIT7);
	*buf = &usbFrameData[0];
	return len;
}

void usbdCamClrConfig(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);

	videoIfEnable = FALSE;
	usbFrameIndex=-1;
	sensorFrameIndex = -1;
	usbSensorFrameIndex = 0;
	sensorBufIdx= 0;
	sensorFrameData[0].empty = TRUE;
	sensorFrameData[1].empty = TRUE;
	ERR_BIT_ENABLE=1;
	wFOTGPeri_Port(0x1b0) |= BIT12;
	wFOTGPeri_Port(0x1b4) |= BIT12;
	wFOTGPeri_Port(0x1b8) |= BIT12;
	wFOTGPeri_Port(0x1bc) |= BIT12;

	wFOTGPeri_Port(0x1a0) &= 0x00ff0000;
	wFOTGPeri_Port(0x1a0) |= 0x33003333;
	wFOTGPeri_Port(0x1a8) &= 0xff000000;
	wFOTGPeri_Port(0x1a8) |= 0x000f0f0f;
	wFOTGPeri_Port(0x1ac) &= 0xff000000;
	wFOTGPeri_Port(0x138) |= 0x00010000;
	wFOTGPeri_Port(0x160) = 0x200;
	if (videoIfEnable) {
		API_Sensor_Stop();
		videoIfEnable = FALSE;
	}
	API_SensorUsb_DisableShareBuf();

	if (bAudioInit) {
		WaveInputEnd();
		Audio_CloseInputDevice();
		bAudioInit=FALSE;
	}
	usbAudioBufPos=-1;
	wFOTGPeri_Port(0x1a0) &= 0xff00ffff;
	wFOTGPeri_Port(0x1a0) |= 0x00330000;
	wFOTGPeri_Port(0x1a8) &= 0x00ffffff;
	wFOTGPeri_Port(0x1a8) |= 0x0f000000;
	wFOTGPeri_Port(0x1ac) &= 0x00ffffff;
	wFOTGPeri_Port(0x138) |= 0x00080000;
	wFOTGPeri_Port(0x168) = 0x200;
}

BOOL UsbdInfSet(WHICH_OTG eWhichOtg)
{
    PST_USB_OTG_DES psUsbOtg = (PST_USB_OTG_DES)UsbOtgDesGet(eWhichOtg);
    PST_USB_OTG_DEVICE psDev = &psUsbOtg->sUsbDev;
	WORD infNum, altNum;
    BOOL retCode = TRUE;

	webCamWhichOtg = eWhichOtg;
	infNum = psDev->psControlCmd->Index;
	altNum = psDev->psControlCmd->Value;

    mpDebugPrint("eWhichOtg %d Inf num:%2d, Alt num:%2d", eWhichOtg, infNum, altNum);

// ep1 for uvc, fifo0~3
// ep3 for uac, fifo3
	if (infNum == 1) // video interface ep1 iso in mpx 1024 fifo0
	{
		if (altNum == 0)
		{
			SysTimerProcEnable();
			videoIfEnable = FALSE;
			usbFrameIndex=-1;
			sensorFrameIndex = -1;
			usbSensorFrameIndex = 0;
			sensorBufIdx= 0;
			sensorFrameData[0].empty = TRUE;
			sensorFrameData[1].empty = TRUE;
			ERR_BIT_ENABLE=1;
			wFOTGPeri_Port(0x1b0) |= BIT12;
			wFOTGPeri_Port(0x1b4) |= BIT12;
			wFOTGPeri_Port(0x1b8) |= BIT12;

			wFOTGPeri_Port(0x1a0) &= 0xffffff00;
			wFOTGPeri_Port(0x1a0) |= 0x00000033;
			wFOTGPeri_Port(0x1a8) &= 0xff000000;
			wFOTGPeri_Port(0x1a8) |= 0x000f0f0f;
			wFOTGPeri_Port(0x1ac) &= 0xff000000;
			wFOTGPeri_Port(0x138) |= 0x00010000;
			wFOTGPeri_Port(0x160) = 0x200;
			API_Sensor_Stop();

			videoIfEnable = FALSE;
		}
		else if (altNum == 2)
		{
			SysTimerProcDisable();

			wFOTGPeri_Port(0x1a0) &= 0xfffffff0;
			wFOTGPeri_Port(0x1a8) &= 0xff000000;
			wFOTGPeri_Port(0x1a8) |= 0x00110011;
			wFOTGPeri_Port(0x1ac) |= 0x00150035;

			if (mUsbOtgHighSpeedST())
				wFOTGPeri_Port(0x160) = 0x400|BIT14;
			else
				wFOTGPeri_Port(0x160) = 0x00000040;
			wFOTGPeri_Port(0x138) &= 0xfffeffff;

			API_Sensor_Initial();
			videoIfEnable = TRUE;
		}
		mUsbOtgCfgSet();
	}

	if (infNum == 3) // audio interface ep3 iso in mpx 16 fifo2
	{
		if (altNum == 0)
		{
			int i;
			if (bAudioInit) {
				WaveInputEnd();
				Audio_CloseInputDevice();
				bAudioInit = FALSE;
			}

			audioIndex=0;
			for (i=0; i< NUM_AUDIO_BUF; ++i) {
				audioStream[i].len=0;
				audioStream[i].data=0;
			}
			usbAudioIndex=0;
			usbAudioBufPos=0;

			wFOTGPeri_Port(0x1bc) |= BIT12;

			wFOTGPeri_Port(0x1a0) &= 0xff00ffff;
			wFOTGPeri_Port(0x1a0) |= 0x00330000;
			wFOTGPeri_Port(0x1a8) &= 0x00ffffff;
			wFOTGPeri_Port(0x1a8) |= 0x0f000000;
			wFOTGPeri_Port(0x1ac) &= 0x00ffffff;
			wFOTGPeri_Port(0x138) |= 0x00080000;
			wFOTGPeri_Port(0x168) = 0x200;
		}
		else if (altNum == 1)
		{
			if (!bAudioInit) {
				Audio_OpenInputDevice(16000, 2, 16/8, writepcm1, AUDIO_BUF_SIZE, 0);
				WaveInputStart();
				bAudioInit = TRUE;
			}
			wFOTGPeri_Port(0x1a0) &= 0xfff0ffff;
			wFOTGPeri_Port(0x1a0) |= 0x00030000;
			wFOTGPeri_Port(0x1a8) &= 0x00ffffff;
			wFOTGPeri_Port(0x1a8) |= 0x13000000;
			wFOTGPeri_Port(0x1ac) |= 0x21000000;
			wFOTGPeri_Port(0x168) = AS_EP_PKSIZE;
			wFOTGPeri_Port(0x138) &= 0xfff7ffff;
		}
		mUsbOtgCfgSet();
	}

#if 1
//////////////////////////////////////////////

	psDev->eUsbCxFinishAction  = ACT_DONE;
	return TRUE;

//////////////////////////////////////////////
#endif
    return retCode;
}



void UsbdWebCamFuncConstruct(ST_USBD_FUNC * UsbdMsFunc)
{

    UsbdMsFunc->Uid = USBD_FUNC_UID_PCCAM;
    UsbdMsFunc->FuncInit = UsbdWebCamFuncInit;
    UsbdMsFunc->FuncDeInit = UsbdWebCamFuncDeInit;
    UsbdMsFunc->FuncStart = UsbdWebCamFuncStart;
    UsbdMsFunc->FuncStop = UsbdWebCamFuncStop;

    UsbdMsFunc->FuncPipeCntGet = UsbdWebCamFuncPipeCntGet;
    UsbdMsFunc->FuncInfCntGet = UsbdWebCamFuncInfCntGet;
    UsbdMsFunc->FuncPipeTypeGet = UsbdWebCamFuncPipeTypeGet;
    UsbdMsFunc->FuncPipeNumSet = UsbdWebCamFuncPipeNumSet;

    UsbdMsFunc->FuncInfoGet = UsbdWebCamFuncInfoGet;
    UsbdMsFunc->FuncClassReq = UsbdWebCamFunctionClassReq;
    UsbdMsFunc->FuncVendorReq = UsbdWebCamFunctionVendorReq;

    UsbdMsFunc->FuncDevDescGet = UsbdWebCamFuncDevDecrpGet;
    UsbdMsFunc->FuncConfDescGet = UsbdWebCamFuncConfigDecrpGet;
    UsbdMsFunc->FuncInfSet = UsbdInfSet;
}

//REMOVE MPX_REGISTRY_SET("USBD", "FUNC", (U32) UsbdWebCamFuncConstruct);

#endif




