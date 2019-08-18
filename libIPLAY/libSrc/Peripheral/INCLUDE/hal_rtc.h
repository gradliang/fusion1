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
* Filename      : hal_RTC.h
* Programmer(s) :
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/
#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#include "UtilTypedef.h"
#include "bitsdefine.h"
#include "iplaysysconfig.h"

#ifndef Make_EMBEDDED_RTC
    #define Make_EMBEDDED_RTC       1
#endif

#ifndef RTC_AUTO_CORRECT_EN
    #define RTC_AUTO_CORRECT_EN     0
#endif

///
///@ingroup     RTC_MODULE
///@brief       Register callback function for RTC's interrupt.
///
///@param       isrFuncPtr  callback function's pointer, prototype is void (*)(void)
///
///@retval      NO_ERR                      No any error
///
///@remark
///
void RTC_RegisterRtcIsrCallback(void (*isrFuncPtr)(void));

///
///@ingroup     RTC_MODULE
///@brief       RTC count setting function.
///
///@param       value       second count
///
///@retval      None
///
///@remark
///
void RTC_SetCount(DWORD value);

///
///@ingroup     RTC_MODULE
///@brief       RTC alarm setting function.
///
///@param       value   Alarm's second count
///
///@retval      None
///
///@remark
///
void RTC_AlarmSet(DWORD value);

///
///@ingroup     RTC_MODULE
///@brief       RTC alarm interrupt enable or disable.(out pin triggered)
///
///@param       enable  ENABLE(1)/DISABLE(0) the interrupt of alarm.
///
///@retval      None
///
///@remark
///
void RTC_AlarmEnableSet(BOOL enable);

///
///@ingroup     RTC_MODULE
///@brief       Configure RTC's alarm-out pin
///
///@param       mode        pin function of RALARM
///                         0/1: RTC alarm out
///                         2: GPIN(GP-8)
///                         3: GPOUT
///             dataHigh    RALARM output vlue is high when mode = 3 (1:H / 0:L)
///@retval      None
///
///@remark      open drain
///
void RTC_RAlarmModeSet(BYTE mode, BOOL dataHigh);

///
///@ingroup     RTC_MODULE
///@brief       Configure RTC's alarm-out pin to GPIO output mode
///
///@param       dataHigh    Output high(1) / low(0)
///
///@retval      None
///
///@remark      open drain
///
void RTC_AlarmOutSet(BOOL dataHigh);

///
///@ingroup     RTC_MODULE
///@brief       RTC alarm IE Enable or Disable.
///
///@param       enable  ENABLE(1)/DISABLE(0) the interrupt of alarm IE.
///
///@retval      None
///
///@remark
///
void RTC_AlarmIeEnableSet(BOOL enable);

///
///@ingroup     RTC_MODULE
///@brief       RTC low power Enable or Disable.
///
///@param       enable  ENABLE(1)/DISABLE(0) the low power mode of RTC.
///
///@retval      None
///
///@remark
///
void RTC_LowPowerEnable(BOOL enable);

///
///@ingroup     RTC_MODULE
///@brief       Read current RTC's second count.
///
///@param       None
///
///@retval      RTC's second count.
///
///@remark
///
DWORD RTC_ReadCount(void);

///
///@ingroup     RTC_MODULE
///@brief       Read current RTC's alarm second count.
///
///@param       None
///
///@retval      RTC's alarm second count.
///
///@remark
///
DWORD RTC_ReadAlarm(void);

void RTC_InternalLdoEnable(BOOL enable);
void RTC_SecCountToleranceAdj(DWORD adjPeriod, BOOL direction, DWORD tolerance);
void RTC_SetReservedCount(void);
DWORD RTC_ReadReservedCount(void);

#endif      // #define __HAL_RTC_H

