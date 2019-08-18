/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "display.h"
#include "ui.h"
#include "slideEffect.h"
#include "ui_timer.h"
#include "setup.h"
#include "cv.h"
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

volatile BYTE bSlideCount;
volatile ST_IMGWIN *bufWin;
//WORD PrimeNum[10] = { 23209, 21701, 19937, 11213, 9941, 9689, 4423, 4253, 3217, 2281 };

/*
special effects for slide show
*/
ST_IMGWIN WinOriginal;

#define RUNNING     1
#define STOPPED     0
BYTE bInTransition = STOPPED;
static BYTE SubSlideFlag = 0;


#if (SLIDE_TRANSITION_FADE ||SLIDE_TRANSITION_MULTIEFFECT||SLIDE_TRANSITION_KEN_BURNS)

/*
 * Slide show effect -- Fade
 *
 * Do interpolution for all pixels from source window 1 and source window 2.
 *
 * from : source window
 * to : target window
 * ratio : ratio value is more large, picture is more like source window image.
 *
 */
SWORD _WinInterpolution(ST_IMGWIN *from1, ST_IMGWIN *from2, ST_IMGWIN *to, BYTE ratio)
{
    WORD width, height;
    ST_IMGWIN *win;
    register WORD pixel;
    register DWORD *old, *src, *tar;
    register DWORD dwMask, prev, curr;

    src = from2->pdwStart;
    win = from1;
    old = win->pdwStart;

    width = win->wWidth >> 1;
    height = win->wHeight;

    win = to;
    tar = win->pdwStart;

    dwMask = 0x7f7f7f7f;
    pixel = ratio - 1;

    while (pixel)
    {
        dwMask = dwMask & (dwMask >> 1);
        pixel--;
    }

    while (height)
    {
        pixel = width;

        while (pixel)
        {
            prev = *old;
            curr = prev;
            curr >>= ratio;
            curr &= dwMask;
            prev -= curr;

            curr = *src;
            curr >>= ratio;
            curr &= dwMask;

            *tar = curr + prev;

            tar++;
            src++;
            old++;
            pixel--;
        }

        height--;
    }

    return PASS;
}


SWORD WinInterpolution(BYTE ratio)
{
    DWORD dwFrameStartTime = GetSysTime();

    if ( _WinInterpolution(Idu_GetCurrWin(), &WinOriginal, Idu_GetNextWin(), ratio) == FAIL )
        return FAIL;

    do {
        if (Polling_Event())
            return FAIL;
    } while (SystemGetElapsedTime(dwFrameStartTime) < FADE_DELAY);

    Idu_ChgWin(Idu_GetNextWin());

    return PASS;
}


SWORD FadeIn(ST_IMGWIN * srcWin, BYTE * workingBuff, BYTE HighSpeed)
{
    ST_IMGWIN *win;
	int i, j = 0;

    if (!CheckDimension(srcWin))
        return;

    TransWinReset(srcWin, workingBuff);

	if(HighSpeed)
	{

		for(i=3; i>0 ; i--)
		{
			for(j=0; j<2 ; j++)
			{
				if (WinInterpolution(i) == FAIL)
					return FAIL;
			}
		}
	}
	else
	{
		BYTE Ratio[20]={4,4,4,3,3,3,2,2,2,2,1,1,1,1,1,1,1,1,1};
		for(i=0; i<20 ; i++)
		{
			if (WinInterpolution(Ratio[i]) == FAIL)
				return FAIL;
		}

	}

    Slide_WinCopy(&WinOriginal, Idu_GetNextWin());
    Idu_ChgWin(Idu_GetNextWin());

    return PASS;
}

#endif

#if SLIDE_TRANSITION_SHUTTER

// 07.13.2006 modified by Athena
// 12.30.2009 Lighter modify
SWORD SlideBlinds(ST_IMGWIN * psSWin, BYTE * workingBuff)   //Lighter 0225
{
    WORD wHeight, i, InterNum = 12, wHeight1;
    BYTE bSlideCount;
    WORD top;
    DWORD dwFrameStartTime;
    BYTE bSlideStep = BLINDS_SLIDE_STEP;
    DWORD h1 = psSWin->wHeight / InterNum;

    if (!CheckDimension(psSWin))
        return;

    TransWinReset(psSWin, workingBuff);
	psSWin = &WinOriginal;
	Slide_WinCopy(Idu_GetCurrWin(),Idu_GetNextWin());

    wHeight = (psSWin->wHeight / bSlideStep) / InterNum;

    MP_DEBUG("[SlideBlinds]%d x %d", psSWin->wWidth, psSWin->wHeight);
    MP_DEBUG("wHeight : %d", wHeight);

    top = 0;

    if (wHeight == 0) wHeight = 2;
	wHeight1 = wHeight;

    for (bSlideCount = 0; bSlideCount <= bSlideStep; bSlideCount++)
    {
        dwFrameStartTime = GetSysTime();
        top = wHeight1 * bSlideCount;

		if(bSlideCount != 0)
		{
			top -= wHeight1;
		}

        for (i = 0; (i < InterNum) && (top < psSWin->wWidth); i++)
        {
			__Block_Copy(psSWin,Idu_GetNextWin(),0,top,psSWin->wWidth,wHeight,0,top);
            top += h1;

            if (Polling_Event())
                return FAIL;
        }

		if(bSlideCount == 0)
		{
			wHeight = wHeight << 1;
		}

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwFrameStartTime) < SHUTTER_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(Idu_GetNextWin());
    }

	Slide_WinCopy(psSWin,Idu_GetNextWin());
	Idu_ChgWin(Idu_GetNextWin());
    return PASS;
}


// 07.13.2006 Athena - fix slideshow audio error
// 08.03.2006 Abel fix
// 12.30.2009 Lighter modify
SWORD SlideShutter(ST_IMGWIN * psSWin, BYTE * workingBuff)	//Mason 11/20
{
    WORD wWidth, i, InterNum = 10, wWidth1;
    BYTE bSlideCount;
    WORD left;
    DWORD dwFrameStartTime;
    BYTE bSlideStep = SHUTTER_SLIDE_STEP;
    DWORD w1 = psSWin->wWidth / InterNum;

    if (!CheckDimension(psSWin))
        return;

    TransWinReset(psSWin, workingBuff);
	psSWin = &WinOriginal;
	Slide_WinCopy(Idu_GetCurrWin(),Idu_GetNextWin());

    wWidth = (psSWin->wWidth / bSlideStep) / InterNum;

    MP_DEBUG("[slideshutter]%d x %d", psSWin->wWidth, psSWin->wHeight);
    MP_DEBUG("wWidth : %d", wWidth);

    left = 0;

	w1 = ALIGN_2(w1);
	wWidth = ALIGN_2(wWidth);
    if (wWidth == 0) wWidth = 2;
	wWidth1 = wWidth;

    for (bSlideCount = 0; bSlideCount <= bSlideStep; bSlideCount++)
    {
        dwFrameStartTime = GetSysTime();
        left = wWidth1 * bSlideCount;

		if(bSlideCount != 0)
		{
			left -= wWidth1;
		}

        for (i = 0; (i < InterNum) && (left < psSWin->wWidth); i++)
        {
			__Block_Copy(psSWin,Idu_GetNextWin(),left,0,wWidth,psSWin->wHeight,left,0);
            left += w1;

            if (Polling_Event())
                return FAIL;
        }

		if(bSlideCount == 0)
		{
			wWidth = wWidth << 1;
		}

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwFrameStartTime) < SHUTTER_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(Idu_GetNextWin());
    }

	Slide_WinCopy(psSWin,Idu_GetNextWin());
	Idu_ChgWin(Idu_GetNextWin());
    return PASS;
}

#endif

#if SLIDE_TRANSITION_CROSS

// 07.13.2006 Athena modified for TV cell size
// 08.03.2006 Athena modified for bug
// 12.30.2009 Lighter modify
SWORD SlideCrossComb(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin)	//Mason 11/20
{
    WORD wWidth, wHeight, i;
    BYTE bSlideCount;
    WORD x, y;
    DWORD dwFrameStartTime;

    wWidth = CROSSCOMBO_WIDTH;
    wHeight = psSWin->wHeight / CROSSCOMBO_HEIGHT;

    for (bSlideCount = 0, x = 0; x < psSWin->wWidth; x += wWidth)
    {
        if (x + wWidth > psSWin->wWidth) wWidth = psSWin->wWidth - x;

        dwFrameStartTime = GetSysTime();

        for (i = 0, y = 0; y < psSWin->wHeight; i++, y = wHeight * i)
        {
            if ((i & 1) == 0) // even line, from right to left
            {
				__Block_Copy(psSWin,psTWin,psSWin->wWidth - x,y,wWidth,wHeight,psSWin->wWidth - x,y);
            }
            else
            {
				__Block_Copy(psSWin,psTWin,x,y,wWidth,wHeight,x,y);
            }
        }

        // delay for a while and polling event
        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwFrameStartTime) < CROSSCOMBO_DELAY * SLIDEEFFECTTIME);  //Lighter 3/9
    }

    return PASS;
}

#endif

#if SLIDE_TRANSITION_MASK

/*
 * -- MaskFromTo --
 *
 * SlideShow Mask Effect
 *
 * from		: source picture
 * to		: target picture
 * CellSize	: Size of each masked block.  CellSize = Width / 2
 *
 */
SWORD MaskFromTo(ST_IMGWIN *from, ST_IMGWIN *to, WORD CellSize)
{
    WORD width, height, i, j, k;
    DWORD *src, *tar;
    DWORD dwPixel;

    if (CellSize < 2)
    {
        mpDebugPrint("CellSize is less than 2, don't need to mask!");
        return FAIL;
    }

    width  = to->wWidth >> 1;
    height = to->wHeight;

    //Mask source and target windows
    src = from->pdwStart;
    tar = to->pdwStart;

    //src = (DWORD*)((DWORD)src | 0xA0000000); //Lighter 1/18
    //tar = (DWORD*)((DWORD)tar | 0xA0000000);

    for (i = 0; i < height; i++)
    {
        // Make serial 'wCwllSize' row use same source color ,
        // so picture look like composition of blocks.
        src = from->pdwStart + (i - (i % CellSize)) * width;

        // Extract middle pixel from  serial  wCellSize  pixels as source color
        src += CellSize >> 2;

        for (j = 0, k = 0; j < width; j++, k++)
        {
            if (k == CellSize) k = 0;
            if (k == 0) dwPixel = *src; //Change to next block color

            *tar = dwPixel;
            src++;
            tar++;
        }

        if (Polling_Event())
            return FAIL;
    }

    Idu_ChgWin(to);

    return PASS;
}



/*
 * -- FadingMask --
 *
 * Fade two source picture and then mask them.
 *
 * from : source picture 1
 * from : source picture 2
 * to   : target picture
 * CellSize : Size of each masked block.  CellSize = Width / 2
 * ratio : If this value is more large, picture looks like source picture 1.
 *         Else picture looks like source picture 2.
 *
 */
SWORD FadingMask(ST_IMGWIN *from1, ST_IMGWIN *from2, ST_IMGWIN *to, WORD CellSize, BYTE ratio)
{
    WORD width, height, i, j, k;
    DWORD *src1, *src2, *tar;
    DWORD dwPixel;

    if (CellSize < 2)
    {
        mpDebugPrint("CellSize is less than 2, don't need to mask!");
        return FAIL;
    }

    width  = to->wWidth >> 1;
    height = to->wHeight;

    //Mask source and target windows
    src1 = from1->pdwStart;
    src2 = from2->pdwStart;
    tar = to->pdwStart;

    //src1 = (DWORD*)((DWORD)src1 | BIT29); //Lighter 1/18
    //src2 = (DWORD*)((DWORD)src2 | BIT29);
    //tar  = (DWORD*)((DWORD)tar  | BIT29);

    for (i = 0; i < height; i++)
    {
        // Make serial 'wCwllSize' row use same source color ,
        // so picture look like composition of blocks.
        src1 = from1->pdwStart + (i - (i % CellSize)) * width;
        src2 = from2->pdwStart + (i - (i % CellSize)) * width;

        // Extract middle pixel from  serial  wCellSize  pixels as source color
        src1 += CellSize >> 2;
        src2 += CellSize >> 2;

        for (j = 0, k = 0; j < width; j++, k++)
        {
            if (k == CellSize) k = 0;

            if (k == 0)
            {
                //Fade two pixels' color into target window's pixel
                register DWORD dwMask = 0x7f7f7f7f;
                register DWORD src_cor1, src_cor2 ,tmp_cor;
                BYTE ratio_cnt;

                tmp_cor = src_cor1 = *src1;
                src_cor2 = *src2;

                for(ratio_cnt = 0; ratio_cnt < ratio; ratio_cnt++)
                {
                    tmp_cor >>= 1;
                    tmp_cor &= dwMask;

                    src_cor2 >>= 1;
                    src_cor2 &= dwMask;
                }

                src_cor1 -=	tmp_cor;
                dwPixel = src_cor1 + src_cor2;
            }

            *tar = dwPixel;
            src1++;
            src2++;
            tar++;
        }

        if (Polling_Event())
            return FAIL;
    }

    Idu_ChgWin(to);

    return PASS;
}

/*
-- Prototype from Athena --

// 07.13.2006 Athena - Avoid audio noise
SWORD ShowMask(BYTE ratio)
{
    ST_IMGWIN *psTrgWin;
    WORD width, height, i, j, k;
    DWORD *src, *tar;
    WORD wCellSize;
    DWORD dwPixel;

    psTrgWin = Idu_GetNextWin();
    width = psTrgWin->wWidth >> 1;
    height = psTrgWin->wHeight;

    src = WinOriginal.pdwStart;
    tar = psTrgWin->pdwStart;
    wCellSize = width >> (ratio + 3);

    if (wCellSize < 2) return FAIL;

    //src = (DWORD*)((DWORD)src | 0xA0000000); //Lighter 1/18
    //tar = (DWORD*)((DWORD)tar | 0xA0000000);

    for (i = 0; i < height; i++)
    {
        // Make serial wCwllSize row use same source color ,
        // so picture look like composition of blocks.
        src = WinOriginal.pdwStart + (i - (i % wCellSize)) * width;

        // Extract middle pixel from  serial  wCellSize  pixels as source color
        src += wCellSize >> 2;
        for (j = 0, k = 0; j < width; j++, k++)
        {
            if (k == wCellSize) k = 0;
            if (k == 0) dwPixel = *src;

            *tar = dwPixel;
            src++;
            tar++;
        }

        if (Polling_Event())
            return FAIL;
    }

    Idu_ChgWin(psTrgWin);

    return PASS;
}
*/



SWORD SlideMask(ST_IMGWIN * srcWin, BYTE * workingBuff)
{
    BYTE i, j;
    DWORD dwStartTime;

    TransWinReset(srcWin, workingBuff);

    //Step1, mask original picture (masked blocks become large.)
    for (i = 2; i <= 10 ; i+=2)
    {
        dwStartTime = GetSysTime();

        if (MaskFromTo(Idu_GetCurrWin(), Idu_GetNextWin(), i) == FAIL)
            return FAIL;

        do {
            if (Polling_Event())
                return FAIL;
        }while (SystemGetElapsedTime(dwStartTime) < MASK_DELAY * SLIDEEFFECTTIME);
    }

    //Step2, fade two pictures and then mask faded picture (masked blocks become large.)
    for (i = 12, j = 5; i <= 22 ; i+=2)
    {
        dwStartTime = GetSysTime();

        if (FadingMask(Idu_GetCurrWin(), &WinOriginal, Idu_GetNextWin(), i, j) == FAIL)
            return FAIL;

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwStartTime) < MASK_DELAY * SLIDEEFFECTTIME);

        if(i%4 == 0)
            j--;
    }

    //Step3, fade two pictures and then mask faded picture (masked blocks become little.)
    for (i = 20, j = 2; i >= 12; i-=2)
    {
        dwStartTime = GetSysTime();
        if (FadingMask(Idu_GetCurrWin(), &WinOriginal, Idu_GetNextWin(), i, j) == FAIL)
            return FAIL;

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwStartTime) < MASK_DELAY * SLIDEEFFECTTIME);

        if(i%5 == 0)
            j--;
    }

    //Step4, mask next picture (masked blocks become little.)
    for (i = 10; i >= 2; i-=2)
    {
        dwStartTime = GetSysTime();

        if (MaskFromTo(&WinOriginal, Idu_GetNextWin(), i) == FAIL)
            return FAIL;

        do {
            if (Polling_Event())
                return FAIL;
        } while (SystemGetElapsedTime(dwStartTime) < MASK_DELAY * SLIDEEFFECTTIME);
    }

    Slide_WinCopy(&WinOriginal, Idu_GetNextWin());

    if (Polling_Event())
        return FAIL;

    Idu_ChgWin(Idu_GetNextWin());

    return PASS;
}


#endif


#if SLIDE_TRANSITION_DISSOLVE

// 12.30.2009 Lighter Modify
SWORD SlideDissolve(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin)	//Mason 11/20
{
    DWORD i, CurDWORD, TotalDWORD = psSWin->wWidth * psSWin->wHeight / 2;
    DWORD dwStartTime;

    for (i = 0; i < TotalDWORD; i++)
    {
        CurDWORD = (i * 0x779) % TotalDWORD;//(i * 2281) % TotalDWORD;
        *(psTWin->pdwStart + CurDWORD) = *(psSWin->pdwStart + CurDWORD);

        if ((i & 0x1FE) == 0)
        {
            dwStartTime = GetSysTime();

            do{
                if (Polling_Event())
                    return FAIL;
            } while(SystemGetElapsedTime(dwStartTime) < DISSOLVE_DELAY);
        }
    }

    return PASS;
}

#endif

#if SLIDE_TRANSITION_KEN_BURNS

SWORD KenBurnsZoomIn(WORD x, WORD y)
{
	int StepX, StepY, StepX1, StepY1;
	int DispX, DispY, DispX1, DispY1;
	DWORD i;
	DWORD dwStartTime;
	ST_IMGWIN *pNWin,*pOWin = &WinOriginal;

	x = (x<KEN_BURNS_HELF_VIEW_X)? (KEN_BURNS_HELF_VIEW_X):(x);
	y = (y<KEN_BURNS_HELF_VIEW_Y)? (KEN_BURNS_HELF_VIEW_Y):(y);

	StepX = (x-KEN_BURNS_HELF_VIEW_X)/ KEN_BURNS_STEP;
	StepY = (y-KEN_BURNS_HELF_VIEW_Y)/ KEN_BURNS_STEP;
	StepX1 = ((pOWin->wWidth>(x+KEN_BURNS_HELF_VIEW_X))? (pOWin->wWidth-(x+KEN_BURNS_HELF_VIEW_X)):0)/ KEN_BURNS_STEP;
	StepY1 = ((pOWin->wHeight>(y+KEN_BURNS_HELF_VIEW_Y))? (pOWin->wHeight-(y+KEN_BURNS_HELF_VIEW_Y)):0)/ KEN_BURNS_STEP;

	DispX = StepX;
	DispY = StepY;
	DispX1 = pOWin->wWidth - StepX1;
	DispY1 = pOWin->wHeight - StepY1;

	for(i=1 ; i<KEN_BURNS_STEP ; i++)
	{
        dwStartTime = GetSysTime();
		pNWin = Idu_GetNextWin();

		//mpDebugPrint("~~~ Display (%d,%d) (%d,%d)",DispX,DispY,DispX1,DispY1);
		Ipu_ImageScaling(pOWin,pNWin,DispX,DispY,DispX1,DispY1,0,0,pNWin->wWidth,pNWin->wHeight,0);

		DispX += StepX;
		DispY += StepY;
		DispX1 -= StepX1;
		DispY1 -= StepY1;

		do {
			if (Polling_Event())
				return FAIL;
		} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(pNWin);
	}
	pNWin = Idu_GetNextWin();
	Ipu_ImageScaling(pOWin,pNWin,(x-KEN_BURNS_HELF_VIEW_X),(y-KEN_BURNS_HELF_VIEW_Y),
		((pOWin->wWidth>(x+KEN_BURNS_HELF_VIEW_X))? (x+KEN_BURNS_HELF_VIEW_X):pOWin->wWidth),
		((pOWin->wHeight>(y+KEN_BURNS_HELF_VIEW_Y))? (y+KEN_BURNS_HELF_VIEW_Y):pOWin->wHeight),
		0,0,pNWin->wWidth,pNWin->wHeight,0);

	Idu_ChgWin(pNWin);

	return PASS;
}

SWORD KenBurnsZoomOut(WORD x, WORD y)
{
	int StepX, StepY, StepX1, StepY1;
	int DispX, DispY, DispX1, DispY1;
	DWORD i;
	DWORD dwStartTime;
	ST_IMGWIN *pNWin,*pOWin = &WinOriginal;

	x = (x<KEN_BURNS_HELF_VIEW_X)? (KEN_BURNS_HELF_VIEW_X):(x);
	y = (y<KEN_BURNS_HELF_VIEW_Y)? (KEN_BURNS_HELF_VIEW_Y):(y);

	StepX = (x-KEN_BURNS_HELF_VIEW_X)/ KEN_BURNS_STEP;
	StepY = (y-KEN_BURNS_HELF_VIEW_Y)/ KEN_BURNS_STEP;
	StepX1 = ((pOWin->wWidth>(x+KEN_BURNS_HELF_VIEW_X))? (pOWin->wWidth-(x+KEN_BURNS_HELF_VIEW_X)):0)/ KEN_BURNS_STEP;
	StepY1 = ((pOWin->wHeight>(y+KEN_BURNS_HELF_VIEW_Y))? (pOWin->wHeight-(y+KEN_BURNS_HELF_VIEW_Y)):0)/ KEN_BURNS_STEP;

	DispX = x-KEN_BURNS_HELF_VIEW_X;
	DispY = y-KEN_BURNS_HELF_VIEW_Y;
	DispX1 = (pOWin->wWidth>(x+KEN_BURNS_HELF_VIEW_X))? (x+KEN_BURNS_HELF_VIEW_X):pOWin->wWidth;
	DispY1 = (pOWin->wHeight>(y+KEN_BURNS_HELF_VIEW_Y))? (y+KEN_BURNS_HELF_VIEW_Y):pOWin->wHeight;


	for(i=1 ; i<KEN_BURNS_STEP ; i++)
	{
        dwStartTime = GetSysTime();
		pNWin = Idu_GetNextWin();

		DispX -= StepX;
		DispY -= StepY;
		DispX1 += StepX1;
		DispY1 += StepY1;

		if(DispX <= 0)
		{
			DispX = 0;
		}
		if(DispY <= 0)
		{
			DispY = 0;
		}
		if(DispX1 >= pNWin->wWidth)
		{
			DispX1 = pNWin->wWidth;
		}
		if(DispY1 >= pNWin->wHeight)
		{
			DispY1 = pNWin->wHeight;
		}

		Ipu_ImageScaling(pOWin,pNWin,DispX,DispY,DispX1,DispY1,0,0,pNWin->wWidth,pNWin->wHeight,0);

		do {
			if (Polling_Event())
				return FAIL;
		} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(pNWin);
	}

	Slide_WinCopy(pOWin,Idu_GetNextWin());
	Idu_ChgWin(pNWin);

	return PASS;
}



SWORD KenBurnsPan(WORD ox, WORD oy, WORD tx, WORD ty)
{
	int StepX, StepY;
	int DispX, DispY, DispX1, DispY1;
	DWORD i;
	DWORD dwStartTime;
	ST_IMGWIN *pNWin,*pOWin = &WinOriginal;

	ox = (ox<KEN_BURNS_HELF_VIEW_X)? (KEN_BURNS_HELF_VIEW_X):(ox);
	oy = (oy<KEN_BURNS_HELF_VIEW_Y)? (KEN_BURNS_HELF_VIEW_Y):(oy);
	tx = (tx<KEN_BURNS_HELF_VIEW_X)? (KEN_BURNS_HELF_VIEW_X):(tx);
	ty = (ty<KEN_BURNS_HELF_VIEW_Y)? (KEN_BURNS_HELF_VIEW_Y):(ty);

	StepX = (ox-tx)/ KEN_BURNS_STEP;
	StepY = (oy-ty)/ KEN_BURNS_STEP;

	DispX = ox-KEN_BURNS_HELF_VIEW_X-StepX;
	DispY = oy-KEN_BURNS_HELF_VIEW_Y-StepY;
	DispX1 = ox+KEN_BURNS_HELF_VIEW_X-StepX;
	DispY1 = oy+KEN_BURNS_HELF_VIEW_Y-StepY;

	//mpDebugPrint("~~~ step %d,%d,%d,%d",StepX,StepY,StepX1,StepY1);
	for(i=1 ; i<KEN_BURNS_STEP ; i++)
	{
        dwStartTime = GetSysTime();
		pNWin = Idu_GetNextWin();

		DispX -= StepX;
		DispY -= StepY;
		DispX1 -= StepX;
		DispY1 -= StepY;

		if(DispX <= 0)
		{
			DispX = 0;
		}
		if(DispY <= 0)
		{
			DispY = 0;
		}
		if(DispX1 >= pNWin->wWidth)
		{
			DispX1 = pNWin->wWidth;
		}
		if(DispY1 >= pNWin->wHeight)
		{
			DispY1 = pNWin->wHeight;
		}
		//mpDebugPrint("~~~ Display (%d,%d) (%d,%d)",DispX,DispY,DispX1,DispY1);
		Ipu_ImageScaling(pOWin,pNWin,DispX,DispY,DispX1,DispY1,0,0,pNWin->wWidth,pNWin->wHeight,0);

		do {
			if (Polling_Event())
				return FAIL;
		} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY * SLIDEEFFECTTIME);
		Idu_ChgWin(pNWin);
	}

	pNWin = Idu_GetNextWin();
	Ipu_ImageScaling(pOWin,pNWin,tx-KEN_BURNS_HELF_VIEW_X,ty-KEN_BURNS_HELF_VIEW_Y,
		tx+KEN_BURNS_HELF_VIEW_X,ty+KEN_BURNS_HELF_VIEW_Y,0,0,pNWin->wWidth,pNWin->wHeight,0);

	Idu_ChgWin(pNWin);

	return PASS;
}


SWORD KenBurnsEffect(ST_IMGWIN* pSrcWin, BYTE * workingBuff)
{
    CvRect Face[KEN_BURNS_TRACE_NUMBER];
    BYTE i;
    SWORD swRet = PASS;
    DWORD dwStartTime;
    WORD x,y,x1,y1;

    memset((BYTE *) &Face, 0, KEN_BURNS_TRACE_NUMBER * sizeof(CvRect));

    if(ImageDetectFace(pSrcWin, Face)== PASS)
    {
        //mpDebugPrint("~~~Face (%d,%d)(%dx%d)", Face[0].x, Face[0].y, Face[0].width, Face[0].height);
    }


    if (Polling_Event())
        return FAIL;

    swRet = FadeIn(pSrcWin, workingBuff,1);
    if(swRet != PASS)
        return FAIL;

	if(Face[0].width!= 0)
	{
		x = Face[0].x+(Face[0].width>>1);
		y = Face[0].y+(Face[0].height>>1);
	}
	else	// do not find any face
	{
		Face[0].x = pSrcWin->wWidth >> 1;
		Face[0].y = pSrcWin->wHeight>> 1;
		Face[0].width = 0;
		Face[0].height = 0;

		x = Face[0].x+(Face[0].width>>1);
		y = Face[0].y+(Face[0].height>>1);
	}

    if (Polling_Event())
        return FAIL;

	swRet = KenBurnsZoomIn(x,y);
	if(swRet != PASS)
		return FAIL;

    if (Polling_Event())
        return FAIL;

	dwStartTime = GetSysTime();
	do {
		if (Polling_Event())
			return FAIL;
	} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY_VIEW);

	for(i=0 ; i<KEN_BURNS_TRACE_NUMBER ; i++)
	{
		if(Face[i+1].width!= 0)
		{
			dwStartTime = GetSysTime();

			x = Face[i].x+(Face[i].width>>1);
			y = Face[i].y+(Face[i].height>>1);
			x1 = Face[i+1].x+(Face[i+1].width>>1);
			y1 = Face[i+1].y+(Face[i+1].height>>1);
			swRet = KenBurnsPan(x,y,x1,y1);
			if(swRet != PASS)
				return FAIL;

			do {
				if (Polling_Event())
					return FAIL;
			} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY_VIEW);
		}
		else
		{
			break;
		}
	}

	x = Face[i].x+(Face[i].width>>1);
	y = Face[i].y+(Face[i].height>>1);
	swRet = KenBurnsZoomOut(x,y);

	if(ImageRemoveRedEye(Idu_GetCurrWin())!= PASS)
	{
		return PASS;
	}

	dwStartTime = GetSysTime();
	do {
		if (Polling_Event())
			return FAIL;
	} while (SystemGetElapsedTime(dwStartTime) < KEN_BURNS_DELAY_VIEW);


	if(swRet != PASS)
		return FAIL;
	else
		return PASS;
}

#endif

#if (SLIDE_TRANSITION_SILK ||SLIDE_TRANSITION_MULTIEFFECT)

SWORD SlideSilk(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin)	//Mason 20060731
{
    DWORD dwLineArray[34];  // 1080/32=34
    WORD wCurLine, wTotalLine = psTWin->wHeight;
    DWORD i, j, dwFactor;
    DWORD dwStartTime;
    DWORD *pdwSrcPtr = psSWin->pdwStart, *pdwTrgPtr = psTWin->pdwStart;

    for (i = 0; i < 1080; i ++)
    {
        if (i < wTotalLine)
            SetBitByOffset (&dwLineArray, i, 0);
        else
            SetBitByOffset (&dwLineArray, i, 1);
    }

    for (i = 0; i < wTotalLine; i ++)
    {
        dwStartTime = GetSysTime();
        wCurLine = (*(pdwTrgPtr + (psTWin->wWidth >> 2)) + dwStartTime) % wTotalLine;

        while (GetBitByOffset(&dwLineArray, wCurLine))
            wCurLine = (wCurLine + 97) % wTotalLine;

        SetBitByOffset (&dwLineArray, wCurLine, 1);
        pdwSrcPtr = psSWin->pdwStart + ((psSWin->dwOffset * wCurLine) >> 2);
        pdwTrgPtr = psTWin->pdwStart + ((psSWin->dwOffset * wCurLine) >> 2);

        for (j = 0; j < (psTWin->wWidth >> 1); j ++)
            *(pdwTrgPtr + j) = *(pdwSrcPtr + j);

        if ((i & 0x7) == 0)
        {
            do {
                if (Polling_Event())
                    return FAIL;
            } while (SystemGetElapsedTime(dwStartTime) < SILK_DELAY * SLIDEEFFECTTIME);//Lighter 3/9
        }
    }

    return PASS;
}


#endif

#if SLIDE_TRANSITION_SKETCH
SWORD SlideSKETCH(ST_IMGWIN * psSWin, BYTE * workingBuff)
{
    SWORD swRet = PASS;
	ST_IMGWIN * pNWin = Idu_GetNextWin();
	ST_IMGWIN * pNewWin = Win_New(psSWin->wWidth,psSWin->wHeight);
	IMAGEEFFECT Value;

	Slide_WinCopy(psSWin,pNewWin);
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	mpGetIPUEffectValue(&Value);
#endif

    Value.Sketch_Enable = 1;
    Value.SketchMode = 1;
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	mpSetIPUEffectValue(Value);
#endif
	Ipu_ImageScaling(pNewWin,pNWin,0,0,pNWin->wWidth,pNWin->wHeight,0,0,pNWin->wWidth,pNWin->wHeight,0);

	Value.Sketch_Enable = 0;
	Value.SketchMode = 0;
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	mpSetIPUEffectValue(Value);
#endif

	swRet = FadeIn(pNWin, workingBuff,1);
	if(swRet != PASS)
	{
		Win_Free(pNewWin);
		return FAIL;
	}

	swRet = FadeIn(pNewWin, workingBuff,0);
	Win_Free(pNewWin);
	if(swRet != PASS)
	{
		return FAIL;
	}
	/*
	SWORD i,j,stepCb,stepCr,step;
	ST_IMGWIN * pNWin;
	ST_IMGWIN * pNewWin = Win_New(psSWin->wWidth,psSWin->wHeight);
    SWORD swRet = PASS;
	IMAGEEFFECT Value;

	Slide_WinCopy(psSWin,pNewWin);

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	mpGetIPUEffectValue(&Value);
#endif

	for(i=1 ; i<5 ; i++)
	{
		pNWin = Idu_GetNextWin();
		//SetSketchSlideShowFlag(i);

		switch(i)
		{
			case 1:
				Value.Sketch_Enable = 1;
				break;
			case 2:
				Value.Sketch_Enable = 0;
				Value.SEBIAN_Enable = 1;
				break;
			case 3:
				Value.SEBIAN_Enable = 0;
				Value.BW_Enable = 1;
				break;
			case 4:
				Value.BW_Enable = 0;
				Value.CM_Enable= 1;
				Value.CM_Cb_Mode = 0x80+SKETCH_COLORMIX_RANGE;
				Value.CM_Cr_Mode = 0x80+SKETCH_COLORMIX_RANGE;
				break;

		}
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
		mpSetIPUEffectValue(Value);
#endif
		Ipu_ImageScaling(pNewWin,pNWin,0,0,psSWin->wWidth,psSWin->wHeight,0,0,psSWin->wWidth,psSWin->wHeight,0);

		swRet = FadeIn(pNWin, workingBuff,1);
		if(swRet != PASS)
		{
			Value.CM_Enable= 0;
			Value.CM_Cb_Mode = 0;
			Value.CM_Cr_Mode = 0;
			Win_Free(pNewWin);
			return FAIL;
		}
	}

	// Color Mix

	for(j=0 ; j<8 ; j++)
	{
		switch(j)
		{
			case 0:
				stepCb = 0;
				stepCr = -SKETCH_COLORMIX_STEP;
				break;
			case 1:
				stepCb = 0;
				stepCr = SKETCH_COLORMIX_STEP;
				Value.CM_Cr_Mode = 1;
				break;
			case 2:
				stepCb = -SKETCH_COLORMIX_STEP;
				stepCr = 0;
				Value.CM_Cb_Mode = 0x80+SKETCH_COLORMIX_RANGE;
				Value.CM_Cr_Mode = SKETCH_COLORMIX_RANGE;
				break;
			case 3:
				stepCb = SKETCH_COLORMIX_STEP;
				stepCr = 0;
				Value.CM_Cb_Mode = 1;
				break;
			case 4:
				stepCb = 0;
				stepCr = -SKETCH_COLORMIX_STEP;
				Value.CM_Cb_Mode = SKETCH_COLORMIX_RANGE;
				Value.CM_Cr_Mode = SKETCH_COLORMIX_RANGE;
				break;
			case 5:
				stepCb = 0;
				stepCr = SKETCH_COLORMIX_STEP;
				Value.CM_Cr_Mode = 0x81;
				break;
			case 6:
				stepCb = -SKETCH_COLORMIX_STEP;
				stepCr = 0;
				Value.CM_Cb_Mode = SKETCH_COLORMIX_RANGE;
				Value.CM_Cr_Mode = 0x80+SKETCH_COLORMIX_RANGE;
				break;
			case 7:
				stepCb = SKETCH_COLORMIX_STEP;
				stepCr = 0;
				Value.CM_Cb_Mode = SKETCH_COLORMIX_RANGE;
				break;

		}

		for(i=SKETCH_COLORMIX_RANGE ; i>SKETCH_COLORMIX_STEP ; i-=SKETCH_COLORMIX_STEP)
		{
			if((Value.CM_Cb_Mode == 0x80)||(Value.CM_Cb_Mode == 0)||(Value.CM_Cr_Mode == 0x80)||(Value.CM_Cr_Mode == 0))
				break;

			pNWin = Idu_GetNextWin();
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
			mpSetIPUEffectValue(Value);
#endif

			Ipu_ImageScaling(pNewWin,pNWin,0,0,psSWin->wWidth,psSWin->wHeight,0,0,psSWin->wWidth,psSWin->wHeight,0);

			Idu_ChgWin(pNWin);

			Value.CM_Cb_Mode += stepCb;
			Value.CM_Cr_Mode += stepCr;

			if (Polling_Event())
			{
				Value.CM_Enable= 0;
				Value.CM_Cb_Mode = 0;
				Value.CM_Cr_Mode = 0;
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
				mpSetIPUEffectValue(Value);
#endif
				Win_Free(pNewWin);
				return FAIL;
			}
		}
	}
	pNWin = Idu_GetNextWin();
	Slide_WinCopy(pNewWin,pNWin);
	Idu_ChgWin(pNWin);

	Value.CM_Enable= 0;
	Value.CM_Cb_Mode = 0;
	Value.CM_Cr_Mode = 0;
#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	mpSetIPUEffectValue(Value);
#endif
	Win_Free(pNewWin);*/
    return PASS;
}
#endif


SWORD Img_SlideShow(ST_IMGWIN * srcWin, BYTE * workingBuff, BYTE slideShowType)
{
    SWORD swRet = PASS;

    Img_SetTransition(RUNNING);
    MP_DEBUG1("slide type %d", slideShowType);

#if (SLIDE_TRANSITION_BRICK&&(SLIDE_TRANSITION_MASK||SLIDE_TRANSITION_EXPANSION))
    if (g_psSetupMenu->bSlideSeparate == SETUP_MENU_SLIDESEPARATE_ON)
    {
#if SLIDE_TRANSITION_MASK
        if (slideShowType == SETUP_MENU_TRANSITION_MASK)
#endif
#if SLIDE_TRANSITION_EXPANSION
			if(slideShowType == SETUP_MENU_TRANSITION_EXPANSION)
#endif
	            slideShowType = SETUP_MENU_TRANSITION_BRICK;
    }
#endif
    switch (slideShowType)
    {
#if SLIDE_TRANSITION_FADE
		case SETUP_MENU_TRANSITION_FADE:
			swRet = FadeIn(srcWin, workingBuff,0);
			break;
#endif

#if SLIDE_TRANSITION_SHUTTER
		case SETUP_MENU_TRANSITION_SHUTTER:
			SubSlideFlag = (SubSlideFlag + 1) & 1;
			MP_DEBUG1("SubSlideFlag %d", SubSlideFlag & 1);

			if (SubSlideFlag & 0x1)
				swRet = SlideShutter(srcWin, workingBuff);
			else
				swRet = SlideBlinds(srcWin, workingBuff);
			break;
#endif

#if SLIDE_TRANSITION_CROSS
		case SETUP_MENU_TRANSITION_CROSS:
			swRet = SlideCrossComb(srcWin, Idu_GetCurrWin());
			break;
#endif

#if SLIDE_TRANSITION_MASK
		case SETUP_MENU_TRANSITION_MASK:
			swRet = SlideMask(srcWin, workingBuff);
			break;
#endif

#if SLIDE_TRANSITION_BRICK
		case SETUP_MENU_TRANSITION_BRICK:
			SubSlideFlag = (SubSlideFlag + 1) % 5;

			if (SubSlideFlag == 0)
				swRet = SpiralBrick(srcWin, workingBuff);
			else if (SubSlideFlag == 1)
				swRet = UpperLeftBrick(srcWin, workingBuff);
			else if (SubSlideFlag == 2)
				swRet = LowerRightBrick(srcWin, workingBuff);
			else if (SubSlideFlag == 3)
				swRet = TopDownBrick(srcWin, workingBuff);
			else
				swRet = RandomBrick(srcWin, workingBuff);
			break;
#endif

#if SLIDE_TRANSITION_DISSOLVE
		case SETUP_MENU_TRANSITION_DISSOLVE:
			swRet = SlideDissolve(srcWin, Idu_GetCurrWin());
			break;
#endif

#if SLIDE_TRANSITION_BAR
		case SETUP_MENU_TRANSITION_BAR:
			SubSlideFlag = (SubSlideFlag + 1) % 9;

			if (SubSlideFlag == 8)
				swRet = SlideRandomBar(srcWin, Idu_GetCurrWin());
			else
				swRet = SlideBarTransition(srcWin, SubSlideFlag, workingBuff);
			break;
#endif

#if SLIDE_TRANSITION_EXPANSION
		case SETUP_MENU_TRANSITION_EXPANSION:
			SubSlideFlag = (SubSlideFlag + 1) % 4;

			if (SubSlideFlag == 0)
				swRet = SlideLineExpansion(srcWin, workingBuff, 0);
			else if (SubSlideFlag == 1)
				swRet = SlideLineExpansion(srcWin, workingBuff, 1);
			else if (SubSlideFlag == 2)
				swRet = SlideCentralExpansion(srcWin, workingBuff);
			else
				swRet = SlideCentralConstriction(srcWin, workingBuff);
			break;
#endif

#if SLIDE_TRANSITION_SILK
		case SETUP_MENU_TRANSITION_SILK:
			swRet = SlideSilk(srcWin, Idu_GetCurrWin());
			break;
#endif

#if SLIDE_TRANSITION_SKETCH
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))//TYChen add Sketch Slideshow
			case SETUP_MENU_TRANSITION_SKETCH:
				swRet = SlideSKETCH(srcWin, workingBuff);
				break;
#endif
#endif

#if SLIDE_TRANSITION_GRID
			case SETUP_MENU_TRANSITION_GRID:		//Frank Kao 12/09/07 added
				//swRet = GridTransition(srcWin, workingBuff);
				swRet = Effect_Grid(workingBuff);
				break;
#endif

#if SLIDE_TRANSITION_SCROLL
			case SETUP_MENU_TRANSITION_SCROLL:
				swRet = SlideScroll(srcWin, workingBuff);
				break;
#endif

#if SLIDE_TRANSITION_KEN_BURNS
	case SETUP_MENU_TRANSITION_KEN_BURNS:
			swRet = KenBurnsEffect(srcWin, workingBuff);
		break;
#endif

#if SLIDE_TRANSITION_BULLENTINBOARD

	case SETUP_MENU_TRANSITION_BULLENTIN_BOARD:
		BulletinBoard(Idu_GetNextWin(), Idu_GetCurrWin(), workingBuff);
		break;
#endif
#if SLIDE_TRANSITION_3DCUBE
	case SETUP_MENU_TRANSITION_3DCUBE:
		swRet = Slide_3DCube();
		break;
#endif
#if SLIDE_TRANSITION_3DFLIP
	case SETUP_MENU_TRANSITION_3DFLIP:
		swRet = Slide_3DFlip();
		break;
#endif
#if SLIDE_TRANSITION_3DSWAP
	case SETUP_MENU_TRANSITION_3DSWAP:
		swRet = Slide_3DSwap();
		break;
#endif
#if SLIDE_TRANSITION_PUSH
		case SETUP_MENU_TRANSITION_PUSH:
			SubSlideFlag = (SubSlideFlag + 1) % 4;
			swRet = SlidePush(srcWin, workingBuff, SubSlideFlag);
			break;
#endif
#if SLIDE_TRANSITION_MULTIEFFECT
		case SETUP_MENU_TRANSITION_MULTIEFFECT:
			swRet = SubEffectLoop(srcWin, Idu_GetCurrWin(), workingBuff);
			break;
#endif
	case SETUP_MENU_TRANSITION_RANDOM:
    default:
    	slideShowType = RandomGen()%SETUP_MENU_TRANSITION_RANDOM;
		swRet = Img_SlideShow(srcWin,workingBuff,slideShowType);
    	break;
/*#if SLIDE_TRANSITION_BRICK
        SubSlideFlag = (SubSlideFlag + 1) % 5;

        if (SubSlideFlag == 0)
            swRet = SpiralBrick(srcWin, workingBuff);
        else if (SubSlideFlag == 1)
            swRet =	UpperLeftBrick(srcWin, workingBuff);
        else if (SubSlideFlag == 2)
            swRet = LowerRightBrick(srcWin, workingBuff);
        else if (SubSlideFlag == 3)
            swRet =	TopDownBrick(srcWin, workingBuff);
        else
            swRet = RandomBrick(srcWin, workingBuff);
        break;
#endif*/
    }

    Img_SetTransition(STOPPED);

    return swRet;
}

