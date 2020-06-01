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

#include "FontDisp.h"

WORD Str_Length(BYTE *pStr, BYTE UnicodeFlag)
{
    MP_DEBUG("%s", __func__);
    //mpDebugPrint("%s: wData = 0x%X", __func__, wData);
    
    WORD size=0,wtemp;
    BYTE *pStr1 = pStr;

    if(UnicodeFlag)
    {
        wtemp = (((*pStr1)<<8)|(*(pStr1+1)));
        pStr1 ++;
		MP_DEBUG("----- str = 0x%x", wtemp);
    }
    else
		wtemp = *pStr1;

	pStr1 ++;

    while (wtemp)
    {
        size++;

	    if(UnicodeFlag)
	    {
	        wtemp = (((*pStr1)<<8)|(*(pStr1+1)));
	        pStr1 ++;
	        size++;
			//MP_DEBUG("  0x%x", wtemp);
	    }
	    else
	        wtemp = *pStr1;
		pStr1 ++;

    }
    return size;
}

void Font_Draw(ST_IMGWIN *psWin, ST_FONT *sFont, WORD *str, BYTE UnicodeFlag)
{
	WORD i = 0;
	WORD wtemp, Num;

	BYTE *pbStr = (BYTE *)str;
	Num = Str_Length(pbStr,UnicodeFlag);

	for(i = 0; i < Num; i++)
	{
		if(UnicodeFlag)
		{
			wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
			i++;

			if( wtemp <= 0x20 || wtemp==0xA0 ) 
			{
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else
			{
				if (ImgPutChar(psWin, sFont, wtemp, 0))
					break;
			}
		}
		else	// UTF-8
		{
			int utf8CharLen, valid;
			unsigned u32char;
			U8BuffToU32Char(pbStr+i, &utf8CharLen, &u32char, &valid);

			if (u32char == 0)
				break;
			wtemp = (WORD) u32char;
			if( wtemp <= 0x20 || wtemp==0xA0 ) {
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else 
			{
				if (ImgPutChar(psWin, sFont, wtemp, 0))
					break;
			}
			i += (utf8CharLen-1);
		}
	}
}

void Font_GetStrWidth(ST_FONT *sFont, WORD *str, BYTE UnicodeFlag)
{
	WORD i = 0;
	WORD wtemp, Num;

	BYTE *pbStr = (BYTE *)str;
	Num = Str_Length(pbStr,UnicodeFlag);

	for(i = 0; i < Num; i++)
	{
		if(UnicodeFlag)
		{
			wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
			i++;

			if( wtemp <= 0x20 || wtemp==0xA0 ) 
			{
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else
			{
                if (ImgPutCharFake(sFont, wtemp, 0))
                    break;
			}
		}
		else	// UTF-8
		{
			int utf8CharLen, valid;
			unsigned u32char;
			U8BuffToU32Char(pbStr+i, &utf8CharLen, &u32char, &valid);

			if (u32char == 0)
				break;
			wtemp = (WORD) u32char;
			if( wtemp <= 0x20 || wtemp==0xA0 ) {
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else 
			{
				if (ImgPutCharFake(sFont, wtemp, 0))
                    break;
			}
			i += (utf8CharLen-1);
		}
	}
}


SWORD Font_GetStrPosByWidth(ST_FONT *sFont, WORD *str, BYTE UnicodeFlag,WORD wWidth)
{
	WORD i = 0;
	WORD wtemp, Num;
	BYTE *pbStr = (BYTE *)str;
	
	Num = Str_Length(pbStr,UnicodeFlag);
	for(i = 0; i < Num; i++)
	{
		if(UnicodeFlag)
		{
			wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));

			if( wtemp <= 0x20 || wtemp==0xA0 ) 
			{
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else
			{
                if (ImgPutCharFake(sFont, wtemp, 0))
                    break;
			}
			if (sFont->wX>wWidth)
				break;
			i++;
		}
		else	// UTF-8
		{
			int utf8CharLen, valid;
			unsigned u32char;
			U8BuffToU32Char(pbStr+i, &utf8CharLen, &u32char, &valid);

			if (u32char == 0)
				break;
			wtemp = (WORD) u32char;
			if( wtemp <= 0x20 || wtemp==0xA0 ) {
				sFont->wX += (IduFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (IduFontGetBlankWidth() + sFont->bTextGap);
			}
			else 
			{
				if (ImgPutCharFake(sFont, wtemp, 0))
                    break;
			}
			if (sFont->wX>wWidth)
				break;
			i += (utf8CharLen-1);
		}
	}
	return i;
}

void Font_Draw_OSD(ST_OSDWIN *psWin,ST_FONT *sFont,unsigned short int *str)
{
    DWORD    j;
    BYTE   bufB, rtn =0, err;
    WORD  i = 0;
    BYTE   *bitmap;
    DWORD bitmapBufferSize = 0;
    DWORD get_word;
    WORD wtemp,Num;

	BYTE *pbStr = (BYTE *)str,get_byte;

	Num = Str_Length(pbStr,0);

    for(i = 0; i < Num; i++)
    {
        for (j = 0; j < bitmapBufferSize; j++)
            bitmap[j] = 0x00;

        get_byte = *(pbStr+i);

        if(get_byte <= 0x20)
        {
    	    sFont->wX += (OsdFontGetBlankWidth() + sFont->bTextGap);
    	    sFont->wDisplayWidth += (OsdFontGetBlankWidth() + sFont->bTextGap);
            get_word = 0;
        }
        else
        {
			int utf8CharLen, valid;
			unsigned u32char;
			U8BuffToU32Char(pbStr+i, &utf8CharLen, &u32char, &valid);

			if (u32char == 0)
				break;
			wtemp = (WORD) u32char;
			if( wtemp <= 0x20 || wtemp==0xA0 ) {
				sFont->wX += (OsdFontGetBlankWidth() + sFont->bTextGap);
				sFont->wDisplayWidth += (OsdFontGetBlankWidth() + sFont->bTextGap);
			}
			else {
				Idu_OsdPutChar_1X(psWin, sFont, wtemp, 0);
			}
			i += (utf8CharLen-1);
        }

        if(i==0xFF) break;
    }
}

#endif

