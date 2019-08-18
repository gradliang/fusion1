

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "display.h"


#define YYCbCr_Y0(c) ((c >> 24) & 0xff)
#define YYCbCr_Y1(c) ((c >> 16) & 0xff)
#define YYCbCr_Cb(c) ((c >> 8) & 0xff)
#define YYCbCr_Cr(c) (c & 0xff)
#define SetYYCbCr(cy0, cy1, cb, cr) (((cy0 << 24) | (cy1 << 16) | (cb << 8) | cr))
static IMAGEEFFECT EffectValue;


//-------------------------------------------------------------------
// Image Special Effect Functions
//-------------------------------------------------------------------

#if ( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
int mpResetIPUEffectValue()
{
    MP_DEBUG("%s", __func__);
    
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    mpSetIPUEffectValue(Value);
    
	return 0;
}

int mpSetIPUEffectValue(IMAGEEFFECT Value)
{
    MP_DEBUG("%s", __func__);
	EffectValue.Edge_Enable  = Value.Edge_Enable;
	EffectValue.EdgeGain     = Value.EdgeGain;

	EffectValue.Blur_Enable  = Value.Blur_Enable;
	EffectValue.BlurGain     = Value.BlurGain;

	EffectValue.Sketch_Enable= Value.Sketch_Enable;
	EffectValue.SketchMode   = Value.SketchMode;

	EffectValue.BW_Enable    = Value.BW_Enable;

	EffectValue.CM_Enable    = Value.CM_Enable;
	EffectValue.CM_Cb_Mode	 = Value.CM_Cb_Mode;
	EffectValue.CM_Cr_Mode	 = Value.CM_Cr_Mode;

	EffectValue.Y_Gain_Enable= Value.Y_Gain_Enable;
	EffectValue.Y_Gain 		 = Value.Y_Gain;

	EffectValue.Y_Offset_Enable= Value.Y_Offset_Enable;
	EffectValue.Y_Offset	 = Value.Y_Offset;

	EffectValue.CbCrOffset_Enable= Value.CbCrOffset_Enable;
	EffectValue.Cb_Offset	 	= Value.Cb_Offset;
	EffectValue.Cr_Offset	 	= Value.Cr_Offset;

	EffectValue.SEHG_Enable		= Value.SEHG_Enable;
	EffectValue.SEHG_Offset		= Value.SEHG_Offset;
	EffectValue.SEHG_Gain		= Value.SEHG_Gain;

	EffectValue.Noise_Enable		= Value.Noise_Enable;
	EffectValue.NoiseGain		= Value.NoiseGain;

	EffectValue.Bilevel_Enable		= Value.Bilevel_Enable;
	EffectValue.BilevelGain		= Value.BilevelGain;
	EffectValue.Bilevel_cbralsoGain		= Value.Bilevel_cbralsoGain;
	EffectValue.Reverse_Enable		= Value.Reverse_Enable;

	EffectValue.Crayon_Enable		= Value.Crayon_Enable;

	EffectValue.IpuGamma_Enable		= Value.IpuGamma_Enable;
	EffectValue.IpuGamma_Step		= Value.IpuGamma_Step;

	int i;
	for(i=0 ; i<34 ;i++)
	{
		EffectValue.IpuGammaValue[i]		= Value.IpuGammaValue[i];
	}

	return 0;
}


int mpGetIPUEffectValue(IMAGEEFFECT *Value)
{
    //MP_DEBUG("%s", __func__); // log too many
    
	Value->Edge_Enable = EffectValue.Edge_Enable;
	Value->EdgeGain=	EffectValue.EdgeGain;

	Value->Blur_Enable=	EffectValue.Blur_Enable;
	Value->BlurGain = 	EffectValue.BlurGain;

	Value->Sketch_Enable=	EffectValue.Sketch_Enable;
	Value->SketchMode   = 	EffectValue.SketchMode;

	Value->BW_Enable    = 	EffectValue.BW_Enable;

	Value->CM_Enable    = 	EffectValue.CM_Enable;
	Value->CM_Cb_Mode    = 	EffectValue.CM_Cb_Mode;
	Value->CM_Cr_Mode    = 	EffectValue.CM_Cr_Mode;

	Value->Y_Gain_Enable	= 	EffectValue.Y_Gain_Enable;
	Value->Y_Gain	 		= 	EffectValue.Y_Gain;

	Value->Y_Offset_Enable	= 	EffectValue.Y_Offset_Enable;
	Value->Y_Offset			= 	EffectValue.Y_Offset;

	Value->CbCrOffset_Enable= 	EffectValue.CbCrOffset_Enable;
	Value->Cb_Offset		= 	EffectValue.Cb_Offset;
	Value->Cr_Offset		= 	EffectValue.Cr_Offset;

	Value->SEHG_Enable		= 	EffectValue.SEHG_Enable;
	Value->SEHG_Offset		= 	EffectValue.SEHG_Offset;
	Value->SEHG_Gain		= 	EffectValue.SEHG_Gain;

	Value->Noise_Enable		= 	EffectValue.Noise_Enable;
	Value->NoiseGain		= 	EffectValue.NoiseGain;
	Value->Bilevel_Enable	= 	EffectValue.Bilevel_Enable;
	Value->BilevelGain		= 	EffectValue.BilevelGain;
	Value->Bilevel_cbralsoGain= 	EffectValue.Bilevel_cbralsoGain;
	Value->Reverse_Enable	= 	EffectValue.Reverse_Enable;
	Value->Crayon_Enable	= 	EffectValue.Crayon_Enable;

	Value->IpuGamma_Enable	= 	EffectValue.IpuGamma_Enable;
	Value->IpuGamma_Step	= 	EffectValue.IpuGamma_Step;
	int i;
	for(i=0 ; i<34 ;i++)
		{
		Value->IpuGammaValue[i]= 	EffectValue.IpuGammaValue[i];
		}
	return 0;

}

int mpSE_Off(void)//ok
{
    MP_DEBUG("%s", __func__);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Sketch(BOOL level)//ok
{
    MP_DEBUG("%s: level = %d", __func__, level);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.Sketch_Enable = 1;
    Value.SketchMode = level; // range : 0(thinner)..255(thicker)
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Gray(void)//ok - BlackWhite
{
    MP_DEBUG("%s", __func__);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.BW_Enable = 1;
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Sepia(void)//ok
{
    MP_DEBUG("%s", __func__);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.CM_Enable = 1;
	Value.CM_Cb_Mode= -128;
	Value.CM_Cr_Mode=  30;
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Negative(void)//ok
{
    MP_DEBUG("%s", __func__);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.Reverse_Enable = 1;
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Edge(void)//ok
{
    MP_DEBUG("%s", __func__);
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.Sketch_Enable = 1;
    Value.Reverse_Enable = 1;
    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_RGB_stretch(void)//ok
{
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
	//gamma = 1
	unsigned char IPUGammaTable1[34]= {0x00,0x08,0x10,
								   0x18,0x20,0x28,0x30,
								   0x38,0x40,0x48,0x50,
								   0x58,0x60,0x68,0x70,
								   0x78,0x80,0x88,0x90,
								   0x98,0xA0,0xA8,0xB0,
								   0xB8,0xC0,0xC8,0xD0,
								   0xD8,0xE0,0xE8,0xF0,
								   0xF8,0xFF};

	Value.IpuGamma_Enable = 1;
	Value.IpuGamma_Step =0;//0:32 level ; 1:16level
	int i;
	for(i=0 ; i<34 ;i++)
		{
		Value.IpuGammaValue[i] = IPUGammaTable1[i];
		}


    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Solarzation(void)//
{
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));

	unsigned char IPUGammaTable1[34]=
	{0x0f,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78,0x87,0x96,0xA5,0xB4,0xC3,0xD2,0xE1,0xF0,0xff,
	0xF0,0xE1,0xD2,0xC3,0xB4,0xA5,0x96,0x87,0x78,0x69,0x5A,0x4B,0x3C,0x2D,0x1E,0x0f};

    Value.BW_Enable = 1;
    Value.Reverse_Enable = 1;
	Value.IpuGamma_Enable = 1;
	Value.IpuGamma_Step =0;//0:32 level ; 1:16level
	int i;
	for(i=0 ; i<34 ;i++)
		{
			Value.IpuGammaValue[i] = IPUGammaTable1[i];
		}

    mpSetIPUEffectValue(Value);
	return 0;
}

int mpSE_Crayon(void)//ok
{
    IMAGEEFFECT  Value;
    memset(&Value,0,sizeof(IMAGEEFFECT));
    Value.Crayon_Enable = 1;
    mpSetIPUEffectValue(Value);
	return 0;
}

#endif

//-------------------------------------------------------------------
// Image Copy Functions
//-------------------------------------------------------------------
void mpCopyWin(ST_IMGWIN * pDstWin, ST_IMGWIN * pSrcWin)
{
	//Ipu_ImageScaling(pSrcWin, pDstWin, pDstWin->wWidth, pDstWin->wHeight, 0, 0);
	if ((pDstWin->dwOffset==pSrcWin->dwOffset)&&(pDstWin->dwOffset==pDstWin->wWidth<<1)&&(pSrcWin->dwOffset==pSrcWin->wWidth<<1))
		mmcp_memcpy(pDstWin->pdwStart,pSrcWin->pdwStart,pDstWin->dwOffset*pDstWin->wHeight);
	else if ((pDstWin->wWidth==pSrcWin->wWidth)&& (pDstWin->wHeight==pSrcWin->wHeight))
		mpCopyWinAreaSameSize(pSrcWin, pDstWin, 0, 0, 0, 0, pDstWin->wWidth, pDstWin->wHeight);
	else
		Ipu_ImageScaling(pSrcWin, pDstWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight, 0, 0, pDstWin->wWidth, pDstWin->wHeight, 0);
}

void mpCopyEqualWin(ST_IMGWIN * pDstWin, ST_IMGWIN * pSrcWin)
{
#if 0
    //Ipu_ImageScaling(pSrcWin, pDstWin, pDstWin->wWidth, pDstWin->wHeight, 0, 0);
    //Ipu_ImageScaling(pSrcWin, pDstWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight, 0, 0, pDstWin->wWidth, pDstWin->wHeight, 0);
		mmcp_memcpy(pDstWin->pdwStart,pSrcWin->pdwStart,pDstWin->dwOffset*pDstWin->wHeight);
#else
    mpCopyWin(pDstWin, pSrcWin);
#endif
}
void mpCopyWinArea(ST_IMGWIN * pDstWin, ST_IMGWIN * pSrcWin, SWORD left, SWORD top, WORD right,
					WORD bottom)
{
#if 0
    //image_scale(pSrcWin, pDstWin, left, top, right, bottom, left, top, right, bottom);
	Ipu_ImageScaling(pSrcWin, pDstWin, left, top, right, bottom, left, top, right, bottom,0);
#else
	//register DWORD *pdwSrcBuffer = (DWORD *) ((DWORD) pSrcWin->pdwStart | 0xa0000000);
	//register DWORD *pdwDstBuffer = (DWORD *) ((DWORD) pDstWin->pdwStart | 0xa0000000);
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset;

    left = (left + 1) & 0xFFFE;
    right = (right + 1) & 0xFFFE;

	if (pDstWin == pSrcWin || left >= pDstWin->wWidth || top >= pDstWin->wHeight)
		return;

	if (top < 0)
		top = 0;

	if (left < 0)
		left = 0;

#if 0//( (CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660) )
	pdwSrcBuffer += top * pSrcWin->dwOffset;
	pdwSrcBuffer += left;
	pdwDstBuffer += top * pDstWin->dwOffset;
	pdwDstBuffer += left;

	if (right >= pDstWin->wWidth)  // 07.13.2006 Athena - avoid out of win
		right = pDstWin->wWidth;

	if (bottom >= pDstWin->wHeight)
		bottom = pDstWin->wHeight;

#if 1
    mmcp_block((BYTE *) pdwSrcBuffer, (BYTE *) pdwDstBuffer,
               bottom - top + 1, (right - left + 1) << 1,
               pSrcWin->dwOffset, pDstWin->dwOffset);
#else
//   It is same as last line mmcp_block  Tech 20140728
	register DWORD y;
	DWORD dwLineWidth = (right - left) << 1;

	if (dwLineWidth == 0)
		return;

	for (y = top; y < bottom; y++)
	{
		memcpy(pdwDstBuffer, pdwSrcBuffer, dwLineWidth);
		pdwSrcBuffer += dwSrcOffset;
		pdwDstBuffer += dwDstOffset;
	}
#endif

#else
	dwSrcOffset = pSrcWin->dwOffset >> 2;

	pdwSrcBuffer += top * dwSrcOffset;
	pdwSrcBuffer += left >> 1;

	dwDstOffset = pDstWin->dwOffset >> 2;

	pdwDstBuffer += top * dwDstOffset;
	pdwDstBuffer += left >> 1;

	if (right >= pDstWin->wWidth)  // 07.13.2006 Athena - avoid out of win
		right = pDstWin->wWidth;

	if (bottom >= pDstWin->wHeight)
		bottom = pDstWin->wHeight;

	register DWORD y;
	DWORD dwLineWidth = (right - left) << 1;

	if (dwLineWidth == 0)
		return;

	for (y = top; y < bottom; y++)
	{
		memcpy(pdwDstBuffer, pdwSrcBuffer, dwLineWidth);
		pdwSrcBuffer += dwSrcOffset;
		pdwDstBuffer += dwDstOffset;
	}
#endif

#endif
}

void mpCopyWinAreaSameSize( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin,WORD wSx,WORD wSy,WORD wDx,WORD wDy,WORD wW,WORD wH)
{
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;

	pdwSrcBuffer += (wSx>>1);
	pdwSrcBuffer += wSy * (pSrcWin->dwOffset>>2);
	pdwDstBuffer += (wDx>>1);
	pdwDstBuffer += wDy * (pDstWin->dwOffset>>2);

    mmcp_block_polling((BYTE *) pdwSrcBuffer, (BYTE *) pdwDstBuffer,wH, wW << 1,pSrcWin->dwOffset, pDstWin->dwOffset);
}

#if USBCAM_TECH_MODE
void myCopyWinPartSameArea( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin)
{
	DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset,i,k;
	WORD wWidth,wHeight;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	dwDstOffset=pDstWin->dwOffset>>2;
	wHeight=pDstWin->wHeight;
	wWidth=pDstWin->wWidth>>2;
	if (pSrcWin->wWidth>pDstWin->wWidth)
		pdwSrcBuffer+=((pSrcWin->wWidth-pDstWin->wWidth)>>2);

	for (i=0;i<wHeight;i++)
	{
		TaskYield();
		for (k=0;k<wWidth;k++)
		{
			pdwDstBuffer[k]=pdwSrcBuffer[k];
		}
		pdwDstBuffer+=dwDstOffset;
		pdwSrcBuffer+=dwSrcOffset;
	}
	
}
#endif

#if MINIDV_YBD_FUNCION
void myCopyWinPart( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin)
{
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset,i,k;
	WORD wWidth,wHeight;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	dwDstOffset=pDstWin->dwOffset>>2;
	wHeight=pSrcWin->wHeight;
	wWidth=pSrcWin->wWidth>>1;

	for (i=0;i<wHeight;i+=2)
	{
		for (k=0;k<wWidth;k+=2)
		{
			pdwDstBuffer[k>>1]=pdwSrcBuffer[k];
		}
		pdwDstBuffer+=dwDstOffset;
		pdwSrcBuffer+=(dwSrcOffset<<1);
	}
	
}

void WinVirtualScaleDown( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin,WORD wDx,WORD wDy,WORD wDw,WORD wDh)
{
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset,i,k;
	WORD wWidth,wHeight;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	dwDstOffset=pDstWin->dwOffset>>2;
	wHeight=wDh;
	wWidth=wDw>>1;

	pdwDstBuffer+=(wDx>>1);
	pdwDstBuffer+=dwDstOffset*wDy;
	for (i=0;i<wHeight;i++)
	{
		for (k=0;k<wWidth;k++)
		{
			pdwDstBuffer[k]=pdwSrcBuffer[k*(pSrcWin->wWidth/wDw)];
		}
		pdwDstBuffer+=dwDstOffset;
		pdwSrcBuffer+=(dwSrcOffset*(pSrcWin->wHeight/wDh));
	}
	
}

void WinVirtualScaleUp( ST_IMGWIN * pSrcWin,ST_IMGWIN * pDstWin,WORD wDx,WORD wDy,WORD wDw,WORD wDh)
{
	register DWORD *pdwSrcBuffer = (DWORD *) pSrcWin->pdwStart;
	register DWORD *pdwDstBuffer = (DWORD *) pDstWin->pdwStart;
	register DWORD dwSrcOffset, dwDstOffset,i,k;
	WORD wWidth,wHeight;

	dwSrcOffset=pSrcWin->dwOffset>>2;
	dwDstOffset=pDstWin->dwOffset>>2;
	wHeight=wDh;
	wWidth=wDw>>1;

	pdwDstBuffer+=(wDx>>1);
	pdwDstBuffer+=dwDstOffset*wDy;
	for (i=0;i<wHeight;i++)
	{
		for (k=0;k<wWidth;k++)
		{
			pdwDstBuffer[k]=pdwSrcBuffer[k/(wDw/pSrcWin->wWidth)];
		}
		pdwDstBuffer+=dwDstOffset;
		//pdwSrcBuffer+=(dwSrcOffset*(pSrcWin->wHeight/wDh));
		pdwSrcBuffer=pSrcWin->pdwStart+(dwSrcOffset*(i*pSrcWin->wHeight/wHeight));
	}
	
}

#endif


BOOL mpCheckBufferContent(BYTE *pImageBuffer, ST_IMGWIN *pWin, DWORD left, DWORD top, WORD w, WORD h)
{
	if (pWin == NULL) return FALSE;

	register DWORD *pdwImgBuffer = (DWORD *)((DWORD)pImageBuffer | 0xa0000000);
    register DWORD *pdwWinBuffer = (DWORD *)((DWORD)pWin->pdwStart | 0xa0000000);
    register DWORD dwSrcOffset = pWin->dwOffset >> 2;
	WORD w1 = w >> 1;  //Mason 20060619  //From Athena

    pdwWinBuffer += top * dwSrcOffset;
    pdwWinBuffer += left >> 1;


    //DWORD bottom = top + h;
    //if (bottom >= pWin->wHeight) bottom = pWin->wHeight;

    register DWORD x;


	// check first horizontal line and last line
    for (x = 0; x < w1; x++, pdwImgBuffer++)
    {
        if (*(pdwWinBuffer + x) != *pdwImgBuffer) {
			//mpPrintMessage("buffer changed");
			return FALSE;
        }
    }

	return TRUE;
}

void mpWinInit(ST_IMGWIN * psWin, DWORD * pdwStart, WORD wHeight, WORD wWidth)
{
	psWin->pdwStart = pdwStart;
	psWin->dwOffset = wWidth * 2;
	psWin->wWidth = wWidth;
	psWin->wHeight = wHeight;
	psWin->wX = psWin->wY = 0;
	Idu_ResetWinClipRegion(psWin);
}

void mpPaintWin(ST_IMGWIN * win, DWORD dwColor)
{
	Idu_PaintWin(win, dwColor);
}

void mpPaintWinAreaAntiBlending(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor, BYTE bBlend)
{
    //mpDebugPrint("mpPaintWinAreaAntiBlending(x=%d, y=%d, w=%d, h=%d, dwColor=0x%X, bBlend=%d)", x, y, w, h, dwColor, bBlend);
	DWORD i, j;

	DWORD *pdwScreenBuffer, *src;
	DWORD dwOffset;
	WORD hw;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	pdwScreenBuffer = ImgWin->pdwStart;
	pdwScreenBuffer += (dwOffset * y) + (x >> 1);

	DWORD cy0 = YYCbCr_Y0(dwColor) * bBlend;
	DWORD cy1 = YYCbCr_Y1(dwColor) * bBlend;
	DWORD cb = YYCbCr_Cb(dwColor) * bBlend;
	DWORD cr = YYCbCr_Cr(dwColor) * bBlend;
	//mpDebugPrint("cy0=0x%X, cy1=0x%X, cb=0x%X, cr=0x%X", cy0, cy1, cb, cr);
	BYTE bBlend1 = 256 - bBlend;
	hw = w >> 1;
	//mpDebugPrint("bBlend1=%d, hw=%d", bBlend1, hw);
	for (i = h - 1; i > 0; i--)
	{
		for (j = 0; j < hw; j++)
		{
			DWORD cDst = *(pdwScreenBuffer + j);
            //if(j==0 && i==319)
			//{
			//     mpDebugPrint("j=%d, i=%d", j, i);
			//     mpDebugPrint("cDst=0x%X", cDst);
			//}
			WORD  dst_cy0, dst_cy1, dst_cb, dst_cr;

			dst_cy0 = (((YYCbCr_Y0(cDst)*0x100)-cy0) / bBlend1);
			dst_cy1 = (((YYCbCr_Y1(cDst)*0x100)-cy1) / bBlend1);
			dst_cb  = (((YYCbCr_Cb(cDst)*0x100)- cb) / bBlend1);
			dst_cr  = (((YYCbCr_Cr(cDst)*0x100)- cr) / bBlend1);

			cDst = ((dst_cy0 & 0xff) << 24) | ((dst_cy1 & 0xff) << 16) | ((dst_cb & 0xff) << 8) | (dst_cr & 0xff);

			*(pdwScreenBuffer + j) = cDst;
		}

		pdwScreenBuffer += dwOffset;
	}
}

void mpPaintWinAreaBlending(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor, BYTE bBlend)
{
	DWORD i, j;

	DWORD *pdwScreenBuffer, *src;
	DWORD dwOffset;
	WORD hw;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	pdwScreenBuffer = ImgWin->pdwStart;
	pdwScreenBuffer += (dwOffset * y) + (x >> 1);

	DWORD cy0 = YYCbCr_Y0(dwColor) * bBlend;
	DWORD cy1 = YYCbCr_Y1(dwColor) * bBlend;
	DWORD cb = YYCbCr_Cb(dwColor) * bBlend;
	DWORD cr = YYCbCr_Cr(dwColor) * bBlend;
	BYTE bBlend1 = 256 - bBlend;
	hw = w >> 1;

	for (i = h - 1; i > 0; i--)
	{
		for (j = 0; j < hw; j++)
		{
			DWORD cDst = *(pdwScreenBuffer + j);
			WORD  dst_cy0, dst_cy1, dst_cb, dst_cr;

			dst_cy0 = (cy0 + YYCbCr_Y0(cDst) * bBlend1) >> 8;
			dst_cy1 = (cy1 + YYCbCr_Y1(cDst) * bBlend1) >> 8;
			dst_cb = (cb + YYCbCr_Cb(cDst) * bBlend1) >> 8;
			dst_cr = (cr + YYCbCr_Cr(cDst) * bBlend1) >> 8;

			cDst = ((dst_cy0 & 0xff) << 24) | ((dst_cy1 & 0xff) << 16) | ((dst_cb & 0xff) << 8) | (dst_cr & 0xff);
			*(pdwScreenBuffer + j) = cDst;
		}

		pdwScreenBuffer += dwOffset;
	}
}

//clear window content with colour black
void mpClearWin(ST_IMGWIN * win)
{
	Idu_PaintWin(win, 0x00008080);
}

void mpPaintWinArea(ST_IMGWIN * ImgWin, WORD x, WORD y, WORD w, WORD h, DWORD dwColor)
{
	DWORD i;

	DWORD *ptr, *src;
	DWORD dwOffset;

	if (ImgWin == NULL || ImgWin->pdwStart == NULL)
		return;
	dwOffset = ImgWin->wWidth >> 1;
	ptr = ImgWin->pdwStart;
	ptr += (dwOffset * y) + (x >> 1);

	src = ptr;
	for (i = w >> 1; i > 0; i--)
	{
		*ptr++ = dwColor;
	}

	ptr = src;
	ptr += dwOffset;
	for (i = h - 1; i > 0; i--)
	{
		memcpy(ptr, src, w << 1);
		ptr += dwOffset;
	}

}


void mpDraw1BitBitmap(ST_IMGWIN * pWin, WORD x, WORD y, WORD w, WORD h, BYTE * pImgBuffer,
						BYTE bColor)
{
	DWORD *pdwPixel;
	DWORD dwLineAddr;
	WORD wWidth;
	BYTE bImgByte;
	BYTE bLineBytesWidth = (w & 0x7) ? ((w >> 3) + 1) : (w >> 3);
	BYTE i;
	DWORD c40, c80, cC0;

	if (x & 1)
		x--;
	if ((pWin->pdwStart == NULL) || (x >= pWin->wWidth) || (y >= pWin->wHeight))
		return;

	c40 = bColor << 16;
	c80 = bColor << 24;
	cC0 = c40 | c80;

	dwLineAddr = ((DWORD) (pWin->pdwStart) + ((y * pWin->wWidth + x) << 1));
	while (h)
	{
		wWidth = bLineBytesWidth;
		pdwPixel = (DWORD *) dwLineAddr;

		while (wWidth)
		{
			bImgByte = *pImgBuffer;

			if (bImgByte == 0)
				pdwPixel += 4;
			else
			{

				for (i = 0; i < 4; i++)
				{
					switch (bImgByte & 0xc0)
					{
					case 0:
						break;
					case 0x40:
						*pdwPixel &= 0xff00ffff;
						*pdwPixel |= c40;
						break;
					case 0x80:
						*pdwPixel &= 0x00ffffff;
						*pdwPixel |= c80;
						break;
					case 0xc0:
						*pdwPixel &= 0x0000ffff;
						*pdwPixel |= cC0;
						break;
					}
					bImgByte <<= 2;
					pdwPixel++;
				}

			}
			wWidth--;
			pImgBuffer++;
		}

		h--;
		y++;
		dwLineAddr += pWin->dwOffset;
	}

}


uint16_t VerticalOffset = 0;
uint16_t HorizontalOffset = 0;

#if 0
uint16_t ViewWidth = 640;
uint16_t ViewHeight = 480;




uint16_t GetScreenHeight()
{
	return ViewHeight;
}



uint16_t GetScreenWidth()
{
	return ViewWidth;
}


#endif
void GetScreenOffset(uint16_t * vertical, uint16_t * horizontal)
{
	*vertical = VerticalOffset;
	*horizontal = HorizontalOffset;
}





int DisplayWinPosition(ST_IMGWIN * pWin, ST_IMGWIN * pMovieWin)
{
	WORD TimeX0, TimeX1, TimeY;
	WORD outerOffsetH = 0, outerOffsetV = 0, offsetX = 0, offsetY = 0;

	GetScreenOffset(&outerOffsetV, &outerOffsetH);	// get the offset from actual buffer to viewed frame in vertical direction
	if (pMovieWin->wWidth > (pWin->dwOffset / 2))
	{							//Meng 2005-7-11 adjust IMG_WIN.dwOffset
		//JohnnyChen modified for image_width > win_offset 2005-01-13.
		pMovieWin->wWidth = pWin->dwOffset / 2;	//Meng 2005-7-11 adjust IMG_WIN.dwOffset
		//MP_DEBUG(" movie frame size too large!resize to win->Offset \r\n");

	}

	// position the movie display window
	pMovieWin->dwOffset = pWin->dwOffset;

	if (pMovieWin->wWidth < pWin->wWidth)
		offsetX = outerOffsetH + ((pWin->wWidth - pMovieWin->wWidth) >> 1);
	else
	{
		offsetX = (pMovieWin->wWidth - pWin->wWidth) >> 1;
		if (offsetX > outerOffsetH)
			offsetX = outerOffsetH;
		else
			offsetX = outerOffsetH - offsetX;
	}

	if (pMovieWin->wHeight < pWin->wHeight)
		offsetY = outerOffsetV + ((pWin->wHeight - pMovieWin->wHeight) >> 1);
	else
	{
		offsetY = (pMovieWin->wHeight - pWin->wHeight) >> 1;
		if (offsetY > outerOffsetV)
			offsetY = 0;
		else
			offsetY = outerOffsetV - offsetY;
	}


	pMovieWin->pdwStart = pWin->pdwStart + ((offsetY * (pWin->dwOffset / 2) + offsetX) >> 1);

#if 0
	// position the movie time displaying site
	if (pMovieWin->wWidth < pWin->wWidth)
	{
		TimeX0 = (pWin->wWidth - pMovieWin->wWidth) >> 1;
		//TimeX1 = TimeX0 + pMovieWin->Width - MOVIE_TIME_WIDTH - MOVIE_TIME_FRAME_WIDTH;
		TimeX1 = TimeX0 + (pMovieWin->wWidth * 41 / 100);
	}
	else
	{
		TimeX0 = 0;
		TimeX1 = TimeX0 + pWin->wWidth - MOVIE_TIME_WIDTH - MOVIE_TIME_FRAME_WIDTH;
	}

	if (pMovieWin->wHeight < pWin->wHeight - MOVIE_TIME_HEIGHT)
		TimeY = offsetY - outerOffsetV + pMovieWin->wHeight + 2;	// put inside the display win
	else
		TimeY = pWin->wHeight - MOVIE_TIME_HEIGHT + 2;	// put outside the display win
#endif
	return 1;
}

#if 0

void
ImgFill(ST_IMGWIN * win, uint16_t top, uint16_t left, uint16_t width, uint16_t height,
		uint32_t color)
{
	register uint32_t *pixel, line;

	// modify the horizontal vector to even pixel boundary
	left = left & 0xfffe;
	width = width + (width & 1);

	if (left > win->wWidth)
		return;
	if (top > win->wHeight)
		return;
	if (left + width > win->wWidth)
		width = win->wWidth - left;
	if (top + height > win->wHeight)
		height = win->wHeight - top;

	pixel = win->pdwStart + (top * (win->dwOffset / 2) + left) / 2;	// devide 2 because a U32 contain 2 pixel
	while (height)
	{
		line = width;
		while (line)
		{
			*pixel = color;
			pixel++;
			line -= 2;
		}
		pixel = pixel + ((win->dwOffset / 2) - width) / 2;
		height--;
	}
}
#endif

//following function is picked up from video.c function videoplay()
int WinInit4VideoPlay(ST_IMGWIN * pWin, ST_IMGWIN * pMovieWin)
{
	//ImgFill(win, 0, 0, 640, 480, 0xffff8080);

	if (!DisplayWinPosition(pWin, pMovieWin))
		return 0;
	//ImgPaintArea(pWin, pWin->wWidth, pWin->wHeight, RGB2YUV(0, 0, 0));
	return 1;

}

