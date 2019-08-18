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
* Filename      : hal_Timer.h
* Programmer(s) : TY Miao
* Created       : TY Miao
* Descriptions  :
*******************************************************************************
*/
#ifndef __HAL_TIMER_H
#define __HAL_TIMER_H

////////////////////////////////////////////////////////////
//
// Include section
//
////////////////////////////////////////////////////////////

#include "UtilTypedef.h"
#include "UtilRegfile.h"
#include "bitsdefine.h"
#include "iplaysysconfig.h"

////////////////////////////////////////////////////////////
//
// Constant declarations
//
////////////////////////////////////////////////////////////
#define E_TIMER_SOURCE_IS_OSC               0
#define E_TIMER_SOURCE_IS_CPU               1
#define E_TIMER_SOURCE_IS_AUX0              2

#if (CHIP_VER_MSB == CHIP_VER_615)
    #define MAX_TCB_COUNT                   0xFFFF
    #define MAX_PCV_COUNT                   0xFF
    #define TIMER_CLOCK_SOURCE              E_TIMER_SOURCE_IS_CPU
#else   // MP650/660
    #if (CHIP_VER_MSB == CHIP_VER_650)
        // AUX0 was used by HUart-B, don't choose E_TIMER_SOURCE_IS_AUX0
        #if (CHIP_VER_LSB == CHIP_VER_FPGA)
        #define TIMER_CLOCK_SOURCE          E_TIMER_SOURCE_IS_CPU
        #else
        #define TIMER_CLOCK_SOURCE          E_TIMER_SOURCE_IS_OSC
        #endif
    #else   // MP66x
        #define TIMER_CLOCK_SOURCE          E_TIMER_SOURCE_IS_AUX0
    #endif

///
///@ingroup     TIMER_MODULE
///@brief       define the max count of the pulse period.\n
///             When CHIP_VER is MP650, the value of MAX_TCB_COUNT is \b 0xFFFFFFFF.\n
///             When CHIP_VER is MP615, the value of MAX_TCB_COUNT is \b 0xFFFF.
    #define MAX_TCB_COUNT           0xFFFFFFFF

///
///@ingroup     TIMER_MODULE
///@brief       define the max count of the pulse.\n
///             When CHIP_VER is MP650, the value of MAX_PCV_COUNT is \b 0xFFFF.\n
///             When CHIP_VER is MP615, the value of MAX_PCV_COUNT is \b 0xFF.
    #define MAX_PCV_COUNT           0xFFFF

#endif

//////These macro are used by timer2 and only for video. (For MP615 platform only)
#define VIDEO_TIMER_TPC_PERIOD      1000    // us per TPC


////////////////////////////////////////////////////////////
//
// Function prototype
//
////////////////////////////////////////////////////////////

///
///@ingroup TIMER_MODULE
///@brief   Initial timer module
///
///@param   timerIndex              Timer Index\n
///                                 1~5: Timer1~5 when cip is MP650.
///                                 1~2: Timer1~2 when cip is MP615.
///@param   usec                    Timer interrupt period, unit is us.
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  NO_ERR
///@retval  -1      timer period is out of scale.
///@retval  -2      Wrong number of timer index.
///
///@remark  Timer0 was reserved by system timer.
///
SDWORD TimerInit(BYTE timerIndex, DWORD usec, void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Deinitial timer module
///
///@param   timerIndex  see TimerInit
///
///@retval  NO_ERR
///@retval  -1      Wrong number of timer index.
///
///@remark  Timer0 was reserved by system timer.
///
SDWORD TimerDeInit(BYTE timerIndex);

///
///@ingroup TIMER_MODULE
///@brief   Register/Change callback function for timer1's interrupt
///
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  None
///
///@remark
///
void Tm1RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Clean callback function for timer1's interrupt
///
///@param   None
///
///@retval  None
///
///@remark
///
void Tm1ClearCallBackFunc(void);

///
///@ingroup TIMER_MODULE
///@brief   Register/Change callback function for timer2's interrupt
///
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  None
///
///@remark
///
void Tm2RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Clean callback function for timer2's interrupt
///
///@param   None
///
///@retval  None
///
///@remark
///
void Tm2ClearCallBackFunc(void);

///
///@ingroup TIMER_MODULE
///@brief   Register/Change callback function for timer3's interrupt
///
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  None
///
///@remark
///
void Tm3RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Clean callback function for timer3's interrupt
///
///@param   None
///
///@retval  None
///
///@remark
///
void Tm3ClearCallBackFunc(void);

///
///@ingroup TIMER_MODULE
///@brief   Register/Change callback function for timer4's interrupt
///
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  None
///
///@remark  Only for MP650/660 serial
///
void Tm4RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Clean callback function for timer4's interrupt
///
///@param   None
///
///@retval  None
///
///@remark  Only for MP650/660 serial
///
void Tm4ClearCallBackFunc(void);

///
///@ingroup TIMER_MODULE
///@brief   Register/Change callback function for timer5's interrupt
///
///@param   *timerCallBackFuncPtr   callback function's pointer, prototype is void (*)(WORD)
///
///@retval  None
///
///@remark  Only for MP650/660 serial
///
void Tm5RegisterCallBackFunc(void (*timerCallBackFuncPtr)(WORD));

///
///@ingroup TIMER_MODULE
///@brief   Clean callback function for timer5's interrupt
///
///@param   None
///
///@retval  None
///
///@remark  Only for MP650/660 serial
///
void Tm5ClearCallBackFunc(void);

///
///@ingroup TIMER_MODULE
///@brief   Pause Timer function (Disable Timer's interrupt and stop count)
///
///@param   timerIndex  See TimerInit
///
///@retval  None
///
///@remark
///
void TimerPause(BYTE timerIndex);

///
///@ingroup TIMER_MODULE
///@brief   Resume Timer function (Enable Timer's interrupt and start count)
///
///@param   timerIndex  See TimerInit
///@param   cleanCount  0: Keep current timer's count.\n
///                     1: Clean current timer's count.
///
///@retval  None
///
///@remark
///
void TimerResume(BYTE timerIndex, BYTE cleanCount);

///
///@ingroup TIMER_MODULE
///@brief   Enable HW PWM function.
///
///@param   timerIndex  See TimerInit. Only for timer2~timer5
///@param   pwmFreq     PWM's frequency, unit is Hz.
///@param   HighDuty    PWM's duty, 0~100.\n
///                     0:      always low
///                     50:     50% high, 50% low
///                     100:    always high
///
///@retval  NO_ERR
///@retval  -1          zero PWM freq or wrong timer index.
///@retval  -2          Frequence out of range.
///
///@remark
///
SDWORD TimerPwmEnable(BYTE timerIndex, DWORD pwmFreq, BYTE HighDuty);

///
///@ingroup TIMER_MODULE
///@brief   Disable HW PWM function.
///
///@param   timerIndex  See TimerPwmEnable
///
///@retval  NO_ERR
///@retval  -1          Wrong timer index.
///
///@remark
///
SDWORD TimerPwmDisable(BYTE timerIndex);

///
///@ingroup TIMER_MODULE
///@brief   Enable timer's interrupt.
///
///@param   timerIndex  Timer Index\n
///                     0: Timer0 was reserved for system timer.\n
///                     1~5: Timer1~5.
///
///@retval  NO_ERR
///
///@remark
///
void TimerIntEnable(BYTE timerIndex);

///
///@ingroup TIMER_MODULE
///@brief   Disable timer's interrupt.
///
///@param   timerIndex  see TimerIntEnable
///
///@retval  NO_ERR
///
///@remark
///
void TimerIntDisable(BYTE timerIndex);

// wrapper
void InitTimer2(DWORD usec);

#endif  // __HAL_TIMER_H

