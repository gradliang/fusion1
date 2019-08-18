/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "idu.h"
#include "FontDisp.h"

#if (OSD_BIT_WIDTH == 2)

DWORD dwDefaultPalette[3] = { // Palette[1..3], b_Blending = OSD_BLEND_MAX = 0x06
//  0x00RRGGBB
	0x06FF0000, // 6  - OSD_COLOR_RED
	0x0600FF00, // 7  - OSD_COLOR_GREEN
	0x060000FF, // 8  - OSD_COLOR_BLUE
};

BOOL Idu_OsdPaletteInit(BYTE b_Blending)
{
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;
    DWORD dwPalette;
	BYTE Y, Cb, Cr, bIndex;

	if (g_bAniFlag != 0)
		return FALSE;
	if (b_Blending == g_bPrevBlending)
		return TRUE;

	b_Blending &= 0xf;
    idu->Palette[0] = 0;
	idu->Palette[1] = (b_Blending << 24) + (RGB2YUV(255, 0, 0) & 0x00FFFFFF);
	idu->Palette[2] = (b_Blending << 24) + (RGB2YUV(0, 255, 0) & 0x00FFFFFF);
	idu->Palette[3] = (b_Blending << 24) + (RGB2YUV(0, 0, 255) & 0x00FFFFFF);

	g_bPrevBlending = b_Blending;
#endif
    return TRUE;
}

BOOL Idu_OsdChangeBlending(BYTE b_Blending)
{
#if OSD_ENABLE

	IDU *idu = (IDU *) IDU_BASE;
    DWORD dwPalette;
	BYTE Y, Cb, Cr, bIndex;

	if (g_bAniFlag != 0)
		return FALSE;
	if (b_Blending == g_bPrevBlending)
		return TRUE;

	b_Blending &= 0xf;
    idu->Palette[0] = 0;
	idu->Palette[1] = (b_Blending << 24) | idu->Palette[1];
	idu->Palette[2] = (b_Blending << 24) | idu->Palette[2];
	idu->Palette[3] = (b_Blending << 24) | idu->Palette[3];
	g_bPrevBlending = b_Blending;
#endif
    return TRUE;

}


void Idu_OsdPaintArea(WORD startX, WORD startY, WORD SizeX, WORD SizeY, BYTE bColor)
{
#if OSD_ENABLE
	WORD wWidth;
	BYTE *Pixel, *pbStartAddr;
	ST_OSDWIN *OSDWin = Idu_GetOsdWin();
	if ((OSDWin->pdwStart == NULL) || (startX >= OSDWin->wWidth) || (startY >= OSDWin->wHeight))
		return;
	OSDWin->boPainted = TRUE;

	if ((startX + SizeX) > OSDWin->wWidth)
		SizeX = OSDWin->wWidth - startX;
	if ((startY + SizeY) > OSDWin->wHeight)
		SizeY = OSDWin->wHeight - startY;

	bColor &= OSD_MASK;
	pbStartAddr = Pixel =
        (BYTE*)OSDWin->pdwStart + ((startY * OSDWin->wWidth + startX) >> OSD_BIT_OFFSET);

	while (SizeY)
	{
		wWidth = 0;
		if((SizeX < 0x04))
		{
		while (wWidth < SizeX)
		{
			if (((startX + wWidth) & 0x3) == 3)
				*Pixel = (*Pixel & 0xfc) + bColor;
			else if (((startX + wWidth) & 0x3) == 2)
				*Pixel = (*Pixel & 0xf3) + (bColor << 2);
			else if (((startX + wWidth) & 0x3) == 1)
				*Pixel = (*Pixel & 0xcf) + (bColor << 4);
			else
				*Pixel = (*Pixel & 0x3f) + (bColor << 6);
   			wWidth ++;
            if (!((startX + wWidth) & 0x3) && (wWidth < SizeX))
                Pixel ++;
		}
		}
		else  //memset last lens muset big than 4
		{
			bColor |= (bColor << 2)|(bColor << 4)|(bColor << 6);
			mmcp_memset((BYTE *)Pixel, bColor, SizeX>>2);
			if((!(SizeX & 0x04 )))
			{	//SizeX is not 4 bytes alignment.
				for(wWidth=1;wWidth<=4;wWidth++)
				{
					*(Pixel+(SizeX>>1)-wWidth) = bColor;

				}
			}
		}

		pbStartAddr = Pixel = pbStartAddr + (OSDWin->wWidth >> OSD_BIT_OFFSET);
		SizeY --;
	}
#endif
}

void Idu_OsdPaintVColorBar()
{
    MP_ALERT("%s", __func__);
    
    ST_IMGWIN *psWin = Idu_GetCurrWin();
    WORD w = psWin->wWidth / 16;
    WORD h = psWin->wHeight;
    BYTE i;
    for(i=0; i<16; i++)
    {
        Idu_OsdPaintArea(i*w, 0, w, h, i);
    }
 }
 
#endif//#if (OSD_BIT_WIDTH == 2)


