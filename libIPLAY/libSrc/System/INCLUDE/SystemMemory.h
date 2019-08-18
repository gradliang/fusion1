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
* Filename      : SystemMemory.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : System memory mapping
*******************************************************************************
*/
#ifndef __SYSTEM_MEMORY_H
#define __SYSTEM_MEMORY_H

///////////////////////////////////////////////////////////////////////////
//
//  Include section
//
///////////////////////////////////////////////////////////////////////////
#include "iplaysysconfig.h"


///////////////////////////////////////////////////////////////////////////
//
// Constant declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  memory ID
//
///////////////////////////////////////////////////////////////////////////
enum {
    MAIN_PROC_MEM_ID = 0,
    OS_BUF_MEM_ID,
    DISPLAY_BUF1_MEM_ID,
    DISPLAY_BUF2_MEM_ID,
    OSD_BUF_MEM_ID,
#if OSD_DISPLAY_CACHE
    OSD_BUF_MEM_ID1,
#endif
    USB_OTG_BUF_MEM_ID,
    USB_OTG1_BUF_MEM_ID,
    XPG_BUF_MEM_ID,
    AV_BUF_MEM_ID,
    JPEG_SOURCE_MEM_ID,
    JPEG_TARGET_MEM_ID,

    //FONT_BUF_MEM_ID,
    HEAP_BUF_MEM_ID,
    NETPOOL_BUF_ID,

    //STRING_BUF_MEM_ID,
    KEY_SOUND_BUF_MEM_ID,

	FONT_TTF_BUF_MEM_ID,

#if RESET_EXCEPTION
    BACKUP_BOOT_BUF_ID,
#endif

    //////////////////////////////////////////
    // Keep it at last.
    MEM_COUNT
};

///////////////////////////////////////////////////////////////////////////
//
//  defines
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Enum defines
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Structure declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  grobal variables
//
///////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////
//
//  functions
//
///////////////////////////////////////////////////////////////////////////
DWORD SystemGetMemAddr(BYTE bMemID);
SWORD SystemSetMemAddr(BYTE bMemID, DWORD dwAddr);
DWORD SystemGetMemSize(BYTE bMemID);
SWORD SystemSetMemSize(BYTE bMemID, DWORD dwSize);
DWORD SystemMemoryMapInit(void);
DWORD SystemMemoryAllocAlignDW(DWORD allocSize, DWORD dwNumber, DWORD *orginalAddress);

#endif  //__SYSTEM_MEMORY_H

