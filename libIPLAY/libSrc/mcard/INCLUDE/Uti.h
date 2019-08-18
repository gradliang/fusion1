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
* Filename      : uti.h
* Programmer(s) : 
* Created       : 
* Descriptions  :
*******************************************************************************
*/
#ifndef	__UTI_H
#define __UTI_H

/*
// Include section 
*/
/*
// Constant declarations
*/
#define MCARD_ENABLE            0x00000001
#define MCARD_FLASH_ENABLE      BIT0 //// MCARD_FLASH_ENABLE is dedicated to XD,NAND,CF,SM 
#define MCARD_SD_ENABLE			BIT16
#define MCARD_MS_ENABLE			BIT17
#define MCARD_CF_ENABLE			BIT18
//#define MCARD_SM_ENABLE			BIT19

#define MCARD_FIFO_ENABLE       0x00000002
#define MCARD_DMA_CF            0x00000000
#define MCARD_DMA_SM            0x00000004
#define MCARD_DMA_MMC           0x00000008
#define MCARD_DMA_SD            0x0000000c
#define MCARD_DMA_DIR_MC        0x00000000
#define MCARD_DMA_DIR_CM        0x00000010
#define MCARD_MS_P_MODE         0x00000020
#define MCARD_MS_S_MODE         0x00000060
#define MCARD_PHASE_INV         0x00000080
#define MCARD_ATA_ENABLE        0x00000100
#define MCARD_FIFO_EMPTY        0x00000400
#define MCARD_DMA_LIMIT_ENABLE  0x00800000
#define MCARD_WDT_ENABLE        0x00200000
#define MCARD_SD_MODULE_ENABLE	0x00010000

#define MCARD_DMA_ENABLE        0x00000001
#define MCARD_INT_DBFEND_ENABLE 0x01000000

#define MCARD_DIR_MASK			(1<<4)

#define MCARD_SECTOR_SIZE		0x200
#define MCARD_SECTOR_SIZE_EXP	9

#define MCARD_RETRY_TIME    3

#define TIME_PERIOD(now, since) ((now>=since)?(now-since):(0xffffffff-since+now+1))

enum
{
    PIN_DEFINE_FOR_SD	= 0,
    PIN_DEFINE_FOR_SD2,
    PIN_DEFINE_FOR_MMC,
    PIN_DEFINE_FOR_MS,
    PIN_DEFINE_FOR_NAND,
    PIN_DEFINE_FOR_XD,
    PIN_DEFINE_FOR_CFM,
    PIN_DEFINE_FOR_SPI,
    MAX_MCARD_PIN_DEFINE
};

/*
// Structure declarations
*/


/*
// Type declarations
*/

/*
// Macro declarations
*/		//test MS delay time   //          DWORD i, j; //
#define McardIODelay(DelayCount)                        \
        {                                               \
          volatile DWORD i, j;                          \
          for (i = 0; i < DelayCount; i++)              \
            for (j = 0; j < DelayCount; j++);           \
        }

/*
// Function prototype 
*/
// for debug
void MemDump(BYTE *ptr, DWORD num);

void GeneratePLLClockTab(DWORD pll1, DWORD pll2);

void SetMcardClock(DWORD freq);

void SetSD2Clock(DWORD freq);


void McardFgpioSet(BYTE pintype, BYTE nr, const BYTE *pins);

void McardSelect(DWORD type);

void McardDeselect(DWORD type);


#endif	// __UTI_H

