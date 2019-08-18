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
* Filename		: usbotg_device_VAC.c
* Programmer(s)	: Morse
* Created Date	: 2010/05/06 
* Description   : USB Device UVC/UAC Class
******************************************************************************** 
*/

#ifndef __USBOTG_DEVICE_VAC_H
#define __USBOTG_DEVICE_VAC_H

#include "typedef.h"

// Start Video Preview
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_PREVIEW_START_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PREVIEW_START_RET_MESSAGE;

// Stop Video Preview
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_PREVIEW_STOP_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PREVIEW_STOP_RET_MESSAGE;

// Start Video Record
typedef struct
{
    U32 u32ActionId;
    U08 u08Quality;
    U08 u08Resolution;
    U08 u08AudioFormat;
    U08 u08VideoFormat;
    U08 u08VideoCodecType;
    U16 u16SampleRateIdx;
    U32 u32FileHandle;
    U16 u16IntPeriod;
    U16 u16MaxLength;
} ST_VIDEO_RECORD_START_MESSAGE;

typedef struct
{
    S32 s32Response;
    U16 u16SearilNumber;
} ST_VIDEO_RECORD_START_RET_MESSAGE;

// Stop Video Record
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_RECORD_STOP_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_RECORD_STOP_RET_MESSAGE;

// Pause Video Record
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_RECORD_PAUSE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_RECORD_PAUSE_RET_MESSAGE;

// Resume Video Record
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_RECORD_RESUME_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_RECORD_RESUME_RET_MESSAGE;

// Open Video Clip
typedef struct
{
    U32 u32ActionId;
    U16 u16FileFormat;
    BOOL boolAudioEnable;
    U32 u32FileHandle;
} ST_VIDEO_CLIP_OPEN_MESSAGE;

typedef struct
{
    S32 s32Response;
    U32 u32TotalLength;
    U32 u32SampleRate;
    U32 u32BitRate;
    U08 u08ChannelCount;
} ST_VIDEO_CLIP_OPEN_RET_MESSAGE;

// Start Video Play
typedef struct
{
    U32 u32ActionId;
    U16 u16IntPeriod;
} ST_VIDEO_PLAY_START_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PLAY_START_RET_MESSAGE;

// Stop Video Play
typedef struct
{
    U32 u32ActionId;
    BOOL boolFrameMode;
} ST_VIDEO_PLAY_STOP_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PLAY_STOP_RET_MESSAGE;

// Close Video Clip
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_CLIP_CLOSE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CLIP_CLOSE_RET_MESSAGE;

// Set Video Progress
typedef struct
{
    U32 u32ActionId;
    U32 u32VideoProgress;
} ST_VIDEO_PROGRESS_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PROGRESS_SET_RET_MESSAGE;

// Pause Video Play
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_PLAY_PAUSE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PLAY_PAUSE_RET_MESSAGE;

// Set Video Forward
typedef struct
{
    U32 u32ActionId;
    U16 u16SupportedStatus;
    U16 u16ActionStatus;
    BOOL boolSpeedup;
} ST_VIDEO_FORWARD_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
    U16 u16SupportedStatus;
} ST_VIDEO_FORWARD_SET_RET_MESSAGE;

// Redraw Video Image Window
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_IMAGE_WINDOW_REDRAW_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_IMAGE_WINDOW_REDRAW_RET_MESSAGE;

// Set Video Digital Zoom
typedef struct
{
    U32 u32ActionId;
    U08 u08Direction;
} ST_VIDEO_DIGITAL_ZOOM_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
    U08 u08MaxZoom;
    U08 u08MinZoom;
    U08 u08CurZoom;
} ST_VIDEO_DIGITAL_ZOOM_SET_RET_MESSAGE;

// Video Play End Order
typedef struct
{
    U32 u32ActionId;
    U32 u32OrderStatus;
} ST_VIDEO_PLAY_END_ORDER_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PLAY_END_ORDER_RET_MESSAGE;

// Video Error Recovery Order
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_RECOVERY_ORDER_MESSAGE;

// Video Record End Order
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_RECORD_END_ORDER_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_RECORD_END_ORDER_RET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_RECOVERY_ORDER_RETURN_MESSAGE;

// Start Video Conference Preview
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_CON_PREVIEW_START_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_PREVIEW_START_RET_MESSAGE;

// Stop Video Conference Preview
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_CON_PREVIEW_STOP_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_PREVIEW_STOP_RET_MESSAGE;

// Start Video Conference Transmission
typedef struct
{
    U32 u32ActionId;
    BOOL boolSoundEnable;
} ST_VIDEO_CON_TRANSMISSION_START_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_TRANSMISSION_START_RET_MESSAGE;

// Stop Video Conference Transmission
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_CON_TRANSMISSION_STOP_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_TRANSMISSION_STOP_RET_MESSAGE;

// Init Video Conference Visual Codec
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_CON_VISUAL_CODEC_INIT_MESSAGE;

typedef struct
{
    S32 s32Response;
    U08 u08CodecType;
    U08 u08EncodedQuality;
    U08 u08EncodedResolution;
    U08 u08DecodedResolution;
} ST_VIDEO_CON_VISUAL_CODEC_INIT_RET_MESSAGE;

// Set Video Conference Visual Encode Type
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
    U08 u08WindowType;
    U08 u08EncodedQuality;
    U08 u08EncodedResolution;
    U08 u08NextFrameType;
    U08 u08SkipFrames;
    BOOL boolErrResillence;
    BOOL boolRVLC;
    S32 (*callBackHandle)(U08*, U32*);
} ST_VIDEO_CON_VISUAL_ENCODE_TYPE_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_ENCODE_TYPE_SET_RET_MESSAGE;

// Set Video Conference Sound Encode Type
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
    U08 u08SampleRateIdx;
    S32 (*callBackHandle)(U08*, U32, BOOL);
} ST_VIDEO_CON_SOUND_ENCODE_TYPE_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_SOUND_ENCODE_TYPE_SET_RET_MESSAGE;

// Set Video Conference Visual Decode Type
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
    U08 u08WindowType;
    U08 u08DecodedResolution;
    S32 (*callBackHandle)(void);
} ST_VIDEO_CON_VISUAL_DECODE_TYPE_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_DECODE_TYPE_SET_RET_MESSAGE;

// Change Video Conference Visual Encode Param
typedef struct
{
    U32 u32ActionId;
    U08 u08EncodedQuality;
    U08 u08NextFrameType;
    U08 u08SkipFrames;
} ST_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE_RET_MESSAGE;

// Set Video Conference Visual CODEC VOL SET
typedef struct
{
    U32 u32ActionId;
    U32 u32VOLHeaderAddr;
    U16 u16VOLHeaderSize;
} ST_VIDEO_CON_VISUAL_CODEC_VOL_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_CODEC_VOL_SET_RET_MESSAGE;

// Fill Video Conference Visual Stream
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
    U08 *u08ComDataBuf;
    U32 u32ComDataLength;
} ST_VIDEO_CON_VISUAL_STREAM_FILL_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_STREAM_FILL_RET_MESSAGE;

// Measure Video Conference Visual Buffer
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
    U32 u32StreamSize;
} ST_VIDEO_CON_VISUAL_BUFFER_MEASURE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_BUFFER_MEASURE_RET_MESSAGE;

// Decode Video Conference Visual Stream
typedef struct
{
    U32 u32ActionId;
    U08 u08CodecType;
} ST_VIDEO_CON_VISUAL_STREAM_DECODE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_CON_VISUAL_STREAM_DECODE_RET_MESSAGE;

// Visual frame set
typedef struct
{
    U32 u32ActionId;
    U08 u08TargetId;
    U32 u32FrameDataSize;
    U32 u32FramePos;
    U32 u32FrameSize;
} ST_VIDEO_VISUAL_FRAME_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
    U32 u32FrameAddress;
} ST_VIDEO_VISUAL_FRAME_SET_RET_MESSAGE;

// Visual frame apply
typedef struct
{
    U32  u32ActionId;
    U08  u08TargetId;
    BOOL boolEnable;
} ST_VIDEO_VISUAL_FRAME_APPLY_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_VISUAL_FRAME_APPLY_RET_MESSAGE;

// Visual frame font set
typedef struct
{
    U32  u32ActionId;
    U08  u08TargetId;
    U08  u08FontIndex;
    U32  u32BmpSize;
    U32  u32BmpBufferPtr;
    U16  u16BmpColorKey;
} ST_VIDEO_VISUAL_FRAME_FONT_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_VISUAL_FRAME_FONT_SET_RET_MESSAGE;

// Visual frame font clean
typedef struct
{
    U32  u32ActionId;
    U08  u08TargetId;
} ST_VIDEO_VISUAL_FRAME_FONT_CLEAN_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_VISUAL_FRAME_FONT_CLEAN_RET_MESSAGE;

// Visual timestamp region set
typedef struct
{
    U32  u32ActionId;
    U08  u08TargetId;
    U32  u32RegionPos;
    U32  u32RegionSize;
} ST_VIDEO_TIMESTAMP_REGION_SET_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_TIMESTAMP_REGION_SET_RET_MESSAGE;

// Visual timestamp enable
typedef struct
{
    U32  u32ActionId;
    U08  u08TargetId;
    U08  u08DateEnable;
    U08  u08TimeEnable;
} ST_VIDEO_TIMESTAMP_ENABLE_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_TIMESTAMP_ENABLE_RET_MESSAGE;

// Video timestamp time set Order
typedef struct
{
    U32 u32ActionId;
} ST_VIDEO_TIMESTAMP_SET_ORDER_MESSAGE;

// Set Video Progress frame by frame
typedef struct
{
    U32 u32ActionId;
    U16 u16ActionStatus;
} ST_VIDEO_PROGRESS_FRAME_BY_FRAME_MESSAGE;

typedef struct
{
    S32 s32Response;
} ST_VIDEO_PROGRESS_FRAME_BY_FRAME_RET_MESSAGE;

typedef union
{
    ST_VIDEO_PREVIEW_START_MESSAGE                      VideoPreviewStartMsg;
    ST_VIDEO_PREVIEW_START_RET_MESSAGE                  VideoPreviewStartRetMsg;
    ST_VIDEO_PREVIEW_STOP_MESSAGE                       VideoPreviewStopMsg;
    ST_VIDEO_PREVIEW_STOP_RET_MESSAGE                   VideoPreviewStopRetMsg;
    ST_VIDEO_RECORD_START_MESSAGE                       VideoRecordStartMsg;
    ST_VIDEO_RECORD_START_RET_MESSAGE                   VideoRecordStartRetMsg;
    ST_VIDEO_RECORD_STOP_MESSAGE                        VideoRecordStopMsg;
    ST_VIDEO_RECORD_STOP_RET_MESSAGE                    VideoRecordStopRetMsg;
    ST_VIDEO_RECORD_PAUSE_MESSAGE                       VideoRecordPauseMsg;
    ST_VIDEO_RECORD_PAUSE_RET_MESSAGE                   VideoRecordPauseRetMsg;
    ST_VIDEO_RECORD_RESUME_MESSAGE                      VideoRecordResumeMsg;
    ST_VIDEO_RECORD_RESUME_RET_MESSAGE                  VideoRecordResumeRetMsg;
    ST_VIDEO_CLIP_OPEN_MESSAGE                          VideoOpenMsg;
    ST_VIDEO_CLIP_OPEN_RET_MESSAGE                      VideoOpenRetMsg;
    ST_VIDEO_PLAY_START_MESSAGE                         VideoStartMsg;
    ST_VIDEO_PLAY_START_RET_MESSAGE                     VideoStartRetMsg;
    ST_VIDEO_PLAY_STOP_MESSAGE                          VideoStopMsg;
    ST_VIDEO_PLAY_STOP_RET_MESSAGE                      VideoStopRetMsg;
    ST_VIDEO_CLIP_CLOSE_MESSAGE                         VideoCloseMsg;
    ST_VIDEO_CLIP_CLOSE_RET_MESSAGE                     VideoCloseRetMsg;
    ST_VIDEO_PROGRESS_SET_MESSAGE                       VideoProgressSetMsg;
    ST_VIDEO_PROGRESS_SET_RET_MESSAGE                   VideoProgressSetRetMsg;
    ST_VIDEO_PLAY_PAUSE_MESSAGE                         VideoPauseMsg;
    ST_VIDEO_PLAY_PAUSE_RET_MESSAGE                     VideoPauseRetMsg;
    ST_VIDEO_FORWARD_SET_MESSAGE                        VideoForwardSetMsg;
    ST_VIDEO_FORWARD_SET_RET_MESSAGE                    VideoForwardSetRetMsg;
    ST_VIDEO_IMAGE_WINDOW_REDRAW_MESSAGE                VideoImageWindowRedrawMsg;
    ST_VIDEO_IMAGE_WINDOW_REDRAW_RET_MESSAGE            VideoImageWindowRedrawRetMsg;
    ST_VIDEO_DIGITAL_ZOOM_SET_MESSAGE                   VideoDigitalZoomSetMsg;
    ST_VIDEO_DIGITAL_ZOOM_SET_RET_MESSAGE               VideoDigitalZoomSetRetMsg;
    ST_VIDEO_PLAY_END_ORDER_MESSAGE                     VideoPlayEndOrderMsg;
    ST_VIDEO_PLAY_END_ORDER_RET_MESSAGE                 VideoPlayEndOrderRetMsg;
    ST_VIDEO_RECOVERY_ORDER_MESSAGE                     VideoRecoveryOrderMsg;
    ST_VIDEO_RECOVERY_ORDER_RETURN_MESSAGE              VideoRecoveryOrderRetMsg;
    ST_VIDEO_RECORD_END_ORDER_MESSAGE                   VideoRecordEndOrderMsg;
    ST_VIDEO_RECORD_END_ORDER_RET_MESSAGE               VideoRecordEndOrderRetMsg;
    ST_VIDEO_CON_PREVIEW_START_MESSAGE                  VideoConPreviewStartMsg;
    ST_VIDEO_CON_PREVIEW_START_RET_MESSAGE              VideoConPreviewStartRetMsg;
    ST_VIDEO_CON_PREVIEW_STOP_MESSAGE                   VideoConPreviewStopMsg;
    ST_VIDEO_CON_PREVIEW_STOP_RET_MESSAGE               VideoConPreviewStopRetMsg;
    ST_VIDEO_CON_TRANSMISSION_START_MESSAGE             VideoConTransmissionStartMsg;
    ST_VIDEO_CON_TRANSMISSION_START_RET_MESSAGE         VideoConTransmissionStartRetMsg;
    ST_VIDEO_CON_TRANSMISSION_STOP_MESSAGE              VideoConTransmissionStopMsg;
    ST_VIDEO_CON_TRANSMISSION_STOP_RET_MESSAGE          VideoConTransmissionStopRetMsg;
    ST_VIDEO_CON_VISUAL_CODEC_INIT_MESSAGE              VideoConVisualCodecInitMsg;
    ST_VIDEO_CON_VISUAL_CODEC_INIT_RET_MESSAGE          VideoConVisualCodecInitRetMsg;
    ST_VIDEO_CON_VISUAL_ENCODE_TYPE_SET_MESSAGE         VideoConVisualEncodeTypeSetMsg;
    ST_VIDEO_CON_VISUAL_ENCODE_TYPE_SET_RET_MESSAGE     VideoConVisualEncodeTypeSetRetMsg;
    ST_VIDEO_CON_SOUND_ENCODE_TYPE_SET_MESSAGE          VideoConSoundEncodeTypeSetMsg;
    ST_VIDEO_CON_SOUND_ENCODE_TYPE_SET_RET_MESSAGE      VideoConSoundEncodeTypeSetRetMsg;
    ST_VIDEO_CON_VISUAL_DECODE_TYPE_SET_MESSAGE         VideoConVisualDecodeTypeSetMsg;
    ST_VIDEO_CON_VISUAL_DECODE_TYPE_SET_RET_MESSAGE     VideoConVisualDecodeTypeSetRetMsg;
    ST_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE_MESSAGE     VideoConVisualEncodeParamChangeMsg;
    ST_VIDEO_CON_VISUAL_ENCODE_PARAM_CHANGE_RET_MESSAGE VideoConVisualEncodeParamChangeRetMsg;
    ST_VIDEO_CON_VISUAL_CODEC_VOL_SET_MESSAGE           VideoConVisualCodecVolSetMsg;
    ST_VIDEO_CON_VISUAL_CODEC_VOL_SET_RET_MESSAGE       VideoConVisualCodecVolSetRetMsg;
    ST_VIDEO_CON_VISUAL_STREAM_FILL_MESSAGE             VideoConVisualStreamFillMsg;
    ST_VIDEO_CON_VISUAL_STREAM_FILL_RET_MESSAGE         VideoConVisualStreamFillRetMsg;
    ST_VIDEO_CON_VISUAL_BUFFER_MEASURE_MESSAGE          VideoConVisualBufferMeasureMsg;
    ST_VIDEO_CON_VISUAL_BUFFER_MEASURE_RET_MESSAGE      VideoConVisualBufferMeasureRetMsg;
    ST_VIDEO_CON_VISUAL_STREAM_DECODE_MESSAGE           VideoConVisualStreamDecodeMsg;
    ST_VIDEO_CON_VISUAL_STREAM_DECODE_RET_MESSAGE       VideoConVisualStreamDecodeRetMsg;
    ST_VIDEO_VISUAL_FRAME_SET_MESSAGE                   VideoVisualFrameSetMsg;
    ST_VIDEO_VISUAL_FRAME_SET_RET_MESSAGE               VideoVisualFrameSetRetMsg;
    ST_VIDEO_VISUAL_FRAME_APPLY_MESSAGE                 VideoVisualFrameApplyMsg;
    ST_VIDEO_VISUAL_FRAME_APPLY_RET_MESSAGE             VideoVisualFrameApplyRetMsg;
    ST_VIDEO_VISUAL_FRAME_FONT_SET_MESSAGE              VideoVisualFrameFontSetMsg;
    ST_VIDEO_VISUAL_FRAME_FONT_SET_RET_MESSAGE          VideoVisualFrameFontSetRetMsg;
    ST_VIDEO_VISUAL_FRAME_FONT_CLEAN_MESSAGE            VideoVisualFrameFontCleanMsg;
    ST_VIDEO_VISUAL_FRAME_FONT_CLEAN_RET_MESSAGE        VideoVisualFrameFontCleanRetMsg;
    ST_VIDEO_TIMESTAMP_REGION_SET_MESSAGE               VideoTimeStampRegionSetMsg;
    ST_VIDEO_TIMESTAMP_REGION_SET_RET_MESSAGE           VideoTimeStampRegionSetRetMsg;
    ST_VIDEO_TIMESTAMP_ENABLE_MESSAGE                   VideoTimeStampEnableMsg;
    ST_VIDEO_TIMESTAMP_ENABLE_RET_MESSAGE               VideoTimeStampEnableRetMsg;
    ST_VIDEO_TIMESTAMP_SET_ORDER_MESSAGE                VideoTimeStampSetOrderMsg;
    ST_VIDEO_PROGRESS_FRAME_BY_FRAME_MESSAGE            VideoProgressFramebyFrameMsg;
    ST_VIDEO_PROGRESS_FRAME_BY_FRAME_RET_MESSAGE        VideoProgressFramebyFrameRetMsg;
} UNION_VIDEO_API_MESSAGE;


typedef struct {
    U16 u16H;           ///< Image width
    U16 u16V;           ///< Image hight
} ST_IP_IMAGE_SIZE;
typedef ST_IP_IMAGE_SIZE ST_IP_IMAGE_POINT;

typedef struct
{
    U32 u32Event;
    U32 u32Len;
    U08 *data;
} ST_PIPE_MESSAGE;


#endif // __USBOTG_DEVICE_VAC_H

