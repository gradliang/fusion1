/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "xpgUtil.h"
#include "xpgRole.h"
#include "xpgSprite.h"

#define YYCbCr_Y0(c) ((c >> 24) & 0xff)
#define YYCbCr_Y1(c) ((c >> 16) & 0xff)
#define YYCbCr_Cb(c) ((c >> 8) & 0xff)
#define YYCbCr_Cr(c) (c & 0xff)
#define SetYYCbCr(cy0, cy1, cb, cr) (((cy0 << 24) | (cy1 << 16) | (cb << 8) | cr))

//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Init Role struct
///
void xpgRoleInit(STXPGROLE * pstRole)
{
	memset(pstRole, 0, sizeof(STXPGROLE));
}

//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Release Role ext memory
///
///@param   pstRole - specified Role
///
void xpgRoleRelease(STXPGROLE * pstRole)
{
	if (pstRole->m_pRawImage)
	{
		ext_mem_freeEx(pstRole->m_pRawImage, __FILE__, __LINE__);
		pstRole->m_pRawImage = NULL;
	}
}
//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Preload Role image to memory
///
///@param   pstRole - specified Role
///
void xpgRolePreload(STXPGROLE * pstRole)
{
	DWORD dwSize;

	if (pstRole->m_bBitDepth != 24)
	{
	    MP_ALERT("pstRole->m_bBitDepth != 24");
		return;
	}
	if (pstRole->m_pRawImage != NULL)
	{
	    //MP_ALERT("pstRole->m_pRawImage != NULL"); // log too often
		return;
	}
#if XPG_ROLE_USE_CACHE
	if (pstRole->m_wIndex == 0) 
    {
		MP_DEBUG3("malloc role %d, w%d h%d null", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight);
		return;
	}
#endif		
	dwSize = ALIGN_16(pstRole->m_wWidth) * ALIGN_16(pstRole->m_wHeight) * 2 + 256;

	MP_DEBUG3("malloc role %d, w%d h%d ", pstRole->m_wIndex, pstRole->m_wWidth, pstRole->m_wHeight);

	pstRole->m_pRawImage = (BYTE *) ext_mem_malloc(dwSize);

	MP_ASSERT(pstRole->m_pRawImage != NULL);

	if (pstRole->m_pRawImage != NULL)
	{
		Img_Jpeg2ImgBuf(pstRole->m_pImage, pstRole->m_pRawImage, 0, pstRole->m_dwDataLen, dwSize);
		pstRole->m_wRawWidth = Img_JpegGetCurWidth();
	}
	MP_DEBUG3("size %d, pstRole->m_pRawImage %x pstRole->m_wRawWidth %d null", dwSize, pstRole->m_pRawImage, pstRole->m_wRawWidth);
#if XPG_ROLE_USE_CACHE
    if(dwRoleCacheBuf[pstRole->m_wIndex] == 0)
	{
        dwRoleCacheBuf[pstRole->m_wIndex] = pstRole->m_pRawImage;
        wRoleCacheRawWidth[pstRole->m_wIndex] = pstRole->m_wRawWidth;
        //mpDebugPrint("xpgRolePreload save to dwRoleCacheBuf[%d] = 0x%X, wRawWidth = %d", pstRole->m_wIndex, dwRoleCacheBuf[pstRole->m_wIndex], pstRole->m_wRawWidth);
    } 	
#endif		
}

//---------------------------------------------------------------------------
void xpgRoleDraw(STXPGROLE * pstRole, void *pTarget, register DWORD * pdwImageBuffer, DWORD px, DWORD py, DWORD dwScrWd, DWORD dwScrHt)
{
	register DWORD *pdwScreenBuffer = (DWORD *) pTarget;
	MP_DEBUG2("xpgRoleDraw %08x %08x", (DWORD)pdwImageBuffer, (DWORD)pTarget );
	MP_ASSERT(pdwImageBuffer != NULL);
	MP_ASSERT(pTarget != NULL);

    DWORD right, bottom;
	right = px + ALIGN_CUT_2(pstRole->m_wWidth); // ALIGN_CUT_2解决右边白条
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)
		px = 0;
	if (py < 0)
		py = 0;

	DWORD dwLineWidth = (right - px) << 1;
	register DWORD dwScrOffset = dwScrWd >> 1;
	register DWORD dwImgOffset = pstRole->m_wRawWidth >> 1;

	pdwScreenBuffer += ((py * dwScrWd) + px) >> 1;
	for (; py < bottom; py++)
	{
		memcpy(pdwScreenBuffer, pdwImageBuffer, dwLineWidth);
		pdwScreenBuffer += dwScrOffset;
		pdwImageBuffer += dwImgOffset;
	}
}

//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Draw Role by Transparent
///
///@param   pstRole - specified Role
///
///@param   pTarget - specified Screen buffer
///
///@param   px - left
///
///@param   py - top
///
///@param   dwScrWd - Screen buffer width
///
///@param   dwScrHt - Screen Buffer Height
///
void xpgRoleDrawTransperant(STXPGROLE * pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd,
							DWORD dwScrHt)
{
	register DWORD *pdwScreenBuffer = (DWORD *) pTarget;
	register DWORD *pdwSourceBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k;

	MP_DEBUG("xpgRoleDrawTransperant");
	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	pdwSourceBuffer = (DWORD *) pstRole->m_pRawImage;
	if (pstRole->m_pRawImage == NULL || pTarget == NULL)
	{
		MP_DEBUG("xpgRoleDrawTransperant null");
		return;
	}

	if (px & 1)	px++;
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}
	right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;
#define CY_MAX 0xf0
	register DWORD cy0, cy1, cb, cr;
	register DWORD c, cDst;
	for (y = py; y < bottom; y++) {
		j = ((y * dwScrWd) >> 1) + (px >> 1);
		k = (((y - py) * pstRole->m_wRawWidth) >> 1);
		for (x = px; x < right; x += 2, j++, k++) {
			c = *(pdwSourceBuffer + k);
			if ((c & 0xffff0000) == 0) continue; // skip black pixel

			cy0 = YYCbCr_Y0(c);
			cy1 = YYCbCr_Y1(c);
			if (cy0 < 24 && cy1 < 24)
				continue;
			if (cy0 > CY_MAX && cy1 > CY_MAX)
				*(pdwScreenBuffer + j) = c;
#if 0
			else if (cy0 > CY_MIN) {
				cDst = *(pdwScreenBuffer + j);
				cb = (YYCbCr_Cb(c) + YYCbCr_Cb(cDst)) >> 1;
				cr = (YYCbCr_Cr(c) + YYCbCr_Cr(cDst)) >> 1;
				cy0 = (cy0 * cy0 + YYCbCr_Y0(cDst) * (256 - cy0)) >> 8;
				cy1 = YYCbCr_Y1(cDst);
				cDst = (cy0 << 24) | (cy1 << 16) | (cb << 8) | cr;
				*(pdwScreenBuffer + j) = cDst;
			}
			else if (cy1 > CY_MIN) {
				cDst = *(pdwScreenBuffer + j);
				cb = (YYCbCr_Cb(c) + YYCbCr_Cb(cDst)) >> 1;
				cr = (YYCbCr_Cr(c) + YYCbCr_Cr(cDst)) >> 1;
				cy0 = YYCbCr_Y0(cDst);
				cy1 = (cy1 * cy1 + YYCbCr_Y1(cDst) * (256 - cy1)) >> 8;
				cDst = (cy0 << 24) | (cy1 << 16) | (cb << 8) | cr;
				*(pdwScreenBuffer + j) = cDst;
			}
#endif
			else {
				cDst = *(pdwScreenBuffer + j);
#define getBlend2(cy, a) { cy = a;  }
//#define getBlend(cy, a) { cy = a; if (cy < 32) cy = 32; else if (cy > 250) cy = 250; }
				register WORD cy;
				getBlend2(cy, (cy0 + cy1) >> 1);
				cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (256 - cy)) >> 8;
				cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (256 - cy)) >> 8;

				getBlend2(cy, cy0);
				cy0 = (cy0 * cy + YYCbCr_Y0(cDst) * (256 - cy)) >> 8;

				getBlend2(cy, cy1);
				cy1 = (cy1 * cy + YYCbCr_Y1(cDst) * (256 - cy)) >> 8;
				cDst = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);
				*(pdwScreenBuffer + j) = cDst;
			}
		}
	}
#if !XPG_ROLE_USE_CACHE
	xpgRoleRelease(pstRole);
#endif	
}


//--Role部分显示在WIN上
//--bMix  0-128:0->pwin  128->role
void xpgRoleMixOnWin(ST_IMGWIN * pWin, STXPGROLE * pstRole, DWORD px, DWORD py, BYTE bMix)
{
	register DWORD *pdwScreenBuffer =  pWin->pdwStart;
	register DWORD *pdwSourceBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k;

	if (!bMix)
		return;
	if (bMix>128)
		bMix=128;
	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	pdwSourceBuffer = (DWORD *) pstRole->m_pRawImage;
	if (pstRole->m_pRawImage == NULL || pdwScreenBuffer == NULL)
	{
		MP_DEBUG("xpgRoleMixOnWin null");
		return;
	}

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;
	if (px & 1)	px++;
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}
	right = px + pstRole->m_wWidth;
	if (right > pWin->wWidth)
		right = pWin->wWidth;

	bottom = py + pstRole->m_wHeight;
	if (bottom > pWin->wHeight)
		bottom = pWin->wHeight;

	register DWORD cy0, cy1, cb, cr;
	register DWORD c, cDst;
	for (y = py; y < bottom; y++) 
	{
		j = ((y * pWin->wWidth) >> 1) + (px >> 1);
		k = (((y - py) * pstRole->m_wRawWidth) >> 1);
		for (x = px; x < right; x += 2, j++, k++) 
		{
			c = *(pdwSourceBuffer + k);
			cDst = *(pdwScreenBuffer + j);

			cy0 = (YYCbCr_Y0(c)*bMix+YYCbCr_Y0(cDst)*(128-bMix))>>7;
			cy1 = (YYCbCr_Y1(c)*bMix+YYCbCr_Y1(cDst)*(128-bMix))>>7;
			cb = (YYCbCr_Cb(c)*bMix+YYCbCr_Cb(cDst)*(128-bMix))>>7;
			cr = (YYCbCr_Cr(c)*bMix+YYCbCr_Cr(cDst)*(128-bMix))>>7;
			*(pdwScreenBuffer + j) = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);
		}
	}
#if !XPG_ROLE_USE_CACHE
	xpgRoleRelease(pstRole);
#endif	
}

//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Draw Role by Mask
///
///@param   pstRole - specified Role
///
///@param   pTarget - specified Screen buffer
///
///@param   px - left
///
///@param   py - top
///
///@param   dwScrWd - Screen buffer width
///
///@param   dwScrHt - Screen Buffer Height
///
///@param   pstMaskRole - specified Mask Role
///
void xpgRoleDrawMask(STXPGROLE * pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd,
							DWORD dwScrHt, STXPGROLE * pstMaskRole)
{
 register DWORD *pdwScreenBuffer = (DWORD *) ((DWORD)pTarget|0xA0000000);
#if XPG_USE_YUV444	
	BYTE * pbScreenBuf = (BYTE *) pdwScreenBuffer;
	BYTE * pbScreen;
#endif	
	register DWORD *pdwSourceBuffer;
	register DWORD *pdwMaskBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k,l;

	MP_DEBUG("xpgRoleDrawMask");
	if (pstRole == NULL || pstMaskRole == NULL ||pTarget == NULL)
	{
		MP_DEBUG("xpgRoleDrawMask null");
		return;
	}
	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	if (pstMaskRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstMaskRole);
	}

	if (pstRole->m_pRawImage == NULL || pstMaskRole->m_pRawImage == NULL)
	{
		if (pstRole->m_pRawImage) ext_mem_freeEx(pstRole->m_pRawImage, __FILE__, __LINE__);
		if (pstMaskRole->m_pRawImage) ext_mem_freeEx(pstMaskRole->m_pRawImage, __FILE__, __LINE__);

			pstRole->m_pRawImage = NULL;
		pstMaskRole->m_pRawImage = NULL;
		MP_DEBUG("xpgRoleDrawMask null");
		return;
	}
     //mpDebugPrint("pstRole->m_wRawWidth=%d,pstMaskRole->m_wRawWidth=%d", pstRole->m_wRawWidth, pstMaskRole->m_wRawWidth);

	pdwSourceBuffer = (DWORD *) ((DWORD)pstRole->m_pRawImage|0xA0000000);//(DWORD *) pstRole->m_pRawImage;
	pdwMaskBuffer = (DWORD *) ((DWORD)pstMaskRole->m_pRawImage|0xA0000000);//(DWORD *) pstMaskRole->m_pRawImage;

	//if (px & 1)	px++; //fengrs 06/04 marked for icon redraw with a darkline at right side
	if ((DWORD)pdwMaskBuffer & 3) {
		MP_DEBUG("pdwMaskBuffer & 3");
	}
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}
	if (pstMaskRole->m_wRawWidth<pstRole->m_wWidth)
		right = px + pstMaskRole->m_wRawWidth;
	else
		right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;

	register DWORD cy0, cy1, cb, cr;
	register DWORD c, cDst;
	register WORD cy;
	for (y = py; y < bottom; y++) 
	{
		j = ((y * dwScrWd) >> 1) + (px >> 1);
		k = (((y - py) * pstRole->m_wRawWidth) >> 1);  //pstRole->m_wRawWidth is align 16
		l = (((y - py) * pstMaskRole->m_wRawWidth) >> 1); // ex:pstMaskRole->m_wRawWidth=170  pstRole->m_wRawWidth=176
		for (x = px; x < right; x += 2, j++, k++,l++) 
		{
			c = *(pdwMaskBuffer + l);
			if ((c & 0xffff0000) == 0) continue; // skip black pixel

			cy0 = YYCbCr_Y0(c);
			cy1 = YYCbCr_Y1(c);
			if (cy0 > 0xfa && cy1 > 0xfa)
				*(pdwScreenBuffer + j) = *(pdwSourceBuffer + k);
			else {
				cDst = *(pdwScreenBuffer + j);
				c = *(pdwSourceBuffer + k);

				cy = (cy0 + cy1) >> 1;
				cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (256 - cy)) >> 8;
				cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (256 - cy)) >> 8;

				cy0 = (YYCbCr_Y0(c) * cy0 + YYCbCr_Y0(cDst) * (256 - cy0)) >> 8;
				cy1 = (YYCbCr_Y1(c) * cy1 + YYCbCr_Y1(cDst) * (256 - cy1)) >> 8;
				cDst = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);
				*(pdwScreenBuffer + j) = cDst;
			}
		}
	}

	if (pstRole->m_pRawImage) ext_mem_freeEx(pstRole->m_pRawImage, __FILE__, __LINE__);
	if (pstMaskRole->m_pRawImage) ext_mem_freeEx(pstMaskRole->m_pRawImage, __FILE__, __LINE__);

	pstRole->m_pRawImage = NULL;
	pstMaskRole->m_pRawImage = NULL;
}

#if 0
//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Draw Role by FlashClock way - OR
///
///@param   pstRole - specified Role
///
///@param   pTarget - specified Screen buffer
///
///@param   pdwImageBuffer - input image buffer
///
///@param   px - left
///
///@param   py - top
///
///@param   dwScrWd - Screen buffer width
///
///@param   dwScrHt - Screen Buffer Height
///
void xpgRoleDrawFlashClock(STXPGROLE * pstRole, void *pTarget, register DWORD * pdwImageBuffer, DWORD px,
				 DWORD py, DWORD dwScrWd, DWORD dwScrHt)
{
	register DWORD *pdwScreenBuffer = (DWORD *) pTarget;
	DWORD right, bottom;
	MP_DEBUG2("xpgRoleDrawFlashClock %08x %08x", (DWORD)pdwImageBuffer, (DWORD)pTarget );
	MP_ASSERT(pdwImageBuffer != NULL);
	if (pdwImageBuffer == NULL || pTarget == NULL)
		return;
	right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)
		px = 0;
	if (py < 0)
		py = 0;

	DWORD dwLineWidth = (right - px) << 1;
	register DWORD dwScrOffset = dwScrWd >> 1;
	register DWORD dwImgOffset = pstRole->m_wRawWidth >> 1;

	pdwScreenBuffer += ((py * dwScrWd) + px) >> 1;
	for (; py < bottom; py++)
	{
	   int i;
	   for(i=0; i<(dwLineWidth/4); i++) {
	      if(*(pdwImageBuffer+i) != 0x00008080)
            *(pdwScreenBuffer+i) |= *(pdwImageBuffer+i);
     }
		//memcpy(pdwScreenBuffer, pdwImageBuffer, dwLineWidth);

		pdwScreenBuffer += dwScrOffset;
		pdwImageBuffer += dwImgOffset;
	}
}

//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Draw Role by Clip
///
///@param   pstRole - specified Role
///
///@param   pWin - specified Win buffer
///
///@param   pdwImageBuffer - input image buffer
///
///@param   px - left
///
///@param   py - top
///
// Example : Video/Music List's ListFrame display
void xpgRoleDrawInClip(STXPGROLE * pstRole, ST_IMGWIN * pWin, register DWORD * pdwImageBuffer,
					   DWORD px, DWORD py)
{
	register DWORD *pdwScreenBuffer;
	WORD right, bottom;
	MP_DEBUG("xpgRoleDrawInClip");
	MP_ASSERT(pdwImageBuffer != NULL);
	if (pdwImageBuffer == NULL || pWin == NULL || pWin->pdwStart == NULL)
		return;

	if (px > pWin->wClipRight)
		return;
	if (py > pWin->wClipBottom)
		return;

	right = px + pstRole->m_wWidth;
	if (right < pWin->wClipLeft)
		return;
	if (right > pWin->wClipRight)
		right = pWin->wClipRight;

	bottom = py + pstRole->m_wHeight;
	if (bottom < pWin->wClipTop)
		return;
	if (bottom > pWin->wClipBottom)
		bottom = pWin->wClipBottom;

	WORD left, top;

	left--;
	left = (px < pWin->wClipLeft) ? pWin->wClipLeft : px;
	top = (py < pWin->wClipTop) ? pWin->wClipTop : py;

	DWORD dwLineWidth = (right - left) << 1;
	register DWORD dwScrOffset = pWin->wWidth >> 1;
	register DWORD dwImgOffset = pstRole->m_wRawWidth >> 1;

	pdwScreenBuffer = pWin->pdwStart;
	pdwScreenBuffer += (((top * pWin->wWidth) + left) >> 1);
	pdwImageBuffer += ((((top - py) * pstRole->m_wRawWidth) + (left - px)) >> 1);
	for (; top < bottom; top++)
	{
		memcpy(pdwScreenBuffer, pdwImageBuffer, dwLineWidth);

		pdwScreenBuffer += dwScrOffset;
		pdwImageBuffer += dwImgOffset;
	}
}

//---------------------------------------------------------------------------
///@ingroup xpgRole
///@brief   Draw Role by Blend
///
///@param   pstRole - specified Role
///
///@param   pTarget - specified Screen buffer
///
///@param   px - left
///
///@param   py - top
///
///@param   dwScrWd - Screen buffer width
///
///@param   dwScrHt - Screen Buffer Height
///
void xpgRoleDrawBlend(STXPGROLE * pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd,
							DWORD dwScrHt)
{
	register DWORD *pdwScreenBuffer = (DWORD *) pTarget;
	register DWORD *pdwSourceBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k;

	MP_DEBUG("xpgRoleDrawBlend");
	MP_DEBUG("pstRole  %x , pTarget %x",pstRole,pTarget);
	MP_DEBUG("px %d ,  py %d",px,py);
	MP_DEBUG("dwScrWd %d ,  dwScrHt %d",dwScrWd,dwScrHt);


	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	pdwSourceBuffer = (DWORD *) pstRole->m_pRawImage;
	if (pstRole->m_pRawImage == NULL || pTarget == NULL)
	{
		MP_DEBUG("xpgRoleDrawTransperant null");
		return;
	}

	if (px & 1)	px++;
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}
	right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;
#define CY_MAX 0xf0
#define getBlend(cy, a) { cy = a << 1; if (cy < 32) cy = 32; else if (cy > 225) cy = 225; }

	register DWORD cy0, cy1, cb, cr;
	register DWORD c, cDst;
	for (y = py; y < bottom; y++) {
		j = ((y * dwScrWd) >> 1) + (px >> 1);
		k = (((y - py) * pstRole->m_wRawWidth) >> 1);
		for (x = px; x < right; x += 2, j++, k++) {
			c = *(pdwSourceBuffer + k);
			if ((c & 0xffff0000) == 0) continue; // skip black pixel

			cy0 = YYCbCr_Y0(c);
			cy1 = YYCbCr_Y1(c);

			if (cy0 < 24 && cy1 < 24)
				continue;
			if (cy0 > CY_MAX && cy1 > CY_MAX)
				*(pdwScreenBuffer + j) = c;
			else {
				cDst = *(pdwScreenBuffer + j);
				WORD cy;
				getBlend(cy, (cy0 + cy1) >> 1);
				cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (256 - cy)) >> 8;
				cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (256 - cy)) >> 8;

				getBlend(cy, cy0);
				cy0 = (cy0 * cy + YYCbCr_Y0(cDst) * (256 - cy)) >> 8;

				getBlend(cy, cy1);
				cy1 = (cy1 * cy + YYCbCr_Y1(cDst) * (256 - cy)) >> 8;
				cDst = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);
				*(pdwScreenBuffer + j) = cDst;
			}
		}
	}
//	xpgRoleRelease(pstRole);

}

//#if SAMSUNG_DPF
//---------------------------------------------------------------------------
void xpgRoleDrawBlend2(STXPGROLE * pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd,
							DWORD dwScrHt, DWORD percentage)
{
	register DWORD *pdwScreenBuffer = (DWORD *)((DWORD)pTarget | 0xA0000000);
	register DWORD *pdwSourceBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k;
	DWORD lowerLimit = (percentage<<8)/100;

	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	pdwSourceBuffer = (DWORD *) pstRole->m_pRawImage;
	if (pstRole->m_pRawImage == NULL || pTarget == NULL)
	{
		MP_DEBUG("xpgRoleDrawBlend2 null");
		return;
	}

	if (px & 1)	px++;
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}	
	right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;

	if (percentage == 100)
	{
		for (y = py; y < bottom; y++)
		{
			j = ((y * dwScrWd) >> 1) + (px >> 1);
			k = (((y - py) * pstRole->m_wRawWidth) >> 1);
			for (x = px; x < right; x += 2, j++, k++)
			{	
				*(pdwScreenBuffer + j) = *(pdwSourceBuffer + k);
			}
		}
	}
	else
	{
		#define CY_MAX 0xf0
		#define CalBlend(cy, a) { cy = a << 1; if (cy < lowerLimit) cy = lowerLimit; else if (cy > 225) cy = 225; }
		register DWORD cy0, cy1, cb, cr;
		register DWORD c, cDst;
		for (y = py; y < bottom; y++) {
			j = ((y * dwScrWd) >> 1) + (px >> 1);
			k = (((y - py) * pstRole->m_wRawWidth) >> 1);
			for (x = px; x < right; x += 2, j++, k++) {	
				c = *(pdwSourceBuffer + k);
				//if ((c & 0xffff0000) == 0) continue; // skip black pixel
				
				cy0 = YYCbCr_Y0(c);
				cy1 = YYCbCr_Y1(c);			
				cDst = *(pdwScreenBuffer + j);
				
				//if (cy0 < 24 && cy1 < 24) 
				//	continue;
				if (((cy0 > CY_MAX) && (cy1 > CY_MAX)) || ((cDst & 0xffff0000) == 0))
					*(pdwScreenBuffer + j) = c;
				else
				{
					WORD cy;
					CalBlend(cy, (cy0 + cy1) >> 1);
					cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (256 - cy)) >> 8;
					cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (256 - cy)) >> 8;		

					CalBlend(cy, cy0);
					cy0 = (cy0 * cy + YYCbCr_Y0(cDst) * (256 - cy)) >> 8;
					
					CalBlend(cy, cy1);
					cy1 = (cy1 * cy + YYCbCr_Y1(cDst) * (256 - cy)) >> 8;
					cDst = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);
					*(pdwScreenBuffer + j) = cDst;
				}
			}
		}
	}

	xpgRoleRelease(pstRole);
}
//#endif

// add different ink to draw role  - 07.24.2006 Athena
//---------------------------------------------------------------------------
///
///@ingroup xpgRole
///@brief   Draw Role by Ink
///
///@param   pstRole - specified Role
///
///@param   pTarget - specified Screen buffer
///
///@param   px - left
///
///@param   py - top
///
///@param   dwScrWd - Screen buffer width
///
///@param   dwScrHt - Screen Buffer Height
///
///@param   bInk - ink
///
///@param   dwInkValue - specified ink value
///
void xpgRoleDrawInk(STXPGROLE * pstRole, void *pTarget, DWORD px, DWORD py, DWORD dwScrWd,
							DWORD dwScrHt, BYTE bInk, DWORD dwInkValue)
{
	register DWORD *pdwScreenBuffer = (DWORD *) pTarget;
	register DWORD *pdwSourceBuffer;
	DWORD right, bottom;
	register DWORD x, y, j, k;

	MP_DEBUG("xpgRoleDrawTransperant");
	if (pstRole->m_pRawImage == NULL)
	{
		xpgRolePreload(pstRole);
	}
	pdwSourceBuffer = (DWORD *) pstRole->m_pRawImage;
	if (pstRole->m_pRawImage == NULL || pTarget == NULL)
	{
		MP_DEBUG("xpgRoleDrawTransperant null");
		return;
	}

	if (px & 1)	px++;
	if ((DWORD)pdwSourceBuffer & 3) {
		MP_DEBUG("pdwSourceBuffer & 3");
	}
	if ((DWORD)pdwScreenBuffer & 3) {
		MP_DEBUG("pdwScreenBuffer & 3");
	}
	right = px + pstRole->m_wWidth;
	if (right > dwScrWd)
		right = dwScrWd;

	bottom = py + pstRole->m_wHeight;
	if (bottom > dwScrHt)
		bottom = dwScrHt;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;

	register DWORD cy0, cy1, cb, cr;
	register DWORD c, cDst;
	BYTE bInkValue = (BYTE)(dwInkValue & 0xff);

	if (bInk == XPG_INK_DARKEN) {
		for (y = py; y < bottom; y++) {
			j = ((y * dwScrWd) >> 1) + (px >> 1);
			k = (((y - py) * pstRole->m_wRawWidth) >> 1);
			for (x = px; x < right; x += 2, j++, k++) {
				c = *(pdwSourceBuffer + k);

				cy0 = YYCbCr_Y0(c);
				cy1 = YYCbCr_Y1(c);

				cy0 = (cy0 > bInkValue) ? cy0 - bInkValue : 0;
				cy1 = (cy1 > bInkValue) ? cy1 - bInkValue : 0;

				*(pdwScreenBuffer + j) = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | (c & 0xffff);
			}
		}
	} else if (bInk == XPG_INK_WHITEN) {
		for (y = py; y < bottom; y++) {
			j = ((y * dwScrWd) >> 1) + (px >> 1);
			k = (((y - py) * pstRole->m_wRawWidth) >> 1);
			for (x = px; x < right; x += 2, j++, k++) {
				c = *(pdwSourceBuffer + k);

				cy0 = YYCbCr_Y0(c) + bInkValue;
				cy1 = YYCbCr_Y1(c) + bInkValue;

				if (cy0 > 0xff) cy0 = 0xff;
				if (cy1 > 0xff) cy1 = 0xff;

				*(pdwScreenBuffer + j) = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | (c & 0xffff);
			}
		}
	} else if (bInk == XPG_INK_COLOR) {
		for (y = py; y < bottom; y++) {
			j = ((y * dwScrWd) >> 1) + (px >> 1);
			k = (((y - py) * pstRole->m_wRawWidth) >> 1);
			for (x = px; x < right; x += 2, j++, k++) {
				*(pdwScreenBuffer + j) = dwInkValue;
			}
		}
	}

	xpgRoleRelease(pstRole);
}


#define unfixed(v) ((v) >> 16)
#define fixed(v)   ((v) * (1 << 16))
#define MARGIN_MIN 3
#define MARGIN_MAX (16-MARGIN_MIN)

int cos_table[360] =
{
 65536,  65526,  65496,  65446,  65376,  65286,  65176,  65047,
 64898,  64729,  64540,  64331,  64103,  63856,  63589,  63302,
 62997,  62672,  62328,  61965,  61583,  61183,  60763,  60326,
 59870,  59395,  58903,  58393,  57864,  57319,  56755,  56175,
 55577,  54963,  54331,  53684,  53019,  52339,  51643,  50931,
 50203,  49460,  48702,  47930,  47142,  46341,  45525,  44695,
 43852,  42995,  42125,  41243,  40348,  39440,  38521,  37590,
 36647,  35693,  34729,  33753,  32768,  31772,  30767,  29753,
 28729,  27697,  26656,  25607,  24550,  23486,  22415,  21336,
 20252,  19161,  18064,  16962,  15855,  14742,  13626,  12505,
 11380,  10252,   9121,   7987,   6850,   5712,   4572,   3430,
  2287,   1144,      0,  -1143,  -2286,  -3429,  -4570,  -5711,
 -6849,  -7986,  -9120, -10251, -11379, -12504, -13625, -14741,
-15853, -16961, -18063, -19160, -20251, -21335, -22414, -23485,
-24549, -25606, -26655, -27696, -28728, -29752, -30766, -31771,
-32767, -33752, -34728, -35692, -36646, -37589, -38520, -39439,
-40347, -41242, -42125, -42994, -43851, -44694, -45524, -46340,
-47142, -47929, -48702, -49460, -50202, -50930, -51642, -52338,
-53019, -53683, -54331, -54962, -55577, -56174, -56755, -57318,
-57864, -58392, -58902, -59395, -59869, -60325, -60763, -61182,
-61583, -61965, -62328, -62672, -62996, -63302, -63589, -63856,
-64103, -64331, -64540, -64728, -64898, -65047, -65176, -65286,
-65376, -65446, -65496, -65525, -65535, -65526, -65496, -65446,
-65376, -65286, -65177, -65047, -64898, -64729, -64540, -64332,
-64104, -63856, -63589, -63303, -62997, -62672, -62328, -61965,
-61584, -61183, -60764, -60326, -59870, -59396, -58903, -58393,
-57865, -57319, -56756, -56175, -55578, -54963, -54332, -53684,
-53020, -52340, -51643, -50931, -50204, -49461, -48703, -47930,
-47143, -46341, -45526, -44696, -43853, -42996, -42126, -41244,
-40349, -39441, -38522, -37591, -36648, -35694, -34730, -33754,
-32769, -31773, -30768, -29754, -28730, -27698, -26657, -25608,
-24551, -23487, -22416, -21337, -20253, -19162, -18065, -16963,
-15856, -14743, -13627, -12506, -11381, -10253,  -9122,  -7988,
 -6851,  -5713,  -4573,  -3431,  -2288,  -1145,     -1,   1142,
  2285,   3428,   4569,   5710,   6848,   7985,   9119,  10250,
 11378,  12503,  13624,  14740,  15852,  16960,  18062,  19159,
 20250,  21334,  22412,  23484,  24548,  25605,  26654,  27695,
 28727,  29751,  30765,  31770,  32766,  33751,  34727,  35691,
 36645,  37588,  38519,  39439,  40346,  41241,  42124,  42994,
 43850,  44694,  45523,  46339,  47141,  47928,  48701,  49459,
 50202,  50929,  51641,  52338,  53018,  53682,  54330,  54962,
 55576,  56174,  56754,  57318,  57863,  58392,  58902,  59394,
 59869,  60325,  60763,  61182,  61582,  61964,  62327,  62671,
 62996,  63302,  63588,  63855,  64103,  64331,  64539,  64728,
 64897,  65047,  65176,  65286,  65376,  65446,  65496,  65525,
};

int sin_table[360] =
{    0,   1143,   2287,   3429,   4571,   5711,   6850,   7986,
  9120,  10252,  11380,  12504,  13625,  14742,  15854,  16961,
 18064,  19160,  20251,  21336,  22414,  23485,  24550,  25606,
 26655,  27696,  28728,  29752,  30767,  31772,  32767,  33753,
 34728,  35693,  36647,  37589,  38520,  39440,  40347,  41242,
 42125,  42995,  43851,  44695,  45524,  46340,  47142,  47929,
 48702,  49460,  50203,  50930,  51642,  52339,  53019,  53683,
 54331,  54962,  55577,  56175,  56755,  57318,  57864,  58392,
 58903,  59395,  59869,  60326,  60763,  61182,  61583,  61965,
 62328,  62672,  62997,  63302,  63589,  63856,  64103,  64331,
 64540,  64729,  64898,  65047,  65176,  65286,  65376,  65446,
 65496,  65526,  65535,  65526,  65496,  65446,  65376,  65286,
 65177,  65047,  64898,  64729,  64540,  64332,  64104,  63856,
 63589,  63303,  62997,  62672,  62328,  61965,  61583,  61183,
 60764,  60326,  59870,  59396,  58903,  58393,  57865,  57319,
 56756,  56175,  55578,  54963,  54332,  53684,  53020,  52339,
 51643,  50931,  50203,  49461,  48703,  47930,  47143,  46341,
 45525,  44696,  43852,  42996,  42126,  41243,  40348,  39441,
 38521,  37590,  36647,  35694,  34729,  33754,  32768,  31773,
 30768,  29753,  28729,  27697,  26656,  25607,  24551,  23486,
 22415,  21337,  20252,  19161,  18065,  16962,  15855,  14743,
 13626,  12505,  11381,  10253,   9121,   7987,   6851,   5712,
  4572,   3430,   2288,   1144,      1,  -1142,  -2286,  -3428,
 -4570,  -5710,  -6849,  -7985,  -9119, -10250, -11379, -12503,
-13624, -14741, -15853, -16960, -18063, -19159, -20250, -21335,
-22413, -23484, -24549, -25605, -26654, -27695, -28727, -29751,
-30766, -31771, -32766, -33752, -34727, -35692, -36646, -37588,
-38520, -39439, -40346, -41242, -42124, -42994, -43851, -44694,
-45524, -46339, -47141, -47929, -48701, -49459, -50202, -50930,
-51642, -52338, -53018, -53683, -54331, -54962, -55576, -56174,
-56755, -57318, -57864, -58392, -58902, -59395, -59869, -60325,
-60763, -61182, -61583, -61965, -62327, -62671, -62996, -63302,
-63588, -63855, -64103, -64331, -64540, -64728, -64897, -65047,
-65176, -65286, -65376, -65446, -65496, -65525, -65535, -65526,
-65496, -65446, -65376, -65286, -65177, -65047, -64898, -64729,
-64540, -64332, -64104, -63856, -63589, -63303, -62997, -62672,
-62328, -61966, -61584, -61183, -60764, -60326, -59870, -59396,
-58904, -58393, -57865, -57319, -56756, -56176, -55578, -54964,
-54332, -53685, -53020, -52340, -51644, -50932, -50204, -49461,
-48704, -47931, -47143, -46342, -45526, -44696, -43853, -42996,
-42127, -41244, -40349, -39442, -38522, -37591, -36648, -35695,
-34730, -33755, -32769, -31774, -30769, -29754, -28730, -27698,
-26657, -25608, -24552, -23487, -22416, -21338, -20253, -19162,
-18066, -16963, -15856, -14744, -13627, -12506, -11382, -10254,
 -9122,  -7988,  -6852,  -5713,  -4573,  -3432,  -2289,  -1145,
};
//---------------------------------------------------------------------------
void xpgRoleDrawStretchRotate(STXPGROLE * role, void *img_trg, BYTE *img_src, BYTE *img_mask,
			DWORD left, DWORD top, DWORD ox, DWORD oy, DWORD canvas_w, DWORD canvas_h, int scale_w, int scale_h, int blend, int angle)
{
	if( img_trg == NULL || img_src == NULL ) return;

    int i;
    int iCanvasWidth  = canvas_w;
	int iCanvasHeight = canvas_h;
	int iBmpWidth  = role->m_wWidth;
	int iBmpHeight = role->m_wHeight;

	int bmp_w = iBmpWidth*scale_w/100;
	int bmp_h = iBmpHeight*scale_h/100;
	if (bmp_w == 0 || bmp_h == 0)
	{
		return;
	}

	/* caculate rotated image target width and height */
	int nx[4];
	int ny[4];

    while (angle < 0)
        angle += 360;
	angle = angle % 360;
#if 0
	float a = 0.0174532*((float)-angle);

	int cos0 = fixed(cosf(a));
	int sin0 = fixed(sinf(a));
	int cos1 = fixed(cosf(-a));
	int sin1 = fixed(sinf(-a));
#else
	int cos0 = cos_table[360-angle];
	int sin0 = sin_table[360-angle];
	int cos1 = cos_table[angle];
	int sin1 = sin_table[angle];
#endif

    int ox0 = ox;
	int oy0 = oy;
	ox = ox * scale_w/100;
	oy = oy * scale_h/100;

	/* make stretch by center */
	//left -= (bmp_w - iBmpWidth) / 2;
	//top -= (bmp_h - iBmpHeight) / 2;
	left -= ox - ox0;
	top -= oy - oy0;

	/* get rotated x, y range */
	/* move (-ox, -oy) */
	//ox = bmp_w >> 1;
	//oy = bmp_h >> 1;
	int x0 = -ox;
	int y0 = -oy;
	int x1 = x0 + bmp_w - 1;
	int y1 = y0 + bmp_h - 1;

	//x' = x cos f + y sin f
	//y' = y cos f - x sin f
	nx[0] = unfixed(x0 * cos0 + y0 * sin0);
	ny[0] = unfixed(y0 * cos0 - x0 * sin0);
	nx[1] = unfixed(x1 * cos0 + y0 * sin0);
	ny[1] = unfixed(y0 * cos0 - x1 * sin0);
	nx[2] = unfixed(x1 * cos0 + y1 * sin0);
	ny[2] = unfixed(y1 * cos0 - x1 * sin0);
	nx[3] = unfixed(x0 * cos0 + y1 * sin0);
	ny[3] = unfixed(y1 * cos0 - x0 * sin0);

	int xMin = x1, xMax = 0;
	int yMin = y1, yMax = 0;
	for (i = 0; i < 4; i++)
	{
		if (nx[i] < xMin)
		{
			xMin = nx[i];
		}
		if (ny[i] < yMin)
		{
			yMin = ny[i];
		}
		if (nx[i] > xMax)
		{
			xMax = nx[i];
		}
		if (ny[i] > yMax)
		{
			yMax = ny[i];
		}
	}
    if (xMin & 1) xMin++;

	//printf("xMin %d xMax %d  - yMin %d yMax %d\n", xMin, xMax, yMin, yMax);
	left += xMin + ox;
	top += yMin + oy;

	/* set bytes count per line of bitmap and canvas */
	int iBmpBytesPerLine = role->m_wRawWidth << 1;
	int iCanvasBytesPerLine = iCanvasWidth << 1;
	int yCanvas = top;
	int xCanvas = left;
	int yBmp = 0;

	int alpha = ((100 - blend) << 8) / 100;

	/* check if out of canvas */
	if (top + yMax - yMin > iCanvasHeight) {
		yMax = iCanvasHeight + yMin - top - 1;
	}

	if (left + xMax - xMin > iCanvasWidth) {
		xMax = iCanvasWidth + xMin - left - 1;
	}

	int mask_cy = 256;
	int x, y;

	BOOL boScale = (scale_w != 100 && scale_h != 100);
	if (boScale)
	{
	    scale_w = fixed(100) / scale_w;
	    scale_h = fixed(100) / scale_h;
	}
	for (y = yMin; y < yMax && yCanvas < iCanvasHeight; y++, yCanvas++, yBmp++)
	{
		if (yCanvas < 0) continue;

		unsigned char *pCanvasAddr = (unsigned char *)(img_trg + yCanvas * iCanvasBytesPerLine + (xCanvas>>1) * 4);

		for (x = xMin; x < xMax; x+=2, pCanvasAddr+=4)
		{
			/* Y0 Y1 Cb Cr, left is x+0, right is x+1 */
			int xL0 = (x+0) * cos1 + y * sin1;
			int xR0 = (x+1) * cos1 + y * sin1;
			int yL0 = y * cos1 - (x+0) * sin1;
			int yR0 = y * cos1 - (x+1) * sin1;

			int xL = unfixed(xL0)+ox;
			int xR = unfixed(xR0)+ox;
			int yL = unfixed(yL0)+oy;
			int yR = unfixed(yR0)+oy;

            if (boScale)
            {
                xL = unfixed(xL*scale_w);
                xR = unfixed(xR*scale_w);
                yL = unfixed(yL*scale_h);
                yR = unfixed(yR*scale_h);
            }

			if (xL < 0 || xL >= iBmpWidth || xR < 0 || xR >= iBmpWidth )  // TODO : should check individual ....
			{
				continue;
			}

			if (yL < 0 || yL >= iBmpHeight || yR < 0 || yR >= iBmpHeight)
			{
				continue;
			}
			unsigned char *pBmpAddrL = (unsigned char *)(img_src + yL * iBmpBytesPerLine + (xL>>1)*4);
			unsigned char *pBmpAddrR = (unsigned char *)(img_src + yR * iBmpBytesPerLine + (xR>>1)*4);

			unsigned char *pyL = pBmpAddrL+3-(xL&1);  // get from pixel L
			unsigned char *pyR = pBmpAddrR+3-(xR&1);  // get from pixel R
			unsigned char *pcbL = pBmpAddrL+1;
			unsigned char *pcrL = pBmpAddrL+0;
			unsigned char *pcbR = pBmpAddrR+1;
			unsigned char *pcrR = pBmpAddrR+0;

            if (img_mask == NULL)
            {
                /* edge anti-aliasing */
                if (yL == 0 )
                    mask_cy = ((yL0 & 0xffff) >> 8);
                else if (yL == iBmpHeight-1)
                    mask_cy = 257 - ((yL0 & 0xffff) >> 8);
                else if (yR == 0)
                    mask_cy = ((yR0 & 0xffff) >> 8);
                else if (yR == iBmpHeight-1)
                    mask_cy = 257 - ((yR0 & 0xffff) >> 8);
                else if (xL == 0)
                    mask_cy = ((xL0 & 0xffff) >> 8);
                else if (xL == iBmpWidth-1)
                    mask_cy = 257 - ((xL0 & 0xffff) >> 8);
                else if (xR == 0)
                    mask_cy = ((xR0 & 0xffff) >> 8);
                else if (xR == iBmpWidth-1)
                    mask_cy = 257 - ((xR0 & 0xffff) >> 8);
                else
                    mask_cy = 255;
            }
            else
            {
                mask_cy = (*(img_mask + yL * iBmpBytesPerLine + (xL>>1)*4+3) +
                           *(img_mask + yR * iBmpBytesPerLine + (xR>>1)*4+3)) >> 1;
            }

            /* goto to check endian */
			if (mask_cy >= 255 && alpha >= 255)
			{
				pCanvasAddr[3] = *pyL;
				pCanvasAddr[2] = *pyR;
 				pCanvasAddr[1] = (*pcbL + *pcbR)>>1;
				pCanvasAddr[0] = (*pcrL + *pcrR)>>1;
			}
			else
			{
				int dst_y0 = pCanvasAddr[3];
				int dst_y1 = pCanvasAddr[2];
				int dst_cb = pCanvasAddr[1];
				int dst_cr = pCanvasAddr[0];

				int src_a = mask_cy * alpha;
				int dst_a = 65536 - src_a;
				unsigned long cy0 = *pyL;
				unsigned long cy1 = *pyR;
 				unsigned long cb = (*pcbL + *pcbR)>>1;
				unsigned long cr = (*pcrL + *pcrR)>>1;

				pCanvasAddr[3] = (((dst_a * dst_y0) + (src_a * cy0)) >> 16) & 0xff;
				pCanvasAddr[2] = (((dst_a * dst_y1) + (src_a * cy1)) >> 16) & 0xff;
				pCanvasAddr[1] = (((dst_a * dst_cb) + (src_a * cb)) >> 16) & 0xff;
				pCanvasAddr[0] = (((dst_a * dst_cr) + (src_a * cr)) >> 16) & 0xff;
			}
		}
    }
}
// ************ End of xpgRoleDrawStretchRotate() ******************************
void xpgRoleDrawMaskScaleBlendRotate(STXPGROLE * role, STXPGSPRITE * pstSprite, void *img_trg, BYTE *img_src, BYTE *img_mask,  
			DWORD left, DWORD top, DWORD ox, DWORD oy, DWORD canvas_w, DWORD canvas_h, int scale_w, int scale_h, int blend, int angle)
{
    //mpDebugPrint("xpgRoleDrawMaskScaleBlendRotate()");
	if( img_trg == NULL || img_src == NULL ) return;
	 
	//mpDebugPrint("MaskScaleBlendRotate: role->m_wWidth = %d, role->m_wHeight = %d, role->m_wRawWidth = %d", role->m_wWidth, role->m_wHeight, role->m_wRawWidth);
	//scale_w = 80; scale_h = 80; // test
	BYTE *pbScaleSrcAllocBuf = NULL;
	DWORD *pScaleSrcBuf = NULL;
	BYTE *pbScaleMaskAllocBuf = NULL;
	DWORD *pScaleMaskBuf = NULL;
	WORD targetW, targetH;  
    ST_IMGWIN sSrcWin, sTargetWin;
	
	STXPGROLE stRoleBackup;
	memcpy((BYTE *)&stRoleBackup, (BYTE *)role, sizeof(STXPGROLE));
	if(scale_w != 100 || scale_h != 100)// Do Scale img_src
	{
	   
    	DWORD dwScaleSrcSize;
    	targetW = role->m_wRawWidth  * scale_w / 100;
    	targetW &= 0xFFFE; // to EVEN targetW = ALIGN_16(targetW);
    	targetH = role->m_wHeight * scale_h / 100;
    	//mpDebugPrint("targetW = %d, targetH = %d", targetW, targetH);
    	dwScaleSrcSize = (ALIGN_16(targetW) + 16) * ALIGN_16(targetH) * 2 + 4096;
    	pbScaleSrcAllocBuf = (BYTE *)ext_mem_mallocEx(dwScaleSrcSize, __FILE__, __LINE__);
    	if (pbScaleSrcAllocBuf == NULL) {
    		MP_ALERT("ScaleSrc size %d > buffer alloc fail", dwScaleSrcSize);
    		return;
    	}
    	pScaleSrcBuf = (DWORD *) ((DWORD) pbScaleSrcAllocBuf | 0x20000000);
        
        sSrcWin.pdwStart = img_src;
        sSrcWin.dwOffset = role->m_wRawWidth << 1;
    	sSrcWin.wHeight = role->m_wHeight;  // simulate pstRole->m_wRawHeight
    	sSrcWin.wWidth = role->m_wRawWidth;
    	
        sTargetWin.pdwStart = pScaleSrcBuf;
        sTargetWin.dwOffset = targetW << 1;
    	sTargetWin.wHeight = targetH;
    	sTargetWin.wWidth = targetW;
    	Ipu_ImageScaling(&sSrcWin, &sTargetWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, 0, 0, 0+targetW, 0+targetH, 0);
    	img_src = pScaleSrcBuf;
    }
    if(img_mask == NULL)
	{
	    if(scale_w != 100 || scale_h != 100)
	    {
	        // Recalculate left/top by Scalew/Scaleh
            left += ox - (targetW / 2);
            top  += oy - (targetH / 2);
            
   	        role->m_wWidth = targetW;
    	    role->m_wHeight = targetH;
    	    role->m_wRawWidth = targetW;
    	}
        if(angle == 0)
        {
        	ST_IMGWIN *pWin = Idu_GetNextWin();
        	xpgRoleDraw(role, pWin->pdwStart, img_src, left, top, pWin->wWidth, pWin->wHeight);
        }
        else    	
    	    xpgRoleDrawMaskBlendRotate(role, img_trg, img_src, NULL, left, top, ox, oy, canvas_w, canvas_h, blend, angle);
    }
    else if(scale_w != 100 || scale_h != 100)// Do Scale img_mask
	{	     	
    	DWORD dwScaleMaskSize;
    	dwScaleMaskSize = (ALIGN_16(targetW) + 16) * ALIGN_16(targetH) * 2 + 4096;
    	pbScaleMaskAllocBuf = (BYTE *)ext_mem_mallocEx(dwScaleMaskSize, __FILE__, __LINE__);
    	if (pbScaleMaskAllocBuf == NULL) {
    		MP_ALERT("ScaleMask size %d > buffer alloc fail", dwScaleMaskSize);
    		return;
    	}
    	pScaleMaskBuf = (DWORD *) ((DWORD) pbScaleMaskAllocBuf | 0x20000000);
    	sSrcWin.pdwStart = img_mask;
        sSrcWin.dwOffset = role->m_wRawWidth << 1;
    	sSrcWin.wHeight = role->m_wHeight;  // simulate pstRole->m_wRawHeight
    	sSrcWin.wWidth = role->m_wRawWidth;
    	sTargetWin.pdwStart = pScaleMaskBuf;
        sTargetWin.dwOffset = targetW << 1;
    	sTargetWin.wHeight = targetH;
    	sTargetWin.wWidth = targetW;
    	Ipu_ImageScaling(&sSrcWin, &sTargetWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, 0, 0, 0+targetW, 0+targetH, 0);
    	img_mask = pScaleMaskBuf;
    	
    	role->m_wWidth = targetW;
    	role->m_wHeight = targetH;
    	role->m_wRawWidth = targetW;
    	
    	xpgRoleDrawMaskBlendRotate(role, img_trg, img_src, img_mask, left, top, ox, oy, canvas_w, canvas_h, blend, angle);
	}
	else
	    xpgRoleDrawMaskBlendRotate(role, img_trg, img_src, img_mask, left, top, ox, oy, canvas_w, canvas_h, blend, angle);
	
	if (pbScaleSrcAllocBuf) ext_mem_freeEx(pbScaleSrcAllocBuf, __FILE__, __LINE__);
	if (pbScaleMaskAllocBuf) ext_mem_freeEx(pbScaleMaskAllocBuf, __FILE__, __LINE__);
	memcpy((BYTE *)role, (BYTE *)&stRoleBackup, sizeof(STXPGROLE));
}

void xpgRoleDrawMaskBlendRotate(STXPGROLE * role, void *img_trg, BYTE *img_src, BYTE *img_mask,
			DWORD left, DWORD top, DWORD ox, DWORD oy, DWORD canvas_w, DWORD canvas_h, int blend, int angle)
{
    //mpDebugPrint("xpgRoleDrawMaskBlendRotate()");
	if( img_trg == NULL || img_src == NULL ) return;
		
	while (angle < 0)
        angle += 360;
	angle = angle % 360;
	if(angle == 0) // Do MaskBlend only and Show *******
    {
        xpgRoleDrawMaskBlend(role, img_trg, img_src, img_mask, left, top, canvas_w, canvas_h, blend);
    	return;
    } // End MaskBlend ********
    int i;
    int iCanvasWidth  = canvas_w;
	int iCanvasHeight = canvas_h;
	int iBmpWidth  = role->m_wWidth;
	int iBmpHeight = role->m_wHeight;

	int bmp_w = iBmpWidth;
	int bmp_h = iBmpHeight;

	/* caculate rotated image target width and height */
	int nx[4];
	int ny[4];

    //while (angle < 0)
    //    angle += 360;
	//angle = angle % 360;

	int cos0 = cos_table[360-angle];
	int sin0 = sin_table[360-angle];
	int cos1 = cos_table[angle];
	int sin1 = sin_table[angle];

    int ox0 = ox;
	int oy0 = oy;

	/* make stretch by center */
	//left -= (bmp_w - iBmpWidth) / 2;
	//top -= (bmp_h - iBmpHeight) / 2;
	left -= ox - ox0;
	top -= oy - oy0;

	/* get rotated x, y range */
	/* move (-ox, -oy) */
	//ox = bmp_w >> 1;
	//oy = bmp_h >> 1;
	int x0 = -ox;
	int y0 = -oy;
	int x1 = x0 + bmp_w - 1;
	int y1 = y0 + bmp_h - 1;

	//x' = x cos f + y sin f
	//y' = y cos f - x sin f
	nx[0] = unfixed(x0 * cos0 + y0 * sin0);
	ny[0] = unfixed(y0 * cos0 - x0 * sin0);
	nx[1] = unfixed(x1 * cos0 + y0 * sin0);
	ny[1] = unfixed(y0 * cos0 - x1 * sin0);
	nx[2] = unfixed(x1 * cos0 + y1 * sin0);
	ny[2] = unfixed(y1 * cos0 - x1 * sin0);
	nx[3] = unfixed(x0 * cos0 + y1 * sin0);
	ny[3] = unfixed(y1 * cos0 - x0 * sin0);

	int xMin = x1, xMax = 0;
	int yMin = y1, yMax = 0;
	for (i = 0; i < 4; i++)
	{
		if (nx[i] < xMin)
		{
			xMin = nx[i];
		}
		if (ny[i] < yMin)
		{
			yMin = ny[i];
		}
		if (nx[i] > xMax)
		{
			xMax = nx[i];
		}
		if (ny[i] > yMax)
		{
			yMax = ny[i];
		}
	}
    if (xMin & 1) xMin++;

	//printf("xMin %d xMax %d  - yMin %d yMax %d\n", xMin, xMax, yMin, yMax);
	left += xMin + ox;
	top += yMin + oy;

	/* set bytes count per line of bitmap and canvas */
	int iBmpBytesPerLine = role->m_wRawWidth << 1;
	int iCanvasBytesPerLine = iCanvasWidth << 1;
	int yCanvas = top;
	int xCanvas = left;
	int yBmp = 0;

	int alpha = 256;

	/* check if out of canvas */
	if (top + yMax - yMin > iCanvasHeight) {
		yMax = iCanvasHeight + yMin - top - 1;
	}

	if (left + xMax - xMin > iCanvasWidth) {
		xMax = iCanvasWidth + xMin - left - 1;
	}

	int mask_cy = 256;
	int x, y;

    DWORD k;
	for (y = yMin; y < yMax && yCanvas < iCanvasHeight; y++, yCanvas++, yBmp++)
	{
		if (yCanvas < 0) continue;
		k = (((y - yMin) * role->m_wRawWidth) >> 1);

		unsigned char *pCanvasAddr = (unsigned char *)(img_trg + yCanvas * iCanvasBytesPerLine + (xCanvas>>1) * 4);

		for (x = xMin; x < xMax; x+=2, pCanvasAddr+=4, k++)
		{
			/* Y0 Y1 Cb Cr, left is x+0, right is x+1 */
			int xL0 = (x+0) * cos1 + y * sin1;
			int xR0 = (x+1) * cos1 + y * sin1;
			int yL0 = y * cos1 - (x+0) * sin1;
			int yR0 = y * cos1 - (x+1) * sin1;

			int xL = unfixed(xL0)+ox;
			int xR = unfixed(xR0)+ox;
			int yL = unfixed(yL0)+oy;
			int yR = unfixed(yR0)+oy;

			if (xL < 0 || xL >= iBmpWidth || xR < 0 || xR >= iBmpWidth )  // TODO : should check individual ....
			{
				continue;
			}

			if (yL < 0 || yL >= iBmpHeight || yR < 0 || yR >= iBmpHeight)
			{
				continue;
			}
			unsigned char *pBmpAddrL = (unsigned char *)(img_src + yL * iBmpBytesPerLine + (xL>>1)*4);
			unsigned char *pBmpAddrR = (unsigned char *)(img_src + yR * iBmpBytesPerLine + (xR>>1)*4);

            //unsigned char *pyL = pBmpAddrL+3-(xL&1);  // get from pixel L
			unsigned char *pyR = pBmpAddrR+3-(xR&1);  // get from pixel R
			//unsigned char *pcbL = pBmpAddrL+1;
			//unsigned char *pcrL = pBmpAddrL+0;
			unsigned char *pcbR = pBmpAddrR+1;
			unsigned char *pcrR = pBmpAddrR+0;
			
			unsigned char *pyL  = pBmpAddrL+3;  // get from pixel L
			unsigned char *pyL1 = pBmpAddrL+2;  // get from pixel L
			unsigned char *pcbL = pBmpAddrL+1;
			unsigned char *pcrL = pBmpAddrL+0;
				
			if(img_mask == NULL)
                mask_cy = 256;
            else
			    //mask_cy = (*(img_mask + yL * iBmpBytesPerLine + (xL>>1)*4+3) + *(img_mask + yR * iBmpBytesPerLine + (xR>>1)*4+3)) >> 1;
                mask_cy = *(img_mask + yL * iBmpBytesPerLine + (xL>>1)*4);

			//if (mask_cy >= 255)
			if (mask_cy >= 32)
			{
				pCanvasAddr[3] = *pyL;
				pCanvasAddr[2] = *pyL1; //*pyR;
				pCanvasAddr[1] = ((*pcbL + *pcbR)>>1) * (100 - blend) / 100;
    			pCanvasAddr[0] = ((*pcrL + *pcrR)>>1) * (100 - blend) / 100;
			}
		
		}
    }

}
void xpgRoleDrawMaskBlend(STXPGROLE * role, void *img_trg, BYTE *img_src, BYTE *img_mask, DWORD px, DWORD py, DWORD canvas_w,
							DWORD canvas_h, int blend)
{
	WORD *canvas = (WORD *) img_trg;
	WORD *source = img_src;
	WORD *mask = img_mask;
	DWORD right, bottom;
	DWORD x, y, j, k;

	MP_DEBUG("xpgRoleDrawMaskBlend");
	if (role == NULL || img_src == NULL ||img_trg == NULL)
	{
		MP_ALERT("xpgRoleDrawMaskBlend: role == null || img_src == NULL img_trg == NULL");
		return;
	}

	//source = (WORD *)xpgDecodeRole(role, NULL);
	//mask = (WORD *)xpgDecodeRole(pstMaskRole, NULL);;

	if (px & 1)	px++;
	//if ((DWORD)mask & 3) {
	//	MP_DEBUG("source & 3");
	//}
	if ((DWORD)source & 3) {
		MP_DEBUG("source & 3");
	}
	if ((DWORD)canvas & 3) {
		MP_DEBUG("canvas & 3");
	}
	right = px + role->m_wWidth;
	if (right > canvas_w)
		right = canvas_w;

	bottom = py + role->m_wHeight;
	if (bottom > canvas_h)
		bottom = canvas_h;

	if (px < 0)	px = 0;
	if (py < 0)	py = 0;

    //SDL_LockSurface(iplay->canvas);

	DWORD cy0, cy1, cb, cr;
	DWORD c, cDst;
	WORD cy;

	blend = (100 - blend) * 256 / 100;
	for (y = py; y < bottom; y++)
	{
		j = ((y * canvas_w) >> 1) + (px >> 1);
		k = (((y - py) * role->m_wRawWidth) >> 1);
		for (x = px; x < right; x += 2, j++, k++)
		{
			if(mask == NULL)
			    c = 0xffff0000; 
			else
                c = *((DWORD *)mask + k);
			cy0 = YYCbCr_Y0(c);
			cy1 = YYCbCr_Y1(c);
			cy = (cy0 + cy1) >> 1;

			cDst = *((DWORD *)canvas + j);
			c = *((DWORD *)source + k);

			if (blend == 0)
			{
                cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (256 - cy)) >> 8;
                cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (256 - cy)) >> 8;

                cy0 = (YYCbCr_Y0(c) * cy0 + YYCbCr_Y0(cDst) * (256 - cy0)) >> 8;
                cy1 = (YYCbCr_Y1(c) * cy1 + YYCbCr_Y1(cDst) * (256 - cy1)) >> 8;
			}
			else
			{
				cy = cy * blend;
				cy0 = cy0 * blend;
				cy1 = cy1 * blend;
                cb = (YYCbCr_Cb(c) * cy + YYCbCr_Cb(cDst) * (65536 - cy)) >> 16;
                cr = (YYCbCr_Cr(c) * cy + YYCbCr_Cr(cDst) * (65536 - cy)) >> 16;

                cy0 = (YYCbCr_Y0(c) * cy0 + YYCbCr_Y0(cDst) * (65536 - cy0)) >> 16;
                cy1 = (YYCbCr_Y1(c) * cy1 + YYCbCr_Y1(cDst) * (65536 - cy1)) >> 16;
			}

			*((DWORD *)canvas + j) = ((cy0 & 0xff) << 24) | ((cy1 & 0xff) << 16) | ((cb & 0xff) << 8) | (cr & 0xff);

		}
	}

    //SDL_UnlockSurface(iplay->canvas);
}
// ************ End of xpgRoleDrawMaskScaleBlendRotate() ******************************

void xpgRoleDrawScaleBlendRotate(STXPGROLE * role, STXPGSPRITE * pstSprite, void *img_trg, BYTE *img_src, DWORD left, DWORD top, DWORD ox, DWORD oy, DWORD canvas_w, DWORD canvas_h, int scale_w, int scale_h, int blend, int angle)
{
    //mpDebugPrint("xpgRoleDrawScaleBlendRotate() -----------------------------------");
	if( img_trg == NULL || img_src == NULL ) return;
	BYTE *pbScaleAllocBuf = NULL;
	//mpDebugPrint("left = %d, top = %d, ox = %d, oy = %d", left, top, ox, oy);
    //mpDebugPrint("scale_w = %d, scale_h = %d", scale_w, scale_h);
    
	// Sprite type == AniPhoto - *.JPG's wRawWidth/wRawHeight is not same as Sprite's Width/Height
	// Sprite type == NoType - role's wRawWidth/wRawHeight is almost same as Sprite's Width/Height
    if(	!(role->m_wRawWidth == pstSprite->m_wWidth && role->m_wHeight == pstSprite->m_wHeight))
    {
    	ST_IMGWIN sSrcWin;
    	ST_IMGWIN * pWin = Idu_GetNextWin();
    	
    	sSrcWin.pdwStart = img_src; //pImgBuf;
        sSrcWin.dwOffset = role->m_wRawWidth << 1;
    	sSrcWin.wHeight = role->m_wHeight;  // simulate pstRole->m_wRawHeight
    	sSrcWin.wWidth = role->m_wRawWidth;
    	//mpDebugPrint("sSrcWin.dwOffset = %d, sSrcWin.wWidth=%d, sSrcWin.wHeight=%d", sSrcWin.dwOffset, sSrcWin.wWidth, sSrcWin.wHeight);
    	
    	WORD xs = pstSprite->m_wPx;
    	WORD ys = pstSprite->m_wPy;
    	WORD ws = pstSprite->m_wWidth;
    	WORD hs = pstSprite->m_wHeight;
    	//mpDebugPrint("pstSprite->m_wWidth = %d, pstSprite->m_wHeight = %d", pstSprite->m_wWidth, pstSprite->m_wHeight);
    #if 0	// show role to verify
        ws &= 0xFFFE; // to EVEN ws = ALIGN_16(ws);
    	Ipu_ImageScaling(&sSrcWin, pWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, xs, ys, xs+ws, ys+hs, 0);
    	return;
    #endif	
    
    	DWORD *pScaleBuf = NULL;
    	DWORD dwScaleSize;
    	WORD targetW = pstSprite->m_wWidth  * scale_w / 100;
    	targetW &= 0xFFFE; // to EVEN  targetW = ALIGN_16(targetW);
    	WORD targetH = pstSprite->m_wHeight * scale_h / 100;
    	//mpDebugPrint("targetW = %d, targetH = %d", targetW, targetH);
    	dwScaleSize = (ALIGN_16(targetW) + 16) * ALIGN_16(targetH) * 2 + 4096;
    	pbScaleAllocBuf = (BYTE *)ext_mem_mallocEx(dwScaleSize, __FILE__, __LINE__);
    	if (pbScaleAllocBuf == NULL) {
    		MP_ALERT("Scale size %d > buffer alloc fail", dwScaleSize);
    		return;
    	}
    	pScaleBuf = (DWORD *) ((DWORD) pbScaleAllocBuf | 0x20000000);
    	
    	ST_IMGWIN sTargetWin;
        sTargetWin.pdwStart = pScaleBuf;
        sTargetWin.dwOffset = targetW << 1;
    	sTargetWin.wHeight = targetH;
    	sTargetWin.wWidth = targetW;
    	Ipu_ImageScaling(&sSrcWin, &sTargetWin, 0, 0, sSrcWin.wWidth, sSrcWin.wHeight, 0, 0, 0+targetW, 0+targetH, 0);
    
        // Recalculate left/top by Scalew/Scaleh
        left += ox - (targetW / 2);
        top  += oy - (targetH / 2);
        
        role->m_wWidth = targetW; //pstSprite->m_wWidth;
    	role->m_wHeight = targetH; //pstSprite->m_wHeight;
    	role->m_wRawWidth = targetW; //pstSprite->m_wWidth;
    
    	if(angle == 0 && blend == 0)
    	{
           xpgRoleDraw(role, pWin->pdwStart, (void *)pScaleBuf, left, top, pWin->wWidth, pWin->wHeight);
    	   if (pbScaleAllocBuf) ext_mem_freeEx(pbScaleAllocBuf, __FILE__, __LINE__);
    	   return;
        }
        else
            img_src = pScaleBuf;
    }
    
    xpgRoleDrawMaskBlendRotate(role, img_trg, img_src, NULL, left, top, ox, oy, canvas_w, canvas_h, blend, angle);
    if (pbScaleAllocBuf) ext_mem_freeEx(pbScaleAllocBuf, __FILE__, __LINE__);
    return;

}
// ************ End of xpgRoleDrawScaleBlendRotate() ******************************
#endif

