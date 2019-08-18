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
* Filename      : hal_rtc.c
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  : Real time clock Module
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

#include "mpTrace.h"

#include "iplaysysconfig.h"
#include "UtilRegFile.h"
#include "hal_rtc.h"
#include "hal_gpio.h"


/*************************************************************************/
/***               Internally Visible Constants and Static Data        ***/
/*************************************************************************/
#if ( (CHIP_VER == (CHIP_VER_650 | CHIP_VER_A)) || (CHIP_VER == (CHIP_VER_650 | CHIP_VER_FPGA)) )
#define __USING_SYSTEM_TICK__                   // simulate RTC by system timer

volatile DWORD rtcTickCounter = 0;
volatile DWORD rtcSecCounter = 0;
#endif

#define RTC_PREDEFIND_CODE      0x005C0000
#define RTC_DELAY_COUNT         1000

// RTC_MODEN
#define RTC_ID_CODE             0x1710
// RTC_CTL
#define RTC_LOAD                BIT0
#define ALARM_EN                BIT1
#define ALARM_IE                BIT2
#define LOW_POWER_EN            BIT3
#define ADJ_PER_SEC_SEL         BIT15
// RTC_EXT0
#define INTERNAL_LDO_EN         BIT5            // 0: Enable, 1:Disable

#define RTC_ID_CODE_R           0x00FF0000

static void nullRtcCallbackFunc(void);
static void (*rtcIsrCallbackPtr)(void) = nullRtcCallbackFunc;

static rtcDelayWait(void)
{   // at least one RTC clock, 1/32768s = 31us
    volatile DWORD i;

    for (i = 0; i < RTC_DELAY_COUNT; i++);
}



static void nullRtcCallbackFunc(void)
{
    MP_ALERT("--E-- Null RTC call function pointer !!!");
}



static void rtcIsr(void)
{
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    regRtcPtr->RtcCtl &= ~ALARM_IE;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;

    if (rtcIsrCallbackPtr)
        rtcIsrCallbackPtr();
#endif
}



void RTC_RegisterRtcIsrCallback(void (*isrFuncPtr)(void))
{
#ifndef __USING_SYSTEM_TICK__
    if (isrFuncPtr)
        rtcIsrCallbackPtr = isrFuncPtr;
    else
        rtcIsrCallbackPtr = nullRtcCallbackFunc;
#endif
}



void RTC_AlarmSet(DWORD value)
{
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcAlarm = value;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;

    if (regRtcPtr->RtcAlarm != value)
        MP_ALERT("--E-- RTC_AlarmSet fail !!!");

    IntEnable();
#endif
}



void RTC_AlarmEnableSet(BOOL enable)
{
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();

    // Config RALARM to RTC ALARM OUT
    regRtcPtr->RtcExt0 &= 0xFFFFFFF9;
    regRtcPtr->RtcExt0 |= BIT0;

    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~ALARM_EN;     // reset
    rtcDelayWait();

    if (enable)
    {
        regRtcPtr->RtcCtl |= ALARM_EN;
        rtcDelayWait();
    }

    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#endif
}



void RTC_AlarmOutSet(BOOL dataHigh)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    DWORD rtcExt0;

    IntDisable();
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
#if (CHIP_VER_MSB == CHIP_VER_650)      // except to MP615 and MP66x
    rtcExt0 = (regRtcPtr->RtcExt0 & 0xFFFFFFF0) | BIT2 | BIT1 | BIT0;

    if (dataHigh == TRUE)
        rtcExt0 |= BIT3;

    regRtcPtr->RtcExt0 = rtcExt0;
#else
    regRtcPtr->RtcExt0 |= 0x0000000F;
#endif
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#endif
#endif
}



void RTC_RAlarmModeSet(BYTE mode, BOOL dataHigh)
{
#if (CHIP_VER_MSB != CHIP_VER_615)
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    DWORD rtcExt0;

    IntDisable();
    mode &= 0x03;
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
#if (CHIP_VER_MSB == CHIP_VER_650)      // except to MP615 and MP66x
    rtcExt0 = (regRtcPtr->RtcExt0 & 0xFFFFFFF0) | ((mode & 0x03) << 1);

    if (mode != 2)
        rtcExt0 |= BIT0;        // direction is output

    if (dataHigh == TRUE)
        rtcExt0 |= BIT3;

    regRtcPtr->RtcExt0 = rtcExt0;
#else
    regRtcPtr->RtcExt0 |= 0x0000000F;
#endif

    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#endif
#endif
}



void RTC_AlarmIeEnableSet(BOOL enable)
{
#ifndef __USING_SYSTEM_TICK__
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    regRtcPtr->RtcCtl &= ~ALARM_IE;
    rtcDelayWait();

    if (enable > 0)
    {
        regRtcPtr->RtcCtl |= ALARM_IE;
        rtcDelayWait();
    }

    regRtcPtr->RtcModeEna = 0;

    if (enable > 0)
    {
        SystemIntEna(IM_RTC);
        SystemIntHandleRegister(IM_RTC, rtcIsr);
    }
    else
    {
        SystemIntDis(IM_RTC);
    }

    IntEnable();
#endif
}



void RTC_LowPowerEnable(BOOL enable)
{
/*
#ifndef __USING_SYSTEM_TICK__
#if (CHIP_VER_MSB != CHIP_VER_615)
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    DWORD lowPowerEn = LOW_POWER_EN;

    if (enable == TRUE)
        lowPowerEn = 0;

    IntDisable();

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcCtl = (regRtcPtr->RtcCtl & ~LOW_POWER_EN) | lowPowerEn;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#else
    MP_ALERT("--E-- MP%X not support %s", CHIP_VER_MSB >> 16, __FUNCTION__);
#endif
#endif
*/
}



void RTC_InternalLdoEnable(BOOL enable)
{
/*
#ifndef __USING_SYSTEM_TICK__
#if (CHIP_VER_MSB != CHIP_VER_615)
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    DWORD interLdoEn = INTERNAL_LDO_EN;

    if (enable == TRUE)
        interLdoEn = 0;     // Low enable internal LDO

    IntDisable();

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcExt0 = (regRtcPtr->RtcExt0 & ~INTERNAL_LDO_EN) | interLdoEn;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#else
    MP_ALERT("--E-- MP%X not support %s", CHIP_VER_MSB >> 16, __FUNCTION__);
#endif
#endif
*/
}



DWORD RTC_ReadAlarm(void)
{
#ifdef __USING_SYSTEM_TICK__
    return 0;
#else
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    return regRtcPtr->RtcAlarm;
#endif
}



// adjPeriod: n RTC second count per time, avolid value of n are 0,1,2,4,8,16,32,64,...,32768
// direction: 0 => decrease, 1: increase
// tolerance: RTC clock adjust per time
void RTC_SecCountToleranceAdj(DWORD adjPeriod, BOOL direction, DWORD tolerance)
{
#ifndef __USING_SYSTEM_TICK__
#if (CHIP_VER_MSB != CHIP_VER_615)
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    DWORD settingValue;

    if (adjPeriod > 0x8000)
        adjPeriod = 0x8000;

    if (tolerance > 2047)
    {
        MP_ALERT("--E-- out of range to compensation, max value is 2047!!!");
        MP_ALERT("--E-- Decrease compensation time to reduce tolerance !!!");
        tolerance = 2047;
    }

    if (direction)
        direction = 1;
    else
        direction = 0;

    if (tolerance == 0)
    {
        adjPeriod = 0;
        direction = 0;
    }

    settingValue = adjPeriod <<= 16;
    settingValue |= ((DWORD) direction) << 15;
    settingValue |= tolerance << 4;

    IntDisable();
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcCtl = (regRtcPtr->RtcCtl & 0x0000000E) | settingValue;
    rtcDelayWait();
    //mpDebugPrint("regRtcPtr->RtcCtl = 0x%08X, 0x%08X", regRtcPtr->RtcCtl & 0xFFFFFFF0, settingValue);
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#else
    MP_ALERT("--E-- MP%X not support %s", CHIP_VER_MSB >> 16, __FUNCTION__);
#endif
#endif
}



void RTC_SetCount(DWORD value)
{
#ifdef __USING_SYSTEM_TICK__
    rtcSecCounter = value;
#else
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    MP_DEBUG("RTC_SetCount - New RTC count is 0x%08X", value);

    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
    regRtcPtr->RtcCnt = value;
#if (RTC_AUTO_CORRECT_EN == 1)
    regRtcPtr->RtcExt1 = value;
#endif
    rtcDelayWait();
    regRtcPtr->RtcCtl |= RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;

    if ((regRtcPtr->RtcCnt > (value + 1)) || (regRtcPtr->RtcCnt < value))
        MP_ALERT("RTC_SetCount fail !!!");

    //MP_DEBUG("Real RTC count is 0x%08X", regRtcPtr->RtcCnt);

    IntEnable();
#endif
}


void RTC_SetExt1(DWORD value)
{
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    MP_DEBUG("RTC_SetExt1: New RTC Ext1 is 0x%08X", value);
    
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
#if (RTC_AUTO_CORRECT_EN == 1)
    regRtcPtr->RtcExt1 = value;
#endif
    rtcDelayWait();
    regRtcPtr->RtcCtl |= RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;

    if ((regRtcPtr->RtcExt1 > (value + 1)) || (regRtcPtr->RtcExt1 < value))
        MP_ALERT("RTC_SetExt1 fail !!!");

    //MP_DEBUG("Real RTC Ext1 is 0x%08X", regRtcPtr->RtcExt1);

    IntEnable();
}

void RTC_SetExt0(DWORD value)
{
    MP_DEBUG("%s", __func__);
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();

    MP_DEBUG("RTC_SetExt0: New RTC Ext0 is 0x%08X", value);
    
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
#if (RTC_AUTO_CORRECT_EN == 1)
    regRtcPtr->RtcExt0 = value;
#endif
    rtcDelayWait();
    regRtcPtr->RtcCtl |= RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcCtl &= ~RTC_LOAD;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;

    if ((regRtcPtr->RtcExt0 > (value + 1)) || (regRtcPtr->RtcExt0 < value))
        MP_ALERT("RTC_SetExt0 fail !!!");

    MP_DEBUG("Real RTC Ext0 is 0x%08X", regRtcPtr->RtcExt0);

    IntEnable();
}


DWORD RTC_ReadExt0(void)
{
    MP_DEBUG("%s", __func__);
#ifdef __USING_SYSTEM_TICK__
    return rtcSecCounter;
#else
    RTC *regRtcPtr = (RTC *) RTC_BASE;
    return regRtcPtr->RtcExt0;
    //return 0xAB546798; // shift left 1 bit - test data
    //return 0x56A8CF30; // shift left 2 bit - test data
#endif
}


DWORD RTC_ReadCount(void)
{
    MP_DEBUG("%s", __func__);
#ifdef __USING_SYSTEM_TICK__
    return rtcSecCounter;
#else
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    return regRtcPtr->RtcCnt;
#endif
}



void RTC_SetReservedCount(void)
{
#ifdef __USING_SYSTEM_TICK__
    //rtcSecCounter = value;
    return;
#elif (RTC_AUTO_CORRECT_EN == 1)
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    IntDisable();
    regRtcPtr->RtcModeEna = RTC_PREDEFIND_CODE;
    rtcDelayWait();
    regRtcPtr->RtcExt1 = regRtcPtr->RtcCnt;
    rtcDelayWait();
    regRtcPtr->RtcModeEna = 0;
    IntEnable();
#else
    __asm("break 100");;
#endif
    MP_DEBUG("%s: regRtcPtr->RtcExt1 = 0x%08X", __func__, regRtcPtr->RtcExt1);
}



DWORD RTC_ReadReservedCount(void)
{
    MP_DEBUG("%s", __func__);
#ifdef __USING_SYSTEM_TICK__
    return rtcSecCounter;
#elif (RTC_AUTO_CORRECT_EN == 1)
    RTC *regRtcPtr = (RTC *) RTC_BASE;

    return regRtcPtr->RtcExt1;
#else
    __asm("break 100");;
#endif
}

////////////////////////////////////////////////////////////////////
//
// Add here for test console
//
////////////////////////////////////////////////////////////////////
#if Make_TESTCONSOLE
MPX_KMODAPI_SET(RTC_AlarmEnableSet);
MPX_KMODAPI_SET(RTC_AlarmIeEnableSet);
MPX_KMODAPI_SET(RTC_AlarmSet);
MPX_KMODAPI_SET(RTC_LowPowerEnable);
MPX_KMODAPI_SET(RTC_ReadAlarm);
MPX_KMODAPI_SET(RTC_ReadCount);
MPX_KMODAPI_SET(RTC_RegisterRtcIsrCallback);
MPX_KMODAPI_SET(RTC_SetCount);
#endif

