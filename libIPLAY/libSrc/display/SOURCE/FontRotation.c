/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

#include "global612.h"
#include "mpTrace.h"
#include "idu.h"

#if BMP_FONT_ENABLE
#if NEWFONTSTRCUCTURE   

void Font_DrawOSDSpace(ST_OSDWIN *psWin,ST_FONT *sFont)
{
	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT) //right
		{
			sFont->wX += (sFont->bFontSize>>1) + sFont->bTextGap;
			sFont->wDisplayWidth += (sFont->bFontSize>>1) + sFont->bTextGap;

			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				{
					Font_DrawOSD2Point(psWin , sFont);	// The string too long, display ".."
				}
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)// left
		{
			sFont->wX -= (sFont->bFontSize>>1) -sFont->bTextGap;
			sFont->wDisplayWidth += (sFont->bFontSize>>1) + sFont->bTextGap;

			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				{
					Font_DrawOSD2Point(psWin , sFont);	// The string too long, display ".."
				}

		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_UP)//up
		{
			sFont->wY -= (sFont->bFontSize>>1) ;
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_DOWN)//down
		{
			sFont->wY += (sFont->bFontSize>>1) ;

		}

}


void DrawFontBitmap2OsdwinV(BYTE *StartDrawAddr, DWORD DrawWinOffset,ST_FONT *sFont, char *bitmap)
{
	DWORD i,j;
	BYTE bMask = 0,temp;
	BYTE bPixel;
	DWORD dwOffset;
	BYTE *pbStartAddr;

	pbStartAddr=StartDrawAddr;
	dwOffset=DrawWinOffset;
	i=sFont->bHeight;

	while (i--)
	{
		j = sFont->bWidth;
#if (OSD_BIT_WIDTH == 2)
		temp = 6;
#elif (OSD_BIT_WIDTH == 4)
		temp = 4;
#elif (OSD_BIT_WIDTH == 8)
		temp = 0;
#endif
		while (j--)
		{
			if (!bMask)
			{
				bMask = 0x80;
				bPixel = *bitmap++;				
				while (j >= 8 && bPixel == 0)
				{
					if(sFont->bAngle==ROTATE_RIGHT_0)
						pbStartAddr += OSD_BIT_WIDTH;
					else if(sFont->bAngle==ROTATE_RIGHT_180)
						pbStartAddr -= OSD_BIT_WIDTH;
					bPixel = *bitmap++;
					j -= 8;
				}
			}

			if (bPixel & bMask)
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
			bMask >>= 1;

			if (temp == 0)
			{
#if (OSD_BIT_WIDTH == 2)
				temp = 6;
				if (j)
#elif (OSD_BIT_WIDTH == 4)
				temp = 4;
#endif
				if(sFont->bAngle==ROTATE_RIGHT_0)
					pbStartAddr ++;
				else if(sFont->bAngle==ROTATE_RIGHT_180)
					pbStartAddr--;
			}
#if (OSD_BIT_WIDTH == 2)
			else
				temp -= 2;
#elif (OSD_BIT_WIDTH == 4)
			else
				temp -= 4;
#endif
		}
		if(sFont->bAngle==ROTATE_RIGHT_0)
			pbStartAddr += dwOffset;
		else if(sFont->bAngle==ROTATE_RIGHT_180)
			pbStartAddr -= dwOffset;
	}

}


void DrawFontBitmap2OsdwinH(BYTE *StartDrawAddr, DWORD DrawWinOffset,ST_FONT *sFont, char *bitmap)
{
	BYTE bMask = 0,temp;
	BYTE bPixel,x,y;
	DWORD dwOffset;
	BYTE *pbStartAddr;
	DWORD nW,nH;
	DWORD count=0;

	nW=sFont->bHeight;
	nH= sFont->bWidth;

	dwOffset=DrawWinOffset;
	bMask = 0x80;
	temp=0;

	for(x=0;x<nW;x++)
		{
			if(sFont->bAngle==ROTATE_RIGHT_90)
				pbStartAddr=StartDrawAddr+nW-x;
			else
				pbStartAddr=StartDrawAddr+dwOffset*sFont->bWidth+x;

			for(y=0;y<nH;y++)	
				{
					BYTE bMask2 = 0;

					bPixel = *bitmap;
					bMask2=bMask >> temp;

					count++;
					temp=count%OSD_BIT_WIDTH;

					if(temp==0)
						bitmap++;

					if (bPixel & bMask2)
						*(pbStartAddr) = (sFont->bFontColor & 0x7f);

					if(sFont->bAngle==ROTATE_RIGHT_90)
						pbStartAddr += dwOffset;
					else
						pbStartAddr -= dwOffset;
				}
		}

}


void FontDrawOSDDirectionRight(ST_OSDWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	DWORD dwOffset;
	BYTE *pbStartAddr;


	if(sFont->bAngle==ROTATE_RIGHT_0)
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth) >> OSD_BIT_OFFSET));
	else if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));

#if (OSD_BIT_WIDTH == 2)
	dwOffset = ((psWin->wWidth - ALIGN_4(sFont->bWidth)) >> OSD_BIT_OFFSET) + 1;
#else
	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		dwOffset = (psWin->wWidth ) >> OSD_BIT_OFFSET;
	else
		dwOffset = (psWin->wWidth - sFont->bWidth) >> OSD_BIT_OFFSET;
#endif

if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
	DrawFontBitmap2OsdwinH(pbStartAddr,dwOffset,sFont,bitmap);
else
	DrawFontBitmap2OsdwinV(pbStartAddr,dwOffset,sFont,bitmap);

	sFont->wDisplayWidth += sFont->bTextGap;
	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		sFont->wX += sFont->bHeight+ sFont->bTextGap;
	else
		sFont->wX += sFont->bWidth + sFont->bTextGap;

}

void FontDrawOSDDirectionLeft(ST_OSDWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	DWORD dwOffset;
	BYTE *pbStartAddr;

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		sFont->wX -=sFont->bWidth +sFont->bTextGap;
	else if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		sFont->wX -=sFont->bHeight;

	if(sFont->wX & 0x01) 
		 sFont->wX&=~0x1;

	if(sFont->bAngle==ROTATE_RIGHT_0)
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth) >> OSD_BIT_OFFSET));
	else if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));

#if (OSD_BIT_WIDTH == 2)
	dwOffset = ((psWin->wWidth - ALIGN_4(sFont->bWidth)) >> OSD_BIT_OFFSET) + 1;
#else
	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		dwOffset = (psWin->wWidth ) >> OSD_BIT_OFFSET;
	else
		dwOffset = (psWin->wWidth - sFont->bWidth) >> OSD_BIT_OFFSET;	
#endif

	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		DrawFontBitmap2OsdwinH(pbStartAddr,dwOffset,sFont,bitmap);
	else
		DrawFontBitmap2OsdwinV(pbStartAddr,dwOffset,sFont,bitmap);

	sFont->wDisplayWidth += sFont->bTextGap;

}
void FontDrawOSDDirectionUp(ST_OSDWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	DWORD dwOffset;
	BYTE *pbStartAddr;

	if(sFont->wX & 0x01) sFont->wX--;

	if(sFont->bAngle==ROTATE_RIGHT_0)
		{
			sFont->wY -=sFont->bHeight;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));
		}
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth) >> OSD_BIT_OFFSET));
			sFont->wY -=sFont->bHeight;
		}
	else if(sFont->bAngle==ROTATE_RIGHT_270)
		{
			sFont->wY -=sFont->bWidth+sFont->bTextGap;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));

		}
	else if(sFont->bAngle==ROTATE_RIGHT_90)
		{
			sFont->wY -=sFont->bWidth+sFont->bTextGap;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX-sFont->bHeight) >> OSD_BIT_OFFSET));
		}
		
	
#if (OSD_BIT_WIDTH == 2)
	dwOffset = ((psWin->wWidth - ALIGN_4(sFont->bWidth)) >> OSD_BIT_OFFSET) + 1;
#else
	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		dwOffset = (psWin->wWidth ) >> OSD_BIT_OFFSET;
	else
		dwOffset = (psWin->wWidth - sFont->bWidth) >> OSD_BIT_OFFSET;	
#endif

	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		DrawFontBitmap2OsdwinH(pbStartAddr,dwOffset,sFont,bitmap);
	else
		DrawFontBitmap2OsdwinV(pbStartAddr,dwOffset,sFont,bitmap);

}
void FontDrawOSDDirectionDown(ST_OSDWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	DWORD dwOffset;
	BYTE *pbStartAddr;

	if(sFont->wX & 0x01) sFont->wX--;

	if(sFont->bAngle==ROTATE_RIGHT_0)
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));
			sFont->wY +=sFont->bHeight;
		}
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		{
			sFont->wY +=sFont->bHeight;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth) >> OSD_BIT_OFFSET));
		}
	else if(sFont->bAngle==ROTATE_RIGHT_270)
		{	
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) >> OSD_BIT_OFFSET));
			sFont->wY +=sFont->bWidth+sFont->bTextGap;
		}
	else if (sFont->bAngle==ROTATE_RIGHT_90)
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX-sFont->bHeight) >> OSD_BIT_OFFSET));
			sFont->wY +=sFont->bWidth+sFont->bTextGap;
		}
		
#if (OSD_BIT_WIDTH == 2)
	dwOffset = ((psWin->wWidth - ALIGN_4(sFont->bWidth)) >> OSD_BIT_OFFSET) + 1;
#else
	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		dwOffset = (psWin->wWidth ) >> OSD_BIT_OFFSET;
	else
		dwOffset = (psWin->wWidth - sFont->bWidth) >> OSD_BIT_OFFSET;
#endif

	if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		DrawFontBitmap2OsdwinH(pbStartAddr,dwOffset,sFont,bitmap);
	else
		DrawFontBitmap2OsdwinV(pbStartAddr,dwOffset,sFont,bitmap);

}

int FontDrawOSDRotation(ST_OSDWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	DWORD dwOffset;
	register DWORD j;
	register BYTE *pbStartAddr;
	register BYTE bMask = 0,temp;
	register BYTE bPixel;

	if (g_bLargeFontFlag)//Mason 20061031
		return FontDrawOSD2(psWin, sFont, bitmap);
	if(sFont->bBitsPerPixel>1)
		return FontDrawOSDGrayImage(psWin, sFont, bitmap);

	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT) //right
		{
			if(sFont->wX & 0x01) sFont->wX++;

			sFont->wDisplayWidth += sFont->bWidth;
			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				return 1;
	
			FontDrawOSDDirectionRight(psWin,sFont,bitmap);
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)// left
		{
			sFont->wDisplayWidth += sFont->bWidth;
			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				return 1;

			FontDrawOSDDirectionLeft(psWin,sFont,bitmap);
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_UP)//up
		{
			sFont->wDisplayWidth += sFont->bWidth;
			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				return 1;

			FontDrawOSDDirectionUp(psWin,sFont,bitmap);
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_DOWN)//down
		{
			sFont->wDisplayWidth += sFont->bWidth;
			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				return 1;

			FontDrawOSDDirectionDown(psWin,sFont,bitmap);
		}
	return 0;

}
void Font_Draw_OSD_Rotation(ST_OSDWIN *psWin,ST_FONT *sFont, BYTE *str, BYTE UnicodeFlag)
{
    WORD  i = 0;
    BYTE   *bitmap;
    DWORD get_word;
    WORD wtemp,Num;
    BYTE *pbStr = (BYTE *)str,get_byte;

    Num = Str_Length(pbStr,UnicodeFlag);

    if(UnicodeFlag)
    {
        for(i = 0; i < Num; i++)
        {
        	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT)
        		{
				if(sFont->wX +sFont->bWidth+sFont->bTextGap>psWin->wWidth)
					{
						mpDebugPrint("Over Range U-1 sFont->wX=%d sFont->bAngle=%d",sFont->wX,sFont->bAngle);
						return;
					}
        		}
		else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)
        		{
				if(sFont->wX -sFont->bWidth-sFont->bTextGap<0)
					{
						mpDebugPrint("Over Range U-2 sFont->wX=%d sFont->bAngle=%d",sFont->wX,sFont->bAngle);
						return;
					}
        		}
            wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
            i++;
		
            if(wtemp <= 0x20)       // Space
            {
                get_word = 0;
		  Font_DrawOSDSpace(psWin,sFont);
            }
            else
            {
                bitmap = (BYTE*)UnicodeGetFontBitmap(wtemp, sFont);

                if(bitmap != NULL)
                {
                    MP_DEBUG("bHeight=0x%x, bWidth=0x%x\r\n",sFont->bHeight, sFont->bWidth);
			if (FontDrawOSDRotation(psWin, sFont, (char *)bitmap))
                    {
                  		if(sFont->bBitsPerPixel>1)
					{
						bitmap =(BYTE*) UnicodeGetFontBitmap(0x2E, sFont);		
						FontDrawOSDGrayImage2point(psWin, sFont, (char *)bitmap);
					}
				else	
					Font_DrawOSD2Point(psWin , sFont);	// The string too long, display "..
                        break;//Mason 20061024	//From Lighter //Lighter 20080817 over the display area do not display anymore
                    }

                }
                else        // big problem
                {
                    mpDebugPrint("Rotation-System do not have UNKNOWN_CHAR \r\n");
                    return;
                }
            }
        }
    }
    else
    {
        for(i = 0; i < Num; i++)
        {
            get_byte = *(pbStr+i);
            MP_DEBUG("Native get_byte %x",get_byte);
        	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT)
        		{		
				if(sFont->wX +sFont->bWidth+sFont->bTextGap>psWin->wWidth)
					{
						mpDebugPrint("Over Range A-1 sFont->wX=%d sFont->bAngle=%d",sFont->wX,sFont->bAngle);
						return;
					}
        		}
		else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)
        		{
				if(sFont->wX -sFont->bWidth-sFont->bTextGap<0)
					{
						mpDebugPrint("Over Range A-2 sFont->wX=%d sFont->bAngle=%d",sFont->wX,sFont->bAngle);
						return;
					}
        		}

            if(get_byte <= 0x20)	//Space case
            {
                get_word = 0;
		  Font_DrawOSDSpace(psWin,sFont);
            }
            else
            {
                if((get_byte <= 0x7F)||((get_byte == 0xb0)&&((*(pbStr+i+1))== 0x00)))	//check the font data need to be a 16 bit
                {
                    bitmap = (BYTE*)ANSIGetFontBitmap(get_byte, sFont);		
                }
                else
                {
                    wtemp = 0;
                    switch(g_bNCTable)
                    {
#if (FONT_GB2 || FONT_BIG5 || FONT_JIS || FONT_KSC)
#if FONT_GB2
                    case LANGUAGE_S_CHINESE:
#endif
#if FONT_BIG5
                    case LANGUAGE_T_CHINESE:
#endif
#if FONT_JIS
                    case LANGUAGE_JAPAN:
#endif
#if FONT_KSC
                    case LANGUAGE_KOREA:
#endif
                        wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
                        i++;
                        break;
#endif
                    default:
                        wtemp = get_byte;
                        break;
                    }

                    MP_DEBUG("Native word 0x%x, 0x%x, 0x%x",wtemp,(*(pbStr+i-1)), (*(pbStr+i)));

                    bitmap =(BYTE*) ANSIGetFontBitmap(wtemp, sFont);
                }

                MP_DEBUG("Native bitmap=0x%x", bitmap);

                if(bitmap != NULL)
                {
                    MP_DEBUG("Native bHeight=0x%x, bWidth=0x%x",sFont->bHeight, sFont->bWidth);
			if (FontDrawOSDRotation(psWin, sFont, (char *)bitmap))		
                    {
                  		if(sFont->bBitsPerPixel>1)
					{
						bitmap = (BYTE*)ANSIGetFontBitmap(0x2E, sFont);		
						FontDrawOSDGrayImage2point(psWin, sFont, (char *)bitmap);
					}
				else		
                      		  Font_DrawOSD2Point(psWin , sFont);	// The string too long, display ".."
                        break;//Mason 20061024	//From Lighter //Lighter 20080817 over the display area do not display anymore
                    }
                    //continue;
                }
                else		// big problem
                {
                    mpDebugPrint("Rotation-Native System do not have UNKNOWN_CHAR \r\n");
                    return ;
                }
            }
        }
    }
}

void Idu_OsdPrintRotation(ST_OSDWIN * psWin, BYTE * pbString, WORD startX, WORD startY, BYTE bColorIndex, BYTE UnicodeFlag, WORD wWidth, BYTE Font_Size,BYTE angle,BYTE direction)
{
	ST_FONT ft;

	ft.wX = startX;
	ft.wY = startY;
	ft.bTextGap = 1;
	ft.bFontColor = bColorIndex;
	ft.wDisplayWidth = 0;
	ft.wWidth = wWidth; 
	ft.bWidth = DEFAULT_FONT_SIZE;
	ft.bHeight = DEFAULT_FONT_SIZE;
	ft.bFontSize = Font_Size;
	ft.bAngle=angle;
	ft.bPrintDirection=direction;
	ft.bBitsPerPixel = 1;
	ft.bAlign = 0;
	ft.bYOffset = 0;
	
	Font_Draw_OSD_Rotation(psWin, &ft, pbString, UnicodeFlag);
}

//////////////////////////////////////////////////
//////////         Draw Bitmap on IDU        /////////////
/////////////////////////////////////////////////

void DrawFontBitmap2iduV(BYTE *StartDrawAddr, DWORD DrawWinOffset,ST_FONT *sFont, char *bitmap)
{
	BYTE bMask = 0,bMask1 = 0;
	BYTE *pbStartAddr=StartDrawAddr;
	DWORD j = 0;
	DWORD dwOffset=DrawWinOffset;
	unsigned char   *str, bPixel;
	BYTE cy = sFont->bFontColor;
	BYTE cb = sFont->bFontCb;
	BYTE cr = sFont->bFontCr;
	DWORD dwFontColor = (cy << 24) | (cy << 16) | (cb << 8) | cr;
	DWORD xx=sFont->bHeight;

	str = (unsigned char*)bitmap;

	while(xx--)
	{
		j = sFont->bWidth>>1;
		while(j--)
		{
			if(!bMask)
			{
				bMask = 0xC0;
				bMask1 = 0x80;
				bPixel = *str++;
			}

			if(bPixel & bMask)
			{
				if((bPixel & bMask) == bMask)
				{
					*((DWORD *)pbStartAddr) = dwFontColor;
				}				
				else
				{
					if(bPixel & bMask1)
					{
						if(sFont->bAngle==ROTATE_RIGHT_0)
							*(pbStartAddr + 0) = cy;//white 0;
						else
							*(pbStartAddr+1) = cy;//white 0;
					}
					else
					{
						if(sFont->bAngle==ROTATE_RIGHT_0)
							*(pbStartAddr + 1) = cy;//white 0;
						else
							*(pbStartAddr ) = cy;//white 0;			
					}

					if(sFont->bAngle==ROTATE_RIGHT_0)
					{
						*(pbStartAddr + 2) = (*(pbStartAddr + 2) + cb) >> 1;
						*(pbStartAddr + 3) = (*(pbStartAddr + 3) + cr) >> 1;
					}
					else if(sFont->bAngle==ROTATE_RIGHT_180)
					{
						*(pbStartAddr + 2) = (*(pbStartAddr + 2) + cb) >> 1;
						*(pbStartAddr + 3) = (*(pbStartAddr + 3) + cr) >> 1;
					}
				}
			}

			if(sFont->bAngle==ROTATE_RIGHT_0)
				pbStartAddr += 4;
			else if(sFont->bAngle==ROTATE_RIGHT_180)
				pbStartAddr -= 4;
			bMask >>= 2;
			bMask1 >>= 2;
		}

		if(sFont->bAngle==ROTATE_RIGHT_0)
			pbStartAddr += dwOffset;
		else if(sFont->bAngle==ROTATE_RIGHT_180)
			pbStartAddr -= dwOffset;

	}

}

void DrawFontBitmap2iduH(BYTE *StartDrawAddr, DWORD DrawWinOffset,ST_FONT *sFont, char *bitmap)
{
	BYTE bMask = 0,bMask1 = 0;
	BYTE *pbStartAddr=StartDrawAddr;
	BYTE x,y;
	DWORD dwOffset=DrawWinOffset;
	DWORD nW,nH,temp,count=0;;
	
	unsigned char   *str, bPixel;
	BYTE cy = sFont->bFontColor;
	BYTE cb = sFont->bFontCb;
	BYTE cr = sFont->bFontCr;
	DWORD dwFontColor = (cy << 24) | (cy << 16) | (cb << 8) | cr;

	if(sFont->bHeight & 0x01) 
		 sFont->bHeight--;

	nW=sFont->bHeight;
	nH=sFont->bWidth ;
	bMask = 0x80;
	temp=0;

	for(x=0;x<nW;x++)
		{
			if(sFont->bAngle==ROTATE_RIGHT_90)
				pbStartAddr=StartDrawAddr+(nW-x)*2;
			else
				pbStartAddr=StartDrawAddr+dwOffset*nH+x*2;

			for(y=0;y<nH;y++)
				{
					BYTE bMask2 = 0;

					bPixel = *bitmap;
					bMask2=bMask >> temp;

					count++;
					temp=count%8;

					if(temp==0)
						bitmap++;
					if (bPixel & bMask2)
						{
							if(x%2==0) 
								{
									if(sFont->bAngle==ROTATE_RIGHT_90)
										*(pbStartAddr +0) = cy;
									else 
										*(pbStartAddr +0) = cy;

									*(pbStartAddr + 2) = (*(pbStartAddr + 2) + cb) >> 1;
									*(pbStartAddr + 3) = (*(pbStartAddr + 3) + cr) >> 1;
								}
							else
								{
									BYTE * tempaddr=(BYTE *)pbStartAddr;

									tempaddr-=2;

									if(sFont->bAngle==ROTATE_RIGHT_90)
										*(tempaddr +1) = cy;
									else 
										*(tempaddr +1) = cy;

									*(tempaddr + 2) = (*(tempaddr + 2) + cb) >> 1;
									*(tempaddr + 3) = (*(tempaddr + 3) + cr) >> 1;
								}
						}
					if(sFont->bAngle==ROTATE_RIGHT_90)
						pbStartAddr += dwOffset;
					else
						pbStartAddr -= dwOffset;	
				}
		}

}



int FontDrawImageDirectionRight(ST_IMGWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	BYTE *pbStartAddr ;	
	DWORD dwOffset=0;
		
	if(sFont->bAngle==ROTATE_RIGHT_0)
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
			dwOffset = (psWin->wWidth - sFont->bWidth) << 1;
		}
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth-2) << 1));
			dwOffset = (psWin->wWidth - sFont->bWidth) << 1;
		}
	else if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		{
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
			dwOffset = (psWin->wWidth) << 1;

		}

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		{
			if (sFont->wX + sFont->bWidth + sFont->bTextGap>psWin->wWidth)
				return -1;
			DrawFontBitmap2iduV(pbStartAddr,dwOffset,sFont,bitmap);
			sFont->wX += sFont->bWidth + sFont->bTextGap;
		}
	else
		{
			if (sFont->wX + sFont->bHeight + sFont->bTextGap>psWin->wWidth)
				return -1;
			DrawFontBitmap2iduH(pbStartAddr,dwOffset,sFont,bitmap);
			sFont->wX += sFont->bHeight+ sFont->bTextGap;
		}

	sFont->wDisplayWidth += sFont->bTextGap;
		return  0 ;
}

int FontDrawImageDirectionLeft(ST_IMGWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	BYTE *pbStartAddr ;	
	DWORD dwOffset=0;
	int diff=0;
	
	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		{
			if (sFont->wX -sFont->bWidth <0)
				return -1;
			sFont->wX -=sFont->bWidth +sFont->bTextGap;
		}
	else if((sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		{
			if(sFont->wX -sFont->bHeight<0)
				return -1;
			sFont->wX -=sFont->bHeight;
		}
	
	if(sFont->wX & 0x01) 
		 sFont->wX&=~0x1;
	
	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_90)||(sFont->bAngle==ROTATE_RIGHT_270))
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth-2) << 1));

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		dwOffset = (psWin->wWidth - sFont->bWidth) << 1;
	else
		dwOffset = (psWin->wWidth) << 1;

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		DrawFontBitmap2iduV(pbStartAddr,dwOffset,sFont,bitmap);	
	else
		DrawFontBitmap2iduH(pbStartAddr,dwOffset,sFont,bitmap);
	
	sFont->wDisplayWidth += sFont->bTextGap;

	return  0 ;
}

int FontDrawImageDirectionUP(ST_IMGWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	BYTE *pbStartAddr ;	
	DWORD dwOffset=0;

	if(sFont->wX & 0x01) sFont->wX--;

	if(sFont->bAngle==ROTATE_RIGHT_0)
		{	
			if (sFont->wY -sFont->bHeight <0)	
				return -1;
			sFont->wY -=sFont->bHeight;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
		}
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		{
			if (sFont->wY -sFont->bHeight <0)	
				return -1;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth-2) << 1));
			sFont->wY -=sFont->bHeight;
		}
	else if(sFont->bAngle==ROTATE_RIGHT_270)
		{
			if (sFont->wY -sFont->bWidth <0)	
				return -1;
			sFont->wY -=sFont->bWidth+sFont->bTextGap;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
		}
	else if(sFont->bAngle==ROTATE_RIGHT_90)
		{
			if (sFont->wY -sFont->bWidth <0)	
				return -1;
			sFont->wY -=sFont->bWidth+sFont->bTextGap;
			if(sFont->bHeight & 0x01) sFont->bHeight--;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX-sFont->bHeight) << 1));
		}
		
	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		dwOffset = (psWin->wWidth - sFont->bWidth) << 1;
	else
		dwOffset = (psWin->wWidth) << 1;

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		DrawFontBitmap2iduV(pbStartAddr,dwOffset,sFont,bitmap);	
	else
		DrawFontBitmap2iduH(pbStartAddr,dwOffset,sFont,bitmap);

	return  0 ;
}

int FontDrawImageDirectionDown(ST_IMGWIN *psWin, ST_FONT *sFont, char *bitmap)
{
	BYTE *pbStartAddr ;	
	DWORD dwOffset=0;

	if(sFont->wX & 0x01) sFont->wX--;

	if(sFont->bAngle==ROTATE_RIGHT_0)
		{
			if (sFont->wY +sFont->bHeight >psWin->wHeight)	
				return -1;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) << 1));
			sFont->wY +=sFont->bHeight;
		}
	else if(sFont->bAngle==ROTATE_RIGHT_180)
		{
			if (sFont->wY +sFont->bHeight >psWin->wHeight)	
				return -1;
			sFont->wY +=sFont->bHeight;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX+sFont->bWidth-2) << 1));
		}
	else if(sFont->bAngle==ROTATE_RIGHT_270)
		{	
			if (sFont->wY +sFont->bWidth >psWin->wHeight)	
				return -1;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX) <<1));
			sFont->wY +=sFont->bWidth+sFont->bTextGap;
		}
	else if (sFont->bAngle==ROTATE_RIGHT_90)
		{
			if (sFont->wY +sFont->bWidth >psWin->wHeight)	
				return -1;
			if(sFont->bHeight & 0x01) sFont->bHeight--;
			pbStartAddr = (BYTE *) ((DWORD) psWin->pdwStart + (((sFont->wY + sFont->bYOffset) * psWin->wWidth + sFont->wX-sFont->bHeight) <<1));
			sFont->wY +=sFont->bWidth+sFont->bTextGap;
		}
		
	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		dwOffset = (psWin->wWidth - sFont->bWidth) << 1;
	else
		dwOffset = (psWin->wWidth ) << 1;

	if((sFont->bAngle==ROTATE_RIGHT_0)||(sFont->bAngle==ROTATE_RIGHT_180))
		DrawFontBitmap2iduV(pbStartAddr,dwOffset,sFont,bitmap);
	else
		DrawFontBitmap2iduH(pbStartAddr,dwOffset,sFont,bitmap);

	return  0 ;
}

int FontDrawImageRotation(ST_IMGWIN *psWin, ST_FONT *sFont, char *bitmap)
{
        BYTE bMask = 0,bMask1 = 0;
        BYTE *pbStartAddr;
        DWORD j,k = 0;
        DWORD dwOffset;
        unsigned char   *str, bPixel;
	int ret=0;

#if NEWFONTSTRCUCTURE
    if (sFont->bBitsPerPixel > 1)
        return FontDrawGaryImage(psWin, sFont, bitmap);
#endif

    if (g_bLargeFontFlag)//Mason 20061031
        return FontDrawImage2(psWin, sFont, bitmap);

	if((sFont->bHeight == 0)||(sFont->bWidth == 0)||(sFont->bHeight == 0xff)||(sFont->bWidth == 0xff))
	{
		MP_DEBUG(" -E- Size error");
		return 0;
	}

	if(sFont->wX & 0x01) sFont->wX++;

	sFont->wDisplayWidth += sFont->bWidth;

	if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
		return 1;
	
	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT) //right
		ret=FontDrawImageDirectionRight(psWin,sFont,bitmap);
	else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)// left
		ret=FontDrawImageDirectionLeft(psWin,sFont,bitmap);
	else if(sFont->bPrintDirection==PRINT_DIRECTION_UP)//up
		ret=FontDrawImageDirectionUP(psWin,sFont,bitmap);
	else if(sFont->bPrintDirection==PRINT_DIRECTION_DOWN)//down
		ret=FontDrawImageDirectionDown(psWin,sFont,bitmap);

    return ret;
}


void Font_DrawSpace(ST_OSDWIN *psWin,ST_FONT *sFont)
{
	if(sFont->bPrintDirection==PRINT_DIRECTION_RIGHT) //right
		{
			sFont->wX += (sFont->bFontSize>>1) + sFont->bTextGap;
			sFont->wDisplayWidth += (sFont->bFontSize>>1) + sFont->bTextGap;

			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				{
					Font_Draw2Point(psWin , sFont);	// The string too long, display ".."
				}
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_LEFT)// left
		{
			sFont->wX -= (sFont->bFontSize>>1) -sFont->bTextGap;
			sFont->wDisplayWidth += (sFont->bFontSize>>1) + sFont->bTextGap;

			if ((sFont->wWidth <= sFont->wDisplayWidth)&&(sFont->wWidth != 0))
				{
					Font_Draw2Point(psWin , sFont);	// The string too long, display ".."
				}
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_UP)//up
		{
			sFont->wY -= (sFont->bFontSize>>1) ;
		}
	else if(sFont->bPrintDirection==PRINT_DIRECTION_DOWN)//down
		{
			sFont->wY += (sFont->bFontSize>>1) ;

		}

}



void Font_Draw_Rotation(ST_IMGWIN *psWin, ST_FONT *sFont, BYTE *str, BYTE UnicodeFlag)
{
	WORD i = 0;
	BYTE *bitmap = NULL;
	DWORD get_word;
	WORD wtemp,Num;

	BYTE *pbStr = (BYTE *)str,get_byte;

	Num = Str_Length(pbStr,UnicodeFlag);

	sFont->bBitsPerPixel = 1;
	sFont->bAlign = 0;
	sFont->bYOffset = 0;

	if(UnicodeFlag)
	{
		for(i = 0; i < Num; i++)
		{
			wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
			i++;

			if(wtemp <= 0x20)		// Space
			{
				get_word = 0;
				Font_DrawSpace(psWin,sFont);
			}
			else
			{
				bitmap =(BYTE *) UnicodeGetFontBitmap(wtemp, sFont);
				MP_DEBUG("bitmap=0x%x\r\n", bitmap);

				if(bitmap != NULL)
				{
					int ret2=0;
					 ret2=FontDrawImageRotation(psWin, sFont, (char *)bitmap);
					MP_DEBUG("Native bHeight=0x%x, bWidth=0x%x",sFont->bHeight, sFont->bWidth);
					if (ret2==1)
						{
							Font_Draw2Point(psWin , sFont); 	// The string too long, display ".."
							break;//Mason 20061024	//From Lighter //Lighter 20080817 over the display area do not display anymore
						}
					else if (ret2==-1)
							break;
				}
				else		// big problem
				{
					MP_DEBUG("System do not have UNKNOWN_CHAR \r\n");
					return;
				}
			}
		}
	}
	else
	{

		for(i = 0; i < Num; i++)
		{
			get_byte = *(pbStr+i);
			MP_DEBUG("Native get_byte %x",get_byte);

			if(get_byte <= 0x20)	//Space case
			{
				get_word = 0;
				Font_DrawSpace(psWin,sFont);
			}
			else
			{
				if((get_byte <= 0x7F)||((get_byte == 0xb0)&&((*(pbStr+i+1))== 0x00)))	//check the font data need to be a 16 bit
				{
					bitmap = (BYTE *)ANSIGetFontBitmap(get_byte, sFont);
				}
				else
				{
					wtemp = 0;
					switch(g_bNCTable)
					{
#if FONT_GB2
						case LANGUAGE_S_CHINESE:
#endif
#if FONT_BIG5
						case LANGUAGE_T_CHINESE:
#endif
#if FONT_JIS
						case LANGUAGE_JAPAN:
#endif
#if FONT_KSC
						case LANGUAGE_KOREA:
#endif
							wtemp = (((*(pbStr+i))<<8)|(*(pbStr+i+1)));
							i++;
							break;
						default:
							wtemp = get_byte;
							break;
					}
					MP_DEBUG("Native word 0x%x, 0x%x, 0x%x",wtemp,(*(pbStr+i-1)), (*(pbStr+i)));

					bitmap = (BYTE *)ANSIGetFontBitmap(wtemp, sFont);
				}
				MP_DEBUG("Native bitmap=0x%x", bitmap);

				if(bitmap != NULL)
				{
					int ret2=0;

					ret2=FontDrawImageRotation(psWin, sFont, (char *)bitmap);
					MP_DEBUG("Native bHeight=0x%x, bWidth=0x%x",sFont->bHeight, sFont->bWidth);
					if (ret2==1)
						{
							Font_Draw2Point(psWin , sFont); 	// The string too long, display ".."
							break;//Mason 20061024	//From Lighter //Lighter 20080817 over the display area do not display anymore
						}
					else if (ret2==-1)
							break;
				}
				else		// big problem
				{
					MP_ALERT("Native System do not have UNKNOWN_CHAR \r\n");
					return;
				}
			}
		}
	}
}


WORD Idu_PrintStringRotation(ST_IMGWIN * trgWin, BYTE * string, WORD startX, WORD startY, BYTE UnicodeFlag,
					 WORD wWidth, BYTE Font_size,BYTE angle,BYTE direction,BYTE ColorR,BYTE ColorG,BYTE ColorB)
{
	ST_FONT ft;

	if (trgWin == NULL || trgWin->pdwStart == NULL || string == NULL)
		return;

	ft.wX = startX;
	ft.wY = startY;
	ft.bTextGap = 1;
	register DWORD dwFontColor = Idu_FontColorSet(ColorR,ColorG,ColorB);
	ft.bFontColor = (dwFontColor >> 16) & 0xff;	//YYCbCr - Y
	ft.bFontCb = (dwFontColor >> 8) & 0xff;
	ft.bFontCr = dwFontColor & 0xff;
	ft.wWidth = wWidth;
	ft.wDisplayWidth = 0;
	ft.bFontSize = Font_size;
	ft.bWidth = DEFAULT_FONT_SIZE;
	ft.bHeight = DEFAULT_FONT_SIZE;
	ft.bAngle=angle;
	ft.bPrintDirection=direction;
	ft.bBitsPerPixel = 1;
	ft.bAlign = 0;
	ft.bYOffset = 0;
	
	// check if out of win
	if (startX >trgWin->wWidth)
		return 0;
	if (startY > trgWin->wHeight)
		return 0;

	Font_Draw_Rotation(trgWin, &ft, (BYTE *)string, UnicodeFlag);

	//if(ft.wX < trgWin->wWidth)
		//return ft.wX;
	//else
		return 0 ;

}

void OsdFontRotationTest()
{
	WORD str0[] = {0x5146,0x5B8F, 0x96FB, 0x5B50, ' ', 'M', 'a', 'g', 'i', 'c', 'P', 'i', 'x', 'e', 'l',' '};
	BYTE UniFlag=0;
	BYTE Fontsize=18;

#if 0 //OSD font test
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 30, 0, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_DOWN);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 30, 0, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_DOWN);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 70, 0, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_DOWN);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 100, 0, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_DOWN);

	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 150, 600, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_UP);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 150, 600, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_UP);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 190, 600, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_UP);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 220, 600, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_UP);

	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 800, 0, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_LEFT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 800, 30, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_LEFT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 800, 90, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_LEFT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 800, 90, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_LEFT);

	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 280, 470, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_RIGHT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 280, 500, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_RIGHT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 280, 560, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_RIGHT);
	Idu_OsdPrintRotation(Idu_GetOsdWin(), "MagicPixel Test= 123/<> [ABC]", 280, 560, 1,UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_RIGHT);
#endif
#if 1 //IDU font test	
	//g_bNCTable = 12;
	//UniFlag=1;
	//FontReload();
	//g_bNCTable=1;
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 620, 0, UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_RIGHT,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 720, 70, UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_RIGHT,255,255,255);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 720, 80, UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_RIGHT,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 720, 110, UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_RIGHT,255,255,255);
	
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 100, 0, UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_LEFT,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 100, 60, UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_LEFT,255,255,255);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 200, 140, UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_LEFT,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 200, 170, UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_LEFT,255,255,255);

	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 200, 100, UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_UP,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 230, 100, UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_UP,255,255,255);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 290, 100, UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_UP,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 290, 100, UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_UP,255,255,255);

	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 300, 400, UniFlag,0,Fontsize,ROTATE_RIGHT_0,PRINT_DIRECTION_DOWN,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 400, 400, UniFlag,0,Fontsize,ROTATE_RIGHT_180,PRINT_DIRECTION_DOWN,255,255,255);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 500, 400, UniFlag,0,Fontsize,ROTATE_RIGHT_90,PRINT_DIRECTION_DOWN,0,0,0);
	Idu_PrintStringRotation(Idu_GetCurrWin(), "MagicPixel Test= 123/<> [ABC]", 500, 400, UniFlag,0,Fontsize,ROTATE_RIGHT_270,PRINT_DIRECTION_DOWN,255,255,255);
#endif
__asm("break 100");

}

#endif //  Rotation(NEWFONTSTRCUCTURE) 
#endif

