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
* Filename      : mjpeg_mov.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/

#ifndef __MJPEG_MOV_H
#define __MJPEG_MOV_H

#ifndef __MJPEG_API_H
#include "mjpeg_api.h"
#endif

extern void Mov_ReleaseBuf(void);
extern SWORD Mov_TypeCheck (void);
extern SWORD Mov_DisplayFirstFrame (void);
extern SWORD Mov_playInit (void);
extern SWORD Mov_ScreenRefresh (void);
extern SWORD Mov_AudioChunkConnect (void);
extern SWORD Mov_DataLoad (void);
extern SWORD Mov_Play (void);
extern SWORD Mov_Stop (void);
extern SWORD Mov_Pause (void);
extern SWORD Mov_Resume (void);
extern SWORD Mov_Shift (void);
extern SWORD Mov_VideoDecode (void);
extern SWORD Mov_GetMediaInfo(STREAM *sHandle, ST_MEDIA_INFO* p_media_info);
#endif  //__MJPEG_MOV_H

