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
* Filename      : imageplayer.h
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/
#ifndef __IMAGE_PLAYER_H
#define __IMAGE_PLAYER_H

/*
// Include section
*/
/*
// Variable declarations
*/

/*
// Constant declarations
*/

/*
// Static function prototype
*/

/*
// Definition of external functions
*/
extern ST_IMGWIN *ImageGetDecodeTargetWin();
extern SWORD ImageDraw (ST_IMGWIN *pWin, BYTE blClearFlag);
int ImageDraw_Decode(ST_IMGWIN * pWin, BYTE blSlideShow);
DWORD ImageGetSourceSize();

#endif //__IMAGE_PLAYER_H


