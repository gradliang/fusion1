/**
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
* @file api_main.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
///
///@mainpage Media Player API User Guide
///The Media Player is responsible for all video and audio files playing.
///The iPlay Media Player API is designed by MagicPixel for MagicPixel's IC.
///All rights are reserved for MagicPixel.
///
///Specification:
///
///  Video codec:
///    H.264           : 720P,30fps
///    MPEG-4          : 720P,30fps
///    MPEG-2          : D1,30fps
///    MPEG-1          : 720P,30fps
///    H.263           : 720P,30fps
///    Sorenson-Spark  : 720P,30fps
///    Motion-JPEG     : 720P,30fps
///
///  Audio codec:
///    PCM/ADPCM
///    MP3
///    AAC
///    WMA
///    AC3
///    AMR
///    OGG
///    RAM
///
///  Diagram
///@image html av_flow.gif

///
///@defgroup group_MediaPlayer_AV AV API
///

///
///@defgroup group_MediaPlayer_Audio Aideo API
///

///
///@defgroup group_MediaPlayer_Video Video API
///

#ifndef __API_MAIN_H
#define __API_MAIN_H

#include "xpg.h"
#include "../../demux/include/Demux_types.h"
#include "record.h"
//index
enum
{
	FILE_EXT_NULL = 0,
	FILE_EXT_RA,
	FILE_EXT_RM,
	FILE_EXT_RAM,
	FILE_EXT_OGG,
	FILE_EXT_WMA,
	FILE_EXT_WAV,
	FILE_EXT_AAC,
	FILE_EXT_M4A,
	FILE_EXT_AC3,
	FILE_EXT_MP3,
	FILE_EXT_AMR,
	FILE_EXT_AVI,
	FILE_EXT_MOV,
	FILE_EXT_MAX_COUNT
};

/*
// Structure declarations
*/


///@ingroup group_MediaPlayer_Video
///@brief   This function will play video using full screen mode.
///@param   None
///
///@return  None
///
///@remark
///
extern void  Api_MovieVideoFullScreen (void);

///@ingroup group_MediaPlayer_Video
///@brief   This function will do fast forward for video files.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MovieForward (DWORD sec);

///@ingroup group_MediaPlayer_Video
///@brief   This function will do fast backward for video files.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MovieBackward (DWORD sec);

///@ingroup group_MediaPlayer_Audio
///@brief   Backward progress of current audio playback
///@param   None
///
///@return  Always return PASS at PLAYER status(Should be enhanced at future)
///
///@remark
///
extern SWORD Api_AudioBackward (DWORD sec);

///@ingroup group_MediaPlayer_Audio
///@brief   Forward  progress of current audio playback
///@param   None
///
///@return  Always return PASS at PLAYER status(Should be enhanced at future)
///
///@remark
///
extern SWORD Api_AudioForward (DWORD sec);

///@ingroup group_MediaPlayer_Audio
///@brief   Get Equalizer information
///@return  EQ data structure inside our decoder
///         unsigned char mp3_EQ_band[16];                          \n
///         If there is no equalizer information, return NULL       \n
///         Ex:	for(a=0; a<16; a++)                                 \n
///                 mpDebugPrint("EQ %d value: %x", a, *(AUDIO_EQ_INFO + a));
///
extern BYTE *Api_GetEQInfo();

///@ingroup group_MediaPlayer_Audio
///@brief   Turn on hardware audio dac. You should call this function before starting to use audio.
///@param   None
///
///@return  Always return PASS(Should be enhanced at future)
///
///@remark
///
extern inline SWORD Api_AudioHWEnable(void);

///@ingroup group_MediaPlayer_Audio
///@brief   Turn off hardware audio dac. You should call this function after using audio for saving power.
///@param   None
///
///@return  Always return PASS(Should be enhanced at future)
///
///@remark
///
extern inline SWORD Api_AudioHWDisable(void);

///@ingroup group_MediaPlayer_AV
///@brief   This function will start to play media file.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MoviePlay (void);


///@ingroup group_MediaPlayer_AV
///@brief   This function will pause the media file during playing.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MoviePause (void);


///@ingroup group_MediaPlayer_AV
///@brief   This function will resume the paused media file to play.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MovieResume (void);

///@ingroup group_MediaPlayer_AV
///@brief   This function will stop the media file being played.
///@param   None
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern SWORD Api_MovieStop (void);

///@ingroup group_MediaPlayer_AV
///@brief   This function will initialize the media play according to the media type
///         before starting media play.
///@param   STREAM *sHandle            The media file STREAM handle.
///@param   BYTE *bExt                 The file extension of the media file.
///@param   STREAM *sLyricHandle       The lyric file STREAM handle.
///@param   ST_IMGWIN *stScreenWin     The previw screen window.
///@param   ST_IMGWIN *stMovieWin      The video window.
///@param   XPG_FUNC_PTR* xpg_func_ptr The function pointers used by media player
///@param   BYTE play_mode The playing mode used by media player
///@param   int subtitle_sync_align Subtitle sync. alignment. The unit of alignment is ms.
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern int   Api_EnterMoviePlayer (STREAM *sHandle, BYTE *bExt, STREAM *sLyricHandle, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin, XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode, int subtitle_sync_align);

///@ingroup group_MediaPlayer_AV
///@brief   This function will exit the media player after media file stopped.
///@param   None
///
///@return  None
///
///@remark
///
extern void  Api_ExitMoviePlayer (void);

///@ingroup group_MediaPlayer_Video
///@brief   This function will draw a preview window for a video file.
///@param   STREAM *sHandle            The media file STREAM handle.
///@param   BYTE *bExt                 The file extension of the media file.
///@param   ST_IMGWIN *stScreenWin     The previw screen window.
///@param   ST_IMGWIN *stMovieWin      The video window.
///@param   DWORD dwX                  The horizontal position of the left-top corner or the preview window.
///@param   DWORD dwY                  The vertical position of the left-top corner or the preview window.
///@param   DWORD dwWidth              The width of the preview window.
///@param   DWORD dwHeight             The height of the preivew window.
///@param   XPG_FUNC_PTR* xpg_func_ptr The function pointers used by media player.
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
extern int   Api_EnterMoviePreview (STREAM *sHandle, BYTE *bExt, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                            DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr);

///@ingroup group_MediaPlayer_Video
///@brief   This function will draw a thumb window for a video file.
///@param   STREAM *sHandle            The media file STREAM handle.
///@param   BYTE *bExt                 The file extension of the media file.
///@param   ST_IMGWIN *stScreenWin     The previw screen window.
///@param   ST_IMGWIN *stMovieWin      The video window.
///@param   DWORD dwX                  The horizontal position of the left-top corner or the preview window.
///@param   DWORD dwY                  The vertical position of the left-top corner or the preview window.
///@param   DWORD dwWidth              The width of the preview window.
///@param   DWORD dwHeight             The height of the preivew window.
///@param   XPG_FUNC_PTR* xpg_func_ptr The function pointers used by media player.
///@param   BYTE play_mode The playing mode used by media player.
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
SWORD Api_VideoDrawThumb(STREAM *sHandle, BYTE *bExt, ST_IMGWIN *stScreenWin, ST_IMGWIN *stMovieWin,
                            DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, XPG_FUNC_PTR* xpg_func_ptr, BYTE play_mode);

///@ingroup group_MediaPlayer_Video
///@brief   This function will return video information.
///@param   STREAM *sHandle              The media file STREAM handle.
///@param   BYTE *bExt                   The file extension of the media file.
///@param   Media_Info* p_media_info  The information about the video file,
///                                      includes total time, resolution.
///
///@return  PASS for success, FAIL for fail
///
///@remark
///
int Api_MovieGetMediaInfo(STREAM *sHandle, BYTE *bExt, Media_Info* p_media_info);

DWORD Api_MovieStatusGet(void);

/**
 *  @ingroup  group_MediaPlayer_Audio
 *  @brief    Enable sound of left channel or right channel
 *  @param    left_temp  Enable left channel
 *  @param    right_temp Enable right channel
 */
extern void Api_audio_channel_select(int left_temp,int right_temp);

extern void Api_Audio_getID3_pic(int *pic_size, int *offset, STREAM *handle);
void AiuDmaRegCallBackFunc(void (*AiuDmaCallBack_function) (void));
void AiuDmaClearCallBackFunc(void);
/**
* if resolution set to 0, display panel full size
* if resolution set to 1, display origne size
* if resolution set to 2, display 16:9
* if resolution set to 3, display 4:3
*/
int Api_ChangeDisplayRatio(int resolution);

#if RECORD_AUDIO
int Api_AudioRecording(record_argument *p);
int Api_StopAudioRecording(void);
#endif

#if RECORD_ENABLE
int Api_VideoRecording(record_argument *p);
int Api_StopVideoRecording(void);
int Api_VideoRecordingPause(void);
int Api_VideoRecordingResume(void);
BYTE Api_SetRecordVolume(WORD ivol);
SDWORD Api_VideoRecordingPreviewStart(record_argument *p);
SDWORD Api_VideoRecordingPreviewStop(void);
//SDWORD Api_VideoRecordingGetPicture(STREAM *shandle_jpeg);

#endif

#endif //__API_MAIN_H

