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


#if SLIDE_TRANSITION_BULLENTINBOARD

typedef struct
{
	int x;
	int y;
} ST_POINT;

ST_POINT g_Point_min;
ST_POINT g_Point_max;

inline __swap(int *a,int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

inline void Idu_DrawBlend(ST_IMGWIN * pWin, int x, int y, ST_PIXEL *pColor, int alpha)
{
	#if 1
		if (x < 0 || x >= pWin->wWidth)
			return;

		if (y <0 || y >= pWin->wHeight)
			return;
	#endif

	register ST_PIXEL * pPixel = (ST_PIXEL *)(pWin->pdwStart + (y * pWin->wWidth + x) /2);
	register int inv_alpha = 256 - alpha;

	if (x & 1)
		pPixel->Y1 = (inv_alpha * pPixel->Y1 + alpha * pColor->Y1) >> 8;
	else
		pPixel->Y0 = (inv_alpha * pPixel->Y0 + alpha * pColor->Y0) >> 8;

	pPixel->Cb = ((pPixel->Cb << 8) + (inv_alpha * pPixel->Cb + alpha * pColor->Cb) ) >> 9;
	pPixel->Cr = ((pPixel->Cr << 8) + (inv_alpha * pPixel->Cr + alpha * pColor->Cr) ) >> 9;
}

void mp_getRotatePoints(ST_POINT *pTrg_Point,ST_POINT * pSrc_Point,int Point_Counts, ST_POINT *pSrc_Center, ST_POINT *pTrg_Center,int Angle,int x_Ratio, int y_Ratio)
{
	int CosValue = COS_S(-Angle);
	int SinValue = SIN_S(-Angle);

	int x_factor;
	int y_factor;

	if (x_Ratio > 0)
		x_factor = (2048 *2048 /x_Ratio);
	if (y_Ratio > 0)
		y_factor = (2048 *2048/y_Ratio);

	if (x_factor == 0) x_factor = 1;
	if (y_factor == 0) y_factor = 1;

	int src_x;
	int src_y;
	int i;

//	mpDebugPrint("x_factor= %d, y_factor=%d", x_factor, y_factor);

	for (i= 0; i < Point_Counts;i++)
	{
		src_x = pSrc_Point->x - pSrc_Center->x;
		src_y = pSrc_Point->y - pSrc_Center->y;

		pTrg_Point->x = (src_x * CosValue + src_y * SinValue) / x_factor + pTrg_Center->x;
		pTrg_Point->y = (src_y * CosValue - src_x * SinValue) / y_factor + pTrg_Center->y;

		pSrc_Point++;
		pTrg_Point++;
	}
}

void Draw_Line(ST_IMGWIN *pWin,int x0, int y0, int x1, int y1,DWORD dwColor)
{
	int dy, dx;
	int alpha = 256;

	dy = abs(y1-y0);
	dx = abs(x1-x0);

    BOOL steep = (dy > dx);

	if (steep)
	{
		__swap(&x0, &y0);
        __swap(&x1, &y1);
 	}

	if (x0 > x1)
	{
		__swap(&x0, &x1);
		__swap(&y0, &y1);
	}

    dx = x1 - x0;
    dy = abs(y1 - y0);

    //int error = -(dx + 1) / 2;
	int error = 0;
    int ystep;
	int x;
    int y = y0;

	if (y0 < y1)
		ystep = 1;
	else
		ystep = -1;

	if (steep)
	{
		for (x = x0; x <= x1; x++)
		{
			alpha = -error * 256 / (dx+1);
			Idu_DrawBlend(pWin,y-ystep, x,(ST_PIXEL *)&dwColor, alpha); //(y,x)
			Idu_DrawBlend(pWin,y, x,(ST_PIXEL *)&dwColor, 256- alpha);

        	error += dy;
        	if (error >= 0)
        	{
				y += ystep;
        		error -= dx;
        	}
		}
	}
	else
	{
		for (x = x0; x <= x1; x++)
		{
			alpha = -error * 256 / (dx+1);
			Idu_DrawBlend(pWin,x, y-ystep,(ST_PIXEL *)&dwColor, alpha); //(x,y)
			Idu_DrawBlend(pWin,x, y,(ST_PIXEL *)&dwColor, 256- alpha);

        	error += dy;
        	if (error >= 0)
        	{
				y += ystep;
		       	error -= dx;
        	}
		}
	}
}

void Draw_Rect_Rotated(ST_IMGWIN *pWin, int x0,int y0, int x1,int y1, int Angle, DWORD dwColor,int src_cx, int src_cy, int trg_cx, int trg_cy, int x_Ratio, int y_Ratio)
{
	ST_POINT sTrg_Center;
	ST_POINT sSrc_Center;
	ST_POINT sTrg_Point[4];
	ST_POINT sSrc_Point[4];

	sTrg_Center.x = trg_cx;
	sTrg_Center.y = trg_cy;

	sSrc_Center.x = src_cx;
	sSrc_Center.y = src_cy;

//	mpDebugPrint("src cx =%d , cy=%d, trg cx = %d, cy=%d", src_cx,src_cy,trg_cx,trg_cy);

	sSrc_Point[0].x = x0;
	sSrc_Point[0].y = y0;

	sSrc_Point[1].x = x0;
	sSrc_Point[1].y = y1;

	sSrc_Point[2].x = x1;
	sSrc_Point[2].y = y1;

	sSrc_Point[3].x = x1;
	sSrc_Point[3].y = y0;

	mp_getRotatePoints(sTrg_Point, sSrc_Point, 4, &sSrc_Center, &sTrg_Center, Angle, x_Ratio, y_Ratio);

	int i;

	for (i = 0; i < 4; i++)
	{
//		MP_DEBUG("i=%d x=%d y=%d",i, sTrg_Point[i].x,sTrg_Point[i].y);
	    Draw_Line(pWin,sTrg_Point[i].x, sTrg_Point[i].y, sTrg_Point[(i+1)%4].x, sTrg_Point[(i+1)%4].y,dwColor);
	}
}

void Rotate_CalculateBoundry(ST_IMGWIN *pWin, int x0,int y0, int x1,int y1, int Angle, int src_cx, int src_cy, int trg_cx, int trg_cy, int x_Ratio, int y_Ratio)
{
	ST_POINT sTrg_Center;
	ST_POINT sSrc_Center;
	ST_POINT sTrg_Point[4];
	ST_POINT sSrc_Point[4];

	sTrg_Center.x = trg_cx;
	sTrg_Center.y = trg_cy;

	sSrc_Center.x = src_cx;
	sSrc_Center.y = src_cy;

//	MP_DEBUG("src cx =%d , cy=%d, trg cx = %d, cy=%d", src_cx,src_cy,trg_cx,trg_cy);

	sSrc_Point[0].x = x0;
	sSrc_Point[0].y = y0;

	sSrc_Point[1].x = x0;
	sSrc_Point[1].y = y1;

	sSrc_Point[2].x = x1;
	sSrc_Point[2].y = y1;

	sSrc_Point[3].x = x1;
	sSrc_Point[3].y = y0;

	mp_getRotatePoints(sTrg_Point, sSrc_Point, 4, &sSrc_Center, &sTrg_Center, Angle, x_Ratio, y_Ratio);

    int i;

	g_Point_min.x = g_Point_max.x = sTrg_Point[0].x;
	g_Point_min.y = g_Point_max.y = sTrg_Point[0].y;

	for (i = 1;i < 4;i++)
	{
		if (g_Point_min.x > sTrg_Point[i].x)
			g_Point_min.x = sTrg_Point[i].x;

		if (g_Point_min.y > sTrg_Point[i].y)
			g_Point_min.y = sTrg_Point[i].y;

		if (g_Point_max.x < sTrg_Point[i].x)
			g_Point_max.x = sTrg_Point[i].x;

		if (g_Point_max.y < sTrg_Point[i].y)
			g_Point_max.y = sTrg_Point[i].y;
	}
}

void mpRotate_4(ST_IMGWIN * pTrg,ST_IMGWIN * pSrc, int Angle,float x_Ratio, float y_Ratio,int trg_cx, int trg_cy)
{
#define DEFAULT_COLOR   0x00008080

	int tx, ty;
	int sx, sy;

	int u, v;
	int srcW = pSrc->wWidth;
	int srcH = pSrc->wHeight;

    DWORD * pdwPixelTrg;
	DWORD * pdwPixelSrc;
	DWORD temp;

	DWORD Y0,Y1;
	DWORD CbCr;

	int CosValue;
	int SinValue;
	int x_factor = (x_Ratio * 2048);
	int y_factor = (y_Ratio * 2048);

	if (x_factor == 0) x_factor = 1;
	if (y_factor == 0) y_factor = 1;

	CosValue = COS_S(Angle);
	SinValue = SIN_S(Angle);

	int CosValue_2 = CosValue + CosValue;
	int SinValue_2 = SinValue + SinValue;

	pdwPixelTrg = pTrg->pdwStart;

	//int trg_cx, trg_cy;
//	int trg_cx;
//	int trg_cy;

//	trg_cx = pTrg->wWidth / 2;
//	trg_cy = pTrg->wHeight/ 2;

	int x_cos, y_cos, x_sin, y_sin;

	int tx1 = -trg_cx;
	int tx2 =pTrg->wWidth-trg_cx;

	int ty1 = -trg_cy;
	int ty2 = pTrg->wHeight-trg_cy;

	int src_cx = pSrc->wWidth / 2;
	int src_cy = pSrc->wHeight / 2;

#if 1  //jeffery 080818
	Rotate_CalculateBoundry(pSrc,0,0,pSrc->wWidth, pSrc->wHeight,Angle,src_cx, src_cy,trg_cx,trg_cy,x_factor,y_factor);


	if (tx1 < g_Point_min.x-trg_cx)
		tx1 = g_Point_min.x-trg_cx;

	if (tx2 > g_Point_max.x-trg_cx)
		tx2 = g_Point_max.x-trg_cx;

	if (ty1 < g_Point_min.y-trg_cy)
		ty1 = g_Point_min.y-trg_cy;

	if (ty2 > g_Point_max.y-trg_cy)
		ty2 = g_Point_max.y-trg_cy;

	tx1 = ALIGN_CUT_2(tx1);
	tx2 = ALIGN_2(tx2);

	int x_stride = (pTrg->wWidth -(tx2-tx1))/2;
	pdwPixelTrg += (g_Point_min.y * pSrc->wWidth + g_Point_min.x)/2;

#endif
	x_cos = tx1 * CosValue;
	x_sin = tx1 * SinValue;

	for (ty= ty1; ty< ty2; ty++)
	{

		y_cos = ty * CosValue;
		y_sin = ty * SinValue;

		u = x_cos + y_sin;
		v = y_cos - x_sin;

		for (tx= tx1;tx < tx2; tx+=2,pdwPixelTrg++, u += CosValue_2, v -= SinValue_2)
		{
			sx = u /x_factor + src_cx;
			sy = v /y_factor + src_cy;


			if (sx >= 0 && sx < srcW && sy >=0 && sy < srcH)
			{
				pdwPixelSrc = pSrc->pdwStart + (srcW * sy + sx) /2;
				temp = *pdwPixelSrc;

				Y0 = (temp << ((sx & 0x1) * 8)) & 0xff000000;
			}
			else
			{
			    //continue;
				//temp = DEFAULT_COLOR;
				temp = *pdwPixelTrg;
				Y0 = temp & 0xff000000;
			}

			CbCr = temp & 0xfefe; // lost a little precision but, a little fast,

			sx = (u + CosValue) /x_factor + src_cx;
			sy = (v - SinValue) /y_factor + src_cy;

			if (sx >= 0 && sx < srcW && sy >=0 && sy < srcH)
			{
				pdwPixelSrc = pSrc->pdwStart + (srcW * sy + sx) /2;
				temp = *pdwPixelSrc;

				Y1 = (temp >> (((~sx) & 0x1)* 8)) & 0x00ff0000;
			}
			else
			{
				//continue;
				//temp = DEFAULT_COLOR;
				temp = *pdwPixelTrg;
				Y1 = temp & 0x00ff0000;
			}

			CbCr += temp & 0xfefe;
			CbCr >>= 1;

			*pdwPixelTrg = (Y0 | Y1 | CbCr);
		}
                pdwPixelTrg += x_stride;   // jeffery 080818
		//TaskYield();
	}
}


//---------------------------------------
// Added by Frank Kao - 05/05/08
//---------------------------------------
//---------------------------------------
// function name: xpgZoomThumbToFullScreen()
// parameters:
// return value:
// Purpose: Zoom the focused thumbnail to full screen
// descriptions:
//
//---------------------------------------
void Idu_PaintNonWinArea(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor)
{
	DWORD i;

	DWORD *ptr, *src;
	DWORD dwOffset;
	DWORD wNewW;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;

	//Draw right border
	ptr = ImgWin->pdwStart;
	dwOffset = ImgWin->wWidth >> 1;

	ptr += (dwOffset * y) + ((x+w) >> 1);
	src = ptr;

	if((x+w) < ImgWin->wWidth){
		wNewW = ImgWin->wWidth-x-w;
		for (i = (wNewW >> 1); i > 0; i--){
			*ptr++ = dwColor;
		}
		ptr = src;
		ptr += dwOffset;

		for (i = h - 1; i > 0; i--)
		{
			mmcp_memcpy((BYTE *) ptr, (BYTE *) src, wNewW << 1);
			ptr += dwOffset;
		}
	}

	//Draw upper border
	ptr = ImgWin->pdwStart;
	src = ptr;
	dwOffset = ImgWin->wWidth >> 1;

	if(y > 0){
		for (i = dwOffset; i > 0; i--)
			*ptr++ = dwColor;

		for (i = y - 1; i > 0; i--){
			mmcp_memcpy((BYTE *) ptr, (BYTE *) src, dwOffset << 2);
			ptr += dwOffset;
		}
	}

	//Draw lower border
	ptr = ImgWin->pdwStart;
	ptr += dwOffset * (y+h);
	src = ptr;

	if((y+h) <= ImgWin->wHeight){
		for (i = dwOffset; i > 0; i--)
			*ptr++ = dwColor;

		for (i = ImgWin->wHeight-1; i > (y+h); i--)
		{
			mmcp_memcpy((BYTE *) ptr, (BYTE *) src, dwOffset << 2);
			ptr += dwOffset;
		}
	}

	//Draw left border
	ptr = ImgWin->pdwStart;
	ptr += (dwOffset * y);
	src = ptr;

	if(x > 0){
		for (i = (x>>1); i>0 ; i--)
			*ptr++ = dwColor;

		ptr = src+dwOffset;
		for (i = h - 1; i > 0; i--)
		{
			mmcp_memcpy((BYTE *) ptr, (BYTE *) src, x << 1);
			ptr += dwOffset;
		}
	}

}

SWORD xpgPasteWin(ST_IMGWIN* pSrcWin, ST_IMGWIN* pTrgWin, BYTE * workingBuff,
						int wSX, int wSY, int wSW, int wSH,
						int wTX, int wTY, int wTW, int wTH,
						int iDir, int iDegree, WORD wStep, WORD wDelay)
{
	int wOldTX, wOldTY, wOldTW, wOldTH;
	int wNewTX, wNewTY, wNewTW, wNewTH;
	int wXStep;
	int wXdiff, wYdiff;
	WORD wHelfW, wHelfH, wQH, wQW;
	int iRX, iRY, iRW, iRH;
	ST_IMGWIN sTmpWin;
	ST_IMGWIN *psTmpWin;
	BYTE bBorder;
	BYTE SCALE_FACTOR;
    DWORD delayTime = wDelay * wDelay;

	int i;

	MP_DEBUG("(Func) xpgShrinkTo");
	MP_DEBUG8("(%d,%d)(%dx%d) => (%d,%d)(%dx%d)", wSX, wSY, wSW, wSH, wTX, wTY, wTW, wTH);

	//Find the quarter
	wSW = ALIGN_16(wSW);
	wTW = ALIGN_4(wTW);

	wXStep = (wTW - wSW)/wStep;

	wXdiff = (wTX - wSX)/wStep;
	wYdiff = (wTY - wSY)/wStep;

	psTmpWin = &sTmpWin;

	ImgWinInit (psTmpWin, (DWORD*)workingBuff, pTrgWin->wHeight, pTrgWin->wWidth);

	bBorder = 16;

	Ipu_ImageScaling(pSrcWin, psTmpWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight,
		bBorder,bBorder, bBorder + pSrcWin->wWidth - bBorder*2,  bBorder + pSrcWin->wHeight - bBorder*2,0);

	Idu_PaintNonWinArea(psTmpWin, bBorder, bBorder, pSrcWin->wWidth-bBorder*2, pSrcWin->wHeight-bBorder*2, RGB2YUV(255,255,255));

	bBorder = 4;

	Idu_PaintNonWinArea(psTmpWin, bBorder, bBorder, pSrcWin->wWidth-bBorder*2, pSrcWin->wHeight-bBorder*2, RGB2YUV(0,0,0));

	Slide_WinCopy(psTmpWin, pSrcWin);
	Slide_WinCopy(pTrgWin, psTmpWin);


	wOldTX = wOldTY = 0;
	wOldTW = psTmpWin->wWidth;
	wOldTH = psTmpWin->wHeight;

	for(i=1; i<=wStep; i++){
		wNewTX = ALIGN_2(wSX+i*wXdiff);
		wNewTY = wSY+i*wYdiff;
		wNewTW = ALIGN_4(wSW+i*wXStep);
		wNewTH = ALIGN_2(wNewTW * wSH / wSW);

		MP_DEBUG8("(%d,%d)[%d x %d] => (%d,%d)[%d x %d]",
						wSX, wSY, wSW, wSH,	wNewTX,wNewTY, wNewTW, wNewTH);

		RingCopy(psTmpWin, pTrgWin, wOldTX, wOldTY, wOldTW, wOldTH, wNewTX, wNewTY, wNewTW, wNewTH);

		Idu_WaitBufferEnd();

		Ipu_ImageScaling(pSrcWin, pTrgWin, wSX, wSY, wSW, wSH,
			wNewTX,wNewTY, wNewTX + wNewTW,  wNewTY + wNewTH,0);


		wOldTX = wNewTX;
		wOldTY = wNewTY;
		wOldTW = wNewTW;
		wOldTH = wNewTH;

        while (delayTime--)
        {
            if (Polling_Event())
                return FAIL;
        }

		if (!(g_bAniFlag & ANI_SLIDE)){
			Img_SetTransition(0);
			MP_DEBUG("Mode change exit");
			return FAIL; //Frank Kao 070807 test
		}

	}

	float fRatioH, fRatioWH, fRatioPixel;
	WORD  wOrgX, wOrgY, wMixX, wMixY;

	iRW = wNewTW;
	iRH = wNewTH;

	//Backup background
	wHelfW = pTrgWin->wWidth/2;
	wHelfH = ALIGN_2(pTrgWin->wHeight/2 + ((pTrgWin->wHeight%2)?1:0));
	wQW = pTrgWin->wWidth/4;
	wQH = wHelfH/2;

	MP_DEBUG("wNewTX %d wNewTY %d", wNewTX, wNewTY);

	iRX = wNewTX+wNewTW/2-wQW;
	iRY = wNewTY+wNewTH/2+((wNewTH%2)?1:0)-wQH;

	MP_DEBUG("iRx %d iRY %d bHelfW %d bHelfH %d", iRX, iRY, wHelfW, wHelfH);

	if((iRX + wHelfW) > wHelfW)
		wOrgX = 0;
	else
		wOrgX = wHelfW;

	if((iRY+wHelfH) > wHelfH)
		wOrgY = 0;
	else
		wOrgY = wHelfH;


	wMixX = (wOrgX + wHelfW) % pTrgWin->wWidth;
	wMixY = (wOrgY + wHelfH) % pTrgWin->wHeight;

	MP_DEBUG("wOrgX %d wOrgY %d wMixX %d wMixY %d", wOrgX, wOrgY, wMixX, wMixY);
	//Ipu_ImageScaling(psTmpWin, psTmpWin, iRX, iRY, iRX+wHelfW, iRY+wHelfH, wOrgX, wOrgY, wOrgX+wHelfW, wOrgY+wHelfH,0);
	__Block_Copy(psTmpWin, psTmpWin,iRX, iRY,wHelfW,wHelfH,wOrgX, wOrgY);


	fRatioH = (float)(pTrgWin->wHeight) / (float)(pTrgWin->wWidth);

	fRatioWH = (float)(1 - fRatioH) / 90;

	if ((pTrgWin->wWidth == 800)&&(pTrgWin->wHeight == 600))
	{
		SCALE_FACTOR = 6;
	}
	else
	{
		SCALE_FACTOR = 5;
	}

	int Angle;
	float x_ratio;
	float y_ratio;
	int   x_factor;
	int   y_factor;
	int k;
	iDegree *= 6;
	for(i=1; i<=iDegree; i++)
	{
		Idu_WaitBufferEnd();

		//Ipu_ImageScaling(psTmpWin, psTmpWin, wOrgX, wOrgY, wOrgX+wHelfW, wOrgY+wHelfH, wMixX, wMixY, wMixX+wHelfW, wMixY+wHelfH,0);
		__Block_Copy(psTmpWin, psTmpWin,wOrgX, wOrgY,wHelfW,wHelfH,wMixX, wMixY);

		Angle = iDir * i * 1;
		fRatioPixel = 1 - fRatioWH * i * 1 * SCALE_FACTOR;
		x_ratio = (float)(wTW)/pTrgWin->wWidth * fRatioPixel;
		y_ratio = (float)(wTH)/pTrgWin->wHeight * fRatioPixel;
		x_factor = x_ratio * 2048;
		y_factor = y_ratio * 2048;

		mpRotate_4(psTmpWin, pSrcWin, Angle, x_ratio, y_ratio , wMixX+wQW, wMixY+wQH);
		if (i == iDegree)
		{
			for (k = 0;k < 15; k++)
			{
				Draw_Rect_Rotated(psTmpWin, 3+k, 3+k, pSrcWin->wWidth-3-k, pSrcWin->wHeight-3-k, Angle, 0xf0f08080,pSrcWin->wWidth/2,pSrcWin->wHeight/2, wMixX+wQW, wMixY+wQH, x_factor, y_factor);
			}

			for (k = -1;k < 3; k++)
			{
				Draw_Rect_Rotated(psTmpWin, k, k, pSrcWin->wWidth-k, pSrcWin->wHeight-k, Angle, 0x00008080,pSrcWin->wWidth/2,pSrcWin->wHeight/2, wMixX+wQW, wMixY+wQH, x_factor, y_factor);
			}
		}
		//Ipu_ImageScaling(psTmpWin, pTrgWin, wMixX, wMixY, wMixX+wHelfW, wMixY+wHelfH, iRX, iRY, iRX+wHelfW, iRY+wHelfH,0);
		__Block_Copy(psTmpWin, pTrgWin,wMixX, wMixY,wHelfW,wHelfH,iRX, iRY);
		IODelay(10);
	}


	return PASS;
}

SWORD BulletinBoard(ST_IMGWIN* pSrcWin, ST_IMGWIN* pTrgWin,  BYTE * workingBuff)
{
	static BYTE i=0;
	DWORD dwW;

	MP_DEBUG("(Func) BulletinBoard	");
	//The position must be divided by 10

	int  iDir[] ={1, -1, -1, 1, -1, 1, -1, 1, -1};
	int  iDegree[] ={2, 2, 1, 1, 2, 2, 2, 1, 1};


	dwW = ALIGN_4(pSrcWin->wWidth/2);
	MP_DEBUG("[%d](%d, %d) ",i, ALIGN_2(pSrcWin->wWidth*10/wPosX[i]),   ALIGN_2(pSrcWin->wHeight*10/wPosY[i]));


	if ((pTrgWin->wWidth == 800)&&(pTrgWin->wHeight == 600))
	{
		WORD wPosX[]={400,  16, 200, 300, 400, 10,  350, 50, 250};
		WORD wPosY[]={200,  10, 250, 30,  300, 280, 200, 150, 20};
		xpgPasteWin(Idu_GetNextWin(), Idu_GetCurrWin(), workingBuff, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight,
			wPosX[i],  wPosY[i], dwW, dwW*pSrcWin->wHeight/pSrcWin->wWidth, iDir[i], iDegree[i], 16, 300); //200
	}
	else
	{
		WORD wPosX[]={400,	16, 200, 300, 400, 10,	350, 50, 250};
		WORD wPosY[]={160,	 8, 200,  24, 240, 224, 160, 120, 16};
		xpgPasteWin(Idu_GetNextWin(), Idu_GetCurrWin(), workingBuff, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight,
			wPosX[i],  wPosY[i], dwW, dwW*pSrcWin->wHeight/pSrcWin->wWidth, iDir[i], iDegree[i], 16, 300); //200
	}

	i = ++i % 9;
}

#endif


#if SLIDE_TRANSITION_MULTIEFFECT

DWORD dwMainEffect = 0;
DWORD dwSubEffectPicNum, dwSubEffectType, dwSubEffectPreType, dwSubEffect;
DWORD dwSubEffectX, dwSubEffectY, dwSubEffectWidth, dwSubEffectHeigh;

#define MAIN_EFFECT0_NUM		8
#define SUB_EFFECT_STEP			16
#define SUB_EFFECT_ROTATE_STEP	30
#if(TCON_ID == TCON_INTERNAL_A_480x234)
#define FRAM_WIDTH				4
#else
#define FRAM_WIDTH				8
#endif
#define MAX_PIC_NUM				32


///Copy image data from one image win to another image win
///
///ST_IMGWIN *pSrcWin 	point of source win
///ST_IMGWIN *pTrgWin 	point of target win
///(x1, y1)         	The point of center after rotate, The
///WORD wDegree        	The Angle to rotate
///
/// Note : The center of rotate is the center of source win
///
void WinRotate(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin, int x, int y, WORD Degree, DWORD x_Ratio, DWORD y_Ratio)
{

	int tx, ty;
	int sx, sy;

	int u, v;
	WORD srcW, srcH;

	DWORD * pdwPixelTrg;
	DWORD * pdwPixelSrc;
	DWORD temp;

	DWORD Y0,Y1, CbCr;

	int CosValue, SinValue, CosValue_2, SinValue_2;
	int Xl, Xr, Yt, Yb;

	srcH = pSrcWin->wHeight;
	srcW = pSrcWin->wWidth;

	v = (pSrcWin->wWidth*x_Ratio + pSrcWin->wHeight*y_Ratio) >> 1;
	temp = (pSrcWin->wWidth*x_Ratio) * (pSrcWin->wWidth*x_Ratio) + (pSrcWin->wHeight*y_Ratio) * (pSrcWin->wHeight*y_Ratio);
	temp = temp >> 2;

	for(u=v; u>0; u--)
	{
		if((u*u) < temp)
			break;
	}
	u++;

	if(((u + y) < 0) || ((y - u) > pTrgWin->wHeight) || ((u + x) < 0) || ((x - u) > pTrgWin->wWidth))
		return;

	Degree = Degree % 360;

	// Get scan range
	if(u > y)									// top
		Yt = 0;
	else
		Yt = y - u;

	if((u + y) > pTrgWin->wHeight)				// bottom
		Yb = pTrgWin->wHeight;
	else
		Yb = y + u;

	if(u > x)									// left
		Xl = 0;
	else
		Xl = (x - u) & 0xfffffffe ;

	if((u + x) > pTrgWin->wWidth)				// right
		Xr = pTrgWin->wWidth;
	else
		Xr = (x + u + 1) & 0xfffffffe;

	int x_factor = (x_Ratio * 2048);
	int y_factor = (y_Ratio * 2048);
	if (x_factor == 0) x_factor = 1;
	if (y_factor == 0) y_factor = 1;

	CosValue = COS_S(Degree);
	SinValue = SIN_S(Degree);

	CosValue_2 = CosValue + CosValue;
	SinValue_2 = SinValue + SinValue;

	int cx, cy;
	cx = pSrcWin->wWidth / 2;
	cy = pSrcWin->wHeight/ 2;

	int x_cos, y_cos, x_sin, y_sin;

	for (ty= Yt; ty< Yb; ty++)
	{

		y_cos = (y - ty) * CosValue;
		y_sin = (y - ty) * SinValue;

		x_cos = (Xl - x) * CosValue;
		x_sin = (Xl - x) * SinValue;

		u = x_cos + y_sin;
		v = y_cos - x_sin;

		pdwPixelTrg = pTrgWin->pdwStart + ((pTrgWin->dwOffset/2) * ty + Xl) /2;

		for (tx= Xl; tx < Xr; tx+=2)
		{
			sx = u /x_factor + cx;
			sy = cy - v /y_factor;

			if (sx >= 0 && sx < srcW && sy >=0 && sy < srcH)
			{
				pdwPixelSrc = pSrcWin->pdwStart + ((pSrcWin->dwOffset/2) * sy + sx) /2;
				temp = *pdwPixelSrc;

				Y0 = (temp << ((sx & 0x1) * 8)) & 0xff000000;
			}
			else
			{
				//continue;
				temp = *pdwPixelTrg;
				Y0 = temp & 0xff000000;
			}

			CbCr = temp & 0xfefe; // lost a little precision but, a little fast,

			sx = (u + CosValue) /x_factor + cx;
			sy = cy - (v - SinValue) /y_factor;

			if (sx >= 0 && sx < srcW && sy >=0 && sy < srcH)
			{
				pdwPixelSrc = pSrcWin->pdwStart + ((pSrcWin->dwOffset/2) * sy + sx) /2;
				temp = *pdwPixelSrc;

				Y1 = (temp >> (((~sx) & 0x1)* 8)) & 0x00ff0000;
			}
			else
			{
				//continue;
				temp = *pdwPixelTrg;
				Y1 = temp & 0x00ff0000;
			}

			CbCr += temp & 0xfefe;
			CbCr >>= 1;

			*pdwPixelTrg = (Y0 | Y1 | CbCr);

			pdwPixelTrg++;
			u += CosValue_2;
			v -= SinValue_2;
		}
	}
}

void SubEffectGetPicNum()
{
	dwSubEffectPicNum = RandomGen() % MAX_PIC_NUM;
}


void SubEffectGetType()
{
	dwSubEffectType = RandomGen() % MAIN_EFFECT0_NUM;

	if(dwSubEffectPreType == dwSubEffectType)
	{
		dwSubEffectType++;
		dwSubEffectType = dwSubEffectType % MAIN_EFFECT0_NUM;
	}
	dwSubEffectPreType = dwSubEffectType;
}


// Must call SubEffectGetDimension() for getting dimension before calling the function
void SubEffectGetPosition(ST_IMGWIN * psTWin)
{
	DWORD tmp,tmp1;

	tmp = RandomGen();

	tmp1 = psTWin->wWidth - dwSubEffectWidth - 16;
	tmp1 = tmp % tmp1;
	dwSubEffectX = (dwSubEffectWidth/2 + tmp1 + 15) & 0xfffffff0;

	tmp1 = psTWin->wHeight - dwSubEffectHeigh;
	tmp1 = tmp % tmp1;
	dwSubEffectY = dwSubEffectHeigh/2 + tmp1;
}



void SubEffectGetDimension(ST_IMGWIN * psTWin)
{
#if(TCON_ID == TCON_INTERNAL_A_480x234)
	dwSubEffectWidth = (psTWin->wWidth>>1)&0xfffffff0;
	dwSubEffectHeigh = (psTWin->wHeight>>1)&0xfffffff0;
#else
	DWORD tmp;

	tmp = RandomGen();
	if(tmp & 0x00000001)
	{
		dwSubEffectWidth = (psTWin->wWidth>>1)&0xfffffff0;
		dwSubEffectHeigh = (psTWin->wHeight>>1)&0xfffffff0;
	}
	else
	{
		dwSubEffectWidth = (psTWin->wWidth/3)&0xfffffff0;
		dwSubEffectHeigh = (psTWin->wHeight/3)&0xfffffff0;
	}
#endif
}


// SlideSilk
SWORD SubEffect07(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
	ST_IMGWIN sBufWin, *pNWin;

	SetDataCacheInvalid();
    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);

	pNWin = Idu_GetNextWin();
	//Ipu_ImageScaling(psTWin, pNWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psTWin->wWidth, psTWin->wHeight,0);
	Slide_WinCopy(psTWin, pNWin);
	Ipu_ImageScaling(&sBufWin, pNWin, 0, 0, sBufWin.wWidth, sBufWin.wHeight, (dwSubEffectX - (dwSubEffectWidth>>1)), (dwSubEffectY - (dwSubEffectHeigh>>1)), (dwSubEffectX + (dwSubEffectWidth>>1)), (dwSubEffectY + (dwSubEffectHeigh>>1)),0);

	SlideSilk(pNWin, Idu_GetCurrWin());

	return PASS;
}


// SlideBarTransition
SWORD SubEffect06(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
	ST_IMGWIN sBufWin, *pNWin;

	SetDataCacheInvalid();
    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);

	pNWin = Idu_GetNextWin();
	//Ipu_ImageScaling(psTWin, pNWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psTWin->wWidth, psTWin->wHeight,0);
	Slide_WinCopy(psTWin, pNWin);
	Ipu_ImageScaling(&sBufWin, pNWin, 0, 0, sBufWin.wWidth, sBufWin.wHeight, (dwSubEffectX - (dwSubEffectWidth>>1)), (dwSubEffectY - (dwSubEffectHeigh>>1)), (dwSubEffectX + (dwSubEffectWidth>>1)), (dwSubEffectY + (dwSubEffectHeigh>>1)),0);

	SlideBarTransition(pNWin, SubSlideFlag, workingBuff);

	return PASS;
}


// FadeIn
SWORD SubEffect05(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
	ST_IMGWIN sBufWin, *pNWin;

	SetDataCacheInvalid();
    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);

	pNWin = Idu_GetNextWin();
	//Ipu_ImageScaling(psTWin, pNWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psTWin->wWidth, psTWin->wHeight,0);
	Slide_WinCopy(psTWin, pNWin);
	Ipu_ImageScaling(&sBufWin, pNWin, 0, 0, sBufWin.wWidth, sBufWin.wHeight, (dwSubEffectX - (dwSubEffectWidth>>1)), (dwSubEffectY - (dwSubEffectHeigh>>1)), (dwSubEffectX + (dwSubEffectWidth>>1)), (dwSubEffectY + (dwSubEffectHeigh>>1)),0);

	FadeIn(pNWin, workingBuff,0);

	return PASS;
}


// SlideBarTransition
SWORD SubEffect04(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
	ST_IMGWIN sBufWin, *pNWin;

	SetDataCacheInvalid();
    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);

	pNWin = Idu_GetNextWin();
	//Ipu_ImageScaling(psTWin, pNWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psTWin->wWidth, psTWin->wHeight,0);
	Slide_WinCopy(psTWin, pNWin);
	Ipu_ImageScaling(&sBufWin, pNWin, 0, 0, sBufWin.wWidth, sBufWin.wHeight, (dwSubEffectX - (dwSubEffectWidth>>1)), (dwSubEffectY - (dwSubEffectHeigh>>1)), (dwSubEffectX + (dwSubEffectWidth>>1)), (dwSubEffectY + (dwSubEffectHeigh>>1)),0);

	SlideBarTransition(pNWin, SubSlideFlag, workingBuff);

	return PASS;
}


// rotate from up to down
SWORD SubEffect03(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j, *TmpBuf;
    WORD wUnitHeight;
	DWORD dwStartTime;
	ST_IMGWIN sBufWin, sCWin, *pNWin;

	SetDataCacheInvalid();
	wUnitHeight = dwSubEffectY/SUB_EFFECT_ROTATE_STEP;

	TmpBuf = (BYTE *)ext_mem_malloc(dwSubEffectWidth * dwSubEffectHeigh*2 + 4096);
	if(TmpBuf == NULL)
		return PASS;

	ImgWinInit(&sCWin, (DWORD)workingBuff, psTWin->wHeight, psTWin->wWidth);
    ImgWinInit(&sBufWin, (DWORD)TmpBuf, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));

	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);
	//Ipu_ImageScaling(psTWin, &sCWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psSWin->wWidth, psSWin->wHeight,0);
	Slide_WinCopy(psTWin, &sCWin);

	for (j = 1; j <= SUB_EFFECT_ROTATE_STEP; j++)
	{
    	dwStartTime = GetSysTime();
    	pNWin = Idu_GetNextWin();
		//Ipu_ImageScaling(&sCWin, pNWin, 0, 0, psSWin->wWidth, psSWin->wHeight, 0, 0, psTWin->wWidth, psTWin->wHeight,0);
		Slide_WinCopy(&sCWin, pNWin);
		WinRotate(&sBufWin, pNWin, dwSubEffectX, j*wUnitHeight, (12*j)%360, 1, 1);
		Idu_ChgWin(pNWin);
		while (GetElapsedMs2(dwStartTime) < MULTIEFFECT_ROTATE_DELAY)
        {
            if (Polling_Event() > 0)
            {
                ext_mem_free(TmpBuf);
                return FAIL;
            }
        }
	}

	ext_mem_free(TmpBuf);
	return PASS;
}



// move from up to down
DWORD dwSubEffectX, dwSubEffectY, dwSubEffectWidth, dwSubEffectHeigh;
SWORD SubEffect02(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
    WORD wStepHeigh, wUnitHeigh;
	DWORD dwStartTime;
	ST_IMGWIN sBufWin, *pNWin, *pCWin;

	SetDataCacheInvalid();
	wUnitHeigh = (dwSubEffectY + (dwSubEffectHeigh>>1))/SUB_EFFECT_STEP;

    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);
	//Ipu_ImageScaling(psTWin, psSWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psSWin->wWidth, psSWin->wHeight,0);
	Slide_WinCopy(psTWin, psSWin);

    for (j = 1; j <= SUB_EFFECT_STEP; j++)
    {
    	dwStartTime = GetSysTime();
        wStepHeigh = wUnitHeigh * j;

		if(wStepHeigh < dwSubEffectHeigh)
		{
			//Ipu_ImageScaling(&sBufWin, psTWin, 0, (dwSubEffectHeigh - wStepHeigh), dwSubEffectWidth, dwSubEffectHeigh, (dwSubEffectX - (dwSubEffectWidth>>1)), 0, (dwSubEffectX + (dwSubEffectWidth>>1)), wStepHeigh,0);
			__Block_Copy(&sBufWin, psTWin,0, (dwSubEffectHeigh - wStepHeigh),dwSubEffectWidth,wStepHeigh, (dwSubEffectX - (dwSubEffectWidth>>1)), 0);
		}
		else
		{
			Ipu_ImageScaling(&sBufWin, psTWin, 0, 0
			                           , dwSubEffectWidth, dwSubEffectHeigh
			                           , (dwSubEffectX - (dwSubEffectWidth>>1)), (wStepHeigh - dwSubEffectHeigh)
			                           , (dwSubEffectX + (dwSubEffectWidth>>1)), wStepHeigh,0);

			Ipu_ImageScaling(psSWin, psTWin, (dwSubEffectX - (dwSubEffectWidth>>1)), 0
			                           , (dwSubEffectX + (dwSubEffectWidth>>1)), (wStepHeigh - dwSubEffectHeigh)
			                           , (dwSubEffectX - (dwSubEffectWidth>>1)), 0
			                           , (dwSubEffectX + (dwSubEffectWidth>>1)), (wStepHeigh - dwSubEffectHeigh),0);
		}

		while (GetElapsedMs2(dwStartTime) < MULTIEFFECT_PUSH_DELAY);
    }

	return PASS;
}


// move form left to right
SWORD SubEffect01(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
    WORD wStepWidth, wUnitWidth;
	DWORD dwStartTime;
	ST_IMGWIN sBufWin, *pNWin, *pCWin;

	SetDataCacheInvalid();
	wUnitWidth = (dwSubEffectX + (dwSubEffectWidth>>1))/SUB_EFFECT_STEP;
	wUnitWidth = ALIGN_2(wUnitWidth);

    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);
	//Ipu_ImageScaling(psTWin, psSWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psSWin->wWidth, psSWin->wHeight,0);
	Slide_WinCopy(psTWin, psSWin);

    for (j = 1; j <= SUB_EFFECT_STEP; j++)
    {
    	dwStartTime = GetSysTime();
        wStepWidth = wUnitWidth * j;

		if(wStepWidth < dwSubEffectWidth)
		{
			//Ipu_ImageScaling(&sBufWin, psTWin, (dwSubEffectWidth - wStepWidth), 0, dwSubEffectWidth, dwSubEffectHeigh, 0, (dwSubEffectY - (dwSubEffectHeigh>>1)), wStepWidth, dwSubEffectY + (dwSubEffectHeigh>>1),0);
			__Block_Copy(&sBufWin, psTWin,(dwSubEffectWidth - wStepWidth), 0,wStepWidth,dwSubEffectHeigh,0, (dwSubEffectY - (dwSubEffectHeigh>>1)));
		}
		else
		{
			Ipu_ImageScaling(&sBufWin, psTWin, 0, 0
			                           , dwSubEffectWidth, dwSubEffectHeigh
			                           , (wStepWidth - dwSubEffectWidth), (dwSubEffectY - (dwSubEffectHeigh>>1))
			                           , wStepWidth, dwSubEffectY + (dwSubEffectHeigh>>1),0);

			Ipu_ImageScaling(psSWin, psTWin, 0, dwSubEffectY - (dwSubEffectHeigh>>1)
			                           , (wStepWidth - dwSubEffectWidth), dwSubEffectY + (dwSubEffectHeigh>>1)
			                           , 0, dwSubEffectY - (dwSubEffectHeigh>>1)
			                           , (wStepWidth - dwSubEffectWidth), dwSubEffectY + (dwSubEffectHeigh>>1),0);
		}

		while (GetElapsedMs2(dwStartTime) < MULTIEFFECT_PUSH_DELAY);
    }

	return PASS;
}



// move from right ot left
SWORD SubEffect00(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
    BYTE j;
    WORD wStepWidth, wUnitWidth;
	DWORD dwStartTime;
	ST_IMGWIN sBufWin, *pNWin, *pCWin;

	SetDataCacheInvalid();
	wUnitWidth = (psTWin->wWidth - dwSubEffectX + (dwSubEffectWidth>>1))/SUB_EFFECT_STEP;
	wUnitWidth = ALIGN_2(wUnitWidth);

    ImgWinInit(&sBufWin, (DWORD)workingBuff, dwSubEffectHeigh, dwSubEffectWidth);
    ImgPaintArea(&sBufWin, sBufWin.wWidth, sBufWin.wHeight, RGB2YUV(255, 255, 255));
	Ipu_ImageScaling(psSWin, &sBufWin, 0, 0, psSWin->wWidth, psSWin->wHeight , FRAM_WIDTH, FRAM_WIDTH, (sBufWin.wWidth- FRAM_WIDTH), (sBufWin.wHeight - FRAM_WIDTH),0);
	//Ipu_ImageScaling(psTWin, psSWin, 0, 0, psTWin->wWidth, psTWin->wHeight, 0, 0, psSWin->wWidth, psSWin->wHeight,0);
	Slide_WinCopy(psTWin, psSWin);

    for (j = 1; j <= SUB_EFFECT_STEP; j++)
    {
    	dwStartTime = GetSysTime();
        wStepWidth = wUnitWidth * j;

		if(wStepWidth < dwSubEffectWidth)
		{
			//Ipu_ImageScaling(&sBufWin, psTWin, 0, 0, wStepWidth, dwSubEffectHeigh, (psTWin->wWidth - wStepWidth), (dwSubEffectY - (dwSubEffectHeigh>>1)), psTWin->wWidth, dwSubEffectY + (dwSubEffectHeigh>>1),0);
			__Block_Copy(&sBufWin, psTWin, 0, 0,wStepWidth, dwSubEffectHeigh,(psTWin->wWidth - wStepWidth), (dwSubEffectY - (dwSubEffectHeigh>>1)));
		}
		else
		{
			Ipu_ImageScaling(&sBufWin, psTWin, 0, 0
			                           , dwSubEffectWidth, dwSubEffectHeigh
			                           , (psTWin->wWidth - wStepWidth), (dwSubEffectY - (dwSubEffectHeigh>>1))
			                           , (psTWin->wWidth - wStepWidth + dwSubEffectWidth), dwSubEffectY + (dwSubEffectHeigh>>1),0);

			Ipu_ImageScaling(psSWin, psTWin, (psTWin->wWidth - wStepWidth + dwSubEffectWidth), dwSubEffectY - (dwSubEffectHeigh>>1)
			                           , psTWin->wWidth, dwSubEffectY + (dwSubEffectHeigh>>1)
			                           , (psTWin->wWidth - wStepWidth + dwSubEffectWidth), dwSubEffectY - (dwSubEffectHeigh>>1)
			                           , psTWin->wWidth, dwSubEffectY + (dwSubEffectHeigh>>1),0);
		}

		while (GetElapsedMs2(dwStartTime) < MULTIEFFECT_PUSH_DELAY);
    }

	return PASS;
}


SWORD SubEffectLoop(ST_IMGWIN * psSWin, ST_IMGWIN * psTWin, BYTE *workingBuff)
{
	if(dwSubEffectPicNum == 0)
		dwSubEffectPicNum = MAIN_EFFECT0_NUM;
	SubEffectGetType();
	SubEffectGetDimension(psTWin);
	SubEffectGetPosition(psTWin);

	switch (dwSubEffectType)
	{
		case 1:
			SubEffect01(psSWin, psTWin, workingBuff);
		break;
		case 2:
			SubEffect02(psSWin, psTWin, workingBuff);
		break;
		case 3:
			SubEffect03(psSWin, psTWin, workingBuff);
		break;
		case 4:
			SubEffect04(psSWin, psTWin, workingBuff);
		break;
		case 5:
			SubEffect05(psSWin, psTWin, workingBuff);
		break;
		case 6:
			SubEffect06(psSWin, psTWin, workingBuff);
		break;
		case 7:
			SubEffect07(psSWin, psTWin, workingBuff);
		break;
		case 0:
		default:
			SubEffect00(psSWin, psTWin, workingBuff);
		break;

	}

	dwSubEffect++;
	dwSubEffect = dwSubEffect%dwSubEffectPicNum;

	if(dwSubEffect == 0)
		SubEffectGetPicNum();

	return PASS;
}

#endif


