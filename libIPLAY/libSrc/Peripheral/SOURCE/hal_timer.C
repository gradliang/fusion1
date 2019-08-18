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
* Filename      : MP650_timer.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Timer moudle for MP650
*******************************************************************************
*/

////////////////////////////////////////////////////////////
//
// define this module show debug message or not,  0 : disable, 1 : enable
//
////////////////////////////////////////////////////////////
#define LOCAL_DEBUG_ENABLE  0

////////////////////////////////////////////////////////////
//
// Include section
//
////////////////////////////////////////////////////////////
#include "global612.h"
#include "mpTrace.h"
#include "os.h"
#include "peripheral.h"
#include "bios.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// Structure declarations
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// Type declarations
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// Function declarations
//
////////////////////////////////////////////////////////////
static void nullTimerProc(WORD);
static void tm1Isr(void);
static void tm2Isr(void);
static void tm3Isr(void);
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
static void tm4Isr(void);
static void tm5Isr(void);
#endif

////////////////////////////////////////////////////////////
//
// Variable declarations
//
////////////////////////////////////////////////////////////
static void (*timer1CallBackFunctionPtr)(WORD) = nullTimerProc;
static void (*timer2CallBackFunctionPtr)(WORD) = nullTimerProc;
static void (*timer3CallBackFunctionPtr)(WORD) = nullTimerProc;
#if (CHIP_VER_MSB == CHIP_VER_615)
// For low frame rate video
static DWORD dwTimer2Cnt = 0;
static DWORD dwTimer2Cycle = 0;
#else   // MP650/660
static void (*timer4CallBackFunctionPtr)(WORD) = nullTimerProc;
static void (*timer5CallBackFunctionPtr)(WORD) = nullTimerProc;
#endif

////////////////////////////////////////////////////////////
//
// Macro declarations
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//
// Definition of local functions
//
////////////////////////////////////////////////////////////
static void nullTimerProc(WORD tick)
{

}

////////////////////////////////////////////////////////////
//
// Definition of external functions
//
////////////////////////////////////////////////////////////
void TimerEnable(BYTE timerIndex)
{
    TimerIntEnable(timerIndex);
}



void TimerDisable(BYTE timerIndex)
{
    TimerIntDisable(timerIndex);
}



void TimerIntEnable(BYTE timerIndex)
{
    switch (timerIndex)
    {
    case 1:
        SystemIntHandleRegister(IM_TM1, tm1Isr);
        SystemIntEna(IM_TM1);
        break;

    case 2:
        SystemIntHandleRegister(IM_TM2, tm2Isr);
        SystemIntEna(IM_TM2);
        break;

    case 3:
        SystemIntHandleRegister(IM_TM3, tm3Isr);
        SystemIntEna(IM_TM3);
        break;

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        SystemIntHandleRegister(IM_TM4, tm4Isr);
        SystemIntEna(IM_TM4);
        break;

    case 5:
        SystemIntHandleRegister(IM_TM5, tm5Isr);
        SystemIntEna(IM_TM5);
        break;
#endif

    default:
        MP_ALERT("--E-- TimerIntEnable - Wrong timer index-%3d!!!", timerIndex);
        break;
    }
}



//---------------------------------------------------------------------------
void TimerIntDisable(BYTE timerIndex)
{
    switch (timerIndex)
    {
    case 1:
        SystemIntDis(IM_TM1);
        break;

    case 2:
        SystemIntDis(IM_TM2);
        break;

    case 3:
        SystemIntDis(IM_TM3);
        break;

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        SystemIntDis(IM_TM4);
        break;

    case 5:
        SystemIntDis(IM_TM5);
        break;
#endif

    default:
        MP_ALERT("--E-- TimerIntDisable - Wrong timer index-%3d!!!", timerIndex);

        return;
        break;
    }

    TimerPause(timerIndex);
}



void TimerResume(BYTE timerIndex, BYTE cleanCount)
{
    TIMER *regTimerPtr;

    switch (timerIndex)
    {
    case 1:
        regTimerPtr = (TIMER *) TIMER1_BASE;
        break;

    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
        break;

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
        break;
#endif

    default:
        MP_ALERT("--E-- TimerResume - Wrong timer index-%3d!!!", timerIndex);

        return;
        break;
    }

    if (cleanCount)
    {
        regTimerPtr->TmC &= ~BIT0;
        regTimerPtr->Tpc &= 0xFFFF0000;
        regTimerPtr->TmV = 0;
    }

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while ((regTimerPtr->TmC & BIT0) == 0)
        regTimerPtr->TmC |= BIT0;
#else
    regTimerPtr->TmC |= BIT0;
#endif
}



void TimerPause(BYTE timerIndex)
{
    TIMER *regTimerPtr;

    switch (timerIndex)
    {
    case 1:
        regTimerPtr = (TIMER *) TIMER1_BASE;
        break;

    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
        break;

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
        break;
#endif

    default:
        MP_ALERT("--E-- TimerPause - Wrong timer index-%3d!!!", timerIndex);

        return;
        break;
    }


#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & BIT0)
        regTimerPtr->TmC &= ~BIT0;
#else
    regTimerPtr->TmC &= ~BIT0;
#endif
}



static SDWORD calTimerParameter(DWORD period, BYTE extCountUnit, DWORD *Tcb, DWORD *Tpc, DWORD *ExtSwCounter)
{
    DWORD dwFREQ;
    DWORD tmMaxTcbResolution, miniPeriod;

    MP_DEBUG("calTimerParameter -\r\ntimer period is %dus", period);

    if ( !Tcb || !Tpc )
    {
        MP_ALERT("--E-- %s - Null pointer !!!", __FUNCTION__);

        return -2;
    }

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    dwFREQ = MAIN_CRYSTAL_CLOCK / 1000;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    dwFREQ = Clock_CpuFreqGet() / 1000;                     // KHz
#else
    dwFREQ = Clock_PllFreqGet(CLOCK_PLL2_INDEX) / 1000; // KHz
#endif

    MP_DEBUG("Timer source is %dKHz", dwFREQ);

    tmMaxTcbResolution = (LONG) MAX_TCB_COUNT * (LONG) 1000 / (LONG) dwFREQ;    // us
    MP_DEBUG("tmMaxTcbResolution = %dus", tmMaxTcbResolution);

#if (CHIP_VER_MSB == CHIP_VER_615)
    // Common case
    if (((LONG) tmMaxTcbResolution * (LONG) MAX_PCV_COUNT) < (LONG) period)
    {
        if ( (extCountUnit == 0) || !ExtSwCounter )
        {
            MP_ALERT("Timer out of scale and no extend SW counter !!!");

            return -1;
        }

        *Tcb = Clock_CpuFreqGet() / 1000;           // 1ms
        *Tpc = ((DWORD) extCountUnit) << 16;        // 3ms
        period = (period + (VIDEO_TIMER_TPC_PERIOD >> 1)) / VIDEO_TIMER_TPC_PERIOD;
        *ExtSwCounter = period / extCountUnit;
    }
    else
    {
        if (tmMaxTcbResolution >= period)
        {   // Do not use Tpc
            *Tpc = 1 << 16;
            *Tcb = (LONG) period * (LONG) dwFREQ / (LONG) 1000;

            MP_DEBUG("Do not used PCE !!!");
        }
        else
        {
            // Calculate the min unit of Pcv
            if (period < MAX_PCV_COUNT)
            {   // period < MAX_PCV_COUNT(us)
                miniPeriod = 1;     // us

                MP_DEBUG("Timer resolution is 1us !!");
            }
            else
            {
                DWORD count, currTolerance, minTolerance = 0xFFFFFFFF;

                // Calculate the start dividend
                // Ex. period = 2000
                // 1. start dividend, count = (2000 / MAX_PCV_COUNT) + 1 = (2000 / 255) + 1 = 8
                // 2. remainder of (period / count) is 0. So, counter period is count (8us)
                //    (2000 % 8) is 0. So, Timer resolution is 8us, and PCE is (2000 / 8) = 250 times.
                count = (period / MAX_PCV_COUNT) + 1;

                MP_DEBUG("Start of timer resolution is %d", count);

                for (; count <= tmMaxTcbResolution; count++)
                {
                    currTolerance = period % count;

                    if (currTolerance == 0)
                    {   // tolerance is zero
                        miniPeriod = count;
                        minTolerance = 0;

                        break;
                    }
                    else if (currTolerance < minTolerance)
                    {   // Update when tolerance is min
                        minTolerance = currTolerance;
                        miniPeriod = count;
                    }
                }

                MP_DEBUG("Timer resolution is %dus", miniPeriod);
            }

            *Tcb = (LONG) miniPeriod * (LONG) dwFREQ / 1000;
            *Tpc = (period / miniPeriod) << 16;
        }

        if (ExtSwCounter)
            *ExtSwCounter = 0;
    }
#else   // MP650/660
    if (((LONG) tmMaxTcbResolution * (LONG) MAX_PCV_COUNT) < (LONG) period)
    {
        MP_ALERT("Timer out of scale !!!");

        return -1;
    }

    // Do not use Tpc, because max resulation is 318sec base on 13.5MHz (OSC)
    *Tpc = 0x00010000;
    *Tcb = (LONG) period * (LONG) dwFREQ / (LONG) 1000;
#endif

    MP_DEBUG("TcB = %d, period is %dus", *Tcb, *Tcb / (dwFREQ / 1000));
    MP_DEBUG("Pcv = %d", *Tpc >> 16);

    if (ExtSwCounter)
        MP_DEBUG("Extend SW Counter = %d, unit is %dms", *ExtSwCounter, extCountUnit);

    return 0;
}



static SDWORD calTimerParameterForPwm(DWORD freq, DWORD *Tcb)
{
    DWORD dwFREQ;
    DWORD tmMaxTcbResolution, miniPeriod;

    if (!Tcb)
    {
        MP_ALERT("--E-- %s - Null pointer !!!", __FUNCTION__);

        return -2;
    }

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    dwFREQ = MAIN_CRYSTAL_CLOCK / 1000;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    dwFREQ = Clock_CpuFreqGet() / 1000;                     // KHz
#else
    dwFREQ = Clock_PllFreqGet(CLOCK_PLL2_INDEX) / 1000;     // KHz
#endif

    MP_DEBUG("Timer source is %dKHz", dwFREQ);
    *Tcb = dwFREQ * 1000 / freq;
    MP_DEBUG("TcB = %d, period is %dus", *Tcb, *Tcb / (dwFREQ / 1000));

    return 0;
}



SDWORD TimerInit(BYTE timerIndex, DWORD usec, void (*timerCallBackFuncPtr)(WORD))
{
    TIMER *regTimerPtr;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD tcb, tpc, regValue;

    MP_DEBUG("Initial Timer%d -", timerIndex);

    if (!timerCallBackFuncPtr)
        timerCallBackFuncPtr = nullTimerProc;

    if (usec == 0)
        usec = 1000;

#if (CHIP_VER_MSB == CHIP_VER_615)
    if (timerIndex == 2)
    {
        dwTimer2Cnt = 0;
        dwTimer2Cycle = 0;

        if (calTimerParameter(usec, 3, &tcb, &tpc, &dwTimer2Cycle) != 0)
            return -1;
    }
    else
#endif
    {
        if (calTimerParameter(usec, 0, &tcb, &tpc, NULL) != 0)
            return -1;
    }

    TimerIntDisable(timerIndex);

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    MP_DEBUG("Using OSC case -");

    switch (timerIndex)
    {
    case 1:
        regTimerPtr = (TIMER *) TIMER1_BASE;
        regClockPtr->TmClkC |= BIT3 | BIT2;
        regClockPtr->MdClken |= CKE_TM1;

        timer1CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
        regClockPtr->TmClkC |= BIT5 | BIT4;
        regClockPtr->MdClken |= CKE_TM2;

        timer2CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
        regClockPtr->TmClkC |= BIT7 | BIT6;
        regClockPtr->MdClken |= CKE_TM3;

        timer3CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    #if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
        regClockPtr->TmClkC |= BIT9 | BIT8;
        regClockPtr->MdClken |= CKE_TM4;

        timer4CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
        regClockPtr->TmClkC &= 0xFFE0FFFF;          // no divide
        //regClockPtr->TmClkC |= 0x1F << 16;          // divid by (n+1)
        regClockPtr->TmClkC |= BIT11 | BIT10;
        regClockPtr->MdClken |= CKE_TM5;

        timer5CallBackFunctionPtr = timerCallBackFuncPtr;
        break;
    #endif

    default:
        mpDebugPrint("--E-- TimerInit - Wrong Timer Index Number - %d!!!", timerIndex);

        return -2;
        break;
    }

#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
    MP_DEBUG("Using CPU clock case -");

    switch (timerIndex)
    {
    case 1:
        regTimerPtr = (TIMER *) TIMER1_BASE;
        regClockPtr->TmClkC &= ~(BIT3 | BIT2);
        regClockPtr->MdClken |= CKE_TM1;

        timer1CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
        regClockPtr->TmClkC &= ~(BIT5 | BIT4);
        regClockPtr->MdClken |= CKE_TM2;

        timer2CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
        regClockPtr->TmClkC &= ~(BIT7 | BIT6);
        regClockPtr->MdClken |= CKE_TM3;

        timer3CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    #if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
        regClockPtr->TmClkC &= ~(BIT9 | BIT8);
        regClockPtr->MdClken |= CKE_TM4;

        timer4CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
        regClockPtr->TmClkC &= 0xFFE0FFFF;          // no divide
        //regClockPtr->TmClkC |= 0x1F << 16;          // divid by (n+1)
        regClockPtr->TmClkC &= ~(BIT11 | BIT10);
        regClockPtr->MdClken |= CKE_TM5;

        timer5CallBackFunctionPtr = timerCallBackFuncPtr;
        break;
    #endif

    default:
        MP_ALERT("--E-- TimerInit - Wrong Timer Index Number - %d!!!", timerIndex);

        return -2;
        break;
    }

#else
    MP_DEBUG("Using AUX0 clock case -");
    regClockPtr->Aux0ClkC = BIT18 | BIT17;      // AUX0 as PLL2
    //regClockPtr->Aux0ClkC = BIT17;              // AUX0 as PLL1

    switch (timerIndex)
    {
    case 1:
        regTimerPtr = (TIMER *) TIMER1_BASE;
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT3 | BIT2)) | BIT2;
        regClockPtr->MdClken |= CKE_TM1;

        timer1CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT5 | BIT4)) | BIT4;
        regClockPtr->MdClken |= CKE_TM2;

        timer2CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT7 | BIT6)) | BIT6;
        regClockPtr->MdClken |= CKE_TM3;

        timer3CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    #if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT9 | BIT8)) | BIT8;
        regClockPtr->MdClken |= CKE_TM4;

        timer4CallBackFunctionPtr = timerCallBackFuncPtr;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
        regClockPtr->TmClkC &= 0xFFE0FFFF;          // no divide
        //regClockPtr->TmClkC |= 0x1F << 16;          // divid by (n+1)
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT11 | BIT10)) | BIT10;
        regClockPtr->MdClken |= CKE_TM5;

        timer5CallBackFunctionPtr = timerCallBackFuncPtr;
        break;
    #endif

    default:
        MP_ALERT("--E-- TimerInit - Wrong Timer Index Number - %d!!!", timerIndex);

        return -2;
        break;
    }
#endif

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while ((regTimerPtr->TmC & BIT0) != 0)
        regTimerPtr->TmC = 0;
#else
    regTimerPtr->TmC = 0;
#endif

    regTimerPtr->Tpc = tpc;
    regTimerPtr->TcB = tcb;
    regTimerPtr->TcA = tcb;
    regTimerPtr->TmV = 0;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while ((regTimerPtr->TmC & BIT0) != BIT0)
        regTimerPtr->TmC = 0x00400003;
#else
    regTimerPtr->TmC = 0x00400003;
#endif

    MP_DEBUG("TcA = 0x%X", regTimerPtr->TcA);
    MP_DEBUG("TcB = 0x%X", regTimerPtr->TcB);
    MP_DEBUG("Tpc = 0x%08X, 0x%08X", regTimerPtr->Tpc, tpc);
    MP_DEBUG("Tmc = 0x%08X", regTimerPtr->TmC);

    if ((regTimerPtr->Tpc >> 16) != (tpc >> 16))
    {
        BYTE i = 0;

        for (i = 0; i < 3; i++)
            MP_ALERT("Timer%d - The issue of PCE can't read out still exist !!!!", timerIndex);

        __asm("break 100");
    }

    TimerIntEnable(timerIndex);

    return NO_ERR;
}



SDWORD TimerDeInit(BYTE timerIndex)
{
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;

    MP_DEBUG("De-Initial Timer%d -", timerIndex);

    TimerIntDisable(timerIndex);

    switch (timerIndex)
    {
    case 1:
        regClockPtr->MdClken &= ~CKE_TM1;

        timer1CallBackFunctionPtr = nullTimerProc;
        break;

    case 2:
        regClockPtr->MdClken &= ~CKE_TM2;

        timer2CallBackFunctionPtr = nullTimerProc;
        break;

    case 3:
        regClockPtr->MdClken &= ~CKE_TM3;

        timer3CallBackFunctionPtr = nullTimerProc;
        break;

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
    case 4:
        regClockPtr->MdClken &= ~CKE_TM4;

        timer4CallBackFunctionPtr = nullTimerProc;
        break;

    case 5:
        regClockPtr->MdClken &= ~CKE_TM5;

        timer5CallBackFunctionPtr = nullTimerProc;
        break;
#endif

    default:
        MP_ALERT("--E-- TimerDeInit - Wrong Timer Index Number - %d!!!", timerIndex);

        return -1;
        break;
    }

    return NO_ERR;
}



//---------------------------------------------------------------------------
//
//  Timer 1 - Used for UI task
//
//---------------------------------------------------------------------------
static void tm1Isr(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER1_BASE;
    DWORD tmp, pce, pcv, tick;

    pce = regTimerPtr->Tpc >> 16;
    pcv = regTimerPtr->Tpc & 0xFFFF;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & ~0xFFF8FFFF)
        regTimerPtr->TmC &= 0xFFF8FFFF;     // TPC_IC, TC_B_IC, TC_A_IC
#else
    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
#endif

    if (pcv < pce)
    {   // overfollow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / pce;
        tmp %= pce;
    }
    else
    {
        tmp = pcv % pce;
        tick = pcv / pce;
    }

    pce = (pce << 16) + tmp;
    regTimerPtr->Tpc = pce;
    timer1CallBackFunctionPtr(tick);
}



void Tm1RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD))
{
    if (timerCallBackFuncPtr)
        timer1CallBackFunctionPtr = timerCallBackFuncPtr;
    else
        timer1CallBackFunctionPtr = nullTimerProc;
}



void Tm1ClearCallBackFunc(void)
{
    TimerIntDisable(1);
    timer1CallBackFunctionPtr = nullTimerProc;
}



//---------------------------------------------------------------------------
//
//  Timer 2
//
//---------------------------------------------------------------------------
void InitTimer2(DWORD usec)
{
    TimerInit(2, usec, NULL);
}



static void tm2Isr(void)
{
    TIMER *regTimerPtr = (TIMER *) TIMER2_BASE;
    DWORD tmp, pce, pcv, tick;

    pce = regTimerPtr->Tpc >> 16;
    pcv = regTimerPtr->Tpc & 0xFFFF;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & ~0xFFF8FFFF)
        regTimerPtr->TmC &= 0xFFF8FFFF;     // TPC_IC, TC_B_IC, TC_A_IC
#else
    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
#endif

    if (pcv < pce)
    {   // overfollow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / pce;
        tmp %= pce;
    }
    else
    {
        tmp = pcv % pce;
        tick = pcv / pce;
    }

    pce = (pce << 16) + tmp;
    regTimerPtr->Tpc = pce;

#if (CHIP_VER_MSB == CHIP_VER_615)
    if (dwTimer2Cycle)
    {
        if (dwTimer2Cnt)
        {
            dwTimer2Cnt--;

            return;
        }

        dwTimer2Cnt = dwTimer2Cycle;
    }
#endif

    timer2CallBackFunctionPtr(tick);
}



void Tm2RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD))
{
    if (timerCallBackFuncPtr)
        timer2CallBackFunctionPtr = timerCallBackFuncPtr;
    else
        timer2CallBackFunctionPtr = nullTimerProc;
}



void Tm2ClearCallBackFunc(void)
{
    TimerIntDisable(2);
    timer2CallBackFunctionPtr = nullTimerProc;
}



//---------------------------------------------------------------------------
//
//  Timer 3
//
//---------------------------------------------------------------------------
static void tm3Isr(void)
{
    register TIMER *regTimerPtr = (TIMER *) TIMER3_BASE;
    DWORD tmp, pce, pcv, tick;

    pce = regTimerPtr->Tpc >> 16;
    pcv = regTimerPtr->Tpc & 0xFFFF;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & ~0xFFF8FFFF)
        regTimerPtr->TmC &= 0xFFF8FFFF;     // TPC_IC, TC_B_IC, TC_A_IC
#else
    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
#endif

    if (pcv < pce)
    {   // overfollow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / pce;
        tmp %= pce;
    }
    else
    {
        tmp = pcv % pce;
        tick = pcv / pce;
    }

    pce = (pce << 16) + tmp;
    regTimerPtr->Tpc = pce;
    regTimerPtr->Tpc = pce;
    timer3CallBackFunctionPtr(tick);
}



void Tm3RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD))
{
    if (timerCallBackFuncPtr)
        timer3CallBackFunctionPtr = timerCallBackFuncPtr;
    else
        timer3CallBackFunctionPtr = nullTimerProc;
}



void Tm3ClearCallBackFunc(void)
{
    TimerIntDisable(3);
    timer3CallBackFunctionPtr = nullTimerProc;
}



#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )

//---------------------------------------------------------------------------
//
//  Timer 4
//
//---------------------------------------------------------------------------
static void tm4Isr(void)
{
    register TIMER *regTimerPtr = (TIMER *) TIMER4_BASE;
    DWORD tmp, pce, pcv, tick;

    pce = regTimerPtr->Tpc >> 16;
    pcv = regTimerPtr->Tpc & 0xFFFF;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & ~0xFFF8FFFF)
        regTimerPtr->TmC &= 0xFFF8FFFF;     // TPC_IC, TC_B_IC, TC_A_IC
#else
    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
#endif

    if (pcv < pce)
    {   // overfollow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / pce;
        tmp %= pce;
    }
    else
    {
        tmp = pcv % pce;
        tick = pcv / pce;
    }

    pce = (pce << 16) + tmp;
    regTimerPtr->Tpc = pce;
    timer4CallBackFunctionPtr(tick);
}



void Tm4RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD))
{
    if (timerCallBackFuncPtr)
        timer4CallBackFunctionPtr = timerCallBackFuncPtr;
    else
        timer4CallBackFunctionPtr = nullTimerProc;
}



void Tm4ClearCallBackFunc(void)
{
    TimerIntDisable(4);
    timer4CallBackFunctionPtr = nullTimerProc;
}



//---------------------------------------------------------------------------
//
//  Timer 5
//
//---------------------------------------------------------------------------

static void tm5Isr(void)
{
    register TIMER *regTimerPtr = (TIMER *) TIMER5_BASE;
    DWORD tmp, pce, pcv, tick;

    pce = regTimerPtr->Tpc >> 16;
    pcv = regTimerPtr->Tpc & 0xFFFF;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & ~0xFFF8FFFF)
        regTimerPtr->TmC &= 0xFFF8FFFF;     // TPC_IC, TC_B_IC, TC_A_IC
#else
    regTimerPtr->TmC &= 0xFFF8FFFF;         // TPC_IC, TC_B_IC, TC_A_IC
#endif

    if (pcv < pce)
    {   // overfollow
        tmp = MAX_PCV_COUNT + 1 + pcv;
        tick = tmp / pce;
        tmp %= pce;
    }
    else
    {
        tmp = pcv % pce;
        tick = pcv / pce;
    }

    pce = (pce << 16) + tmp;
    regTimerPtr->Tpc = pce;
    timer5CallBackFunctionPtr(tick);
}



void Tm5RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD))
{
    if (timerCallBackFuncPtr)
        timer5CallBackFunctionPtr = timerCallBackFuncPtr;
    else
        timer5CallBackFunctionPtr = nullTimerProc;
}



void Tm5ClearCallBackFunc(void)
{
    TimerIntDisable(5);
    timer5CallBackFunctionPtr = nullTimerProc;
}



//---------------------------------------------------------------------------
//
// function for timer PWM, Timer2 ~ 5
//
//---------------------------------------------------------------------------
SDWORD TimerPwmDisable(BYTE timerIndex)
{
    if ((timerIndex < 2) || (timerIndex > 5))
        return -1;

    TimerDeInit(timerIndex);

    switch (timerIndex)
    {
    case 2:
        Gpio_ConfiguraionSet(GPIO_GPIO_2, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
        break;

    case 3:
        Gpio_ConfiguraionSet(GPIO_GPIO_3, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
        break;
#if (CHIP_VER_MSB == CHIP_VER_650)
    case 4:
        Gpio_ConfiguraionSet(GPIO_GPIO_5, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
        break;

    case 5:
        Gpio_ConfiguraionSet(GPIO_GPIO_6, GPIO_DEFAULT_FUNC, GPIO_INPUT_MODE, 0, 1);
        break;
#endif
    default:
        MP_ALERT("--E-- Wrong Timer Index Number - %d!!!", timerIndex);

        return -1;
        break;
    }

    return NO_ERR;
}



SDWORD TimerPwmEnable(BYTE timerIndex, DWORD pwmFreq, BYTE HighDuty)
{
    TIMER *regTimerPtr;
    CLOCK *regClockPtr = (CLOCK *) CLOCK_BASE;
    DWORD tcb;

    if (!pwmFreq || (timerIndex < 2) || (timerIndex > 5))
    {
        MP_ALERT("--E-- PWM Freq or Wrong Timer index - %d", timerIndex);

        return -2;
    }

    if (HighDuty > 100)
        HighDuty = 100;

    MP_DEBUG("Initial Timer%d as PWM -", timerIndex);

    if (calTimerParameterForPwm(pwmFreq, &tcb) != 0)
        return -1;

    TimerIntDisable(timerIndex);

    switch (timerIndex)
    {
    case 2:
        regTimerPtr = (TIMER *) TIMER2_BASE;
#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
        regClockPtr->TmClkC |= BIT5 | BIT4;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
        regClockPtr->TmClkC &= ~(BIT5 | BIT4);
#else
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT5 | BIT4)) | BIT4;
#endif
        regClockPtr->MdClken |= CKE_TM2;
        Gpio_ConfiguraionSet(GPIO_GPIO_2, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 1, 1);
        timer2CallBackFunctionPtr = nullTimerProc;
        break;

    case 3:
        regTimerPtr = (TIMER *) TIMER3_BASE;
#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
        regClockPtr->TmClkC |= BIT7 | BIT6;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
        regClockPtr->TmClkC &= ~(BIT7 | BIT6);
#else
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT7 | BIT6)) | BIT6;
#endif
        regClockPtr->MdClken |= CKE_TM3;
        Gpio_ConfiguraionSet(GPIO_GPIO_3, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 1, 1);
        timer3CallBackFunctionPtr = nullTimerProc;
        break;

#if (CHIP_VER_MSB == CHIP_VER_650)
    case 4:
        regTimerPtr = (TIMER *) TIMER4_BASE;
#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
        regClockPtr->TmClkC |= BIT9 | BIT8;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
        regClockPtr->TmClkC &= ~(BIT9 | BIT8);
#else
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT9 | BIT8)) | BIT8;
#endif
        regClockPtr->MdClken |= CKE_TM4;
        Gpio_ConfiguraionSet(GPIO_GPIO_5, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 1, 1);
        timer4CallBackFunctionPtr = nullTimerProc;
        break;

    case 5:
        regTimerPtr = (TIMER *) TIMER5_BASE;
#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
        regClockPtr->TmClkC |= BIT11 | BIT10;
#elif (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_CPU)
        regClockPtr->TmClkC &= ~(BIT11 | BIT10);
#else
        regClockPtr->TmClkC = (regClockPtr->TmClkC & ~(BIT11 | BIT10)) | BIT10;
#endif
        regClockPtr->MdClken |= CKE_TM5;
        Gpio_ConfiguraionSet(GPIO_GPIO_6, GPIO_ALT_FUNC_3, GPIO_OUTPUT_MODE, 1, 1);
        timer5CallBackFunctionPtr = nullTimerProc;
        break;
#endif
    }

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while (regTimerPtr->TmC & BIT0)
        regTimerPtr->TmC = 0;
#else
    regTimerPtr->TmC = 0;
#endif

    regTimerPtr->Tpc = 1 << 16;
    regTimerPtr->TcB = tcb;

    if (HighDuty)
        regTimerPtr->TcA = tcb * (100 - HighDuty) / 100;
    else
        regTimerPtr->TcA = tcb - 1;

    regTimerPtr->TmV = 0;

#if (TIMER_CLOCK_SOURCE == E_TIMER_SOURCE_IS_OSC)
    while ((regTimerPtr->TmC & BIT0) == 0)
        regTimerPtr->TmC = 0x00000001;
#else
    regTimerPtr->TmC = 0x00000001;
#endif

    MP_DEBUG("TcA = 0x%X", regTimerPtr->TcA);
    MP_DEBUG("TcB = 0x%X", regTimerPtr->TcB);
    MP_DEBUG("Tpc = 0x%X", regTimerPtr->Tpc);
    MP_DEBUG("Tmc = 0x%X", regTimerPtr->TmC);

    return NO_ERR;
}

#endif



////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////

#if Make_TESTCONSOLE
MPX_KMODAPI_SET(TimerInit);
MPX_KMODAPI_SET(TimerDeInit);
MPX_KMODAPI_SET(Tm1RegisterCallBackFunc);
MPX_KMODAPI_SET(Tm1ClearCallBackFunc);
MPX_KMODAPI_SET(Tm2RegisterCallBackFunc);
MPX_KMODAPI_SET(Tm2ClearCallBackFunc);
MPX_KMODAPI_SET(Tm3RegisterCallBackFunc);
MPX_KMODAPI_SET(Tm3ClearCallBackFunc);
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
MPX_KMODAPI_SET(Tm4RegisterCallBackFunc);
MPX_KMODAPI_SET(Tm4ClearCallBackFunc);
MPX_KMODAPI_SET(Tm5RegisterCallBackFunc);
MPX_KMODAPI_SET(Tm5ClearCallBackFunc);
#endif
MPX_KMODAPI_SET(TimerResume);
MPX_KMODAPI_SET(TimerPause);
MPX_KMODAPI_SET(TimerIntEnable);
MPX_KMODAPI_SET(TimerIntDisable);
#endif

