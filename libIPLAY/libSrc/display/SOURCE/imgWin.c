
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "idu.h"
#include "icon.h"

extern BYTE g_IDU_444_422_Flag;

ST_IMGWIN *psCurrWin, *psNextWin;
ST_IMGWIN sWin[2];				// used for real screen buffer
#if ANTI_TEARING_ENABLE
	ST_IMGWIN *psExtraWin1, *psExtraWin2;
	ST_IMGWIN sExtraWin[2];
#endif

#define YYCbCr_Y0(c) ((c >> 24) & 0xff)
#define YYCbCr_Y1(c) ((c >> 16) & 0xff)
#define YYCbCr_Cb(c) ((c >> 8) & 0xff)
#define YYCbCr_Cr(c) (c & 0xff)
//**********************************************************************************
// RGB2YUV
// This routine converts RGB color to YUV color.
//
// INPUT:
//  BYTE R - Red
//  BYTE G - Green
//  BYTE B - Blue
// OUTPUT:
//  NONE
//**********************************************************************************
DWORD RGB2YUV(BYTE R, BYTE G, BYTE B)
{
	DWORD Y, Cb, Cr;

	Y = ((306 * R) + (601 * G) + (117 * B)) >> 10;
	Cb = ((-172 * R) + (-340 * G) + (512 * B) + 131072) >> 10;
	Cr = ((512 * R) + (-429 * G) + (-83 * B) + 131072) >> 10;

  return (Y << 24) | (Y << 16) | (Cb << 8) | Cr;
}
#if 0
DWORD RGB2YVU(BYTE R, BYTE G, BYTE B) //yycrcb  for cbcr swap
{
	DWORD Y, Cb, Cr;

	Y = ((306 * R) + (601 * G) + (117 * B)) >> 10;
	Cb = ((-172 * R) + (-340 * G) + (512 * B) + 131072) >> 10;
	Cr = ((512 * R) + (-429 * G) + (-83 * B) + 131072) >> 10;

	return (Y << 24) | (Y << 16) | (Cr << 8) | Cb;
}
#endif
DWORD RGB2RGB888(BYTE R, BYTE G, BYTE B)
{
	BYTE Temp;
	Temp = 0;

	return (Temp << 24) | (R << 16) | (G << 8) | B;
}

DWORD RGB2RGB565(BYTE R, BYTE G, BYTE B)
{

	R >>= 3;
	G >>= 2;
	B >>= 3;

	return (R << 11) | (G << 6) | B;
}

//initial ST_IMGWIN structure, and set the position
void ImgWinInit(ST_IMGWIN * psWin, DWORD * pdwStart, WORD wHeight, WORD wWidth)
{
	psWin->pdwStart = pdwStart;
	psWin->dwOffset = wWidth * 2;
	psWin->wWidth = wWidth;
	psWin->wHeight = wHeight;
	psWin->wX = psWin->wY = 0;
	Idu_ResetWinClipRegion(psWin);
}
void InitWins(WORD wWidth, WORD wHeight)
{
	DWORD bWin0Buf = SystemGetMemAddr(DISPLAY_BUF1_MEM_ID);
	DWORD bWin1Buf = SystemGetMemAddr(DISPLAY_BUF2_MEM_ID);

	ImgWinInit(&sWin[0], (DWORD *) bWin0Buf, wHeight, wWidth);
	ImgWinInit(&sWin[1], (DWORD *) bWin1Buf, wHeight, wWidth);
	psCurrWin = &sWin[0];
	psNextWin = &sWin[1];

#if ANTI_TEARING_ENABLE
	DWORD dwWinSize = (wWidth * wHeight) << 1;
	bWin0Buf = SystemGetMemAddr(JPEG_TARGET_MEM_ID) - (dwWinSize << 1);
	bWin1Buf = SystemGetMemAddr(JPEG_TARGET_MEM_ID) - dwWinSize;
	ImgWinInit(&sExtraWin[0], (DWORD *) bWin0Buf, wHeight, wWidth);
	ImgWinInit(&sExtraWin[1], (DWORD *) bWin1Buf, wHeight, wWidth);
	psExtraWin1 = &sExtraWin[0];
	psExtraWin2 = &sExtraWin[1];
#endif
}


///
///@ingroup group_IDU
///@brief   Get the pointer of current win
///
///@param   NONE
///
///@retval  The pointer of psCurrWin
///
///@remark  The frame buffer of psCurrWin is displaying on screen now, so if you modify
///         the content of this frame buffer the display change at the same time.
///
ST_IMGWIN *Idu_GetCurrWin()
{
	if (psCurrWin == NULL) return;
	if (psCurrWin->pdwStart == NULL)
		return NULL;
	return psCurrWin;
}


///
///@ingroup group_IDU
///@brief   Get the pointer of next win
///
///@param   NONE
///
///@retval  The pointer of psNextWin
///
///
ST_IMGWIN *Idu_GetNextWin()
{
	if (psNextWin == NULL) return;
	if (psNextWin->pdwStart == NULL)
		return NULL;
	return psNextWin;
}


#if ANTI_TEARING_ENABLE
///
///@ingroup IMAGE_WIN
///@brief   Get the pointer of extra win
///
///@param   NONE
///
///@retval  The pointer of psExtraWin
///
///
ST_IMGWIN *Idu_GetExtraWin(BYTE bWinIndex)
{
	ST_IMGWIN *psExtraWin;

	if (bWinIndex == 0)
		psExtraWin = psExtraWin1;
	else if (bWinIndex == 1)
		psExtraWin = psExtraWin2;
	else
		return NULL;

	if (psExtraWin == NULL) return NULL;
	if (psExtraWin->pdwStart == NULL)
		return NULL;
	return psExtraWin;
}
#endif


///
///@ingroup IMAGE_WIN
///@brief   set image win clip region
///
///@param   register ST_IMGWIN * psWin, WORD wLeft, WORD wTop, WORD wRight, WORD wBottom
///
///@retval  NONE
///
///@remark
///
///
void Idu_SetWinClipRegion(register ST_IMGWIN * psWin, WORD wLeft, WORD wTop, WORD wRight,
						  WORD wBottom)
{
	if (psWin == NULL)
		return;
	if (wLeft & 1)
		wLeft--;
	if (wRight & 1)
		wRight++;

	psWin->wClipLeft = wLeft;
	psWin->wClipTop = wTop;
	psWin->wClipRight = (wRight > psWin->wWidth) ? psWin->wWidth : wRight;
	psWin->wClipBottom = (wBottom > psWin->wHeight) ? psWin->wHeight : wBottom;
}


///
///@ingroup IMAGE_WIN
///@brief   reset image win clip region to whole win
///
///@param   register ST_IMGWIN * psWin
///
///@retval  NONE
///
///@remark
///
///
void Idu_ResetWinClipRegion(ST_IMGWIN * psWin)
{
	if (psWin == NULL)
		return;
	psWin->wClipLeft = 0;
	psWin->wClipTop = 0;
	psWin->wClipRight = psWin->wWidth - 1;
	psWin->wClipBottom = psWin->wHeight - 1;
}

// Copy from Idu_CurrWin to Idu_NextWin or from Idu_NextWin to Idu_CurrWin with same (x, y, w, h)
void WinToWin(ST_IMGWIN * pTargetWin, WORD wX, WORD wY, WORD wWidth, WORD wHeight, ST_IMGWIN * pSourceWin)
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        WinToWin444(pTargetWin,wX,wY,wWidth,wHeight,pSourceWin);
        return;
    }
#endif
    MP_DEBUG("pTrgWin = 0x%X, %d %d %d %d, pSrcWin = 0x%X", pTargetWin, wX, wY, wWidth, wHeight, pSourceWin);
    wX = wX & 0xfffe;
    
    DWORD *pdwTargetBuffer = (DWORD *) pTargetWin;
    DWORD *pdwSourceBuffer = (DWORD *) pSourceWin;
    DWORD dwScreenWidth = pTargetWin->wWidth;
    DWORD dwScreenOffset = dwScreenWidth >> 1;
    DWORD dwLineWidth = wWidth << 1;

	pdwTargetBuffer += (wY * dwScreenWidth * 2) + (wX * 2);
	pdwSourceBuffer += (wY * dwScreenWidth * 2) + (wX * 2);
	int py;
	for (py=0; py<wHeight; py++)
	{
		memcpy(pdwTargetBuffer, pdwSourceBuffer, dwLineWidth);

        pdwTargetBuffer += dwScreenOffset; // next line
        pdwSourceBuffer += dwScreenOffset; // next line
	}
}
#if YUV444_ENABLE
// Copy from Idu_CurrWin to Idu_NextWin or from Idu_NextWin to Idu_CurrWin with same (x, y, w, h)
void WinToWin444(ST_IMGWIN * pTargetWin, WORD wX, WORD wY, WORD wWidth, WORD wHeight, ST_IMGWIN * pSourceWin)
{
    MP_DEBUG("pTrgWin = 0x%X, %d %d %d %d, pSrcWin = 0x%X", pTargetWin, wX, wY, wWidth, wHeight, pSourceWin);
    wX = wX & 0xfffe;
    
    DWORD *pdwTargetBuffer = (DWORD *) pTargetWin;
    DWORD *pdwSourceBuffer = (DWORD *) pSourceWin;
    DWORD dwScreenWidth = pTargetWin->wWidth;
    DWORD dwScreenOffset = (dwScreenWidth*3)>>2;
    DWORD dwLineWidth = (wWidth*3);

    if(wWidth%4)
    {
        mpDebugPrint("wWidth can't div 4");
    }

    if(wX%4)
    {
        mpDebugPrint("wX can't div 4");
    }
	pdwTargetBuffer += (wY * (dwScreenWidth * 3)>>2) + (wX * 3)>>2;
	pdwSourceBuffer += (wY * (dwScreenWidth * 3)>>2) + (wX * 3)>>2;
	int py;
	for (py=0; py<wHeight; py++)
	{
		memcpy(pdwTargetBuffer, pdwSourceBuffer, dwLineWidth);

        pdwTargetBuffer += dwScreenOffset; // next line
        pdwSourceBuffer += dwScreenOffset; // next line
	}
}
#endif
///Copy image data from one image win to another image win
///
///ST_IMGWIN *psWin point of target win
///(wX, wY)         The start point of target area
///WORD wWidth          The width to copy
///WORD wHeight     The height to copy
///DWORD *pdwSrc        The start address of source data stream
///DWORD dwSrcWidth The width of source win. It is used while the width to copy
///                             is not equal to the width of source win.
void WinCopy(ST_IMGWIN * psWin, WORD wY, WORD wX, WORD wWidth, WORD wHeight, DWORD * pdwSrc,
			 DWORD dwSrcWidth)
{
	register DWORD *pixel, line;
	DWORD dwCurTime = GetSysTime();
	DWORD dwSrcOffset;
	DWORD dwDstOffset;
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        WinCopy444(psWin,wY,wX,wWidth,wHeight,pdwSrc,dwSrcWidth);
        return;
    }
#endif

	// modify the horizontal vector to even pixel boundary
	wX = wX & 0xfffe;
	wWidth = wWidth + (wWidth & 1);

	if (wX > psWin->wWidth)
		return;
	if (wY > psWin->wHeight)
		return;
	if (wX + wWidth > psWin->wWidth)
		wWidth = psWin->wWidth - wX;
	if (wY + wHeight > psWin->wHeight)
		wHeight = psWin->wHeight - wY;

	dwSrcOffset = (dwSrcWidth - wWidth) / 2;
	dwDstOffset = (psWin->dwOffset / 2 - wWidth) / 2;
	pixel = psWin->pdwStart + (wY * psWin->dwOffset / 2 + wX * 2) / 2;	// devide 2 because a DWORD contain 2 pixel
	while (wHeight)
	{
		line = wWidth;
		#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		mmcp_memcpy((BYTE *)pixel,(BYTE *)pdwSrc, line<<1);
		pixel += line>>1;
		pdwSrc += line>>1;
		#else
		while (line)
		{
			*pixel = *pdwSrc;
			pixel++;
			pdwSrc++;
			line -= 2;
		}
		#endif
		pixel = pixel + dwDstOffset;
		pdwSrc += dwSrcOffset;
		wHeight--;

		if (SystemGetElapsedTime(dwCurTime) > 20) {
			TaskYield();
			dwCurTime = GetSysTime();
		}
	}
}

#if YUV444_ENABLE
void WinCopy444(ST_IMGWIN * psWin, WORD wY, WORD wX, WORD wWidth, WORD wHeight, DWORD * pdwSrc,
			 DWORD dwSrcWidth)
{
	register DWORD *pixel, line;
	DWORD dwCurTime = GetSysTime();
	DWORD dwSrcOffset;
	DWORD dwDstOffset;

	// modify the horizontal vector to even pixel boundary
	wX = wX & 0xfffe;
	wWidth = wWidth + (wWidth & 1);

	if (wX > psWin->wWidth)
		return;
	if (wY > psWin->wHeight)
		return;
	if (wX + wWidth > psWin->wWidth)
		wWidth = psWin->wWidth - wX;
	if (wY + wHeight > psWin->wHeight)
		wHeight = psWin->wHeight - wY;

	dwSrcOffset = (dwSrcWidth - wWidth)*3/4;
	dwDstOffset = ((psWin->dwOffset*3/4) - wWidth) *3/4;
	pixel = psWin->pdwStart + (wY * (psWin->dwOffset*3/4) + (wX*3/4)) * 3 / 4;	//  because 3 DWORD contain 4 pixel
	while (wHeight)
	{
		line = wWidth;
		#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
		mmcp_memcpy((BYTE *)pixel,(BYTE *)pdwSrc, (line*3));
		pixel += (line*3)>>2;
		pdwSrc += (line*3)>>2;
		#else
		while (line)
		{
			*pixel = *pdwSrc;
			pixel++;
			pdwSrc++;
            *pixel = *pdwSrc;
			pixel++;
			pdwSrc++;
            *pixel = *pdwSrc;
			pixel++;
			pdwSrc++;
			line -= 4;
		}
		#endif
		pixel = pixel + dwDstOffset;
		pdwSrc += dwSrcOffset;
		wHeight--;

		if (SystemGetElapsedTime(dwCurTime) > 20) {
			TaskYield();
			dwCurTime = GetSysTime();
		}
	}
}
#endif


//=============================================================================================//

//=============================================================================================//
void ImgPaintArea(ST_IMGWIN * ImgWin, WORD SizeX, WORD SizeY, DWORD YUVColor)
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        ImgPaintArea444(ImgWin, SizeX, SizeY, YUVColor);
        return;
    }   
#endif
	WORD StartOdd, SizeOdd, Height, Width;
	BYTE Y, Cb, Cr, *Pixel;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if ((ImgWin->wX >= ImgWin->wWidth) || (ImgWin->wY >= ImgWin->wHeight))
	{
		return;
	}

	if ((ImgWin->wX + SizeX) > ImgWin->wWidth)
	{
		SizeX = ImgWin->wWidth - ImgWin->wX;
	}

	if ((ImgWin->wY + SizeY) > ImgWin->wHeight)
	{
		SizeY = ImgWin->wHeight - ImgWin->wY;
	}

	Idu_PaintWinArea(ImgWin, ImgWin->wX, ImgWin->wY, SizeX, SizeY, YUVColor);
#if 0
	Y = (YUVColor & 0x00ff0000) >> 16;
	Cb = (YUVColor & 0x0000ff00) >> 8;
	Cr = YUVColor & 0x000000ff;

	Width = SizeX;
	Height = SizeY;

	StartOdd = (ImgWin->wX & 0x1);
	SizeOdd = (SizeX & 0x1);

	Pixel = (BYTE *) ImgWin->pdwStart + (ImgWin->wY * ImgWin->dwOffset) + (ImgWin->wX << 1);

	if (StartOdd == 1)
	{
		if (SizeX == 1)
		{
			while (Height)
			{
				while (Width)
				{
					Pixel--;
					*Pixel = Y;
					Pixel += 2;
					*Pixel = Cr;
					Width -= 1;
				}
				Pixel += (ImgWin->wWidth << 1) - 3;
				Width = SizeX;
				Height--;
			}
		}
		else if (SizeX > 1)
		{
			if (SizeOdd == 1)
			{
				while (Height)
				{
					*(--Pixel) = Y;
					Pixel += 2;
					*(Pixel++) = Cr;
					Width -= 1;
					while (Width)
					{
						*(Pixel++) = Y;
						*(Pixel++) = Y;
						*(Pixel++) = Cb;
						*(Pixel++) = Cr;
						Width -= 2;
					}
					Pixel += (ImgWin->wWidth << 1) - (SizeX << 1);
					Width = SizeX;
					Height--;
				}
			}
			else
			{
				while (Height)
				{
					*(--Pixel) = Y;
					Pixel += 2;
					*(Pixel++) = Cr;
					Width -= 1;
					while (Width > 1)
					{
						*(Pixel++) = Y;
						*(Pixel++) = Y;
						*(Pixel++) = Cb;
						*(Pixel++) = Cr;
						Width -= 2;
					}
					*Pixel = Y;
					Pixel += 2;
					*Pixel = Cb;
					Pixel += (ImgWin->wWidth << 1) - (SizeX << 1);
					Width = SizeX;
					Height--;
				}
			}
		}
		else
		{
			return;
		}
	}
	else
	{
		if (SizeX == 1)
		{
			while (Height)
			{
				while (Width)
				{
					*Pixel = Y;
					Pixel += 2;
					*Pixel = Cb;
					Width -= 1;
				}
				Pixel += (ImgWin->wWidth << 1) - 3;
				Width = SizeX;
				Height--;
			}
		}
		else if (SizeX > 1)
		{
			if (SizeOdd)
			{
				while (Height)
				{
					while (Width > 1)
					{
						*(Pixel++) = Y;
						*(Pixel++) = Y;
						*(Pixel++) = Cb;
						*(Pixel++) = Cr;
						Width -= 2;
					}
					*Pixel = Y;
					Pixel += 2;
					*Pixel = Cb;
					Pixel += (ImgWin->wWidth << 1) - (SizeX << 1);
					Width = SizeX;
					Height--;
				}
			}
			else
			{
				while (Height)
				{
					while (Width)
					{
						*(Pixel++) = Y;
						*(Pixel++) = Y;
						*(Pixel++) = Cb;
						*(Pixel++) = Cr;
						Width -= 2;
					}
					Pixel += (ImgWin->wWidth << 1) - (SizeX << 1);
					Width = SizeX;
					Height--;
				}
			}
		}
		else
		{
			return;
		}
	}
	#endif
}


#if YUV444_ENABLE
void ImgPaintArea444(ST_IMGWIN * ImgWin, WORD SizeX, WORD SizeY, DWORD YUVColor)
{
	WORD StartOdd, SizeOdd, Height, Width;
	BYTE Y, Cb, Cr, *Pixel;
    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;
	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if ((ImgWin->wX >= ImgWin->wWidth) || (ImgWin->wY >= ImgWin->wHeight))
	{
		return;
	}

	if ((ImgWin->wX + SizeX) > ImgWin->wWidth)
	{
		SizeX = ImgWin->wWidth - ImgWin->wX;
	}

	if ((ImgWin->wY + SizeY) > ImgWin->wHeight)
	{
		SizeY = ImgWin->wHeight - ImgWin->wY;
	}

	Idu_PaintWinArea444(ImgWin, ImgWin->wX, ImgWin->wY, SizeX, SizeY, YUVColor);
}
#endif


void ImgSetPoint(ST_IMGWIN * ImgWin, WORD X, WORD Y)
{
	if (ImgWin == NULL)
		return;
	if (X > ImgWin->wWidth)
	{
		X = ImgWin->wWidth;
	}

	if (Y > ImgWin->wHeight)
	{
		Y = ImgWin->wHeight;
	}
	ImgWin->wX = X;
	ImgWin->wY = Y;
}




void Idu_PaintWin(ST_IMGWIN * ImgWin, DWORD dwColor)
{
#if YUV444_ENABLE    
    if(g_IDU_444_422_Flag==1)
    {
        Idu_PaintWin444(ImgWin, dwColor);
        return;
    }    
#endif
	DWORD i;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;

	DWORD *ptr, *src;
	DWORD dwOffset;

	ptr = ImgWin->pdwStart;
	src = ptr;
	dwOffset = ImgWin->dwOffset>> 2;
	#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	mmcp_memset_u32((BYTE *)ptr,dwColor, ImgWin->wWidth<<1);
	ptr+=dwOffset;
	#else
	for (i = dwOffset; i > 0; i--)
	{
		*ptr++ = dwColor;
	}
	#endif
	for (i = ImgWin->wHeight - 1; i > 0; i--)
	{
		memcpy(ptr, src, ImgWin->wWidth<<1);
		ptr += dwOffset;
	}

}


#if YUV444_ENABLE
void Idu_PaintWin444(ST_IMGWIN * ImgWin, DWORD dwColor)
{
	DWORD i;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;

	DWORD *ptr, *src;
	DWORD dwOffset;

	ptr = ImgWin->pdwStart;
	src = ptr;
	dwOffset = ImgWin->wWidth >> 2;

    for (i = dwOffset; i > 0; i--)
	{
		*ptr++ = dwColor<<8 | dwColor>>24;
		*ptr++ = dwColor<<16 | (dwColor&0x00ffff00)>>8;
		*ptr++ = dwColor<<24| (dwColor&0x00ffffff);
	}
	for (i = ImgWin->wHeight - 1; i > 0; i--)
	{
		memcpy(ptr, src,  (dwOffset<<2)*3);
		ptr += dwOffset*3;
	}

}
#endif


// similar as mpCatchWinToBuffer(...)
void Idu_GetWinData(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD *pdwData)
{
	DWORD i,j, dwOffset;;
	DWORD *ptr, *ptr_save;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	ptr = ImgWin->pdwStart;
	ptr += (dwOffset * y) + (x >> 1);

	for (i = 0; i < h; i++)
	{
	    ptr_save = ptr;
	  	#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	  	mmcp_memcpy((BYTE *)pdwData,(BYTE *)ptr, w <<1);
		pdwData += w >> 1;
	  	#else
  		for (j = w >> 1; j > 0; j--)
  		{
  			*pdwData++ = *ptr++; //*ptr++ = *pdwData++;
  		}
		#endif
	  	ptr = ptr_save;
	  	ptr += dwOffset;
	}
}

void Idu_PaintWinData(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD *pdwData)
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        Idu_PaintWinData444(ImgWin, x, y, w, h, pdwData);
        return;
    }   
#endif
	DWORD i,j, dwOffset;;
	DWORD *ptr, *ptr_save;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	ptr = ImgWin->pdwStart;
	ptr += (dwOffset * y) + (x >> 1);

	for (i = 0; i < h; i++)
	{
	    ptr_save = ptr;
	    #if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	    mmcp_memcpy((BYTE *)ptr,(BYTE *)pdwData, w <<1);
	    pdwData += w >> 1;
	    #else
  	    for (j = w >> 1; j > 0; j--)
  	    {
  		    *ptr++ = *pdwData++;
  	    }
	    #endif
	    ptr = ptr_save;
	    ptr += dwOffset;
	}
}


#if YUV444_ENABLE
void Idu_PaintWinData444(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD *pdwData)
{
	DWORD i,j, dwOffset;;
	BYTE *ptr, *ptr_save;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth * 3;
	ptr = ImgWin->pdwStart;
	ptr += (y * dwOffset) + (x * 3);

    DWORD c;
	for (i = 0; i < h; i++)
	{
	    ptr_save = ptr;
  	    for (j = w >> 1; j > 0; j--)
  	    {
  	        c = *pdwData++;
            *ptr++ = YYCbCr_Y0(c); 
		    *ptr++ = YYCbCr_Cb(c); 
		    *ptr++ = YYCbCr_Cr(c); 
            *ptr++ = YYCbCr_Y1(c); 
		    *ptr++ = YYCbCr_Cb(c); 
		    *ptr++ = YYCbCr_Cr(c); 		    		    
  	    }
	    ptr = ptr_save;
	    ptr += dwOffset;
	}
}
#endif


///
///@ingroup group_IDU
///@brief   Paint a rectangle area in win by designated color
///
///@param   ST_IMGWIN *ImgWin 	: The image win that you want paint
///@param		WORD			X					: The area left-top x-coordinate
///@param		WORD			Y					: The area left-top y-coordinate
///@param		WORD			W					: The area width
///@param		WORD			H					: The area height
///@param		DWORD dwColor				: The color you want to paint
///
///@retval  NONE
///
///@remark  
///
void Idu_PaintWinArea(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor)
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        Idu_PaintWinArea444(ImgWin, x, y, w, h, dwColor);
        return;
    }
#endif
	DWORD i;

	DWORD *ptr, *src;
	DWORD dwOffset;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if(w <= 1 || h <= 1)
		return;
	if(w > ImgWin->wWidth || h > ImgWin->wHeight)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	ptr = ImgWin->pdwStart;
	ptr += (dwOffset * y) + (x >> 1);
	src = ptr;
	#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	mmcp_memset_u32((BYTE *)ptr,dwColor, w<<1);
	#else
	for (i = w >> 1; i > 0; i--)
	{
		*ptr++ = dwColor;
	}
	#endif
	ptr = src;
	ptr += dwOffset;
	for (i = h - 1; i > 0; i--)
	{
		memcpy(ptr, src, w << 1);
		ptr += dwOffset;
	}

}


#if YUV444_ENABLE
void Idu_PaintWinArea444(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor)
{
    //mpDebugPrint("Idu_PaintWinArea444(x=%d, y=%d, w=%d, h=%d, dwColor=0x%X) --------------->", x, y, w, h, dwColor);

	WORD i;
	BYTE *ptr, *src;
	WORD dwOffset;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if(w <= 1 || h <= 1)
		return;
	if(w > ImgWin->wWidth || h > ImgWin->wHeight)
		return;
		
	dwOffset = ImgWin->wWidth * 3;
	
	ptr = ImgWin->pdwStart;
	
	ptr += (y * dwOffset) + x * 3;
	
	src = ptr;
	DWORD c, cb, cr, cy0, cy1;
    c   = dwColor;
	cy0 = YYCbCr_Y0(c);
	cy1 = YYCbCr_Y1(c);
    cb  = YYCbCr_Cb(c);
    cr  = YYCbCr_Cr(c);
	for (i = w >> 1; i > 0; i--)
	{
		*ptr++ = (cy0 + cy1) / 2;
		*ptr++ = cb;
		*ptr++ = cr;
		*ptr++ = (cy0 + cy1) / 2;
		*ptr++ = cb;
		*ptr++ = cr;				
	}
	ptr = src;
	ptr += dwOffset;
	for (i = h - 1; i > 0; i--)
	{
		memcpy(ptr, src, w * 3);
		ptr += dwOffset;
	}
}

#endif


/*
 * color = red, green, blue, or grey = scale color bar
 * black or white = 1 single color
 */
 #if 0
void Idu_PaintBackGround(ST_IMGWIN * ImgWin, WORD wMinColor, WORD wMaxColor, BYTE bColor)
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        Idu_PaintBackGround444(ImgWin, wMinColor, wMaxColor, bColor);
        return;
    }    
#endif
	WORD i, x, paintArea, barLength;
	WORD r, g, b, rr, gg, bb;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if ((wMinColor > wMaxColor) || (wMinColor > 255) || (wMaxColor > 255))
	{
		bColor = BGCOLOR_BLACK;
	}

	x = r = g = b = rr = gg = bb = 0;
	if (bColor == BGCOLOR_BLACK)
	{
		Idu_PaintWin(ImgWin, 0x00008080);
		return;
	}
	else if (bColor == BGCOLOR_RED)
	{
		r = 1;
		rr = wMaxColor;
	}
	else if (bColor == BGCOLOR_GREEN)
	{
		g = 1;
		gg = wMaxColor;
	}
	else if (bColor == BGCOLOR_BLUE)
	{
		b = 1;
		bb = wMaxColor;
	}
	else if (bColor == BGCOLOR_GREY)
	{
		r = g = b = 1;
		rr = gg = bb = wMaxColor;
	}
	else if (bColor == BGCOLOR_WHITE)
	{
		Idu_PaintWin(ImgWin, 0xFFFF8080);
		return;
	}

	// 1 single color
	if (wMinColor == wMaxColor)
	{
		Idu_PaintWin(ImgWin, RGB2YUV((r * wMaxColor), (g * wMaxColor), (b * wMaxColor)));
		return;
	}

	paintArea = ImgWin->wWidth >> 1;
	if ((wMaxColor - wMinColor) != 0)
		barLength = (ImgWin->wWidth / (wMaxColor - wMinColor)) >> 1;
	else
		barLength = (ImgWin->wWidth) >> 1;

	if (barLength == 1)
		barLength = 2;

	for (i = 0; i < paintArea; i += barLength)
	{
		Idu_PaintWinArea(ImgWin, x, 0, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		//ImgSetPoint(ImgWin, x, 0);
		//ImgPaintArea(ImgWin, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		rr -= r;
		gg -= g;
		bb -= b;
		x += barLength;
		TaskYield();
	}

	for (i = 0; i < paintArea; i += barLength)
	{
		Idu_PaintWinArea(ImgWin, x, 0, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		//ImgSetPoint(ImgWin, x, 0);
		//ImgPaintArea(ImgWin, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		rr += r;
		gg += g;
		bb += b;
		x += barLength;
		TaskYield();
	}
}
#endif

#if YUV444_ENABLE
void Idu_PaintBackGround444(ST_IMGWIN * ImgWin, WORD wMinColor, WORD wMaxColor, BYTE bColor)
{
	WORD i, x, paintArea, barLength;
	WORD r, g, b, rr, gg, bb;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	if ((wMinColor > wMaxColor) || (wMinColor > 255) || (wMaxColor > 255))
	{
		bColor = BGCOLOR_BLACK;
	}

	x = r = g = b = rr = gg = bb = 0;
	if (bColor == BGCOLOR_BLACK)
	{
		Idu_PaintWin444(ImgWin, 0x00008080);
		return;
	}
	else if (bColor == BGCOLOR_RED)
	{
		r = 1;
		rr = wMaxColor;
	}
	else if (bColor == BGCOLOR_GREEN)
	{
		g = 1;
		gg = wMaxColor;
	}
	else if (bColor == BGCOLOR_BLUE)
	{
		b = 1;
		bb = wMaxColor;
	}
	else if (bColor == BGCOLOR_GREY)
	{
		r = g = b = 1;
		rr = gg = bb = wMaxColor;
	}
	else if (bColor == BGCOLOR_WHITE)
	{
		Idu_PaintWin444(ImgWin, 0xFFFF8080);
		return;
	}

	// 1 single color
	if (wMinColor == wMaxColor)
	{
		Idu_PaintWin444(ImgWin, RGB2YUV((r * wMaxColor), (g * wMaxColor), (b * wMaxColor)));
		return;
	}

	paintArea = ImgWin->wWidth >> 1;
	if ((wMaxColor - wMinColor) != 0)
		barLength = (ImgWin->wWidth / (wMaxColor - wMinColor)) >> 1;
	else
		barLength = (ImgWin->wWidth) >> 1;

	if (barLength == 1)
		barLength = 2;

	for (i = 0; i < paintArea; i += barLength)
	{
		Idu_PaintWinArea444(ImgWin, x, 0, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		//ImgSetPoint(ImgWin, x, 0);
		//ImgPaintArea(ImgWin, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		rr -= r;
		gg -= g;
		bb -= b;
		x += barLength;
		TaskYield();
	}

	for (i = 0; i < paintArea; i += barLength)
	{
		Idu_PaintWinArea444(ImgWin, x, 0, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		//ImgSetPoint(ImgWin, x, 0);
		//ImgPaintArea(ImgWin, barLength, ImgWin->wHeight, RGB2YUV(rr, gg, bb));
		rr += r;
		gg += g;
		bb += b;
		x += barLength;
		TaskYield();
	}
}
#endif


void PaintVColorBar()
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        PaintVColorBar444();
        return;
    }
#endif
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	WORD w = psWin->wWidth / 8;

	ImgSetPoint(psWin, 0, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 0));

	ImgSetPoint(psWin, w * 1, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 0));

	ImgSetPoint(psWin, w * 2, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 255));

	ImgSetPoint(psWin, w * 3, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 0));

	ImgSetPoint(psWin, w * 4, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 255));

	ImgSetPoint(psWin, w * 5, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 0));

	ImgSetPoint(psWin, w * 6, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 255));

	ImgSetPoint(psWin, w * 7, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 255));
}


#if YUV444_ENABLE
void PaintVColorBar444()
{
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	WORD w = psWin->wWidth / 8;

	ImgSetPoint(psWin, 0, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 0));

	ImgSetPoint(psWin, w * 1, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 0));

	ImgSetPoint(psWin, w * 2, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 255));

	ImgSetPoint(psWin, w * 3, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 0));

	ImgSetPoint(psWin, w * 4, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 255));

	ImgSetPoint(psWin, w * 5, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 0));

	ImgSetPoint(psWin, w * 6, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 255));

	ImgSetPoint(psWin, w * 7, 0);
	ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 255));
}
#endif

void PaintAllColorBar()
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        PaintHColorBar444();
        return;
    }
#endif
#if 1
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	UartOutText("PaintAllColourBar\r\n");  // test 608 Griffy

	ImgSetPoint(psWin, 0, 0);
	ImgPaintArea(psWin, psWin->wWidth,  psWin->wHeight, RGB2YUV(255, 0, 0));


#endif
}

void PaintGrayScaleColorBar()
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        PaintVColorBar444();
        return;
    }
#endif
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	WORD w = psWin->wWidth / 16;

	ImgSetPoint(psWin, w * 15, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 0));

	ImgSetPoint(psWin, w * 14, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(1, 1, 1));

	ImgSetPoint(psWin, w * 13, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(3, 3, 3));

	ImgSetPoint(psWin, w * 12, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(5, 5, 5));

	ImgSetPoint(psWin, w * 11, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(7, 7, 7));

	ImgSetPoint(psWin, w * 10, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(11, 11, 11));

	ImgSetPoint(psWin, w * 9, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(15, 15, 15));

	ImgSetPoint(psWin, w * 8, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(23, 23, 23));

	ImgSetPoint(psWin, w * 7, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(31, 31, 31));

	ImgSetPoint(psWin, w * 6, 0);
	ImgPaintArea(psWin, w, sWin->wHeight, RGB2YUV(47, 47, 47));

	ImgSetPoint(psWin, w * 5, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(63, 63, 63));

	ImgSetPoint(psWin, w * 4, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(95, 95, 95));

	ImgSetPoint(psWin, w * 3, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(127, 127, 127));

    ImgSetPoint(psWin, w * 2, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(191, 191, 191));

	ImgSetPoint(psWin, w * 1, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(223, 223, 223));

	ImgSetPoint(psWin, w * 0, 0);
	ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 255));
	
}



void PaintHColorBar()
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        PaintHColorBar444();
        return;
    }
#endif
#if 1
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	WORD h = psWin->wHeight / 8;
	UartOutText("PaintVColourBar\r\n");  // test 608 Griffy
	mpDebugPrint("h = %d\r\n", h);

	ImgSetPoint(psWin, 0, 0);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 0));

	ImgSetPoint(psWin, 0, h * 1);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 0));

	ImgSetPoint(psWin, 0, h * 2);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 255));

	ImgSetPoint(psWin, 0, h * 3);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 0));

	ImgSetPoint(psWin, 0, h * 4);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 255));

	ImgSetPoint(psWin, 0, h * 5);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 0));

	ImgSetPoint(psWin, 0, h * 6);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 255));

	ImgSetPoint(psWin, 0, h * 7);
	ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 255));
#endif
}


#if YUV444_ENABLE
void PaintHColorBar444()
{
#if 1
	ST_IMGWIN *psWin = Idu_GetCurrWin();
	WORD h = psWin->wHeight / 8;
	UartOutText("PaintVColourBar\r\n");  // test 608 Griffy
	mpDebugPrint("h = %d\r\n", h);

	ImgSetPoint(psWin, 0, 0);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 0));

	ImgSetPoint(psWin, 0, h * 1);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 0));

	ImgSetPoint(psWin, 0, h * 2);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 255));

	ImgSetPoint(psWin, 0, h * 3);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 0));

	ImgSetPoint(psWin, 0, h * 4);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 255));

	ImgSetPoint(psWin, 0, h * 5);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 0));

	ImgSetPoint(psWin, 0, h * 6);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 255));

	ImgSetPoint(psWin, 0, h * 7);
	ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 255));
#endif
}
#endif


//Griffy ++
//This function paint both vertical and horizontal color bar
void PaintMixColorBar()
{
#if YUV444_ENABLE
    if(g_IDU_444_422_Flag==1)
    {
        PaintMixColorBar444();
        return;
    }
#endif
    ST_IMGWIN *psWin = Idu_GetCurrWin();
    //First, paint horizontal color bar
	  WORD h = psWin->wHeight / 16;
	  ImgSetPoint(psWin, 0, 0);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 0));

	  ImgSetPoint(psWin, 0, h * 1);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 0));

	  ImgSetPoint(psWin, 0, h * 2);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 255));

	  ImgSetPoint(psWin, 0, h * 3);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 0));

	  ImgSetPoint(psWin, 0, h * 4);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 255));

	  ImgSetPoint(psWin, 0, h * 5);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 0));

	  ImgSetPoint(psWin, 0, h * 6);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 255));

	  ImgSetPoint(psWin, 0, h * 7);
	  ImgPaintArea(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 255));

    //Then, paint vertical color bar
    WORD w = psWin->wWidth / 8;
	  ImgSetPoint(psWin, 0, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 0));

	  ImgSetPoint(psWin, w * 1, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 0));

	  ImgSetPoint(psWin, w * 2, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 255));

	  ImgSetPoint(psWin, w * 3, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 0));

	  ImgSetPoint(psWin, w * 4, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 255));

	  ImgSetPoint(psWin, w * 5, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 0));

	  ImgSetPoint(psWin, w * 6, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 255));

	  ImgSetPoint(psWin, w * 7, h * 8);
	  ImgPaintArea(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 255));
}


#if YUV444_ENABLE
void PaintMixColorBar444()
{
    ST_IMGWIN *psWin = Idu_GetCurrWin();
    //First, paint horizontal color bar
	  WORD h = psWin->wHeight / 16;
	  ImgSetPoint(psWin, 0, 0);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 0));

	  ImgSetPoint(psWin, 0, h * 1);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 0));

	  ImgSetPoint(psWin, 0, h * 2);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 255));

	  ImgSetPoint(psWin, 0, h * 3);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 0, 0));

	  ImgSetPoint(psWin, 0, h * 4);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 255));

	  ImgSetPoint(psWin, 0, h * 5);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 255, 0));

	  ImgSetPoint(psWin, 0, h * 6);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(0, 255, 255));

	  ImgSetPoint(psWin, 0, h * 7);
	  ImgPaintArea444(psWin, psWin->wWidth, h, RGB2YUV(255, 0, 255));

    //Then, paint vertical color bar
    WORD w = psWin->wWidth / 8;
	  ImgSetPoint(psWin, 0, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 0));

	  ImgSetPoint(psWin, w * 1, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 0));

	  ImgSetPoint(psWin, w * 2, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 255));

	  ImgSetPoint(psWin, w * 3, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 0, 0));

	  ImgSetPoint(psWin, w * 4, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 255));

	  ImgSetPoint(psWin, w * 5, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 255, 0));

	  ImgSetPoint(psWin, w * 6, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(0, 255, 255));

	  ImgSetPoint(psWin, w * 7, h * 8);
	  ImgPaintArea444(psWin, w, psWin->wHeight, RGB2YUV(255, 0, 255));
}
//Griffy --
#endif



