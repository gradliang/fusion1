
/*
 	define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "xpg.h"
#include "display.h"
//#include "xpgFunc.h"
#include "taskid.h"
#include "ui.h"
#include "util.h"
#include "../../image/include/jpeg.h"
#include "../../record/include/record.h"

#if ( ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650) || ((CHIP_VER & 0xFFFF0000) == CHIP_VER_660))
#define memcpy                  mmcp_memcpy
#endif

#define RemoveTimerProc         Ui_TimerProcRemove


//---------------------------------------------------------------------------
// xpg draw functions
//---------------------------------------------------------------------------
extern SWORD(*drawSpriteFunctions[]) (ST_IMGWIN *, STXPGSPRITE *, BOOL); // for xpgUpdateStage()

#pragma alignvar(4)

//---------------------------------------------------------------------------
ST_IMGWIN g_sXpgCanvasWin;
ST_IMGWIN *g_pXpgCanvasWin;

// Calling in main.c XpgInit()
void xpgInitCanvas()
{
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	ST_IMGWIN *pWin;
	DWORD w, h;
	w = pstMovie->m_wScreenWidth;
	h = pstMovie->m_wScreenHeight;
	
	pWin = Idu_GetNextWin();
	ImgWinInit(&g_sXpgCanvasWin, SystemGetMemAddr(DISPLAY_BUF2_MEM_ID), h, w);
	g_pXpgCanvasWin = &g_sXpgCanvasWin;

}

void xpgUpdateCanvas(void)
{
  
	ST_IMGWIN *pDestWin = Idu_GetCurrWin();

	if (g_pXpgCanvasWin != pDestWin)
	{
		Idu_ChgWin(g_pXpgCanvasWin);
	}

}


//------------------------------------------------------------------------------
///
///@ingroup xpgDraw
///@brief   Update Stage Clip by Sprite
///
///@param   pWin - selected image Win
///
///@param   pstSprite - pointer of input Sprite
///
///@param   boCopyToCurrWin - booloean of CopyToCurrWin - true or not
///
///@return
///
void xpgUpdateStageClipBySprite(ST_IMGWIN * pWin, register STXPGSPRITE * pstSprite, BOOL boCopyToCurrWin)
{
	if (pstSprite == NULL || pWin == NULL || g_stXpgMovie.m_bUpdateStage)
		return;
	WORD x = pstSprite->m_wPx;
	WORD y = pstSprite->m_wPy;

	xpgUpdateStageClip(pWin, x, y, x + pstSprite->m_wWidth, y + pstSprite->m_wHeight, boCopyToCurrWin);
}

//---------------------------------------------------------------------------
///
///@ingroup xpgDraw
///@brief   Update Stage Clip
///
///@param   pWin - selected image Win
///
///@param   left, top, right, bottom
///
///@param   boCopyToCurrWin - booloean of Copy to Current Win
///
///@return
///
void xpgUpdateStageClip(ST_IMGWIN * pWin, WORD left, WORD top, WORD right, WORD bottom, BOOL boCopyToCurrWin)
{
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	//if (boScalerBusy)    TaskYield();

	if (pstMovie == NULL || pWin == NULL)
		return;
    if ((g_bAniFlag & ANI_VIDEO) != 0)
    {
        MP_ALERT("video playing - xpgUpdateStageClip request skip!");
        return;
    }
#if ERROR_HANDLER_ENABLE
	if (g_boShowDialog)
		return;
#endif
#if AUTO_DEMO
	if (BootUpAutoDemoFlag > 1)
		return;
#endif
	BOOL boVideoPlaying = (!(g_bAniFlag & ANI_AUDIO)) && (g_psSystemConfig->sVideoPlayer.dwPlayerStatus & MOVIE_STATUS_PLAY);

	if (boVideoPlaying)	{
		MP_DEBUG("video playing updateStageClip");
		//return;
		//Api_MoviePause();
	}

	MP_DEBUG("Update Clip");
	register DWORD i;
	register STXPGSPRITE *pstSprite;
	SWORD swRet = PASS;

	if (left >= pstMovie->m_wScreenWidth || top >= pstMovie->m_wScreenHeight)
		return;
	if (right >= pstMovie->m_wScreenWidth)
		right = pstMovie->m_wScreenWidth - 1;
	if (bottom >= pstMovie->m_wScreenHeight)
		bottom = pstMovie->m_wScreenHeight - 1;
	if (left & 1)
		left--;
	if (right & 1)
		right++;
	pstMovie->m_bUpdateStage = true;
	pstMovie->m_boClip = true;
	Idu_SetWinClipRegion(pWin, left, top, right, bottom);

	BYTE bFirstSprite = 0;

	for (i = bFirstSprite; i < pstMovie->m_dwSpriteCount; i++)
	{
		pstSprite = &(pstMovie->m_astSprite[i]);
		if (pstSprite->m_boVisible)
		{

			if (pstSprite->m_wPx > right || pstSprite->m_wPy > bottom ||
				pstSprite->m_wPx + pstSprite->m_wWidth < left ||
				pstSprite->m_wPy + pstSprite->m_wHeight < top)
				continue;
			if (drawSpriteFunctions[pstSprite->m_dwType] == NULL)
				continue;

			MP_DEBUG2("draw sprite %d type = %d", i, pstSprite->m_dwType);
			swRet = (*drawSpriteFunctions[pstSprite->m_dwType]) (pWin, pstSprite, 1);
			//boXpgBusy = false;
		}
#if SC_USBDEVICE
		if (SystemCheckUsbdPlugIn()) TaskYield();
#endif
	}

	if (boCopyToCurrWin)
	{
		ST_IMGWIN *pCurrWin = Idu_GetCurrWin();

		if (pWin != pCurrWin)
		{
			//Idu_WaitBufferEnd();
#if 0
			mpCopyWinArea(pCurrWin, pWin, left, top, right, bottom);
#else
            /* speedup image copy with ipu */
			//image_scale(pWin, pCurrWin, left, top, right, bottom, left, top, right, bottom);
			Ipu_ImageScaling(pWin, pCurrWin, left, top, right, bottom, left, top, right, bottom,0);
#endif
		}
	}
	pstMovie->m_bUpdateStage = false;
	//g_boNeedRepaint = false;
}


//---------------------------------------------------------------------------
void xpgUpdateStage()
{
    STXPGMOVIE *pstMovie = &g_stXpgMovie;
    STXPGSPRITE *pstSprite;
    SWORD swRet = PASS;
    DWORD i;

    //if (boScalerBusy)    TaskYield();

#if VIDEO_ON
    if (g_bAniFlag & ANI_VIDEO)
    {
        MP_ALERT("video playing - xpgUpdateStage request skip!");
        return;
    }
#endif
/*
	if (g_boNeedRepaint&XPG_REPAINT_IGNORE)
	{
		g_boNeedRepaint&= ~XPG_REPAINT_IGNORE;
		return;
	}
*/
    pstMovie->m_bUpdateStage = true;
    pstMovie->m_boClip = false;
    g_pXpgCanvasWin = Idu_GetNextWin();

	pstSprite = &(pstMovie->m_astSprite[0]);
	if (pstSprite->m_wWidth !=g_pXpgCanvasWin->wWidth  && pstSprite->m_wHeight!=g_pXpgCanvasWin->wHeight)
		mpClearWin(g_pXpgCanvasWin);

    for (i = 0; i < pstMovie->m_dwSpriteCount; i++)
    {
        pstSprite = &(pstMovie->m_astSprite[i]);
        //mpDebugPrint("sprite %d type = %d, x=%d, y=%d, w=%d, h=%d, m_boVisible=%d", i, pstSprite->m_dwType, pstSprite->m_wPx, pstSprite->m_wPy, pstSprite->m_wWidth, pstSprite->m_wHeight, pstSprite->m_boVisible);
        if (pstSprite->m_boVisible && drawSpriteFunctions[pstSprite->m_dwType] != NULL)
        {
            MP_DEBUG("sprite %d type = %d, width=%d, height=%d, wCurFrame=%d", i, pstSprite->m_dwType, pstSprite->m_wWidth, pstSprite->m_wHeight, pstSprite->m_wCurFrame);
            swRet = (*drawSpriteFunctions[pstSprite->m_dwType]) (g_pXpgCanvasWin, pstSprite, 0);
            if (pstSprite->m_dwHashKey)
            {
					if (pstSprite->m_dwHashKey==xpgHash("Touch0000"))//4  设置触摸区域为SPRITE区域
					{
			            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy , pstSprite->m_wWidth, pstSprite->m_wHeight);
					}
					else if (pstSprite->m_dwHashKey==xpgHash("Touch1111"))//4  设置触摸区域上下左右各增加一倍
					{
			            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx-pstSprite->m_wWidth, pstSprite->m_wPy-pstSprite->m_wHeight , pstSprite->m_wWidth*3, pstSprite->m_wHeight*3);
					}
					else if (pstSprite->m_dwHashKey==xpgHash("Touch0011"))//4  
					{
			            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy-pstSprite->m_wHeight , pstSprite->m_wWidth, pstSprite->m_wHeight*3);
					}
					else if (pstSprite->m_dwHashKey==xpgHash("Touch0R11"))  //4 L R U D 触摸区域延长到显示区域的左右上下边
					{
			            xpgSpriteSetTouchArea(pstSprite, pstSprite->m_wPx, pstSprite->m_wPy-pstSprite->m_wHeight , g_pXpgCanvasWin->wWidth-pstSprite->m_wPx, pstSprite->m_wHeight*3);
					}
            }
        }

    }

#if SC_USBDEVICE
    if (SystemCheckUsbdPlugIn()) TaskYield();
#endif
 
   xpgUpdateCanvas();
    pstMovie->m_bUpdateStage = false;
    g_boNeedRepaint = false;


#if 0
    DWORD dwHashKey = g_pstXpgMovie->m_pstCurPage->m_dwHashKey;

    if (dwHashKey == xpgHash("Mode_Photo") || dwHashKey == xpgHash("Photo_Viewer"))
    {
			ShowItemIndex();
    }
#endif

    MP_DEBUG("%s() exit, g_bXpgStatus = %u", __FUNCTION__, g_bXpgStatus);
	Timer_CheckPopDialogAfterUpdatestage();

}

//---------------------------------------------------------------------------
void xpgSpriteRedraw(ST_IMGWIN * pWin, DWORD dwType, DWORD dwIndex)
{
	STXPGMOVIE *pstMovie = &g_stXpgMovie;
	STXPGSPRITE *pstSprite;
	DWORD i;

    for (i = 0; i < pstMovie->m_dwSpriteCount; i++)
    {
        pstSprite = &(pstMovie->m_astSprite[i]);
        if (dwType == pstSprite->m_dwType && dwIndex == pstSprite->m_dwTypeIndex&& pstSprite->m_boVisible && drawSpriteFunctions[pstSprite->m_dwType] != NULL)
        {
			//MP_DEBUG("sprite %d type = %d, width=%d, height=%d, wCurFrame=%d", i, pstSprite->m_dwType, pstSprite->m_wWidth, pstSprite->m_wHeight, pstSprite->m_wCurFrame);
			(*drawSpriteFunctions[pstSprite->m_dwType]) (pWin, pstSprite, 0);
			break;
        }

    }
}

//---------------------------------------------------------------------------
static DWORD dwAllocSize;
DWORD getdwAllocSize()
{
    return dwAllocSize;
}
///
///@ingroup xpgDraw
///@brief   Allocate ext memory for Decode Buffer
///
///@param   pstRole - pointer of Current Role
///
///@return  buffer pointer
///
BYTE *xpgAllocDecodeBuffer(STXPGROLE * pstRole)
{
	BYTE *pbAllocBuf = NULL;

	// allocate buffer to decode image
	dwAllocSize = (ALIGN_16(pstRole->m_wWidth) + 16) * ALIGN_16(pstRole->m_wHeight) * 2 + 4096;
	pbAllocBuf = (BYTE *)ext_mem_mallocEx(dwAllocSize, __FILE__, __LINE__);

	if (pbAllocBuf == NULL) {
		MP_ALERT("role size %d > buffer alloc fail", dwAllocSize);
		return NULL;
	}
	return pbAllocBuf;
}

///
///@ingroup xpgDraw
///@brief   Draw Role by 1Bit Bitmap
///
///@param   pWin - selected image Win
///
///@param   x - left
///
///@param   y - top
///
///@param   w - width
///
///@param   h - height
///
///@param   pImgBuffer - inage data, normally is the buffer after Img_Jpeg2ImgBuf()
///
///@param   bColor - color
///
///@return
///
void xpgDraw1BitBitmap(ST_IMGWIN * pWin, WORD x, WORD y, WORD w, WORD h, BYTE * pImgBuffer,
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

//---------------------------------------------------------------------------
///
///@ingroup xpgDraw
///@brief   Draw Role by Color Bitmap by OSD
///
///@param   pWin - selected image Win
///
///@param   left - left
///
///@param   top - top
///
///@param   w - width
///
///@param   h - height
///
///@param   pImgBuffer - inage data, normally is the buffer after Img_Jpeg2ImgBuf()
///
///@param   bColor - color index of OSD color
///
///@return
///
void xpgOsdDrawIndexColorBitmap(ST_OSDWIN * pWin, WORD left, WORD top, WORD w, WORD h,
								BYTE * pImgBuffer, BYTE bColor)
{
	//if ((pWin == NULL) || (pWin->pdwStart == NULL) || (left >= pWin->wWidth) || (top >= pWin->wHeight))
	//    return;

	BYTE *pbPixel;
	DWORD dwLineAddr;

	dwLineAddr = ((DWORD) (pWin->pdwStart) + (top * pWin->wWidth) + left) | 0xa0000000;
	pbPixel = (BYTE *) dwLineAddr;

	int iBufferSize;
	int i, j;
	WORD x, y;
	register DWORD *pdwPalette;
	register BYTE bColorIndex, bCount;
	BYTE bColorCount = *pImgBuffer;
	register int c;

	pImgBuffer += 4;
	pdwPalette = (DWORD *) pImgBuffer;

	//memcpy((BYTE *)pdwPalette, pImgBuffer, 4 * bColorCount);
	Idu_OsdSetPalette(pdwPalette, 127);

	pImgBuffer += 4 * bColorCount;

	x = 0;
	y = 0;
	j = 0;
	while (y < h)
	{
		bColorIndex = *pImgBuffer;
		pImgBuffer++;

		if ((bColorIndex & 0xc0) == 0xc0)
		{
			bCount = bColorIndex & 0x3f;	// *** get run length ***
			bColorIndex = *pImgBuffer;
			pImgBuffer++;
		}
		else
			bCount = 1;

		if (bColorIndex >= bColorCount)
		{
			bColorIndex = 0;
		}

		while (bCount)
		{
			if (bColorIndex != 0)
				pbPixel[x] = bColorIndex;
			x++;
			bCount--;

			if (x >= w)
			{
				y++;
				if (y == h - 1)
				{
					return;
				}
				x = 0;
				dwLineAddr += pWin->wWidth;
				pbPixel = (BYTE *) dwLineAddr;
			}
		}
	}

}

//---------------------------------------------------------------------------
// SONY-like Clock
///
///@ingroup xpgDraw
///@brief   Free clock buffer and hourMin buffer of ext_mem
///
///@return
///
void ImgBufRotate(DWORD *pImgBuf, BYTE type, WORD width, WORD height)
{
    if(! (type>=1 && type<=3))
        return;
   ST_IMGWIN src_win, rotate_win;
   rotate_win.wWidth = width ;
   rotate_win.wHeight = height ;
   rotate_win.pdwStart = (DWORD *) ext_mem_mallocEx(rotate_win.wWidth*rotate_win.wHeight*2, __FILE__, __LINE__);
   rotate_win.dwOffset = (rotate_win.wWidth<<1) ;
   src_win.wWidth = width ;
   src_win.wHeight = height ;
   src_win.pdwStart = pImgBuf;
   src_win.dwOffset = (src_win.wWidth<<1) ;
   if(type==1) Img_Rotate90(&rotate_win, &src_win);
   if(type==2) Img_Rotate180(&rotate_win, &src_win);
   if(type==3) Img_Rotate270(&rotate_win, &src_win);
   memcpy((BYTE *)pImgBuf, (BYTE *)rotate_win.pdwStart, width*height*2);
   ext_mem_freeEx((BYTE *)((DWORD)rotate_win.pdwStart & ~0x20000000), __FILE__, __LINE__);
}

//---------------------------------------------------------------------------
///
///@ingroup xpgDraw
///@brief   Direct Draw Role On Win by each type
///
///@param   pWin - selected image Win
///
///@param   pstRole - pointer of Current Role
///
///@param   x - left
///
///@param   y - top
///@param   boClip - boolean of Clip
///
///@return
///
void xpgDirectDrawRoleOnWin(ST_IMGWIN * pWin, STXPGROLE * pstRole,
							DWORD x, DWORD y, STXPGSPRITE *pstSprite, BOOL boClip)
{
    MP_DEBUG("%s - x=%d y=%d", __FUNCTION__, x, y);

	if (pstRole == NULL)
		return;
	if (pstRole->m_pImage == NULL)
		return;
	if (x >= pWin->wWidth || y >= pWin->wHeight)
		return;
    if (x + pstRole->m_wWidth > pWin->wWidth)
	{
	     //mpDebugPrint("x + pstRole->m_wWidth > pWin->wWidth!, x=%d, pstRole->m_wWidth=%d, reset x to 0", x, pstRole->m_wWidth);
	     if(pstRole->m_wWidth == pWin->wWidth)
	       x = 0;
		  else
            return;					//boClip = true;
	}
    if (y + pstRole->m_wHeight > pWin->wHeight)
	{
	   //mpDebugPrint("y + pstRole->m_wHeight > pWin->wHeight!, y=%d, pstRole->m_wHeight=%d", y, pstRole->m_wHeight);
	   if(pstRole->m_wHeight == pWin->wHeight)
	       y = 0;
	   else	
        return;					//boClip = true;
	}

	int swRet = PASS;
	BYTE *pbAllocBuf = NULL;
	DWORD *pImgBuf = NULL;
	DWORD dwImgBufSize = 0;
	WORD wRawWidth;


#if 1
	//mpDebugPrint("bit %d type=%d version=%d", pstRole->m_bBitDepth,pstRole->m_bImageType,g_stXpgMovie.m_dwVersion);//bit 24 type=0 version=4112
	if (pstRole->m_bBitDepth >= 24 && pstRole->m_pRawImage != NULL)
		pImgBuf = (DWORD *) pstRole->m_pRawImage;
	else
	{
			//JPEG
			// allocate buffer to decode image
			pbAllocBuf = (BYTE *)xpgAllocDecodeBuffer(pstRole);
			//pbAllocBuf = (BYTE *)((DWORD)xpgAllocDecodeBuffer(pstRole)|0xA0000000);

 			if (pbAllocBuf == NULL) 
            {
                MP_ALERT("pbAllocBuf == NULL");
                return;
            }
			pImgBuf = (DWORD *) ((DWORD) pbAllocBuf | 0x20000000);

			// decode jpeg role
			swRet = Img_Jpeg2ImgBuf(pstRole->m_pImage, (BYTE *)pImgBuf, 0, pstRole->m_dwDataLen, dwAllocSize);
			pstRole->m_wRawWidth = Img_JpegGetCurWidth();
			wRawWidth  = Img_JpegGetCurWidth();

			if (swRet != 0)
			{
				MP_ALERT("xpgDirectDrawRoleOnWin Jpeg decode fail %d", swRet);
				swRet = FAIL;
			}
    }

    if (pImgBuf && swRet == PASS)
    {
		MP_DEBUG1("pstRole->m_bImageType %d", pstRole->m_bImageType);
		if (Ipu_CheckComplete() && pstRole->m_wWidth == pstRole->m_wRawWidth)
		{
			ST_IMGWIN sSrcWin;

			sSrcWin.pdwStart = pImgBuf;
			sSrcWin.dwOffset = pstRole->m_wRawWidth << 1;
			sSrcWin.wHeight = pstRole->m_wHeight;
			sSrcWin.wWidth = pstRole->m_wRawWidth;

			//    			Ipu_ImageScaling((ST_IMGWIN *) & sSrcWin, (ST_IMGWIN *) pWin, pstRole->m_wWidth,sSrcWin.wHeight, x, y);
			Ipu_ImageScaling(&sSrcWin, pWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, x, y, x+pstRole->m_wWidth, y+sSrcWin.wHeight, 0);
		}
		else
		{
			xpgRoleDraw(pstRole, pWin->pdwStart, (void *)pImgBuf, x, y, pWin->wWidth, pWin->wHeight);
		}
	}

#else
	MP_DEBUG1("bit %d", pstRole->m_bBitDepth);
	if (pstRole->m_bBitDepth >= 24 && pstRole->m_pRawImage != NULL)
		pImgBuf = (DWORD *) pstRole->m_pRawImage;
	else
	{
		switch (pstRole->m_bBitDepth)
		{
		case 1:
			xpgDraw1BitBitmap(pWin, x, y, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_pImage,
							  0xff);
			//Idu_OsdDrawBitmap4Bit(x, y, pstRole->m_wWidth, pstRole->m_wHeight, pstRole->m_pImage, 2);
			return;

		case 7:
		case 8:
			xpgOsdDrawIndexColorBitmap(Idu_GetOsdWin(), x, y, pstRole->m_wWidth, pstRole->m_wHeight,
									   pstRole->m_pImage, 0xff);
			return;
        case 32: // BuilderV5 Animation, Simulate xpg_player flow
            MP_DEBUG("case 32: // for MaskRole of .png");
            // allocate buffer to decode image
            //BYTE *pbAllocBuf = NULL;
			pbAllocBuf = (BYTE *)xpgAllocDecodeBuffer(pstRole);
			//pbAllocBuf = (BYTE *)((DWORD)xpgAllocDecodeBuffer(pstRole)|0xA0000000);

 			if (pbAllocBuf == NULL) 
            {
                MP_ALERT("pbAllocBuf == NULL");
                return;
            }
 			//DWORD *pImgBuf = NULL;
			pImgBuf = (DWORD *) ((DWORD) pbAllocBuf | 0x20000000);

			//boXpgBusy = true;
			// decode jpeg role
			swRet = Img_Jpeg2ImgBuf(pstRole->m_pImage, (BYTE *)pImgBuf, 0, pstRole->m_dwDataLen, dwAllocSize);
			pstRole->m_wRawWidth = Img_JpegGetCurWidth();

			if (swRet != 0)
			{
				MP_ALERT("xpgDirectDrawRoleOnWin Jpeg decode fail %d", swRet);
				swRet = FAIL;
			}

			pstRole->m_wRawWidth = Img_JpegGetCurWidth();

            BYTE *pbMaskBuf = NULL;
            // allocate buffer to decode image
			pbMaskBuf = (BYTE *)xpgAllocDecodeBuffer(pstRole);
			DWORD *pMaskBuf = NULL;
			if (pbMaskBuf != NULL)
			     pMaskBuf = (DWORD *) ((DWORD) pbMaskBuf | 0x20000000);

            if (pstRole->m_pMaskImage)
            {
                swRet = Img_Jpeg2ImgBuf(pstRole->m_pMaskImage, (BYTE *)pMaskBuf, 0, pstRole->m_dwDataLen, dwAllocSize);
                if (swRet != 0)
    			{
    				MP_ALERT("xpgDirectDrawRoleOnWin Jpeg decode fail %d", swRet);
    				swRet = FAIL;
    			}
            }

        #if 0
            xpgRoleDrawMaskBlend(pstRole, pWin->pdwStart, pImgBuf, pMaskBuf, x, y, pWin->wWidth, pWin->wHeight, 0);
            if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
			if (pbMaskBuf) ext_mem_freeEx(pbMaskBuf, __FILE__, __LINE__);
	        //boXpgBusy = FALSE;
			return;
        #endif

            pstRole->m_pRawImage = (BYTE *) pImgBuf; //pstRole->m_pImage;
            STXPGROLE stMaskRole;
            STXPGROLE * pstMaskRole = &stMaskRole;
            pstMaskRole->m_pRawImage = (BYTE *) pMaskBuf; //pstRole->m_pMaskImage;
            pstMaskRole->m_wWidth = pstRole->m_wWidth;
            pstMaskRole->m_wHeight = pstRole->m_wHeight;
            pstMaskRole->m_wRawWidth = pstRole->m_wRawWidth;
            
            WORD wCurFrame = pstSprite->m_wCurFrame;
            if(wCurFrame >= 256)
	        {
	            MP_ALERT("ALERT : wCurFrame >= 256, skip!");
	            if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
			    if (pbMaskBuf) ext_mem_freeEx(pbMaskBuf, __FILE__, __LINE__);
                //boXpgBusy = FALSE;
                return;
            }
            WORD wRole = pstSprite->m_wRole;
            WORD wScalew  = pstSprite->m_astFrame[wCurFrame].m_alValue[4];
     		WORD wScaleh  = pstSprite->m_astFrame[wCurFrame].m_alValue[5];
     		WORD wBlend  = pstSprite->m_astFrame[wCurFrame].m_alValue[6];
     		WORD wAngle  = pstSprite->m_astFrame[wCurFrame].m_alValue[7];
     	#if 1	
     		DWORD pageHashKey = xpgHash("Clock");
     		if(g_pstXpgMovie->m_pstCurPage->m_dwHashKey == pageHashKey)
     		{
     		    //mpDebugPrint("pstSprite->m_dwType = %d, pstSprite->m_dwTypeIndex = %d", pstSprite->m_dwType, pstSprite->m_dwTypeIndex);
     		    BYTE hour, minute, second; 
     		    //xpgGetBuildHourMinuteSecond(&hour, &minute, &second);
     		    ST_SYSTEM_TIME stSystemTime;
                SystemTimeGet(&stSystemTime);
                hour = stSystemTime.u08Hour;
                minute = stSystemTime.u08Minute;
                second = stSystemTime.u08Second;
     	        //mpDebugPrint("hour = %d, minute = %d, second = %d", hour, minute, second);
     	        
     		    if(pstSprite->m_dwType == 0 && pstSprite->m_dwTypeIndex == 1) // hour
     		        wAngle = hour * 30 + (minute / 2);
     		    if(pstSprite->m_dwType == 0 && pstSprite->m_dwTypeIndex == 2) // minute
     		        wAngle = minute * 6 + (second / 10);
     		    if(pstSprite->m_dwType == 0 && pstSprite->m_dwTypeIndex == 3) // second
     		        wAngle = second * 6;
            }
        #endif
            //mpDebugPrint("wCurFrame=%d, wScalew=%d, wScaleh=%d, wBlend=%d, wAngle=%d", wCurFrame, wScalew, wScaleh, wBlend, wAngle);
   		    DWORD ox = pstRole->m_wOx;
		    DWORD oy = pstRole->m_wOy;
   		    //xpgRoleDrawStretchRotate(pstRole, pWin->pdwStart, (void *)pImgBuf, (void *)pMaskBuf, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wScalew, wScaleh, wBlend, wAngle);
   		    //xpgRoleDrawMaskRotate(pstRole, pWin->pdwStart, (void *)pImgBuf, (void *)pMaskBuf, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wAngle);
   		    //xpgRoleDrawMaskBlendRotate(pstRole, pWin->pdwStart, (void *)pImgBuf, (void *)pMaskBuf, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wBlend, wAngle);
            xpgRoleDrawMaskScaleBlendRotate(pstRole, pstSprite, pWin->pdwStart, (void *)pImgBuf, (void *)pMaskBuf, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wScalew, wScaleh, wBlend, wAngle);		        
    
            pstRole->m_pRawImage = NULL; // to prevnt from enter if (pstRole->m_bBitDepth >= 24 && pstRole->m_pRawImage != NULL)
            pstMaskRole->m_pRawImage = NULL;

			if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
			if (pbMaskBuf) ext_mem_freeEx(pbMaskBuf, __FILE__, __LINE__);
	        //boXpgBusy = FALSE;

			return;
		default:				//JPEG
		/*	if (!g_bAniFlag && !boClip && pstRole->m_wWidth == pWin->wWidth )
			{
			    //Image height of JEPG and position in window should be multiple of 16 pixel when JPEG format is 420.
		        if (ALIGN_16(y) == y && ALIGN_16(pWin->wHeight) == pWin->wHeight)
		        {
    				swRet = Img_Jpeg2ImgBuf(pstRole->m_pImage, (BYTE *)pWin->pdwStart + y * pWin->dwOffset, 0,
    					                    pstRole->m_dwDataLen, pWin->dwOffset * (pWin->wHeight - y));
    				//MP_ALERT("ALIGN_16(pWin->wHeight) == pWin->wHeight");
    				return;
                }
			} -- useless ?!*/

			//boXpgBusy = false;

			// allocate buffer to decode image
			pbAllocBuf = (BYTE *)xpgAllocDecodeBuffer(pstRole);
			//pbAllocBuf = (BYTE *)((DWORD)xpgAllocDecodeBuffer(pstRole)|0xA0000000);

 			if (pbAllocBuf == NULL) 
            {
                MP_ALERT("pbAllocBuf == NULL");
                return;
            }
			pImgBuf = (DWORD *) ((DWORD) pbAllocBuf | 0x20000000);

			//boXpgBusy = true;
			// decode jpeg role
			swRet = Img_Jpeg2ImgBuf(pstRole->m_pImage, (BYTE *)pImgBuf, 0, pstRole->m_dwDataLen, dwAllocSize);
			pstRole->m_wRawWidth = Img_JpegGetCurWidth();
			wRawWidth  = Img_JpegGetCurWidth();

			if (swRet != 0)
			{
				MP_ALERT("xpgDirectDrawRoleOnWin Jpeg decode fail %d", swRet);
				swRet = FAIL;
			}
			break;
		}
    }

    if (pImgBuf && swRet == PASS)
    {
        MP_DEBUG1("pstRole->m_bImageType %d", pstRole->m_bImageType);

#if ( ((CHIP_VER & 0xFFFF0000) == CHIP_VER_650) || ((CHIP_VER & 0xFFFF0000) == CHIP_VER_660))
        if (pstRole->m_bImageType >= 96)
        {   // for FlashClockSecond 96..99
            //mpDebugPrint("xpgDraw.c, FlashClockSecond ImageType=%d", pstRole->m_bImageType);

            if (pstRole->m_bImageType == 97)
            {   //rotate 90 degree
                ST_IMGWIN src_win, rotate_win;

                rotate_win.wWidth = pstRole->m_wWidth; //400 ;
                rotate_win.wHeight = pstRole->m_wHeight; //400 ;
                rotate_win.pdwStart = (DWORD *) ext_mem_mallocEx(rotate_win.wWidth*rotate_win.wHeight*2, __FILE__, __LINE__);
                rotate_win.dwOffset = (rotate_win.wWidth<<1) ;
                src_win.wWidth = pstRole->m_wWidth; //400 ;
                src_win.wHeight = pstRole->m_wHeight; //400 ;
                src_win.pdwStart = pImgBuf;
                src_win.dwOffset = (src_win.wWidth<<1) ;
                Img_Rotate90(&rotate_win, &src_win);
                memcpy((BYTE *)pImgBuf, (BYTE *) rotate_win.pdwStart, rotate_win.wWidth*rotate_win.wHeight*2);
                ext_mem_freeEx((BYTE *)((DWORD)rotate_win.pdwStart & ~0x20000000), __FILE__, __LINE__);
            }

            if (pstRole->m_bImageType == 98)
            {   //rotate 180 degree
                ST_IMGWIN src_win, rotate_win;

                rotate_win.wWidth = pstRole->m_wWidth; //400 ;
                rotate_win.wHeight = pstRole->m_wHeight; //400 ;
                rotate_win.pdwStart = (DWORD *) ext_mem_mallocEx(rotate_win.wWidth*rotate_win.wHeight*2, __FILE__, __LINE__);
                rotate_win.dwOffset = (rotate_win.wWidth<<1) ;
                src_win.wWidth = pstRole->m_wWidth; //400 ;
                src_win.wHeight = pstRole->m_wHeight; //400 ;
                src_win.pdwStart = pImgBuf;
                src_win.dwOffset = (src_win.wWidth<<1) ;
                Img_Rotate180(&rotate_win, &src_win);
                memcpy((BYTE *) pImgBuf, (BYTE *) rotate_win.pdwStart, rotate_win.wWidth*rotate_win.wHeight*2);
                ext_mem_freeEx((BYTE *)((DWORD)rotate_win.pdwStart & ~0x20000000), __FILE__, __LINE__);
                //ImgBuf_Rotate180(pImgBuf, 400, 400);// call Rotate180;
            }

            if (pstRole->m_bImageType == 99)
            {   //rotate 270 degree
                ST_IMGWIN src_win, rotate_win;

                rotate_win.wWidth = pstRole->m_wWidth; //400 ;
                rotate_win.wHeight = pstRole->m_wHeight; //400 ;
                rotate_win.pdwStart = (DWORD *) ext_mem_mallocEx(rotate_win.wWidth*rotate_win.wHeight*2, __FILE__, __LINE__);
                rotate_win.dwOffset = (rotate_win.wWidth<<1) ;
                src_win.wWidth = pstRole->m_wWidth; //400 ;
                src_win.wHeight = pstRole->m_wHeight; //400 ;
                src_win.pdwStart = pImgBuf;
                src_win.dwOffset = (src_win.wWidth<<1) ;
                Img_Rotate270(&rotate_win, &src_win);
                memcpy((BYTE *) pImgBuf, (BYTE *) rotate_win.pdwStart, rotate_win.wWidth*rotate_win.wHeight*2);
                ext_mem_freeEx((BYTE *)((DWORD)rotate_win.pdwStart & ~0x20000000), __FILE__, __LINE__);
                //ImgBuf_Rotate270(pImgBuf, 400, 400);// call Rotate270;
            }

            xpgRoleDrawFlashClock(pstRole, pWin->pdwStart, (void *)pImgBuf, x, y, pWin->wWidth, pWin->wHeight);

/*
            memcpy(clockBuf.pdwStart, pImgBuf, 400*400*2);
            //Idu_PaintWin(&clockBuf, 0x00008080);
            xpgRoleDrawFlashClock(pstRole, clockBuf.pdwStart, (void *)pImgBuf, 0, 0, clockBuf.wWidth, clockBuf.wHeight);
            ST_IMGWIN *srcMainWin=Idu_GetCurrWin();
            ST_IMGWIN srcSubWin;
            ST_IMGWIN *trgWin=Idu_GetNextWin();
            srcSubWin.wWidth = 400 ;
            srcSubWin.wHeight = 400 ;
            srcSubWin.pdwStart = pImgBuf;
            srcSubWin.dwOffset = (srcSubWin.wWidth<<1) ;
            WORD start_x1=40, start_y1=200; //WORD start_x1=0, start_y1=0;
            BYTE level=1; //0;
            Ipu_Overlay(pWin, &srcSubWin, trgWin, start_x1, start_y1, level);
            //mpDebugPrint("after Ipu_Overlay()");
*/
        }
        else
#endif
        {
            if ( (pstRole->m_bImageType >= 21) && (pstRole->m_bImageType <= 24) ) // for DigitClockCircle
            {
                ImgBufRotate(pImgBuf, pstRole->m_bImageType-21, pstRole->m_wWidth, pstRole->m_wHeight);
                xpgRoleDraw(pstRole, pWin->pdwStart, (void *)pImgBuf, x, y, pWin->wWidth, pWin->wHeight);
            }
            else if ( (pstRole->m_bImageType >= 11) && (pstRole->m_bImageType <= 14) ) // for FlashClock
            {
#if FB_OVERLAP            
                //ImgBufRotate(pImgBuf, pstRole->m_bImageType-11, 400, 400);
                ImgBufRotate(pImgBuf, pstRole->m_bImageType-11, pstRole->m_wWidth, pstRole->m_wHeight);

                ST_IMGWIN srcTopWin;
                ST_BOX_POS stOverlapPos;
                ST_COLOR_KEY stColorkey = {0, 128, 128, 0};
                ST_COLOR_KEY_NR stColorKeyNR = {25, 25, 25, 25, 25, 25};

                srcTopWin.pdwStart = pImgBuf;
                srcTopWin.wWidth = pstRole->m_wWidth; //400;
                srcTopWin.wHeight = pstRole->m_wHeight; //400;
                srcTopWin.dwOffset = (srcTopWin.wWidth << 1);
                stOverlapPos.posX = x;
                stOverlapPos.posY = y;

                Util_FbOverlapByColorkey(pWin, &srcTopWin, Idu_GetNextWin(),
                                         &stOverlapPos,
                                         &stColorkey, &stColorKeyNR);
#endif                                         
            }
            else
            {
                // origin type
            	if (pstRole->m_bImageType == 9)
            		xpgRoleDrawMask(pstRole, pWin->pdwStart, x, y, pWin->wWidth, pWin->wHeight, g_pstXpgMovie->m_pstObjRole[0]);
            	else if (pstRole->m_bImageType == 8)
            		xpgRoleDrawBlend(pstRole, pWin->pdwStart, x, y, pWin->wWidth, pWin->wHeight);
            	else if (pstRole->m_bType == 2 || pstRole->m_bImageType == 5)
            		xpgRoleDrawTransperant(pstRole, pWin->pdwStart, x, y, pWin->wWidth, pWin->wHeight);
            	else if (boClip) // for Video/Music ListFrame display
            	{
            	    ST_IMGWIN * psWin = pWin;
            	    if( ! (psWin->wClipLeft == 0 && psWin->wClipTop == 0 && psWin->wClipRight == (psWin->wWidth-1) && psWin->wClipBottom == (psWin->wHeight-1)))
            	    {
            	        MP_ALERT("%s: boClip = true, If that's NOT you want, need to check!", __func__);
            	        mpDebugPrint("psWin->wClipLeft   = %d", psWin->wClipLeft);
                        mpDebugPrint("psWin->wClipTop    = %d", psWin->wClipTop);
                        mpDebugPrint("psWin->wClipRight  = %d", psWin->wClipRight);
                        mpDebugPrint("psWin->wClipBottom = %d", psWin->wClipBottom);
                    }
            		xpgRoleDrawInClip(pstRole, pWin, pImgBuf, x, y);
            	}
            	else
            	{
            		if (Ipu_CheckComplete() && pstRole->m_wWidth == pstRole->m_wRawWidth)
            		{
            			ST_IMGWIN sSrcWin;

            			sSrcWin.pdwStart = pImgBuf;
            			sSrcWin.dwOffset = pstRole->m_wRawWidth << 1;
            				sSrcWin.wHeight = pstRole->m_wHeight;
            			sSrcWin.wWidth = pstRole->m_wRawWidth;

        //    			Ipu_ImageScaling((ST_IMGWIN *) & sSrcWin, (ST_IMGWIN *) pWin, pstRole->m_wWidth,sSrcWin.wHeight, x, y);
        				Ipu_ImageScaling(&sSrcWin, pWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, x, y, x+pstRole->m_wWidth, y+sSrcWin.wHeight, 0);
                    }
            		else
            		{
            		    
            		    if (g_stXpgMovie.m_dwVersion >= 5000 && pstSprite->m_wFrameCount > 1 && pstSprite->m_wFrameCount < 256)
            		    {
            		        WORD wScalew = 100, wScaleh = 100;
            		        int wBlend = 0, wAngle = 0;
            		        WORD wCurFrame = pstSprite->m_wCurFrame;
            		        wScalew = pstSprite->m_astFrame[wCurFrame].m_alValue[4];
            		        wScaleh = pstSprite->m_astFrame[wCurFrame].m_alValue[5];
            		        wBlend  = pstSprite->m_astFrame[wCurFrame].m_alValue[6];
            		        wAngle  = pstSprite->m_astFrame[wCurFrame].m_alValue[7];

                            STXPGROLE stRoleBackup;
                            memcpy((BYTE *)&stRoleBackup, (BYTE *)pstSprite->m_pstRole, sizeof(STXPGROLE));
                   		    pstRole->m_wRawWidth = wRawWidth;
                            
            		        //mpDebugPrint("wCurFrame=%d, wScalew=%d, wScaleh=%d, wBlend=%d, wAngle=%d", wCurFrame, wScalew, wScaleh, wBlend, wAngle);
               		        DWORD ox = pstRole->m_wOx;
            		        DWORD oy = pstRole->m_wOy;
               		        //xpgRoleDrawStretchRotate(pstRole, pWin->pdwStart, (void *)pImgBuf, NULL, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wScalew, wScaleh, wBlend, wAngle);
               		        //xpgRoleDrawScaleBlendRotate(pstRole, pstSprite, pWin->pdwStart, pImgBuf, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wScalew, wScaleh, wBlend, wAngle);
               		        xpgRoleDrawMaskScaleBlendRotate(pstRole, pstSprite, pWin->pdwStart, pImgBuf, NULL, x, y, ox, oy, pWin->wWidth, pWin->wHeight, wScalew, wScaleh, wBlend, wAngle);
               		        memcpy((BYTE *)pstSprite->m_pstRole, (BYTE *)&stRoleBackup, sizeof(STXPGROLE));
            		    }
            		    else  
        				    xpgRoleDraw(pstRole, pWin->pdwStart, (void *)pImgBuf, x, y, pWin->wWidth, pWin->wHeight);
        			}
        		}
            }
        }
	}
#endif

	if (pbAllocBuf) ext_mem_freeEx(pbAllocBuf, __FILE__, __LINE__);
	//boXpgBusy = FALSE;
}

