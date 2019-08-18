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
* Filename      : Calendar.c
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

#if XPG_ROLE_USE_CACHE
#define ROLE_DAY_HL		13

typedef struct
{
	WORD x;
	WORD y;
	WORD w;
	WORD h;
} ST_RECT; 

#if (TCON_ID == TCON_RGB24Mode_800x480) // 800x480
static ST_RECT g_rectCalendarDays = {

	468,
  	268,
	40,
	26,
};
#else // default for 800x600 
static ST_RECT g_rectCalendarDays = {

	516, // 470,
  	418,
	40, // 38,
	26,
};
#endif

static ST_RECT g_rectCalanderWeekTitle = 
{
	400,
  	216,
	40,
	22,
};

static ST_RECT g_rectCalanderHeader = 
{
	400,
  	190,
	280,
	22,
};

static ST_RECT g_rectCalanderYear = 
{
	583,
  	359,
	60,
	22,
};

static ST_SYSTEM_TIME * g_stCalendar; 
static ST_SYSTEM_TIME * g_pCalendar = &g_stCalendar;

static ST_SYSTEM_TIME * g_stTime; 
static ST_SYSTEM_TIME * g_pTime = &g_stTime;

static ST_RECT g_stRect; 
static ST_RECT * g_pRect = &g_stRect; 

void xpgCb_CalendarNow()
{
    SystemTimeGet(g_pTime);
    memcpy(g_pCalendar, g_pTime, sizeof(ST_SYSTEM_TIME)); 
    xpgCb_Calendar_Update(); 
}
void xpgCb_Calendar_Update()
{	 
    //mpDebugPrint("xpgCb_Calendar_Update()");

	WORD year = g_pCalendar->u16Year; 
	BYTE month = g_pCalendar->u08Month;
	BYTE day = g_pCalendar->u08Day;
	mpDebugPrint("FujiCalendar: year = %d, month = %d, day = %d", year, month, day);
	
	ST_IMGWIN *pWin = Idu_GetCurrWin();
	
	//xpgDrawSprite(pWin, &g_pstXpgMovie->m_astSprite[Calendar_BackGround], 0); 
	
	memcpy(g_pRect, &g_rectCalanderHeader, sizeof(ST_RECT)); 
	Calendar_DrawHeader(pWin
			                , year
			                , month			              
			                , g_pRect);
	
	memcpy(g_pRect, &g_rectCalendarDays, sizeof(ST_RECT)); 
	Calendar_DrawDays(pWin
			                , year
			                , month
			                , day
			                , g_pRect); 

	memcpy(g_pRect, &g_rectCalanderYear, sizeof(ST_RECT)); 
	Calendar_DrawYear(pWin
			                , year
			                , month
			                , day
			                , g_pRect);
                             
}

// ----------------------------------------------------------------------
// Calendar Draw Func
// ----------------------------------------------------------------------
void xpgDrawTextCenter(ST_IMGWIN * pWin,ST_RECT *pRect, char * str,DWORD dwColor)
{
	if (!str)
		return; 

	Idu_SetFontYUV(dwColor); 

	BYTE boUnicode = g_bNCTable;	
	
	boUnicode = 0; 
	
	BYTE FontSize = 18;

	int str_w = Idu_GetStringWidthWithSize(str, boUnicode,FontSize);
	int str_h =  Idu_GetStringHeight(str,boUnicode);
	str_h =26; 
		
	int x = pRect->x;
	int y = pRect->y; 
	int w = pRect->w; 
	int h = pRect->h;

	if (w >  str_w)
		x += (w -str_w) / 2; 
#if 0
	if (h > str_h)
		y += (h- str_h) / 2; 
#endif
	y -= 2; 

	Idu_PrintStringWithSize(pWin
							,str
							,x
							,y
							,boUnicode
							,w
							,h
							,FontSize);

}

// --------------------------------------------------------------------
char g_msg[16]; 

DWORD Color_Sunday;
DWORD Color_NormalDay; 
DWORD Color_Today; 
DWORD Color_Title; 
DWORD Color_Year;

void Calendar_Init()
{
	static BOOL boInited = FALSE; 

	if (boInited)
		return; 
	
//	Color_Sunday = RGB2YUV(200, 0, 0);
	Color_Sunday = Color_NormalDay = RGB2YUV(107, 72, 39); // RGB2YUV(240, 240, 240); 
	Color_Today = RGB2YUV(107, 72, 39); // RGB2YUV(10, 200, 10); 
	Color_Title = RGB2YUV(20, 0, 200); 
	Color_Year = RGB2YUV(200, 200, 200); 
	boInited = TRUE; 
}

void Calendar_DrawSunday(ST_IMGWIN *pWin, BYTE day, ST_RECT * pRect)
{	
	sprintf(g_msg,"%d",day);	
	xpgDrawTextCenter(pWin, pRect, g_msg,Color_Sunday);
}

void Calendar_DrawToday(ST_IMGWIN *pWin, BYTE day, ST_RECT * pRect)
{
#if 1
	STXPGSPRITE *pSprite = &g_pstXpgMovie->m_astSprite[ROLE_DAY_HL];
	xpgDirectDrawRoleOnWin(Idu_GetCurrWin()
		                               ,pSprite->m_pstRole

		                               ,pRect->x+((pRect->w - pSprite->m_wWidth)/2) //pSprite->m_wPx
		                               ,pRect->y //pSprite->m_wPy
		                               ,pSprite
		                               ,0);
#else
	Idu_PaintWinArea(pWin, pRect->x, pRect->y+1, pRect->w, pRect->h, RGB2YUV(0,0,240));
#endif
	sprintf(g_msg,"%d",day);	
	xpgDrawTextCenter(pWin, pRect, g_msg,Color_Today);	

}

void Calendar_DrawNormalDay(ST_IMGWIN *pWin, BYTE day, ST_RECT * pRect)
{

	sprintf(g_msg,"%d",day);	
	xpgDrawTextCenter(pWin, pRect, g_msg,Color_NormalDay);	
}


//--------------------------
// Draw calendar Header
//--------------------------
void Calendar_DrawHeader(ST_IMGWIN * psWin, WORD year, BYTE month, ST_RECT* pRect)
{
#if 0
	sprintf(g_msg,"%04d/%02d",year, month);
	xpgDrawTextCenter(psWin, pRect, g_msg,Color_Title);	
#endif
	BYTE Month_RoleArry[]= { 1, 2, 3, 4,5,6,7,8,9,10, 11,12 }; 
	STXPGSPRITE *pSprite = &g_pstXpgMovie->m_astSprite[Month_RoleArry[0]]; 

	xpgDirectDrawRoleOnWin(Idu_GetCurrWin()
		                               ,g_pstXpgMovie->m_astSprite[Month_RoleArry[month-1]].m_pstRole

		                               ,pSprite->m_wPx
		                               ,pSprite->m_wPy
		                               ,pSprite
		                               ,0);

}


//--------------------------
// Draw calendar week Title
//--------------------------
void Calendar_DrawWeekTitle(ST_IMGWIN * psWin, ST_RECT *pRect)
{
const char * strArry[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" }; 

	int i;
	for ( i = 0; i < 7; i++)
	{
		xpgDrawTextCenter(psWin, pRect, strArry[i], Color_Title);	
		pRect->x += pRect->w; 
	}
}

//--------------------------
// Draw calendar days
//--------------------------
#define WEEKDAY_BAS             6   // THE week day of 2000.1.1 is 6
static const BYTE DaysTab[13] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static BYTE GetWeekDay(WORD year, BYTE month, BYTE day)
{
	MP_DEBUG("GetWeekDay");
	DWORD temp;
	BYTE i;

	if (year)
		temp = year * 365 + ((year - 1) >> 2) + 1;
	else
		temp = 0;

	for (i=1; i<month; i++)
		temp += DaysTab[i];

	if ((month > 2) && (!(year & 0x03)))
		temp++;

	temp += (day - 1);

	temp += WEEKDAY_BAS;

	return (BYTE)(temp%7);
}

BYTE GetMonthDays(WORD year, BYTE month)
{
	if (month == 2 && LeapYearCheck(year)) 
		return 29;

	return DaysTab[month];
}

void Calendar_DrawDays(ST_IMGWIN * psWin, WORD year, BYTE month, BYTE today, ST_RECT* pRect)
{
	BYTE bDay;
	int x, y;

	int start_x = pRect->x;  
	int start_y = pRect->y; 
	

	BYTE bWeekDay = GetWeekDay(year-2000, month, 1); 
	BYTE bMonthDays = GetMonthDays(year, month);		

	//mpDebugPrint("%s", __func__);
	//mpDebugPrint("year =%d, month =%d", year, month);
	//mpDebugPrint("WeekDay =%d, MonthDays=%d", bWeekDay, bMonthDays); 

	x =  start_x + bWeekDay * pRect->w;
	y = pRect->y; 

	Calendar_Init(); 
		
	for(bDay =1; bDay<= bMonthDays; bDay++)
	{
		pRect->x = x; 
		pRect->y = y; 
		
		if (bDay ==today)
			Calendar_DrawToday(psWin, bDay,pRect); 
		else if (bWeekDay) 
			Calendar_DrawNormalDay(psWin, bDay,pRect); 
		else
			Calendar_DrawSunday(psWin, bDay,pRect); 
		
		if(bWeekDay < 6)
		{
			x += pRect->w;		
			bWeekDay++;
		}
		else
		{
			bWeekDay = 0;
			x = start_x;
			y += pRect->h;		
		}
			
	}
}

//--------------------------
// Draw calendar year
//--------------------------
void Calendar_DrawYear(ST_IMGWIN * psWin, WORD year, BYTE month, BYTE today, ST_RECT* pRect)
{
	sprintf(g_msg,"%d",year);
	xpgDrawTextCenter(psWin, pRect, g_msg,Color_Year);
}

#endif
