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

	if (bHeight > 256) 
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

BYTE ImgPutCharFake(ST_FONT *sFont, WORD wData, BYTE MappingTable)
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
		return 1;
	
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

	if (bHeight > 256) 
		return 1;

	//pbStartAddr = (BYTE *)((DWORD)psWin->pdwStart + (sFont->wY * psWin->dwOffset/2 + sFont->wX ) * 2);
	//dwOffset = psWin->dwOffset- (bWidth << 1);

	BYTE cy = sFont->bFontColor;
	BYTE cb = sFont->bFontCb;
	BYTE cr = sFont->bFontCr;
	DWORD dwFontColor = (cy << 24) | (cy << 16) | (cb << 8) | cr;



	sFont->wX += bWidth + sFont->bTextGap; 
	sFont->wDisplayWidth += sFont->bTextGap;
    return 0;
}



BYTE ImgPutChar(ST_IMGWIN *psWin, ST_FONT *sFont, WORD wData, BYTE MappingTable)
{
		return ImgPutChar_GrayFont(psWin, sFont, wData, MappingTable);
}

DWORD g_dwSystemFontColor = IDU_FONT_YUVCOLOR_DEFAULT_WHITE;
DWORD Idu_GetSystemFontColor()
{
    return g_dwSystemFontColor;
}
#if 0
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
#endif

void Idu_SetFontYUV(DWORD dwColor)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s", __FUNCTION__);
	g_dwSystemFontColor = dwColor;
}

void Idu_FontColorSet(BYTE R, BYTE G, BYTE B)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s", __FUNCTION__);
	g_dwSystemFontColor = RGB2YUV(R, G, B);
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
		return 0;

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

WORD Idu_GetStrPosByWidth(BYTE *string, BYTE UnicodeFlag,WORD wWidth)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s: string = %s", __func__, string);
    
	ST_FONT ft;

	if (string == NULL)
		return 0;

	ft.wX = 0;
	ft.wY = 0;
	ft.bTextGap = g_bImgTextGap;

	DWORD dwFontColor = g_dwSystemFontColor;

	ft.bFontColor = (dwFontColor >> 16) & 0xff;	//YYCbCr - Y
	//mpDebugPrint("%s: dwFontColor = 0x%X, ft.bFontColor = 0x%X", __func__, dwFontColor, ft.bFontColor);
	ft.bFontCb = (dwFontColor >> 8) & 0xff;
	ft.bFontCr = dwFontColor & 0xff;
	ft.wWidth = 0;
	ft.wDisplayWidth = 0;
	ft.bFontSize = 0;

    return Font_GetStrPosByWidth(&ft, string, UnicodeFlag,wWidth);
}

WORD Idu_GetStringWidth(BYTE *string, BYTE UnicodeFlag)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s: string = %s", __func__, string);
    
	ST_FONT ft;

	if (string == NULL)
		return 0;

	ft.wX = 0;
	ft.wY = 0;
	ft.bTextGap = g_bImgTextGap;

	DWORD dwFontColor = g_dwSystemFontColor;

	ft.bFontColor = (dwFontColor >> 16) & 0xff;	//YYCbCr - Y
	//mpDebugPrint("%s: dwFontColor = 0x%X, ft.bFontColor = 0x%X", __func__, dwFontColor, ft.bFontColor);
	ft.bFontCb = (dwFontColor >> 8) & 0xff;
	ft.bFontCr = dwFontColor & 0xff;
	ft.wWidth = 0;
	ft.wDisplayWidth = 0;
	ft.bFontSize = 0;

#if 0
	ft.bWidth = DEFAULT_FONT_SIZE;
	ft.bHeight = DEFAULT_FONT_SIZE;
#endif
	// check if out of win

    Font_GetStrWidth(&ft, string, UnicodeFlag);

    return ft.wX;
    
}

WORD Idu_PrintStringLeftNewLine(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag, WORD wWidth)
{
    WORD wLen;
    int pos=0,pos1;
	BYTE *pbBuffer=NULL,*pbStr=string;
	
    if (wWidth <= 8)
        return Idu_PrintString(trgWin, string, startX, startY, UnicodeFlag, 0);
	wWidth-=8;
	startX+=4;
	pos=0;
	pbBuffer = (BYTE *) ext_mem_malloc((Str_Length(string,UnicodeFlag)+4)<<1);
	while (1)
	{
		wLen=Str_Length(&pbStr[pos],UnicodeFlag);
		if (wLen<=0)
			break;
		pos1 = Idu_GetStrPosByWidth(&pbStr[pos], UnicodeFlag,wWidth);
		if (pos1<=0)
			break;
		memcpy(pbBuffer,&pbStr[pos],pos1);
		pbBuffer[pos1]=0;
		pbBuffer[pos1+1]=0;
		Idu_PrintString(trgWin, pbBuffer, startX, startY, UnicodeFlag, wWidth);
		if (wLen<=pos1)
			break;
		pos+=pos1;
		startY+=IduFontGetMaxHeight();
	}

	if (pbBuffer)
		ext_mem_free(pbBuffer);
}

WORD Idu_PrintStringCenterNewLine(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag, WORD wWidth)
{
    WORD wStrWidth,wLen;
    int newX,pos=0,pos1;
	BYTE *pbBuffer=NULL,*pbStr=string;
	
    if (wWidth <= 8)
        return Idu_PrintString(trgWin, string, startX, startY, UnicodeFlag, 0);
	wWidth-=8;
	startX+=4;
	pos=0;
	pbBuffer = (BYTE *) ext_mem_malloc((Str_Length(string,UnicodeFlag)+4)<<1);
	while (1)
	{
		wLen=Str_Length(&pbStr[pos],UnicodeFlag);
		if (wLen<=0)
			break;
		pos1 = Idu_GetStrPosByWidth(&pbStr[pos], UnicodeFlag,wWidth);
		if (pos1<=0)
			break;
		memcpy(pbBuffer,&pbStr[pos],pos1);
		pbBuffer[pos1]=0;
		pbBuffer[pos1+1]=0;
	    wStrWidth = Idu_GetStringWidth(pbBuffer, UnicodeFlag);
		newX = startX + (wWidth - wStrWidth)/2;
		if (newX < 0)
			newX = 0;
		Idu_PrintString(trgWin, pbBuffer, (WORD)newX, startY, UnicodeFlag, wWidth);
		if (wLen<=pos1)
			break;
		pos+=pos1;
		startY+=IduFontGetMaxHeight();
	}

	if (pbBuffer)
		ext_mem_free(pbBuffer);
}

WORD Idu_PrintStringCenter(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag, WORD wWidth)
{
    WORD wStrWidth;
    int offsetx;
    int newX;
    if (wWidth == 0)
        return Idu_PrintString(trgWin, string, startX, startY, UnicodeFlag, 0);

    wStrWidth = Idu_GetStringWidth(string, UnicodeFlag);
    //mpDebugPrint("wStrWidth = %d", wStrWidth);
    
    offsetx = wWidth - wStrWidth;
    offsetx /= 2;
    newX = startX + offsetx;
    if (newX < 0)
        newX = 0;
    return Idu_PrintString(trgWin, string, (WORD)newX, startY, UnicodeFlag, 0);
}

WORD Idu_PrintStringCenterWH(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag, WORD wWidth,WORD wHeight)
{
    WORD wStrWidth;
    int offsetx;
    int newX;
    if (wWidth == 0)
        return Idu_PrintString(trgWin, string, startX, startY, UnicodeFlag, 0);

    wStrWidth = Idu_GetStringWidth(string, UnicodeFlag);
    //mpDebugPrint("wStrWidth = %d", wStrWidth);
    
    offsetx = wWidth - wStrWidth;
    offsetx /= 2;
    newX = startX + offsetx;
    if (newX < 0)
        newX = 0;
	if (wHeight>IduFontGetMaxHeight())
		startY+=(wHeight-IduFontGetMaxHeight())/2;

    return Idu_PrintString(trgWin, string, (WORD)newX, startY, UnicodeFlag, 0);
}

WORD Idu_PrintStringRight(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag)
{
    int newX;
    WORD wStrWidth;
    wStrWidth = Idu_GetStringWidth(string, UnicodeFlag);
    newX = startX - wStrWidth;
    if (newX < 0)
        newX = 0;
    return Idu_PrintString(trgWin, string, (WORD)newX, startY, UnicodeFlag, 0);
}




#endif






