/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#if BMP_FONT_ENABLE

#include "idu.h"
#include "display.h"
#include "FontDisp.h"
#include "ReaderFontStructure.h"

BYTE g_bFixFontFlag = 0;
static BYTE g_bImgTextGap = 2;

void ChangFontSize2Fix()
{
    g_bFixFontFlag = 1;
}

void ChangeFontSize2Nonfix()
{
    g_bFixFontFlag = 0;
}

BYTE ImgPutChar_GrayFont(ST_IMGWIN *psWin, ST_FONT *sFont, WORD wData, BYTE MappingTable)
{
	DWORD dwOffset;	
	BYTE i, j;
	BYTE *pbStartAddr;
	BYTE *bitmap;
	BYTE bMask = 0,temp;
	BYTE bWidth,bHeight;
	BYTE bPixel, bNextPixel;
	BYTE bSingleBit, bExtraBit;
	BYTE bBitPerPixel;
	BYTE tt0, tt1;
	WORD wTemp;
	SBYTE temp1;

	if(wData == ' ')
	{
		sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
		sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
		return 0;
	}	
	
	bitmap =(BYTE *)IduFontGetCharBitmapAddr(wData);
	if (bitmap == NULL)
	{
		bitmap =(BYTE *)IduFontGetCharBitmapAddr(UNKNOWN_CHAR);
		if (bitmap == NULL) return 0;
	}

	bBitPerPixel = IduFontGetBitPerPixel();
	if(bBitPerPixel > 4)
		return;
	
	wTemp = (1 << bBitPerPixel) - 1;
	bWidth = *bitmap;
	bitmap++;
	
	if (bWidth == 0 || bWidth > 64) return 1;
	
//	if(sFont->wX & 0x01) sFont->wX++;
	if(sFont->wX & 0x01)
		bSingleBit = 1;
	else
		bSingleBit = 0;

	if((bWidth - bSingleBit) & 0x01)
		bExtraBit = 1;
	else
		bExtraBit = 0;
	
	sFont->wDisplayWidth += bWidth;
	if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
    {   
		MP_DEBUG("%x exceed", wData);
        //idu_Draw2Point(psWin , sFont);
		return 1;
    }

	bHeight = *bitmap;
	bitmap++;

	if (bHeight > 64) 
		return 1;

	pbStartAddr = (BYTE *)((DWORD)psWin->pdwStart + (sFont->wY * psWin->dwOffset/2 + sFont->wX ) * 2);
	dwOffset = psWin->dwOffset- (bWidth << 1);

	BYTE cy = sFont->bFontColor;
	BYTE cb = sFont->bFontCb;
	BYTE cr = sFont->bFontCr;
	DWORD dwFontColor = (cy << 24) | (cy << 16) | (cb << 8) | cr;

	while (bHeight--)
	{
		j = ((bWidth - bSingleBit) >> 1) + bSingleBit + bExtraBit;
		i = j - 1;

		bMask = 0;
		temp = 0;
		
		while (j--)
		{
			if (!bMask)
			{
				for(tt0=0; tt0<bBitPerPixel; tt0++)
				{
					if((bSingleBit && (i == j)) || (bExtraBit && (j == 0)))
					{
						bMask >>= 1;
						bMask |= 0x80;
					}
					else
					{
						bMask >>= 2;
						bMask |= 0xc0;
					}
				}
					
				temp = 8 - bBitPerPixel*2;
					
				bPixel = *bitmap++;
				
				if((bSingleBit) && (i != j))
				{
					bNextPixel = *bitmap;
					bPixel = (bPixel << bBitPerPixel) | (bNextPixel >> (8 - bBitPerPixel));
				}

				while (j > 4 && bPixel == 0)
				{
					if((bSingleBit) && (i == j))
						break;
					
					pbStartAddr += (16/bBitPerPixel);
					j -= (4/bBitPerPixel);
					bPixel = *bitmap++;
					if(bSingleBit)
					{
						bNextPixel = *bitmap;
						bPixel = (bPixel << bBitPerPixel) | (bNextPixel >> (8 - bBitPerPixel));
					}					
				}
			}

			if((bSingleBit) && (i == j))			// access bSingleBit
			{
				pbStartAddr -= 2;
				
				if (bPixel & bMask)
				{
					tt1 = (bPixel & bMask) >> (temp + bBitPerPixel);
					tt0 = 0;
					*(pbStartAddr) =  ((cy*tt0) + (*pbStartAddr)*(wTemp-tt0))/wTemp;
					*(pbStartAddr+1) =  ((cy*tt1) + (*(pbStartAddr+1))*(wTemp-tt1))/wTemp;
					
					tt0 = tt0 + tt1;
					tt1 = (wTemp<<1) - tt0;
					*(pbStartAddr + 2) = ((*(pbStartAddr + 2))*tt1 + cb*tt0)/(wTemp<<1);
					*(pbStartAddr + 3) = ((*(pbStartAddr + 3))*tt1 + cr*tt0)/(wTemp<<1);
				}
	
				bMask = 0;
				bitmap--;
			}
			else if((bExtraBit) && (j == 0))		// access bSingleBit
			{
				if (bPixel & bMask)
				{
					tt1 = 0;
					tt0 = (bPixel & bMask) >> (temp + bBitPerPixel);
					*(pbStartAddr) =  ((cy*tt0) + (*pbStartAddr)*(wTemp-tt0))/wTemp;
					*(pbStartAddr+1) =  ((cy*tt1) + (*(pbStartAddr+1))*(wTemp-tt1))/wTemp;
					
					tt0 = tt0 + tt1;
					tt1 = (wTemp<<1) - tt0;
					*(pbStartAddr + 2) = ((*(pbStartAddr + 2))*tt1 + cb*tt0)/(wTemp<<1);
					*(pbStartAddr + 3) = ((*(pbStartAddr + 3))*tt1 + cr*tt0)/(wTemp<<1);
				}				
				pbStartAddr -= 2;
			}
			else									// access other pixel
			{
				if (bPixel & bMask)
				{
					if ((bPixel & bMask) == bMask)
					{
						*((DWORD *) pbStartAddr) = dwFontColor;
					}
					else
					{
						tt1 = ((bPixel & bMask) >> temp) & ((1<<bBitPerPixel) - 1);
						tt0 = (bPixel & bMask) >> (temp + bBitPerPixel);
						*(pbStartAddr) =  ((cy*tt0) + (*pbStartAddr)*(wTemp-tt0))/wTemp;
						*(pbStartAddr+1) =  ((cy*tt1) + (*(pbStartAddr+1))*(wTemp-tt1))/wTemp;
						
						tt0 = tt0 + tt1;
						tt1 = (wTemp<<1) - tt0;
						*(pbStartAddr + 2) = ((*(pbStartAddr + 2))*tt1 + cb*tt0)/(wTemp<<1);
						*(pbStartAddr + 3) = ((*(pbStartAddr + 3))*tt1 + cr*tt0)/(wTemp<<1);
					}
				}
	
				bMask >>= (2*bBitPerPixel) ;
			}

			pbStartAddr += 4;
			temp -= (2*bBitPerPixel);
			
		}
		
		if (bSingleBit  && !bMask)
			bitmap++;
		pbStartAddr += dwOffset;
	}

	sFont->wX += bWidth + sFont->bTextGap; 
	sFont->wDisplayWidth += sFont->bTextGap;
    return 0;
}


BYTE ImgPutChar(ST_IMGWIN *psWin, ST_FONT *sFont, WORD wData, BYTE MappingTable)
{
		return ImgPutChar_GrayFont(psWin, sFont, wData, MappingTable);
}

DWORD g_dwSystemFontColor = 0xffff8080;
DWORD Idu_GetSystemFontColor()
{
    return g_dwSystemFontColor;
}
BYTE bSystemFontColorRGB = 0; // 0 - not yet set
BYTE bSystemFontColorR = 0;
BYTE bSystemFontColorG = 0;
BYTE bSystemFontColorB = 0;
BYTE Idu_GetFontColor(BYTE *pR, BYTE *pG, BYTE *pB)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s", __FUNCTION__);
    //mpDebugPrint("bSystemFontColorRGB = %d", bSystemFontColorRGB);
    //mpDebugPrint("bSystemFontColorR = %d", bSystemFontColorR);
    //mpDebugPrint("bSystemFontColorG = %d", bSystemFontColorG);
    //mpDebugPrint("bSystemFontColorB = %d", bSystemFontColorB);
    if(bSystemFontColorRGB == 1)
    {
        *pR = bSystemFontColorR;
        *pG = bSystemFontColorG;
        *pB = bSystemFontColorB;
        return PASS;
    }
    return FAIL;
}
// first init set - in xpgDriveFunc.c, xpgStartAfterBootFinish()
void Idu_SetFontColor(BYTE R, BYTE G, BYTE B)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s, R = %d, B = %d, B = %d", __FUNCTION__, R, G, B);
    bSystemFontColorRGB = 1;
    bSystemFontColorR = R;
    bSystemFontColorG = G;
    bSystemFontColorB = B;
	g_dwSystemFontColor = RGB2YUV(R, G, B);
}

void Idu_SetFontYUV(DWORD dwColor)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s", __FUNCTION__);
	g_dwSystemFontColor = dwColor;
}

DWORD Idu_FontColorSet(BYTE R, BYTE G, BYTE B)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s", __FUNCTION__);
	return g_dwSystemFontColor = RGB2YUV(R, G, B);
}


///
///@ingroup group_IDU
///@brief   Put one string to image win
///
///@param   BYTE *string    output characters
///@param   WORD startX starting point X coordinate
///@param   WORD startY starting point Y coordinate
///@param   BYTE length short file name (8) or long file name (16)
///
///@retval  NONE
///
///@remark  The service call will put one string to image win. And the string
///         must end by 0x00.
///
/*WORD Idu_PrintString(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag,
					 WORD wWidth)
*/
WORD Idu_PrintStringWithSize(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag,
					 WORD wWidth, WORD wHeight, BYTE Font_size)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s: string = %s", __func__, string);
    
	ST_FONT ft;

	if (trgWin == NULL || trgWin->pdwStart == NULL || string == NULL)
		return;

	ft.wX = startX;
	ft.wY = startY;
	ft.bTextGap = g_bImgTextGap;

	register DWORD dwFontColor = g_dwSystemFontColor;

	ft.bFontColor = (dwFontColor >> 16) & 0xff;	//YYCbCr - Y
	//mpDebugPrint("%s: dwFontColor = 0x%X, ft.bFontColor = 0x%X", __func__, dwFontColor, ft.bFontColor);
	ft.bFontCb = (dwFontColor >> 8) & 0xff;
	ft.bFontCr = dwFontColor & 0xff;
	ft.wWidth = wWidth;
	ft.wDisplayWidth = 0;
	ft.bFontSize = Font_size;

#if 0
	ft.bWidth = DEFAULT_FONT_SIZE;
	ft.bHeight = DEFAULT_FONT_SIZE;
#endif
	// check if out of win
	if (startX >= trgWin->wWidth)
		return 0;
	if (startY >= trgWin->wHeight)
		return 0;

    Font_Draw(trgWin, &ft, string, UnicodeFlag);

    if(ft.wX < trgWin->wWidth)
        return ft.wX;
    else
        return 0;

}


#endif

