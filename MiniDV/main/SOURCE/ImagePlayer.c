/*
*******************************************************************************
*                               Magic Pixel
*                  5F, No.3, Creation Road III, Science_Based
*                   Industrial Park, Hsinchu, Taiwan, R.O.C
*               (c) Copyright 2004, Magic Pixel Inc, Hsinchu, Taiwan
*
* All rights reserved. Magic Pixel's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code contains
* confidential, trad secret material. Any attempt or participation in
* deciphering, decoding, reverse engineering or in ay way altering the source
* code is strictly prohibited, unless the prior written consent of Magic
* Pixel is obtained.
*
* Filename      : imageplayer.c
* Programmer(s) :
* Created       :
* Descriptions  :
*******************************************************************************
*/

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"

#include "devio.h"
#include "../../libIPLAY/libSrc/display/include/displaystructure.h"
#include "xpg.h"
#include "xpgFunc.h"
#include "setup.h"
#include "Icon.h"
#include "mpapi.h"
#include "imageplayer.h"
//#include "slideEffect.h"
//#include "cv.h"


#if ((CHIP_VER_MSB == CHIP_VER_650) || (CHIP_VER_MSB == CHIP_VER_660))
#define memset          mmcp_memset
#define memcpy          mmcp_memcpy
#else
#define memset          MpMemSet
#define memcpy          MpMemCopy
#endif

#if OUTPUT_JPEG
extern BOOL DUMPMEMORY;
#endif

#pragma alignvar(4)
static IMAGEFILE stImageFile;
static ST_IMGWIN g_sDecodeWin;

/*
// Constant declarations
*/

#define CONTINUE            -3

#define ROTATE_DEGREE_0     0
#define ROTATE_DEGREE_90    1
#define ROTATE_DEGREE_180   2
#define ROTATE_DEGREE_270   3

#define IMAGE_DECODE_THUMB          0x1
#define IMAGE_DECODE_SLIDE_SHOW     0x2

#define ZOOMFACTOR 6


enum
{
    IMAGE_DECODER_JPEG = 0,
    IMAGE_DECODER_BMP,
#if GIF
    IMAGE_DECODER_GIF,
#endif
#if PNG
    IMAGE_DECODER_PNG,
#endif
#if TIFF
    IMAGE_DECODER_TIFF,
#endif
    IMAGE_DECODER_TOTAL
};

enum
{
    ZOOM_RATIO_0_0 = 0,
    ZOOM_RATIO_1_0,
    ZOOM_RATIO_2_0,
    ZOOM_RATIO_3_0,
    ZOOM_RATIO_4_0,
    ZOOM_RATIO_TOTAL
};

ST_IMGWIN *ImageGetDecodeTargetWin()
{
    return &g_sDecodeWin;
}

WORD ImageGetDecodeWinType()
{
    return g_sDecodeWin.wType;
}

#define ZOOM_CENTRE             10
#define ZOOM_CURRENT_XY         20

#define DENOMINATOR             10
#define NEW_DIMENSION_RATIO     7       // ratio = this number / denominator.  i.e. 6 = 60%, this number < denominator
#define NEW_POSITION_RATIO      ((10 - NEW_DIMENSION_RATIO) >> 1)

void ImagePlayer_CloseFile()
{
    IMAGEFILE *psImageFile = &stImageFile;

    ImageFile_Close(psImageFile);
}

IMAGEFILE *ImagePlayer_DecodeFile(ST_SEARCH_INFO *pSearchInfo, BYTE *pbTarget, DWORD dwTargetSize, BYTE bMode)
{
    STREAM *psHandle;
    BYTE *pbSource;
    DWORD dwSourceSize;
    int   iErrorCode = 0;
    IMAGEFILE *psImageFile = &stImageFile;

    ImageReleaseAllBuffer();

#if SAVE_FILE_TO_SPI
    if (g_boSaveFileToSPI)
    {
        DWORD dwFileIndex;
        DWORD dwFileSize;
        int error;

        if (pSearchInfo == NULL) {
            ST_FILE_BROWSER *psBrowser;
            mpDebugPrint("%s: (pSearchInfo == NULL) => get current image file...", __FUNCTION__);
            // get current image file
            psBrowser = (ST_FILE_BROWSER *)&g_psSystemConfig->sFileBrowser;
            if (psBrowser->dwImgAndMovCurIndex >= psBrowser->dwImgAndMovTotalFile)
            {
                MP_ALERT("%s: No file ! (psBrowser->dwImgAndMovCurIndex (%u) >= psBrowser->dwImgAndMovTotalFile (%u)) !", __FUNCTION__, psBrowser->dwImgAndMovCurIndex, psBrowser->dwImgAndMovTotalFile);
                return NULL;
            }
            pSearchInfo = &psBrowser->sImgAndMovFileList[psBrowser->dwImgAndMovCurIndex];
        }
        
        dwFileIndex = *((DWORD*)(pSearchInfo->bName));
        mpDebugPrint("Decode SPI image file %d", dwFileIndex);
        psHandle = NULL;

        dwFileSize = spifs_GetFileSize(dwFileIndex);
        mpDebugPrint("spi file %d, size = %d", dwFileIndex, dwFileSize);
        dwSourceSize = dwFileSize; //256 * 1024;
        pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
        if (pbSource)
            spifs_ReadFileToBuffer(dwFileIndex, pbSource, &error);
        
        if (error)
            mpDebugPrint("Load SPI image file error - %d", error);
    }
    else
#endif
    {
        psHandle = (STREAM *)FileBrowser_GetCurImageFile(pSearchInfo);
        if (psHandle == NULL)
        {
            MP_ALERT("%s: (psHandle == NULL) !!", __FUNCTION__);
            return NULL;
        }
        dwSourceSize = 256 * 1024;
        pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
    }

    if (pbSource != NULL)
    {
        iErrorCode = ImageFile_DecodeFile(psImageFile, psHandle, pbSource, dwSourceSize, bMode);
        //ImagePlayer_CloseFile();
    }

    ImagePlayer_CloseFile();
    ImageReleaseSourceBuffer();

    if (iErrorCode)
    {
#if RECORD_INVALID_MEDIA
        if ((bMode & 0x3f) != IMG_DECODE_THUMB)
            pSearchInfo->bParameter |= SEARCH_INFO_INVALID_MEDIA; // record invalid file
#endif
        MP_DEBUG1("-E- Image decode fail %d", iErrorCode);
        //ImagePlayer_CloseFile();

        return NULL;
    }

    return psImageFile;
}

WORD Img_GetOriginalWidth()
{
    return stImageFile.wRealTargetWidth;
}

WORD Img_GetOriginalHeight()
{
    return stImageFile.wRealTargetHeight;
}

WORD Img_GetCDU_DecodeWidth()
{
    return stImageFile.wTargetWidth;
}

void ImageDraw_FitToFull(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin)
{
    int x, y, w, h;
    x = 0;
    y = 0;
    w = pTrgWin->wWidth;
    h = pTrgWin->wHeight;
#if 1 // don't scale too much
    if (w > pSrcWin->wWidth * 4) w = pSrcWin->wWidth * 4;
    if (h > pSrcWin->wHeight * 4) h = pSrcWin->wHeight * 4;

    w = ALIGN_16(w);
    if (w > pTrgWin->wWidth) w = pTrgWin->wWidth;
    if (h > pTrgWin->wHeight) h = pTrgWin->wHeight;

    x = ((pTrgWin->wWidth - w) >> 1);
    y = ((pTrgWin->wHeight - h) >> 1);

    x = ALIGN_CUT_4(x);
#endif

    //Ipu_ImageScaling(pSrcWin, pTrgWin, w, h, x, y);
    Ipu_ImageScaling(pSrcWin, pTrgWin, 0, 0, pSrcWin->wWidth, pSrcWin->wHeight, x, y, x+w, y+h, 0);
}



void ImageDraw_FitToWidth(ST_IMGWIN * pSrcWin, ST_IMGWIN * pTrgWin)
{
    DWORD Ratio;
    WORD wSrcWidth, wSrcHeight, wSrcSX, wSrcSY;
    WORD wTrgWidth, wTrgHeight, wTrgSX, wTrgSY;

    wSrcWidth = pSrcWin->wWidth;
    wSrcHeight = pSrcWin->wHeight;

    Ratio = FIX_POINT(wSrcWidth) / pTrgWin->wWidth;
    wSrcSX = 0;
    wTrgSX = 0;
    wTrgWidth = pTrgWin->wWidth;
    wTrgHeight = FIX_POINT(wSrcHeight) / Ratio;

    if (pTrgWin->wHeight >= wTrgHeight)
    {
        wSrcSY = 0;
        wTrgSY = ((pTrgWin->wHeight - wTrgHeight) >> 1);
    }
    else if (wTrgHeight > pTrgWin->wHeight)
    {
        wTrgSY = 0;
        wTrgHeight = pTrgWin->wHeight;
        wSrcHeight = FIX_POINT_R(wTrgHeight * Ratio);
        wSrcSY = ((pSrcWin->wHeight - wSrcHeight) >> 1);
    }

    Ipu_Zoom(pSrcWin, pTrgWin,
    wSrcSX, wSrcSY, wSrcSX + wSrcWidth, wSrcSY + wSrcHeight,
    wTrgSX, wTrgSY, wTrgSX + wTrgWidth, wTrgSY + wTrgHeight);
}



//#define GUARANTEE_SIZE   8 // 80%
void ImageScaleFromJPEGTarget(ST_IMGWIN * pWin, BYTE * pbTargetBuf)
{
    MP_DEBUG("%s()", __FUNCTION__);
    mpClearWin(pWin);
        //ImageDraw_FitToWidth(&g_sDecodeWin, pWin);
        //ImageDraw_FitToFull(&g_sDecodeWin, pWin);
        ImageDraw_FitToOrigin(&g_sDecodeWin, pWin);
}



ST_IMGWIN* ImageGetDecodeWin()
{
    return &g_sDecodeWin;
}

int ImageDraw_Decode(ST_IMGWIN * pWin, BYTE blSlideShow)
{
    ST_JPEG * pjpg = NULL;
    BYTE bMode ;
    BYTE *TargetBuffer = NULL ;

    if (blSlideShow == IMG_DECODE_ROTATE_CLOCKWISE || blSlideShow == IMG_DECODE_ROTATE_COUNTERCLOCKWISE || blSlideShow == IMG_DECODE_ROTATE_UPSIDEDOWN)
        bMode = blSlideShow ;
    else
        bMode = blSlideShow ? IMG_DECODE_SLIDE : IMG_DECODE_PHOTO;

    IMAGEFILE *psImageFile = ImagePlayer_DecodeFile(NULL, NULL, 0, bMode);

    if (psImageFile == NULL)
    {
        MP_ALERT("%s: #psImageFile == NULL !!", __FUNCTION__);
        return FAIL;
    }

    if (psImageFile->bNityDegree)
        TargetBuffer = (BYTE *)psImageFile->pbNityDegreeTarget;
    else
        TargetBuffer = (BYTE *)psImageFile->pbTarget;

    g_sDecodeWin.pdwStart = (DWORD *)((DWORD)TargetBuffer | 0xA0000000);
    WORD wd = ImageFile_GetTargetWidth();

    if (wd != ALIGN_16(wd))
    {
        MP_DEBUG("target width not 16 aligned");
    }

    pjpg = (ST_JPEG *)(psImageFile->psJpeg);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
    g_sDecodeWin.dwOffset = (Img_GetCDU_DecodeWidth() << 1) ;
#else
    g_sDecodeWin.dwOffset = ALIGN_16(wd) << 1;
#endif
    g_sDecodeWin.wWidth = Img_GetOriginalWidth();
    g_sDecodeWin.wHeight = Img_GetOriginalHeight();
    g_sDecodeWin.wType = pjpg->wImageType ;

    return PASS;
}



SWORD ImageDraw(ST_IMGWIN * pWin, BYTE bSlideShow)
{
#if OUTPUT_JPEG
    DUMPMEMORY = TRUE;
#endif
		BYTE *TargetBuffer = NULL ;

    if (ImageDraw_Decode(pWin, bSlideShow) == FAIL)
    {
        MP_ALERT("%s: jpeg = %d fail", __FUNCTION__, g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
        return FAIL;
    }

    TargetBuffer = ImageGetTargetBuffer();

    ST_PANEL *pstPanel = g_psSystemConfig->sScreenSetting.pstPanel;
    ImageScaleFromJPEGTarget(pWin, TargetBuffer);
    ImageReleaseTargetBuffer();

#if OUTPUT_JPEG
    if (DUMPMEMORY)
    {
        DRIVE *sDrv;
        STREAM *shandle;
        int ret;
        int len;
        WORD width, height;

        width = (pWin->wWidth + 15) & 0xFFF0;
        height = pWin->wHeight;
        len = (width * height) << 1;

        mpDebugPrint("creat scaling.bin");
        mpDebugPrint("width %04d", width);
        mpDebugPrint("height %04d", height);

        sDrv = DriveGet(SD_MMC);
        ret = CreateFile(sDrv, "scaling", "bin");

        if (ret)
            mpDebugPrint("create file fail\r\n");
        {
            shandle = FileOpen(sDrv);

            if (!shandle)
                mpDebugPrint("open file fail\r\n");
            else
            {
                ret = FileWrite(shandle, pWin->pdwStart, len);

                if (!ret)
                    mpDebugPrint("write file fail\r\n");

                FileClose(shandle);
                mpDebugPrint("\n\rfile close\n\r");
            }
        }
    }
#endif

    MP_DEBUG1("jpeg = %d pass", g_psSystemConfig->sFileBrowser.dwImgAndMovCurIndex);
    //ext_mem_leak_detector();

    return PASS;
}

#if FILE_IN_RESOURCE//SlideFromResource
int mpx_DrawPhotoFromRES(ST_IMGWIN * pTrgWin,DWORD dwTag,BOOL bSlideShow)
{
	SWORD swRet = FAIL;
	DWORD  dwWidth, dwHeight;
	DWORD dwSourceSize = 0, dwTrgSize = 0, dwRetry = 0;
	DWORD *pdwSource0 = NULL, *pdwTrgBuf = NULL;
	BYTE * TempBuffer=NULL;
	ST_IMGWIN *pNextWin = Idu_GetNextWin(),stTmpWin;

//	MP_DEBUG("_%s_", __FUNCTION__);

	ImageReleaseSourceBuffer();
	ImageReleaseTargetBuffer();
	
#if (BOOTUP_TYPE == BOOTUP_TYPE_NAND)
	#if DRAW_DEMO_THUMB_FROM_RES
	if ((bSlideShow == IMG_DECODE_THUMB)&&((dwTag&0xffff0000)==0x4a500000)) //JPXX
	{
		dwTag = (dwTag & 0x0000ffff)|0x544d0000;//TMXX
		dwSourceSize = ISP_GetResourceSize(dwTag);
		if (!dwSourceSize)
		{
		dwTag = (dwTag & 0x0000ffff)|0x4a500000;
		dwSourceSize = ISP_GetResourceSize(dwTag);
		}
	}
	else
	#endif
	dwSourceSize = ISP_GetResourceSize(dwTag);
    	//mpDebugPrint("mpx_DrawPhotoFromRES dwTag=%x dwSourceSize=%d", dwTag,dwSourceSize);
    if (!dwSourceSize)
        goto CLEAR_AND_RETURN;
    pdwSource0 = ext_mem_malloc(dwSourceSize + 512);
    //memset (pdwSource0, 0, dwSourceSize + 512);
    if (!ISP_GetResource(dwTag, pdwSource0,0))
        goto CLEAR_AND_RETURN;
	#if RUN_CLOCK_WHEN_DECODING
	CheckToUpdateClock();
	#endif
#elif(BOOTUP_TYPE == BOOTUP_TYPE_SPI)
	dwSourceSize = ISP_GetResourceSize(dwTag);   
	MP_DEBUG("mpx_DrawPhotoFromRES dwTag:%x->%d",dwTag,dwSourceSize);
	if (!dwSourceSize)
        	goto CLEAR_AND_RETURN;	
	TempBuffer = (BYTE *)ext_mem_malloc(dwSourceSize+512);
	if (TempBuffer != NULL)
		pdwSource0 = (DWORD *) ((DWORD)TempBuffer | 0xa0000000);
   	 if (!pdwSource0)
        	goto CLEAR_AND_RETURN;
		if(! ISP_GetResource(dwTag,pdwSource0,dwSourceSize))
		{
			mpDebugPrint("ISP_GetResource  FAIL ==================");
			 goto CLEAR_AND_RETURN;
		}
#else 
	DWORD *pdwRes = (DWORD *) ISP_GetResource(dwTag);
    dwSourceSize = ISP_GetResourceSize(dwTag);
    if (!dwSourceSize)
        goto CLEAR_AND_RETURN;
	pdwSource0 = (DWORD)ext_mem_malloc(ALIGN_4(dwSourceSize));
    if (!pdwSource0)
        goto CLEAR_AND_RETURN;
	Idu_WaitBufferEnd();
	memcpy(pdwSource0, pdwRes, dwSourceSize);
#endif

	#if DRAW_DEMO_THUMB_FROM_RES
	if ((bSlideShow == IMG_DECODE_THUMB)&&((dwTag&0xffff0000)==0x544d0000)) //TMXX
	{
		dwWidth=(*(pdwSource0+3)<<32)|(*(pdwSource0+2));
		dwHeight=(*(pdwSource0+5)<<32)|(*(pdwSource0+4));
		mpDebugPrint("RES thumb %x size is(wxh) %d x %d",dwTag,dwWidth,dwHeight);
		dwTrgSize = ALIGN_16(dwWidth)*dwHeight*2;
		mpWinInit(ImageGetDecodeWin(),ImageAllocTargetBuffer(dwHeight*ALIGN_16(dwWidth)*2+32),dwHeight,dwWidth);
		pdwTrgBuf = pdwSource0+6;
		memcpy((BYTE*)ImageGetTargetBuffer(),(BYTE*)(pdwTrgBuf),dwWidth*dwHeight*2);
		pdwTrgBuf = NULL;
		swRet = PASS;
		
		goto CLEAR_AND_RETURN;
	}
	#endif
	if(pTrgWin ==NULL)
	{
		dwWidth = pNextWin->wWidth;
		dwHeight = pNextWin->wHeight;
	}
	else
	{
		dwWidth = pTrgWin->wWidth;
		dwHeight = pTrgWin->wHeight;
	MP_DEBUG("Trg WIN size:w=%d  h=%d",pTrgWin->wWidth,pTrgWin->wHeight);
		
		if(dwWidth < pNextWin->wWidth)
			dwWidth = pNextWin->wWidth;
		if(dwHeight < pNextWin->wHeight)
			dwHeight = pNextWin->wHeight;
	}
	MP_DEBUG("WIN size:w=%d  h=%d",dwWidth,dwHeight);
	dwTrgSize = ALIGN_16(dwWidth) * dwHeight* 2+64*1024;

	if (ext_mem_get_free_space()<dwTrgSize)
		dwTrgSize=ext_mem_get_free_space();
	pdwTrgBuf =(DWORD)ext_mem_malloc(dwTrgSize);
    if ((0 == dwTrgSize) || (NULL == pdwTrgBuf))
        goto CLEAR_AND_RETURN;
	MP_TRACE_LINE();
	swRet = Img_Jpeg2ImgBuf((BYTE*)pdwSource0, (BYTE *)pdwTrgBuf, IMG_DECODE_PHOTO, dwSourceSize, dwTrgSize);
	if (swRet != 0)
	{
		mpDebugPrint("Img_Jpeg2ImgBuf fail , swRet =%d", swRet);
	}
	else
	{
		dwWidth = Jpg_GetImageWidth(); //Jpg_GetTargetWidth by Tech 20091231 
		dwHeight = Jpg_GetImageHeight();//Jpg_GetTargetHeight by Tech 20091231 
		MP_DEBUG("Img size:w=%d  h=%d",dwWidth,dwHeight);
		
		if((dwWidth == 0) || (dwHeight == 0) || (swRet != 0)) 
	        goto CLEAR_AND_RETURN;
		if (pTrgWin !=NULL)
		{
			mpWinInit(ImageGetDecodeWin() ,pdwTrgBuf, dwHeight, ALIGN_CUT_16(dwWidth)); //fengrs 20080401 tempory change //mpWinInit(GetDecodeWin() ,pdwTrgBuf, dwHeight, dwWidth);
			#if 0//GIINII_FOR_PHILIP2009_UI_FLOW
			if(g_bAniFlag & ANI_CALSLIDE)
				ImageScaleFromJPEGTarget(pTrgWin, pdwTrgBuf);
			else
			#endif
				ImageDraw_FitToFull(ImageGetDecodeWin(), pTrgWin);
			//ImageDraw_Rotate((ST_IMGWIN *)GetDecodeWin(),pTrgWin,ROTATE_DEGREE_90);
			//ImageDraw_FitToHeight((ST_IMGWIN *)GetDecodeWin(), pTrgWin);	
		}
		if (pTrgWin ==NULL
#if (ANGLESWITCH && SlideFromResource)
			||(GetSpecFlag(SlideFromResource) && !bSlideShow)
#endif
			)
		{
			mpWinInit(&stTmpWin ,pdwTrgBuf, pNextWin->wHeight, pNextWin->wWidth); 
			mpWinInit(ImageGetDecodeWin(),ImageAllocTargetBuffer(pNextWin->wHeight*ALIGN_CUT_16(pNextWin->wWidth)*2+32),
				pNextWin->wHeight,ALIGN_CUT_16(pNextWin->wWidth));
			mpCopyWin(ImageGetDecodeWin(), &stTmpWin);
		}
	}
CLEAR_AND_RETURN:
	if (pdwTrgBuf) 
		ext_mem_free(pdwTrgBuf);
	if(pdwSource0)
		ext_mem_free(pdwSource0);
	if (bSlideShow)
	{
		ImageReleaseSourceBuffer();
		ImageReleaseTargetBuffer();
	}
    if (swRet)
    	mpDebugPrint("Decode internal image [0x%x] fail!", dwTag);
	return swRet;
}
#endif



