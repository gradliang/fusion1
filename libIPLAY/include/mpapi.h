/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpapi.h
*
* Programmer:    Joshua Lu
*                MPX E120 division
*
* Created: 	 03/30/2005
*
* Description:
*
*
* Change History (most recent first):
*     <1>     03/30/2005    Joshua Lu    first file
****************************************************************
*/

#ifndef MPAPI_H
#define MPAPI_H

#include "flagdefine.h"
#include "display.h"
#include "fs.h"
#include "avTypeDef.h"
#include "../../libIPLAY/libsrc/mpapi/include/ImageFile.h"

#if 0
#define EVENT_ENTER_MOVIE_PLAYER BIT1
#define EVENT_EXIT_MOVIE_PLAYER  BIT2
#define EVENT_MOVIE_PLAY 		 BIT3

// the following definitions are used for indicate whether relative member of Media_Infor is useful  //Joshus & Meng 2005.6.9
#define MOVIE_TotalTime_USEFUL				1<<0

//video special
#define MOVIE_ImageWidth_USEFUL				1<<1
#define MOVIE_ImageHeight_USEFUL			1<<2
#define MOVIE_TotalFrame_USEFUL				1<<3
#define MOVIE_FrameRate_USEFUL				1<<4
//video content
#define MOVIE_Title_USEFUL					1<<5
#define MOVIE_Author_USEFUL					1<<6
#define MOVIE_Copyright_USEFUL				1<<7
#define MOVIE_Rating_USEFUL					1<<8
#define MOVIE_Description_USEFUL			1<<9


//audio special
#define MOVIE_Bitrate_USEFUL				1<<10
#define MOVIE_SampleRate_USEFUL				1<<11
#define MOVIE_SampleSize_USEFUL				1<<12

//audio content
#define MOVIE_ID3_Title_USEFUL				1<<13
#define MOVIE_ID3_Artist_USEFUL				1<<14
#define MOVIE_ID3_Album_USEFUL				1<<15
#define MOVIE_ID3_Year_USEFUL				1<<16
#define MOVIE_ID3_Comment_USEFUL			1<<17
#define MOVIE_ID3_Track_USEFUL				1<<18
#define MOVIE_ID3_Genre_USEFUL				1<<19


//whether has video/audio relative infor
#define MOVIE_INFO_WITH_AUDIO				1<<30
#define MOVIE_INFO_WITH_VIDEO				1<<31

#define MOVIE_VIDEO_FF_FRAMES				10	//video FF frames
#define MOVIE_AUDIO_FF_SECONDS				2	//audio FF seconds
#define MOVIE_VIDEO_FB_FRAMES				-100	//video FF frames
#define MOVIE_AUDIO_FB_SECONDS				-6	//audio FF seconds
#endif

void MovieTaskCreate();
int EnterMoviePlayer (STREAM * sHandle, STREAM * sLyricHandle, unsigned char *extension, ST_IMGWIN * screen_win, ST_IMGWIN * movie_win, int subtitle_sync_align);
void MoviePlay();
void MovieVideoFullScreen();
void MovieStop();
float MovieBackward(BYTE flag);
float MovieForward(BYTE flag);
void MoviePause();
void MovieResume();
void ExitMoviePlayer();
int MovieSeek(float pts,int flags);
int MovieShowFirstFrame();
//BYTE MovieCheckEof();
//int MoviePreParsing();
int VideoDrawThumb(STREAM *sHandle, BYTE *bExt,  ST_IMGWIN *pWin, DWORD x, DWORD y,  DWORD w, DWORD h);
int MovieGetMediaInfo(FILE_TYPE_T enFileType , BYTE *byInfoBuf);
int MovieOpenFileAndGetMediaInfo(STREAM *sHandle, FILE_TYPE_T enFileType , BYTE *byInfoBuf);
void WaitMs(DWORD ms);
void IODelay(WORD nDelay);
void TimerDelay(DWORD dwDelayTime);

int GetTotalVideoFrames(void);
int GetMissedVideoFrames(void);
int GetVideoDisplayPTS(void);
void GetVideoCodecCategory(char* const p_video_codec);
void GetAudioCodecCategory(char* const p_audio_codec);

void MpMemCopy(BYTE *, BYTE *, DWORD);
void MpMemSet(BYTE *tar, BYTE value, DWORD length);
void MpMemSetDWORD(DWORD *tar, DWORD value, DWORD byteCount);

#endif // MPAPI_H

