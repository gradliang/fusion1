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
#include "mpapi.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif


extern BYTE bInTransition;
extern ST_IMGWIN WinOriginal;

void Img_SetTransition(BYTE bFlag)
{
    bInTransition = bFlag;
}

BYTE Img_GetTransitionStatus()
{
    return bInTransition;
}

int __SIN_S[] =
{
0,
36,
71,
107,
143,
179,
214,
250,
285,
320,
356,
391,
426,
461,
495,
530,
564,
599,
633,
667,
700,
734,
767,
800,
833,
865,
898,
930,
962,
993,
1024,
1055,
1085,
1115,
1145,
1175,
1204,
1232,
1261,
1289,
1316,
1344,
1370,
1397,
1423,

1448, // 45
1473,
1498,
1522,
1546,
1569,
1592,
1614,
1636,
1657,
1678,
1698,
1718,
1737,
1756,
1774,
1791,
1808,
1825,
1841,
1856,
1871,
1904,
1899,
1912,
1925,
1936,
1948,
1959,
1969,
1978,
1987,
1996,
2003,
2010,
2017,
2023,
2028,
2033,
2037,
2040,
2043,
2045,
2047,
2048,
2048 // 90
};

int SIN_S(int Angle)
{
	Angle = (((Angle) % 360 ) + 360) % 360;
	int Quadrant = Angle / 90;
	Angle %= 90;

	switch (Quadrant)
	{
		case 0:
			return __SIN_S[Angle];
		case 1:
			return __SIN_S[90- Angle];
		case 2:
			return - (__SIN_S[Angle]);
		case 3:
			return - (__SIN_S[90- Angle]);
	}
}

int COS_S(int Angle)
{
	return SIN_S(Angle + 90);
}

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
// 12.31.2009 Lighter modify
void __Block_Copy(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin,
				  int sx, int sy, int sw, int sh, int tx, int ty)
{
	//DWORD count = GetSysTime();
	BYTE * pSrc = (BYTE *)((DWORD)pSrcWin->pdwStart | 0xa0000000);
	BYTE * pTrg = (BYTE *)((DWORD)pTrgWin->pdwStart | 0xa0000000);

	sx = sx;
	tx = ALIGN_CUT_2(tx);
	sw = sw;

	pSrc += (sy * pSrcWin->wWidth + sx) << 1;
	pTrg += (ty * pTrgWin->wWidth + tx) << 1;
	mmcp_block(pSrc,pTrg,sh,sw<<1,pSrcWin->dwOffset,pTrgWin->dwOffset);
	//mpDebugPrint("new elapsed time %d",SystemGetElapsedTime(count));
}

#else

// jeffery 06/11/2008
void __Block_Copy(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin,
				  int sx, int sy, int sw, int sh, int tx, int ty)
{
	DWORD src_lines = pSrcWin->wWidth /2;
	DWORD trg_lines = pTrgWin->wWidth /2;
	DWORD copy_width = sw * 2;

	DWORD * pSrc = (DWORD *)((DWORD)pSrcWin->pdwStart | 0xa0000000);
	DWORD * pTrg = (DWORD *)((DWORD)pTrgWin->pdwStart | 0xa0000000);


	sx = ALIGN_CUT_2(sx);
	tx = ALIGN_CUT_2(tx);
	sw = ALIGN_CUT_2(sw);

	pSrc += sy * src_lines + sx /2;
	pTrg += ty * trg_lines + tx /2;

	int h;

	for (h = 0; h < sh;h++)
	{
		//bcopy(pSrc, pTrg, copy_width);

		memcpy(pTrg,pSrc, copy_width);
		pSrc += src_lines;
		pTrg += trg_lines;
	//	TaskYield();
	}
}

#endif

void WinCopy_Clip(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin,
				  int sx, int sy, int sw, int sh, int tx, int ty)
{
	int src_width = pSrcWin->wWidth;
	int src_height = pSrcWin->wHeight;
	int trg_width = pTrgWin->wWidth;
	int trg_height = pTrgWin->wHeight;

#if 0
	if (sx < 0 || sy < 0)
		return;
#else
	if (sx < 0)
	{
		sw += sx;
		tx -= sx;
		sx = 0;
	}

	if (sy < 0)
	{
		sh += sy;
		ty -= sy;
		sy = 0;
	}
#endif
	if (tx < 0)
	{
		sw += tx;
		sx -= tx;
		tx = 0;
	}

	if (ty < 0)
	{
		sh += ty;
		sy -= ty;
		ty = 0;
	}

	if (sw > src_width - sx)
		sw = src_width - sx;

	if (sh > src_height - sy)
		sh = src_height - sy;

	if (sw > trg_width - tx)
		sw = trg_width - tx;

	if (sh > trg_height - ty)
		sh = trg_height - ty;

	if (sw <= 0 || sh <= 0)
		return;

//	TaskYield();

	__Block_Copy(pSrcWin, pTrgWin,sx, sy, sw, sh, tx, ty);

}

//---------------------------------------
// Added by Frank Kao - 05/05/08
//---------------------------------------
//---------------------------------------
// function name: RingCopy()
// parameters:
// return value:
// Purpose: Copy a Ring from source to target win
// descriptions:
// Notice: to save CPU time, this function doesn't perform strict boundary checking
//
//---------------------------------------
void RingCopy(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin, WORD x1, WORD y1, WORD w1, WORD h1, WORD x2, WORD y2, WORD w2, WORD h2)
{
	int x, y, w, h;

	// top
	x = x1;
	y = y1;
	w = w1;
	h = y2 -y1;

	WinCopy_Clip(pSrcWin,pTrgWin, x, y, w, h, x, y);

	// bottom
	x = x1;
	y = y2+ h2;
	w = w1;
	h = y1 + h1 - y;
	WinCopy_Clip(pSrcWin,pTrgWin, x, y, w, h, x, y);

	// left
	x = x1;
	y = y2;
	w = x2 -x1;
	h = h2;

	WinCopy_Clip(pSrcWin,pTrgWin, x, y, w, h, x, y);

	// right
	x = x2+w2;
	y = y2;
	w = x1+w1- x;
	h = h2;
	WinCopy_Clip(pSrcWin,pTrgWin, x, y, w, h, x, y);
}


// This win copy is the memory copy only
// 12.31.2009 Lighter modify
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
void Slide_WinCopy(ST_IMGWIN * sWin, ST_IMGWIN * tWin)
{
	mmcp_memcpy((BYTE *)tWin->pdwStart, (BYTE *)sWin->pdwStart, tWin->dwOffset*tWin->wHeight);
}
#else

SWORD Slide_WinCopy(ST_IMGWIN * sWin, ST_IMGWIN * tWin)
{
    register DWORD *target, *source;
    DWORD dwSrcOffset;
    DWORD dwTrgOffset;
    DWORD dwBytes;
    DWORD row, j;

    target = tWin->pdwStart;
    source = sWin->pdwStart;

    row = sWin->wHeight;
    dwSrcOffset = sWin->dwOffset >> 2;
    dwTrgOffset = tWin->dwOffset >> 2;
    dwBytes = sWin->wWidth << 1;

    while (row)
    {
        for(j = 0; j < (sWin->wWidth>>1); j++)
            *(target + j) = *(source + j);

        target += dwTrgOffset;
        source += dwSrcOffset;
        row--;

        if ((row & 0x0000007f) == 0 )
        {
            if (Polling_Event())
                return FAIL;
        }
    }

    return PASS;
}

#endif
BOOL CheckDimension(ST_IMGWIN * win)
{
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

    if (win->wWidth != pstScreen->wInnerWidth)
        return 0;

    if (win->wHeight != pstScreen->wInnerHeight)
        return 0;

    return 1;
}


void TransWinReset(ST_IMGWIN * srcWin, BYTE * buffer)
{
    ImgWinInit(&WinOriginal, buffer, srcWin->wHeight, srcWin->wWidth);
    Slide_WinCopy(srcWin, &WinOriginal);
}

//Frank Kao 12/09/07
//Don't alter the seed and parameters. This is 100,000 point generator.
DWORD RandomGen()
{
	static DWORD dwXn=1;
	DWORD dwP1=16807, dwP2=0;
	DWORD dwN = 2147483647;

	dwXn = (dwP1 * dwXn + dwP2)%dwN;

	return dwXn;

}

ST_IMGWIN * Win_New(int wWidth, int wHeight)
{
	DWORD * pBuffer = (DWORD*)ext_mem_malloc(wWidth* wHeight<<1);
	if (!pBuffer)
		return NULL;

	ST_IMGWIN *pWin = (ST_IMGWIN *) ext_mem_malloc(sizeof(ST_IMGWIN));

    if (!pWin)
    {
		ext_mem_free(pBuffer);
		return NULL;
    }
	ImgWinInit(pWin, pBuffer, wHeight, wWidth);
	return pWin;
}

void Win_Free(ST_IMGWIN * pWin)
{
	if (!pWin)
		return;

    if (pWin->pdwStart)
		ext_mem_free(pWin->pdwStart);

	ext_mem_free(pWin);
}

BOOL __SlideReturn(DWORD dwStartTime, DWORD dwDelayTime)
{
	do
	{
		if (~g_bAniFlag & ANI_SLIDE)
			return TRUE;

		if (Polling_Event())
			return TRUE;
	}while (GetElapsedMs2(dwStartTime) < dwDelayTime);

	return FALSE;
}


