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
* Filename      : chain.h
* Programmers   : 
* Created       : 
* Descriptions  : 
*******************************************************************************
*/
#ifndef __CHAIN_H
#define __CHAIN_H

#include "utiltypedef.h"

int    ChainChangeSize(DRIVE * drv, CHAIN * chain, DWORD size);
int    ChainCopy(DRIVE * target, CHAIN * trgChain, DRIVE * source, CHAIN * srcChain, DWORD bufaddress, DWORD size);
int    ChainExtending(DRIVE * drv, CHAIN * chain);
int    ChainFree(DRIVE * drv, CHAIN * chain);
int    ChainFreeToReserveStartCluster(DRIVE * drv, CHAIN * chain);
DWORD  ChainGetContinuity(DRIVE * drv, CHAIN * chain, DWORD * request);
DWORD  ChainGetLba(DRIVE * drv, CHAIN * chain);
void   ChainInit(CHAIN * chain, DWORD start, DWORD size);
int    ChainRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector);
int    ChainWrite(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD sector);
int    ChainSeek(DRIVE * drv, CHAIN * chain, DWORD position);
void   ChainSeekSet(CHAIN * chain);
void   ChainSeekEnd(DRIVE * drv, CHAIN * chain);
int    ChainSeekForward(DRIVE * drv, CHAIN * chain, DWORD distance);
int    SectorClear(DRIVE * drv, DWORD start, SDWORD count);
int    ChainFragmentWrite(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size);
int    ChainFragmentRead(DRIVE * drv, CHAIN * chain, BYTE * buffer, DWORD size);
DWORD  GetChainTotalSizeByTraverseFatTable(DRIVE * drv, DWORD start_clus);
BOOL   Is_ChainPoint_EOC_and_At_Boundary(DRIVE * drv, CHAIN * chain);

#endif //__CHAIN_H

