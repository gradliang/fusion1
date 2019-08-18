/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2005, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:    RTC.h
*
* Programmer:    Fengrs
*                MPX E320 division
*
* Created: 22/03/2007
*
* Description:
* Version : 0001
*
****************************************************************
*/
#ifndef __RTC_H
#define __RTC_H

/*
// Include section
*/
#include "UtilTypedef.h"
#include "bitsdefine.h"
#include "iplaysysconfig.h"
#include "peripheral.h"

/*
//Variable declaration
*/
/*
//Macro declaration
*/

/*
//Function prototype
*/
extern void UpdateClock(void);
extern void Cal_PhotoUpdate(void);
extern void ShowCalendarLeft(ST_IMGWIN * psWin, WORD year, BYTE month, BYTE day);
extern void AnaClockStart(void);

void CalendarBlackProcess(void);
void CalendarWhiteProcess(void);
void DigitClockProcess(void);

#if VCALENDAR
/*
// neiladd for vcam
*/
typedef struct {
    volatile BYTE Seconds;
    volatile BYTE Minutes;
    volatile BYTE Hours;
    volatile BYTE Days;
    volatile BYTE Weekdays;
    volatile BYTE Months;
    volatile BYTE Years;
} VCALENDAR_EVENT_TIME;

typedef struct {
    DWORD m_dwFastSearchTag;            //exp: 071101
    BYTE m_EventCount;                  //1当天的事件数
    VCALENDAR_EVENT_TIME m_stVcalendarEventTag[6];
} VCALENDAR_EVENT_TIME_TAG;

void VCalendarGetDayIndex(WORD, BYTE, BYTE, WORD *, WORD *);
void VCalendarEventTimeSet(VCALENDAR_EVENT_TIME_TAG *, BYTE, BYTE) ;
void RTC_SetAlarmTime(WORD);
#endif

ST_SYSTEM_TIME *xpgGetBuildTime();

#endif      // #ifndef __RTC_H

