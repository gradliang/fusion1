

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

#include "xpg.h"

// Image resize
// IPU module



#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)
// return the scaling factor of the input 
// the scale ratio is  1/2 ~ 256
WORD GetSF(DWORD dwSrc, DWORD dwTrg)
{
	WORD wSf;

	if (dwTrg <= 1)
		dwTrg = 2;				/* make sure there's no division by 0 */

	if (dwSrc == dwTrg)
		wSf = 0;
	else
		wSf = ((dwSrc - 1) * 256 + 1) / dwTrg;

	wSf &= 0xff;
	return wSf;
}


// Calculate post image sub-sample ratio
// x1 ~ x1/16
BYTE GetSubSampleRatio(register WORD wSrc, register WORD wTrg)
{
	register BYTE i;

	for (i = 4; i > 0; i--)
	{
		if ((wSrc >> i) >= wTrg)
			return i;
	}

	return 0;
}


BYTE ImgGetResizeParam(register ST_IMAGEINFO * psSourceImage, register ST_IMAGEINFO * psTargetImage, ST_SCA_PARM *psScaParm)
{
	BYTE bVCount, bHCount;
	BYTE i, j, divisor;
	WORD k;
	register DWORD dwSrcWidth, dwTrgWidth, dwSrcHeight, dwTrgHeight, tmpSDiff, tmpTDiff, tmp, tmp1;
	DWORD src2StartPosition, srcImgWidth;
	WORD wDisplayWidth = g_psSystemConfig->sScreenSetting.wInnerWidth;

	//ST_IMAGEINFO sSourceImageSlice, sTargetImageSlice;

	psScaParm->wHSubRatio = GetSubSampleRatio(psSourceImage->wWidth, psTargetImage->wWidth);
	psScaParm->wVSubRatio = GetSubSampleRatio(psSourceImage->wHeight, psTargetImage->wHeight);
	psScaParm->wVSF =
		GetSF(psSourceImage->wHeight, ((DWORD) psTargetImage->wHeight) << psScaParm->wVSubRatio);
	psScaParm->wHSF =
		GetSF(psSourceImage->wWidth, ((DWORD) psTargetImage->wWidth) << psScaParm->wHSubRatio);

	srcImgWidth = psSourceImage->wWidth;

	psSourceImage->wWidth = (psSourceImage->wWidth + 15) & 0xfff0;
	src2StartPosition = 0;

	if (psTargetImage->wWidth <= 1280)
		tmp1 = 160;
	else if (psTargetImage->wWidth <= 1920)
		tmp1 = 240;

	if (psSourceImage->wWidth > psTargetImage->wWidth)
	{							// image scaling down
		psScaParm->wHUp = SCALING_DOWN;

		if (psSourceImage->wWidth <= 1024)
		{
			dwSrcWidth = psSourceImage->wWidth;
			dwTrgWidth = psTargetImage->wWidth;
			bHCount = 0;
		}
		else
		{
			DWORD srcBudget;

			if (psTargetImage->wWidth <= 800)
			{
				divisor = 2;
				tmp1 = psSourceImage->wWidth / divisor;
				srcBudget = 31;
				if (wDisplayWidth == 720 || wDisplayWidth == 1280)
				{
					if (psScaParm->wHSF < 247)
						srcBudget = 15;
				}
				if (wDisplayWidth == 1920)
					srcBudget = 15;

				dwSrcWidth = (tmp1 + srcBudget) & 0xfff0;
			}
			else
			{
				divisor = 16;
				tmp1 = psSourceImage->wWidth / divisor;
				dwSrcWidth = (tmp1 + 15) & 0xfff0;
			}

			dwTrgWidth = ((tmp1 * psTargetImage->wWidth) / psSourceImage->wWidth) & 0xfffc;
			if (psTargetImage->wWidth <= 360)
				while (((dwTrgWidth + 2) * divisor) < psTargetImage->wWidth)
					dwTrgWidth += 2;	// for 480i thumbnail, preventing short of data

			bHCount = (psSourceImage->wWidth - 1) / tmp1;
			src2StartPosition = tmp1;

			if (psSourceImage->wWidth == 1536 && psTargetImage->wWidth == 1440)
				src2StartPosition -= 4;
		}

	}
	else if (psSourceImage->wWidth < psTargetImage->wWidth)
	{							// image scaling up
		psScaParm->wHUp = SCALING_UP;
		if (srcImgWidth <= 1024)
		{
			dwSrcWidth = psSourceImage->wWidth;
			dwTrgWidth = psTargetImage->wWidth;
			bHCount = 0;
		}
		else
		{
			bHCount = (psTargetImage->wWidth - 1) / tmp1;

			tmp = (tmp1 * srcImgWidth) / psTargetImage->wWidth;
			dwSrcWidth = (tmp + 16) & 0xfff0;	// need to check again

			dwTrgWidth = tmp1;
			src2StartPosition = tmp;
		}
	}
	else
	{
		psScaParm->wHUp = NON_SCALING;
		dwSrcWidth = 1024;
		dwTrgWidth = 1024;
		bHCount = (psTargetImage->wWidth - 1) >> 10;	// divide by 1024, total horizontal sectors 
	}
	if (((dwTrgWidth * 2) == dwSrcWidth) || ((dwTrgWidth * 4) == dwSrcWidth)
		|| ((dwTrgWidth * 8) == dwSrcWidth))
		psScaParm->wHUp = NON_SCALING;


	if (psSourceImage->wHeight > psTargetImage->wHeight)
	{							// image scaling down
		psScaParm->wVUp = SCALING_DOWN;
		dwSrcHeight = 2048 + psScaParm->wVSF * 8;
		dwTrgHeight = 2048 >> psScaParm->wVSubRatio;
		bVCount = ((psTargetImage->wHeight << psScaParm->wVSubRatio) - 1) >> 11;	// divide by 2048, total vertical sectors 
	}
	else if (psSourceImage->wHeight < psTargetImage->wHeight)
	{
		psScaParm->wVUp = SCALING_UP;	// image scaling up 
		dwSrcHeight = psScaParm->wVSF * 8;	// SF uses 2048 as base 
		dwTrgHeight = 2048;
		bVCount = (psTargetImage->wHeight - 1) >> 11;	// divide by 2048, total vertical sectors 
	}
	else
	{
		psScaParm->wVUp = NON_SCALING;	// image none scale 
		dwSrcHeight = 2048;
		dwTrgHeight = 2048;
		bVCount = (psTargetImage->wHeight - 1) >> 11;	// divide by 2048, total vertical sectors 
	}
	if (((dwTrgHeight * 2) == dwSrcHeight) || ((dwTrgHeight * 4) == dwSrcHeight)
		|| ((dwTrgHeight * 8) == dwSrcHeight))
		psScaParm->wVUp = NON_SCALING;



	return PASS;

}

#endif	// #if ((CHIP_VER & 0xffff0000) == CHIP_VER_615)

void FitToOriginRatio(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin)
{
	DWORD Ratio, RatioX, RatioY;
    WORD wSrcWidth, wSrcHeight, wSrcSX, wSrcSY;
    WORD wTrgWidth, wTrgHeight, wTrgSX, wTrgSY;

    struct ST_SCREEN_TAG *pstScreen = &g_psSystemConfig->sScreenSetting;

    if((pSrcWin->wWidth <= pstScreen->wInnerWidth) && (pSrcWin->wHeight <= pstScreen->wInnerHeight))
    {
		return;
    }
	wSrcWidth  = pSrcWin->wWidth;
    wSrcHeight = pSrcWin->wHeight;
    wTrgWidth  = pTrgWin->wWidth;
    wTrgHeight = pTrgWin->wHeight;

	RatioX = FIX_POINT_D(wSrcWidth) / pTrgWin->wWidth;
    RatioY = FIX_POINT_D(wSrcHeight) / pTrgWin->wHeight;
	MP_DEBUG("ImageDraw_FitToOrigin RatioX=%d RatioY=%d", RatioX, RatioY);

	if (RatioY > RatioX)
    {
        wTrgHeight = pTrgWin->wHeight;
        wTrgWidth = FIX_POINT_D(wSrcWidth) / RatioY;
    }
    else
    {
        wTrgWidth = pTrgWin->wWidth;
        wTrgHeight = FIX_POINT_D(wSrcHeight) /  RatioX;
    }
	MP_DEBUG("First round: Target width=%d Target height=%d", wTrgWidth, wTrgHeight);

    if ((pTrgWin->wWidth == 720) && (pTrgWin->wHeight == 480))
        wTrgWidth = wTrgWidth * 7 / 8; //for org display patch value

    wTrgWidth = ALIGN_16(wTrgWidth);
    if (wTrgWidth > pTrgWin->wWidth) wTrgWidth = pTrgWin->wWidth;
    if (wTrgHeight > pTrgWin->wHeight) wTrgHeight = pTrgWin->wHeight;

	pTrgWin->wWidth = wTrgWidth;
	pTrgWin->wHeight = wTrgHeight;
}

void ImageDraw_FitToOrigin(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin)
{
    DWORD Ratio, RatioX, RatioY;
    WORD wSrcWidth, wSrcHeight, wSrcSX, wSrcSY;
    WORD wTrgWidth, wTrgHeight, wTrgSX, wTrgSY;

    wSrcWidth  = pSrcWin->wWidth;
    wSrcHeight = pSrcWin->wHeight;
    wTrgWidth  = pTrgWin->wWidth;
    wTrgHeight = pTrgWin->wHeight;

    MP_DEBUG("ImageDraw_FitToOrigin(): %dx%d -> %dx%d; (pTrgWin->wX, pTrgWin->wY)=(%d,%d)", wSrcWidth, wSrcHeight, wTrgWidth, wTrgHeight, pTrgWin->wX, pTrgWin->wY);

    if (pTrgWin->wX > 0 || pTrgWin->wY > 0)
    {
        pTrgWin->wX = pTrgWin->wY = 0; //force to reset start pixel to display in this window
        MP_DEBUG("ImageDraw_FitToOrigin(): force to reset start pixel to display in this window");
    }

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    RatioX = FIX_POINT_D(wSrcWidth) / pTrgWin->wWidth;
    RatioY = FIX_POINT_D(wSrcHeight) / pTrgWin->wHeight;
#else
    RatioX = FIX_POINT(wSrcWidth) / pTrgWin->wWidth;
    RatioY = FIX_POINT(wSrcHeight) / pTrgWin->wHeight;
#endif
    MP_DEBUG("ImageDraw_FitToOrigin RatioX=%d RatioY=%d", RatioX, RatioY);

    Ratio = (RatioX > RatioY) ? RatioX : RatioY;

    if (RatioY > RatioX)
    {
        wTrgHeight = pTrgWin->wHeight;
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
        wTrgWidth = FIX_POINT_D(wSrcWidth) / RatioY;
#else
        wTrgWidth = FIX_POINT(wSrcWidth) / RatioY;
#endif
    }
    else
    {
        wTrgWidth = pTrgWin->wWidth;
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
        wTrgHeight = FIX_POINT_D(wSrcHeight) /  RatioX;
#else
        wTrgHeight = FIX_POINT(wSrcHeight) /  RatioX;
#endif
    }

    MP_DEBUG("First round: Target width=%d Target height=%d", wTrgWidth, wTrgHeight);

    if ((pTrgWin->wWidth == 720) && (pTrgWin->wHeight == 480))
        wTrgWidth = wTrgWidth * 7 / 8; //for org display patch value

    wTrgWidth = ALIGN_16(wTrgWidth);
    if (wTrgWidth > pTrgWin->wWidth) wTrgWidth = pTrgWin->wWidth;
    if (wTrgHeight > pTrgWin->wHeight) wTrgHeight = pTrgWin->wHeight;

    wTrgSX = pTrgWin->wX + ((pTrgWin->wWidth - wTrgWidth) >> 1);
    wTrgSY = pTrgWin->wY + ((pTrgWin->wHeight - wTrgHeight) >> 1);

    wTrgSX = ALIGN_CUT_4(wTrgSX);
    MP_DEBUG("wTrgHeight=%d wTrgWidth=%d", wTrgHeight, wTrgWidth);
    /* for face detection */
	  pTrgWin->wClipLeft = wTrgSX;
	  pTrgWin->wClipRight = wTrgSX + wTrgWidth;
	  pTrgWin->wClipTop = wTrgSY;
	  pTrgWin->wClipBottom = wTrgSY + wTrgHeight;

#if XPG_USE_YUV444
    //mpDebugPrint("ImageDraw_FitToOrigin(), g_sDecodeWin.wType = %d", ImageGetDecodeWinType());
    if(ImageGetDecodeWinType() == 444)
    {
        Ipu_ImageScaling(pSrcWin, pTrgWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight,wTrgSX, wTrgSY,
            wTrgSX+wTrgWidth, wTrgSY+wTrgHeight, 0);
    }
    else 
    {       // Convert YUV422 to YUV444
            ST_IMGWIN * pWin = Idu_GetNextWin();
            STXPGROLE  stRole;
            STXPGROLE * pstRole = &stRole;

            pstRole->m_wWidth = pSrcWin->wWidth;
            pstRole->m_wHeight = pWin->wHeight;
            pstRole->m_wRawWidth = pSrcWin->wWidth;
            xpgRoleDraw(pstRole, pWin->pdwStart, pSrcWin->pdwStart, 0, 0, pWin->wWidth, pWin->wHeight);
    }
#else   

    Ipu_ImageScaling(pSrcWin, pTrgWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight,wTrgSX, wTrgSY,
    wTrgSX+wTrgWidth, wTrgSY+wTrgHeight, 0);
#endif    
}




BYTE Extract_Ipu_ImageScaling(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin,DWORD *dwWinSrcTempStart,WORD wNewImageWidth, WORD wNewImageHeight)
{
	ST_IMGWIN sWinSource;
	ST_IMGWIN sWinSourceTemp;
	ST_IMGWIN sWinSourceFinal;

	int i=0;

	sWinSource.pdwStart= srcWin->pdwStart;
	sWinSource.dwOffset= srcWin->dwOffset;
	sWinSource.wWidth  = srcWin->wWidth;
	sWinSource.wHeight = srcWin->wHeight;
	sWinSource.wType   = srcWin->wType;


	for(i=0;i<1;i++)
	{

		if( ((sWinSource.wWidth/wNewImageWidth) > 15) || ((sWinSource.wHeight/wNewImageHeight) > 15) )
   		{
    		MP_DEBUG("Need to sclae one more");
    		sWinSourceTemp.wWidth   = ALIGN_2((sWinSource.wWidth / 15)+1); 
    		sWinSourceTemp.wHeight  = ALIGN_2((sWinSource.wHeight/15)+1);
    		sWinSourceTemp.wType    = sWinSource.wType;
    		if(sWinSourceTemp.wType == 444)
    		{
    	 		sWinSourceTemp.dwOffset = sWinSourceTemp.wWidth*3;
    		}
    		else
    		{
     			sWinSourceTemp.dwOffset = sWinSourceTemp.wWidth*2;
    		}

			if(i%2 == 0)
			{	
    			sWinSourceTemp.pdwStart = dwWinSrcTempStart ;
			}
			else
			{
				sWinSourceTemp.pdwStart = srcWin->pdwStart;
			}

	
   	 		MP_DEBUG2("### bef %d sWinSourceTemp.wWidth =%d",i,sWinSourceTemp.wWidth);
    		MP_DEBUG2("### bef %d sWinSourceTemp.wHeight =%d",i,sWinSourceTemp.wHeight);
    		MP_DEBUG2("### bef %d sWinSourceTemp.dwOffset =%d",i,sWinSourceTemp.dwOffset);
    		MP_DEBUG2("### bef %d sWinSourceTemp.wType =%d",i,sWinSourceTemp.wType);
 
			MP_DEBUG2("### bef %d sWinSource.wWidth =%d",i,sWinSource.wWidth);
			MP_DEBUG2("### bef %d sWinSource.wHeight =%d",i,sWinSource.wHeight);
			MP_DEBUG2("### bef %d sWinSource.dwOffset =%d",i,sWinSource.dwOffset);
			MP_DEBUG2("### bef %d sWinSource.wType =%d",i,sWinSource.wType);
			
    		Ipu_ImageScaling(&sWinSource, &sWinSourceTemp, 0, 0, sWinSource.wWidth, sWinSource.wHeight,
         		0, 0, sWinSourceTemp.wWidth, sWinSourceTemp.wHeight, 0);

			sWinSource.pdwStart = sWinSourceTemp.pdwStart;
   			sWinSource.dwOffset = sWinSourceTemp.dwOffset;
    		sWinSource.wWidth   = sWinSourceTemp.wWidth;
    		sWinSource.wHeight  = sWinSourceTemp.wHeight;
    		sWinSource.wType    = sWinSourceTemp.wType;

   		}

	}

	trgWin->pdwStart = sWinSource.pdwStart;
   	trgWin->dwOffset = sWinSource.dwOffset;
    trgWin->wWidth   = sWinSource.wWidth;
    trgWin->wHeight  = sWinSource.wHeight;
    trgWin->wType    = sWinSource.wType; 
}



///
///@ingroup ImageScaler
///@brief Scale image from source to target
///
///@param ST_IMGWIN *srcWin : The image win that you want to scale (source win) \n
///@param	ST_IMGWIN *trgWin : The output image win which has been scaled you want to store (target win) \n
///@param	WORD			sx1			: The left-top x-coordinate of the source win \n
///@param	WORD			sy1			: The left-top y-coordinate of the source win \n
///@param	WORD			sx2			: The right-bottom x-coordinate of the source win \n
///@param	WORD			sy2			: The right-bottom y-coordinate of the source win \n
///@param	WORD			tx1			: The left-top x-coordinate of the target win \n
///@param	WORD			ty1			: The left-top y-coordinate of the target win \n
///@param WORD			tx2			: The right-bottom x-coordinate of the target win \n
///@param	WORD			ty2			: The right-bottom y-coordinate of the target win \n
///@param	BYTE			filter	: if using filter scaling, fill this paremeter with 1, if not fill it with 0 \n
///
///@retval 0:Success / Others:Fail
///
///@remark 
///
BYTE Ipu_ImageScaling(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2, WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE filter)
{
	BYTE ret ;
	
	tx1 = ALIGN_2(tx1);
	tx2 = ALIGN_2(tx2);
	
#if ((CHIP_VER & 0xffff0000) == CHIP_VER_615 || (CHIP_VER & 0xffff0000) == CHIP_VER_612)
	//image_scale(srcWin, trgWin, 0, 0, srcWin->wWidth, srcWin->wHeight, X, Y, X + dstWidth, Y + dstHeight);
	//image_scale(srcWin, trgWin, 0, 0, srcWin->wWidth, srcWin->wHeight, tx1, ty1, tx2, ty2);
	image_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
#elif((CHIP_VER & 0xffff0000) == CHIP_VER_650 || (CHIP_VER & 0xffff0000) == CHIP_VER_660)
	if(filter)
	{
		ret = image_scale(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
		return ret ;
	}
	else
	{
		ret = image_scale_non_filter(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2, 0);
		return ret ;
	}
#endif	
}


/*
 *  src:  source window
 *  trg:  target window
 *  x1, y1:  source start position (x1, y1)
 *  x2, y2:  source end position (x2, y2)
 *  x1 < x2, y1 < y2
 */
void Ipu_Zoom(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2,
			  WORD tx1, WORD ty1, WORD tx2, WORD ty2)
{
	Ipu_ImageScaling(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2,0);
}


// MP650 new functions add here
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
///
///@ingroup ImageScaler
///@brief Overlay two image
///
///@param ST_IMGWIN *srcMainWin 				: The image win that you want to be covered
///@param	ST_IMGWIN *srcSubWin 					: The image win that you want cover the srcMainWin
///@param ST_IMGWIN *trgWin 						: The image win that you want the result image be stored in
///@param	WORD			start_x1						: The left-top x-coordinate of the srcMainWin that you want to cover from
///@param	WORD			start_y1						: The left-top y-coordinate of the srcMainWin that you want to cover from
///@param	BYTE			non_colorkey_level	: Not color key part level, you can choose 0, 1, or 2
///@param	BYTE			colorkey_enable			: If using color key, enable this
///@param	BYTE			colorkey_level			: Color key part level, you can choose 0, 1, or 2
///@param	BYTE			colorkey_Y					: The color key Y value
///@param	BYTE			colorkey_Cb					: The color key Cb value
///@param	BYTE			colorkey_Cr					: The color key Cr value
///
///@retval 0:Success / Others:Fail
///
///@remark  level 0 : Sub win 100 %, Main win 0% \n
// 					level 1 : Sub win 50 %, Main win 50% \n 
// 					level 2 : Sub win 0 %, Main win 100% \n
///
int Ipu_Overlay(ST_IMGWIN * srcMainWin, ST_IMGWIN * srcSubWin, ST_IMGWIN * trgWin, WORD start_x1, WORD start_y1, BYTE non_colorkey_level, BYTE colorkey_enable, BYTE colorkey_level, BYTE colorkey_Y, BYTE colorkey_Cb, BYTE colorkey_Cr)
{
	int ret = 1 ;
	ret = image_overlay(srcMainWin, srcSubWin, trgWin, start_x1, start_y1, non_colorkey_level, colorkey_enable, colorkey_level, colorkey_Y, colorkey_Cb, colorkey_Cr);
	MP_DEBUG("Ipu_Overlay ret =%d", ret);
	return ret ;
}


///
///@ingroup ImageScaler
///@brief Video bypass mode scaling
///
///@param ST_IMGWIN *srcWin 		: The image win that you want to scale (source win)
///@param	ST_IMGWIN *trgWin 		: The output image win which has been scaled you want to store (target win)
///@param	WORD			sx1					: The left-top x-coordinate of the source win \n
///@param	WORD			sy1					: The left-top y-coordinate of the source win \n
///@param	WORD			sx2					: The right-bottom x-coordinate of the source win \n
///@param	WORD			sy2					: The right-bottom y-coordinate of the source win \n
///@param	WORD			tx1					: The left-top x-coordinate of the target win \n
///@param	WORD			ty1					: The left-top y-coordinate of the target win \n
///@param WORD			tx2					: The right-bottom x-coordinate of the target win \n
///@param	WORD			ty2					: The right-bottom y-coordinate of the target win \n
///@param	WORD			mode				: As MJPG, fill this paremeter with 0, if not fill it with 1 \n
///
///@retval None
///
///@remark  
///
BYTE Ipu_Video_Bypass_Scaling(ST_IMGWIN * srcWin, ST_IMGWIN * trgWin, WORD sx1, WORD sy1, WORD sx2, WORD sy2, WORD tx1, WORD ty1, WORD tx2, WORD ty2, BYTE mode)
{
	if(mode == 0)
		image_scale_mjpg_bypass(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	else if(mode == 1)
		image_scale_mpv_bypass(srcWin, trgWin, sx1, sy1, sx2, sy2, tx1, ty1, tx2, ty2);
	return 0 ;
}


BYTE Ipu_Video_Bypass_Show_Main_Win()
{
	Bypass_Overlay_Show_Main_Win();
	
	return 0 ;
}

#endif

