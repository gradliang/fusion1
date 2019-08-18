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
#include "slideEffect.h"


#if SLIDE_TRANSITION_3DCUBE

void __Transform_15(ST_IMGWIN * pSrcWin, int w,int h_diff1, int h_diff2)
{
	DWORD src_lines = pSrcWin->wWidth /2;
#if 0
	register DWORD * pSrc = (DWORD *)((DWORD)pSrcWin->pdwStart | 0xa0000000);

#else
	DWORD * pSrc = pSrcWin->pdwStart;
#endif

	DWORD * pSrc_Start = pSrc;

	register int x,y,h;

	int width = (pSrcWin->wWidth -w)/2;
	int height = pSrcWin->wHeight;
    BYTE orgCpuClkSel = Clock_CpuClockSelGet();
    BYTE newCpuClkSel;

	DWORD * pTemp_Start =(DWORD *) ext_mem_malloc(height * 4);
	if (! pTemp_Start)
		return;

	register DWORD *pTemp = pTemp_Start;

	int h2;
	int y_start;
	register int h_total;

	pSrc_Start += w/2;

	int h_diff_total = 0;

	DWORD *pButtom_Start;
	DWORD *pButtom;

	pButtom_Start = pSrc_Start + (src_lines * (height-1));
	pButtom = pButtom_Start;

	for (x = 0; x < width; x++ )
	{
	    y_start = h_diff_total/ width;
		h_diff_total += h_diff1;
		h2 = height -y_start*2;

		pTemp = pTemp_Start;
		pSrc = pSrc_Start;

		for ( y = 0; y < height;y++)
		{
			*pTemp++ = *pSrc;
			pSrc += src_lines;
		}

		pSrc = pSrc_Start;
		pButtom = pButtom_Start;
		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			*pButtom =0x00008080;
			pButtom -= src_lines;
			pSrc += src_lines;
		}

		h_total = 0;

		for (y = 0; y < h2;y++)
		{
			*pSrc = *(pTemp_Start + (h_total / h2 ));
			h_total += height;
			pSrc += src_lines;
		}

		pSrc_Start++;
		pButtom_Start++;

        if (Polling_Event() > 0)
        {
            ext_mem_free(pTemp_Start);
            return;
        }
	}

    // -----------------------------------------------------
	width = w /2;
	pSrc = pSrcWin->pdwStart;

	pSrc_Start = pSrc;

	h_diff_total = h_diff2 * width;

	pButtom_Start = pSrc_Start + (src_lines * (height-1));
	pButtom = pButtom_Start;

	for (x = 0; x < width; x++)
	{
	    h_diff_total -= h_diff2;
		y_start = h_diff_total / width;
		h2 = height - y_start*2;

		pTemp = pTemp_Start;
		pSrc = pSrc_Start;

		for ( y = 0; y < height;y++)
		{
			*pTemp++ = *pSrc;
			pSrc += src_lines;
		}

		pSrc = pSrc_Start;
		pButtom = pButtom_Start;
		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			*pButtom =0x00008080;
			pButtom -= src_lines;
			pSrc += src_lines;
		}

		h_total = 0;

		for (y = 0; y < h2;y++)
		{
			*pSrc = *(pTemp_Start + (h_total / h2 ));
			h_total += height;
			pSrc += src_lines;
		}

		pSrc_Start++;
		pButtom_Start++;

        if (Polling_Event() > 0)
            break;
    }

	ext_mem_free(pTemp_Start);
}

SWORD Slide_3DCube()  // jeffery 080910, used two extra win, not three
{
	BYTE i;
    DWORD dwStartTime;
	SWORD swRet = FAIL;

	ST_IMGWIN * pNextWin = Idu_GetNextWin();
	ST_IMGWIN * pCurrWin = Idu_GetCurrWin();

	WORD wWidth = pNextWin->wWidth;
	WORD wHeight = pNextWin->wHeight;

	ST_IMGWIN * pNextBack = Win_New(wWidth,wHeight);
	ST_IMGWIN * pCurrBack = Win_New(wWidth,wHeight);

	Slide_WinCopy(Idu_GetCurrWin(), pCurrBack);
	Slide_WinCopy(Idu_GetNextWin(), pNextBack);
	Slide_WinCopy(pCurrBack, Idu_GetNextWin());

	int w;
	int step = wWidth / CUBE3D_STEP;
	int diff = wHeight / (CUBE3D_STEP<<1);

	#if (___PLATFORM___ ==0x622D8)
		diff = 4;
	#endif

	for (i = 2; i < (CUBE3D_STEP-1) ; i++)
	{
        dwStartTime = GetSysTime();
		w = i* step;

        if (Polling_Event() > 0)
            goto Cube3D_Release;

		pNextWin = Idu_GetNextWin();
		Ipu_ImageScaling(pNextBack, pNextWin, 0,0, wWidth, wHeight, 0 ,0, w, wHeight, 0);
		Ipu_ImageScaling(pCurrBack, pNextWin, 0,0, wWidth, wHeight, w ,0, wWidth, wHeight, 0);

		__Transform_15(pNextWin, w, i * diff, (CUBE3D_STEP-i) * diff);

		if (__SlideReturn(dwStartTime,CUBE3D_DELAY))
			goto Cube3D_Release;

		Idu_ChgWin(pNextWin);
	}

	pNextWin = Idu_GetNextWin();
	Slide_WinCopy(pNextBack, pNextWin);
	Idu_ChgWin(pNextWin);
	swRet = PASS;

Cube3D_Release:
	Win_Free(pNextBack);
	Win_Free(pCurrBack);
	return swRet;
}

#endif

#if SLIDE_TRANSITION_3DFLIP

#if ((___PLATFORM___ == 0x622D8) || ( ___PLATFORM___ == 0x622D))
static int step_arry[]=
{
0,
20,
40,
60,
80,
100,
120,
140,
160,
180,
200,
220,
240,
260,
280,
300,
320,
340,
350, // the value near 360 will cause noise
380,
400,
};
#endif

void __Transform_10(ST_IMGWIN * pSrcWin, int h_diff,int x1,int x2)
{
	DWORD src_lines = pSrcWin->wWidth /2;
	int width = pSrcWin->wWidth/2;
	int height = pSrcWin->wHeight;

	DWORD * pTemp_Start =(DWORD *) ext_mem_malloc(height * 4);

	if (! pTemp_Start)
		return;

	register DWORD *pTemp = pTemp_Start;

	register DWORD * pSrc = pSrcWin->pdwStart;
	DWORD * pSrc_Start = pSrc;

	register int x,y,h;

	int h2;
	int y_start;
	register int h_total;

	x1 /= 2;
	x2 /= 2;

	pSrc_Start += x1;

	int h_diff_total = x1 * h_diff;

	for (x = x1; x < x2; x++)
	{
		y_start = h_diff_total / width;
		h_diff_total += h_diff;

		h2 = height - 2 * y_start;

		pTemp = pTemp_Start;
		pSrc = pSrc_Start;

		for ( y = 0; y < height;y++)
		{
			*pTemp++ = *pSrc;
			pSrc += src_lines;
		}

		pSrc = pSrc_Start;

		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			pSrc += src_lines;
		}

		h_total = 0;

		for (y = 0; y < h2;y++)
		{
			*pSrc = *(pTemp_Start + (h_total / h2 ));
			h_total += height;
			pSrc += src_lines;
		}

		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			pSrc += src_lines;
		}

		pSrc_Start++;

        if (Polling_Event() > 0)
        {
            ext_mem_free(pTemp_Start);
            return;
        }
	}

	ext_mem_free(pTemp_Start);
}

void __Transform_12(ST_IMGWIN * pSrcWin, int h_diff,int x1,int x2)
{
	DWORD src_lines = pSrcWin->wWidth /2;
	int width = pSrcWin->wWidth/2;
	int height = pSrcWin->wHeight;

	DWORD * pTemp_Start = (DWORD *) ext_mem_malloc(height * 4);

	if (! pTemp_Start)
		return;

	register DWORD *pTemp = pTemp_Start;

    register DWORD * pSrc = pSrcWin->pdwStart;
	DWORD * pSrc_Start = pSrc;

	register int y,x,h;

	int h2;
	int y_start;
    register int h_total;

	x1 /= 2;
	x2 /= 2;

	pSrc_Start += x1;

	int h_diff_total = h_diff * (width -x1);

	for (x = x1; x < x2; x++)
	{
		y_start = h_diff_total / width;

		h_diff_total -= h_diff;

		h2 = height - 2 * y_start;

		pTemp = pTemp_Start;
		pSrc = pSrc_Start;

		for ( y = 0; y < height;y++)
		{
			*pTemp++ = *pSrc;
			pSrc += src_lines;
		}

		pSrc = pSrc_Start;

		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			pSrc += src_lines;
		}

		h_total = 0;

		for (y = 0; y < h2;y++)
		{
			*pSrc = *(pTemp_Start + (h_total / h2 ));
			h_total += height;
			pSrc += src_lines;
		}

		for (y=0;y < y_start;y++)
		{
			*pSrc = 0x00008080;
			pSrc += src_lines;
		}

		pSrc_Start++;

        if (Polling_Event() > 0)
        {
            ext_mem_free(pTemp_Start);
            return;
        }
	}

	ext_mem_free(pTemp_Start);
}


SWORD Slide_3DFlip()
{
	ImageReleaseAllBuffer(); // jeffery 081013

	BYTE i;

	SWORD swRet = FAIL;

	ST_IMGWIN * pNextWin = Idu_GetNextWin();
	ST_IMGWIN * pCurrWin = Idu_GetCurrWin();

	WORD wWidth = pNextWin->wWidth;
	WORD wHeight = pNextWin->wHeight;

	// Prepare extra 2 windows
	ST_IMGWIN * pNextBack = Win_New(wWidth,wHeight);
	ST_IMGWIN * pCurrBack = Win_New(wWidth,wHeight);

	Slide_WinCopy(Idu_GetCurrWin(), pCurrBack);
	Slide_WinCopy(Idu_GetNextWin(), pNextBack);
	Slide_WinCopy(pCurrBack, Idu_GetNextWin());

	int w;
	int step = wWidth / FLIP3D_STEP;
	int diff = wHeight / FLIP3D_STEP;

	for (i = 1; i < (FLIP3D_STEP>>1) ; i++)
	{
	#if ((___PLATFORM___ == 0x622D8) || ( ___PLATFORM___ == 0x622D))
		w = step_arry[i];
	#else
		w = i* step;
	#endif

		pNextWin = Idu_GetNextWin();

		Idu_PaintWin(pNextWin, 0x00008080);
		Ipu_ImageScaling(pCurrBack, pNextWin, 0,0, wWidth, wHeight, w ,0, wWidth-w, wHeight, 0);
		__Transform_10(pNextWin, i * diff, w, wWidth -w);

		if (__SlideReturn(0,0))
				goto Flip3D_Release;

		Idu_ChgWin(pNextWin);
	}

	for (i = (FLIP3D_STEP>>1)-1 ; i > 0 ; i--)
	{
	#if ((___PLATFORM___ == 0x622D8) || ( ___PLATFORM___ == 0x622D))
		w = step_arry[i];
	#else
	 	w = i * step;
	#endif

		pNextWin = Idu_GetNextWin();

		Idu_PaintWin(pNextWin, 0x00008080);
		Ipu_ImageScaling(pNextBack, pNextWin, 0,0, wWidth, wHeight, w ,0, wWidth-w, wHeight, 0);
		__Transform_12(pNextWin, i * diff, w, wWidth -w);

		if (__SlideReturn(0,0))
              goto Flip3D_Release;

        Idu_ChgWin(pNextWin);
	}

	pNextWin = Idu_GetNextWin();
	Slide_WinCopy(pNextBack, pNextWin);
	Idu_ChgWin(pNextWin);
	swRet = PASS;

Flip3D_Release:
	Win_Free(pNextBack);
	Win_Free(pCurrBack);
	return swRet;
}

#endif

#if SLIDE_TRANSITION_3DSWAP

typedef struct
{
	int x1;
	int y1;
	int x2;
	int y2;
} ST_AREA;

ST_AREA Area_Next[32];
ST_AREA Area_Curr[32];

inline void __Area_Set(ST_AREA * pArea, int x1, int y1, int x2, int y2)
{
	pArea->x1 = x1;
	pArea->y1 = y1;
	pArea->x2 = x2;
	pArea->y2 = y2;
}

#define MIN(a,b) (((a) > (b)) ? (b) : (a))
void RotateTable_Init()
{

	static BOOL boInited = FALSE;

	if (boInited)
		return;

	ST_IMGWIN * pWin =Idu_GetCurrWin();
	WORD wWidth = pWin->wWidth;
	WORD wHeight = pWin->wHeight;

 	int i;
	int x,y,w,h;
	int x2;
	int cx = wWidth/2;
	int r = wWidth/4;

	ST_AREA *pAreaNext = Area_Next;
	ST_AREA *pAreaCurr = Area_Curr;

	for (i = 1; i < 16; i++)
	{
		w = (wWidth * i) / 32;
		h = (wHeight* i) / 32;
		y = (wHeight - h) /2;

		x = cx + (r * SIN_S((90 * i)/16))/2048 - w /2;
		if (x < 0) x = 0;
		x = ALIGN_CUT_4(x);
		x2 = MIN(x+w,wWidth);

		__Area_Set(pAreaNext,x,y,x2, y+h);

		w = wWidth - w;
		h = wHeight- h;
		y = (wHeight - h) /2;
		x = cx - (r * SIN_S((90 * i)/16))/2048 - w /2;

		if (x < 0) x = 0;
		x = ALIGN_CUT_4(x);
		x2 = MIN(x+w,wWidth);

		__Area_Set(pAreaCurr,x,y,x2, y+h);
		pAreaNext++;
		pAreaCurr++;
	}

	for (i = 16; i < 32; i++)
	{
        w = (wWidth * (32-i)) / 32;
		h = (wHeight* (32-i)) / 32;
		y = (wHeight - h) /2;
		x = cx - (r * SIN_S((90 * i)/16))/2048 - w /2;

		if (x < 0) x = 0;
		x = ALIGN_CUT_4(x);
        x2 = MIN(x+w,wWidth);

		__Area_Set(pAreaCurr,x,y,x2, y+h);

		w = wWidth - w;
		h = wHeight- h;
		y = (wHeight - h) /2;

		x = cx + (r * SIN_S((90 * i)/16))/2048 - w /2;

		if (x < 0) x = 0;
		x = ALIGN_CUT_4(x);
        x2 = MIN(x+w,wWidth);

		__Area_Set(pAreaNext,x,y,x2, y+h);
		pAreaNext++;
		pAreaCurr++;
	}
    boInited = TRUE;
}

SWORD Slide_3DSwap()
{
	BYTE i;
    DWORD dwStartTime;

	SWORD swRet = FAIL;

	RotateTable_Init();

	ST_IMGWIN * pNextWin = Idu_GetNextWin();
	ST_IMGWIN * pCurrWin = Idu_GetCurrWin();

	WORD wWidth = pNextWin->wWidth;
	WORD wHeight = pNextWin->wHeight;

	ST_IMGWIN * pNextBack = Win_New(wWidth,wHeight);
	ST_IMGWIN * pCurrBack = Win_New(wWidth,wHeight);

	Slide_WinCopy(Idu_GetCurrWin(), pCurrBack);
	Slide_WinCopy(Idu_GetNextWin(), pNextBack);

	ST_AREA * pArea;

	for (i = 5; i < 16; i++)
	{
        dwStartTime = GetSysTime();
		pNextWin = Idu_GetNextWin();

		Idu_PaintWin(pNextWin, 0x00008080);

		pArea = &(Area_Next[i]);

		Ipu_ImageScaling(pNextBack, pNextWin, 0,0, wWidth, wHeight, pArea->x1 ,pArea->y1, pArea->x2, pArea->y2,0);

		pArea = &(Area_Curr[i]);

		Ipu_ImageScaling(pCurrBack, pNextWin, 0,0, wWidth, wHeight, pArea->x1 ,pArea->y1, pArea->x2, pArea->y2,0);

		if (__SlideReturn(dwStartTime,SWAP3D_DELAY))
            goto Swap3D_Release;

		Idu_ChgWin(pNextWin);
	}

	for (i = 16; i < 31; i++)  // jeffery 081031
	{
        dwStartTime = GetSysTime();
		pNextWin = Idu_GetNextWin();

		Idu_PaintWin(pNextWin, 0x00008080);

		pArea = &(Area_Curr[i]);

		Ipu_ImageScaling(pCurrBack, pNextWin, 0,0, wWidth, wHeight, pArea->x1 ,pArea->y1, pArea->x2, pArea->y2,0);

		pArea = &(Area_Next[i]);

		Ipu_ImageScaling(pNextBack, pNextWin, 0,0, wWidth, wHeight, pArea->x1 ,pArea->y1, pArea->x2, pArea->y2,0);

		if (__SlideReturn(dwStartTime,SWAP3D_DELAY))
            goto Swap3D_Release;

		Idu_ChgWin(pNextWin);
	}

	pNextWin = Idu_GetNextWin();
	Slide_WinCopy(pNextBack, pNextWin);

	Idu_ChgWin(pNextWin);
	swRet = PASS;

Swap3D_Release:
	Win_Free(pNextBack);
	Win_Free(pCurrBack);
	return swRet;
}

#endif



