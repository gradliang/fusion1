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
* Filename      : hal_mmcp.c
* Programmer(s) : Morris Lin
* Created       : Morris Lin
* Descriptions  : MMCP module only for MP650/660 serial
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
#include "global612.h"
#include "peripheral.h"
#include "taskid.h"

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )

static SDWORD mmcp_m2mcpy(BYTE* const destAddr, const BYTE * const srcAddr, DWORD lens, BYTE flag);
static BOOL mmcpUsed = FALSE;

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////
#define ENABLE_MMCP_INT     1
#define DISABLE_MMCP_INT    0
#define MMCP_WOMODE         BIT8
#define MMCP_INT_DIS        BIT2
#define MMCP_ENABLE         BIT1
#define MMCP_INT_DONE       BIT0
#define TIME_OUT_BASE       500     // ms
#define SHIFT_BYTES         1       // shift 2^SHIFT_BYTES bytes

////////////////////////////////////////////////////////////
//
// globel variable declarations
//
////////////////////////////////////////////////////////////

static BOOL mmcpSemaphoreCreated = FALSE;
static BOOL *isrResponsedPtr = NULL;
//DWORD mmcp_period[2], mmcp_tick[3], mmcp_tpc[3], mmcp_tmv[3], mmcp_maxTime = 0;

static void mmcp_isr(void)
{
    MMCP* regPtrMMCP = (MMCP*)MMCP_BASE;

    regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DONE;

    if (isrResponsedPtr)
    {
        *isrResponsedPtr = TRUE;
        isrResponsedPtr = NULL;
    }

//    mmcp_period[0] = get_elapsed_timeL(mmcp_tick[0], mmcp_tpc[0], mmcp_tmv[0]);
//    get_cur_timeL(&mmcp_tick[1], &mmcp_tpc[1], &mmcp_tmv[1]);
    EventSet(SYSTEM_EVENT_ID, MMCP_DMA_DONE);
    mmcp_resource_unlock(TRUE);
}



static void mmcp_clear_data_cache(DWORD addr)
{
    if ((addr & BIT29) == 0)
    {   // cache region
        SetDataCacheInvalid();
    }
}



static void mmcp_sema_create(void)
{

    if (SemaphoreCreate(MMCP_SEMA_ID, OS_ATTR_PRIORITY, 1) != OS_STATUS_OK)
    {
        MP_ALERT("MMCP semaphore create fail!!!");
        __asm("break 100");
    }

    SystemIntHandleRegister(IM_MMCP, mmcp_isr);
    SystemIntEna(IM_MMCP);
    mmcpSemaphoreCreated = TRUE;
}



static SWORD mmcp_resource_lock(BOOL *semaFlag, BOOL *isrResponsedFlagPtr)
{
    BIU* regBiuPtr = (BIU *) BIU_BASE;
    if (ContextStatusGet())
    {   // in Task
        SemaphoreWait(MMCP_SEMA_ID);
        EventClear(SYSTEM_EVENT_ID, ~MMCP_DMA_DONE);
        IntDisable();      
        *semaFlag = TRUE;
        mmcpUsed = TRUE;
        isrResponsedPtr = isrResponsedFlagPtr;
        //IntEnable();
		//get_cur_timeL(&mmcp_tick[0], &mmcp_tpc[0], &mmcp_tmv[0]);
    }
    else
    {   // in ISR
        if (mmcpUsed)
        {
            MP_ALERT("--E-- Reentry for mmcp_resource_lock by ISR!!!!!");

            return FAIL;
        }
        *semaFlag = FALSE;
        mmcpUsed = TRUE;
    }
    regBiuPtr->BiuArst |= ARST_MMCP;

    return PASS;
}



static void mmcp_resource_unlock(BOOL semaLocked)
{
    MMCP* regPtrMMCP = (MMCP *) MMCP_BASE;
    BIU* regBiuPtr = (BIU *) BIU_BASE;
    
    IntDisable();
    regPtrMMCP->MMCP_CFG |= MMCP_INT_DIS;
    mmcpUsed = FALSE;
    regBiuPtr->BiuArst &= ~ARST_MMCP;

    if (semaLocked == TRUE)
    	SemaphoreRelease(MMCP_SEMA_ID);
	else
		IntEnable();

}



#define MAX_MMCP_TIME_PER_KB            200      // us

static SDWORD mmcp_DmaDoneWait(BOOL waitEvent, DWORD lens, BOOL *isrResponsedPtr)
{
    MMCP* regPtrMMCP = (MMCP *) MMCP_BASE;
    SWORD status = PASS;
    DWORD timeOut = TIME_OUT_BASE;
    DWORD release;

    timeOut = 8 + (((MAX_MMCP_TIME_PER_KB * (lens + 1023)) >> 10) >> 1000);

    if (waitEvent == TRUE)
    {
        DWORD time;
   
		if (EventWaitWithTO(SYSTEM_EVENT_ID, MMCP_DMA_DONE, OS_EVENT_OR, &release, timeOut) != OS_STATUS_OK)
        {
            MP_ALERT("\r\n--E-- %s time out by event!!! - %d bytes", __FUNCTION__, lens);

            if (isrResponsedPtr && (*isrResponsedPtr == FALSE))
                MP_ALERT("--E-- But interrupt had responsed, should not happen !!!");

            status = FAIL;
        }
#if 0
        mmcp_period[1] = get_elapsed_timeL(mmcp_tick[1], mmcp_tpc[1], mmcp_tmv[1]);

        if (mmcp_period[0] > mmcp_maxTime)
        {
            mmcp_maxTime = mmcp_period[0];
            MP_ALERT("Max time is %u, %dus", mmcp_maxTime, mmcp_maxTime * 1024 / lens);
        }
#endif
    }
    else if (ContextStatusGet() != 0)
    {   // In Task
        DWORD startTime = GetSysTime();
        IntEnable();

        while ((regPtrMMCP->MMCP_CFG & MMCP_INT_DONE) == 0)
        {
            if (SystemGetElapsedTime(startTime) >= timeOut)
            {
                if ((regPtrMMCP->MMCP_CFG & MMCP_INT_DONE) == 0)
                {
                    MP_ALERT("\r\n--E-- %s time out by Polling!!! - %d bytes\r\n", __FUNCTION__, lens);
                    status = FAIL;
                }

                break;
            }
        }

        regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DONE;
    }
    else
    {   // In ISR
        timeOut = (lens >> SHIFT_BYTES) + 200000;

        while (((regPtrMMCP->MMCP_CFG & MMCP_INT_DONE) == 0) && timeOut--)
            __asm("NOP");

        if (timeOut == 0)
        {
            MP_ALERT("\r\n--E-- %s time out by Polling in ISR!!! - %d bytes\r\n", __FUNCTION__, lens);
            status = FAIL;
        }

        regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DONE;
    }
    return status;
}



static SDWORD mmcp_m2mcpy_block(const BYTE * const srcAddr, BYTE * const destAddr, DWORD row, DWORD col, DWORD sLineOffset, DWORD dLineOffset, BYTE flag)
{
    MMCP* regPtrMMCP = (MMCP *) MMCP_BASE;
    BOOL semaLocked = FALSE;
    DWORD targetAddr = ((DWORD) destAddr) & ~0xA0000000;
    DWORD sourceAddr = ((DWORD) srcAddr) & ~0xA0000000;
    SDWORD status;
    BOOL isrResponsed = FALSE;

    if ((targetAddr == 0) || (sourceAddr == 0))
    {
        MP_ALERT("--E-- mmcp_block - NULL pointer !!!!");

        return FAIL;
    }

    if (mmcpSemaphoreCreated == FALSE)
    {
        mmcp_sema_create();
    }

    mmcp_clear_data_cache((DWORD) destAddr);

    if (mmcp_resource_lock(&semaLocked, &isrResponsed) == FAIL)
    {
        MP_ALERT("--E-- mmcp_block - mmcp was used !!!!");
        IntEnable();

        return FAIL;
    }

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DIS;
    else
        regPtrMMCP->MMCP_CFG |= MMCP_INT_DIS;
    regPtrMMCP->MMCP_CFG &= ~MMCP_WOMODE;
    regPtrMMCP->SRC_ADDR = (DWORD) srcAddr;
    regPtrMMCP->DST_ADDR = (DWORD) destAddr;
    regPtrMMCP->DATA_LEN = col;
    regPtrMMCP->SRC_INCR = sLineOffset;
    regPtrMMCP->DST_INCR = dLineOffset;
    regPtrMMCP->LOOP_CNT = row;
    regPtrMMCP->WOM_DATA = 0;

    regPtrMMCP->MMCP_CFG |= MMCP_ENABLE;

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        status = mmcp_DmaDoneWait(TRUE, row * col, &isrResponsed);
    else
        status = mmcp_DmaDoneWait(FALSE, row * col, NULL);

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
    {
        // Do not thing, Semaphore was released by MMCP ISR.
        if ((status == FAIL) && (isrResponsed == FALSE))
            mmcp_resource_unlock(semaLocked);
    }
    else
    {
        // For polling case
        mmcp_resource_unlock(semaLocked);
    }

    if (status == FAIL)
    {
        MP_ALERT("--E-- %s timeout !!!", __FUNCTION__);
    }

    return status;
}



SDWORD mmcp_block(const BYTE * const srcAddr, BYTE * const destAddr, DWORD row, DWORD col, DWORD sLineOffset, DWORD dLineOffset)
{
    return mmcp_m2mcpy_block(srcAddr, destAddr, row, col, sLineOffset, dLineOffset, TRUE);
}



SDWORD mmcp_block_polling(const BYTE * const srcAddr, BYTE * const destAddr, DWORD row, DWORD col, DWORD sLineOffset, DWORD dLineOffset)
{
    return mmcp_m2mcpy_block(srcAddr, destAddr, row, col, sLineOffset, dLineOffset, FALSE);
    //return mmcp_m2mcpy_block(srcAddr, destAddr, row, col, sLineOffset, dLineOffset, TRUE);
}



static SDWORD mmcp_fill(BYTE *addr, DWORD value, DWORD lens, BYTE flag)
{
    MMCP* regPtrMMCP = (MMCP*) MMCP_BASE;
    BIU* regBiuPtr = (BIU *) BIU_BASE;
    DWORD targetAddr = ((DWORD) addr) & ~0xA0000000;
    BOOL semaLocked = FALSE;
    SDWORD status;
    BOOL isrResponsed = FALSE;

    if ((targetAddr == 0) || (lens == 0))
    {
        MP_ALERT("--E-- mmcp_fill - NULL pointer !!!!");

        return FAIL;
    }

    if (mmcpSemaphoreCreated == FALSE)
    {
        mmcp_sema_create();
    }

    mmcp_clear_data_cache((DWORD) addr);

    if (mmcp_resource_lock(&semaLocked, &isrResponsed) == FAIL)
    {
        MP_ALERT("--E-- mmcp_fill - mmcp was used !!!!");
        IntEnable();

        return FAIL;
    }

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DIS;
    else
        regPtrMMCP->MMCP_CFG |= MMCP_INT_DIS;

    regPtrMMCP->MMCP_CFG |= MMCP_WOMODE;
    regPtrMMCP->SRC_ADDR = 0;
    regPtrMMCP->DST_ADDR = (DWORD) addr;
    regPtrMMCP->DATA_LEN = lens;
    regPtrMMCP->SRC_INCR = 0;
    regPtrMMCP->DST_INCR = 0;
    regPtrMMCP->LOOP_CNT = 0;
    regPtrMMCP->WOM_DATA = value;
    regPtrMMCP->MMCP_CFG |= MMCP_ENABLE;

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        status = mmcp_DmaDoneWait(TRUE, lens, &isrResponsed);
    else
        status = mmcp_DmaDoneWait(FALSE, lens, NULL);

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
    {
        // Do not thing, Semaphore was released by MMCP ISR.
        if ((status == FAIL) && (isrResponsed == FALSE))
            mmcp_resource_unlock(semaLocked);
    }
    else
    {
        // For polling case
        mmcp_resource_unlock(semaLocked);
    }

    if (status == FAIL)
    {
        MP_ALERT("--E-- %s timeout !!!", __FUNCTION__);
    }

    return status;
}



SDWORD mmcp_memcpy(BYTE * const destAddr, const BYTE * const srcAddr, DWORD lens)
{
    if (lens < 512)
    {
        memcpy(destAddr, srcAddr, lens);

        return PASS;
    }

    return mmcp_m2mcpy(destAddr, srcAddr, lens, ENABLE_MMCP_INT);
}



SDWORD mmcp_memcpy_polling(BYTE * const destAddr, const BYTE * const srcAddr, DWORD lens)
{
    if (lens < 512)
    {
        memcpy(destAddr, srcAddr, lens);

        return PASS;
    }

    return mmcp_m2mcpy(destAddr, srcAddr, lens, DISABLE_MMCP_INT);
    //return mmcp_m2mcpy(destAddr, srcAddr, lens, ENABLE_MMCP_INT);
}



static SDWORD mmcp_m2mcpy(BYTE * const destAddr, const BYTE * const srcAddr, DWORD lens, BYTE flag)
{
    MMCP* regPtrMMCP = (MMCP*)MMCP_BASE;
    BIU* regBiuPtr = (BIU *) BIU_BASE;
    SDWORD status;
    DWORD targetAddr = ((DWORD) destAddr) & ~0xA0000000;
    DWORD sourceAddr = ((DWORD) srcAddr) & ~0xA0000000;
    BOOL semaLocked = FALSE;
    BOOL isrResponsed = FALSE;

    if ((targetAddr == 0) || (srcAddr == 0) || (lens == 0))
    {
        MP_ALERT("--E-- mmcp_m2mcpy - NULL pointer !!!!");

        return FAIL;
    }

    if (mmcpSemaphoreCreated == FALSE)
    {
        mmcp_sema_create();
    }

    mmcp_clear_data_cache((DWORD) destAddr);

    if (mmcp_resource_lock(&semaLocked, &isrResponsed) == FAIL)
    {
        MP_ALERT("--E-- mmcp_m2mcpy - mmcp was used !!!!");
        IntEnable();

        return FAIL;
    }

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        regPtrMMCP->MMCP_CFG &= ~MMCP_INT_DIS;
    else
        regPtrMMCP->MMCP_CFG |= MMCP_INT_DIS;

    regPtrMMCP->MMCP_CFG &= ~(MMCP_WOMODE | MMCP_ENABLE);
    regPtrMMCP->SRC_ADDR = (DWORD) srcAddr;
    regPtrMMCP->DST_ADDR = (DWORD) destAddr;
    regPtrMMCP->DATA_LEN = lens;
    regPtrMMCP->SRC_INCR = 0;
    regPtrMMCP->DST_INCR = 0;
    regPtrMMCP->LOOP_CNT = 0;
    regPtrMMCP->WOM_DATA = 0;

    regPtrMMCP->MMCP_CFG |= MMCP_ENABLE;

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
        status = mmcp_DmaDoneWait(TRUE, lens, &isrResponsed);
    else
        status = mmcp_DmaDoneWait(FALSE, lens, NULL);

    if ((flag == ENABLE_MMCP_INT) && (semaLocked == TRUE))
    {
        // Do not thing, Semaphore was released by MMCP ISR.
        if ((status == FAIL) && (isrResponsed == FALSE))
            mmcp_resource_unlock(semaLocked);
    }
    else
    {
        // For polling case
        mmcp_resource_unlock(semaLocked);
    }

    if (status == FAIL)
    {
        MP_ALERT("--E-- %s timeout !!!", __FUNCTION__);
    }

    return status;
}



SDWORD mmcp_memset_u32(BYTE *addr, DWORD value, DWORD lens)
{
    DWORD tmpAddr = ((DWORD) addr) & ~0xA0000000;
    DWORD newValue = value;
    DWORD remLen = lens;
    BYTE remainder = tmpAddr & 0x03;
    BYTE i;

    if ((tmpAddr == 0) || (lens == 0))
    {
        MP_ALERT("--E-- mmcp_memset_u32 - Null pointer or zero length !!!");

        return FAIL;
    }

    // recover to orginal address
    tmpAddr = (DWORD) addr;

    if (remainder)
    {   // address is not 4 bytes alignment.
        tmpAddr = (DWORD) (addr + 4 - remainder);
        newValue = (value << ((4 - remainder) << 3)) | (value >> (remainder << 3));
        remLen = lens + 4 - remainder;

        for (i = 0; i < (4 - remainder); i++)
        {
            *addr = (BYTE) (value >> ((3 - i) << 3));
            addr++;
        }
    }

    if (remLen)
    {
        if (remLen & 0x03)
        {   // remain length is not multiplex of 4
            remainder = remLen & 0x03;
            remLen -= remainder;

            if (remLen)
            {
                if (mmcp_fill((BYTE *) tmpAddr, newValue, remLen, FALSE) == FAIL)
                    return FAIL;
            }

            addr = (BYTE *) (tmpAddr + remLen);

            while (remainder--)
            {
                *addr = (BYTE) ((newValue >> (remainder << 3)) & 0xFF);
                addr++;
            }
        }
        else
        {   // remain length is multiplex of 4
            return mmcp_fill((BYTE *) tmpAddr, newValue, remLen, FALSE);
        }
    }

    return PASS;
}



SDWORD mmcp_memset(BYTE* addr, BYTE value, DWORD lens)
{
    DWORD newValue = value;

    if (lens < 128)
    {
        memset(addr, value, lens);

        return PASS;
    }

    newValue |= (newValue << 24) | (newValue << 16) | (newValue << 8);

    return mmcp_memset_u32(addr, newValue, lens);
}

#endif

