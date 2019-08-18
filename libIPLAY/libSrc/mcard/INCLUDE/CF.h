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
* Filename      : cf.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef __CF_H
#define __CF_H
/*
// define
*/

enum
{
	INITIAL_MEMORY		= 0,
	INITIAL_TRUE_IDE,
	INITIAL_IO
};

// Follow CF spec. 4.1 Identify Device Information - WORD164
enum{
	TIMING_250ns  = 0,
	TIMING_120ns,
	TIMING_100ns,
	TIMING_80ns,
	TIMING_SLOWEST
};


/*
// Function prototype 
*/
BOOL Polling_CF_Status();
SWORD WaitReady();
SWORD CommandReady();
SWORD WriteSector(DWORD dwBufferAdress, WORD wSize);
SWORD ReadSector(DWORD dwBufferAdress, WORD wSize);
void SetCommand(BYTE bCommand);
void SetParameter(DWORD dwSectorCount, DWORD dwLogAddr);
void McardCFActive(BYTE bMode);
void McardCFInactive();
void CFCardReset();
void CalculatePulseTiming(BYTE speedLevel);
#endif  // __CF_H

