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
* Filename      : Clock.c
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
static void My_Img_Rotate180(ST_IMGWIN * target, ST_IMGWIN * source)
{
	ST_PIXEL *ip, *op;
	int pixel, line;

	line = 0;
	while ( line < (int) source->wHeight )
	{
		ip = (ST_PIXEL *) source->pdwStart + (line*source->dwOffset/4);
		op = (ST_PIXEL *) (target->pdwStart + (((target->wHeight-line-1)*(target->dwOffset/4)) + (target->wWidth/2) -1 ));

		pixel = source->wWidth;
		while (pixel > 0)
		{
			op->Y0 = ip->Y1;
			op->Y1 = ip->Y0;
			op->Cb = ip->Cb;
			op->Cr = ip->Cr;

			ip++;
			op--;
			pixel -= 2;
		}
		line++;
	}
}

ST_IMGWIN rotate_win;
static void MakeRotateRole(STXPGROLE* pSrcRole, STXPGROLE* pDstRole, int angle)
{
	ST_IMGWIN src_win;

	src_win.wWidth = ALIGN_CUT_2(pSrcRole->m_wWidth);
	src_win.wHeight = ALIGN_CUT_2(pSrcRole->m_wHeight);
	src_win.pdwStart = pSrcRole->m_pRawImage;
	src_win.dwOffset = pSrcRole->m_wRawWidth * 2;
//	mpDebugPrint("%s w=%d h = %d", __func__, rotate_win.wWidth, rotate_win.wHeight); 
	if ( angle == 90 )
	{
		rotate_win.wWidth = src_win.wHeight;
		rotate_win.wHeight = src_win.wWidth;
		rotate_win.pdwStart = ext_mem_malloc(rotate_win.wWidth * rotate_win.wHeight * 2);
		rotate_win.dwOffset = rotate_win.wWidth * 2;
		Img_Rotate90(&rotate_win, &src_win);
	}
	else if ( angle == 180 )
	{
		rotate_win.wWidth = src_win.wWidth;
		rotate_win.wHeight = src_win.wHeight;
		rotate_win.pdwStart = ext_mem_malloc(rotate_win.wWidth * rotate_win.wHeight * 2);
		rotate_win.dwOffset = rotate_win.wWidth * 2;
		My_Img_Rotate180(&rotate_win, &src_win);
	}
	else
	{
		rotate_win.wWidth = src_win.wHeight;
		rotate_win.wHeight = src_win.wWidth;
		rotate_win.pdwStart = ext_mem_malloc(rotate_win.wWidth * rotate_win.wHeight * 2);
		rotate_win.dwOffset = rotate_win.wWidth * 2;
		Img_Rotate270(&rotate_win, &src_win);
	}
	
	memcpy(pDstRole, pSrcRole, sizeof(STXPGROLE));
	pDstRole->m_wWidth = rotate_win.wWidth;
	pDstRole->m_wHeight = rotate_win.wHeight;
	pDstRole->m_wRawWidth = rotate_win.wWidth;
	pDstRole->m_pRawImage = rotate_win.pdwStart;
			
}

typedef struct 
{
	int x;
	int y;
} ST_POINT; 
ST_POINT PinOffsetArry[] = 
{
	{128	, 132	}, 
	{132	, 128	}, 	
	{128	, 132	}, 
	{132	, 128	}, 	
}; 

static ST_IMGWIN g_stCacheWin = {0};

ST_IMGWIN *Idu_GetCacheWin_WithInit()
{
	ST_IMGWIN *pCacheWin=&g_stCacheWin, *pWin = Idu_GetCurrWin();
	
	mpDebugPrint("Idu_GetCacheWin_WithInit 0x%08x",g_stCacheWin.pdwStart);
	if (pCacheWin->pdwStart == NULL)
	{
		ImgWinInit(pCacheWin, NULL, pWin->wHeight, pWin->wWidth);
		pCacheWin->pdwStart = ext_mem_malloc(pWin->wWidth * pWin->wHeight * 2);
		Slide_WinCopy(pWin, pCacheWin);
	}
	return pCacheWin;
}
ST_IMGWIN *Idu_GetCacheWin()
{
	return &g_stCacheWin;
}
void Free_CacheWin()
{
	mpDebugPrint("Free_CacheWin 0x%08x",g_stCacheWin.pdwStart);
	if (g_stCacheWin.pdwStart )
	{
		ext_mem_free(g_stCacheWin.pdwStart);
		g_stCacheWin.pdwStart=NULL;
	}
}

/*
ST_IMGWIN *pCacheWin;
BYTE dayBackup = 0;
void UpdateFujiClock(void)
{
    //mpDebugPrint("UpdateFujiClock()");
    
    ST_IMGWIN *pCurrWin = Idu_GetCurrWin();
    ST_IMGWIN *pNextWin = Idu_GetNextWin();
	//-----------------------------------------------------------------
	
	pCacheWin = Idu_GetCacheWin();
	if ( pCacheWin->pdwStart == NULL)
	{
	   mpDebugPrint("do CacheWin_WithInit()");
		Idu_GetCacheWin_WithInit();
	}	
	WinCopy_Clip(pCacheWin, pNextWin, 520, 0, 280, 300, 520, 0 );
	//-----------------------------------------------------------------

    ST_SYSTEM_TIME stSystemTime;
    SystemTimeGet(&stSystemTime);
    BYTE year  = stSystemTime.u16Year; 
    BYTE month = stSystemTime.u08Month; 
    BYTE day   = stSystemTime.u08Day;
    if(day != dayBackup)
    {
        xpgCb_CalendarNow(); //xpgCb_Calendar_Update();
        dayBackup = day;
    }
    
    BYTE hour   = stSystemTime.u08Hour; 
    BYTE minute = stSystemTime.u08Minute; 
    BYTE second = stSystemTime.u08Second;
    //mpDebugPrint("FujiClock: %d : %d : %d", hour, minute, second);
                
    STXPGROLE *pRole; // runtime set pRole
    BYTE bRoleIndex; 
    BYTE degree, quadrant;
    
    WORD iOriginX = 530; // 656
	WORD iOriginY = 20; // 153
    WORD x, y;
    x = iOriginX; // - PinOffsetArry[quadrant].x;
	y = iOriginY; // - PinOffsetArry[quadrant].y;
	STXPGROLE RoleRotate;

    degree = stSystemTime.u08Hour;
    if(degree >= 12)
        degree -= 12;
	degree = degree % 12; 
	quadrant = (degree / 3); // indexHour  = ((hour % 3)*5)+(minute/12);;
	bRoleIndex = ((hour % 3)*5)+(minute/12);
	//mpDebugPrint("Hour: quadrant = %d, bRoleIndex = %d", quadrant, bRoleIndex);
	pRole = g_pstXpgMovie->m_astSprite[14+bRoleIndex].m_pstRole;
	if(quadrant != 0)
    {
        xpgRoleInit(&RoleRotate);
        xpgRolePreload(pRole); 
		MakeRotateRole(pRole, &RoleRotate, quadrant *90);
		xpgRoleDrawTransperant(&RoleRotate, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
		xpgRoleRelease(&RoleRotate);
    }
    else
    {
        xpgRoleDrawTransperant(pRole, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
    }

    degree = stSystemTime.u08Minute;
	degree = degree % 60; 
	quadrant = degree / 15;
	bRoleIndex = degree % 15;
	//mpDebugPrint("Minute: quadrant = %d, bRoleIndex = %d", quadrant, bRoleIndex);
	pRole = g_pstXpgMovie->m_astSprite[29+bRoleIndex].m_pstRole;
	if(quadrant != 0)
    {
        xpgRoleInit(&RoleRotate);
        xpgRolePreload(pRole); 
		MakeRotateRole(pRole, &RoleRotate, quadrant *90);
		xpgRoleDrawTransperant(&RoleRotate, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
		xpgRoleRelease(&RoleRotate);
    }
    else
    {
        xpgRoleDrawTransperant(pRole, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
    }
       
   	degree = stSystemTime.u08Second;
   	degree = degree % 60; 
   	quadrant = degree / 15;
	bRoleIndex = degree % 15;
	//mpDebugPrint("Second: quadrant = %d, bRoleIndex = %d", quadrant, bRoleIndex);
	pRole = g_pstXpgMovie->m_astSprite[44+bRoleIndex].m_pstRole;
    if(quadrant != 0)
    {
        xpgRoleInit(&RoleRotate);
        xpgRolePreload(pRole); 
		MakeRotateRole(pRole, &RoleRotate, quadrant *90);
		xpgRoleDrawTransperant(&RoleRotate, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
		xpgRoleRelease(&RoleRotate);
    }
    else
    {
        xpgRoleDrawTransperant(pRole, pNextWin->pdwStart, x, y, pNextWin->wWidth, pNextWin->wHeight);
    }
    
    // Copy NextWin clock to CurrWin    
    WinCopy_Clip(Idu_GetNextWin(), Idu_GetCurrWin(), 520, 0, 280, 300, 520, 0 );
    
    // Photo
    STXPGSPRITE stSprite = g_pstXpgMovie->m_astSprite[59];
    STXPGSPRITE *pSprite = &stSprite;
    //mpDebugPrint("px = %d, py = %d, pw = %d, ph = %d", pSprite->m_wPx, pSprite->m_wPy, pSprite->m_wWidth, pSprite->m_wHeight);
    WORD px = pSprite->m_wPx; //64;
	WORD py = pSprite->m_wPy; //144;
	WORD pw = pSprite->m_wWidth; //360;
	WORD ph = pSprite->m_wHeight; //400;
    LoadPagePhotoFlip(px, py, pw, ph);
    
    //WinCopy_Clip(Idu_GetNextWin(), Idu_GetCurrWin(), px, py, pw, ph, px, py ); // Photo
}
*/

#endif


