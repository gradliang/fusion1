/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE  1

#include "global612.h"
#include "mpTrace.h"
#include "Peripheral.h"
#include "PhotoFrame.h"
#include "Peripheral.h"
#include "ui_timer.h"

#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

//------------------------------------------------------------------------------
// TimeStamp
//------------------------------------------------------------------------------
#define FVGA_TAG			0x46564741  // FVGA
BYTE *pbTimeStampFontBuf = NULL;
BYTE *xpgGetTimeStampFontBuf()
{
    return pbTimeStampFontBuf;
}
void xpgTimeStampFontInit(void)
{
    MP_DEBUG("%s", __func__);
    
    DWORD dwFVGASize = IspFunc_GetRESOURCESize(FVGA_TAG);
    mpDebugPrint("%s: FVGA_TAG dwFVGASize = %d", __func__, dwFVGASize);
    if(dwFVGASize == 0)
    {
        MP_ALERT("dwFVGASize == 0");
        return;
    }
	pbTimeStampFontBuf = (BYTE *)ext_mem_malloc(dwFVGASize);
	if(pbTimeStampFontBuf == NULL)
	{
	   MP_ALERT("ext_mem_malloc FVGA.yuv buf fail!");
	   return;
    }
	
	if (IspFunc_ReadRESOURCE(FVGA_TAG, (BYTE *)pbTimeStampFontBuf, dwFVGASize) == 0)
	{
		MP_ALERT(" -E- read FVGA.yuv error");
		ext_mem_free(pbTimeStampFontBuf);
		return;
	}
	mpDebugPrint("%s: FVGA.yuv dwFVGASize = %d, pbTimeStampFontBuf = 0x%X", __func__, dwFVGASize, pbTimeStampFontBuf);
}

WORD wTimeStampX = 100;
WORD wTimeStampY = 100;
WORD digit = 0;
void xpgTimeStampDisplay()
{
     xpgTimeStampDisplayXY(wTimeStampX, wTimeStampY);
}    

void xpgTimeStampDisplayXY(WORD x, WORD y)
{
    MP_DEBUG("%s: x = %d, y = %d", __func__, x, y);
    
    ST_IMGWIN *srcMainWin = Idu_GetCurrWin();
    ST_IMGWIN *trgWin     = Idu_GetCurrWin();
    ST_IMGWIN srcSubWin;
    srcSubWin.wWidth = 22 ;
    srcSubWin.wHeight = 40 ;
    srcSubWin.pdwStart = (DWORD *)(pbTimeStampFontBuf + 8 + digit * 1760);
    srcSubWin.dwOffset = (srcSubWin.wWidth<<1) ;
    WORD start_x1 = x, start_y1 = y; 
    BYTE level=0;
    // BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
    // if (colorkey_level == 0)	// Sub win 100 %, Main win 0%
    // if (colorkey_level == 1)	// Sub win  50 %, Main win 50%
    // if (colorkey_level == 2)	// Sub win   0 %, Main win 100%
    Ipu_Overlay(srcMainWin, &srcSubWin, trgWin, start_x1, start_y1, level, ENABLE, 2, 0, 0x80, 0x80);
    digit += 1;
    if(digit > 9)
        digit = 0;
}

WORD wTimeStampBufWidth = 220; // 22 x 10 = 220
WORD wTimeStampBufHeight = 80; // 40 x 2 = 80;
BYTE bTimeStampBuf[35200]; // 220 x 80 x 2 = 35200
void xpgTimeStampFontCopyToBuf(BYTE index, BYTE num)
{
    MP_DEBUG("%s: index = %d, num = %d", __func__, index, num);
    WORD h, row;
    row = index / 10;
    index = index % 10;
    for(h=0; h< 40; h++)
    {
        memcpy((bTimeStampBuf + row * 220 * 2 * 40 + index * 44 + h * 220 * 2), 
            pbTimeStampFontBuf + 8 + num * 1760 + h * 22 * 2, 44);
    }
}

void xpgTimeStampBufDisplayXY(WORD x, WORD y)
{
    MP_DEBUG("%s: x = %d, y = %d", __func__, x, y);
        
    memset(bTimeStampBuf, 0, sizeof(bTimeStampBuf));
    ST_SYSTEM_TIME stCurrTime;
    SystemTimeGet(&stCurrTime);
    WORD number;
    number = stCurrTime.u16Year / 1000;
    xpgTimeStampFontCopyToBuf(0, number);
    number = (stCurrTime.u16Year / 100) % 10;
    xpgTimeStampFontCopyToBuf(1, number);
    number = (stCurrTime.u16Year / 10) % 10;
    xpgTimeStampFontCopyToBuf(2, number);
    number = stCurrTime.u16Year % 10;
    xpgTimeStampFontCopyToBuf(3, number);
    xpgTimeStampFontCopyToBuf(4, 11); // '/'
    number = stCurrTime.u08Month / 10;
    xpgTimeStampFontCopyToBuf(5, number);
    number = stCurrTime.u08Month % 10;
    xpgTimeStampFontCopyToBuf(6, number);
    xpgTimeStampFontCopyToBuf(7, 11); // '/'
    number = stCurrTime.u08Day / 10;
    xpgTimeStampFontCopyToBuf(8, number);
    number = stCurrTime.u08Day % 10;
    xpgTimeStampFontCopyToBuf(9, number);
    
    if (stCurrTime.u08Hour > 11)
        number = E_TIME_STAMP_FONT_P;
    else
        number = E_TIME_STAMP_FONT_A;
    xpgTimeStampFontCopyToBuf(10, number);
    number = E_TIME_STAMP_FONT_M;
    xpgTimeStampFontCopyToBuf(11, number);
    number = stCurrTime.u08Hour / 10;
    xpgTimeStampFontCopyToBuf(12, number);
    number = stCurrTime.u08Hour % 10;
    xpgTimeStampFontCopyToBuf(13, number);
    number = E_TIME_STAMP_FONT_T;
    xpgTimeStampFontCopyToBuf(14, number);
    number = stCurrTime.u08Minute / 10;
    xpgTimeStampFontCopyToBuf(15, number);
    number = stCurrTime.u08Minute % 10;
    xpgTimeStampFontCopyToBuf(16, number);
    number = E_TIME_STAMP_FONT_T;
    xpgTimeStampFontCopyToBuf(17, number);
    number = stCurrTime.u08Second / 10;
    xpgTimeStampFontCopyToBuf(18, number);
    number = stCurrTime.u08Second % 10;
    xpgTimeStampFontCopyToBuf(19, number);
    
    ST_IMGWIN *srcMainWin = Idu_GetCurrWin();
    ST_IMGWIN *trgWin     = Idu_GetCurrWin();
    ST_IMGWIN srcSubWin;
    srcSubWin.wWidth = 220 ;
    srcSubWin.wHeight = 80 ;
    srcSubWin.pdwStart = (DWORD *)(&bTimeStampBuf);
    srcSubWin.dwOffset = (srcSubWin.wWidth<<1) ;
    WORD start_x1 = x, start_y1 = y; 
    BYTE level=0;
    // BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
    // if (colorkey_level == 0)	// Sub win 100 %, Main win 0%
    // if (colorkey_level == 1)	// Sub win  50 %, Main win 50%
    // if (colorkey_level == 2)	// Sub win   0 %, Main win 100%
    Ipu_Overlay(srcMainWin, &srcSubWin, trgWin, start_x1, start_y1, level, ENABLE, 2, 0, 0x80, 0x80);
}
