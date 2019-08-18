/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "idu.h"
#include "FontDisp.h"

#if OSD_ENABLE && (OSD_BIT_WIDTH == 4)

// set b_Blending to 0 - almost transparent
// set b_Blending to 127(0x7F) - no transparent

DWORD dwDefaultPalette[16] = {
//  0x00RRGGBB
    0x00000000, // 0  - OSD_COLOR_ERASE for idu display, must not change

		0x0F000000, // 1  - OSD_COLOR_BLACK--
		0x0FFFFFFF, // 2  - OSD_COLOR_WHITE
		0x0FFF0000, // 3  - OSD_COLOR_RED-icon-
		0x0F00FF00, // 4  - OSD_COLOR_GREEN-icon-
		0x0F0000FF, // 5  - BLUE-
// Customer
		0x0F00FFFF, // 6  - OSD ICON-
		0x0FFF00FF, // 7  - OSD ICON-
		0x0FFFFF00, // 8  - OSD ICON-
		0x0df4f1f2, // 9  - OSD ICON-
		0x0df7f7f7, // 10 a- OSD ICON-
		0x0de5041a, // 11 b- OSD ICON-
		0x0df5a8b2, // 12 c- OSD ICON-
		0x0ded6375, // 13 d- OSD ICON-
		0x0d7eff7e, // 14 e- OSD ICON-
		0x0ded6375, // 15 f- OSD ICON
};

BOOL Idu_OsdPaletteInit(BYTE b_Blending)
{
    MP_DEBUG("%s: b_Blending = %d", __func__, b_Blending);
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;

	if (g_bAniFlag != 0)
		return FALSE;

	BYTE TransBlend = 0;

	b_Blending &= 0xf;
	if (b_Blending > 1)
		TransBlend = b_Blending - 1;

	// this is only for 4Bit OSD
	idu->Palette[0] = 0;

    int i;
	for(i = 0; i<=15; i++)
	{
	   idu->Palette[i] = dwDefaultPalette[i];
	   //mpDebugPrint("Palette[%d] = 0x%08X", i, idu->Palette[i]);
	}

	//--------------------------//
	
#endif

	return TRUE;
}

BOOL Idu_OsdChangeBlending(BYTE b_Blending)
{
#if OSD_ENABLE
	IDU *idu = (IDU *) IDU_BASE;
	if (g_bAniFlag != 0)
		return FALSE;

	BYTE TransBlend = 0;

	b_Blending &= 0xf;
	if (b_Blending > 1)
		TransBlend = b_Blending - 1;

	// this is only for 4Bit OSD
	idu->Palette[0] = 0;
#if 1
    DWORD dwPalette;
    int i;
	for(i = 1; i<=15; i++)
	{
	   dwPalette = idu->Palette[i];
	   dwPalette &= 0x00FFFFFF; // mask off default blending
	   dwPalette |= (b_Blending << 24);
	   idu->Palette[i] = dwPalette;
	   //mpDebugPrint("Palette[%02d] = 0x%08X", i, idu->Palette[i]);
	}
#else	
	idu->Palette[1] = (b_Blending << 24) | idu->Palette[1];
	idu->Palette[2] = (b_Blending << 24) | idu->Palette[2];
	idu->Palette[3] = (b_Blending << 24) | idu->Palette[3];
	idu->Palette[4] = (b_Blending << 24) | idu->Palette[4];
	idu->Palette[5] = (b_Blending << 24) | idu->Palette[5];

	idu->Palette[6] = (b_Blending << 24) | idu->Palette[6];
	idu->Palette[7] = (b_Blending << 24) | idu->Palette[7];
	idu->Palette[8] = (b_Blending << 24) | idu->Palette[8];
	idu->Palette[9] = (b_Blending << 24) | idu->Palette[9];
	idu->Palette[10] = (b_Blending << 24) | idu->Palette[10];
	idu->Palette[11] = (b_Blending << 24) | idu->Palette[11];
	idu->Palette[12] = (b_Blending << 24) | idu->Palette[12];
	idu->Palette[13] = (b_Blending << 24) | idu->Palette[13];
	idu->Palette[14] = (b_Blending << 24) | idu->Palette[14];
	idu->Palette[15] = (b_Blending << 24) | idu->Palette[15];
#endif
#endif
    return TRUE;

}

void Idu_OsdPaintArea(WORD startX, WORD startY, WORD SizeX, WORD SizeY, BYTE b_PaletteIndex)
{
    //mpDebugPrint("### %s ### b_PaletteIndex=%d,x=%d y=%d", __FUNCTION__,b_PaletteIndex,startX, startY);
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

	b_PaletteIndex &= OSD_MASK;
	pbStartAddr = Pixel = 
        (BYTE*)OSDWin->pdwStart + ((startY * OSDWin->wWidth + startX) >> OSD_BIT_OFFSET);

	while (SizeY)
	{
		wWidth = 0;
		while (wWidth < SizeX)
		{
			if ((startX + wWidth) & 0x1)
				*Pixel = (*Pixel & 0xf0) + b_PaletteIndex;
			else
				*Pixel = (*Pixel & 0x0f) + (b_PaletteIndex << 4);
   			wWidth ++;
            if (!((startX + wWidth) & 0x1) && (wWidth < SizeX))
                Pixel ++;
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
 
#endif//#if (OSD_BIT_WIDTH == 4)

