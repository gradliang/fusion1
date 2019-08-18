/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "ipu.h"

#define FIX_POINT(a) ((a) << 8)
#define FIX_POINT_R(a) ((a) >> 8)


#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)

// if scale up
//   dwSrc must > (dwTrg ¡V1) * IP_MNSC_VSF / 256 + 1.
//   wSf = ((dwSrc - 1) * 256) / (dwTrg - 1) + 1;

// if scale down
//   dwSrc must > (dwTrg ¡V1) *  (IP_MNSC_VSF + 256)	/ 256 + 1.
//   (IP_MNSC_VSF + 256) must <= ((dwSrc - 1) / 256) / (dwTrg - 1)
//   IP_MNSC_VSF must <= (((dwSrc - 1) / 256) / (dwTrg - 1)) - 256

// return the scaling factor of the input
// the scale ratio is  1/2 ~ 256

static WORD GetSF(DWORD dwSrc, DWORD dwTrg, BYTE ratio, BYTE flag)
{
	WORD wSf = 0;

	if (dwTrg <= 1)
		dwTrg = 2;				/* make sure there's no division by 0 */

	if (dwSrc == dwTrg)
		return 0;
	else if (dwSrc == (dwTrg << ratio))
	{
		return 1;
	}
	else if (flag == SCALING_UP)
	{
		if (((dwSrc << 1) == dwTrg) || ((dwSrc << 2) == dwTrg) || ((dwSrc << 3) == dwTrg))
		{
			wSf = (((dwSrc - 1) * 256) - 128) / (dwTrg - 1);
		}
		else
		{
		    wSf = ((dwSrc - 1) * 256 + 1) / dwTrg;
		    if (wSf > 0) wSf--;
		    if (wSf >= 0xff)
			    wSf = 0xff;
		    else
			    wSf &= 0xff;
		}
	}
	else if (flag == SCALING_DOWN)
	{
		wSf = (((dwSrc - 1) * 256) / ((dwTrg << ratio) - 1));
		if (wSf <= 256)
			wSf = 0;
		else if (wSf >= 512)
			wSf = 0xff;
		else
			wSf &= 0xff;
	}

	wSf &= 0xff;
	return wSf;
}

// Calculate post image sub-sample ratio
// x1 ~ x1/16
static BYTE GetSubSampleRatio(register WORD wSrc, register WORD wTrg)
{
	register BYTE i;

	for (i = 8; i > 0; i--)
	{
		if ((wSrc >> i) >= wTrg) {
			if (i > 4) {
				MP_DEBUG("down size %d too much!", i);
				i = 4;
			}
			return i;
		}
	}

	return 0;
}


static int GetScaParm(ST_SCA_PARM * psScaParm, WORD SrcWidth, WORD SrcHeight, WORD TrgWidth, WORD TrgHeight)
{
	psScaParm->wHSubRatio = 0;
	psScaParm->wVSubRatio = 0;

	if (SrcWidth > TrgWidth)
	{							// image scaling down
		psScaParm->wHUp = SCALING_DOWN;
	}
	else if (SrcWidth  < TrgWidth)
	{							// image scaling up
		psScaParm->wHUp = SCALING_UP;
	}
	else
	{
		psScaParm->wHUp = NON_SCALING;
	}

	if (SrcHeight > TrgHeight)
	{							// image scaling down
		psScaParm->wVUp = SCALING_DOWN;
	}
	else if (SrcHeight < TrgHeight)
	{
		psScaParm->wVUp = SCALING_UP;	// image scaling up
	}
	else
	{
		psScaParm->wVUp = NON_SCALING;	// image none scale
	}

	if (psScaParm->wHUp)
		psScaParm->wVSubRatio = GetSubSampleRatio(SrcHeight, TrgHeight);

	psScaParm->wHSubRatio = GetSubSampleRatio(SrcWidth, TrgWidth);

	if (SrcWidth == (TrgWidth << psScaParm->wHSubRatio)) {
		psScaParm->wHUp = NON_SCALING;
		psScaParm->wHSF = 1;
	}
	else
		psScaParm->wHSF = GetSF(SrcWidth,  TrgWidth, psScaParm->wHSubRatio, psScaParm->wHUp);

	if (SrcHeight == (TrgHeight << psScaParm->wVSubRatio)) { //Dennis 3/21: miss typo
		psScaParm->wVUp = NON_SCALING;
		psScaParm->wVSF = 1;
	}
	else
		psScaParm->wVSF = GetSF(SrcHeight, TrgHeight, psScaParm->wVSubRatio, psScaParm->wVUp);

	return PASS;
}

static void ipu_reset()
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	BIU *biu;

	biu = (BIU *) BIU_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	// asynchronous IPU reset
	biu->BiuArst &= 0xfffffffb;
	biu->BiuArst |= 0x00000004;

	// disable
	dmaR->Control = 0x0;
	dmaW->Control = 0x0;
	ipu->IpIpw10 = 0x0;
	ipu->IpIpr0 = 0x0;

	//Initial MNSC
	ipu->IpMnsc0 = 0x00;
	ipu->IpMnsc1 = 0x00;
	ipu->IpMnsc2 = 0x00;
	ipu->IpMnsc3 = 0x00;
	ipu->IpMnsc4 = 0x00;
}


static int ipu_scale(ST_IMAGEINFO * psSource, ST_IMAGEINFO * psTarget)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	int ret;
	BIU *biu;
	CLOCK *clock;
	WORD wSrcWd0, wTrgWd0;
	WORD wSrcWd, wTrgWd, wSrcHt, wTrgHt, wSrcPitch, wTrgPitch, tm;
	DWORD wSrcOffset, wTrgOffset;
	CHANNEL *dma;
	ST_SCA_PARM sScaParm;
	ST_SCA_PARM *psScaParm;
	BYTE *tmp_buffer = NULL; //, SCALE_MAX;

	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;

	wSrcWd = psSource->wWidth;
	wTrgWd = psTarget->wWidth;
	wSrcHt = psSource->wHeight;
	wTrgHt = psTarget->wHeight;

	if (wSrcWd < 16) wSrcWd = 16;
	if (wTrgWd < 4) wTrgWd = 4;

	MP_DEBUG("ipu_scale %d %d %d %d", wSrcWd, wSrcHt, wTrgWd, wTrgHt);
        //jessamine - 20070519
	#define SCALE_MAX 2 // 4
	/*if ((wSrcWd <100) || (wSrcHt <100))
        SCALE_MAX = 2;
        else
            SCALE_MAX = 3;*/
        //jessamine - 20070614, for 32M buffer size
//	if ((wSrcWd >= (wTrgWd << SCALE_MAX)) || (wTrgWd >= (wSrcWd << SCALE_MAX)))
	if (wTrgWd >= (wSrcWd << SCALE_MAX))
	{
		MP_DEBUG("Scale too much, preScale one step");

		ST_IMAGEINFO tmp;

		tmp.wType = psSource->wType;
		if (wSrcWd >= (wTrgWd << SCALE_MAX)) // * 3 closer target size more
		{
			tmp.wWidth = (wSrcWd + wTrgWd * 3) >> 2;
			tmp.wHeight = (wSrcHt + wTrgHt * 3) >> 2;
		} else {
			tmp.wWidth = (wSrcWd * 3 + wTrgWd) >> 2;
			tmp.wHeight = (wSrcHt * 3 + wTrgHt) >> 2;
		}

		if (tmp.wWidth != 0 && tmp.wHeight != 0) {
			tmp.wWidth = ALIGN_16(tmp.wWidth);
			tmp.dwOffset = ALIGN_16(tmp.wWidth) << 1;
			tmp_buffer = (BYTE *)ext_mem_malloc((tmp.dwOffset * tmp.wHeight) + 256);
			if (tmp_buffer == NULL){
				MP_ALERT("alloc tmp_buffer fail");
				return FAIL;
			}
			tmp.dwPointer = (DWORD*)tmp_buffer;

			ret = ipu_scale(psSource, &tmp);
			if (ret == FAIL) {
				ret = FAIL;
				goto ipu_scaling_exit;
			}
			psSource = &tmp;
			wSrcWd = psSource->wWidth;
			wSrcHt = psSource->wHeight;
		}
	}


	wSrcPitch = psSource->dwOffset;
	wTrgPitch = psTarget->dwOffset;

	wSrcWd0 = wSrcWd;
	wTrgWd0 = wTrgWd;
	if ((wTrgWd & 0xf) == 2) {  // need check if do needed
		MP_DEBUG(".wTrgWd & 0xf == 2.");
		wTrgWd -= 2;
	}
	wSrcWd = ALIGN_16(wSrcWd);
	wTrgWd = ALIGN_CUT_2(wTrgWd);

	if (wSrcPitch >= (wSrcWd << 1))
		wSrcOffset = wSrcPitch - (wSrcWd << 1);
	else
	{
		wSrcWd = (wSrcPitch >> 1);
		wSrcOffset = 0;
	}
	if (wTrgPitch >= (wTrgWd << 1))
		wTrgOffset = wTrgPitch - (wTrgWd << 1);
	else
	{
		wTrgWd = (wTrgPitch >> 1);
		wTrgOffset = 0;
	}

	MP_DEBUG("wSrcWd %02d, wSrcOffset %d, wSrcPitch %d", wSrcWd, wSrcOffset, wSrcPitch);
	MP_DEBUG("wTrgWd %02d, wTrgOffset %d, wTrgPitch %d", wTrgWd, wTrgOffset, wTrgPitch);

	psScaParm = &sScaParm;
	if (GetScaParm(psScaParm, wSrcWd0, wSrcHt, wTrgWd0, wTrgHt) == FAIL) {
		ret = FAIL;
		goto ipu_scaling_exit;
	}
	//extern int test_val[];
	//psScaParm->wHSF = test_val[0];
	//psScaParm->wVSF = test_val[1];

	MP_DEBUG("psScaParm  %d %d %d %d %d %d", psScaParm->wHUp,
	    psScaParm->wHSF,
	    psScaParm->wHSubRatio,
	    psScaParm->wVUp,
	    psScaParm->wVSF,
	    psScaParm->wVSubRatio);


#if 0	// need check
	if ((wTrgWd << psScaParm->wHSubRatio) >= 1800 ) {//|| wSrcWd > 4096 || wSrcHt > 4096 || wTrgWd > 4096 || wTrgHt > 4096) {
		MP_ALERT("ipu out of range");

		ret = FAIL;
		goto ipu_scaling_exit;
	}
#endif
	ipu_reset();

	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	//source
	dmaR->StartA = (DWORD) psSource->dwPointer | 0xA0000000;
	if (psSource->wType == TYPE_420)
	{
		dmaR->EndA = (DWORD) wSrcOffset << 5;	// MCU row pitch = dwOffset * 2 * 16
		dmaR->LineCount = ((wSrcHt - 1) << 16) + (wSrcWd - 1);
	}
	else
	{
		dmaR->EndA = dmaR->StartA + (wSrcPitch * wSrcHt - (wSrcOffset >> 1)) - 1;
		dmaR->LineCount = (wSrcOffset << 16) + ((wSrcWd << 1) - 1);
	}

	//destination
	dmaW->StartA = (DWORD) psTarget->dwPointer | 0xA0000000;
	dmaW->EndA = dmaW->StartA + (wTrgPitch * wTrgHt - (wTrgOffset >> 1)) - 1;
	dmaW->LineCount = (wTrgOffset << 16) + ((wTrgWd << 1) - 1);


	ipu->IpIpr1 = 0;			// Read Start Position
	ipu->IpIpr2 = (((wSrcWd - 1) & 0xfff) << 16) + ((wSrcHt - 1) & 0xfff);	// Read End Position

	ipu->IpIpw11 = 0;
	ipu->IpIpw13 = 0;			// Write Start Position
	ipu->IpIpw14 = (((wTrgWd - 1) & 0xfff) << 16) + ((wTrgHt - 1) & 0xfff);	// Write End Position

	ipu->IpMnsc0 = (psScaParm->wHUp << 8) + psScaParm->wHSF;
	ipu->IpMnsc1 = (wTrgWd << psScaParm->wHSubRatio) - 1;	// BiLinear target line width
	ipu->IpMnsc2 = (psScaParm->wVUp << 8) + psScaParm->wVSF;



	if (psScaParm->wHUp == SCALING_UP)
		ipu->IpMnsc3 = ((wSrcWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);
	else
		ipu->IpMnsc3 = ((wTrgWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);

	ipu->IpMnsc4 = (psScaParm->wHSubRatio << 16) + psScaParm->wVSubRatio;

	// clear and setup interrupt
	ipu->IpIc = 0x0;
	ipu->IpIm = 0x0F;

	//enable transfer
	if (psSource->wType == TYPE_420)
		ipu->IpIpr0 = 0x00010100;	//Enable IPR0
	else
		ipu->IpIpr0 = 0x00010000;	//Enable IPR0
	ipu->IpIpw10 = 0x00000001;		//Enable IPW1

	clock->MdClken |= BIT6; 		//enable ipu scaler pixel clock

	dmaR->Control = 0x00000005;
	dmaW->Control = 0x00000005;

	ret = Ipu_WaitComplete(10);
	if (ret == TRUE) {
		clock->MdClken &= ~BIT6;	//disable ipu scaler pixel clock
	}
	else{
		MP_DEBUG("scaling FAIL");
		ret = FAIL;
		goto ipu_scaling_exit;
	}
ipu_scaling_exit:
	if (tmp_buffer) ext_mem_free(tmp_buffer);

	return ret;
}

#if 0
// ABEL 20070503 add for MJPEG
static int ipu_scale_not_wait(ST_IMAGEINFO * psSource, ST_IMAGEINFO * psTarget)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	int ret;
	BIU *biu;
	CLOCK *clock;
	WORD wSrcWd0, wTrgWd0;
	WORD wSrcWd, wTrgWd, wSrcHt, wTrgHt, wSrcPitch, wTrgPitch, tm;
	DWORD wSrcOffset, wTrgOffset;
	CHANNEL *dma;
	ST_SCA_PARM sScaParm;
	ST_SCA_PARM *psScaParm;

	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;

	wSrcWd = psSource->wWidth;
	wTrgWd = psTarget->wWidth;
	wSrcHt = psSource->wHeight;
	wTrgHt = psTarget->wHeight;

	if (wSrcWd < 16) wSrcWd = 16;
	if (wTrgWd < 4) wTrgWd = 4;

	wSrcPitch = psSource->dwOffset;
	wTrgPitch = psTarget->dwOffset;

	wSrcWd0 = wSrcWd;
	wTrgWd0 = wTrgWd;
	if ((wTrgWd & 0xf) == 2) {  // need check if do needed
		MP_DEBUG(".wTrgWd & 0xf == 2.");
		wTrgWd -= 2;
	}
	wSrcWd = ALIGN_16(wSrcWd);
	wTrgWd = ALIGN_CUT_2(wTrgWd);

	if (wSrcPitch >= (wSrcWd << 1))
		wSrcOffset = wSrcPitch - (wSrcWd << 1);
	else
	{
		wSrcWd = (wSrcPitch >> 1);
		wSrcOffset = 0;
	}
	if (wTrgPitch >= (wTrgWd << 1))
		wTrgOffset = wTrgPitch - (wTrgWd << 1);
	else
	{
		wTrgWd = (wTrgPitch >> 1);
		wTrgOffset = 0;
	}

	MP_DEBUG("wSrcWd %02d, wSrcOffset %d, wSrcPitch %d", wSrcWd, wSrcOffset, wSrcPitch);
	MP_DEBUG("wTrgWd %02d, wTrgOffset %d, wTrgPitch %d", wTrgWd, wTrgOffset, wTrgPitch);

	psScaParm = &sScaParm;
	if (GetScaParm(psScaParm, wSrcWd0, wSrcHt, wTrgWd0, wTrgHt) == FAIL) {
		return FAIL;
	}
	//extern int test_val[];
	//psScaParm->wHSF = test_val[0];
	//psScaParm->wVSF = test_val[1];

	MP_DEBUG("psScaParm  %d %d %d %d %d %d", psScaParm->wHUp,
	    psScaParm->wHSF,
	    psScaParm->wHSubRatio,
	    psScaParm->wVUp,
	    psScaParm->wVSF,
	    psScaParm->wVSubRatio);

	ipu_reset();

	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	//source
	dmaR->StartA = (DWORD) psSource->dwPointer | 0xA0000000;
	if (psSource->wType == TYPE_420)
	{
		dmaR->EndA = (DWORD) wSrcOffset << 5;	// MCU row pitch = dwOffset * 2 * 16
		dmaR->LineCount = ((wSrcHt - 1) << 16) + (wSrcWd - 1);
	}
	else
	{
		dmaR->EndA = dmaR->StartA + (wSrcPitch * wSrcHt - (wSrcOffset >> 1)) - 1;
		dmaR->LineCount = (wSrcOffset << 16) + ((wSrcWd << 1) - 1);
	}

	//destination
	dmaW->StartA = (DWORD) psTarget->dwPointer | 0xA0000000;
	dmaW->EndA = dmaW->StartA + (wTrgPitch * wTrgHt - (wTrgOffset >> 1)) - 1;
	dmaW->LineCount = (wTrgOffset << 16) + ((wTrgWd << 1) - 1);


	ipu->IpIpr1 = 0;			// Read Start Position
	ipu->IpIpr2 = (((wSrcWd - 1) & 0xfff) << 16) + ((wSrcHt - 1) & 0xfff);	// Read End Position

	ipu->IpIpw11 = 0;
	ipu->IpIpw13 = 0;			// Write Start Position
	ipu->IpIpw14 = (((wTrgWd - 1) & 0xfff) << 16) + ((wTrgHt - 1) & 0xfff);	// Write End Position

	ipu->IpMnsc0 = (psScaParm->wHUp << 8) + psScaParm->wHSF;
	ipu->IpMnsc1 = (wTrgWd << psScaParm->wHSubRatio) - 1;	// BiLinear target line width
	ipu->IpMnsc2 = (psScaParm->wVUp << 8) + psScaParm->wVSF;



	if (psScaParm->wHUp == SCALING_UP)
		ipu->IpMnsc3 = ((wSrcWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);
	else
		ipu->IpMnsc3 = ((wTrgWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);

	ipu->IpMnsc4 = (psScaParm->wHSubRatio << 16) + psScaParm->wVSubRatio;

	// clear and setup interrupt
	ipu->IpIc = 0x0;
	ipu->IpIm = 0x0F;

	//enable transfer
	if (psSource->wType == TYPE_420)
		ipu->IpIpr0 = 0x00010100;	//Enable IPR0
	else
		ipu->IpIpr0 = 0x00010000;	//Enable IPR0
	ipu->IpIpw10 = 0x00000001;		//Enable IPW1

	clock->MdClken |= BIT6; 		//enable ipu scaler pixel clock

	dmaR->Control = 0x00000005;
	dmaW->Control = 0x00000005;

	return PASS;
}
#endif
//extern BYTE CduWaitReady;
static int image_one_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
			  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	ST_IMAGEINFO s, t;
	DWORD *srcPtr, *trgPtr;
	WORD sXdiff, sYdiff, tXdiff, tYdiff;

	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2)) {
		MP_ALERT("-E- scale region error");
		return PASS;
	}

	sXdiff = (sx2 - sx1);
	sYdiff = (sy2 - sy1);
	tXdiff = (tx2 - tx1);
	tYdiff = (ty2 - ty1);

	if (sXdiff < 16) sXdiff = 16;
	if (sYdiff < 8) sYdiff = 8;
	if (tXdiff < 8) tXdiff = 8;
	if (tYdiff < 8) tYdiff = 8;

	sx1 &= 0xfffe;
	tx1 &= 0xfffe;

	srcPtr = srcWin->pdwStart + (((sy1 * srcWin->dwOffset) + sx1 * 2) >> 2);	//go to the start position
	trgPtr = trgWin->pdwStart + (((ty1 * trgWin->dwOffset) + tx1 * 2) >> 2);	//go to the start position
	Ipu_InitImageInfo(&s, srcPtr, sXdiff, sYdiff, TYPE_422);
	Ipu_InitImageInfo(&t, trgPtr, tXdiff, tYdiff, TYPE_422);
	s.dwOffset = srcWin->dwOffset;
	t.dwOffset = trgWin->dwOffset;

//	if(CduWaitReady)
		return ipu_scale(&s, &t);
//	else
//		return ipu_scale_not_wait(&s, &t);


}


#define SCALER_MAX_SIZE 2048 // it's not fixed, can be tuned
#define SCALER_MAX_MASK 0xFFF

int image_slice_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
			  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	int src_w, src_h;
	int i, j, row, col;
	int src_x, dst_x, src_dx, dst_dx, dst_w;
	int src_y, dst_y, src_dy, dst_dy, dst_h;
	DWORD ratio_x, ratio_y;

	src_w = (sx2 - sx1);
	src_h = (sy2 - sy1);

	col = src_w / SCALER_MAX_SIZE;  //  w / 4096
	row = src_h / SCALER_MAX_SIZE;  //  h / 4096

	if (src_w && SCALER_MAX_MASK) col++;
	if (src_h && SCALER_MAX_MASK) row++;

	if (col <= 1)
	{
		dst_dx = tx2 - tx1;
		src_dx = src_w;
	}
	else
	{
		ratio_x = FIX_POINT(tx2 - tx1) / (sx2 - sx1);
		dst_dx = ALIGN_CUT_4((tx2 - tx1) / col);
		src_dx = FIX_POINT(dst_dx) / ratio_x;
	}

	if (row <= 1)
	{
		dst_dy = ty2 - ty1;
		src_dy = src_h;
	}
	else
	{
		ratio_y = FIX_POINT(ty2 - ty1) / (sy2 - sy1);
		dst_dy = ALIGN_CUT_4((ty2 - ty1) / row);
		src_dy = FIX_POINT(dst_dy) / ratio_y;
	}

	src_y = sy1;
	dst_y = ty1;

	src_h = src_dy;
	dst_h = dst_dy;
	for (j = 0; j < row && src_y < sy2; j++)
	{
		if (src_y + src_dy > sy2) src_h = sy2 - src_y;
		if (dst_y + dst_dy > ty2) dst_h = ty2 - dst_y;

		src_x = sx1;
		dst_x = tx1;
		src_w = src_dx;
		dst_w = dst_dx;
		for (i = 0; i < col && src_x < sx2; i++)
		{
			if (src_x + src_dx > sx2) src_w = sx2 - src_x;
			if (dst_x + dst_dx > tx2) dst_w = tx2 - dst_x;

			if (FAIL == image_one_scale(srcWin, trgWin, src_x, src_y, src_x + src_w, src_y + src_h,
									    dst_x, dst_y, dst_x + dst_w, dst_y + dst_h))
				return FAIL;
			src_x += src_dx;
			dst_x += dst_dx;
		}

		src_y += src_dy;
		dst_y += dst_dy;
	}

	return PASS;
}


int image_scale(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
			  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	ST_IMAGEINFO s, t;
	DWORD *srcPtr, *trgPtr;

	if (sx2 > srcWin->wWidth)  sx2 = srcWin->wWidth ;
	if (sy2 > srcWin->wHeight) sy2 = srcWin->wHeight;
	if (tx2 > trgWin->wWidth)  tx2 = trgWin->wWidth ;
	if (ty2 > trgWin->wHeight) ty2 = trgWin->wHeight;

	if ((sx1 >= sx2) || (sy1 >= sy2) || (tx1 >= tx2) || (ty1 >= ty2)) {
		MP_ALERT("-E- scale region error %d->%d %d->%d %d->%d %d->%d", sx1, sx2, sy1, sy2, tx1, tx2, ty1, ty2);
		return FAIL;
	}

	if (srcWin->dwOffset == 0) {
		MP_ALERT("-E- srcWin->dwOffset == 0");
		srcWin->dwOffset = ALIGN_16(srcWin->wWidth) << 1;
	}
	if (trgWin->dwOffset == 0) {
		MP_ALERT("-E- trgWin->dwOffset == 0");
		trgWin->dwOffset = trgWin->wWidth << 1;
	}
	MP_DEBUG("srcWin->dwOffset = %d, trgWin->dwOffset = %d", srcWin->dwOffset, trgWin->dwOffset);

	if ((sx2 - sx1) < SCALER_MAX_SIZE && (sy2 - sy1) < SCALER_MAX_SIZE){
		MP_DEBUG("scale once");
		return image_one_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	}
	else{
		MP_DEBUG("scale slice");
		return image_slice_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
  }
}





int ipu_scale_subsample(ST_IMAGEINFO * psSource, ST_IMAGEINFO * psTarget)
{
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	int ret;
	BIU *biu;
	CLOCK *clock;
	WORD wSrcWd0, wTrgWd0;
	WORD wSrcWd, wTrgWd, wSrcHt, wTrgHt, wSrcPitch, wTrgPitch, tm;
	DWORD wSrcOffset, wTrgOffset;
	CHANNEL *dma;
	ST_SCA_PARM sScaParm;
	ST_SCA_PARM *psScaParm;
	BYTE *tmp_buffer = NULL; //, SCALE_MAX;

	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;

	wSrcWd = psSource->wWidth;
	wTrgWd = psTarget->wWidth;
	wSrcHt = psSource->wHeight;
	wTrgHt = psTarget->wHeight;

	if (wSrcWd < 16) wSrcWd = 16;
	if (wTrgWd < 4) wTrgWd = 4;

	MP_DEBUG("ipu_scale %d %d %d %d", wSrcWd, wSrcHt, wTrgWd, wTrgHt);
        //jessamine - 20070519
	#define SCALE_MAX 2 // 4
	/*if ((wSrcWd <100) || (wSrcHt <100))
        SCALE_MAX = 2;
        else
            SCALE_MAX = 3;*/
        //jessamine - 20070614, for 32M buffer size
//	if ((wSrcWd >= (wTrgWd << SCALE_MAX)) || (wTrgWd >= (wSrcWd << SCALE_MAX)))
	if (wTrgWd >= (wSrcWd << SCALE_MAX))
	{
		MP_DEBUG("Scale too much, preScale one step");

		ST_IMAGEINFO tmp;

		tmp.wType = psSource->wType;
		if (wSrcWd >= (wTrgWd << SCALE_MAX)) // * 3 closer target size more
		{
			tmp.wWidth = (wSrcWd + wTrgWd * 3) >> 2;
			tmp.wHeight = (wSrcHt + wTrgHt * 3) >> 2;
		} else {
			tmp.wWidth = (wSrcWd * 3 + wTrgWd) >> 2;
			tmp.wHeight = (wSrcHt * 3 + wTrgHt) >> 2;
		}

		if (tmp.wWidth != 0 && tmp.wHeight != 0) {
			tmp.wWidth = ALIGN_16(tmp.wWidth);
			tmp.dwOffset = ALIGN_16(tmp.wWidth) << 1;
			tmp_buffer = (BYTE *)ext_mem_malloc((tmp.dwOffset * tmp.wHeight) + 256);
			if (tmp_buffer == NULL){
				MP_ALERT("alloc tmp_buffer fail");
				return FAIL;
			}
			tmp.dwPointer = (DWORD*)tmp_buffer;

			ret = ipu_scale(psSource, &tmp);
			if (ret == FAIL) {
				ret = FAIL;
				goto ipu_scaling_exit;
			}
			psSource = &tmp;
			wSrcWd = psSource->wWidth;
			wSrcHt = psSource->wHeight;
		}
	}


	wSrcPitch = psSource->dwOffset;
	wTrgPitch = psTarget->dwOffset;

	wSrcWd0 = wSrcWd;
	wTrgWd0 = wTrgWd;
	if ((wTrgWd & 0xf) == 2) {  // need check if do needed
		MP_DEBUG(".wTrgWd & 0xf == 2.");
		wTrgWd -= 2;
	}
	wSrcWd = ALIGN_16(wSrcWd);
	wTrgWd = ALIGN_CUT_2(wTrgWd);

	if (wSrcPitch >= (wSrcWd << 1))
		wSrcOffset = wSrcPitch - (wSrcWd << 1);
	else
	{
		wSrcWd = (wSrcPitch >> 1);
		wSrcOffset = 0;
	}
	if (wTrgPitch >= (wTrgWd << 1))
		wTrgOffset = wTrgPitch - (wTrgWd << 1);
	else
	{
		wTrgWd = (wTrgPitch >> 1);
		wTrgOffset = 0;
	}

	MP_DEBUG("wSrcWd %02d, wSrcOffset %d, wSrcPitch %d", wSrcWd, wSrcOffset, wSrcPitch);
	MP_DEBUG("wTrgWd %02d, wTrgOffset %d, wTrgPitch %d", wTrgWd, wTrgOffset, wTrgPitch);

	psScaParm = &sScaParm;
	if (GetScaParm(psScaParm, wSrcWd0, wSrcHt, wTrgWd0, wTrgHt) == FAIL) {
		ret = FAIL;
		goto ipu_scaling_exit;
	}
	//extern int test_val[];
	//psScaParm->wHSF = test_val[0];
	//psScaParm->wVSF = test_val[1];

	MP_DEBUG("psScaParm  %d %d %d %d %d %d", psScaParm->wHUp,
	    psScaParm->wHSF,
	    psScaParm->wHSubRatio,
	    psScaParm->wVUp,
	    psScaParm->wVSF,
	    psScaParm->wVSubRatio);


#if 0	// need check
	if ((wTrgWd << psScaParm->wHSubRatio) >= 1800 ) {//|| wSrcWd > 4096 || wSrcHt > 4096 || wTrgWd > 4096 || wTrgHt > 4096) {
		MP_ALERT("ipu out of range");

		ret = FAIL;
		goto ipu_scaling_exit;
	}
#endif
	ipu_reset();

	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	//source
	dmaR->StartA = (DWORD) psSource->dwPointer | 0xA0000000;
	if (psSource->wType == TYPE_420)
	{
		dmaR->EndA = (DWORD) wSrcOffset << 5;	// MCU row pitch = dwOffset * 2 * 16
		dmaR->LineCount = ((wSrcHt - 1) << 16) + (wSrcWd - 1);
	}
	else
	{
		dmaR->EndA = dmaR->StartA + (wSrcPitch * wSrcHt - (wSrcOffset >> 1)) - 1;
		dmaR->LineCount = (wSrcOffset << 16) + ((wSrcWd << 1) - 1);
	}

	//destination
	dmaW->StartA = (DWORD) psTarget->dwPointer | 0xA0000000;
	dmaW->EndA = dmaW->StartA + (wTrgPitch * wTrgHt - (wTrgOffset >> 1)) - 1;
	dmaW->LineCount = (wTrgOffset << 16) + ((wTrgWd << 1) - 1);


	ipu->IpIpr1 = 0;			// Read Start Position
	ipu->IpIpr2 = (((wSrcWd - 1) & 0xfff) << 16) + ((wSrcHt - 1) & 0xfff);	// Read End Position

	ipu->IpIpw11 = 0;
	ipu->IpIpw13 = 0;			// Write Start Position
	ipu->IpIpw14 = (((wTrgWd - 1) & 0xfff) << 16) + ((wTrgHt - 1) & 0xfff);	// Write End Position

	ipu->IpMnsc0 = (psScaParm->wHUp << 8) + psScaParm->wHSF;
	ipu->IpMnsc1 = (wTrgWd << psScaParm->wHSubRatio) - 1;	// BiLinear target line width
	ipu->IpMnsc2 = (psScaParm->wVUp << 8) + psScaParm->wVSF;



	if (psScaParm->wHUp == SCALING_UP)
		ipu->IpMnsc3 = ((wSrcWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);
	else
		ipu->IpMnsc3 = ((wTrgWd - 1) << 16) + ((wTrgHt << psScaParm->wVSubRatio) - 1);

	ipu->IpMnsc4 = (psScaParm->wHSubRatio << 16) + psScaParm->wVSubRatio;

	// clear and setup interrupt
	ipu->IpIc = 0x0;
	ipu->IpIm = 0x0F;

	//enable transfer
	if (psSource->wType == TYPE_420)
		ipu->IpIpr0 = 0x00010100;	//Enable IPR0
	else
		ipu->IpIpr0 = 0x00010000;	//Enable IPR0
	ipu->IpIpw10 = 0x00000001;		//Enable IPW1

	clock->MdClken |= BIT6; 		//enable ipu scaler pixel clock

	dmaR->Control = 0x00000005;
	dmaW->Control = 0x00000005;

	ret = Ipu_WaitComplete(10);
	if (ret == TRUE) {
		clock->MdClken &= ~BIT6;	//disable ipu scaler pixel clock
	}
	else{
		MP_DEBUG("scaling FAIL");
		ret = FAIL;
		goto ipu_scaling_exit;
	}
ipu_scaling_exit:
	if (tmp_buffer) ext_mem_free(tmp_buffer);

	return ret;
}


// for 420 type, we only support length of width are 176, 256, 320, 352, 512,
// 640, 1024, 2048 and 4096 pixels
int inline ImgChkMCUPitch(ST_IMAGEINFO * psSource)
{
	if (psSource->wType == TYPE_422)
		return TRUE;

	switch(psSource->dwOffset) {
		case 32:
		case 64:
		case 128:
		case 160:
		case 176:
		case 256:
		case 320:
		case 352:
		case 512:
		case 640:
		case 1024:
		case 2048:
		case 4096:
			return TRUE;
	}

	return FALSE;

}


int ImgIpuScaling(ST_SCA_PARM * psScaParm, register ST_IMAGEINFO * psSource, register ST_IMAGEINFO * psTarget)
{
  
	IPU *ipu;
	CHANNEL *dmaR, *dmaW;
	int ret =0 ;
	DWORD value;
	BIU *biu;
	CLOCK *clock;
	CHANNEL *dma;
	
	dma = (CHANNEL *) (DMA_IDU_BASE);
	clock = (CLOCK *) CLOCK_BASE;

	if (ImgChkMCUPitch(psSource) == FALSE)
		return FALSE;

	biu = (BIU *) BIU_BASE;
	ipu = (IPU *) IPU_BASE;
	dmaR = (CHANNEL *) DMA_SCR_BASE;
	dmaW = (CHANNEL *) DMA_SCW_BASE;

	// asynchronous IPU reset
	biu->BiuArst &= 0xfffffffb;
	biu->BiuArst |= 0x00000004;

	// disable 
	dmaR->Control = 0x0;
	dmaW->Control = 0x0;
	ipu->IpIpw10 = 0x0;
	ipu->IpIpr0 = 0x0;

	value = (DWORD) psSource->dwOffset << 5;	// MCU row pitch = dwOffset * 2 * 16

	//source
	dmaR->StartA = (DWORD) psSource->dwPointer;
	if (psSource->wType == TYPE_420)
	{
		dmaR->EndA = value;
		dmaR->LineCount = ((psSource->wHeight - 1) << 16) + (psSource->wWidth - 1);
	}
	else
	{
		dmaR->EndA =
			dmaR->StartA + (((psSource->wWidth << 1) + psSource->dwOffset) * psSource->wHeight -
							psSource->dwOffset) - 1;
		dmaR->LineCount = (psSource->dwOffset << 16) + ((psSource->wWidth << 1) - 1);
	}

	//destination
	dmaW->StartA = (DWORD) psTarget->dwPointer;
	dmaW->EndA =
		dmaW->StartA + (((psTarget->wWidth << 1) + psTarget->dwOffset) * psTarget->wHeight - psTarget->dwOffset) - 1;
	dmaW->LineCount = (psTarget->dwOffset << 16) + ((psTarget->wWidth << 1) - 1);

	// clear and setup interrupt
	ipu->IpIc = 0x0;
	ipu->IpIm = 0x0F;

	//Initial MNSC
	ipu->IpMnsc0 = 0x00;
	ipu->IpMnsc1 = 0x00;
	ipu->IpMnsc2 = 0x00;
	ipu->IpMnsc3 = 0x00;
	ipu->IpMnsc4 = 0x00;

	ipu->IpIpr1 = 0;			// Read Start Position
	ipu->IpIpr2 = (((psSource->wWidth - 1) & 0xfff) << 16) + ((psSource->wHeight - 1) & 0xfff);	// Read End Position

	ipu->IpIpw11 = 0;
	ipu->IpIpw13 = 0;			// Write Start Position
	ipu->IpIpw14 = (((psTarget->wWidth - 1) & 0xfff) << 16) + ((psTarget->wHeight - 1) & 0xfff);	// Write End Position 

	ipu->IpMnsc0 = (psScaParm->wHUp << 8) + psScaParm->wHSF;
	ipu->IpMnsc1 = (psTarget->wWidth << psScaParm->wHSubRatio) - 1;	// BiLinear target line width
	ipu->IpMnsc2 = (psScaParm->wVUp << 8) + psScaParm->wVSF;


	if (psScaParm->wHUp == SCALING_UP)
		ipu->IpMnsc3 =
			((psSource->wWidth - 1) << 16) + ((psTarget->wHeight << psScaParm->wVSubRatio) - 1);
	else
		ipu->IpMnsc3 =
			((psTarget->wWidth - 1) << 16) + ((psTarget->wHeight << psScaParm->wVSubRatio) - 1);

	ipu->IpMnsc4 = (psScaParm->wHSubRatio << 16) + psScaParm->wVSubRatio;


	dmaW->Control = 0x00000000;
	//enable transfer
	if (psSource->wType == TYPE_420)
		ipu->IpIpr0 = 0x00010100;	//Enable IPR0
	else
		ipu->IpIpr0 = 0x00010000;	//Enable IPR0
	ipu->IpIpw10 = 0x00000001;	//Enable IPW1

	clock->MdClken |= BIT6;		//enable ipu scaler pixel clock

	//while ((boMPABusy == TRUE))    
//    Idu_WaitBufferEnd();//Mason 20070829
//    TaskYield();
//	boScalerBusy = TRUE;
	dmaW->Control = 0x00000005;
	dmaR->Control = 0x00000005;

	ret = Ipu_WaitComplete(5);
	clock->MdClken &= ~BIT6;	//disable ipu scaler pixel clock
//	boScalerBusy = FALSE;

	return ret;
}

#endif	//#if ((CHIP_VER & 0xffff0000) != CHIP_VER_650)

