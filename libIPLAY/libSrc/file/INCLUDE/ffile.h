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
* Filename      : ffile.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/

#ifndef __FFILE_H
#define __FFILE_H


extern DWORD Fstream_FileRead(STREAM * handle, BYTE * buffer, DWORD size);
extern SWORD Fstream_Seek(STREAM * handle, DWORD position);
#if ENABLE_VIDEO_FILE_CHAIN_CLUSTERS_CACHING 
extern SWORD InitPreload_FileChainClustersCache(STREAM * handle);
extern void Clear_FileChainClustersCache(void);
#endif


#endif  //__FFILE_H
