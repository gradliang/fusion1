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
* Filename      : SystemMemory.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "mpTrace.h"
#include "SystemConfig.h"
#include "SystemMemory.h"

#include "taskid.h"
#include "os.h"
#include "peripheral.h"
//#include "bios.h"


///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////
#define __RECORD_DATA_LENGTH__              4           // DWORD


///////////////////////////////////////////////////////////////////////////
//
//  Static function prototype
//
///////////////////////////////////////////////////////////////////////////
static DWORD dwSystemMemStartAddr = 0;
static DWORD dwSystemMemCurrSize = 0;
static DWORD dwSystemMemorySize[MEM_COUNT] = {0};
static DWORD dwSystemMemoryMap[MEM_COUNT] = {0};
static DWORD dwSystemMemoryRealSize;

#ifdef __MONITOR_RESERVED_MEMORY_REGION
static DWORD dwSystemReservedData[__RECORD_DATA_LENGTH__];
#endif

///////////////////////////////////////////////////////////////////////////
//
//  Definition of static functions
//
///////////////////////////////////////////////////////////////////////////
static DWORD systemMemAlloc(BYTE bID, DWORD dwSize)
{
    DWORD dwAddr = dwSystemMemCurrSize;

    if (dwSize & 0x1f) dwSize += 0x20 - (dwSize & 0x1f);

    if (dwSystemMemCurrSize + dwSize > dwSystemMemoryRealSize)
    {
        MP_ALERT("-E- System Memory overflow");
        dwSize = dwSystemMemoryRealSize - dwAddr;
    }

    dwSystemMemCurrSize += dwSize;
    dwSystemMemorySize[bID] = dwSize;

    return dwSystemMemStartAddr + dwAddr;
}




///////////////////////////////////////////////////////////////////////////
//
//  Definition of grobal functions
//
///////////////////////////////////////////////////////////////////////////
DWORD SystemGetMemAddr(BYTE bMemID)
{
    return dwSystemMemoryMap[bMemID];
}



SWORD SystemSetMemAddr(BYTE bMemID, DWORD dwAddr)
{
    dwSystemMemoryMap[bMemID] = dwAddr;
}



DWORD SystemGetMemSize(BYTE bMemID)
{
    return dwSystemMemorySize[bMemID];
}



SWORD SystemSetMemSize(BYTE bMemID, DWORD dwAddr)
{
    dwSystemMemorySize[bMemID] = dwAddr;
}

#define __MAX_TEST_DRAM_ADDRESS__       0x04000000          // 64MB
#define __START_TEST_DRAM_ADDRESS__     0x00800000          // 8MB
#define __STEST_DRAM_DATA_PATTERN__     0x55AA00FF

#ifndef MEMORY_SIZE_AUTO_DETECT
    #define MEMORY_SIZE_AUTO_DETECT     1
#endif

DWORD SystemMemoryMapInit(void)
{
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
    ST_TCON *pstTCON = pstScreen->pstCurTCON;
    BYTE *pUserSpace;
    DWORD dwMainProcSize = (GetUserBlkStart() + 0x1000) & 0x0FFFF000;
    DWORD readbackData, nextAddr;

    IntDisable();
#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
    #if (MEMORY_SIZE_AUTO_DETECT == 1)
    *(volatile DWORD *) (BIT29 + __MAX_TEST_DRAM_ADDRESS__ - 4) = __STEST_DRAM_DATA_PATTERN__;

    for (nextAddr = __START_TEST_DRAM_ADDRESS__; nextAddr <= __MAX_TEST_DRAM_ADDRESS__; nextAddr <<= 1)
    {
        readbackData = *(volatile DWORD *) (BIT29 | (nextAddr - 4));
        //MP_ALERT("Address = 0x%08X, Data = 0x%08X", nextAddr, readbackData);

        if (readbackData == __STEST_DRAM_DATA_PATTERN__)
        {
            dwSystemMemoryRealSize = nextAddr;
            break;
        }
    }

    if (nextAddr > __MAX_TEST_DRAM_ADDRESS__)
        dwSystemMemoryRealSize = MEMORY_SIZE;
    #else
    dwSystemMemoryRealSize = MEMORY_SIZE;
    #endif
#else
    dwSystemMemoryRealSize = MEMORY_SIZE;
#endif

    MP_ALERT("Memory Size is %02dMB.", dwSystemMemoryRealSize / 0x00100000);
    MP_ALERT("dwMainProcSize %dKB", dwMainProcSize / 1024);
#if RESET_EXCEPTION
	DWORD i,dwBootSize,dwCopySize,*pdwSAddr,*pdwTAddr;

	dwBootSize=64*1024;
    pUserSpace = (BYTE *) (0x80000000 | (dwMainProcSize + OS_BUF_SIZE+dwBootSize));
	pdwSAddr=(DWORD *)0x80700000;
	pdwTAddr=(DWORD *)(0x80000000 | (dwMainProcSize + OS_BUF_SIZE));
	dwCopySize=(dwBootSize>>2);
	for ( i=0;i<dwCopySize;i++)
		pdwTAddr[i]=pdwSAddr[i];


    memset(pUserSpace, 0, dwSystemMemoryRealSize - (dwMainProcSize + OS_BUF_SIZE+dwBootSize));

#else
    pUserSpace = (BYTE *) (0x80000000 | (dwMainProcSize + OS_BUF_SIZE));
    memset(pUserSpace, 0, dwSystemMemoryRealSize - (dwMainProcSize + OS_BUF_SIZE));
#endif

    dwSystemMemStartAddr = 0x80000000;
    dwSystemMemCurrSize = 0;
#ifdef __MONITOR_RESERVED_MEMORY_REGION
    memset((BYTE *) dwSystemMemStartAddr, 0xFE, __RECORD_DATA_LENGTH__ * sizeof(DWORD));
    memcpy((BYTE *) dwSystemReservedData, (BYTE *) dwSystemMemStartAddr, __RECORD_DATA_LENGTH__ * sizeof(DWORD));
    mem_ReservedRegionInfoSet(__RECORD_DATA_LENGTH__, (DWORD *) dwSystemReservedData);
#endif

    dwSystemMemoryMap[MAIN_PROC_MEM_ID]         = systemMemAlloc(MAIN_PROC_MEM_ID, dwMainProcSize);
    MP_ALERT("MAIN_PROC_MEM_ID %dKB", dwMainProcSize / 1024);
    dwSystemMemoryMap[OS_BUF_MEM_ID]            = systemMemAlloc(OS_BUF_MEM_ID, OS_BUF_SIZE);
    MP_ALERT("OS_BUF_MEM_ID %dKB", OS_BUF_SIZE / 1024);

#if RESET_EXCEPTION
    dwSystemMemoryMap[BACKUP_BOOT_BUF_ID]      = systemMemAlloc(BACKUP_BOOT_BUF_ID, dwBootSize);
#endif

#if (SC_USBHOST | SC_USBDEVICE)
    dwSystemMemoryMap[USB_OTG_BUF_MEM_ID]       = systemMemAlloc(USB_OTG_BUF_MEM_ID, (SC_USBOTG_0) ? USB_OTG_BUF_SIZE : 0);
    MP_ALERT("USB_OTG_BUF_MEM_ID %dKB", SystemGetMemSize(USB_OTG_BUF_MEM_ID) / 1024);
    dwSystemMemoryMap[USB_OTG1_BUF_MEM_ID]      = systemMemAlloc(USB_OTG1_BUF_MEM_ID, (SC_USBOTG_1) ? USB_OTG_BUF_SIZE : 0);
    MP_ALERT("USB_OTG1_BUF_MEM_ID %dKB", SystemGetMemSize(USB_OTG1_BUF_MEM_ID) / 1024);
#endif

    //dwSystemMemoryMap[STRING_BUF_MEM_ID]        = systemMemAlloc(STRING_BUF_MEM_ID, STRING_BUF_SIZE);
    //MP_ALERT("STRING_BUF_MEM_ID %dKB", SystemGetMemSize(STRING_BUF_MEM_ID) / 1024);
    //dwSystemMemoryMap[FONT_BUF_MEM_ID]          = systemMemAlloc(FONT_BUF_MEM_ID, FONT_MEMORY_BUF ? FONT_BUF_SIZE : 0);
    //MP_ALERT("FONT_BUF_MEM_ID %dKB", SystemGetMemSize(FONT_BUF_MEM_ID) / 1024);
    dwSystemMemoryMap[XPG_BUF_MEM_ID]           = systemMemAlloc(XPG_BUF_MEM_ID, XPG_BUF_SIZE);
    MP_ALERT("XPG_BUF_MEM_ID %dKB", SystemGetMemSize(XPG_BUF_MEM_ID) / 1024);
#if OSD_ENABLE
    dwSystemMemoryMap[OSD_BUF_MEM_ID]           = systemMemAlloc(OSD_BUF_MEM_ID, OSD_ENABLE ? ((pstTCON->wWidth * pstTCON->wHeight) >> OSD_BIT_OFFSET) : 0);
    if (dwSystemMemoryMap[OSD_BUF_MEM_ID])
        dwSystemMemoryMap[OSD_BUF_MEM_ID] |= BIT29;
    MP_ALERT("OSD_BUF_MEM_ID %dKB", SystemGetMemSize(OSD_BUF_MEM_ID) / 1024);
#if OSD_DISPLAY_CACHE
    dwSystemMemoryMap[OSD_BUF_MEM_ID1]  = systemMemAlloc(OSD_BUF_MEM_ID1, SystemGetMemSize(OSD_BUF_MEM_ID));
#endif
#endif

#if NETWARE_ENABLE
    dwSystemMemoryMap[NETPOOL_BUF_ID]           = (systemMemAlloc(NETPOOL_BUF_ID, NETPOOL_BUF_SIZE));
    MP_ALERT("Network buffer pool %08x %08x", dwSystemMemoryMap[NETPOOL_BUF_ID], dwSystemMemoryMap[NETPOOL_BUF_ID]);
#endif

#if (Make_TTF == 1)
    dwSystemMemoryMap[FONT_TTF_BUF_MEM_ID]      = systemMemAlloc(FONT_TTF_BUF_MEM_ID, FONT_TTF_MEM_BUF_SIZE);
#endif

#if YUV444_ENABLE  //650 new function
    dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID]      = systemMemAlloc(DISPLAY_BUF1_MEM_ID, ((((pstScreen->wInnerWidth + 0xf) & 0xfffffff0) * ((pstScreen->wInnerHeight + 0xf) & 0xfffffff0))*3));
#else
#ifdef Support_EPD
    dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID]      = systemMemAlloc(DISPLAY_BUF1_MEM_ID, ((((pstScreen->wInnerWidth + 0xf) & 0xfffffff0) * ((pstScreen->wInnerHeight + 0xf) & 0xfffffff0)) << 2));
#else
    dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID]      = systemMemAlloc(DISPLAY_BUF1_MEM_ID, ((((pstScreen->wInnerWidth + 0xf) & 0xfffffff0) * ((pstScreen->wInnerHeight + 0xf) & 0xfffffff0)) << 1));
#endif
#endif
//    if (dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID])
//        dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID] |= BIT29;

    MP_ALERT("DISPLAY_BUF1_MEM_ID %dKB", SystemGetMemSize(DISPLAY_BUF1_MEM_ID) / 1024);
    dwSystemMemoryMap[DISPLAY_BUF2_MEM_ID]      = systemMemAlloc(DISPLAY_BUF2_MEM_ID, dwSystemMemorySize[DISPLAY_BUF1_MEM_ID]);

//    if (dwSystemMemoryMap[DISPLAY_BUF2_MEM_ID])
//        dwSystemMemoryMap[DISPLAY_BUF2_MEM_ID] |= BIT29;

    MP_ALERT("DISPLAY_BUF2_MEM_ID %dKB", SystemGetMemSize(DISPLAY_BUF2_MEM_ID) / 1024);
#if DISPLAY_BUFFER_DYNAMIC
	st_dwDispBufferSize=SystemGetMemSize(DISPLAY_BUF1_MEM_ID);
	st_dwDispBuffer2Addr=dwSystemMemoryMap[DISPLAY_BUF2_MEM_ID];
#endif
    dwSystemMemoryMap[HEAP_BUF_MEM_ID]          = systemMemAlloc(HEAP_BUF_MEM_ID, dwSystemMemoryRealSize - dwSystemMemCurrSize - 16);

    MP_ALERT("disp buffer1 %08x %dKB", dwSystemMemoryMap[DISPLAY_BUF1_MEM_ID], dwSystemMemorySize[DISPLAY_BUF1_MEM_ID] / 1024);
    MP_ALERT("disp buffer2 %08x %dKB", dwSystemMemoryMap[DISPLAY_BUF2_MEM_ID], dwSystemMemorySize[DISPLAY_BUF2_MEM_ID] / 1024);
    MP_ALERT("Heap buffer  %08x %dKB", dwSystemMemoryMap[HEAP_BUF_MEM_ID], dwSystemMemorySize[HEAP_BUF_MEM_ID] / 1024);

    MP_ALERT("Heap memory = %dKB", SystemGetMemSize(HEAP_BUF_MEM_ID) / 1024);

    IntEnable();

    User_mem_init();
    ker_mem_init();
}



#define BOUNDARY_OF_SOME_DW_ADDR(x, y)          ((x + 4 * y - 1) & (0xFFFFFFFF - 4 * y + 1))

DWORD SystemMemoryAllocAlignDW(DWORD allocSize, DWORD dwNumber, DWORD *orginalAddress)
{
    DWORD bufAddr, orgAddr;
    DWORD dwOffset;

    orgAddr = (DWORD) ext_mem_malloc(allocSize + 4 * dwNumber - 1);

    if (orginalAddress)
        *orginalAddress = orgAddr;

    if (!orgAddr)
        return 0;

    bufAddr = BOUNDARY_OF_SOME_DW_ADDR(orgAddr, dwNumber);

    MP_DEBUG("Before align address - 0x%08X", orgAddr);
    MP_DEBUG("After align address - 0x%08X", bufAddr);

    return bufAddr;
}

