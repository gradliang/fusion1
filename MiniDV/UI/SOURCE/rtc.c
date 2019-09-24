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
* Filename      : rtc.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

#include "global612.h"
#include "xpg.h" // for g_stXpgMovie DigitClockShow()
#include "utiltypedef.h"
#include "utilregfile.h"
#include "mpTrace.h"
#include "os.h"
#include "rtc.h"
#include "display.h"
#include "taskid.h"
#include "ui.h"

#include "../../main/include/ui_timer.h"
#include "../../main/include/setup.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if RTC_ENABLE

static void uiRtcIsr(void);

enum {
    RTC_NONE_MODE = 0,
    RTC_RESETCNT_MODE,
    RTC_ALARM_MODE,
#if VCALENDAR
    RTC_VCALENDAR_EVENT_MODE, //neiladd
#endif
};

#define CLOCK_COLOR             6
#define SETTING_COLOR           7
#define CALENDAR_COLOR          5
#define CALENDAR_TODAY_COLOR    6

#define CLOCK_X                 520
#define CLOCK_Y                 8

#define WEEKDAY_BAS             6   // THE week day of 2000.1.1 is 6

static const BYTE DaysTab[13] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static WORD CalendarXTab[7] = {30, 70, 110, 150, 190, 230, 270};
static WORD CalendarYTab[7] = {120, 160, 200, 240, 280, 320, 360};
static WORD WeekXTab[7] = {75, 175, 275, 375, 475, 575, 675};
static WORD WeekYTab[7] = {120, 173, 226, 279, 332, 385, 438}; //static WORD WeekYTab[7] = {120, 175, 230, 285, 340, 395, 450};
static const BYTE WeekDayStrTab[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const BYTE CalendayStrTab[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
static DWORD dwPrevMin = 0xffffffff, dwPrevSec=0xffffffff;
static BYTE bRtcDisplayEnable = FALSE;
static BYTE bRTC_Status = RTC_NONE_MODE;

ST_IMGWIN m_stClockWin[4]; // add second
extern BYTE g_bXpgStatus;

static void CalendarYearMonthDay(int monthOffset, int dayOffset, WORD *yearIn, BYTE *monthIn, BYTE *dayIn, BYTE *bDaysIn);
void ShowDayOfPhoto(BYTE x, BYTE y);
/////////////////////////////////////////////////////////////////////////////////
//               Timer setting
/////////////////////////////////////////////////////////////////////////////////
// display type DD.MM.20YY HH:MM:SS
#if 1
//#define RTC_CLK_Y       Y_position) //(440)
#define RTC_DAY_X       (540-100)
#define RTC_MONTH_X     (576-100)
#define RTC_YEAR_X      (612-100)
#define RTC_HOUR_X      (688-100)
#define RTC_MIN_X       (724-100)
#else
#define RTC_CLK_Y       440
#define RTC_DAY_X       540
#define RTC_MONTH_X     576
#define RTC_YEAR_X      612
#define RTC_HOUR_X      688
#define RTC_MIN_X       724
#endif

static const BYTE RtcGapChar[5] = {0x2e, 0x2e, 0x20, 0x3a, 0x3a};
static WORD RtcClkX[7] ;//= {RTC_DAY_X, RTC_MONTH_X, RTC_YEAR_X, RTC_HOUR_X, RTC_MIN_X, (RTC_MIN_X+34)};//byAlexWang 26apr2008
static BYTE RtcCurBuf[6];

//#define SETTING_RTC_Y       RTC_CLK_Y
#define SETTING_YEAR_X      RTC_YEAR_X
#define SETTING_MONTH_X     RTC_MONTH_X
#define SETTING_DAY_X       RTC_DAY_X
#define SETTING_HOUR_X      RTC_HOUR_X
#define SETTING_MIN_X       RTC_MIN_X

static BYTE RtcChgLen = 0;      // Used for display clock. How mamy bytes need redraw.
static BYTE SettingIndex;
static volatile BYTE RtcSettingBuf[5];
static const BYTE RtcSettingGapChar[5] = {0x2e, 0x2e, 0x20, 0x3a, 0};
static BYTE RtcSettingMaxVal[5] = {31, 12, 99, 23, 59};
static BYTE RtcSettingMinVal[5] = {1, 1, 0, 0, 0};
static WORD RtcSettingX[7];     //byAlexWang 26apr2008

WORD yearWeekStart, yearWeekEnd;
BYTE monthWeekStart, dayWeekStart, monthWeekEnd, dayWeekEnd;
static WORD CompareFileWeekDate(void * pElement1, void * pElement2, BOOL reverse_order);

///
///@ingroup xpgClock
///@brief   Get week day by year+month+day
///
///@param   year
///@param   month
///@param   day
///
///@retval BYTE : 0..6
///
///@remark  The year is from 2000(366 day)
///
static BYTE GetWeekDay(WORD year, BYTE month, BYTE day)
{
  MP_DEBUG("GetWeekDay");
  DWORD temp;
  BYTE i;

  if(year)
    temp = year * 365 + ((year - 1) >> 2) + 1;
  else
    temp = 0;

  for(i=1; i<month; i++)
    temp += DaysTab[i];

  if((month > 2) && (!(year & 0x03)))
    temp++;

  temp += (day - 1);

  temp += WEEKDAY_BAS;

  return (BYTE)(temp%7);
}


#define OSD_TIME_CLOCK_X    100
#define OSD_TIME_CLOCK_Y    560
#define OSD_TIME_CLOCK_W    260
#define OSD_TIME_CLOCK_H    32

///
///@ingroup xpgClock
///@brief   Clear Clock OSD display
///
void ClearClock()
{
    MP_DEBUG("ClearClock");
    Idu_OsdPaintArea(OSD_TIME_CLOCK_X, OSD_TIME_CLOCK_Y, OSD_TIME_CLOCK_W, OSD_TIME_CLOCK_H, 0);
}



///
///@ingroup xpgClock
///@brief   Draw rtc time string
///
void DrawRTC_TimeTag()
{
    MP_DEBUG("DrawRTC_TimeTag");
//byAlexWang 26apr2008
    if  (bRTC_Status == RTC_RESETCNT_MODE)
        Idu_OsdPutStr(Idu_GetOsdWin(), "Set Current Time: ", (RtcClkX[2] - DEFAULT_FONT_SIZE*15), RtcClkX[6], CLOCK_COLOR);
    else if (bRTC_Status == RTC_ALARM_MODE)
        Idu_OsdPutStr(Idu_GetOsdWin(), "Set Alarm Time: ", (RtcClkX[2] - DEFAULT_FONT_SIZE*14), RtcClkX[6], CLOCK_COLOR);
#if VCALENDAR
    else if (bRTC_Status == RTC_VCALENDAR_EVENT_MODE)
            Idu_OsdPutStr(Idu_GetOsdWin(), "Set Event Time: ", (RtcClkX[2] - DEFAULT_FONT_SIZE*14), RtcClkX[6], CLOCK_COLOR);
#endif
    else // Jonny off 20090424
        Idu_OsdPutStr(Idu_GetOsdWin(), "Current Time: ", (RtcClkX[2] - DEFAULT_FONT_SIZE*13), RtcClkX[6], CLOCK_COLOR);
//mpDebugPrint("bRTC_Status==%d",bRTC_Status);
}



///
///@ingroup xpgClock
///@brief   Show Clock OSD string
///
///@param   pExtRtc : specified string
///
void ShowClock(ST_SYSTEM_TIME *pExtRtc)
{ 
    WORD year, month, day;
    WORD hour, min, sec;
    BYTE buffer[128];
    ST_OSDWIN * pOsdWin;

    pOsdWin = Idu_GetOsdWin();
    
    if (pExtRtc->u16Year >= 2000 && pExtRtc->u16Year <= 2099)
        year = pExtRtc->u16Year;
    else
        year = 2012;

    if (pExtRtc->u08Month >= 1 && pExtRtc->u08Month <= 12)
        month = pExtRtc->u08Month;
    else
        month = 1;

    if (pExtRtc->u08Day >= 1 && pExtRtc->u08Day <= 31)
        day = pExtRtc->u08Day;
    else
        day = 1;

    if (pExtRtc->u08Hour <= 23)
        hour = pExtRtc->u08Hour;
    else
        hour = 23;

    if (pExtRtc->u08Minute <= 59)
        min = pExtRtc->u08Minute;
    else
        min = 59;

    if (pExtRtc->u08Second <= 59)
        sec = pExtRtc->u08Second;
    else
        sec = 59;

    sprintf(buffer, "%04d-%02d-%02d   %02d:%02d:%02d", year, month, day, hour, min, sec);
    ClearClock();

    Idu_OsdPutStr(pOsdWin, buffer, OSD_TIME_CLOCK_X, OSD_TIME_CLOCK_Y, 4);
}



///
///@ingroup xpgClock
///@brief   Update Clock OSD string
///
///@remark : The function only read second register, so if you do not call the function
///          call more than 60 second, the clock will be error.
///          static DWORD dwRtcCntBackup = 0xffffffff;
void UpdateClock(void)
{
#if (RTC_ENABLE)
    
#if (SC_USBDEVICE)
    if (SystemCheckUsbdPlugIn()) return;
#endif

    DWORD dwHash = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;
    if ( dwHash == xpgHash("Clock"))
	{
        //mpDebugPrint("UpdateClock4");
		//UpdateClock4();
        Ui_TimerProcAdd(200, UpdateClock);
	}
#endif
}



///
///@ingroup xpgClock
///@brief   Show Calendar in screen left
///
///@param   psWin : specified win
///@param   year
///@param   month
///@param   day
///
///@remark    The year is from 2000
void ShowCalendarLeft(ST_IMGWIN *psWin, WORD year, BYTE month, BYTE day)
{
    MP_DEBUG("ShowCalendarLeft");
    BYTE x, y, bDays, i;
    BYTE CalTmpBuf[3];
    WORD BackupCalendarXTab[7];
    WORD BackupCalendarYTab[7];
    for(i=0; i<7; i++)
    {
        BackupCalendarXTab[i] = CalendarXTab[i];
        BackupCalendarYTab[i] = CalendarYTab[i];
    }

    WORD screenLeft = g_psSystemConfig->sScreenSetting.pstCurTCON->wWidth / 2;
    WORD Y_position = (g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight == 600) ? 60 : 0;//for 800x600
    //mpDebugPrint("screenLeft=%d, Y_position=%d", screenLeft, Y_position);
    WORD weekWidth = screenLeft / 10;
    WORD weekWGap = (screenLeft - weekWidth*7) / 8;
    WORD weekH = g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight / 8;
    //mpDebugPrint("weekWidth=%d, weekWGap=%d, weekH=%d", weekWidth, weekWGap, weekH);
    for(i=0; i<7; i++)
        CalendarXTab[i] = weekWGap+(weekWidth+weekWGap)*i;
    for(i=0; i<7; i++)
        CalendarYTab[i] = weekH+i*weekH;

    x = GetWeekDay(year, month, 1);
    Idu_SetFontColor(225, 146, 0);

    for (i = 0; i < 7; i++)
        Idu_PrintString(psWin, (BYTE *) &(WeekDayStrTab[i]), CalendarXTab[i], CalendarYTab[0] + Y_position, 0, 0);

    if ( (month == 2) && LeapYearCheck(year) )
        bDays = 29;
    else
        bDays = DaysTab[month];

    //  y = day + x - 1;
    //  Idu_OsdPaintArea(CalendarXTab[(y%7)], CalendarYTab[(y/7)+1], 32, 32, CALENDAR_MARK);

    Idu_SetFontColor(50, 50, 50);
    CalTmpBuf[0] = 0x20;
    CalTmpBuf[2] = 0;
    y = 1;

    for (i = 0x31; i <= 0x39; i++)
    {
        CalTmpBuf[1] = i;

        if ((i & 0x0f) == day)
        {
            Idu_SetFontColor(225, 146, 0);
            Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), CalendarXTab[x], CalendarYTab[y]+ Y_position, 0,0);
            Idu_SetFontColor(50, 50, 50);
        }
        else
            Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), CalendarXTab[x], CalendarYTab[y]+ Y_position, 0,0);

        x++;

        if (x > 6)
        {
            x = 0;
            y++;
        }
    }

    CalTmpBuf[0] = 0x31;
    CalTmpBuf[1] = 0x30;

    for (i = 10; i <= bDays; i++)
    {
        if (CalTmpBuf[1] > 0x39)
        {
              CalTmpBuf[1] = 0x30;
              CalTmpBuf[0]++;
        }

        if (i == day)
        {
            Idu_SetFontColor(225, 146, 0);
            Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), CalendarXTab[x], CalendarYTab[y]+ Y_position, 0,0);
            Idu_SetFontColor(50, 50, 50);
        }
        else
            Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), CalendarXTab[x], CalendarYTab[y]+ Y_position, 0,0);

        CalTmpBuf[1]++;
        x++;

        if (x > 6)
        {
            x = 0;
            y++;
        }
    }

    mpSetFontColor(0);
    for(i=0; i<7; i++)
    {
        CalendarXTab[i] = BackupCalendarXTab[i];
        CalendarYTab[i] = BackupCalendarYTab[i];
    }
}



///
///@ingroup xpgClock
///@brief   Get day of photo
///
///@param   year
///@param   month
///@param   day
///@param    wStartIndex - call by reference , return value
///
///@retval  WORD - matched count
///
WORD GetDayOfPhoto(WORD year, BYTE month, BYTE day, WORD *wStartIndex)
{
    //mpDebugPrint("GetDayOfPhoto(year=%d, month=%d, day=%d)", year, month, day);
    DWORD matched_count, start_idx_in_list, matched_folder_count;
    year += 2000; //WORD year=2009, month=7, day=17;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    SetCurFileSortingBasis(FILE_SORTING_BY_DATE_TIME);
    SWORD ret = SearchFileList_for_Date((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE],
                             psFileBrowser->dwFileListCount[OP_IMAGE_MODE],
                             year, month, day, &matched_count, &start_idx_in_list, &matched_folder_count);
    if(ret == PASS) //if (matched_count > 0)
        ; //mpDebugPrint("(%d%02d%02d): matched_count=%d, start_index=%d, folder_count=%d", year, month, day, matched_count, start_idx_in_list, matched_folder_count);
    else
        matched_count = 0;
    *wStartIndex = start_idx_in_list;
    return (WORD)matched_count;
}



///
///@ingroup xpgClock
///@brief   Show Calendar on full screen
///
///@param   psWin : specified win
///@param   bColor : specified OSD color to display
///@param   monthOffset : offset from Current month
///@param   dayOffset : offset from current day
///
void ShowCalendar(ST_IMGWIN *psWin, BYTE bColor, int monthOffset, int dayOffset)
{
    MP_DEBUG("ShowCalendar");
    WORD year;
    BYTE month, day, bDays;
    WORD wStartIndex;
    CalendarYearMonthDay(monthOffset, dayOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("ShowCalendar(), after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    BYTE x, y, i;
    BYTE CalTmpBuf[3];
    WORD Y_position = 80;

    x = GetWeekDay(year, month, 1);
    Idu_SetFontColor(225, 146, 0); // red

    for (i = 0; i < 7; i++)
        //Idu_PrintString(psWin, (BYTE *) &(WeekDayStrTab[i]), WeekXTab[i], WeekYTab[0], 0, 0);
        xpgOsdStringFont48((BYTE *)(&CalendayStrTab[i]), WeekXTab[i], WeekYTab[0], 6); // Red

    if(bColor==10) // Black
        Idu_SetFontColor(50, 50, 50); // light black
    else
        Idu_SetFontColor(250, 250, 250); // white
    CalTmpBuf[0] = 0x20;
    CalTmpBuf[2] = 0;
    y = 1;

    for (i = 0x31; i <= 0x39; i++)
    {
        CalTmpBuf[1] = i;

        if ((i & 0x0f) == day && monthOffset==0)
        {
            Idu_SetFontColor(255, 0, 0); //Idu_SetFontColor(225, 146, 0);
            //Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], 0,0);
            xpgOsdStringFont48((BYTE *)(&CalTmpBuf[1]), WeekXTab[x], WeekYTab[y], 6); // Red
            if(bColor==10) // Black
                Idu_SetFontColor(50, 50, 50); // light black
            else
                Idu_SetFontColor(250, 250, 250); // white
        }
        else
            //Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], 0,0);
            xpgOsdStringFont48((BYTE *)(&CalTmpBuf[1]), WeekXTab[x], WeekYTab[y], bColor);
        if( GetDayOfPhoto(year, month, i-0x30, &wStartIndex) )
            ShowDayOfPhoto(x, y);
        x++;

        if (x > 6)
        {
            x = 0;
            y++;
        }
    }

    CalTmpBuf[0] = 0x31;
    CalTmpBuf[1] = 0x30;

    for (i = 10; i <= bDays; i++)
    {
        if (CalTmpBuf[1] > 0x39)
        {
              CalTmpBuf[1] = 0x30;
              CalTmpBuf[0]++;
        }

        if (i == day && monthOffset==0)
        {
            Idu_SetFontColor(225, 146, 0);
            //Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], 0,0);
            xpgOsdStringFont48((BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], 6); // Red
            if(bColor==10) // Black
                Idu_SetFontColor(50, 50, 50); // light black
            else
                Idu_SetFontColor(250, 250, 250); // white
        }
        else
            //Idu_PrintString(psWin, (BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], 0,0);
            xpgOsdStringFont48((BYTE *)(&CalTmpBuf[0]), WeekXTab[x], WeekYTab[y], bColor);
        if( GetDayOfPhoto(year, month, i, &wStartIndex) )
            ShowDayOfPhoto(x, y);
        CalTmpBuf[1]++;
        x++;

        if (x > 6)
        {
            x = 0;
            y++;
        }
    }
    mpSetFontColor(0);
}



#if VCALENDAR
// The year is from 2000
//neil add 071101
extern BYTE gbAtDayIndexYear, gbAtDayIndexMonth, gbAtDayIndexDay;
extern BYTE gbVcalendarListCount;
extern char gthedayTagIndex;
extern BYTE VCalendarEventCount;

#define EVENT_ADD       1
#define EVENT_EDIT      2

VCALENDAR_EVENT_TIME_TAG *pstVcalendarEventTimeTag;
BYTE gbEventIndex;
BYTE gbFuncSel;         //func sel


void VCalendarGetDayIndex(WORD year, BYTE month, BYTE day, WORD *pwAtDayIndex_X, WORD *pwAtDayIndex_Y)
{
    MP_DEBUG("VCalendarGetDayIndex");
    BYTE x, y,z;
    WORD Y_position = (g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight == 600) ? 60 : 0;//for 800x600

    x = GetWeekDay(year, month, 1);
    y = (x + day - 1) % 7;          //line
    z = (x + day - 1) / 7 + 1;      //col
    *pwAtDayIndex_X = CalendarXTab[y];
    *pwAtDayIndex_Y = CalendarYTab[z] + Y_position;
}



void VCalendarEventTimeSet(VCALENDAR_EVENT_TIME_TAG *pstVcalendarEventTag, BYTE EventIndex, BYTE FUNCid)
{
    MP_DEBUG("VCalendarEventTimeSet");
//#if ( ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650) || (CHIP_VER == (CHIP_VER_615 | CHIP_VER_B)) )
    ST_SYSTEM_TIME stSystemTime;

    SystemTimeGet(&stSystemTime);
    Setup_Exit();
    bRTC_Status = RTC_VCALENDAR_EVENT_MODE;

    SettingIndex = 3;

    RtcSettingBuf[4] = stSystemTime.u08Minute;
    RtcSettingBuf[3] = stSystemTime.u08Hour;
    RtcSettingBuf[2] = stSystemTime.u16Year - 2000;
    RtcSettingBuf[1] = stSystemTime.u08Month;
    RtcSettingBuf[0] = stSystemTime.u08Day;

    pstVcalendarEventTimeTag = pstVcalendarEventTag;
    gbEventIndex = EventIndex;
    gbFuncSel = FUNCid;

    StopRtcDisplay();
    ClearClock();
    Setup_SetNoneXpgMode(SETUP_KEYMAP_RTC);
    RtcSettingShowClock(SettingIndex);
//#endif
}



void RtcSetAlarmTime(WORD wAlarmTime)
{
    MP_DEBUG("RtcSetAlarmTime");
    ST_SYSTEM_TIME stAlarmTime;

    SystemTimeGet(&stAlarmTime);

    stAlarmTime.u08Second = 0;
    stAlarmTime.u08Minute = (wAlarmTime & 0x00FF);
    stAlarmTime.u08Hour = wAlarmTime >> 8;

    SystemTimeAlarmSet(&stAlarmTime);

#if VCALENDAR
    RTC_AlarmEnableSet(FALSE);
    RTC_AlarmIeEnableSet(TRUE);
#else
    RTC_AlarmIeEnableSet(FALSE);
    RTC_AlarmEnableSet(TRUE);
#endif

    bRTC_Status = RTC_ALARM_MODE;
    mpDebugPrint("Set Alarm Time not finished yet");
    __asm("break 100");
    bRTC_Status = RTC_NONE_MODE;
}
#endif      // #if VCALENDAR



void InternalRTC_AlarmSetDefault()
{
    MP_DEBUG("InternalRTC_AlarmSetDefault");
    RTC_AlarmEnableSet(FALSE);
    RTC_AlarmIeEnableSet(FALSE);
}



//byAlexWang 26apr2008 automatically calculate the rtc clock position on a win
void RtcPositionInit(ST_IMGWIN * pWin)
{
    MP_DEBUG("RtcPositionInit");
    ST_IMGWIN *ImgWin = pWin;

    if (!ImgWin) ImgWin = (ST_IMGWIN *) Idu_GetNextWin();

    WORD wRtcDayX,wRtcMonthX,wRtcYearX,wRtcHourX,wRtcMinX,wRtcSecX,wRtcY;
    //byAlexWang 6may2008
    //you can manuallly change those values for your customers.
    wRtcDayX = ImgWin->wWidth / 2;
    wRtcMonthX = wRtcDayX + (DEFAULT_FONT_SIZE * 2);
    wRtcYearX = wRtcMonthX + (DEFAULT_FONT_SIZE * 2);
    wRtcHourX = wRtcYearX + (DEFAULT_FONT_SIZE * 4);
    wRtcMinX = wRtcHourX + (DEFAULT_FONT_SIZE * 2);
    wRtcSecX = wRtcMinX +(DEFAULT_FONT_SIZE * 2);
    //wRtcY=ImgWin->wHeight - DEFAULT_FONT_SIZE * 2;
    wRtcY = g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight -
            (g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight >> 4);

    RtcSettingX[0]=RtcClkX[0]=wRtcDayX;
    RtcSettingX[1]=RtcClkX[1]=wRtcMonthX;
    RtcSettingX[2]=RtcClkX[2]=wRtcYearX;
    RtcSettingX[3]=RtcClkX[3]=wRtcHourX;
    RtcSettingX[4]=RtcClkX[4]=wRtcMinX;
    RtcSettingX[5]=RtcClkX[5]=wRtcSecX;
    RtcSettingX[6]=RtcClkX[6]=wRtcY;

    CalendarXTab[6]=wRtcDayX - (DEFAULT_FONT_SIZE*3);
    CalendarXTab[5]=CalendarXTab[6] - (DEFAULT_FONT_SIZE*2)-4;
    CalendarXTab[4]=CalendarXTab[5]- (DEFAULT_FONT_SIZE*2)-4;
    CalendarXTab[3]=CalendarXTab[4]- (DEFAULT_FONT_SIZE*2)-4;
    CalendarXTab[2]=CalendarXTab[3]- (DEFAULT_FONT_SIZE*2)-4;
    CalendarXTab[1]=CalendarXTab[2]- (DEFAULT_FONT_SIZE*2)-4;
    CalendarXTab[0]=CalendarXTab[1]- (DEFAULT_FONT_SIZE*2)-4;

    CalendarYTab[4]= ImgWin->wHeight/2;
    CalendarYTab[3]=CalendarYTab[4]-(DEFAULT_FONT_SIZE*2);
    CalendarYTab[2]=CalendarYTab[3]-(DEFAULT_FONT_SIZE*2);
    CalendarYTab[1]=CalendarYTab[2]-(DEFAULT_FONT_SIZE*2);
    CalendarYTab[0]=CalendarYTab[1]-(DEFAULT_FONT_SIZE*2);
    CalendarYTab[5]=CalendarYTab[4]+(DEFAULT_FONT_SIZE*2);
    CalendarYTab[6]=CalendarYTab[5]+(DEFAULT_FONT_SIZE*2);
}



static S16 UtilMonMap(S08 *pat)
{
    MP_DEBUG("UtilMonMap");
    if(!strcmp(pat, "Jan"))
        return 1;
    else if(!strcmp(pat,"Feb"))
        return 2;
    else if(!strcmp(pat,"Mar"))
        return 3;
    else if(!strcmp(pat,"Apr"))
        return 4;
    else if(!strcmp(pat,"May"))
        return 5;
    else if(!strcmp(pat,"Jun"))
        return 6;
    else if(!strcmp(pat,"Jul"))
        return 7;
    else if(!strcmp(pat,"Aug"))
        return 8;
    else if(!strcmp(pat,"Sep"))
        return 9;
    else if(!strcmp(pat,"Oct"))
        return 10;
    else if(!strcmp(pat,"Nov"))
        return 11;
    else if(!strcmp(pat,"Dec"))
        return 12;
    else
        return 13;
}


static ST_SYSTEM_TIME stBuildTime;
ST_SYSTEM_TIME * pstBuildTime = &stBuildTime;

ST_SYSTEM_TIME *xpgGetBuildTime()
{
    return pstBuildTime;
}

void xpgGetBuildHourMinuteSecond(BYTE *hour, BYTE *minute, BYTE *second)
{
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    *hour = stSystemTime.u08Hour;
    *minute = stSystemTime.u08Minute;
    *second = stSystemTime.u08Second;
    return;
}

#if 1

// if rtc value is invalid, reset to 2000.01.01 00:00:00 Saturday
void Ui_RtcInit()
{
    MP_DEBUG("Ui_RtcInit -");

    RTC_InternalLdoEnable(TRUE);
#if (CHIP_VER_MSB == CHIP_VER_660)
    RTC_AlarmOutSet(TRUE);
#endif

    //Set system time
    {
        ST_SYSTEM_TIME stSystemTime;
        U08 tempStr[20], i;
        U32 version;
        U08 major, minor, rev;

        for (i = 0; i < 20; i++)
        {
            tempStr[i] = *(U08 *) (__TIME__ + i);
        }

        stBuildTime.u08Hour = (BYTE) atoi(strtok(tempStr, " :"));
        stBuildTime.u08Minute = (BYTE) atoi(strtok(NULL, " :"));
        stBuildTime.u08Second = (BYTE) atoi(strtok(NULL, " :"));

        for (i = 0; i < 20; i++)
        {
            tempStr[i] = *(U08 *) (__DATE__ + i);
        }

        stBuildTime.u08Month = (BYTE) UtilMonMap((S08 *) strtok(tempStr, " "));
        stBuildTime.u08Day = (BYTE) atoi(strtok(NULL, " "));
        stBuildTime.u16Year = (WORD) atoi(strtok(NULL, " "));
        SystemTimeGet(&stSystemTime);
        MP_ALERT("\r\nCurrent RTC time is %04d/%02d/%02d %02d:%02d:%02d",
                  stSystemTime.u16Year, stSystemTime.u08Month, stSystemTime.u08Day,
                  stSystemTime.u08Hour, stSystemTime.u08Minute, stSystemTime.u08Second);
        MP_ALERT("Current RTC count is 0x%08X", RTC_ReadCount());
    }

    RTC_RegisterRtcIsrCallback(uiRtcIsr);
    dwPrevMin = 0xFFFFFFFF;
    bRtcDisplayEnable = 0;
    RtcPositionInit(NULL);//byAlexWang 26apr2008
#if RTC_ENABLE
    UpdateClock();
#endif
}

#else

// if rtc value is invalid, reset to 2000.01.01 00:00:00 Saturday
void Ui_RtcInit()
{
    MP_DEBUG("Ui_RtcInit -");

    RTC_InternalLdoEnable(TRUE);
#if (CHIP_VER_MSB == CHIP_VER_660)
    RTC_AlarmOutSet(TRUE);
#endif

    //Set system time
    {
        ST_SYSTEM_TIME stSystemTime;
        U08 tempStr[20], i;
        U32 version;
        U08 major, minor, rev;

        for (i = 0; i < 20; i++)
        {
            tempStr[i] = *(U08 *) (__TIME__ + i);
        }

        stBuildTime.u08Hour = (BYTE) atoi(strtok(tempStr, " :"));
        stBuildTime.u08Minute = (BYTE) atoi(strtok(NULL, " :"));
        stBuildTime.u08Second = (BYTE) atoi(strtok(NULL, " :"));

        for (i = 0; i < 20; i++)
        {
            tempStr[i] = *(U08 *) (__DATE__ + i);
        }

        stBuildTime.u08Month = (BYTE) UtilMonMap((S08 *) strtok(tempStr, " "));
        stBuildTime.u08Day = (BYTE) atoi(strtok(NULL, " "));
        stBuildTime.u16Year = (WORD) atoi(strtok(NULL, " "));
        SystemTimeGet(&stSystemTime);
        MP_ALERT("\r\nCurrent RTC time is %04d/%02d/%02d %02d:%02d:%02d",
                  stSystemTime.u16Year, stSystemTime.u08Month, stSystemTime.u08Day,
                  stSystemTime.u08Hour, stSystemTime.u08Minute, stSystemTime.u08Second);
        MP_ALERT("Current RTC count is 0x%08X", RTC_ReadCount());

#if ((CHIP_VER_MSB != CHIP_VER_615) && (STD_BOARD_VER != MP650_FPGA))
        if (RTC_ReadCountVerify() == FAIL)
#endif
        {
            MP_ALERT("Reset System Time to %s %s", __DATE__, __TIME__);
            MP_ALERT("Reset System Time to %04d/%02d/%02d %02d:%02d:%02d\n",
                      stBuildTime.u16Year, stBuildTime.u08Month, stBuildTime.u08Day,
                      stBuildTime.u08Hour, stBuildTime.u08Minute, stBuildTime.u08Second);

            SystemTimeSet(&stBuildTime);
            RTC_AlarmEnableSet(FALSE);
            RTC_AlarmIeEnableSet(FALSE);
            RTC_LowPowerEnable(ENABLE);
            //RTC_SecCountToleranceAdj(4, 0, 26);
        }
    }

    RTC_RegisterRtcIsrCallback(uiRtcIsr);
    dwPrevMin = 0xFFFFFFFF;
    bRtcDisplayEnable = 0;
    RtcPositionInit(NULL);//byAlexWang 26apr2008
#if RTC_ENABLE
    UpdateClock(); //AddTimerProc(5, UpdateClock);
#endif
}

#endif

void StartRtcDisplay()
{
    MP_DEBUG("StartRtcDisplay");
    bRtcDisplayEnable = 1;
}



void StopRtcDisplay()
{
    MP_DEBUG("StopRtcDisplay");
    bRtcDisplayEnable = 0;
}



// AnalogClock Function ================================================================
void AnaClockClearWin(ST_IMGWIN *pWin)
{
    MP_DEBUG("AnaClockClearWin");
    int i;
    DWORD *pdwTrg;

    pdwTrg = pWin->pdwStart;

    for (i = pWin->wHeight * (pWin->wWidth >> 1); i > 0; i--)
        *pdwTrg++ = 0x00008080;
}



static int SIN_Second[60]=
{
        0, -105, -208, -310, -407, -500, -588, -670, -744, -810,
     -867, -914, -952, -979, -995,-1000, -995, -979, -952, -914,
     -867, -810, -744, -670, -588, -500, -407, -310, -208, -105,
        0,  104,  207,  309,  406,  500,  587,  669,  743,  809,
      866,  913,  951,  978,  994, 1000,  994,  978,  951,  913,
      866,  809,  743,  669,  587,  500,  406,  309,  207,  104
};



static int COS_Second[60]=
{
     1000,  994,  978,  951,  913,  866,  809,  743,  669,  587,
      500,  406,  309,  207,  104,   -1, -105, -208, -310, -407,
     -500, -588, -670, -744, -810, -867, -914, -952, -979, -995,
    -1000, -995, -979, -952, -914, -867, -810, -744, -670, -588,
     -500, -407, -310, -208, -105,    0,  104,  207,  309,  406,
      500,  587,  669,  743,  809,  866,  913,  951,  978,  994
};



int AnaClockRotateWin(ST_IMGWIN *SrcWin, ST_IMGWIN *TrgWin, BYTE bSecond)
{
    MP_DEBUG("AnaClockRotateWin");
    register int sX, sY, tX, tY, saX, saY, taX, taY, sH, sW, tH, tW, i, SinValue, CosValue;
    register DWORD *pdwSrc, *pdwTrg;
    register ST_PIXEL *SrcPixel, *TrgPixel;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

    saX = SrcWin->wAxisX;
    saY = SrcWin->wAxisY;
    sH = SrcWin->wHeight;
    sW = SrcWin->wWidth;
    taX = TrgWin->wAxisX;
    taY = TrgWin->wAxisY;
    tH = TrgWin->wHeight;
    tW = TrgWin->wWidth;
    SinValue = SIN_Second[bSecond];
    CosValue = COS_Second[bSecond];

    if (sW < 0 || sW > pstScreen->wInnerWidth || sH < 0 || sH > pstScreen->wInnerHeight ||
        tW < 0 || tW > pstScreen->wInnerWidth || tH < 0 || tH > pstScreen->wInnerHeight)
        return FAIL;

    if (saX > sW || saY > sH || taX > tW || taY > tH)
        return FAIL;


    pdwSrc = SrcWin->pdwStart + (saY * SrcWin->dwOffset / 4) + (saX / 2);
    pdwTrg = TrgWin->pdwStart + (taY * TrgWin->dwOffset / 4) + (taX / 2);
//    for (sX = -saX; sX < (sW - saX); sX ++)
    for (sY = -saY; sY < (sH - saY); sY ++)
    {
//        for (sY = -saY; sY < (sH - saY); sY ++)
        for (sX = -saX; sX < (sW - saX); sX ++)
        {
            tX = (sX * CosValue + sY * SinValue) / 1000;
            tY = (sY * CosValue - sX * SinValue) / 1000;

            if ((taX + tX) >= 0 && (taX + tX) < tW && (taY + tY) >= 0 && (taY + tY) < tH)
            {
                SrcPixel = (ST_PIXEL*)(pdwSrc + ((sY * SrcWin->dwOffset / 2) + sX) / 2);
                TrgPixel = (ST_PIXEL*)(pdwTrg + ((tY * TrgWin->dwOffset / 2) + tX) / 2);

                if(((saX + sX) & 0x1) && (SrcPixel->Y1 < 0x40))
                    continue;

                else if((((saX + sX) & 0x1) == 0) && (SrcPixel->Y0 < 0x40))
                    continue;

                if (0) // (TrgPixel->Y0 == 0 && TrgPixel->Y1 == 0 && TrgPixel->Cb == 0x80 && TrgPixel->Cr == 0x80)
                {
                    TrgPixel->Cb = SrcPixel->Cb;
                    TrgPixel->Cr = SrcPixel->Cr;

                    if ((saX + sX) & 0x1)
                        TrgPixel->Y0 = TrgPixel->Y1 = SrcPixel->Y1;
                    else
                        TrgPixel->Y0 = TrgPixel->Y1 = SrcPixel->Y0;
                }
                else
                {
                    TrgPixel->Cb = (TrgPixel->Cb + SrcPixel->Cb) / 2;
                    TrgPixel->Cr = (TrgPixel->Cr + SrcPixel->Cr) / 2;

                    if ((taX + tX) & 0x1)
                    {
                        if ((saX + sX) & 0x1)
                        {
                            TrgPixel->Y1 = SrcPixel->Y1;

                            if(SrcPixel->Y0 < 0x40)
                                continue;

                            if(((ST_PIXEL*)(SrcPixel + 1))->Y0 < 0x40)
                                continue;
                        }
                        else
                        {
                            TrgPixel->Y1 = SrcPixel->Y0;

                            if(((ST_PIXEL*)(SrcPixel - 1))->Y1 < 0x40)
                                continue;

                            if(SrcPixel->Y1 < 0x40)
                                continue;
                        }

                        TrgPixel->Y0 = TrgPixel->Y1;
                    }
                    else
                    {
                        if((saX + sX) & 0x1)
                        {
                            TrgPixel->Y0 = SrcPixel->Y1;

                            if(SrcPixel->Y0 < 0x40)
                                continue;

                            if(((ST_PIXEL*)(SrcPixel + 1))->Y0 < 0x40)
                                continue;
                        }
                        else
                        {
                            TrgPixel->Y0 = SrcPixel->Y0;

                            if(((ST_PIXEL*)(SrcPixel - 1))->Y1 < 0x40)
                                continue;

                            if(SrcPixel->Y1 < 0x40)
                                continue;
                        }

                        TrgPixel->Y1 = TrgPixel->Y0;
                    }
                }
            }
        }
    }

    return PASS;
}



extern BYTE g_boNeedRepaint;
int AnaClockProcess(ST_IMGWIN *psWin)
{
    MP_DEBUG("AnaClockProcess(psWin=0x%X)", psWin);
    DWORD dwHour, dwMin, dwSec;
    ST_IMGWIN *SrcWin;
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);

    SrcWin = &m_stClockWin[0];
    //NextWin   = Idu_GetNextWin();
    //psWin = Idu_GetCurrWin();

    dwPrevSec = dwSec = stSystemTime.u08Second;
    dwPrevMin = dwMin = stSystemTime.u08Minute;

    if (stSystemTime.u08Hour >= 12)
      dwHour = stSystemTime.u08Hour - 12;
    else
      dwHour = stSystemTime.u08Hour;

    dwHour = (dwHour * 5) + (dwMin / 12);

    Slide_WinCopy (SrcWin, psWin);

    psWin->wAxisX   = SrcWin->wAxisX;
    psWin->wAxisY   = SrcWin->wAxisY;
    g_boNeedRepaint = FAIL;

    SrcWin = &m_stClockWin[1];

    if (AnaClockRotateWin (SrcWin, psWin,   dwHour) != PASS)
        return FAIL;

    SrcWin = &m_stClockWin[2];

    if (AnaClockRotateWin (SrcWin, psWin,   dwMin) != PASS)
        return FAIL;

    SrcWin = &m_stClockWin[3];
    //if (AnaClockRotateWin (SrcWin, NextWin, dwSec) != PASS)
    if (AnaClockRotateWin (SrcWin, psWin, dwSec) != PASS)
            return FAIL;

    ShowCalendarLeft(psWin, stSystemTime.u16Year - 2000,  stSystemTime.u08Month, stSystemTime.u08Day);
    //Idu_ChgWin(Idu_GetNextWin());

    return  PASS;
}



void AnaClockStart(void)
{
    MP_DEBUG("AnaClockStart");
    ST_IMGWIN *psWin;

    xpgClearCatch();
    RemoveTimerProc(Cal_PhotoUpdate);
    RemoveTimerProc(DigitClockProcess); // Maybe from switch from DigitClock
    dwPrevMin = 0xFFFFFFFF;
    dwPrevSec = 0xFFFFFFFF;
    psWin = Idu_GetNextWin();
    //Idu_PaintWinArea(psWin, 0, 0, 800, 480, RGB2YUV(255, 255, 255)); // Jonny 20090508
    psWin = Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(255, 255, 255));
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(255, 255, 255));
    xpgUpdateStage();
    psWin = Idu_GetCurrWin();
    AnaClockProcess(psWin);
    g_bXpgStatus = XPG_MODE_ANALOGCLOCK;
    g_boNeedRepaint = FALSE;
    UpdateClock();
}



// Cal_Photo Function ===============================================================
///
///@ingroup xpgClock
///@brief   Update Cal_Photo
///
void Cal_PhotoUpdate(void)
{
    MP_DEBUG("Cal_PhotoUpdate");
    ST_IMGWIN   * psWin;

    if (g_bXpgStatus != XPG_MODE_CALPHOTO)
        return;

    //g_boNeedRepaint = FALSE;
    //xpgClearCatch();
    g_psSystemConfig->dwCurrentOpMode = 0; // Jonny add 20090508
    psWin = Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(255, 255, 255));
    //Idu_PaintWinArea(psWin, 0, 0, 800, 480, RGB2YUV(255, 255, 255)); // Jonny 20090508
    xpgUpdateStage();

    if (g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex >= g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile - 1)
        g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex = 0;
    else
        g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex += 1;
    UpdateClock();
    if(g_bXpgStatus == XPG_MODE_CALPHOTO)
        AddTimerProc(xpgGetSlideInterval(), Cal_PhotoUpdate);
}



///
///@ingroup xpgClock
///@brief   Cal_Photo start routine
///
void Cal_PhotoStart(void)
{
    MP_DEBUG("Cal_PhotoStart");
    if (g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile == 0)
    {
        g_psSystemConfig->dwCurrentOpMode = OP_IMAGE_MODE;
        FileBrowserScanFileList(SEARCH_TYPE);
        xpgMenuListSetIndex(0, false); // then it will set pstBrowser->wCount = g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile;
    }

    if (g_psSystemConfig->sFileBrowser.dwImgAndMovTotalFile == 0)
    {
        //if (xpgSearchAndGotoPage("Calendar") == NULL)
        if (xpgSearchAndGotoPage("CalendarWhite") == NULL)
            xpgCb_GoModePage(); //xpgCb_GoPrevPage();
        else
            EnterCalendarWhite(); //AnaClockStart();
    }
    else
    {
        g_bXpgStatus = XPG_MODE_CALPHOTO;

        if (g_psSystemConfig->dwCurrentOpMode != OP_IMAGE_MODE)
        {
            ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
            psSysConfig->dwCurrentOpMode = OP_AUDIO_MODE; // force xpgChangeMenuMode() boModeChanged = true to do FileBrowserScanList()
            xpgChangeMenuMode(OP_IMAGE_MODE, 1); // it will chnage psSysConfig->dwCurrentOpMode to OP_IMAGE_MODE;
        }
        g_bXpgStatus = XPG_MODE_CALPHOTO;
        Cal_PhotoUpdate();
    }
}



// FlashClockWhite Function ==================
BYTE centerCircle[144]={ // 12x12
 0, 0, 0,13,13,13,13,13,13, 0, 0, 0,
 0, 0,13,13,13,13,13,13,13,13, 0, 0,
 0,13,13,13,13,13,13,13,13,13,13, 0,
13,13,13,13,13,13,13,13,13,13,13,13,
13,13,13,13,13,13,13,13,13,13,13,13,
13,13,13,13,13,13,13,13,13,13,13,13,
13,13,13,13,13,13,13,13,13,13,13,13,
13,13,13,13,13,13,13,13,13,13,13,13,
 0,13,13,13,13,13,13,13,13,13,13, 0,
 0,13,13,13,13,13,13,13,13,13,13, 0,
 0, 0,13,13,13,13,13,13,13,13, 0, 0,
 0, 0, 0,13,13,13,13,13,13, 0, 0, 0,
};



void FlashClockWhiteProcess()
{
    //ShowFlashClockHour(10); // 10=black
    //AddTimerProc(1000, FlashClockWhiteProcess);
}



void FlashClockBlackProcess(void);
///
///@ingroup xpgClock
///@brief   FlashClock start routine
///
void EnterFlashClockWhite()
{
    ST_IMGWIN   * psWin;

    MP_DEBUG("EnterFlashClockWhite");
    RemoveTimerProc(UpdateClock);
    RemoveTimerProc(FlashClockBlackProcess); // from page 'FlashClockBlack'
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    Idu_OsdErase();
    AddTimerProc(100, FlashClockWhiteProcess);
    g_bXpgStatus = XPG_MODE_FLASHCLOCKWHITE;
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Exit from FlashClockWhite
///
void FlashClockWhiteExit()
{
    MP_DEBUG("FlashClockWhiteExit");
    RemoveTimerProc(FlashClockWhiteProcess);
    Idu_OsdErase();
    AnaClockStart();
}



// FlashClockBlack Function ==================
int hourLast=-1, minuteLast=-1, secondLast=-1;
///
///@ingroup xpgClock
///@brief   FlashClockBlack main program
///
void FlashClockBlackProcess()
{
    ST_IMGWIN   * psWin;
    psWin = Idu_GetNextWin();
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE hour, minute, second;
    hour = stSystemTime.u08Hour;
    minute = stSystemTime.u08Minute;
    second = stSystemTime.u08Second;
    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    if(hour >=12 ) hour -= 12;
    int indexHour  = ((hour % 3)*5)+(minute/12);;
    int rotateHour = hour / 3;
    if(hour!=hourLast) {
        hourLast = hour;
    }

    int indexMinute  = minute % 15;
    int rotateMinute = minute / 15;
    if(minute != minuteLast) {
        minuteLast = minute;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 0, 0); // Clockbkg.jpg
        pstSprite->m_pstRole->m_bImageType = 0;
        //xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, 160, 0, 0);
        //xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, 160, 0, 0);
        xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, (psWin->wWidth-pstSprite->m_wWidth)/2, (psWin->wHeight-pstSprite->m_wHeight)/2, pstSprite, 0);

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 0, 1+indexHour); // Hour-?.jpg
        pstSprite->m_pstRole->m_bImageType = 11+rotateHour;
        //xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, 200, 40, 0);
        //xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, 200, 40, 0);
        xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, (psWin->wWidth-pstSprite->m_wWidth)/2, (psWin->wHeight-pstSprite->m_wHeight)/2, pstSprite, 0);

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 0, 16+indexMinute); // Minute-?.jpg
        pstSprite->m_pstRole->m_bImageType = 11+rotateMinute; // Type=11 - FlashClockType
        //xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, 200, 40, 0);
        //xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, 200, 40, 0);
        xpgDirectDrawRoleOnWin(Idu_GetNextWin(), pstSprite->m_pstRole, (psWin->wWidth-pstSprite->m_wWidth)/2, (psWin->wHeight-pstSprite->m_wHeight)/2, pstSprite, 0);

        Idu_ChgWin(Idu_GetNextWin());
    }
    int indexSecond  = second % 15;
    int rotateSecond = second / 15;
    if(second != secondLast) {
        secondLast = second;
        //Idu_OsdErase();
        //Idu_OsdPaintArea(394, 234, 12, 12, 0);
        Idu_OsdPaintArea(psWin->wWidth/2-12/2, psWin->wHeight/2-12/2, 12, 12, 0);

        //DrawSecondRed(400, 240, rotateSecond, indexSecond);
        DrawSecondRed(psWin->wWidth/2, psWin->wHeight/2, rotateSecond, indexSecond);

        //Idu_OsdPaintData(394, 234, 12, 12, &centerCircle[0]);
        Idu_OsdPaintData((psWin->wWidth/2)-6, (psWin->wHeight/2)-6, 12, 12, &centerCircle[0]);
    }

    if(g_bXpgStatus == XPG_MODE_FLASHCLOCKBLACK) // when SD_CARD_OUT event happen
         AddTimerProc(1, FlashClockBlackProcess);
    else
        Idu_OsdErase;
}



//void FlashClockWhiteProcess(void);
///
///@ingroup xpgClock
///@brief   FlashClockBlack start program
///
DWORD palette6Backup;
void EnterFlashClockBlack()
{
    MP_DEBUG("EnterFlashClockBlack");
    RemoveTimerProc(UpdateClock);
    RemoveTimerProc(FlashClockWhiteProcess); // from page 'FlashClockWhite'
    //Idu_PaintWinArea(Idu_GetCurrWin(), 0, 0, 800, 480, RGB2YUV(0, 0, 0));
    //Idu_PaintWinArea(Idu_GetNextWin(), 0, 0, 800, 480, RGB2YUV(0, 0, 0)); // later will Idu_ChgWin()
    ST_IMGWIN   * psWin;
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    psWin = Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    minuteLast = -1;
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    palette6Backup = idu->Palette[6];
    idu->Palette[6]  = (0x0f << 24) + (0x00FF0000);

    AddTimerProc(1, FlashClockBlackProcess);

    g_bXpgStatus = XPG_MODE_FLASHCLOCKBLACK;
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Exit from FlashClockBlack
///
void FlashClockBlackExit()
{
    MP_DEBUG("FlashClockBlackExit");
    RemoveTimerProc(FlashClockBlackProcess);
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    idu->Palette[6] = palette6Backup;
    xpgSearchAndGotoPage("Main_Calendar");
    xpgCb_EnterModeMenu(); //AnaClockStart();
}



// DigitClock Function =============================================================
int digithourLast=-1, digitminuteLast=-1, digitsecondLast=-1;

///
///@ingroup xpgClock
///@brief   DigitClock main program, recursive by timer
///
void DigitClockProcess(void)
{
    MP_DEBUG("DigitClockProcess");
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE hour, minute, second;
    hour = stSystemTime.u08Hour;
    minute = stSystemTime.u08Minute;
    second = stSystemTime.u08Second;

    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    WORD wXPG=pstMov->m_wScreenWidth; // 800x480 => 800
    WORD hXPG=pstMov->m_wScreenHeight;// 800x480 => 480
    //mpDebugPrint("wXPG=%d, hXPG=%d", wXPG, hXPG);
    STXPGSPRITE *pstSprite;
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, 43, 0);
    STXPGROLE * pstRole = pstSprite->m_pstRole;
    WORD wCircle=pstRole->m_wWidth; // 800x480 then 192
    WORD hCircle=pstRole->m_wHeight;// 800x480 then 192
    WORD yCircle = (hXPG/2) - (hCircle/2);
    //mpDebugPrint("wCircle=%d, hCircle=%d, yCircle=%d", wCircle, hCircle, yCircle);
    WORD wDigit, hDigit;
    WORD gap = (wXPG-wCircle*3)/4; //56=(800 - 192*3)/4;
    WORD xHour=gap;
    WORD xMin=wXPG/2-wCircle/2; // 304=800/2-192/2;
    WORD xSec=xMin+wCircle+gap;
    //mpDebugPrint("xHour=%d, xMin=%d, xSec=%d", xHour, xMin, xSec);
    WORD midHour=gap+wCircle/2;
    WORD midMin=wXPG/2;
    WORD midSec=midMin+wCircle+gap;
    WORD x1, x2, y, y1;
    if(hour >=12 ) hour -= 12;
    int indexHour  = ((hour % 3)*5)+(minute/12);
    int rotateHour = hour / 3;
    if(hour!=digithourLast) {
        digithourLast = hour;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 43, indexHour); //
        pstSprite->m_pstRole->m_bImageType = 21+rotateHour;
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, xHour, yCircle, pstSprite, 0);

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, hour/10);
        pstRole = pstSprite->m_pstRole;
        //mpDebugPrint("pstRole->m_wWidth=%d, pstRole->m_wHeight=%d", pstRole->m_wWidth, pstRole->m_wHeight);
        x1 = midHour-(pstRole->m_wWidth)+4;
        x2 = midHour-4;
        y = (hXPG/2) - (pstRole->m_wHeight/2);
        //mpDebugPrint("Hour x1=%d, x2=%d, y=%d", x1, x2, y);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x1, y, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, hour%10);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x2, y, pstSprite, 0);
    }

    int indexMinute  = minute % 15;
    int rotateMinute = minute / 15;
    if(minute != digitminuteLast) {
        digitminuteLast = minute;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 43, indexMinute); //
        pstSprite->m_pstRole->m_bImageType = 21+rotateMinute;
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, xMin, yCircle, pstSprite, 0);

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, minute/10);
        pstRole = pstSprite->m_pstRole;
        //mpDebugPrint("pstRole->m_wWidth=%d, pstRole->m_wHeight=%d", pstRole->m_wWidth, pstRole->m_wHeight);
        x1 = midMin-(pstRole->m_wWidth)+4;
        x2 = midMin-4;
        y = (hXPG/2) - (pstRole->m_wHeight/2);
        //mpDebugPrint("Minute x1=%d, x2=%d, y=%d", x1, x2, y);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x1, y, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, minute%10);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x2, y, pstSprite, 0);
    }
    int indexSecond  = second % 15;
    int rotateSecond = second / 15;
    if(second != digitsecondLast) {
        digitsecondLast = second;

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 43, indexSecond); //
        pstSprite->m_pstRole->m_bImageType = 21+rotateSecond;
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, xSec, yCircle, pstSprite, 0);

        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, second/10);
        pstRole = pstSprite->m_pstRole;
        //mpDebugPrint("pstRole->m_wWidth=%d, pstRole->m_wHeight=%d", pstRole->m_wWidth, pstRole->m_wHeight);
        x1 = midSec-(pstRole->m_wWidth)+4;
        x2 = midSec-4;
        y = (hXPG/2) - (pstRole->m_wHeight/2);
        //mpDebugPrint("Second x1=%d, x2=%d, y=%d", x1, x2, y);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x1, y, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, 46, second%10);
        xpgDirectDrawRoleOnWin(Idu_GetCurrWin(), pstSprite->m_pstRole, x2, y, pstSprite, 0);
    }
    if(g_bXpgStatus == XPG_MODE_DIGITCLOCK) // when SD_CARD_OUT event happen
        AddTimerProc(1, DigitClockProcess);
    else
        Idu_OsdErase;
}



///
///@ingroup xpgClock
///@brief   DigitClock start routine
///
void DigitClockStart(void)
{
    MP_DEBUG("DigitClkStart");
    RemoveTimerProc(UpdateClock);
    RemoveTimerProc(DigitClockProcess);
    RemoveTimerProc(FlashClockBlackProcess);
    //Idu_PaintWinArea(Idu_GetCurrWin(), 0, 0, 800, 480, RGB2YUV(0, 0, 0));
    //Idu_PaintWinArea(Idu_GetNextWin(), 0, 0, 800, 480, RGB2YUV(0, 0, 0));
    ST_IMGWIN   * psWin;
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    psWin = Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    Idu_OsdErase();
    digithourLast = digitminuteLast = digitsecondLast = -1;
    AddTimerProc(1, DigitClockProcess);
    g_bXpgStatus = XPG_MODE_DIGITCLOCK;
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Exit from DigitClock
///
void DigitClockExit(void)
{
    MP_DEBUG("DigitClockExit");
    Idu_OsdErase();
    RemoveTimerProc(DigitClockProcess);
    xpgSearchAndGotoPage("FlashClockBlack");
    EnterFlashClockBlack(); //AnaClockStart();
}



// CalendarBlack Function ======================================================
static void CalendarYearMonthDay(int monthOffset, int dayOffset, WORD *yearIn, BYTE *monthIn, BYTE *dayIn, BYTE *bDaysIn)
{
    //mpDebugPrint("CalendarYearMonthDay()");
    WORD year;
    BYTE month, day, leftYearCount=0, rightYearCount=0;
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    day = stSystemTime.u08Day;
    month = stSystemTime.u08Month;
    year = stSystemTime.u16Year - 2000;

    if((month + monthOffset)<=0) {
        leftYearCount = abs((month + monthOffset)) / 12 + 1;
        //mpDebugPrint("leftYearCount = %d", leftYearCount);
        year -= leftYearCount;
        month = 12 * leftYearCount + (month + monthOffset);
    } else {
        rightYearCount = (month + monthOffset - 1) / 12;
        year += rightYearCount;
        month = (month + monthOffset) - (rightYearCount * 12);
    }
    BYTE bDays = DaysTab[month];
    if ( (month == 2) && LeapYearCheck(year) )
        bDays = 29;
    if(day > bDays) day = bDays;
    *yearIn  = year;
    *monthIn = month;
    *dayIn   = day;
    *bDaysIn = bDays;
}



int dayOffsetBlackSave;

static void ShowCalendarBlackDayBox(ST_IMGWIN *psWin, BYTE bColor, int monthOffset, int dayOffset)
{
    //mpDebugPrint("ShowCalendarDayBox()");
    WORD year;
    BYTE month, day, bDays;
    CalendarYearMonthDay(monthOffset, dayOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    BYTE x, y, dayNew;
    //static WORD WeekXTab[7] = {75, 175, 275, 375, 475, 575, 675};
    //static WORD WeekYTab[7] = {120, 175, 230, 285, 340, 395, 450};

    // show Current dayOffset
    x = GetWeekDay(year, month, 1);
    dayNew = (x-1) + day + dayOffsetBlackSave;
    x = dayNew % 7;
    y = 1 + dayNew / 7;
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]-4,  64, 4, 10); // top
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]+50, 64, 4, 10); // bottom
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     4,50, 10); // left
    Idu_OsdPaintArea(WeekXTab[x]+50, WeekYTab[y],     4,50, 10); // right
    dayOffsetBlackSave = dayOffset;

    // show Current dayOffsetBox
    x = GetWeekDay(year, month, 1);
    dayNew = (x-1) + day + dayOffset;
    x = dayNew % 7;
    y = 1 + dayNew / 7;
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]-4,  64, 4, bColor); // top
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]+50, 64, 4, bColor); // bottom
#if(OSD_BIT_WIDTH==4)
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     4,50, bColor); // left
    Idu_OsdPaintArea(WeekXTab[x]+50, WeekYTab[y],     4,50, bColor); // right
#elif(OSD_BIT_WIDTH==8)
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     3,50, bColor); // left
    Idu_OsdPaintArea(WeekXTab[x]+57, WeekYTab[y],     3,50, bColor); // right
#endif
}
int dayOffsetWhiteSave=0;
static void ShowCalendarWhiteDayBox(ST_IMGWIN *psWin, BYTE bColor, int monthOffset, int dayOffset)
{
    //mpDebugPrint("ShowCalendarWhiteDayBox()");
    WORD year;
    BYTE month, day, bDays;
    CalendarYearMonthDay(monthOffset, dayOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    BYTE x, y, dayNew;
    //static WORD WeekXTab[7] = {75, 175, 275, 375, 475, 575, 675};
    //static WORD WeekYTab[7] = {120, 175, 230, 285, 340, 395, 450};

    //erase last one
    x = GetWeekDay(year, month, 1);
    dayNew = (x-1) + day + dayOffsetWhiteSave;
    x = dayNew % 7;
    y = 1 + dayNew / 7;
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]-4,  64, 4, OSD_COLOR_WHITE); // top
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]+50, 64, 4, OSD_COLOR_WHITE); // bottom
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     4,50, OSD_COLOR_WHITE); // left
    Idu_OsdPaintArea(WeekXTab[x]+50, WeekYTab[y],     4,50, OSD_COLOR_WHITE); // right
    dayOffsetWhiteSave = dayOffset;

    // show Current dayOffsetBox
    x = GetWeekDay(year, month, 1);
    dayNew = (x-1) + day + dayOffset;
    x = dayNew % 7;
    y = 1 + dayNew / 7;
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]-4,  64, 4, bColor); // top
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y]+50, 64, 4, bColor); // bottom
#if(OSD_BIT_WIDTH==4)
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     4,50, bColor); // left
    Idu_OsdPaintArea(WeekXTab[x]+50, WeekYTab[y],     4,50, bColor); // right
#elif(OSD_BIT_WIDTH==8)
    Idu_OsdPaintArea(WeekXTab[x]-10, WeekYTab[y],     3,50, bColor); // left
    Idu_OsdPaintArea(WeekXTab[x]+57, WeekYTab[y],     3,50, bColor); // right
#endif
}

BYTE flashToggle;

static void ShowCalendarHead(BYTE bColor, int monthOffset, int dayOffset)
{
    MP_DEBUG("ShowCalendarHead");
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    WORD year;
    BYTE month, day, bDays;
    CalendarYearMonthDay(monthOffset, dayOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    RtcSettingBuf[0] = year;
    RtcSettingBuf[1] = month;
    RtcSettingBuf[2] = day;
    RtcSettingBuf[3] = stSystemTime.u08Hour;
    RtcSettingBuf[4] = stSystemTime.u08Minute;
    RtcSettingBuf[5] = stSystemTime.u08Second;

    BYTE temp, i, DigitTmpBuf[6];
    BYTE DigitClockGapChar[6] = {0x2e, 0x2e, 0x20, 0x3a, 0x3a, 0};
    DigitTmpBuf[0] = 0x32; // '2'
    DigitTmpBuf[1] = 0x30; // '0'
    temp = RtcSettingBuf[0]/10;
    DigitTmpBuf[2] = 0x30 + temp;
    DigitTmpBuf[3] = 0x30 + RtcSettingBuf[0]%10;
    DigitTmpBuf[4] = 0x2F; // "/"
    DigitTmpBuf[5] = 0x00;
    //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&DigitTmpBuf[0]), 100, 40, bColor); // display year
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 50, 40, bColor); // year
    WORD x=240, y=40;
    //switch(stSystemTime.u08Month) {
    switch(month) {
    case 1 : xpgOsdStringFont48("JAN", x, y, bColor); break;
    case 2 : xpgOsdStringFont48("FEB", x, y, bColor); break;
    case 3 : xpgOsdStringFont48("MAR", x, y, bColor); break;
    case 4 : xpgOsdStringFont48("APR", x, y, bColor); break;
    case 5 : xpgOsdStringFont48("MAY", x, y, bColor); break;
    case 6 : xpgOsdStringFont48("JUN", x, y, bColor); break;
    case 7 : xpgOsdStringFont48("JUL", x, y, bColor); break;
    case 8 : xpgOsdStringFont48("AUG", x, y, bColor); break;
    case 9 : xpgOsdStringFont48("SEP", x, y, bColor); break;
    case 10: xpgOsdStringFont48("OCT", x, y, bColor); break;
    case 11: xpgOsdStringFont48("NOV", x, y, bColor); break;
    case 12: xpgOsdStringFont48("DEC", x, y, bColor); break;
    }

    temp = RtcSettingBuf[2]/10;
    DigitTmpBuf[0] = 0x30 + temp;
    DigitTmpBuf[1] = 0x30 + RtcSettingBuf[2]%10;
    DigitTmpBuf[2] = 0x2F; // "/"
    DigitTmpBuf[3] = 0x00;
    //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&DigitTmpBuf[0]), 50, 60, bColor); // day
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 170, 40, bColor); // day

    Idu_OsdPaintArea(500,60, 300, 30, 0); // reset clear first
    BYTE hour=RtcSettingBuf[3];
    if(RtcSettingBuf[3]>12)
        RtcSettingBuf[3] -= 12;
    temp = RtcSettingBuf[3]/10;
    DigitTmpBuf[0] = 0x30 + temp;
    DigitTmpBuf[1] = 0x30 + RtcSettingBuf[3]%10;
    DigitTmpBuf[2] = 0;
    //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&DigitTmpBuf[0]), 590, 60, bColor); // hour
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 550, 40, bColor); // hour
    if(hour>12)
        //Idu_OsdPutStr(Idu_GetOsdWin(), "PM", 680, 60, bColor);
        xpgOsdStringFont48("PM", 680, 40, bColor);
    else
        //Idu_OsdPutStr(Idu_GetOsdWin(), "AM", 680, 60, bColor);
        xpgOsdStringFont48("AM", 680, 40, bColor);

    temp = RtcSettingBuf[4]/10;
    DigitTmpBuf[0] = 0x30 + temp;
    DigitTmpBuf[1] = 0x30 + RtcSettingBuf[4]%10;
    DigitTmpBuf[2] = 0;
    //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&DigitTmpBuf[0]), 630, 60, bColor); // minute
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 630, 40, bColor); // hour

    flashToggle++;
    if(flashToggle%2)
        //Idu_OsdPutStr(Idu_GetOsdWin(), ":", 600, 60, bColor);
        xpgOsdStringFont48(":", 610, 40, bColor);
    else
        Idu_OsdPaintArea(610, 40, 10, 50, 0); // reset
    Idu_OsdPaintArea(50, 100, 700, 2, bColor); // black line
}



void ShowDayOfPhoto(BYTE x, BYTE y)
{
    BYTE i;
    for(i=0; i<10; i++)
        Idu_OsdPaintArea(WeekXTab[x]-5, WeekYTab[y]+i,  (10-i), 1, 6); // 6=red
    //Idu_OsdPaintArea(WeekXTab[x]-4, WeekYTab[y]+44,  50, 4, 6); // 6=red
}



int monthBlackOffset=0;
int dayBlackOffset=0;

void CalendarBlackProcess()
{
    //mpDebugPrint("CalendarBlackProcess()");
    ShowCalendarHead(OSD_COLOR_WHITE, monthBlackOffset, dayBlackOffset);
    AddTimerProc(1000, CalendarBlackProcess);
}



void CalendarWhiteProcess(void);

///
///@ingroup xpgClock
///@brief   Key Right
///
void CalendarBlackRight()
{
    //mpDebugPrint("CalendarBlackRight()");
    WORD year;
    BYTE month, day, bDays;
    CalendarYearMonthDay(monthBlackOffset, dayBlackOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    if ((day+dayBlackOffset) < bDays)
        dayBlackOffset += 1;
    else mpDebugPrint("(day+dayBlackOffset) == bDays, stop right");
    //EnterCalendarBlack();
    ShowCalendarBlackDayBox(Idu_GetCurrWin(), OSD_COLOR_WHITE, monthBlackOffset, dayBlackOffset);
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Key Left
///
void CalendarBlackLeft()
{
    //mpDebugPrint("CalendarBlackLeft()");
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE day = stSystemTime.u08Day;
    if ((day+dayBlackOffset) > 1)
        dayBlackOffset -= 1;
    else mpDebugPrint("(day+dayBlackOffset) == 1, stop left");
    //EnterCalendarBlack();
    ShowCalendarBlackDayBox(Idu_GetCurrWin(), OSD_COLOR_WHITE, monthBlackOffset, dayBlackOffset);
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Key Up
///
void CalendarBlackUp()
{
    //mpDebugPrint("CalendarBlackUp()");
    monthBlackOffset += 1;
    dayBlackOffset = 0;
    EnterCalendarBlack();
}



///
///@ingroup xpgClock
///@brief   Key Down
///
void CalendarBlackDown()
{
    //mpDebugPrint("CalendarBlackDown()");
    monthBlackOffset -= 1;
    dayBlackOffset = 0;
    EnterCalendarBlack();
}



///
///@ingroup xpgClock
///@brief   Show DayPhoto
///
void CalendarBlackDayPhoto()
{
    mpDebugPrint("CalendarBlackDayPhoto()");
    RemoveTimerProc(CalendarBlackProcess);
    WORD year;
    BYTE month, day, bDays;
    WORD wStartIndex;
    CalendarYearMonthDay(monthBlackOffset, dayBlackOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);
    if((day+dayBlackOffset) < bDays)
        day = day+dayBlackOffset;
    else day = bDays;
    //mpDebugPrint("year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);
    WORD wDayPhotoSum = GetDayOfPhoto(year, month, day, &wStartIndex);

    yearWeekStart =year;
    monthWeekStart=month;
    dayWeekStart  =day;
    yearWeekEnd   =year;
    monthWeekEnd  =month;
    dayWeekEnd    =day;

    if(wDayPhotoSum > 0)
    {
        FILE_SORTING_BASIS_TYPE bFileSortBasis = GetCurFileSortingBasis();
        SetCurFileSortingBasis(5); // by Date/Time
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
        QuickSort((void *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE], psFileBrowser->dwFileListCount[OP_IMAGE_MODE], sizeof(ST_SEARCH_INFO), CompareFileWeekDate, 1); //0);

        Idu_OsdErase();
        xpgSearchAndGotoPage("Mode_Photo");
        xpgCb_EnterDatePhoto(wDayPhotoSum, wStartIndex, bFileSortBasis);
    }
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Enter CalendarBlack
///
int EnterCalendarBlack() //void EnterCalendarBlack()
{
    ST_IMGWIN   * psWin;

    MP_DEBUG("EnterCalendarBlack");
    RemoveTimerProc(UpdateClock);
    RemoveTimerProc(CalendarWhiteProcess); // [Left/Right] ReEntrant
    RemoveTimerProc(CalendarBlackProcess); // [Left/Right] ReEntrant
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    palette6Backup = idu->Palette[6];
    idu->Palette[6]  = (0x0f << 24) + (0x00FF0000);
    ShowCalendar(Idu_GetCurrWin(), OSD_COLOR_WHITE, monthBlackOffset, dayBlackOffset);
    ShowCalendarBlackDayBox(Idu_GetCurrWin(), OSD_COLOR_WHITE, monthBlackOffset, dayBlackOffset);
    AddTimerProc(100, CalendarBlackProcess);
    g_bXpgStatus = XPG_MODE_CALENDARBLACK;
    g_boNeedRepaint = FALSE;
    return 0;
}



///
///@ingroup xpgClock
///@brief   Exit from CalendarBlack
///
void CalendarBlackExit()
{
    MP_DEBUG("CalendarBlackExit");
    RemoveTimerProc(CalendarBlackProcess);
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    idu->Palette[6] = palette6Backup;
    Cal_PhotoStart();
}



// CalendarWhite Function =============================================================
int monthWhiteOffset=0;
int dayWhiteOffset=0;
void CalendarWhiteProcess(void)
{
    MP_DEBUG("CalendarWhiteProcess");
    ShowCalendarHead(10, monthWhiteOffset, dayWhiteOffset); // 10=White
    AddTimerProc(1000, CalendarWhiteProcess);
}



///
///@ingroup xpgClock
///@brief   Key Right
///
void CalendarWhiteRight()
{
    //mpDebugPrint("CalendarWhiteRight()");
    WORD year;
    BYTE month, day, bDays;
    CalendarYearMonthDay(monthWhiteOffset, dayWhiteOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);

    if ((day+dayWhiteOffset) < bDays)
        dayWhiteOffset += 1;
    else mpDebugPrint("(day+dayWhiteOffset) == bDays, stop right");
    //EnterCalendarWhite();
    ShowCalendarWhiteDayBox(Idu_GetCurrWin(), 10, monthWhiteOffset, dayWhiteOffset);
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Key Left
///
void CalendarWhiteLeft()
{
    //mpDebugPrint("CalendarWhiteLeft()");
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE day = stSystemTime.u08Day;
    if ((day+dayWhiteOffset) > 1)
        dayWhiteOffset -= 1;
    else mpDebugPrint("day+dayWhiteOffset == 1, stop left");
    //EnterCalendarWhite();
    ShowCalendarWhiteDayBox(Idu_GetCurrWin(), 10, monthWhiteOffset, dayWhiteOffset);
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Key Up
///
void CalendarWhiteUp()
{
    //mpDebugPrint("CalendarWhiteUp()");
    monthWhiteOffset += 1;
    dayWhiteOffset = 0;
    EnterCalendarWhite();
}



///
///@ingroup xpgClock
///@brief   Key Down
///
void CalendarWhiteDown()
{
    //mpDebugPrint("CalendarWhiteDown()");
    monthWhiteOffset -= 1;
    dayWhiteOffset = 0;
    EnterCalendarWhite();
}



///
///@ingroup xpgClock
///@brief   Show DayPhoto
///
void CalendarWhiteDayPhoto()
{
    //mpDebugPrint("CalendarWhiteDayPhoto(), monthWhiteOffset=%d, dayWhiteOffset=%d", monthWhiteOffset, dayWhiteOffset);
    RemoveTimerProc(CalendarWhiteProcess);
    WORD year;
    BYTE month, day, bDays;
    WORD wStartIndex;
    CalendarYearMonthDay(monthWhiteOffset, dayWhiteOffset, &year, &month, &day, &bDays);
    //mpDebugPrint("after CalendarYearMonthDay(), year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);
    if((day+dayWhiteOffset) < bDays)
        day = day+dayWhiteOffset;
    else day = bDays;
    //mpDebugPrint("year=%d, month=%d, day=%d, bDays=%d", year, month, day, bDays);
    WORD wDayPhotoSum = GetDayOfPhoto(year, month, day, &wStartIndex);

    yearWeekStart =year;
    monthWeekStart=month;
    dayWeekStart  =day;
    yearWeekEnd   =year;
    monthWeekEnd  =month;
    dayWeekEnd    =day;

    if(wDayPhotoSum > 0)
    {
        FILE_SORTING_BASIS_TYPE bFileSortBasis = GetCurFileSortingBasis();
        SetCurFileSortingBasis(5); // by Date/Time
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
        QuickSort((void *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE], psFileBrowser->dwFileListCount[OP_IMAGE_MODE], sizeof(ST_SEARCH_INFO), CompareFileWeekDate, 1); //0);

        Idu_OsdErase();
        xpgSearchAndGotoPage("Mode_Photo");
        xpgCb_EnterDatePhoto(wDayPhotoSum, wStartIndex, bFileSortBasis);
    }
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   Enter Calendar White
///
int EnterCalendarWhite() //void EnterCalendarWhite()
{
    ST_IMGWIN *pWin;

    MP_DEBUG("EnterCalendarWhite");
    RemoveTimerProc(UpdateClock);
    RemoveTimerProc(CalendarWhiteProcess); // [Left/Right] ReEntrant
    RemoveTimerProc(CalendarBlackProcess); // [Left/Right] ReEntrant
    pWin = Idu_GetCurrWin();
    Idu_PaintWinArea(pWin, 0, 0, pWin->wWidth, pWin->wHeight, RGB2YUV(255, 255, 255));
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    palette6Backup = idu->Palette[6];
    idu->Palette[6]  = (0x0f << 24) + (0x00FF0000);
    ShowCalendar(Idu_GetCurrWin(), 10, monthWhiteOffset, dayWhiteOffset);
    ShowCalendarWhiteDayBox(Idu_GetCurrWin(), 10, monthWhiteOffset, dayWhiteOffset);
    AddTimerProc(100, CalendarWhiteProcess);
    g_bXpgStatus = XPG_MODE_CALENDARWHITE;
    g_boNeedRepaint = FALSE;
    return 0;
}



///
///@ingroup xpgClock
///@brief   Exit from Calendar White
///
void CalendarWhiteExit()
{
    MP_DEBUG("CalendarWhiteExit");
    RemoveTimerProc(CalendarWhiteProcess);
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    idu->Palette[6] = palette6Backup;
    //Cal_PhotoStart(); // avoid the case of no photo will recursice error!
}



// WeekPhoto Function =============================================================
WORD weekPhotoMatchCount=0, weekPhotoStartIndex=0;
static void ShowWeekPhoto(WORD x, WORD y)
{
    BYTE i;
    for(i=0; i<10; i++)
        Idu_OsdPaintArea(x, y+i,  (10-i), 1, 6); // 6=red
}



WORD GetWeekPhoto(WORD start_year, WORD start_month, WORD start_day, WORD end_year, WORD end_month, WORD end_day)
{
    DWORD matched_count, start_idx_in_list, matched_folder_count;
    start_year += 2000; //WORD year=2009;
    end_year += 2000;
    ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
    ST_FILE_BROWSER *psFileBrowser = &psSysConfig->sFileBrowser;
    SetCurFileSortingBasis(FILE_SORTING_BY_DATE_TIME);
    SWORD ret = SearchFileList_for_DatePeriod((ST_SEARCH_INFO *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE], psFileBrowser->dwFileListCount[OP_IMAGE_MODE], start_year, start_month, start_day, end_year, end_month, end_day, &matched_count, &start_idx_in_list, &matched_folder_count);
    if(ret == PASS) //if(matched_count > 0)
    {
        mpDebugPrint("(%d%02d%02d - %d%02d%02d): matched_count=%d, start_index=%d, folder_count=%d", start_year, start_month, start_day, end_year, end_month, end_day, matched_count, start_idx_in_list, matched_folder_count);
        weekPhotoStartIndex = start_idx_in_list;
    }
    else
        matched_count = 0;
    return (WORD)matched_count;
}



static void WeekPhotoYearMonthDay(int dayOffset, int weekOffset, WORD *yearIn, BYTE *monthIn, BYTE *dayIn)
{
    //mpDebugPrint("WeekPhotoYearMonthDay(dayOffset=%d, weekOffset=%d, *yearIn=%d, *monthIn=%d, *dayIn=%d)", dayOffset, weekOffset, *yearIn, *monthIn, *dayIn);
    BYTE bDays;
    int sday, wday;
    ST_SYSTEM_TIME stSystemTime;
    WORD year;
    BYTE month, day;
    SystemTimeGet(&stSystemTime);
    day = stSystemTime.u08Day;
    month = stSystemTime.u08Month;
    year = stSystemTime.u16Year - 2000;

    if(weekOffset >= 0)
    {
        //day += weekOffset * 7;
        wday = day + weekOffset * 7;
        bDays = DaysTab[month];
        if ( (month == 2) && LeapYearCheck(year) )
            bDays = 29;
        while(wday > bDays)
        {
            month += 1;
            if(month > 12)
            {
                year += 1;
                month = 1;
            }
            wday = wday - bDays;
            bDays = DaysTab[month];
            if ( (month == 2) && LeapYearCheck(year) )
                bDays = 29;
        }
        day = wday;
        //mpDebugPrint("if(weekOffset >= 0), year=%d, month=%d, day=%d", year, month, day);
    }
    else // weekOffset < 0 - Last week
    {
        sday = day + weekOffset * 7;
        while(sday < 1)
        {
            month -= 1;
            if(month < 1)
            {
                year -= 1;
                month = 12;
            }
            bDays = DaysTab[month];
            if ( (month == 2) && LeapYearCheck(year) )
                bDays = 29;
            sday = bDays + sday;
        }
        day = sday;
        //mpDebugPrint("weekOffset=%d < 0, year=%d, month=%d, day=%d", weekOffset, year, month, day);
    }

    bDays = DaysTab[month];
    if ( (month == 2) && LeapYearCheck(year) )
        bDays = 29;

    if(dayOffset >= 0) // Next days
    {
        day += dayOffset;
        if(day > bDays)
        {
            month += 1;
            if(month > 12)
            {
                year += 1;
                month = 1;
            }
            day = day - bDays;
        }
    }
    else // Previus days
    {
        sday = day + dayOffset;
        if(sday >= 1) // this month
            day = day + dayOffset;
        else // last month
        {
            month -= 1;
            if(month < 1)
            {
                year -= 1;
                month = 12;
            }
            bDays = DaysTab[month];
            if ( (month == 2) && LeapYearCheck(year) )
                bDays = 29;
            day = bDays + sday;
        }
        //mpDebugPrint("dayOffset=%d < 0, year=%d, month=%d, day=%d, bDays=%d, sday=%d", dayOffset, year, month, day, bDays, sday);
    }

    *yearIn  = year;
    *monthIn = month;
    *dayIn   = day;
}



void ShowWeekPhotoBox(WORD x, WORD y, BYTE bColor)
{
    MP_DEBUG("ShowWeekPhotoBox(x=%d, y=%d, bColor=%d)", x, y, bColor);

    Idu_OsdPaintArea(x-10,     y-2,  248,  2, bColor); // top, 248=10+236+2
    Idu_OsdPaintArea(x-10,     y+36, 248,  2, bColor); // bottom
    Idu_OsdPaintArea(x-10,     y-2,    2, 40, bColor); // left, 40=36+2+2
#if(OSD_BIT_WIDTH==4)
    Idu_OsdPaintArea(x+236,    y-2,    2, 40, bColor); // right, 236=width
#elif(OSD_BIT_WIDTH==8)
    Idu_OsdPaintArea(x+240,    y-2,    2, 40, bColor); // right, 238=width
#endif
}



void WeekPhotoProcess(int weekOffset, BYTE bColor)
{
    //mpDebugPrint("WeekPhotoProcess(weekOffset=%d)", weekOffset);
    Idu_OsdErase();
    //BYTE strWeekPhoto[22]="W E E K  P H O T O : ";
    //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&strWeekPhoto[0]), 300, 40, 10);

    BYTE i, bDays, weekTab;
    int weekDay, weekStart, weekEnd, weekMonth;
    BYTE DigitTmpBuf[5];

    ST_SYSTEM_TIME stSystemTime;
    WORD year;
    BYTE month, day;
    SystemTimeGet(&stSystemTime);
    day   = stSystemTime.u08Day;
    month = stSystemTime.u08Month;
    year  = stSystemTime.u16Year - 2000;

    typedef struct
    {
        WORD year;
        BYTE month;
        BYTE day;
    } WEEKPHOTO;
    WEEKPHOTO weekPhoto[7];

    weekTab = GetWeekDay(year, month, day);

    for(i=0; i<=6; i++)
    {
        WeekPhotoYearMonthDay(i-weekTab, weekOffset, &year, &month, &day);
        weekPhoto[i].year  = year;
        weekPhoto[i].month = month;
        weekPhoto[i].day   = day;
    }
     yearWeekStart =  weekPhoto[0].year;
    monthWeekStart =  weekPhoto[0].month;
      dayWeekStart =  weekPhoto[0].day;
     yearWeekEnd   =  weekPhoto[6].year;
    monthWeekEnd   =  weekPhoto[6].month;
      dayWeekEnd   =  weekPhoto[6].day;

    DigitTmpBuf[0] = 0x32; // '2'
    DigitTmpBuf[1] = 0x30; // '0'
    if((year/10)<10)
        DigitTmpBuf[2] = 0x30 + year/10;
    else
        DigitTmpBuf[2] = 0x30 + 10; // show ":"
    DigitTmpBuf[3] = 0x30 + year%10;
    DigitTmpBuf[4] = 0x00;
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 360, 40, bColor); // year

    DigitTmpBuf[0] = 0x30 + weekPhoto[0].month/10;
    DigitTmpBuf[1] = 0x30 + weekPhoto[0].month%10;
    DigitTmpBuf[2] = 0x30 + weekPhoto[0].day/10;
    DigitTmpBuf[3] = 0x30 + weekPhoto[0].day%10;
    DigitTmpBuf[4] = 0x00;
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 300, 100, bColor);

    DigitTmpBuf[0] = 0x30 + weekPhoto[6].month/10;
    DigitTmpBuf[1] = 0x30 + weekPhoto[6].month%10;
    DigitTmpBuf[2] = 0x30 + weekPhoto[6].day/10;
    DigitTmpBuf[3] = 0x30 + weekPhoto[6].day%10;
    DigitTmpBuf[4] = 0x00;
    xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 420, 100, bColor);

    BYTE strWeek[7][5]={"SUN", "MON", "TUE", "WED", "THR", "FRI", "SAT"};
    for (i=0; i<7; i++)
    {
        if(i==weekTab)
        {
            //mpDebugPrint("i==weekTab");
            //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&strWeek[i][0]), 300, 160+i*40+10, 6); // 6=red
            xpgOsdStringFont48((BYTE *)(&strWeek[i][0]), 300, 160+i*40, 6);
        }
        else
            //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&strWeek[i][0]), 300, 160+i*40+10, bColor);
            xpgOsdStringFont48((BYTE *)(&strWeek[i][0]), 300, 160+i*40, OSD_COLOR_WHITE);
        //xpgOsdStringFont48((BYTE *)(&strWeek[i][0]), 300, 200+i*40, 6);
        DigitTmpBuf[0] = 0x30 + weekPhoto[i].month/10;
        DigitTmpBuf[1] = 0x30 + weekPhoto[i].month%10;
        DigitTmpBuf[2] = 0x30 + weekPhoto[i].day/10;
        DigitTmpBuf[3] = 0x30 + weekPhoto[i].day%10;
        DigitTmpBuf[4] = 0x00;
        //Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&DigitTmpBuf[0]), 420, 200+i*40, 10);
        if(i==weekTab && weekOffset==0)
            xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 420, 160+i*40, 6); // 6=red
        else
            xpgOsdStringFont48((BYTE *)(&DigitTmpBuf[0]), 420, 160+i*40, bColor);
    }
    ShowWeekPhotoBox(300, 160+weekTab*40+10, OSD_COLOR_WHITE);
    weekPhotoMatchCount = GetWeekPhoto(weekPhoto[0].year, weekPhoto[0].month, weekPhoto[0].day, weekPhoto[6].year, weekPhoto[6].month, weekPhoto[6].day);
    if(weekPhotoMatchCount>0)
    {
        mpDebugPrint("weekPhotoMatchCount = %d, weekPhotoStartIndex=%d", weekPhotoMatchCount, weekPhotoStartIndex);
        ShowWeekPhoto(330, 40);
    }
}



int weekOffset=0;
///
///@ingroup xpgClock
///@brief   WeekPhoto Next Week
///
void WeekPhotoNextWeek(void)
{
    weekOffset += 1;
    WeekPhotoProcess(weekOffset, OSD_COLOR_WHITE);
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   WeekPhoto Last Week
///
void WeekPhotoPrevWeek(void)
{
    weekOffset -= 1;
    WeekPhotoProcess(weekOffset, OSD_COLOR_WHITE);
    g_boNeedRepaint = FALSE;
}



static WORD CompareFileWeekDate(void * pElement1, void * pElement2, BOOL reverse_order)
// Input: yearWeekStart, monthWeekStart, dayWeekStart, yearWeekEnd, monthWeekEnd, dayWeekEnd;
{
    WORD wYear1, wMonth1, wDay1, wYear2, wMonth2, wDay2;
    WORD value1, value2, ret;
    ST_SEARCH_INFO *pSearchInfo1 = (ST_SEARCH_INFO *) pElement1;
    ST_SEARCH_INFO *pSearchInfo2 = (ST_SEARCH_INFO *) pElement2;


    if ((pSearchInfo1 == NULL) || (pSearchInfo2 == NULL))
    {
        MP_ALERT("%s: NULL pointer element !", __FUNCTION__);
        return ((pSearchInfo2 == NULL)? 1:0); //just return, not very care return value in this case
    }

    value1 = ((pSearchInfo1->DateTime.year - 2000) * 365) + (pSearchInfo1->DateTime.month * 30) + pSearchInfo1->DateTime.day;
    value2 = ((pSearchInfo2->DateTime.year - 2000) * 365) + (pSearchInfo2->DateTime.month * 30) + pSearchInfo2->DateTime.day;

    WORD check_in_range1=0, check_in_range2=0;
    DWORD valueStart, valueEnd, value;
    valueStart = (yearWeekStart * 365) + (monthWeekStart * 30) + dayWeekStart;
    valueEnd   = (yearWeekEnd   * 365) + (monthWeekEnd   * 30) + dayWeekEnd;

    if((value1 >= valueStart) && (value1 <= valueEnd) ) check_in_range1 = 1;
    if((value2 >= valueStart) && (value2 <= valueEnd) ) check_in_range2 = 1;

    if (check_in_range1 == 1 && check_in_range2 == 0) ret = 1;
    if (check_in_range1 == 0 && check_in_range2 == 1) ret = 0;
    if (check_in_range1 == 0 && check_in_range2 == 0) ret = 1;
    if (check_in_range1 == 1 && check_in_range2 == 1) ret = 1;

    return ret;
}



typedef struct
{
	SWORD swLow;
	SWORD swHigh;
} ST_STACK_INFO;

static void SwapData(register DWORD DestAddr, register DWORD SourceAddr, register DWORD dwSize)
{
	BYTE btmpBuffer[128];
//	mpDebugPrint("Swap data: dest(%x), sour(%x) size: %d", DestAddr, SourceAddr, dwSize);
	memcpy((BYTE *) btmpBuffer, (BYTE *) SourceAddr, dwSize);
	memcpy((BYTE *) SourceAddr, (BYTE *) DestAddr, dwSize);
	memcpy((BYTE *) DestAddr,   (BYTE *) btmpBuffer, dwSize);
}



///
///@ingroup xpgClock
///@brief   WeekPhoto current week's photo thumbnail
///
FILE_SORTING_BASIS_TYPE bFileSortBasis;
void WeekPhoto(void)
{
    if(weekPhotoMatchCount >0)
    {
        bFileSortBasis = GetCurFileSortingBasis();
        SetCurFileSortingBasis(5); // by Date/Time
        ST_SYSTEM_CONFIG *psSysConfig = g_psSystemConfig;
        struct ST_FILE_BROWSER_TAG *psFileBrowser = &psSysConfig->sFileBrowser;
        QuickSort((void *) psFileBrowser->dwFileListAddress[OP_IMAGE_MODE], psFileBrowser->dwFileListCount[OP_IMAGE_MODE], sizeof(ST_SEARCH_INFO), CompareFileWeekDate, 1); //0);

        Idu_OsdErase();
        xpgSearchAndGotoPage("Mode_Photo");
        xpgCb_EnterDatePhoto(weekPhotoMatchCount, weekPhotoStartIndex, bFileSortBasis);
        //xpgCb_EnterPhotoMenu();
    }
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   WeekPhoto Start routine
///
void WeekPhotoStart(void)
{
    bFileSortBasis = GetCurFileSortingBasis(); // later for EnterDatePhoto
    //mpDebugPrint("WeekPhotoStart(void), GetCurFileSortingBasis()=bFileSortBasis=%d", bFileSortBasis);
    //Idu_PaintWinArea(Idu_GetCurrWin(), 0, 0, 800, 480, RGB2YUV(255, 255, 255));
    //Idu_PaintWinArea(Idu_GetCurrWin(), 0, 0, 800, 480, RGB2YUV(0, 0, 0));
    ST_IMGWIN * psWin;
    psWin = Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    psWin = Idu_GetCurrWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    palette6Backup = idu->Palette[6];
    idu->Palette[6]  = (0x0f << 24) + (0x00FF0000);
    WeekPhotoProcess(weekOffset, OSD_COLOR_WHITE);
    g_bXpgStatus = XPG_MODE_WEEKPHOTO;
    g_boNeedRepaint = FALSE;
}



///
///@ingroup xpgClock
///@brief   WeekPhoto Exit routine
///
void WeekPhotoExit(void)
{
    Idu_OsdErase();
    IDU *idu = (IDU *) IDU_BASE;
    idu->Palette[6] = palette6Backup;
    xpgSearchAndGotoPage("CalendarWhite");
    EnterCalendarWhite();
}


// DigitRtc =================================================================================================
///
///@ingroup xpgClock
///@brief   DigitRTC main routine
///
void DigitRTCProcess(void)
{
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE year, month, day, hour, minute, second;
    second = stSystemTime.u08Second;
    minute = stSystemTime.u08Minute;
    hour   = stSystemTime.u08Hour;
    day    = stSystemTime.u08Day;
    month  = stSystemTime.u08Month;
    year   = stSystemTime.u16Year - 2000;

    ST_IMGWIN *psWin=Idu_GetCurrWin();
    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite; // // sprite type 43 = SPRITE_TYPE_DIGITCLOCK,
    //WORD DigitClockTab[8] = {40, 130, 220, 310, 400, 490, 580, 670};
    WORD DigitRTCTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wTimeY=260, wType=46;
    if(hour != hourLast)
    {
        hourLast = hour;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, hour/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[0], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, hour%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[1], wTimeY, pstSprite, 0);
    }
    if(minute != minuteLast)
    {
        minuteLast = minute;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, minute/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[3], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, minute%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[4], wTimeY, pstSprite, 0);
    }
    if(second != secondLast)
    {
        secondLast = second;
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, second/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[6], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, second%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[7], wTimeY, pstSprite, 0);
    }

    g_boNeedRepaint = FALSE;
    AddTimerProc(1, DigitRTCProcess);
}



///
///@ingroup xpgClock
///@brief   DigitRTC start routine
///
void xpgCb_DigitRTCStart(void)
{
    mpDebugPrint("xpgCb_DigitRTCStart()");

    ST_IMGWIN *psWin=Idu_GetNextWin();
    Idu_PaintWinArea(psWin, 0, 0, psWin->wWidth, psWin->wHeight, RGB2YUV(0, 0, 0));

    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    WORD DigitRTCTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wDateY=80, wTimeY=260, wType=46;
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE year, month, day, hour, minute, second;
    second = stSystemTime.u08Second;
    minute = stSystemTime.u08Minute;
    hour   = stSystemTime.u08Hour;
    day    = stSystemTime.u08Day;
    month  = stSystemTime.u08Month;
    year   = stSystemTime.u16Year - 2000;

    //pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 2);
    //xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 20, wDateY, 0);
    //pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 0);
    //xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, 80, wDateY, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, year/10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[0], wDateY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, year%10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[1], wDateY, pstSprite, 0);
    //pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    //xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[2], wDateY, 0);
    Idu_PaintWinArea(psWin, 140+130+4, wDateY+50,  40, 8, RGB2YUV(255, 255, 255));
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, month/10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[3], wDateY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, month%10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[4], wDateY, pstSprite, 0);
    //pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    //xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, AlarmTab[5], wDateY, 0);
    Idu_PaintWinArea(psWin, 140+325+4, wDateY+50,  40, 8, RGB2YUV(255, 255, 255));
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, day/10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[6], wDateY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, day%10);
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[7], wDateY, pstSprite, 0);

    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, hour/10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[0], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, hour%10); // Hour
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[1], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[2], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, minute/10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[3], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, minute%10); // Minute
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[4], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, 10); // ":"
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[5], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, second/10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[6], wTimeY, pstSprite, 0);
    pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, second%10); // Second
    xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, DigitRTCTab[7], wTimeY, pstSprite, 0);

    hourLast = minuteLast = secondLast = -1;
    AddTimerProc(1, DigitRTCProcess);
}



unsigned int digitRTCIndex=0; // 0-hour, 1-minute, 2-second
unsigned int digitRTCCount=0;
unsigned int digitRTCYear, digitRTCMonth, digitRTCDay;
unsigned int digitRTCHour, digitRTCMinute, digitRTCSecond;

///
///@ingroup xpgClock
///@brief   DigitRTC setup show time user interface
///
void digitRTCShowTime(void)
{
    ST_IMGWIN *psWin=Idu_GetCurrWin();
    register STXPGMOVIE *pstMov = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    WORD alarmTab[8] = {140+0, 140+65, 140+130, 140+195, 140+260, 140+325, 140+390, 140+455};
    int wDateY=80, wTimeY=260, wType=46;

    if(digitRTCIndex==0)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCYear/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[0], wDateY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCYear%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[1], wDateY, pstSprite, 0);
    } else if(digitRTCIndex==1)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCMonth/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[3], wDateY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCMonth%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[4], wDateY, pstSprite, 0);
    } else if(digitRTCIndex==2)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCDay/10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[6], wDateY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCDay%10);
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[7], wDateY, pstSprite, 0);
    } else if(digitRTCIndex==3)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCHour/10); // Hour
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[0], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCHour%10); // Hour
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[1], wTimeY, pstSprite, 0);
    } else if(digitRTCIndex==4)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCMinute/10); // Minute
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[3], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCMinute%10); // Minute
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[4], wTimeY, pstSprite, 0);
    } else if(digitRTCIndex==5)
    {
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCSecond/10); // Second
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[6], wTimeY, pstSprite, 0);
        pstSprite = xpgSpriteFindType(g_pstXpgMovie, wType, digitRTCSecond%10); // Second
        xpgDirectDrawRoleOnWin(psWin, pstSprite->m_pstRole, alarmTab[7], wTimeY, pstSprite, 0);
    }
}



///
///@ingroup xpgClock
///@brief   DigitRTC setup main routine, recursive by timer
///
void digitRTCShow(void)
{
    //mpDebugPrint("digitRTCShow(), digitRTCCount=%d", digitRTCCount);
    //mpDebugPrint(",");
    ST_IMGWIN *psWin=Idu_GetCurrWin();
    int wDateY=80, wTimeY=260, wType=46;
    //while(1)
    {
        if(digitRTCCount%2)
        {
            digitRTCShowTime();
        } else {
            if(digitRTCIndex==0)
                Idu_PaintWinArea(psWin, 140+0,   wDateY, 130, 110, RGB2YUV(0, 0, 0));
            else if(digitRTCIndex==1)
                Idu_PaintWinArea(psWin, 140+195, wDateY, 130, 110, RGB2YUV(0, 0, 0));
            else if(digitRTCIndex==2)
                Idu_PaintWinArea(psWin, 140+390, wDateY, 130, 110, RGB2YUV(0, 0, 0));
            else if(digitRTCIndex==3)
                Idu_PaintWinArea(psWin, 140+0,   wTimeY, 130, 110, RGB2YUV(0, 0, 0));
            else if(digitRTCIndex==4)
                Idu_PaintWinArea(psWin, 140+195, wTimeY, 130, 110, RGB2YUV(0, 0, 0));
            else if(digitRTCIndex==5)
                Idu_PaintWinArea(psWin, 140+390, wTimeY, 130, 110, RGB2YUV(0, 0, 0));
        }
        digitRTCCount++;
        if(digitRTCCount==10000) digitRTCCount=0; // loop again
        AddTimerProc(100, digitRTCShow);
    }
}



///
///@ingroup xpgClock
///@brief   DigitRTC Setup event entry
///
void xpgCb_DigitRTCSetup(void)
{
    mpDebugPrint("xpgCb_DigitRTCSetup()");
    Setup_SetNoneXpgMode(SETUP_KEYMAP_DIGITRTC); // in global612.h
    RemoveTimerProc(DigitRTCProcess);
    g_boNeedRepaint = FALSE;
    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE year, month, day, hour, minute, second;
    second = stSystemTime.u08Second;
    minute = stSystemTime.u08Minute;
    hour   = stSystemTime.u08Hour;
    day    = stSystemTime.u08Day;
    month  = stSystemTime.u08Month;
    year   = stSystemTime.u16Year - 2000;
    digitRTCYear=year, digitRTCMonth=month, digitRTCDay=day;
    digitRTCHour=hour, digitRTCMinute=minute, digitRTCSecond=second;
    digitRTCShowTime();
    digitRTCShow();
    //mpDebugPrint("digitRTC() exit");
}



///
///@ingroup xpgClock
///@brief   Exit from DigitRTC
///
void xpgCb_DigitRTCExit(void)
{
    mpDebugPrint("xpgCb_DigitRTCExit()");
    RemoveTimerProc(DigitRTCProcess);
    //mpDebugPrint("xpgCb_DigitRTCExit() exit");
}
///
///@ingroup xpgClock
///@brief   DigitRTC Setup key event UP protine
///
void DigitRTCSetupUp()
{
    MP_DEBUG("DigitRTCSetupUp");
    mpDebugPrint("DigitRTCSetupUp()");
    switch(digitRTCIndex)
    {
    case 0 : digitRTCYear++;   break;
    case 1 : digitRTCMonth++; if(digitRTCMonth==13) digitRTCMonth=1; break;
    case 2 : if(digitRTCDay < DaysTab[digitRTCMonth]) digitRTCDay++; break;
    case 3 : digitRTCHour++;   if(digitRTCHour==24) digitRTCHour=0; break;
    case 4 : digitRTCMinute++; if(digitRTCMinute==60) digitRTCMinute=0; break;
    case 5 : digitRTCSecond++; if(digitRTCSecond==60) digitRTCSecond=0; break;
    }
}



///
///@ingroup xpgClock
///@brief   DigitRTC Setup key event DOWN protine
///
void DigitRTCSetupDown()
{
    MP_DEBUG("DigitRTCSetupDown");
    mpDebugPrint("DigitRTCSetupDown(), digitRTCIndex=%d", digitRTCIndex);
    switch(digitRTCIndex)
    {
    case 0 : if(digitRTCYear>0) digitRTCYear--;
             break;
    case 1 : if(digitRTCMonth>0) digitRTCMonth--;
             else digitRTCMinute=12;
             break;
    case 2 : if(digitRTCDay>1) digitRTCDay--;
             break;
    case 3 : if(digitRTCHour>0) digitRTCHour--;
             else digitRTCHour=23;
             break;
    case 4 : if(digitRTCMinute>0) digitRTCMinute--;
             else digitRTCMinute=59;
             break;
    case 5 : if(digitRTCSecond>0) digitRTCSecond--;
             else digitRTCSecond=59;
             break;
    }
}



///
///@ingroup xpgClock
///@brief   DigitRTC Setup key event RIGHT protine
///
void DigitRTCSetupRight()
{
    MP_DEBUG("DigitRTCSetupRight");
    mpDebugPrint("DigitRTCSetupRight()");
    digitRTCShowTime(); // show last setting
    digitRTCIndex++;
    if(digitRTCIndex==7) digitRTCIndex=0;
}



///
///@ingroup xpgClock
///@brief   DigitRTC Setup key event LEFT protine
///
void DigitRTCSetupLeft()
{
    MP_DEBUG("DigitRTCSetupLeft");
    mpDebugPrint("DigitRTCSetupLeft()");
    digitRTCShowTime(); // show last setting
    if(digitRTCIndex>0) digitRTCIndex--;
    else digitRTCIndex=5;
    if(digitRTCCount%2) digitRTCCount++;
}



///
///@ingroup xpgClock
///@brief   DigitRTC Setup Entry init routine
///
void DigitRTCSetupEnter()
{
    MP_DEBUG("DigitRTCSetupEnter");
    mpDebugPrint("DigitRTCSetupEnter()");
    digitRTCShowTime(); // show last setting
    RemoveTimerProc(digitRTCShow);

    ST_SYSTEM_TIME stSystemTime;
    stSystemTime.u08Second = digitRTCSecond;
    stSystemTime.u08Minute = digitRTCMinute;
    stSystemTime.u08Hour = digitRTCHour;
    stSystemTime.u08Day = digitRTCDay;
    stSystemTime.u08Month = digitRTCMonth;
    stSystemTime.u16Year = digitRTCYear + 2000;
    SystemTimeSet(&stSystemTime);

    Setup_SetXpgMode();
}



///
///@ingroup xpgClock
///@brief   Exit from DigitRTC Setup
///
void DigitRTCSetupExit()
{
    MP_DEBUG("DigitRTCSetupExit");
    mpDebugPrint("DigitRTCSetupExit()");
    digitRTCShowTime(); // show last setting
    RemoveTimerProc(digitRTCShow);
    /*
    ST_SYSTEM_TIME stSystemTime;
    stSystemTime.u08Second = digitRTCSecond;
    stSystemTime.u08Minute = digitRTCMinute;
    stSystemTime.u08Hour = digitRTCHour;
    stSystemTime.u08Day = digitRTCDay;
    stSystemTime.u08Month = digitRTCMonth;
    stSystemTime.u16Year = digitRTCYear + 2000;
    SystemTimeSet(&stSystemTime);
    */
    Setup_SetXpgMode();
}



// Rtc =================================================================================================
///
///@ingroup xpgClock
///@brief   Clear RTC setting
///
void ClearRtcSetting()
{
    MP_DEBUG("ClearRtcSetting");
    Idu_OsdPaintArea(RtcSettingX[0], RtcSettingX[6], (RtcSettingX[0]-RtcSettingX[5]), 32, 0);
}



///
///@ingroup xpgClock
///@brief   Show RTC setting clock
///
///@param   bIndex - type
///
void RtcSettingShowClock(BYTE bIndex)
{
    MP_DEBUG("RtcSettingShowClock");
    BYTE temp, i;
    BYTE RtcTmpBuf[6];
    WORD screenWidth = g_psSystemConfig->sScreenSetting.pstCurTCON->wWidth;
    WORD screenHeight = g_psSystemConfig->sScreenSetting.pstCurTCON->wHeight;
    WORD RtcSetCurrentTimeX=screenWidth/5; // 160
    WORD RtcY=screenHeight - (screenHeight/120*7); // 480-28=452

    // year
    RtcTmpBuf[0] = 0x32;
    RtcTmpBuf[1] = 0x30;
    RtcTmpBuf[2] = 0x30 + RtcSettingBuf[2] / 10;
    RtcTmpBuf[3] = 0x30 + RtcSettingBuf[2] % 10;
    RtcTmpBuf[4] = 0x20;
    RtcTmpBuf[5] = 0;

    ClearRtcSetting();
    //DrawRTC_TimeTag();
    Idu_OsdPutStr(Idu_GetOsdWin(), "Set Current Time: ", RtcSetCurrentTimeX, RtcY, CLOCK_COLOR);

    if (bIndex == 2)
        Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&RtcTmpBuf[0]), RtcSettingX[2], RtcSettingX[6], SETTING_COLOR);     //byAlexWang 26apr2008
    else
        Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&RtcTmpBuf[0]), RtcSettingX[2], RtcSettingX[6], CLOCK_COLOR);

    for (i = 0; i < 5; i++)
    {
        if (i == 2) i++;

        temp = RtcSettingBuf[i]/10;

        if(temp)
            RtcTmpBuf[0] = 0x30 + temp;
        else
            RtcTmpBuf[0] = 0x30;

        RtcTmpBuf[1] = 0x30 + RtcSettingBuf[i]%10;
        RtcTmpBuf[2] = RtcSettingGapChar[i];
        RtcTmpBuf[3] = 0;

        if(i == bIndex)
            Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&RtcTmpBuf[0]), RtcSettingX[i], RtcSettingX[6], SETTING_COLOR);
        else
            Idu_OsdPutStr(Idu_GetOsdWin(), (BYTE *)(&RtcTmpBuf[0]), RtcSettingX[i], RtcSettingX[6], CLOCK_COLOR);
    }
}



///
///@ingroup xpgClock
///
///@brief   RTC Setup entry routine
///
///@param   bIndex - pass to RtcSettingShowClock()
///
///@remark The item of index will be high light
void EnterRtcSetup(BYTE bIndex)
{
    MP_DEBUG("EnterRtcSetup(bIndex=%d)", bIndex);
    ST_SYSTEM_TIME stSystemTime;

    bRTC_Status = RTC_RESETCNT_MODE;
    SettingIndex = bIndex;
    SystemTimeGet(&stSystemTime);
    RtcSettingBuf[4] = stSystemTime.u08Minute;
    RtcSettingBuf[3] = stSystemTime.u08Hour;
    RtcSettingBuf[2] = stSystemTime.u16Year - 2000;
    RtcSettingBuf[1] = stSystemTime.u08Month;
    RtcSettingBuf[0] = stSystemTime.u08Day;

    StopRtcDisplay();
    ClearClock();

    if(g_bXpgStatus == XPG_MODE_ANALOGCLOCK)
        AnaClockStart();

    Setup_SetNoneXpgMode(SETUP_KEYMAP_RTC);
    RtcSettingShowClock(SettingIndex);
}



///
///@ingroup xpgClock
///@brief   RTC Setup key event UP routine
///
void RtcSetupUp()
{
    MP_DEBUG("RtcSetupUp");
    BYTE bMaxVal;

    if (SettingIndex == 0)          // day
    {
        if(RtcSettingBuf[1] == 2)   // February
        {
            if (RtcSettingBuf[2] & 0x03)
                bMaxVal = 28;
            else
                bMaxVal = 29;
        }
        else
            bMaxVal = DaysTab[RtcSettingBuf[1]];
    }
    else
        bMaxVal = RtcSettingMaxVal[SettingIndex];

    RtcSettingBuf[SettingIndex]++;

    if (RtcSettingBuf[SettingIndex] > bMaxVal)
        RtcSettingBuf[SettingIndex] = RtcSettingMinVal[SettingIndex];

    // check if year or month change and day over the max value
    if (RtcSettingBuf[1] == 2) // February
    {
        if(RtcSettingBuf[2]& 0x03)
            bMaxVal = 28;
        else
            bMaxVal = 29;
    }
    else
        bMaxVal = DaysTab[RtcSettingBuf[1]];

    if(RtcSettingBuf[0] > bMaxVal)
        RtcSettingBuf[0] = bMaxVal;

    RtcSettingShowClock(SettingIndex);
}



///
///@ingroup xpgClock
///@brief   RTC Setup key event DOWN routine
///
void RtcSetupDown()
{
    MP_DEBUG("RtcSetupDown");
    BYTE bMaxVal;

    if(SettingIndex == 0)     // day
    {
        if(RtcSettingBuf[1] == 2) // February
        {
            if(RtcSettingBuf[2]& 0x03)
                bMaxVal = 28;
            else
                bMaxVal = 29;
        }
        else
            bMaxVal = DaysTab[RtcSettingBuf[1]];
    }
    else
        bMaxVal = RtcSettingMaxVal[SettingIndex];

    if(RtcSettingBuf[SettingIndex] == RtcSettingMinVal[SettingIndex])
        RtcSettingBuf[SettingIndex] = bMaxVal;
    else
        RtcSettingBuf[SettingIndex]--;

    // check if month change and day over the max value
    if(RtcSettingBuf[1] == 2) // February
    {
        if(RtcSettingBuf[2]& 0x03)
            bMaxVal = 28;
        else
            bMaxVal = 29;
    }
    else
        bMaxVal = DaysTab[RtcSettingBuf[1]];

    if(RtcSettingBuf[0] > bMaxVal)
        RtcSettingBuf[0] = bMaxVal;

    RtcSettingShowClock(SettingIndex);
}



///
///@ingroup xpgClock
///@brief   RTC Setup key event LEFT routine
///
void RtcSetupLeft()
{
    MP_DEBUG("RtcSetupLeft");
    if (SettingIndex)
        SettingIndex--;
    else
        SettingIndex = 4;

    RtcSettingShowClock(SettingIndex);
}



///
///@ingroup xpgClock
///@brief   Alarm Setup key event RIGHT routine
///
void RtcSetupRight()
{
    MP_DEBUG("RtcSetupRight");
    if(SettingIndex >= 4)
        SettingIndex = 0;
    else
        SettingIndex++;

    RtcSettingShowClock(SettingIndex);
}



///
///@ingroup xpgClock
///@brief   Exit from RTC Setup
///
void RtcSetupExit()
{
    ST_SYSTEM_TIME stSystemTime;

    ClearClock();

    if (g_bXpgStatus == XPG_MODE_ANALOGCLOCK)
        AnaClockStart();
    else if (g_bXpgStatus == XPG_MODE_CALPHOTO)
    {
        Cal_PhotoUpdate();
        //bRTC_Status = RTC_NONE_MODE;
    }

    Setup_SetXpgMode();
    StartRtcDisplay();
    bRTC_Status = RTC_NONE_MODE;
    SystemTimeGet(&stSystemTime);
    ShowClock(&stSystemTime);
    RemoveTimerProc(UpdateClock);
    UpdateClock();
}



///
///@ingroup xpgClock
///@brief   RTC Setup Entry routine
///
void RtcSetupEnter()
{
    MP_DEBUG("RtcSetupExit");
#if VCALENDAR
   VCALENDAR_EVENT_TIME  *pstVcalendarEventTime = &pstVcalendarEventTimeTag->m_stVcalendarEventTag[gbEventIndex];
#endif
    ST_SYSTEM_TIME TmpRtc;

    TmpRtc.u08Second = 0;
    TmpRtc.u08Minute = RtcSettingBuf[4];
    TmpRtc.u08Hour = RtcSettingBuf[3];
    TmpRtc.u08Day = RtcSettingBuf[0];
    TmpRtc.u08Month = RtcSettingBuf[1];
    TmpRtc.u16Year = RtcSettingBuf[2] + 2000;
    //TmpRtc.Weekdays = GetWeekDay(TmpRtc.Years, TmpRtc.Months, TmpRtc.Days);

 #if VCALENDAR
    if (bRTC_Status == RTC_VCALENDAR_EVENT_MODE)
    {
        pstVcalendarEventTime->Seconds = 0;
        pstVcalendarEventTime->Minutes = TmpRtc.Minutes;
        pstVcalendarEventTime->Hours = TmpRtc.Hours;
        pstVcalendarEventTime->Days = 0;
        pstVcalendarEventTime->Months = 0;
        pstVcalendarEventTime->Years = 0;

        if (gbFuncSel == EVENT_ADD)
        {
            pstVcalendarEventTimeTag->m_dwFastSearchTag = (gbAtDayIndexYear << 24) |
                                                          (gbAtDayIndexMonth << 16) |
                                                          (gbAtDayIndexDay << 8);
            pstVcalendarEventTimeTag->m_EventCount++;
            gbVcalendarListCount++;

            if (gbEventIndex == 0)
                VCalendarEventCount++;
        }
    }
    else
#endif
    {
        SystemTimeSet(&TmpRtc);
    }

    bRTC_Status = RTC_NONE_MODE;
    RtcSetupExit();
}



///
///@ingroup xpgClock
///
///@brief   RTC Alarm time set routine
///
void RtcAlarmTimeSet() //fengrs 03/22 setting internal RTC alarm by user
{
    MP_DEBUG("RtcAlarmTimeSet");
//#if ( ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650) || (CHIP_VER == (CHIP_VER_615 | CHIP_VER_B)) )
    ST_SYSTEM_TIME stSystemTime;

    Setup_Exit();
    bRTC_Status = RTC_ALARM_MODE;
    SettingIndex = 0;
    RtcSettingBuf[4] = stSystemTime.u08Minute;
    RtcSettingBuf[3] = stSystemTime.u08Hour;
    RtcSettingBuf[2] = stSystemTime.u16Year - 2000;
    RtcSettingBuf[1] = stSystemTime.u08Month;
    RtcSettingBuf[0] = stSystemTime.u08Day;

    StopRtcDisplay();
    ClearClock();
    Setup_SetNoneXpgMode(SETUP_KEYMAP_RTC);
    RtcSettingShowClock(SettingIndex);
//#endif
}



///
///@ingroup xpgClock
///@brief   Ui Rtc Isr - out of use
///
static void uiRtcIsr(void)
{
    MP_ALERT("UI_RtcIsr");

    RTC_AlarmEnableSet(FALSE);
    RTC_AlarmIeEnableSet(FALSE);
    //Ui_SetIrKey(KEY_INFO);
    //EventSet(UI_EVENT, EVENT_IR_KEY_DOWN);
}

#define DefaultYear   		2013
#define DefaultMonth   		03
#define DefaultDate  			21
#define DefaultHour 			8
#define DefaultMin  			0

void ResetRTCTime()
{
	ST_SYSTEM_TIME stSystemTime,stDefaultTime;
	DWORD dwSysCnt,dwDefaultCnt;
	BYTE bReset=0;

	SystemTimeGet(&stSystemTime);
	stDefaultTime.u16Year=DefaultYear;
	stDefaultTime.u08Month=DefaultMonth;
	stDefaultTime.u08Day=DefaultDate;
	stDefaultTime.u08Hour=DefaultHour;
	stDefaultTime.u08Minute=DefaultMin;
	stDefaultTime.u08Second=0;
	dwSysCnt=SystemTimeDateToSecConv(&stSystemTime);
	dwDefaultCnt=SystemTimeDateToSecConv(&stDefaultTime);

		mpDebugPrint("!!!!!!!!!ResetRTCTime  %d.%d -> %d.%d",stSystemTime.u16Year,stSystemTime.u08Month,stDefaultTime.u16Year,stDefaultTime.u08Month);
		SystemTimeSet(&stDefaultTime);
}

void CheckRTCTime()
{
	ST_SYSTEM_TIME stSystemTime,stDefaultTime;
	DWORD dwSysCnt,dwDefaultCnt;
	BYTE bReset=0;

	SystemTimeGet(&stSystemTime);
	stDefaultTime.u16Year=DefaultYear;
	stDefaultTime.u08Month=DefaultMonth;
	stDefaultTime.u08Day=DefaultDate;
	stDefaultTime.u08Hour=DefaultHour;
	stDefaultTime.u08Minute=DefaultMin;
	stDefaultTime.u08Second=0;
	dwSysCnt=SystemTimeDateToSecConv(&stSystemTime);
	dwDefaultCnt=SystemTimeDateToSecConv(&stDefaultTime);
	if ((dwSysCnt < dwDefaultCnt)||(stSystemTime.u16Year>2040))
	{
		mpDebugPrint("!!!!!!!!!CheckRTCTime  %d.%d -> %d.%d",stSystemTime.u16Year,stSystemTime.u08Month,stDefaultTime.u16Year,stDefaultTime.u08Month);
		SystemTimeSet(&stDefaultTime);
	}
}
#endif

