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
* Filename      : dir.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/
#ifndef __DIR_H
#define __DIR_H

#include "utiltypedef.h"
#include "fat.h"


int    UpdateChain(DRIVE * drv);
int    CdParent(DRIVE * drv);
int    CdSub(DRIVE * drv);
int    MakeDir(DRIVE * drv, BYTE * name, BYTE * extension);
int    MakeDir_UTF16(DRIVE * drv, WORD * name);
int    DeleteDir(DRIVE * drv);
int    DeleteAllContentWithinDir(DRIVE * drv);
int    DirFirst(DRIVE * drv);
int    DirLast(DRIVE * drv);
int    DirNext(DRIVE * drv);
int    DirPrevious(DRIVE * drv);
int    DirReset(DRIVE * drv);
int    ScanFileName(DRIVE * drv);
int    Unlink(DRIVE * drv);
int    GetNewNode(DRIVE * drv);
int    GetDeletedNode(DRIVE * drv);
void   CheckDeletedCount(DWORD dwCount);
int    DirCaching(DRIVE * drv);
int    DirCaching_for_DirChain(DRIVE * drv, CHAIN * dir);
int    FileDirCaching(STREAM * sHandle);
DWORD  GetRootDirSize(DRIVE * drv);


#endif //__DIR_H
