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
* Filename      : SystemPM.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "mpTrace.h"

#include "global612.h"
#include "taskid.h"
#include "flagdefine.h"
#include "peripheral.h"
#include "bios.h"
#include "ui.h"

#if (CHIP_VER_MSB != CHIP_VER_615)

///////////////////////////////////////////////////////////////////////////
//
//  Variable declarations
//
///////////////////////////////////////////////////////////////////////////
static void (*sysPwdcPrepareCallBackFunc)(void);
static void (*sysPwdcWakeupCallBackFunc)(void);
static ST_EXCEPTION_MSG stSysExceptionPwdcCont = {0};

///////////////////////////////////////////////////////////////////////////
//
//  Constant declarations
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Static function prototype
//
///////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////
//
//  Definition of external functions
//
///////////////////////////////////////////////////////////////////////////
#define PWDC_CAUSE_ADDR             INT_BASE

static void pwdcWakeupWait(void); __attribute__((section(".pwdcwait")))
static void pwdcWakeupWait(void)
{   // special code, don't change it
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    DWORD timeout;

    // waiting for DMA request finish
    timeout = 0x00800000;
    while ((timeout--) > 0);
    regClockPtr->PwrPdc = 0x11044444;       // Start sleep
    regDmaPtr->SdramCtl |= BIT0;

    // The times of NOP depend on assembly code. Align to 8DWORD
    //__asm("nop");
    //__asm("nop");
    __asm("nop");
    while (!((*(volatile DWORD *) PWDC_CAUSE_ADDR) & IC_PWDC));
    regDmaPtr->SdramCtl &= ~BIT0;
    regClockPtr->PwrPdc = 0x00044444;       // Reset sleep
    //timeout = 0x00800000;
    //while ((timeout--) > 0);
    *(volatile DWORD *) PWDC_CAUSE_ADDR &= ~IC_PWDC;
    // measured the MSDCK and MSDCKE? Did it go to Self-refresh already?    
#if (IR_ENABLE)
    IR_IntCauseClear();
#endif
}



// [31:28] = [IR Pol, ALARM Out, GPIO/FGPIO, IR]
#if (CHIP_VER == (CHIP_VER_650 | CHIP_VER_A))
    #define __PWDC_CTRL_VALUE       0xB0000000          // Wakeup by GPIO/FGPIO, IR
    //#define __PWDC_CTRL_VALUE       0xA0000000          // Wakeup by GPIO/FGPIO only
#else
    #if (IR_ENABLE)
    #define __PWDC_CTRL_VALUE       0xF0000000          // Wakeup by AlarmOut, GPIO/FGPIO, IR
    #else
    #define __PWDC_CTRL_VALUE       0xE0000000          // Wakeup by AlarmOut, GPIO/FGPIO
    #endif
#endif

static BOOL inPwdcMode = FALSE;

static void systemPwdcHandler(void)
{
    static DWORD dwTimes = 0;
    E_CPU_CLOCK_SELECT orgCpuClkSel;
    E_MEM_CLOCK_SELECT orgMemClkSel;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DMA *regDmaPtr = (DMA *) DMA_BASE;
    INTERRUPT *regIntPtr = (INTERRUPT *) INT_BASE;
    IDU *regIduPtr = (IDU *) IDU_BASE;
    void (*pwdcPrepareCallBackFunc)(void);
    void (*pwdcWakeupCallBackFunc)(void);
    DWORD mdclken;

    MP_DEBUG("%d-", ++dwTimes);

    inPwdcMode = TRUE;
    orgCpuClkSel = Clock_CpuClockSelGet();
    orgMemClkSel = Clock_MemClockSelGet();

#if SC_USBDEVICE
    SystemUsbdDetectDisable(); // Disable USB Device detect (It will read register when clock is disabled)
#endif

#if SC_USBHOST
    UsbPwdcHandler(USBOTG0, TRUE); // USBOTG0 Host Suspend
    UsbPwdcHandler(USBOTG1, TRUE); // USBOTG1 Host Suspend
#endif

    TaskContextDeactive();
    SystemTimerPause();
    pwdcPrepareCallBackFunc = SystemPwdcPrepqreCallbackGet();

    if (pwdcPrepareCallBackFunc)
        pwdcPrepareCallBackFunc();

    ExceptionTagRelease(stSysExceptionPwdcCont.dwTag);

    switch (orgCpuClkSel)
    {
    case CPUCKS_PLL1:
#if (CHIP_VER_MSB == CHIP_VER_650)
    case CPUCKS_PLL1_DIV_1_5:
#endif
        Clock_CpuClockSelSet(CPUCKS_PLL1_DIV_2);
        break;
    case CPUCKS_PLL2:
#if (CHIP_VER_MSB == CHIP_VER_650)
    case CPUCKS_PLL2_DIV_1_5:
#endif
        Clock_CpuClockSelSet(CPUCKS_PLL2_DIV_2);
        break;
    }

    switch (orgMemClkSel)
    {
    case MEMCKS_PLL1:
        Clock_MemClockSelSet(MEMCKS_PLL1_DIV_2);
        break;

    case MEMCKS_PLL2:
        Clock_MemClockSelSet(MEMCKS_PLL2_DIV_2);
        break;
    }

    IntDisable();
    regClockPtr->Clkss_EXT4 &= ~0xF0000000;                             // Clean [31:28] = [IR Pol, ALARM Out, GPIO/FGPIO, IR]
    mdclken = regClockPtr->MdClken;                                     // 0x200467E0
    MP_ALERT("regClockPtr->MdClken = 0x%08X", regClockPtr->MdClken);

#if (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_A)
    regClockPtr->MdClken &= ~CKE_UARTB;
#elif (MP_TRACE_DEBUG_PORT == DEBUG_PORT_HUART_B)
    regClockPtr->MdClken &= ~CKE_UARTA;
#else
    regClockPtr->MdClken &= ~(CKE_UARTA | CKE_UARTB);
#endif
    regClockPtr->MdClken &= ~0x04FFBFEE;                                // all disable, unless UART-A & UART-B
    regClockPtr->Clkss_EXT4 |= __PWDC_CTRL_VALUE;                       // [31:28] = [IR Pol, ALARM Out, GPIO/FGPIO, IR]
    pwdcWakeupWait();

    regClockPtr->MdClken = mdclken;
    pwdcWakeupCallBackFunc = SystemPwdcWakeupCallbackGet();

    Clock_MemClockSelSet(orgMemClkSel);
    Clock_CpuClockSelSet(orgCpuClkSel);

    if (pwdcWakeupCallBackFunc)
        pwdcWakeupCallBackFunc();

    SystemTimerResume();
    TaskContextActive();

#if SC_USBHOST
    UsbPwdcHandler(USBOTG0, FALSE); //USBOTG0 Host Resume
    UsbPwdcHandler(USBOTG1, FALSE); //USBOTG1 Host Resume
#endif

    EventSet(SYSTEM_EVENT_ID, PWDC_WAKEUP_FINISHED);

#if (SC_USBDEVICE)
    SystemUsbdDetectEnable(); // Enable USB Device detect
#endif
    inPwdcMode = FALSE;
}



BOOL SystemPwdcStausGet(void)
{
    return inPwdcMode;
}



#define __MIN_REENTER_TIME_FOR_PWDC__           5000        // 5sec

void SystemPwdcEnable(void *pwdcPrepareCallBackFunc, void *pwdcWakeupCallBackFunc)
{
    static DWORD preEnterTime = 0;
    DWORD *release;
    DWORD elapsedTime;

    MP_DEBUG("%s - Enter power down mode", __FUNCTION__);

    sysPwdcPrepareCallBackFunc = pwdcPrepareCallBackFunc;
    sysPwdcWakeupCallBackFunc = pwdcWakeupCallBackFunc;

    stSysExceptionPwdcCont.dwTag = ExceptionTagReister(systemPwdcHandler);
    stSysExceptionPwdcCont.msgAddr = 0;

    if (stSysExceptionPwdcCont.dwTag == 0)
    {
        MP_ALERT("--E-- Invalid Exception Tag Index !!!");
        __asm("break 100");
    }

    elapsedTime = SystemGetElapsedTime(preEnterTime);

    if (elapsedTime < __MIN_REENTER_TIME_FOR_PWDC__)
        TaskSleep(__MIN_REENTER_TIME_FOR_PWDC__ - elapsedTime);

    MessageSend(EXCEPTION_MSG_ID, (BYTE *) &stSysExceptionPwdcCont, sizeof(ST_EXCEPTION_MSG));
    EventWait(SYSTEM_EVENT_ID, PWDC_WAKEUP_FINISHED, OS_EVENT_OR, (DWORD *) &release);

    preEnterTime = GetSysTime();
}



void *SystemPwdcPrepqreCallbackGet(void)
{
    return sysPwdcPrepareCallBackFunc;
}



void *SystemPwdcWakeupCallbackGet(void)
{
    return sysPwdcWakeupCallBackFunc;
}

#endif

