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
#include "FontDisp.h"

#if OSD_ENABLE
static ST_OSDWIN sWinOSD;
#if OSD_DISPLAY_CACHE
static ST_OSDWIN sOsdCacheWin;
#endif
#endif

void Idu_OSDWinInit(ST_OSDWIN * win, DWORD * buff, WORD wWidth, WORD wHeight, BYTE bInterlace, BYTE bitWidth)
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	win->pdwStart = buff;
	win->wHeight = wHeight;
	win->wWidth = wWidth;
	win->wX = win->wY = 0;
	win->bDepth = bitWidth;
	win->boPainted = TRUE;
	win->boOn = TRUE;
	win->bInterlace = bInterlace;
	win->dwBufferSize = (wWidth * wHeight) >> OSD_BIT_OFFSET;
#endif
}

void Idu_OsdDMAInit(ST_OSDWIN * OsdImgWin)
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;
	CHANNEL *osd_dma = (CHANNEL *) DMA_OSD_BASE;

	if (OsdImgWin->bInterlace == INTERLACE)
	{
		osd_dma->StartA = (DWORD) OsdImgWin->pdwStart;
		osd_dma->EndA = (DWORD) OsdImgWin->pdwStart +
			(OsdImgWin->wWidth >> OSD_BIT_OFFSET) * (OsdImgWin->wHeight - 1) - 1;

		osd_dma->StartB = osd_dma->StartA + (OsdImgWin->wWidth >> OSD_BIT_OFFSET);
		osd_dma->EndB = osd_dma->EndA + (OsdImgWin->wWidth >> OSD_BIT_OFFSET);

		osd_dma->LineCount = ((OsdImgWin->wWidth >> OSD_BIT_OFFSET) << 16) +
			(OsdImgWin->wWidth >> OSD_BIT_OFFSET) - 1;

		osd_dma->Control = 0x0000000e;
	}
	else
	{
		osd_dma->StartA = (DWORD) OsdImgWin->pdwStart;
		osd_dma->EndA = (DWORD) OsdImgWin->pdwStart +
			(OsdImgWin->wWidth >> OSD_BIT_OFFSET) * OsdImgWin->wHeight - 1;

		osd_dma->LineCount = (OsdImgWin->wWidth >> OSD_BIT_OFFSET) - 1;
		osd_dma->Control = 0x00000002;
	}

	idu->IduCtrl0 &= 0xFFFF8FFF;	//Clear bit12 ~ bit14, default is 2-bit
#if (OSD_BIT_WIDTH == 8)
    idu->IduCtrl0 |= 0x00004000;	//OSD_SEL = 8bit
#elif (OSD_BIT_WIDTH == 2)
    idu->IduCtrl0 |= 0x00002000;	//OSD_SEL = 8bit
#endif

mpDebugPrint("idu->IduCtrl0==%x",idu->IduCtrl0);

	idu->OsdHStr = OsdImgWin->wWidth - 1;
	idu->OsdHEnd = OsdImgWin->wWidth - ALIGN_CUT_16(OsdImgWin->wWidth);
	idu->OsdVStr = OsdImgWin->wHeight - 1;
	idu->OsdVEnd = 0;
#endif
}

// Remove this API outside for internal usage only
//
//@ingroup group_OSD
//@brief   Turn on OSD
//
//@param	NONE
//
//@step1	Turn off OSD. Fill osd_dma parameters.
//@step2	Wait buffer end. If not, it shows noises.
//@step3	Turn off DMA and IDU modules. If not, the OSD or background will shift.
//@step4	Turn on DMA, IDU, OSD_DMA at the same time. If not, the OSD or background will shift.
//
//@retval  NONE
//
void Idu_OsdDMAOn()
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE

    CHANNEL *idu_dma = (CHANNEL *) (DMA_IDU_BASE);
    CHANNEL *osd_dma = (CHANNEL *) DMA_OSD_BASE;
    IDU *idu = (IDU *) IDU_BASE;
    ST_TCON *pstTCON = g_psSystemConfig->sScreenSetting.pstCurTCON;
    ST_IMGWIN *ImgWin = Idu_GetCurrWin();

    idu_dma->Control |= 0x01000000;	//dma buffer end interrupt
    idu_dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt indicator

	if (pstTCON->bInterlace == INTERLACE){
    	while ((idu_dma->Control & 0x00018000) != 0x00018000)  TaskYield();
		idu_dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		idu_dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
		idu_dma->EndA = idu_dma->StartA + (ImgWin->wWidth * (ImgWin->wHeight - 1) * 2) - 1;

		while ((idu_dma->Control & 0x00018000) != 0x00010000) TaskYield();
		idu_dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt
		idu_dma->StartB = idu_dma->StartA + ImgWin->wWidth * 2;
		idu_dma->EndB = idu_dma->EndA + (ImgWin->wWidth * 2);
	}else
	{
		while ((idu_dma->Control & 0x00018000) != 0x00018000) TaskYield();
		idu_dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		idu_dma->StartA = ((DWORD) ImgWin->pdwStart | 0xa0000000);
		idu_dma->EndA =
			((DWORD) ImgWin->pdwStart | 0xa0000000) + (ImgWin->wWidth * ImgWin->wHeight * 2) - 1;

		while ((idu_dma->Control & 0x00018000) != 0x00010000) TaskYield();
		idu_dma->Control &= ~0x00010000;	// clear IDU buffer end interrupt

		idu_dma->StartB = idu_dma->StartA;
		idu_dma->EndB = idu_dma->EndA;

	}
    idu_dma->Control &= ~0x00000001; //disable idu dma //Don't delete it, the OSD will shift.
    idu->IduCtrl0 &= ~0x00000001; //disable OSD
    //Don't move away the following lines.
    idu_dma->Control |= 0x00000001;	//idu dma enable
    osd_dma->Control |= 0x00000001;    //osd dma enable
    idu->IduCtrl0 |= 0x00001001;	//OSD_EN = 1
#endif
}

///
///@ingroup group_OSD
///@brief   Put one string to OSD win
///
///@param   ST_OSDWIN *psWin    point of ST_OSDWIN to put string
///@param   ST_FONT *sFont      point of font parameter
///@param   BYTE *pbString      The point of string
///
///@retval  NONE
///
///@remark  The service call will put one string to OSD win. And the string
///         must end by 0x00.
///
//void Idu_OsdPutStr(ST_OSDWIN * psWin, BYTE * pbString, WORD startX, WORD startY, BYTE bIndex)
void Idu_OsdPutStrWithSize(ST_OSDWIN * psWin, BYTE * pbString, WORD startX, WORD startY, BYTE bColorIndex, BYTE UnicodeFlag, WORD wWidth, BYTE Font_Size)
{
    MP_DEBUG("%s: bColorIndex = %d", __func__, bColorIndex);
    //xpgDumpOSDPalette();
#if OSD_ENABLE
	ST_FONT ft;
	ft.wX = startX;
	ft.wY = startY;
	ft.bTextGap = 1;
	ft.bFontColor = bColorIndex;
    ft.wDisplayWidth = 0;
    ft.wWidth = wWidth;

	//ft.bWidth = DEFAULT_FONT_SIZE;
	//ft.bHeight = DEFAULT_FONT_SIZE;
	ft.bFontSize = Font_Size;

	Font_Draw_OSD(psWin, &ft, pbString);
#endif
}

// imgString.c calling
void Idu_OSDPrint(ST_OSDWIN * psWin, BYTE * pbString, WORD startX, WORD startY, BYTE bColor)
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	WORD wTemp;
	BYTE *pbTemp;
	ST_FONT ft;

	ft.wX = startX;
	ft.wY = startY;
	ft.bTextGap = 1;
	ft.bFontColor = bColor;
    ft.wDisplayWidth = 0;
    ft.wWidth = 0;

	//ft.bWidth = DEFAULT_FONT_SIZE;
	//ft.bHeight = DEFAULT_FONT_SIZE;
	ft.bFontSize = DEFAULT_FONT_SIZE;

    Font_Draw_OSD(psWin, &ft, pbString, 0);
#endif
}


ST_OSDWIN *Idu_GetOsdWin()
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	if (sWinOSD.pdwStart == NULL)
		return NULL;
	else
		return &sWinOSD;
#else
	return NULL;
#endif
}

extern DWORD dwDefaultPalette[];

void Idu_OsdSetDefaultPalette()
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
    Idu_OsdSetPalette(dwDefaultPalette, 16);
#endif
}

BOOL Idu_OsdSetPalette(DWORD *pdwPalette, BYTE bCount)
{
    MP_DEBUG("%s: pdwPalette = 0x%X, bCount = %d", __func__, pdwPalette, bCount);
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;;
	BYTE i;

#if (OSD_BIT_WIDTH == 2)
    bCount = (bCount > 0x4) ? 0x3 : bCount;
#elif (OSD_BIT_WIDTH == 4)
    bCount = (bCount > 0x10) ? 0xf : bCount;
#elif (OSD_BIT_WIDTH == 8)
    bCount = (bCount > 0x80) ? 0x7f : bCount; 
#endif
    
    for (i = 0; i < bCount; i++)
    {
		idu->Palette[i] = pdwPalette[i];
		MP_DEBUG("idu->Palette[%d] = 0x%08X", i, idu->Palette[i]);
	}
	//xpgDumpOSDPalette();
#endif

	return TRUE;
}

void Idu_OsdOnOff(BYTE b_Switch)
{
    MP_DEBUG("%s: b_Switch = %d", __func__, b_Switch);
	IDU *idu = (IDU *) IDU_BASE;
	CHANNEL *osd_dma = (CHANNEL *) DMA_OSD_BASE;

#if OSD_ENABLE
	if (b_Switch)
		idu->IduCtrl0 |= 0x00001000;	//OSD_EN = 1
	else
#endif
	idu->IduCtrl0 &= ~0x00001000;	//OSD_EN = 0
}

void Idu_OsdErase()
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	ST_OSDWIN *OSDWin;

	OSDWin = Idu_GetOsdWin();
	if (OSDWin == NULL)
		return;
		
#if SHOW_OSD_TOP_STATUS
	memset((BYTE *) ((DWORD)OSDWin->pdwStart+((OSD_ICON_ACTION_POS_H * OSDWin->wWidth) >> OSD_BIT_OFFSET)), 0, OSDWin->dwBufferSize-((OSDWin->wWidth * OSD_ICON_ACTION_POS_H) >> OSD_BIT_OFFSET));
//	Clear_Action_Icon();
#elif SHOW_USB_STATUS
	memset(OSDWin->pdwStart, 0, OSDWin->dwBufferSize-((OSDWin->wWidth * 56) >> OSD_BIT_OFFSET));
#elif OSD_BATTERY_X
	Idu_OsdPaintArea(0,0,OSD_BATTERY_X,OSD_ICON_ACTION_POS_H,0);
	memset((BYTE *) ((DWORD)OSDWin->pdwStart+((OSD_ICON_ACTION_POS_H * OSDWin->wWidth) >> OSD_BIT_OFFSET)), 0, OSDWin->dwBufferSize-((OSDWin->wWidth * OSD_ICON_ACTION_POS_H) >> OSD_BIT_OFFSET));
#else
	memset(OSDWin->pdwStart, 0, OSDWin->dwBufferSize);
#endif
#endif
}


void Idu_OsdPutRectangle(ST_OSDWIN * win, WORD x, WORD y, WORD width, WORD height, WORD lineW, BYTE color)
{ // draw internal Rectangle of x/y/w/h
    MP_DEBUG("%s: x = %d, y = %d, w = %d, h = %d, color = %d", __func__, x, y, width, height, color);
#if OSD_ENABLE
	Idu_OsdPaintArea(x, y, width, lineW, color);
	Idu_OsdPaintArea(x, (y + height - lineW), width, lineW, color);
	Idu_OsdPaintArea(x, y, lineW, height, color);
	Idu_OsdPaintArea((x + width - lineW), y, lineW, height, color);
#endif
}


void Idu_OSDInit(DWORD * pdwBuffer, int width, int height, BYTE bInterlace, BYTE bitWidth, BOOL ClearOsd)
{
    MP_DEBUG("%s", __func__);
#if OSD_ENABLE
	Idu_OSDWinInit(&sWinOSD, (DWORD *) pdwBuffer, width, height, bInterlace, bitWidth);
	Idu_OsdDMAInit(&sWinOSD);
#if OSD_DISPLAY_CACHE
	Idu_OSDWinInit(&sOsdCacheWin,(DWORD *) SystemGetMemAddr(OSD_BUF_MEM_ID1), width, height, bInterlace, bitWidth);
	memset(sOsdCacheWin.pdwStart, 0, sOsdCacheWin.dwBufferSize);
#endif
	Idu_OsdSetDefaultPalette();
    Idu_OsdDMAOn();
    Idu_OsdErase();
#endif
}


SWORD Idu_OsdPutChar_4X(ST_OSDWIN * psWin, ST_FONT * sFont, WORD wData, BYTE MappingTable)
{
#if OSD_ENABLE
	DWORD dwOffset;	
	register DWORD j;
	register BYTE *pbStartAddr;
	register BYTE *bitmap;
	register BYTE bMask = 0,temp;
	BYTE bWidth,bHeight,balign,bSingleBit,bExtraBit;
	SBYTE temp1;
	register BYTE bPixel;

	if (!(OsdFontIsNewTabFont() && (OsdFontGetBitPerPixel()==4)))
		return FAIL;
	if(wData == ' ')
	{
		sFont->wX += (OsdFontGetBlankWidth() + sFont->bTextGap);	//	write space
		sFont->wDisplayWidth += (OsdFontGetBlankWidth() + sFont->bTextGap);
		return PASS;
	}
		
	bitmap =(BYTE *)OsdFontGetCharBitmapAddr(wData);
	if (bitmap == NULL)
	{
		bitmap =(BYTE *)OsdFontGetCharBitmapAddr(0x3F);
		if (bitmap == NULL) return;
	}
	bWidth = *bitmap;
	if (bWidth > 64) return;
	if(sFont->wX & 0x01)
		bSingleBit = 1;
	else
		bSingleBit = 0;
	if((bWidth - bSingleBit) & 0x01)
		bExtraBit = 1;
	else
		bExtraBit = 0;
	balign = OsdFontGetFontAlignment();
	psWin->boPainted = TRUE;	// abel 2006.12.12

	sFont->wDisplayWidth += bWidth;
	bitmap++;
	bHeight = *bitmap;//Lighter - 1; //Mason 1/12
	bitmap++;

	pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + ((sFont->wY * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));

#if (OSD_BIT_WIDTH == 2)
	dwOffset = ((psWin->wWidth - ALIGN_4(bWidth)) >> OSD_BIT_OFFSET) + 1;
#elif (OSD_BIT_WIDTH == 4)
	dwOffset = (psWin->wWidth - ALIGN_2(bWidth+bSingleBit)) >> OSD_BIT_OFFSET;
#else
	dwOffset = (psWin->wWidth - bWidth) >> OSD_BIT_OFFSET;
#endif
	while (bHeight--)
	{
		j = bWidth;
		#if (OSD_BIT_WIDTH == 2)
		temp = 6;
		#elif (OSD_BIT_WIDTH == 4)
		temp = 4;
		#elif (OSD_BIT_WIDTH == 8)
		temp = 0;
		#endif
		if (balign)
			bMask = 0;
		while (j--)
		{
			if (!bMask)
			{
				bMask = 0xf0;
				bPixel = *bitmap++;
			}

			if ((bSingleBit)&&(j==bWidth-1))
			{
				#if (OSD_BIT_WIDTH == 2)
				temp -= 2;
				#elif (OSD_BIT_WIDTH == 4)
				temp -= 4;
				#endif
			}

			#if (OSD_BIT_WIDTH == 4) && OSD_4BIT_FONT_ENABLE
			if (bPixel & bMask)
			{
				*(pbStartAddr) &= (~(0x0f << temp));
				if (bMask&0xf0)
					*(pbStartAddr) |= ((bPixel & bMask)>>4<<temp);
				else
					*(pbStartAddr) |= ((bPixel & bMask)<<temp);
			}
			#else
			if ((bPixel & bMask)>(bMask>>1))
			{
				#if (OSD_BIT_WIDTH == 2)
				*(pbStartAddr) &= (~(0x03 << temp));
				*(pbStartAddr) |= ((sFont->bFontColor & 0x03) << temp);
				#elif (OSD_BIT_WIDTH == 4)
				*(pbStartAddr) &= (~(0x0f << temp));
				*(pbStartAddr) |= ((sFont->bFontColor & 0x0f) << temp);
				#elif (OSD_BIT_WIDTH == 8)
				*(pbStartAddr) = (sFont->bFontColor & 0x7f);
				#endif
			}
			#endif
			bMask >>= 4;
			
			if ((bExtraBit)&&(j==0))
				temp = 0;

			if (temp == 0)
			{
			#if (OSD_BIT_WIDTH == 2)
				temp = 6;
				if (j)
			#elif (OSD_BIT_WIDTH == 4)
				temp = 4;
			#endif
					pbStartAddr ++;
			}
			else 
			#if (OSD_BIT_WIDTH == 2)
				temp -= 2;
			#elif (OSD_BIT_WIDTH == 4)
				temp -= 4;
			#endif
		}
		pbStartAddr += dwOffset;
	}

	sFont->wX += bWidth + sFont->bTextGap;
	sFont->wDisplayWidth += sFont->bTextGap;//Add by Tech 20091126

	return PASS;
#endif
}


void Idu_OsdPutChar_1X(ST_OSDWIN * psWin, ST_FONT * sFont, WORD wData, BYTE MappingTable)
{
#if OSD_ENABLE
	WORD wWidth, wHeight, j;
	BYTE *Pixel, *pbStartAddr, *bitmap, bMask = 0;

    sFont->bFontColor &= OSD_MASK;
    if ((sFont->wX > psWin->wWidth) || (sFont->wY > psWin->wHeight) || (psWin->pdwStart == NULL))
        return;
#if OSD_USE_NEW_FONT
	if (Idu_OsdPutChar_4X(psWin, sFont, wData, MappingTable)==PASS)
		return PASS;
#endif
    
	if(wData == ' ')
	{
		sFont->wX += (OsdFontGetBlankWidth() + sFont->bTextGap);	//	write space
		sFont->wDisplayWidth += (OsdFontGetBlankWidth() + sFont->bTextGap);
		return;
	}	
	
	bitmap =(BYTE *)OsdFontGetCharBitmapAddr(wData);
	if (bitmap == NULL)
	{
		bitmap =(BYTE *)OsdFontGetCharBitmapAddr(0x3F);
		if (bitmap == NULL) return;
	}

	wWidth = *bitmap ++;
	wHeight = *bitmap;
	if (wWidth > 64) return;
	
	psWin->boPainted = TRUE;
	sFont->wDisplayWidth += wWidth;

    if (sFont->wX + wWidth > psWin->wWidth)
        wWidth = psWin->wWidth - sFont->wX;
    if (sFont->wY + wHeight > psWin->wHeight)
        wHeight = psWin->wHeight - sFont->wY;
    
	pbStartAddr = Pixel= 
        (BYTE*)psWin->pdwStart + ((sFont->wY * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET);

	while (wHeight)
	{
		j = 0;
		while (j < wWidth)
		{
			if (!bMask)
			{
				bMask = 0x80;
				bitmap ++;
				while ((j + 8 < wWidth) && (*bitmap == 0))
				{
					Pixel += 4;
					bitmap ++;
					j += 8;
				}
			}
            if (*bitmap & bMask)
            {
    			if ((sFont->wX + j) & 0x1)
    			    *Pixel = (*Pixel & 0xf0) + sFont->bFontColor;
    			else
    			    *Pixel = (*Pixel & 0x0f) + (sFont->bFontColor << 4);
            }
			bMask >>= 1;
            j ++;
   			if (!((sFont->wX + j) & 0x1) && (j < wWidth))
   		    	Pixel ++;
		}
    	pbStartAddr = Pixel = pbStartAddr + (psWin->wWidth >> OSD_BIT_OFFSET);
        wHeight --;
	}
	sFont->wDisplayWidth += sFont->bTextGap;
	sFont->wX += (wWidth + sFont->bTextGap); 
#endif
}

#if OSD_BMP

typedef struct
{
	BOOL	boCached;
	STREAM*	handle;
	const BYTE*	buffer;
	DWORD	dwSize;
	BOOL	boUseTransprt;
	DWORD	dwX;
	DWORD	dwY;
	DWORD	dwCurPos;
}ST_OsdBmpParser;
static void InitOsdBmpParser(ST_OsdBmpParser*, STREAM*, const BYTE*, DWORD, DWORD, DWORD, BOOL);
static int ParseAndDrawBmpToOsd(ST_OsdBmpParser*);

//------------------------------------------------------------------------------
///
///@ingroup Osd_DrawBmpByHandle
///
///@brief   Draw a bitmap format photo (*.bmp) to OSD Win
///         The BMP file must be 2/16/256 color, not support TrueColor
///
///@param  handle           - The handle of BMP file 
///@param  X                - The X coordinate of BMP print to
///@param  Y                - The Y coordinate of BMP print to
///@param  boCoverPalette   - Whether copy BMP's palette to OSD's palette
///@param  blend            - Define the OSD's palette blend, only valid when boCoverPalette==true
///@param  boEnableTransprt - If true, the color OSD_TRANSPARENT_RGB will be transparent
///

int Osd_DrawBmpByHandle(STREAM* handle, DWORD X, DWORD Y, BOOL boCoverPalette, BYTE blend, BOOL boEnableTransprt)
{
	int iRet = 0;
	DWORD dwFileSize, dwPos;
	ST_OsdBmpParser	parser;

	if ( handle == NULL ) {
        mpDebugPrint("%s() : handle == NULL", __FUNCTION__);
		return FAIL;
	}
	
	dwFileSize = FileSizeGet(handle);
	dwPos = FilePosGet(handle);
	if ( dwPos != 0 )
		SeekSet(handle);
	if ( dwFileSize+16*1024 < ext_mem_get_free_space())
	{
		BYTE *buffer = (BYTE*) ext_mem_malloc(dwFileSize+32);

		if ( buffer == NULL )
		{
			MP_ALERT("no memory");
			return FAIL;
		}
		FileRead(handle, buffer, dwFileSize);
		iRet = Osd_DrawBmpByBuffer(buffer, dwFileSize, X, Y, boCoverPalette, blend, boEnableTransprt);
		ext_mem_free(buffer);
		return iRet;
	}
	else
	{
		if ( boCoverPalette )
		{
			iRet = OSD_LoadPaletteFromBmpFile(handle, blend);
			if ( iRet != PASS )
				return FAIL;
		}
		dwPos = FilePosGet(handle);
		if ( dwPos != 0 )
			SeekSet(handle);
		InitOsdBmpParser(&parser, handle, NULL, dwFileSize, X, Y, boEnableTransprt);
		return ParseAndDrawBmpToOsd(&parser);
	}
}

//------------------------------------------------------------------------------
///
///@ingroup Osd_DrawBmpByBuffer
///
///@brief   Draw a bitmap format photo (*.bmp) to OSD Win
///         The BMP file must be 2/16/256 color, not support TrueColor
///
///@param  buffer           - The BMP buffer (the BMP have been load to this buffer)
///@param  dwBufSize        - The buffer size in bytes
///@param  X                - The X coordinate of BMP print to
///@param  Y                - The Y coordinate of BMP print to
///@param  boCoverPalette   - Whether copy BMP's palette to OSD's palette
///@param  blend            - Define the OSD's palette blend, only valid when boCoverPalette==true
///@param  boEnableTransprt - If true, the color OSD_TRANSPARENT_RGB will be transparent
///

int Osd_DrawBmpByBuffer(const BYTE* buffer, DWORD dwBufSize, DWORD X, DWORD Y, BOOL boCoverPalette, BYTE blend, BOOL boEnableTransprt)
{
	int iRet;
	ST_OsdBmpParser	parser;

	if ( buffer==NULL || dwBufSize==0 )
		return FAIL;

	if ( boCoverPalette )
	{
		iRet = OSD_LoadPaletteFromBmpBuffer(buffer, dwBufSize, blend);
		if ( iRet != PASS )
			return FAIL;
	}
	
	InitOsdBmpParser(&parser, NULL, buffer, dwBufSize, X, Y, boEnableTransprt);
	return ParseAndDrawBmpToOsd(&parser);
}

//------------------------------------------------------------------------------
///
///@ingroup OSD_LoadPaletteFromBmpFile
///
///@brief   Copy BMP's palette to OSD's palette
///
///@param  handle           - The handle of BMP file 
///@param  blend            - Define the OSD's palette blend, the blend value range : 0x00~0x0f
///

int OSD_LoadPaletteFromBmpFile(STREAM* handle, BYTE blend)
{
	IDU *idu = (IDU *) IDU_BASE;
	WORD wBMFlag, dwBitsPerPx;
	DWORD dwPos, dwHeadSize, Colors, dwColorFull;
	DWORD dwRealColor;
	DWORD *pdwColors, i, color;

    if (handle == NULL) {
        mpDebugPrint("%s() : handle == NULL", __FUNCTION__);
        return FAIL;
    }
    
	dwPos = FilePosGet(handle);
	if ( dwPos != 0 )
		SeekSet(handle);
	FileRead(handle, (BYTE*)(&wBMFlag), 2);
	if ( wBMFlag != 0x424D )
	{
		mpDebugPrint("not bmp format");
		return FAIL;
	}
	Fseek(handle, 12, SEEK_CUR);
	FileRead(handle, (BYTE*)(&dwHeadSize), 4);
	dwHeadSize = swap32(dwHeadSize);
	Fseek(handle, 10, SEEK_CUR);
	FileRead(handle, (BYTE*)(&dwBitsPerPx), 2);
	dwBitsPerPx = swap16(dwBitsPerPx);
	Fseek(handle, 16, SEEK_CUR);
	FileRead(handle, (BYTE*)(&Colors), 4);
	Colors = swap32(Colors);
	mpDebugPrint("dwHeadSize = %d, dwBitsPerPx = %d, Colors = %d", dwHeadSize, dwBitsPerPx, Colors);
	
	if ( dwBitsPerPx != 1 && dwBitsPerPx != 4 && dwBitsPerPx != 8)
	{
		mpDebugPrint("Not support TrueColor bmp");
		return FAIL;
	}
	dwColorFull = 1 << dwBitsPerPx;
	if (Colors)
		dwRealColor = (Colors < dwColorFull) ? Colors : dwColorFull;
	else
		dwRealColor = dwColorFull;
	if ( dwRealColor > 0x7f )
		dwRealColor = 0x7f;
	if ( dwHeadSize+0x0E >= FileSizeGet(handle))
	{
		mpDebugPrint("BMP mangle");
		return FAIL;
	}

	Fseek(handle, dwHeadSize+0x0E, SEEK_SET);
	pdwColors = (DWORD*) ext_mem_malloc(dwRealColor*4 +16);
	if (pdwColors==NULL)
	{
		mpDebugPrint("not mem");
		return FAIL;
	}
	FileRead(handle, (BYTE*)pdwColors, dwRealColor*4);
	idu->Palette[0] = 0;
	for ( i = 0; i < dwRealColor; i++ )
	{
		color = swap32(pdwColors[i]);
		color &= 0x00ffffff;
		color |= (blend<<24);
		idu->Palette[i+1] = color;
	}
	ext_mem_free(pdwColors);
	return PASS;
}

//------------------------------------------------------------------------------
///
///@ingroup OSD_LoadPaletteFromBmpFile
///
///@brief   Copy BMP's palette to OSD's palette
///
///@param  buffer           - The buffer of BMP 
///@param  dwBufSize        - The buffer size
///@param  blend            - Define the OSD's palette blend, the blend value range : 0x00~0x0f
///

int OSD_LoadPaletteFromBmpBuffer(const BYTE* buffer, DWORD dwBufSize, BYTE blend)
{
	IDU *idu = (IDU *) IDU_BASE;
	WORD dwBitsPerPx;
	DWORD dwPos, dwHeadSize, Colors, dwColorFull;
	DWORD dwRealColor;
	const BYTE *pbColors;
	DWORD i, color;

	if ( buffer[0]!='B' && buffer[1]!='M' )
	{
		mpDebugPrint("not bmp format");
		return FAIL;
	}
	memcpy(&dwHeadSize, &buffer[0x0e], 4);
	dwHeadSize = swap32(dwHeadSize);
	memcpy(&dwBitsPerPx, &buffer[0x1c], 2);
	dwBitsPerPx = swap16(dwBitsPerPx);
	memcpy(&Colors, &buffer[0x2e], 4);
	Colors = swap32(Colors);

	mpDebugPrint("dwHeadSize = %d, dwBitsPerPx = %d, Colors = %d", dwHeadSize, dwBitsPerPx, Colors);
	if ( dwBitsPerPx != 1 && dwBitsPerPx != 4 && dwBitsPerPx != 8)
	{
		mpDebugPrint("Not support TrueColor bmp");
		return FAIL;
	}
	dwColorFull = 1 << dwBitsPerPx;
	if (Colors)
		dwRealColor = (Colors < dwColorFull) ? Colors : dwColorFull;
	else
		dwRealColor = dwColorFull;
	
#if (OSD_BIT_WIDTH == 8)
	if ( dwRealColor > 0x7f )
		dwRealColor = 0x7f;
#elif (OSD_BIT_WIDTH == 4)
	if ( dwRealColor > 0x0f )
		dwRealColor = 0x0f;
#elif (OSD_BIT_WIDTH == 2)
	if ( dwRealColor > 0x03 )
		dwRealColor = 0x03;
#endif

	if ( dwHeadSize+0x0E >= dwBufSize)
	{
		mpDebugPrint("BMP mangle");
		return FAIL;
	}

	pbColors = &buffer[dwHeadSize+0x0E];
	idu->Palette[0] = 0;
	for ( i = 0; i < dwRealColor; i++ )
	{
		memcpy(&color, pbColors, 4);
		color = swap32(color);
		color &= 0x00ffffff;
		color |= (blend<<24);
		idu->Palette[i+1] = color;
		pbColors += 4;
	}
	return PASS;
}

static void InitOsdBmpParser(ST_OsdBmpParser* pstParser, STREAM* handle, const BYTE* buf, DWORD dwBufSize, 
							DWORD X, DWORD Y, BOOL boEnableTransprt)
{
	if ( handle != NULL )
	{
		pstParser->boCached= FALSE;
		pstParser->handle = handle;
		pstParser->buffer = NULL;
	}
	else if ( buf != NULL )
	{
		pstParser->boCached = TRUE;
		pstParser->handle = NULL;
		pstParser->buffer = buf;
	}
	
	pstParser->dwSize = dwBufSize;
	pstParser->boUseTransprt = boEnableTransprt;
	pstParser->dwCurPos = 0;
	pstParser->dwX = X;
	pstParser->dwY = Y;
	return;
}

static void BmpParserReadFile(ST_OsdBmpParser* pstParser, BYTE* Buf, DWORD dwReadSize)
{
	FileRead(pstParser->handle, Buf, dwReadSize);
	pstParser->dwCurPos = FilePosGet(pstParser->handle);
}

static void BmpParserReadBuff(ST_OsdBmpParser* pstParser, BYTE* Buf, DWORD dwReadSize)
{
	memcpy(Buf, &pstParser->buffer[pstParser->dwCurPos], dwReadSize);
	pstParser->dwCurPos += dwReadSize;
}

static void BmpParserSeekFile(ST_OsdBmpParser* pstParser, DWORD offset, DWORD whence)
{
	Fseek(pstParser->handle, offset, whence);
	pstParser->dwCurPos = FilePosGet(pstParser->handle);
}

static void BmpParserSeekBuff(ST_OsdBmpParser* pstParser, DWORD offset, DWORD whence)
{
	switch (whence)
	{
		case SEEK_SET:
			pstParser->dwCurPos = offset;
			break;
		case SEEK_CUR:
			pstParser->dwCurPos += offset;
			break;
		case SEEK_END:
			pstParser->dwCurPos = pstParser->dwSize -1;
			pstParser->dwCurPos -= offset;
			break;
	}
}

static int ParseAndDrawBmpToOsd(ST_OsdBmpParser* pstParser)
{
	ST_OSDWIN *pWin;
	IDU *idu = (IDU *) IDU_BASE;
	DWORD dwBegin;				// BMP - begining offset of data
	DWORD dwBmpWidth;			// BMP - BMP width
	DWORD dwBmpHeight;			// BMP - BMP height
	DWORD dwZiped;				// BMP - Compression specifications
	DWORD dwBmpColors;			// BMP - Number of colors used by this bitmap
	WORD  wNumOfPlanes;			// BMP - Number of planes in this bitmap
	WORD  wBitsPerPixel;		// BMP - Bits per pixel used to store palette entry information
	WORD  wBMFlag;				// BMP - BMP flag
	short swLineCnt;
	void (*read)(ST_OsdBmpParser*,BYTE*,DWORD);
	void (*seek)(ST_OsdBmpParser*,DWORD,DWORD);
	DWORD dwPrintWidth, dwPrintHeight;	//need print width & height
	DWORD dwBmpLineByte, dwLineByte;
	DWORD dwBmpLineOffset;		// 
	DWORD i, j, dwTransprtRgb;
	BYTE  *bBmpBuf, *pbAddr;
	BYTE  bIndex;

	if ( pstParser->boCached )
	{
		read = BmpParserReadBuff;
		seek = BmpParserSeekBuff;
	}
	else
	{
		read = BmpParserReadFile;
		seek = BmpParserSeekFile;
	}

	read(pstParser, (BYTE*)(&wBMFlag), 2);
	if ( wBMFlag != 0x424D )
	{
		mpDebugPrint("not bmp");
		return FAIL;
	}
	seek(pstParser, 8, SEEK_CUR);
	read(pstParser, (BYTE*)(&dwBegin), 4);
	dwBegin = swap32(dwBegin);
	if ( dwBegin >= pstParser->dwSize )
	{
		mpDebugPrint("BMP mangle");
		return FAIL;
	}
	seek(pstParser, 4, SEEK_CUR);
	read(pstParser, (BYTE*)(&dwBmpWidth), 4);		// read widhth
	dwBmpWidth = swap32(dwBmpWidth);
	read(pstParser, (BYTE*)(&dwBmpHeight), 4);		// read height
	dwBmpHeight = swap32(dwBmpHeight);
	
	read(pstParser, (BYTE*)(&wNumOfPlanes), 2);
	wNumOfPlanes = swap16(wNumOfPlanes);
	if ( wNumOfPlanes != 1 )
	{
		mpDebugPrint("BMP not support");
		return FAIL;
	}
	
	read(pstParser, (BYTE*)(&wBitsPerPixel), 2);
	wBitsPerPixel = swap16(wBitsPerPixel);
	if ( wBitsPerPixel != 1 && wBitsPerPixel != 4 && wBitsPerPixel != 8)
	{
		mpDebugPrint("Not support TrueColor bmp");
		return FAIL;
	}
	read(pstParser, (BYTE*)(&dwZiped), 4);
	dwZiped = swap32(dwZiped);
	if ( dwZiped )
	{
		mpDebugPrint("BMP not support");
		return FAIL;
	}
	seek(pstParser, 12, SEEK_CUR);
	read(pstParser, (BYTE*)(&dwBmpColors), 4);
	dwBmpColors = swap32(dwBmpColors);

	pWin = Idu_GetOsdWin();
	if ( pstParser->dwX + dwBmpWidth > pWin->wWidth )
		dwPrintWidth = pWin->wWidth - pstParser->dwX;
	else
		dwPrintWidth = dwBmpWidth;
	if ( pstParser->dwY + dwBmpHeight > pWin->wHeight )
		dwPrintHeight = pWin->wHeight- pstParser->dwY;
	else
		dwPrintHeight = dwBmpHeight;

	if ( wBitsPerPixel == 1 )
	{
		dwBmpLineOffset = dwBmpWidth/8;
		if ( dwBmpWidth%8 )
			dwBmpLineOffset++;
		dwBmpLineByte = dwPrintWidth/8;
		if ( dwPrintWidth%8 )
			dwBmpLineByte++;
		
	}
	else if ( wBitsPerPixel == 4 )
	{
		dwBmpLineOffset = dwBmpWidth/2;
		if ( dwBmpWidth%2 )
			dwBmpLineOffset++;
		dwBmpLineByte = dwPrintWidth/2;
		if ( dwPrintWidth%2 )
			dwBmpLineByte++;
	}
	else if ( wBitsPerPixel == 8 )
	{
		dwBmpLineOffset = dwBmpWidth;
		dwBmpLineByte = dwPrintWidth;
	}
	if ( dwBmpLineOffset % 4 )
		dwBmpLineOffset += (4 - (dwBmpLineOffset % 4));
	//
	if ( dwPrintHeight == dwBmpHeight )
		seek(pstParser, dwBegin, SEEK_SET);
	else
		seek(pstParser, dwBegin+(dwBmpHeight-dwPrintHeight)*dwBmpLineOffset, SEEK_SET);

	bBmpBuf = (BYTE*) ext_mem_malloc(dwBmpLineOffset+32);
	if ( bBmpBuf==NULL ) {
		MP_ALERT("no memory");
		return FAIL;
	}

#ifdef OSD_TRANSPARENT_RGB
	dwTransprtRgb = OSD_TRANSPARENT_RGB;		// the transparent color RGB
#else
	dwTransprtRgb = 0;
#endif

	pWin->boPainted = 1;//add by Tech20101225
	
#if (OSD_BIT_WIDTH == 8)
	dwLineByte = dwPrintWidth;
	pbAddr = (BYTE*)pWin->pdwStart + (pstParser->dwX + (pstParser->dwY + dwPrintHeight -1) * pWin->wWidth);
	if ( wBitsPerPixel == 8 )
	{
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			for ( i = 0 ; i < dwLineByte; i++ )
			{
				bIndex = (bBmpBuf[i] + 1) & 0x7f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					pbAddr[i] = 0;
				else
					pbAddr[i] = bIndex;
			}
			pbAddr -= pWin->wWidth;
		}
	}
	else if ( wBitsPerPixel == 4 )
	{
		BYTE  mask[] = {0xf0, 0x0f};
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			for ( i = 0 ; i < dwLineByte; i++ )
			{
				bIndex = bBmpBuf[i/2];
				bIndex &= mask[i%2];
				if ( i % 2 == 0 )
					bIndex >>= 4;
				bIndex += 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					pbAddr[i] = 0;
				else
					pbAddr[i] = bIndex;
			}
			pbAddr -= pWin->wWidth;
		}
	}
	else if ( wBitsPerPixel == 1 )
	{
		BYTE  mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			for ( i = 0 ; i < dwLineByte; i++ )
			{
				bIndex = bBmpBuf[i/8];
				bIndex &= mask[i%8];
				bIndex = bIndex ? 2 : 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					pbAddr[i] = 0;
				else
					pbAddr[i] = bIndex;
			}
			pbAddr -= pWin->wWidth;
		}
	}
#elif (OSD_BIT_WIDTH == 4)

	BYTE boStartOdd, boEndOdd;			// the flag mean the start/end X coordinate is odd number
	boStartOdd = (pstParser->dwX %2) ? 1 : 0;
	boEndOdd = ((pstParser->dwX + dwPrintWidth)%2) ? 1 : 0;
	pbAddr = (BYTE*)pWin->pdwStart + (pstParser->dwX + (pstParser->dwY + dwPrintHeight -1) * pWin->wWidth)/2;
	if ( wBitsPerPixel == 4 )
	{
		BYTE  bIndex2, mask[] = {0xf0, 0x0f};
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			i = j = 0;					// i == pbAddr counter; j == point counter
			if ( boStartOdd )
			{
				bIndex2 = bBmpBuf[0] >> 4;
				bIndex2 = (bIndex2 + 1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				pbAddr[i] = (pbAddr[i] & 0xf0) | bIndex2;
				i++; j++;
			}
			for ( ; j < dwPrintWidth-boStartOdd-boEndOdd; j+=2 )
			{
				bIndex = bBmpBuf[j/2] & mask[j%2];
				if ( j % 2 == 0 )
					bIndex >>= 4;
				bIndex = (bIndex+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				//----
				bIndex2 = bBmpBuf[(j+1)/2] & mask[(j+1)%2];
				if ( (j+1) % 2 == 0 )
					bIndex2 >>= 4;
				bIndex2 = (bIndex2+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				//----
				pbAddr[i] = (bIndex<<4) | bIndex2;
				i++;
			}
			if ( boEndOdd )
			{
				bIndex = bBmpBuf[j/2] & mask[j%2];
				if ( j % 2 == 0 )
					bIndex >>= 4;
				bIndex = (bIndex+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				pbAddr[i] = (bIndex<<4) | (pbAddr[i]& 0x0f);
				//i++; j++;
			}
			pbAddr -= pWin->wWidth/2;
		}
	}
	else if ( wBitsPerPixel == 8 )
	{
		BYTE  bIndex2;
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			i = j = 0;					// i == pbAddr counter; j == point counter
			if ( boStartOdd )
			{
				bIndex2 = (bBmpBuf[j] + 1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				pbAddr[i] = (pbAddr[i] & 0xf0) | bIndex2;
				i++; j++;
			}
			for ( ; j < dwPrintWidth-boStartOdd-boEndOdd; j+=2 )
			{
				bIndex = (bBmpBuf[j]+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				//----
				bIndex2 = (bBmpBuf[j+1]+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				//----
				pbAddr[i] = (bIndex<<4) | bIndex2;
				i++;
			}
			if ( boEndOdd )
			{
				bIndex = (bBmpBuf[j]+1) & 0x0f;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				pbAddr[i] = (bIndex<<4) | (pbAddr[i]& 0x0f);
				//i++; j++;
			}
			pbAddr -= pWin->wWidth/2;
		}
	}
	else if ( wBitsPerPixel == 1 )
	{
		BYTE  bIndex2;
		BYTE  mask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
		for ( swLineCnt = dwPrintHeight-1; swLineCnt >=0 ; swLineCnt-- )
		{
			read(pstParser, bBmpBuf, dwBmpLineOffset);
			i = j = 0;					// i == pbAddr counter; j == point counter
			if ( boStartOdd )
			{
				bIndex2 = bBmpBuf[j/8] & mask[j%8];
				bIndex2 = bIndex2 ? 2 : 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				pbAddr[i] = (pbAddr[i] & 0xf0) | bIndex2;
				i++; j++;
			}
			for ( ; j < dwPrintWidth-boStartOdd-boEndOdd; j+=2 )
			{
				bIndex = bBmpBuf[j/8] & mask[j%8];
				bIndex = bIndex ? 2 : 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				//----
				bIndex2 = bBmpBuf[(j+1)/8] & mask[(j+1)%8];
				bIndex2 = bIndex2 ? 2 : 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex2])&0xffffff)==dwTransprtRgb)
					bIndex2 = 0;
				//----
				pbAddr[i] = (bIndex<<4) | bIndex2;
				i++;
			}
			if ( boEndOdd )
			{
				bIndex = bBmpBuf[j/8] & mask[j%8];
				bIndex = bIndex ? 2 : 1;
				if (pstParser->boUseTransprt && ((idu->Palette[bIndex])&0xffffff)==dwTransprtRgb)
					bIndex = 0;
				pbAddr[i] = (bIndex<<4) | (pbAddr[i]& 0x0f);
				//i++; j++;
			}
			pbAddr -= pWin->wWidth/2;
		}

	}
#elif (OSD_BIT_WIDTH == 2)
	#error - NOT SUPPORT 2-Bit OSD to BMP
#else
    #error - OSD BIT WIDTH NOT SUPPORT
#endif

	ext_mem_free(bBmpBuf);

	return PASS;
}

/*
int OsdBMP_TEST()
{
	STREAM* handle;
	DRIVE* drv;

	drv = DriveGet(NAND);
#if 0
	if(FileSearch(drv, "QQQ1", "BMP", E_FILE_TYPE) != FS_SUCCEED)
	{
		return;
	}
	handle = FileOpen(drv);
	//Osd_DrawBmpByHandle(handle, 600, 400, true, 0xf, FALSE);
	Osd_DrawBmpByHandle(handle, 10, 5, true, 0xf, FALSE);
	FileClose(handle);
#else
	if(FileSearch(drv, "COLOR", "BMP", E_FILE_TYPE) != FS_SUCCEED)
		return;
	handle = FileOpen(drv);
	OSD_LoadPaletteFromBmpFile(handle, 0x0f);
	FileClose(handle);
	
	if(FileSearch(drv, "1A", "BMP", E_FILE_TYPE) != FS_SUCCEED)
		return;
	handle = FileOpen(drv);
	Osd_DrawBmpByHandle(handle, 20, 120, false, 0xf, FALSE);
	FileClose(handle);

	if(FileSearch(drv, "2A", "BMP", E_FILE_TYPE) != FS_SUCCEED)
		return;
	handle = FileOpen(drv);
	Osd_DrawBmpByHandle(handle, 20, 280, false, 0xf, FALSE);
	FileClose(handle);
#endif
}*/


#endif


#if THE_OSD_ICON

void Idu_OsdPaintAreaImage(WORD startX, WORD startY, WORD SizeX, WORD SizeY,
					  const BYTE  *pImgBuffer, register BYTE b_PaletteIndex, BOOL Mark)
{
#if OSD_ENABLE
	WORD w_Width, w_Height, w_Offset, i = 0;
	register BYTE *Pixel;
	register ST_OSDWIN *OSDWin;
	BYTE j, bImgBuffer;
	OSDWin = Idu_GetOsdWin();
	if (OSDWin == NULL)
		return;
	if (SizeX%8) 
		SizeX=((SizeX/8)+1)*8;
	OSDWin->boPainted = TRUE;
	OSDWin->wX = startX;
	OSDWin->wY = startY;
	if ((OSDWin->pdwStart == NULL) || (OSDWin->wX >= OSDWin->wWidth)
		|| (OSDWin->wY >= OSDWin->wHeight))
		return;

	if ((OSDWin->wX + SizeX) > OSDWin->wWidth)
		SizeX = OSDWin->wWidth - OSDWin->wX;

	if ((OSDWin->wY + SizeY) > OSDWin->wHeight)
		SizeY = OSDWin->wHeight - OSDWin->wY;

	w_Height = SizeY;
	if(SizeX & 1) SizeX--;

	Pixel = (BYTE *) OSDWin->pdwStart + (OSDWin->wY * (OSDWin->wWidth >> OSD_BIT_OFFSET)) +
		(OSDWin->wX >> OSD_BIT_OFFSET);
	
	j=8;
	while (w_Height)
	{
		w_Width = SizeX;
		while (w_Width)
		{
			bImgBuffer = (*pImgBuffer);
			j--;
			bImgBuffer = bImgBuffer >> j;
			
			if ((startX + i) & 0x1)
			{
			    #if 0//marco 20080220
				if(((bImgBuffer & 0x01) == 0x00))
					*Pixel = (*Pixel & 0xf0);
				else	
					*Pixel = (*Pixel & 0xf0) + b_PaletteIndex; 
                         #else
				if (Mark)
				{
					if(((bImgBuffer & 0x01) != 0x00))
						*Pixel = (*Pixel & 0xf0) + b_PaletteIndex;
					else
						*Pixel = (*Pixel & 0xf0) ;
	

				}
				else
				{
	                         if(((bImgBuffer & 0x01) != 0x00))
	                            *Pixel = (*Pixel & 0xf0) + b_PaletteIndex;
				}
			    #endif	
				Pixel++;
			}
			else
			{
			       #if 0//marco 20080220
				if(((bImgBuffer & 0x01) == 0x00))
					*Pixel = (*Pixel & 0x0f);	
				else	
					*Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
                            #else
				if (Mark)
				{
	                            if(((bImgBuffer & 0x01) != 0x00))
	                                *Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
					else
						*Pixel = (*Pixel & 0x0f) ;
				}
				else
				{
	                            if(((bImgBuffer & 0x01) != 0x00))
	                                *Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
				}
                            #endif
			}
			
			if(j==0)
			{
				j=8;
				pImgBuffer++;
			}
			
			i++;
			w_Width--;
		}

		Pixel += ((OSDWin->wWidth - SizeX) >> OSD_BIT_OFFSET);
		w_Height--;
		i = 0;
	}
#endif
}

void Idu_OsdPaintAreaLine(WORD startX, WORD startY, WORD SizeX, WORD SizeY,
					   register BYTE b_PaletteIndex)
{
#if OSD_ENABLE
	WORD w_Width, w_Height, w_Offset, i = 0;
	register BYTE *Pixel;
	register ST_OSDWIN *OSDWin;
	BYTE j, bImgBuffer;
	OSDWin = Idu_GetOsdWin();
	if (OSDWin == NULL)
		return;
	if (SizeX%8) 
		SizeX=((SizeX/8)+1)*8;
	OSDWin->boPainted = TRUE;
	OSDWin->wX = startX;
	OSDWin->wY = startY;
	if ((OSDWin->pdwStart == NULL) || (OSDWin->wX >= OSDWin->wWidth)
		|| (OSDWin->wY >= OSDWin->wHeight))
		return;

	if ((OSDWin->wX + SizeX) > OSDWin->wWidth)
		SizeX = OSDWin->wWidth - OSDWin->wX;

	if ((OSDWin->wY + SizeY) > OSDWin->wHeight)
		SizeY = OSDWin->wHeight - OSDWin->wY;

	w_Height = SizeY;
	if(SizeX & 1) SizeX--;

	Pixel = (BYTE *) OSDWin->pdwStart + (OSDWin->wY * (OSDWin->wWidth >> OSD_BIT_OFFSET)) +
		(OSDWin->wX >> OSD_BIT_OFFSET);
	
	j=8;
	while (w_Height)
	{
		w_Width = SizeX;
		while (w_Width)
		{
			j--;
			
			if ((startX + i) & 0x1)
			{
			    #if 0//marco 20080220
				if(((bImgBuffer & 0x01) == 0x00))
					*Pixel = (*Pixel & 0xf0);
				else	
					*Pixel = (*Pixel & 0xf0) + b_PaletteIndex; 
                         #else
                            *Pixel = (*Pixel & 0xf0) + b_PaletteIndex;
			    #endif	
				Pixel++;
			}
			else
			{
			       #if 0//marco 20080220
				if(((bImgBuffer & 0x01) == 0x00))
					*Pixel = (*Pixel & 0x0f);	
				else	
					*Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
                            #else
                                *Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
                            #endif
			}
			
			if(j==0)
			{
				j=8;
			}
			
			i++;
			w_Width--;
		}

		Pixel += ((OSDWin->wWidth - SizeX) >> OSD_BIT_OFFSET);
		w_Height--;
		i = 0;
	}
#endif
}
#endif

void Idu_OsdDrawBar(WORD startX, WORD startY, WORD SegW, WORD SegH,WORD wSegDiff, BYTE bSeg0Num,BYTE bSeg1Num,BYTE bSeg0PaletteIndex,BYTE bSeg1PaletteIndex)
{
#if OSD_ENABLE
	BYTE i;
	WORD wX=startX;

	for (i=0;i<bSeg0Num;i++)
	{
		Idu_OsdPaintArea(wX,startY,SegW,SegH,bSeg0PaletteIndex);
		wX+=SegW+wSegDiff;
	}
	for (i=0;i<bSeg1Num;i++)
	{
		Idu_OsdPaintArea(wX,startY,SegW,SegH,bSeg1PaletteIndex);
		wX+=SegW+wSegDiff;
	}
#endif
}


