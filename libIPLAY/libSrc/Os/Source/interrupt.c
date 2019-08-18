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
* Filename      : interrupt.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Interrupt Service Routine
*******************************************************************************
*/
///
///@defgroup    INT   Interrupt

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "osinner.h"
#include "taskid.h"
#include "interrupt.h"

///////////////////////////////////////////////////////////
//
// Local function
//
///////////////////////////////////////////////////////////
static void dmaIsr(void);

///////////////////////////////////////////////////////////
//
// Local Variable
//
///////////////////////////////////////////////////////////
static OS_INTERRUPT_CALLBACK_FUNC intCallbackFunPtr[MAX_SYSTEM_INTERRUPT_SERVICE] = {0};
static OS_INTERRUPT_CALLBACK_FUNC dmaIntCallbackFunPtr[MAX_SUB_DMA_INTERRUPT_SERVICE] = {0};


///////////////////////////////////////////////////////////
//
// External Variable
//
///////////////////////////////////////////////////////////
extern BOOL boolContext;


///////////////////////////////////////////////////////////
//
// All function about DMA interrupt
//
///////////////////////////////////////////////////////////
///
///@ingroup     INT
///@brief       Enable an sub DMA interrupt service.
///
///@param       dwPattern   The pattern of bit field that it's service will be enabled by this function.
///
void DmaIntEna(DWORD dwPattern)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;

    isr->ImaskDma |= dwPattern;
}



///
///@ingroup     INT
///@brief       Disable an sub DMA interrupt service.
///
///@param       dwPattern   The pattern of bit field that it's service will be disable by this function.
///
void DmaIntDis(DWORD dwPattern)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;

    isr->ImaskDma &= ~dwPattern;
}



///
///@ingroup     INT
///@brief       Register a DMA ISR to system to service the interrupt identify by dwPattern.
///
///@param       dwPattern       The bit field of sub DMA mask.
///@param       callbackFunPtr  The interrupt service routine function point.
///
SDWORD DmaIntHandleRegister(DWORD dwPattern, OS_INTERRUPT_CALLBACK_FUNC callbackFunPtr)
{
    BYTE index = 0;

    if (dwPattern == 0)
        return FAIL;

    IntDisable();

    while (index < MAX_SUB_DMA_INTERRUPT_SERVICE)
    {
        if (dwPattern & BIT0)
        {
            dmaIntCallbackFunPtr[index] = callbackFunPtr;
            break;
        }

        dwPattern >>= 1;
        index++;
    }

    IntEnable();

    return PASS;
}



///
///@ingroup INT
///@brief   Release a sub DMA ISR by dwPattern.
///
///@param   dwPattern   The bit field of sub DMA mask.
///
///@reamrk  Same as DmaIntHandleRegister(dwPattern, NULL)
///
SDWORD DmaIntHandleRelease(DWORD dwPattern)
{
    return DmaIntHandleRegister(dwPattern, NULL);
}



static void dmaIsr(void)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    register DWORD dwValue;
    register DWORD index = 0;

    dwValue = isr->IcauseDma & isr->ImaskDma;
    isr->IcauseDma &= ~isr->ImaskDma;

    while (dwValue)
    {
        if (dwValue & BIT0)
        {
            if (dmaIntCallbackFunPtr[index])
                dmaIntCallbackFunPtr[index]();
            else
            {
                MP_ALERT("\r\n--E-- Sub DMA int-%2d was enabled !!!", index);
                MP_ALERT("--E-- But, it's callback is not registered yet !!!");
            }
        }

        dwValue >>= 1;
        index++;
    }
}



///////////////////////////////////////////////////////////
//
// All function about system interrupt
//
///////////////////////////////////////////////////////////
///
///@ingroup     INT
///@brief       Enable an system interrupt service.
///
///@param       dwPattern   The interrupt number that it's service will be enabled by this function.
///
void SystemIntEna(DWORD dwPattern)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;

    isr->MiMask |= dwPattern;
}



///
///@ingroup INT
///@brief   Disable an system interrupt service.
///
///@param   dwPattern   The interrupt number that it's service will be enabled by this function.
///
void SystemIntDis(DWORD dwPattern)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;

    isr->MiMask &= ~dwPattern;
}



///
///@ingroup     INT
///@brief       Register a ISR to system to service the interrupt identify by dwPattern.
///
///@param       dwPattern       The bit field of master mask.
///@param       callbackFunPtr  The interrupt service routine function point.
///
SDWORD SystemIntHandleRegister(DWORD dwPattern, OS_INTERRUPT_CALLBACK_FUNC callbackFunPtr)
{
    BYTE index = 0;

    if (dwPattern == 0)
        return FAIL;

    if (dwPattern & IM_DMA)
    {
        MP_ALERT("-E- Can't change IM_DMA's ISR fun pointer !!! It had a default ISR.");

        return FAIL;
    }

    IntDisable();

    while (index < MAX_SYSTEM_INTERRUPT_SERVICE)
    {
        if (dwPattern & BIT0)
        {
            intCallbackFunPtr[index] = callbackFunPtr;
            break;
        }

        dwPattern >>= 1;
        index++;
    }

    IntEnable();

    return PASS;
}



///
///@ingroup INT
///@brief   Release a system ISR by dwPattern.
///
///@param   dwPattern   The bit field of sub DMA mask.
///
///@reamrk  Same as SystemIntHandleRegister(dwPattern, NULL)
///
SDWORD SystemIntHandleRelease(DWORD dwPattern)
{
    return SystemIntHandleRegister(dwPattern, NULL);
}



///////////////////////////////////////////////////////////
//
// system interrupt access handle
// access all interrupt by the sequence of bit
//
///////////////////////////////////////////////////////////
//#define __MONITOR_ISR_REENERY

#ifdef __MONITOR_ISR_REENERY
static WORD entryTimes = 0;
#endif

void SystemInt(void)
{
    INTERRUPT *isr = (INTERRUPT *) INT_BASE;
    register DWORD dwValue;
    register DWORD index = 0;

    if (boolContext)
    {
        MP_ALERT("--E-- Wrong status of boolContext !!!");
        __asm("break 100");
    }

#ifdef __MONITOR_ISR_REENERY
    if (entryTimes)
    {
        MP_ALERT("Task-%d - %d %d %d", TaskGetId(), entryTimes, dwValue, index);
        __asm("break 100");
    }

    entryTimes++;
#endif

    dwValue = isr->Micause & isr->MiMask;

    ////////////////////////////////////////////////
    //
    //  For all platform
    //
    ////////////////////////////////////////////////
    while (dwValue)
    {
        if (dwValue & BIT0)
        {
            if (intCallbackFunPtr[index])
                intCallbackFunPtr[index]();
            else
            {
                MP_ALERT("\r\n--E-- Master int-%2d was enabled !!!", index);
                MP_ALERT("--E-- But, it's callback is not registered yet !!!");
                MP_ALERT("\r\n--E-- Int callback is not registered for master int-%2d !!!\r\n", index);
                isr->MiMask &= ~(BIT0 << index);
            }
        }

        isr->Micause &= ~(BIT0 << index);
        dwValue >>= 1;
        index++;
    }

#ifdef __MONITOR_ISR_REENERY
    if (entryTimes)
        entryTimes--;
#endif
}



void SystemIntInit(void)
{
    intCallbackFunPtr[0] = dmaIsr;              // IM_DMA
    SystemIntEna(IM_DMA);
}

