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
* Filename      : mjpeg_avi.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/

#ifndef __MJPEG_AVI_H
#define __MJPEG_AVI_H

#ifndef __MJPEG_API_H
#include "mjpeg_api.h"
#endif

extern void Avi_ReleaseBuf(void);
extern SWORD Avi_TypeCheck (void);
extern SWORD Avi_DisplayFirstFrame (void);
extern SWORD Avi_playInit (void);
extern SWORD Avi_ScreenRefresh (void);
extern SWORD Avi_AudioChunkConnect (void);
extern SWORD Avi_DataLoad (void);
extern SWORD Avi_Play (void);
extern SWORD Avi_Pause (void);
extern SWORD Avi_Resume (void);
extern SWORD Avi_Stop (void);
extern SWORD Avi_Shift (void);
extern SWORD Avi_VideoDecode(void);
extern SWORD Avi_GetMediaInfo(STREAM *sHandle, ST_MEDIA_INFO* p_media_info);
#endif  //__MJPEG_AVI_H

