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
* Filename      : mjpeg_api.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __MJPEG_API_H
#define __MJPEG_API_H

#include "utiltypedef.h"
#include "display.h"

#ifndef __API_MAIN_H
#include "api_main.h"
#endif

/*
// Structure declarations
*/

typedef struct
{
    DWORD dwTotalSeconds;
	DWORD dwWidth;
	DWORD dwHeight;
}ST_MEDIA_INFO;

struct MJPEG_CONTROL_FUNC_TAG
{
    SWORD (*MjpegTypeCheck)(void);
    SWORD (*MjpegDisplayFirst)(void);
    SWORD (*MjpegInitial)(void);
    SWORD (*MJpegAudioPlay)(void);
    SWORD (*MJpegVideoPlay)(void);
    SWORD (*MJpegDataLoad)(void);
    SWORD (*MJpegPlay)(void);
    SWORD (*MJpegStop)(void);
    SWORD (*MJpegPause)(void);
    SWORD (*MJpegResume)(void);
    SWORD (*MJpegShift)(void);
    SWORD (*MJpegVideoDecode)(void);
    void (*MJpegRleaseBuf)(void);
	SWORD (*MJpegGetMediaInfo)(STREAM *sHandle, ST_MEDIA_INFO* p_media_info);
};

struct MJPEG_CONTROL_PARAMETER_TAG
{
    struct MJPEG_CONTROL_FUNC_TAG *stMJpegCtrlFunc;
    STREAM *stFileHandler;
    DWORD dwDispalyX;
    DWORD dwDisplayY;
    DWORD dwDisplayWidth;
    DWORD dwDisplayHeight;
    volatile DWORD dwCommand;
    volatile DWORD dwCurrentMode;
    DWORD *pdwDispalyBuffer;
    DWORD dwOffsetValue;
    DWORD dwParameter;//(bit0 = full screen flag)
    SWORD swStatus;
    BYTE  res[2];
    ST_MPX_STREAM *MPX_Stream;
    ST_MPX_STREAM *MPX_Stream_a;
    ST_JPEG  *psJpeg;
    
};



/*
// Type declarations
*/
typedef struct MJPEG_CONTROL_FUNC_TAG  ST_MJPEG_CONTROL_FUNC;
typedef struct MJPEG_CONTROL_PARAMETER_TAG ST_MJPEG_CONTROL_PARAMETER;

extern ST_MJPEG_CONTROL_PARAMETER stMjpegCtrl;

#define MjpegGetFileHandle() (stMjpegCtrl.stFileHandler)
#define MjpegGetDispBuf() (stMjpegCtrl.pdwDispalyBuffer)
#define MjpegGetDispX() (stMjpegCtrl.dwDispalyX)
#define MjpegGetDispY() (stMjpegCtrl.dwDisplayY)
#define MjpegGetDispWidth() (stMjpegCtrl.dwDisplayWidth)
#define MjpegGetDispHeight() (stMjpegCtrl.dwDisplayHeight)
#define MjpegGetDispOffset() (stMjpegCtrl.dwOffsetValue)
#define MjpegGetCurrentMode() (stMjpegCtrl.dwCurrentMode)
#define MjpegGetCmd() (stMjpegCtrl.dwCommand)
#define MjpegGetParam() (stMjpegCtrl.dwParameter)
#define MjpegSetParam(a) (stMjpegCtrl.dwParameter = a)
#define MjpegSetStatus(a) (stMjpegCtrl.swStatus = a)
#define MjpegSetDispX(a) (stMjpegCtrl.dwDispalyX = a)
#define MjpegSetDispY(a) (stMjpegCtrl.dwDisplayY = a)
#define MjpegSetDispWidth(a) (stMjpegCtrl.dwDisplayWidth = a)
#define MjpegSetDispHeight(a) (stMjpegCtrl.dwDisplayHeight = a)
#define MjpegSetDispBuf(a) (stMjpegCtrl.pdwDispalyBuffer = a)
#define MjpegSetCurrentMode(a) (stMjpegCtrl.dwCurrentMode = a)

#define MjpegGetMPXStream() (stMjpegCtrl.MPX_Stream)
#define MjpegGetMPXStream_a() (stMjpegCtrl.MPX_Stream_a)
#define MjpegGetpsJpeg()     (stMjpegCtrl.psJpeg)

SDWORD MjpegTaskInit (void);
SWORD MjpegPlayerOpen (
	STREAM *stHandle, 
	BYTE bType, 
	ST_IMGWIN *stDispWin, 
	DWORD dwDispX, 
	DWORD dwDispY, 
	DWORD dwDispWidth, 
	DWORD dwDispHeight, 
	DWORD dwCmd);

extern SWORD MjpegSwapDispSize (void);
extern SWORD MjpegShowFirstFrame (void);
extern SWORD MjpegPlayerForward (void);
extern SWORD MjpegPlayerBackward (void);
extern SWORD MjpegPlayerClose (void);
extern SWORD MjpegPlayerPlay (void);
extern SWORD MjpegPlayerPause (void);
extern SWORD MjpegPlayerResume (void);
extern SWORD MjpegPlayerStop (void);
extern SWORD MjpegGetMediaInfo(STREAM *sHandle, BYTE bType, ST_MEDIA_INFO* p_media_info);

void MjpegPlayerDispTotalTime (DWORD dwTotalSeconds);
void MjpegPlayerDispTime (DWORD dwFrameCount, DWORD dwFrameRate, BYTE blRedraw);
SWORD MjpegPlayerAudioFillBlock (BYTE bType, DWORD* Dwreadsize,DWORD* bwordalign);
#endif //__MJPEG_API_H  

