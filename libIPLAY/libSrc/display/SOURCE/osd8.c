/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "idu.h"
#include "FontDisp.h"

#if (OSD_BIT_WIDTH == 8)

// Idu_OSDInit() will first set b_Blending to OSD_BLEND_MAX to show OSD string!
// set b_Blending to 00(0x00) - almost transparent
// set b_Blending to 15(0x0F) - no transparent

DWORD dwDefaultPalette[16] = { // Palette[0..15], b_Blending = OSD_BLEND_MAX = 0x0F
//  0x00RRGGBB
    0x00000000, // 0  - OSD_COLOR_ERASE for idu display, must not change
    0x0F000000, // 1  - OSD_COLOR_BLACK
	0x0F800000, // 2  - OSD_COLOR_LIGHT_RED
	0x0F008000, // 3  - OSD_COLOR_LIGHT_GREEN
	0x0F000080, // 4  - OSD_COLOR_LIGHT_BLUE
	0x0F808080, // 5  - OSD_COLOR_LIGHT_GRAY
	0x0FFF0000, // 6  - OSD_COLOR_RED
	0x0F00FF00, // 7  - OSD_COLOR_GREEN
	0x0F0000FF, // 8  - OSD_COLOR_BLUE
    0x0FFFFF00, // 9  - OSD_COLOR_YELLOW 
    0x0FF000FF, // 10 - OSD_COLOR_PURPLE
	0x0F00FFFF, // 11 - OSD_COLOR_SKYBLUE
	0x0FF0F000, // 12 - OSD_COLOR_LIGHT_YELLOW
	0x0F80FF80, // 13 - OSD_COLOR_APPLEGREEN
	0x0FFF8080, // 14 - OSD_COLOR_ORANGE
	0x0FFFFFFF, // 15 - OSD_COLOR_WHITE
};

DWORD YUV2RGBIndex(BYTE Y, BYTE Cb, BYTE Cr)
{
    MP_DEBUG("%s: Y=0x%08x Cb=0x%08x Cr=0x%08x", __func__, Y, Cb, Cr);
#if OSD_ENABLE
	DWORD R, G, B;
	R = 0xFF&(Y + (Cr-128)+((Cr-128)>>2)+((Cr-128)>>3)+((Cr-128)>>5));
	G = 0xFF&(Y-((Cb-128)>>2)-((Cb-128)>>4)-((Cb-128)>>5)-((Cr-128)>>1)-((Cr-128)>>3)-((Cr-128)>>4)-((Cr-128)>>5));
	B = 0xFF&(Y+(Cb-128)+((Cb-128)>>1)+((Cb-128)>>2)+((Cb-128)>>6));
	//mpDebugPrint("R=0x%08x G=0x%08x B=0x%08x, (((R & 0xe0) >> 1) + ((G & 0xc0) >> 4) + ((B & 0xc0) >> 6)); = 0x%X",R,G,B, (((R & 0xe0) >> 1) + ((G & 0xc0) >> 4) + ((B & 0xc0) >> 6)););
	return (((R & 0xe0) >> 1) + ((G & 0xc0) >> 4) + ((B & 0xc0) >> 6));
#else
    return 0;
#endif
}

BOOL Idu_OsdPaletteInit(BYTE b_Blending)
{
    MP_ALERT("%s: b_Blending = %d", __func__, b_Blending);
    
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;
    DWORD dwPalette;
    
	if (g_bAniFlag != 0)
		return FALSE;

	b_Blending &= 0xf;
	
    idu->Palette[0]  = 0x00000000; //0; // must set 0x0, or else idu display always is covered by black OSD
	int i;
	for(i = 1; i<=15; i++)
	{
	   dwPalette = dwDefaultPalette[i];
	   dwPalette &= 0x00FFFFFF; // mask off default blending
	   dwPalette |= (b_Blending << 24);
	   idu->Palette[i] = dwPalette;
	   //mpDebugPrint("Palette[%02d] = 0x%08X", i, idu->Palette[i]);
	}
#endif
    return TRUE;
}


BOOL Idu_OsdChangeBlending(BYTE b_Blending)
{
#if OSD_ENABLE

	IDU *idu = (IDU *) IDU_BASE;
    DWORD dwPalette;
    BYTE Y, Cb, Cr, bIndex;
	BYTE R, G, B;

	if (g_bAniFlag != 0)
		return FALSE;

   	b_Blending &= 0xf;
    idu->Palette[0] = 0;
    int i;
	for(i = 1; i<=15; i++)
	{
	   dwPalette = idu->Palette[i];
	   dwPalette &= 0x00FFFFFF; // mask off default blending
	   dwPalette |= (b_Blending << 24);
	   idu->Palette[i] = dwPalette;
	   //mpDebugPrint("Palette[%02d] = 0x%08X", i, idu->Palette[i]);
	}
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
	pbStartAddr = Pixel = (BYTE*)OSDWin->pdwStart + ((startY * OSDWin->wWidth + startX) >> OSD_BIT_OFFSET);

	while (SizeY)
	{
		wWidth = 0;

		if((SizeX < 0x04))
		{
			while (wWidth < SizeX)
        	{
            	//*Pixel = bColor;
            	*Pixel &= ~(0x7F);
			    *Pixel |= bColor;
				wWidth ++;
            	Pixel ++;
        	}
		}
		else  //memset last lens muset big than 4
		{
			mmcp_memset((BYTE *)Pixel, bColor, SizeX);
		}

		pbStartAddr = Pixel = pbStartAddr + (OSDWin->wWidth >> OSD_BIT_OFFSET);
		SizeY --;
	}
#endif
}

//Draw image in IMGWIN to OSDWIN
//Ex. Idu_DrawImgOnOsdWin(Idu_GetCurrWin(), 0, 0, 800, 600, Idu_GetOsdWin(), 0, 0);
void Idu_DrawImgOnOsdWin(ST_IMGWIN *psImgWin, WORD wSrcX, WORD wSrcY, WORD wSrcW, WORD wSrcH,
    ST_OSDWIN *psOsdWin,WORD wTrgX, WORD wTrgY)
{
#if OSD_ENABLE
    ST_PIXEL *pPixel, *pStartPixel;
    BYTE *pOsd, *pStart;
    WORD wWidth = wSrcW;
	BYTE R, G, B;
    IDU *idu = (IDU *) IDU_BASE;

    if ((wSrcX >= psImgWin->wWidth) || (wSrcY >= psImgWin->wHeight)
        || (wTrgX >= psOsdWin->wWidth) || (wTrgY >= psOsdWin->wHeight)
        || (!wSrcW) || (!wSrcH))
        return;

    if (wSrcX + wSrcW > psImgWin->wWidth)
        wSrcW = psImgWin->wWidth - wSrcX;
    if (wSrcY + wSrcH > psImgWin->wHeight)
        wSrcH = psImgWin->wHeight - wSrcY;
    if (wTrgX + wSrcW > psOsdWin->wWidth)
        wSrcW = psOsdWin->wWidth - wTrgX;
    if (wTrgY + wSrcH > psOsdWin->wHeight)
        wSrcH = psOsdWin->wHeight - wTrgY;

    pStartPixel = pPixel = (ST_PIXEL*)(psImgWin->pdwStart + (wSrcY * psImgWin->dwOffset + wSrcX * 2) / 4);
    pStart = pOsd = (BYTE*)(psOsdWin->pdwStart) + (wTrgY * psOsdWin->wWidth + wTrgX);

    while (wSrcH)
    {
        wWidth = 0;
        while (wWidth < wSrcW)
        {
            if ((wSrcX + wWidth) & 0x1)
            {
				*pOsd =YUV2RGBIndex(pPixel->Y1,pPixel->Cb,pPixel->Cr);
				//mpDebugPrint("idu->Palette[%d]=0x%08x",*pOsd,idu->Palette[*pOsd]);
				pPixel ++;
            }
            else
            {
				*pOsd =YUV2RGBIndex(pPixel->Y0,pPixel->Cb,pPixel->Cr);
			}
			wWidth ++;
            pOsd ++;
        }
        pStartPixel = pPixel = pStartPixel + (psImgWin->dwOffset / 4);
        pStart = pOsd = pStart + psOsdWin->wWidth;
        wSrcH --;
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
    while(1);
 }

#endif//#if (OSD_BIT_WIDTH == 8)


