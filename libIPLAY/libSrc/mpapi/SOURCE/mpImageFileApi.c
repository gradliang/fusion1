/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/

#include "global612.h"
#include "mpTrace.h"
#include "ImageFile.h"
#include "display.h"
#include "taskid.h"


//--------------------------------------------------------------------------------------------

#pragma alignvar(4)
IMAGEFILE *g_psCurImageFile = NULL;

static BYTE *pb_SourceBuffer = NULL;
static BYTE *pb_TargetBuffer = NULL;
static DWORD dwSourceSize;
static DWORD dwTargetSize;

/*Get the output TYPE of the current  decoded IMAGE*/
#pragma alignvar(4)
WORD g_wDecodedImageType = 422; /*Default = 422*/
/*---------------------------------------------------------*/
/*Default output image type= 422*/
/*If YUV444_ENABLE is enabled, it maybe get 422 or 444.*/
WORD GetCurImgOutputType(void)
{
	return g_wDecodedImageType;
}



//--------------------------------------------------------------------------------------------
BYTE ImageFile_GetImageFormat(IMAGEFILE *psImage)
{
	WORD wFileTag;

	if (psImage->psStream == NULL) return IMAGE_TYPE_UNKOWN;

	mpxStreamSeek(psImage->psStream, 0, SEEK_SET);
	wFileTag = mpxStreamReadWord(psImage->psStream);
	MP_DEBUG1("FileTag %4x", wFileTag);
	switch(wFileTag)
	{
	 	case IMAGE_TAG_JPEG:
			return IMAGE_TYPE_JPEG;
		case IMAGE_TAG_BMP:
			return IMAGE_TYPE_BMP;
#if GIF
		case IMAGE_TAG_GIF:
			return IMAGE_TYPE_GIF;
#endif
#if PNG
		case IMAGE_TAG_PNG:
			return IMAGE_TYPE_PNG;
#endif
#if TIFF
		case IMAGE_TAG_TIFF_LITTLE:
		case IMAGE_TAG_TIFF_BIG:
			return IMAGE_TYPE_TIFF;
#endif
		}
	return IMAGE_TYPE_UNKOWN;
	}
//--------------------------------------------------------------------------------------------
DWORD ImageFile_GetImageSize(IMAGEFILE * psImage)
	{
	MP_ASSERT(psImage != NULL);

	//wOriginalWidth = psImage->wImageWidth;
	//wOriginalHeight = psImage->wImageHeight;

	return (psImage->wImageWidth << 16) | psImage->wImageHeight;
}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetThumbWidth()
{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wThumbWidth;
}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetThumbHeight()
	{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wThumbHeight;
	}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetImageWidth()
{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wImageWidth;
}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetImageHeight()
{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wImageHeight;
}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetTargetWidth()
	{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wTargetWidth;
	}
//--------------------------------------------------------------------------------------------
WORD ImageFile_GetTargetHeight()
	{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->wTargetHeight;
	}
//--------------------------------------------------------------------------------------------
BYTE *ImageFile_GetTargetBuffer()
	{
	if (g_psCurImageFile == NULL) return 0;
	return g_psCurImageFile->pbTarget;
	}
//--------------------------------------------------------------------------------------------
IMAGEFILE *ImageFile_GetCurImageFile()
{
	return g_psCurImageFile;
}
//--------------------------------------------------------------------------------------------

int ImageFile_DecodeThumb(IMAGEFILE * psImage)
{
	ST_JPEG *psJpeg;

	MP_DEBUG("ImageFile_DecodeThumb");
	if (psImage->ImageDecodeThumb == NULL) return FAIL;

	psImage->iErrorCode = psImage->ImageDecodeThumb(psImage);

	MP_DEBUG8("image %dx%d, target %dx%d, real %dx%d, thumb %dx%d",
		psImage->wImageWidth, psImage->wImageHeight,
		psImage->wTargetWidth, psImage->wTargetHeight,
		psImage->wRealTargetWidth, psImage->wRealTargetHeight,
		psImage->wThumbWidth, psImage->wThumbHeight);

	return psImage->iErrorCode;

}
//--------------------------------------------------------------------------------------------

int ImageFile_DecodeImage(IMAGEFILE * psImage)
{
	MP_DEBUG("ImageFile_DecodeImage");
	MP_ASSERT(psImage != NULL);
	TaskYield();

	if (psImage->ImageDecodeImage == NULL) return FAIL;
	psImage->iErrorCode = psImage->ImageDecodeImage(psImage);

	MP_DEBUG8("image %dx%d, target %dx%d, real %dx%d, thumb %dx%d",
		psImage->wImageWidth, psImage->wImageHeight,
		psImage->wTargetWidth, psImage->wTargetHeight,
		psImage->wRealTargetWidth, psImage->wRealTargetHeight,
		psImage->wThumbWidth, psImage->wThumbHeight);
	//MP_DEBUG("ImageFile_DecodeImage done");
	return psImage->iErrorCode;
}

#if MPO
int ImageFile_DecodeImageRight(IMAGEFILE * psImage)
{
    if(!psImage->ImageDecodeImageRight)
        return FAIL;

    MP_DEBUG("ImageFile_DecodeImageRight");
	MP_ASSERT(psImage != NULL);
    TaskYield();

	psImage->iErrorCode = psImage->ImageDecodeImageRight(psImage);
	return psImage->iErrorCode;
}
#endif

//--------------------------------------------------------------------------------------------

int ImageFile_Decoder_Init(IMAGEFILE *psImage, ST_MPX_STREAM *psStream, BYTE bMode, DWORD dwOffset)
{

	MP_ASSERT(psImage != NULL);
	MP_ASSERT(psStream != NULL);

	psImage->iErrorCode = 0;
	//memset(psImage, 0, sizeof(IMAGEFILE));

	psImage->psStream = psStream;
	psImage->bFileFormat = ImageFile_GetImageFormat(psImage);

	psImage->bDecodeMode = bMode;
	psImage->dwDecodeOffset = dwOffset;
	psImage->bScaleDown = 0;
	psImage->bNityDegree = 0;

	switch (psImage->bFileFormat)
{
		case IMAGE_TYPE_JPEG :
#if MPO			
            if (UtilStringCompareWoCase08((BYTE *) &psImage->bExtension[1], (BYTE *) "MPO", 3))
            {			
              psImage->iErrorCode = Mpo_Decoder_ImageFile_Init(psImage);
            }
            else
#endif				
            {
			  psImage->iErrorCode = Jpg_Decoder_ImageFile_Init(psImage);
            }
			break;
#if BMP
		case IMAGE_TYPE_BMP :
			psImage->iErrorCode = Bmp_Decoder_Init(psImage);
			break;
#endif
#if GIF
		case IMAGE_TYPE_GIF :
			psImage->iErrorCode = Gif_Decoder_Init(psImage);
			break;
#endif
#if PNG
		case IMAGE_TYPE_PNG :
			psImage->iErrorCode = Png_Decoder_Init(psImage);
			break;
#endif
#if TIFF
		case IMAGE_TYPE_TIFF :
			psImage->iErrorCode = TIFF_Decoder_Init(psImage);
			break;
#endif
        default:
            psImage->iErrorCode = IMAGE_TYPE_UNKOWN; //file header is not photo, then return error
            break;
		}
	MP_DEBUG("ImageFile_Decoder_Init end, psImage->iErrorCode %d",psImage->iErrorCode);
	return psImage->iErrorCode;
}

//--------------------------------------------------------------------------------------------

void ImageFile_Close(IMAGEFILE *psImage)
{
	MP_ASSERT(psImage != NULL);

#if MPO
    if(psImage->ImageDecodeImageClose)
	    psImage->ImageDecodeImageClose(psImage);
#endif

	if (psImage->psStream) {
		mpxStreamClose(psImage->psStream);
		psImage->psStream = NULL;
	}

#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
	Clear_Img_FileChainFlag();
#endif
}

//--------------------------------------------------------------------------------------------

int ImageFile_Open(IMAGEFILE *psImage, STREAM * psHandle, BYTE  *pbSource, DWORD dwSourceSize, BYTE bMode)
{
	int iErrorCode;
	ST_MPX_STREAM *psStream = &psImage->sStream;

	//MP_DEBUG1("source buffer size %d", dwSrcBufferSize);
	//MP_DEBUG2("head tag %2x%2x", pbSource[0], pbSource[1]);
	//MP_DEBUG1("file handle %08x", (DWORD)psHandle);

#if ENABLE_IMAGE_FILE_CHAIN_CLUSTERS_CACHING
	Set_Img_FileChainFlag();
#endif

	if (psHandle != NULL) {
		bMode |= IMG_DECODE_CHASE;
		BOOL boShowBar = 0;//((bMode & 0x3f) == IMG_DECODE_PHOTO) && FileSizeGet(psHandle) > 800 * 1024;
		iErrorCode = mpxStreamOpen(psStream, psHandle, pbSource, dwSourceSize, boShowBar,STREAM_SOURCE_FILE);
	} else
		iErrorCode = mpxStreamOpenBuffer(psStream, pbSource, dwSourceSize);

	if (iErrorCode != PASS) return iErrorCode;

    iErrorCode = ImageFile_Decoder_Init(psImage, psStream, bMode, 0);

    if (psImage->wImageWidth == 0 || psImage->wImageHeight == 0) return FAIL; //error photo

	MP_DEBUG("ImageFile_Open image %dx%d, iErrcode %d", psImage->wImageWidth, psImage->wImageHeight, iErrorCode);
	return iErrorCode;
}


//--------------------------------------------------------------------------------------------

BYTE ImageFile_CheckSize(WORD width, WORD height, DWORD check_size, BYTE bIfThumbNail)
{
	BYTE i = 0;
	ST_IMGWIN *ImgWin = Idu_GetNextWin();
	DWORD image_size;
	WORD w,h;
	image_size = ALIGN_16(width) * ALIGN_16(height) * 2;

	w=ImgWin->wWidth;
	h=ImgWin->wHeight;

	if (w == 480)
		{
			w=720;
			h=480;
		}

	MP_DEBUG("bIfThumbNail is %d, ImgWin->wWidth is %d, ImgWin->wHeight is %d", bIfThumbNail, w, h);
	MP_DEBUG("image_size is %d, check_size is %d, h is %d", image_size, check_size, h);

	if (bIfThumbNail )
	{
		for(i=0;i<8;i++)
		{
			if ((image_size>>(i*2)) <= check_size)
			break;
		}

		MP_DEBUG("Decoding a ThumbNail psImage->bScaleDown is %d", i);
		return i;

	}
	else
	{
		for(i=7;i>0;i--)
		{
			if(((width>>i)>=w)||((height>>i)>=h))
			break;
		}
		if ((image_size>>(i*2)) >= check_size) i++;

		MP_DEBUG("Decoding an Image psImage->bScaleDown is %d", i);
		return i;

	}
		MP_DEBUG("Exceptions in ImageFile_CheckSize bIfThumbNail is %d", bIfThumbNail);
	return 0;
}


#if ( SET_RATIO_BY_BUF_WIDTH )
/*Jpg_OpenFitTrgBuf is used before  ImageFile_DecodeFile        */
/*Jpg_CloseFitTrgBuf MUST be used after ImageFile_DecodeFile.*/
/*If it is successfully, g_bRatioByTrgBufWidthFlag is enabled.*/
/* return : 0: PASS, -1: FAIL                                                         */
/* Input parameter:                                                                     */
/*          bBufOutPtr: the pointer of the allocated memory buffer to store data.*/
/*          wTrgBufWidth: the width of the frame.*/
/*          wTrgBufHeight: the height of the frame.*/
int Jpg_OpenFitTrgBuf(BYTE *bBufOutPtr, WORD wTrgBufWidth, WORD wTrgBufHeight )
{
	int iErrorCode;
    iErrorCode = Jpg_OpenRatio_ByTrgBufWidth(bBufOutPtr, wTrgBufWidth, wTrgBufHeight);
	return iErrorCode;
}

/*If Jpg_OpenFitTrgBuf is used before  ImageFile_DecodeFile.     */
/*Jpg_CloseFitTrgBuf MUST be used after ImageFile_DecodeFile.*/
/*Jpg_CloseFitTrgBuf :                                                                */
/*         1. Check  it is successfull or not.                                     */
/*         2. g_bRatioByTrgBufWidthFlag is disabled.                     */
/*       If the return value is FAIL, the data of the Buffer is INVALID.*/
/* return : 0: PASS, -1: FAIL*/
/*             *wFinalBufWidth: Final width of the buffer */
/*             *wFinalBufHeight: Final Height: of the buffer */
int Jpg_CloseFitTrgBuf(WORD *wFinalBufWidth, WORD *wFinalBufHeight)
{ 	
	int iErrorCode;
	WORD wFinalBufWidth1;
	WORD wFinalBufHeight1;
    iErrorCode = Jpg_CloseRatio_ByTrgBufWidth(&wFinalBufWidth1, &wFinalBufHeight1);

    *wFinalBufWidth  = wFinalBufWidth1;
	*wFinalBufHeight = wFinalBufHeight1;
    
    MP_DEBUG1("wFinalBufWidth=%d",wFinalBufWidth1);

	return iErrorCode;
}


/*Cut the garbage of  decoded buffer*/

/*----------------------------*/
/*Only Support 422 type*/
int Cut_SrcBuf_to_TrgBuf(BYTE *TrgBuf, BYTE *SrcBuf, WORD SrcBufWidth, WORD SrcBufHeight, WORD CutWidth , WORD CutHeight )
{
	
	WORD dwTempY;
	WORD dwRowDstImgWidth;

    BYTE *bSrcBufAddr = NULL;
	BYTE *bTrgBufAddr = NULL;

	WORD BytesForImgType = 2; /*2: 422 type*/

	if((TrgBuf == NULL) || (SrcBuf == NULL))
	{
	  MP_ALERT("%s: TrgBuf or SrcBuf is NULL!! ", __FUNCTION__ );
	  goto Error;
	}

    if( (SrcBufWidth <= 0) || (SrcBufHeight <= 0) || (CutWidth < 0))
    {
	  MP_ALERT("%s: Input Value <= 0", __FUNCTION__ );
	  goto Error;
    }

	BytesForImgType = 2; /*yuv 422 type*/

	if(CutWidth == 0)
	{
	  //Waring
	  MP_ALERT("%s: Waring. CutWidth = 0", __FUNCTION__);
	}

	if(CutHeight == 0)
	{
	  //Waring
	  MP_ALERT("%s: Waring. CutHeight = 0", __FUNCTION__);
	}
	
	dwRowDstImgWidth = SrcBufWidth - CutWidth;
	bSrcBufAddr      = SrcBuf;
	bTrgBufAddr      = TrgBuf;
	
	for(dwTempY=0; dwTempY < ( SrcBufHeight - CutHeight ) ; dwTempY++)
	 {
	   mmcp_memcpy_polling(bTrgBufAddr, bSrcBufAddr, dwRowDstImgWidth * BytesForImgType);
	   bSrcBufAddr +=  (SrcBufWidth * BytesForImgType);/*2=422 type*/
	   bTrgBufAddr +=  (dwRowDstImgWidth * BytesForImgType); /*2=422 type*/
	 }	
	return PASS;

Error:
	return FAIL;	
}

 /*----------------------------*/

#endif




///
///@ingroup ImageDecoder
///@brief Decode given image file
///
///@param IMAGEFILE *psImage 			: An image structure defined in "ImageFile.h"
///@param	STREAM 		*psHandle 		: The file handle of the given image file
///@param	BYTE			*pbSource			: The file source buffer that would be stored file data from MCARD.
///@param	DWORD			dwSourceSize	: The file source buffer size
///@param	BYTE			bMode					: The decode option defined in FlagDefine.h
///
///@retval 0:Success / Others:Fail
///
///@remark  The decode option would be "IMG_DECODE_PHOTO", "IMG_DECODE_THUMB" and so forth. \n
///					For MP650 "IMG_DECODE_ROTATE_CLOCKWISE", "IMG_DECODE_ROTATE_UPSIDEDOWN" and "IMG_DECODE_ROTATE_COUNTERCLOCKWISE" \n
///					are added for HW rotation \n
///
int ImageFile_DecodeFile(IMAGEFILE *psImage, STREAM * psHandle, BYTE  *pbSource, DWORD dwSourceSize, BYTE bMode)
{
	int iErrorCode;
	DWORD dwSizeLimit = 0,dwFreeSpace,dwReserveBuffer,dwRotateBuffer,dwChasingBuffer;
	ST_JPEG * psJpeg = NULL;
  WORD w, h;
  ST_IMGWIN *ImgWin = Idu_GetCurrWin();
  register struct ST_IMAGE_PLAYER_TAG *sImagePlayer = &(g_psSystemConfig->sImagePlayer);

  extern ST_JPEG g_sJpeg[2];

  if(psImage == NULL)
  { 
  	MP_ALERT("-E- %s: psImage is NULL.");
	__asm("break 100");/*It should not here.*/
  }

  psImage->psJpeg = (ST_JPEG *)&g_sJpeg[0];//(ST_JPEG *)(psImage->psJpeg);

  if(psImage->psJpeg == NULL)
  { 
  	MP_ALERT("-E- %s: psImage->psJpeg is NULL.");
	__asm("break 100");/*It should not here.*/
  }

	g_psCurImageFile = psImage;
	g_wDecodedImageType = 422;/*Default output image type= 422*/

  //Only for DEBGU!! Frank Lin check memroy leak	
  //MP_ALERT("####-------->Total Free mem=%d", mem_get_free_space_total());

#if OPEN_EXIF 
	ST_EXIF *psEXIF=NULL;
	psEXIF=(ST_EXIF *)ext_mem_malloc(sizeof(ST_EXIF));
	extern ST_EXIF *g_psEXIFEnc;
	g_psEXIFEnc=psEXIF;/*NEED!! and NEED to set NULL later. Set the EXIF structure pointer*/
	memset(psEXIF, 0, sizeof(ST_EXIF));
#endif

	iErrorCode = ImageFile_Open(psImage, psHandle, pbSource, dwSourceSize, bMode);

#if OPEN_EXIF 
    if(iErrorCode == PASS)
    {
	  SDWORD ret;
	  ret=JepgExifTagReportInfo(&(psImage->sImageDecEXIFinfo),psEXIF);
    }
    g_psEXIFEnc=NULL;/*NEED!! Set g_psEXIFEnc=NULL before release pxEXIF*/
	if(psEXIF != NULL)
	{
	  ext_mem_free(psEXIF);
	}
#endif

	psJpeg = (ST_JPEG *)(psImage->psJpeg);
#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	if((bMode>>4)&0x3)	// if rotate
		psJpeg->bRotate = (bMode>>4);
#endif

	if (iErrorCode != PASS)
	{
		MP_ALERT("%s: ImageFile_Open() failed !", __FUNCTION__);
		return FAIL;
	}

	w = ALIGN_16(psImage->wImageWidth);
	h = ALIGN_16(psImage->wImageHeight);
	if (w == 0 || h == 0)
	{
		MP_ALERT("%s: Image width or height = 0", __FUNCTION__);
		return FAIL;
	}
  if (bMode == IMG_DECODE_SLIDE)
  {
#ifdef SLIDESHOW_MAX_RESOLUTION
		if ((w*h) > (SLIDESHOW_MAX_RESOLUTION*0x100000))
    {
    	MP_ALERT("%s: SLIDESHOW skip large JPEG!!", __FUNCTION__);
      return FAIL;
    }
#endif
#ifdef SLIDESHOW_MAX_JPEG_WIDTH
#ifdef SLIDESHOW_MAX_JPEG_HEIGHT
    if((w > SLIDESHOW_MAX_JPEG_WIDTH) || (h > SLIDESHOW_MAX_JPEG_HEIGHT))
    {
   		if (psJpeg->blSpecialSizeFlag || psJpeg->blNintyDegreeFlag)
      {
      	MP_ALERT("%s: SLIDESHOW skip 422v JPEG!!", __FUNCTION__);
        return FAIL;
      }
    }
#endif
#endif
	}

    dwChasingBuffer=16*1024*30+512;  //reserve for chasing buffer 16k*22 +512
    dwRotateBuffer=((ImgWin->wWidth *2)*(ImgWin->wHeight+16))+512; //reserve for rotate buffer

    if (g_bAniFlag & ANI_SLIDE)
    {
    	dwFreeSpace = ext_mem_get_free_space() - dwChasingBuffer;

#if ( SET_RATIO_BY_BUF_WIDTH )
      extern BYTE g_bRatioByTrgBufWidthFlag;
      if(g_bRatioByTrgBufWidthFlag == 1) /*Open Smart Copy*/
      {
	    MP_DEBUG1("1-Original dwFreeSpace =%d",dwFreeSpace);
		dwFreeSpace = ( dwFreeSpace / 2 ) - 512 ;//reserve to Smart copy to clear the garbage of decoded fuffer.
		MP_DEBUG("1-After Share to clear garbage");
		MP_DEBUG1("1-After dwFreeSpace =%d", dwFreeSpace);
      }
#endif		

    }
    else
    {
      dwReserveBuffer=(dwChasingBuffer>dwRotateBuffer)?dwChasingBuffer:dwRotateBuffer;
      dwFreeSpace = ext_mem_get_free_space() - dwReserveBuffer;

#if ( SET_RATIO_BY_BUF_WIDTH )
	  extern BYTE g_bRatioByTrgBufWidthFlag;
	  if(g_bRatioByTrgBufWidthFlag == 1) /*Open Smart Copy*/
	  {
	    MP_DEBUG1("2-Original dwFreeSpace =%d",dwFreeSpace);
		dwFreeSpace = ( dwFreeSpace / 2 ) - 512 ;//reserve to Smart copy to clear the garbage of decoded fuffer.
		MP_DEBUG("2-After Share to clear garbage");
		MP_DEBUG1("2-After dwFreeSpace =%d", dwFreeSpace);
	  }
#endif
 
    }

	  psImage->bScaleDown = ImageFile_CheckSize(w ,h , dwFreeSpace,0);

		MP_DEBUG("psImage->bScaleDown is %d", psImage->bScaleDown);

	if(psImage->bFileFormat == IMAGE_TYPE_JPEG)
	{
#if IMAGE_SW_DECODER
    if(psJpeg->bProgressive == 1 && w * h > MAX_PROGRESSIVE_RESOLUTION)
    	return FAIL;   // skip big progressive jpeg
#else
    if(psJpeg->bProgressive == 1)
      return FAIL;
#endif
		if(psJpeg->blNintyDegreeFlag)
		{
			if(psImage->bScaleDown <3) psImage->bScaleDown++;
			if(psImage->bScaleDown ==3) psImage->bScaleDown =2;
			if(psImage->bScaleDown >3) return FAIL;
		}

		MP_DEBUG("psJpeg->blNintyDegreeFlag is %x", psJpeg->blNintyDegreeFlag);
		MP_DEBUG("psJpeg->bProgressive is %x", psJpeg->bProgressive);

		if (psJpeg->blNintyDegreeFlag || sImagePlayer->dwZoomInitFlag)
	    	psImage->bScaleDown = ImageFile_CheckSize(w , h , dwFreeSpace,1);

		if (psJpeg->bProgressive)
	    	psImage->bScaleDown = ImageFile_CheckSize(w , h , dwFreeSpace/2,1);

		if (((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB2)&&(!psJpeg->bProgressive)&&(!psJpeg->blNintyDegreeFlag))
	    	psImage->bScaleDown = ImageFile_CheckSize(w , h , 200*200*2,1);

		if (psImage->bScaleDown >2)
			Jpg_DC_Controller_init();
  }
  else if(psImage->bFileFormat == IMAGE_TYPE_BMP)
	{// add to decode thumbnail for .bmp
		if((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)
    	psImage->bScaleDown = ImageFile_CheckSize(w , h , 200*200*2,1);
	}

	MP_DEBUG("w>>psImage->bScaleDown is %d", w>>psImage->bScaleDown);
	MP_DEBUG("h>>psImage->bScaleDown is %d", h>>psImage->bScaleDown);
    if (psImage->bScaleDown == 8)
		return FAIL;

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	if((psImage->bFileFormat == IMAGE_TYPE_JPEG) || (psImage->bFileFormat == IMAGE_TYPE_BMP) || (psImage->bFileFormat == IMAGE_TYPE_TIFF))
	{
		if(psImage->bFileFormat == IMAGE_TYPE_JPEG && psJpeg->wImageType == 444)
#if YUV444_ENABLE
        {
		  extern BYTE JpegDecoder444;
		  if(JpegDecoder444 == 1)
			dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ALIGN_16(h>>psImage->bScaleDown)* 3);
		  else
		  	dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ALIGN_16(h>>psImage->bScaleDown)* 2);
        }
#else
			dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ALIGN_16(h>>psImage->bScaleDown)* 2);
#endif




		else
			dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ALIGN_16(h>>psImage->bScaleDown)* 2);
		if(psJpeg->bCMYK == 1)
		{
			 dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ((h>>psImage->bScaleDown))* 4) + 16384;
			 MP_DEBUG("CMYK cal memory %d", (ALIGN_16(w>>psImage->bScaleDown) * ((h>>psImage->bScaleDown))* 4));
		}
	}
#else		// not 650
	if ((psImage->bFileFormat == IMAGE_TYPE_JPEG) || (psImage->bFileFormat == IMAGE_TYPE_BMP) || (psImage->bFileFormat == IMAGE_TYPE_TIFF))
        dwSizeLimit = (ALIGN_16(w>>psImage->bScaleDown) * ((h>>psImage->bScaleDown)+4 )* 2) + 16384;
#endif

#if GIF
    else if (psImage->bFileFormat == IMAGE_TYPE_GIF)
    {
        dwSizeLimit = ALIGN_16(w) * h * 3 + 0x10000;

        MP_DEBUG("dwSizeLimit=%d, dwFreeSpace = %d", dwSizeLimit, dwFreeSpace);
        if(dwSizeLimit > MAX_GIF_RESOLUTION || dwSizeLimit > dwFreeSpace)
            return FAIL;
    }
#endif
#if PNG
    else if (psImage->bFileFormat == IMAGE_TYPE_PNG)
    {
        dwSizeLimit = ALIGN_16(w) * h * 4 + 0x10000;

        MP_DEBUG("dwSizeLimit=%d, dwFreeSpace = %d", dwSizeLimit, dwFreeSpace);
        if(dwSizeLimit > MAX_PNG_RESOLUTION || dwSizeLimit > dwFreeSpace)
            return FAIL;
    }
#endif

//   	if (((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)) dwSizeLimit += 0x10000;
    psImage->pbTarget =NULL;

	if (iErrorCode == PASS)
  {
		if (((psImage->bNityDegree))||(((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)&&(psImage->bFileFormat == IMAGE_TYPE_JPEG)))
		{
			dwSizeLimit = dwSizeLimit + 0x10000;
			psImage->pbNityDegreeTarget = (BYTE *)ImageAllocTargetBuffer(dwSizeLimit);
			if(psImage->pbNityDegreeTarget == NULL)
			{
				MP_DEBUG("NityDegreeTarget is NULL");
				iErrorCode = ERR_MEM_MALLOC;

			}
			if (iErrorCode == PASS)
			{
    			psImage->pbTarget = (BYTE *)((DWORD)psImage->pbNityDegreeTarget + 0x10000);
    			psImage->dwTargetSize = dwSizeLimit - 0x10000;

    			MP_DEBUG("NityDegreeTarget = 0x%08x", psImage->pbNityDegreeTarget);
    			MP_DEBUG("Target = 0x%08x", psImage->pbTarget);
    			MP_DEBUG("SizeLimit = 0x%08x", dwSizeLimit);
    			MP_DEBUG("TargetSize = 0x%08x", psImage->dwTargetSize);
		  }
		}
		else
		{
			if(dwSizeLimit > dwFreeSpace)
				MP_ALERT("%s: Resize ratio doesn't meet free memory size", __FUNCTION__);
			psImage->pbTarget = (BYTE *)ImageAllocTargetBuffer((dwSizeLimit < dwFreeSpace) ? dwSizeLimit : dwFreeSpace );
			psImage->dwTargetSize = ImageGetTargetSize();
			MP_DEBUG("Target size is %d", psImage->dwTargetSize);
		}
	}

	if (psImage->pbTarget == NULL) {
		MP_ALERT("%s: image target == NULL", __FUNCTION__);
		iErrorCode = ERR_MEM_MALLOC;
	}
    MP_DEBUG("Target address is %x", psImage->pbTarget);
    MP_DEBUG("(psImage->bDecodeMode & 0x3f) is %x",(psImage->bDecodeMode & 0x3f));

	if (iErrorCode == PASS) //image decode function entry
	{
		if ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)
			iErrorCode = ImageFile_DecodeThumb(psImage);
		else
			iErrorCode = ImageFile_DecodeImage(psImage);
	}
       

#if ( SET_RATIO_BY_BUF_WIDTH )

	    extern BYTE g_bRatioByTrgBufWidthFlag;
    	extern int  g_Retresult;
		BYTE *BufInPtr = NULL;
		WORD wSrcWidth;
		WORD wSrcHeight;
		WORD wGetwImageType;
		WORD wGetImgWidth;
		WORD wGetImgHeight;

		WORD dwLineCut = 0;
		WORD dwHeightCut = 0;
		BYTE *AfterDecBufAddr = NULL;
		BYTE *ClearImgBufAddr = NULL;

		int ret_check;
		WORD SrcBufWidth ;
		WORD SrcBufHeight;
		
		if((g_bRatioByTrgBufWidthFlag == 1) && (iErrorCode == PASS))
		{
          dwLineCut   = (psImage->wTargetWidth ) - (psImage->wRealTargetWidth);
          dwHeightCut = (psImage->wTargetHeight) - (psImage->wRealTargetHeight);
		  if(((dwLineCut > 0) || (dwHeightCut > 0)) && (psImage->bFileFormat == IMAGE_TYPE_BMP)) /*Only BMP need to check*/
		  {    /*------------- NEED cut the garbage------------------*/
			  MP_DEBUG1("%s: Find garbage Need to Copy to NEW Buffer", __FUNCTION__);
			  AfterDecBufAddr = psImage->pbTarget;
              ClearImgBufAddr = (BYTE *) ext_mem_malloc((psImage->wTargetWidth) * (psImage->wTargetHeight) * 2);/*Only support 422 Type*/
			 
              if(ClearImgBufAddr == NULL)
              {
			  	MP_ALERT("%s:Allocte memory FAIL.", __FUNCTION__);
			  	goto SmarCopy_Error;
              }

			  SrcBufWidth = psImage->wTargetWidth;
              SrcBufHeight = psImage->wTargetHeight;

			  //MP_ALERT("---------------wGetwImageType=%d",wGetwImageType);
			  ret_check = Cut_SrcBuf_to_TrgBuf(ClearImgBufAddr, AfterDecBufAddr, SrcBufWidth, SrcBufHeight , dwLineCut , dwHeightCut); /*image type422 */
			  if(ret_check == FAIL)
			  {
			 	MP_ALERT("%s: Cut_SrcBuf_to_TrgBuf FAIL", __FUNCTION__);
				goto SmarCopy_Error;
			  }

			  /*Need mofify for new information*/
              BufInPtr       = ClearImgBufAddr/*psImage->pbTarget*/;
			  wSrcWidth      = SrcBufWidth  - dwLineCut;//psImage->wRealTargetWidth;
			  wSrcHeight     = SrcBufHeight - dwHeightCut;//psImage->wTargetHeight;//psImage->wRealTargetHeight;
			  wGetwImageType = 422;/*psJpeg->wImageType*/
			  wGetImgWidth   = psImage->wImageWidth;
			  wGetImgHeight  = psImage->wImageHeight;
			
		  }
		  else  /*------------- Not need cut the garbage------------------*/
		  {		
              BufInPtr       = psImage->pbTarget;
			  wSrcWidth      = psImage->wTargetWidth;//psImage->wRealTargetWidth;
			  wSrcHeight     = psImage->wTargetHeight;//psImage->wRealTargetHeight;
			  wGetwImageType = 422;/*psJpeg->wImageType*/
			  wGetImgWidth   = psImage->wImageWidth;
			  wGetImgHeight  = psImage->wImageHeight;
		  }
		  MP_DEBUG4("1--wSrcWidth=%d,wSrcHeight=%d,wGetImgWidth=%d,wGetImgHeight=%d",wSrcWidth,wSrcHeight,wGetImgWidth,wGetImgHeight);
          g_Retresult    = GetResizeTrgBuf(BufInPtr, wSrcWidth, wSrcHeight, wGetImgWidth, wGetImgHeight, wGetwImageType);

		}
		else
		{
SmarCopy_Error:			
			MP_DEBUG1("%s: Image decode FAIL!!",__FUNCTION__);
			g_Retresult = FAIL;
		}
		//g_bRatioByTrgBufWidthFlag = 0; //close flag

	    if(ClearImgBufAddr != NULL)
	    {
			ext_mem_free(ClearImgBufAddr );
			ClearImgBufAddr = NULL;
	    }
#endif

	
	if (iErrorCode)
	{
		ImageReleaseTargetBuffer();
	}
	ImageFile_Close(psImage);
	Jpg_DC_Controller_Free();

    MP_DEBUG1("--------Cur IMG type=%d",GetCurImgOutputType());//franklin temp

	if(g_wDecodedImageType == 444)
    {
	  MP_DEBUG1("%s: JPEG decode, Decoded buffer is 444.", __FUNCTION__);
	  psJpeg->wImageType = 444;
    }
	else
	{
	  MP_DEBUG1("%s: JPEG decode, Decoded buffer is 422.", __FUNCTION__);
	  psJpeg->wImageType = 422;
	}
		
	if (iErrorCode != 0)
		MP_DEBUG1("iErrorCode %08x", iErrorCode);
	return iErrorCode;
}


///
///@ingroup ImageDecoder
///@brief Decode given image file
///
///@param BYTE		  *pbSource 		: Source buffer (From FFD8 to FFD9)
///@param	BYTE		  *pbTarget 		: Target buffer
///@param	BYTE			bMode					: The decode option defined in FlagDefine.h
///@param	DWORD			dwSourceSize	: The source buffer size
///@param	DWORD			dwTargetSize	: The target size
///
///@retval 0:Success / Others:Fail
///
///@remark  If the target size less than original image size, the output size would be resize. \n
///
int ImageFile_DecodeFromBuffer(BYTE * pbSource, BYTE * pbTarget, BYTE bMode, DWORD dwSourceSize, DWORD dwTargetSize)
{
		int ret ;
		ret = Img_Jpeg2ImgBuf(pbSource, pbTarget, bMode, dwSourceSize, dwTargetSize);
		return ret ;
}

///
///@ingroup ImageEncoder
///@brief Encode to JPEG file
///
///@param BYTE		  	*pbTrgBuffer 	: Target buffer (make sure this buffer is enough) \n
///@param	ST_IMGWIN		*pWin 				: Source win \n
///
///@retval  : The return value indicate the size of encoded file \n
///

DWORD ImageFile_Encode_Img2Jpeg(BYTE *pbTrgBuffer, ST_IMGWIN *pWin)
{
	return ImageFile_Encode_Img2Jpeg_WithQT(pbTrgBuffer, pWin, 4 );/*4 for good quality video. Default 7*//*need check Standard_QT_NUM = 7*/
}


DWORD ImageFile_Encode_Img2Jpeg_WithQT(BYTE *pbTrgBuffer, ST_IMGWIN *pWin, BYTE bQualityTable)
{
	DWORD size = 0;

	size = Img2Jpeg_WithQTable(pbTrgBuffer, pWin,bQualityTable);
	if(size <= (JPEG_HEADER_LENGTH + 1))
	{
		MP_ALERT("%s: Encode Jpeg Fail", __FUNCTION__);
		return 0;
	}

	return size;
}



/*-----------------------------------------------------------------*/
#if OPEN_EXIF

/*----Add by Frank Lin 20100916----*/
/*API description: Extract EXIF form FileA. Encode JPEG with this EXIF  into FileB           */
/*Input parameter:                                                                                                  */
/*         psHandleA= the HANDLE of FileA with EXIF information                                    */
/*         psHandleB= the HANDLE of FileB will be encoded.                                            */
/*         wWidthEnc= the WIDTH  of the input IMAGE buffer                                           */
/*         wHeightEnc=  the Heitht  of the input IMAGE buffer                                           */
/*         pbSourceEnc=  the opinter of the input IMAGE buffer                                        */
/*Return : PASS= success. Extract EXIF from FileA ,and encode JPEG with EXIF into FileB*/
/*             */
int ImageFile_EncodeJpeg_ExtractAExifToB(STREAM *psHandleA, STREAM *psHandleB, WORD wWidthEnc, WORD wHeightEnc, BYTE *pbSourceEnc)
{
	return ImageFile_EncodeJpeg_ExtractAExifToB_WithQT(psHandleA, psHandleB, wWidthEnc, wHeightEnc, pbSourceEnc,7);/*need check Standard_QT_NUM = 7*/		
}

int ImageFile_EncodeJpeg_ExtractAExifToB_WithQT(STREAM *psHandleA, STREAM *psHandleB, WORD wWidthEnc, WORD wHeightEnc, BYTE *pbSourceEnc, BYTE bQualityTable)
{
  BYTE *pbSource;
  DWORD dwSourceSize;
  int iErrorCode=PASS;
  static IMAGEFILE stImageFileEnc;
  IMAGEFILE *psImageFile = &stImageFileEnc;

  BYTE bMode=IMG_DECODE_PHOTO;

  extern ST_EXIF *g_psEXIFEnc;

  static int ret;

  ImageReleaseAllBuffer();
  if(psHandleA == NULL) 
  	{
  	  MP_ALERT("%s: (psHandleA == NULL) !!", __FUNCTION__);
  	  return FAIL;
  	}

  if(psHandleB == NULL) 
  	{
  	  MP_ALERT("%s: (psHandleB == NULL) !!", __FUNCTION__);
  	  return FAIL;
  	}

  if(g_psEXIFEnc==NULL)
  	{
  	  MP_ALERT("%s: (g_psEXIFEnc == NULL) !!", __FUNCTION__);
  	  return FAIL;
  	}

  dwSourceSize = 256 * 1024;
  pbSource = ImageAllocSourceBuffer(dwSourceSize + 32); 

  if(pbSource != NULL)
    {
      iErrorCode=ImageFile_Open(psImageFile, psHandleA, pbSource, dwSourceSize, bMode);
    }
  
  ImageReleaseSourceBuffer();

  if(iErrorCode!=PASS)
  {
    MP_ALERT("%s: Extract EXIF Fail", __FUNCTION__);
    return iErrorCode;
  }

/*---Frank Lin for Debug---------------------------------------*/
#if 0
DWORD fi;
for(fi=0;fi<g_psEXIFEnc->NoOfZeroIFD;fi++)
{
  MP_DEBUG1("-----i=%d-----",fi);
  MP_DEBUG1("g_psEXIFEnc->IFD0Tag=0x%4x",g_psEXIFEnc->IFD0Tag[fi]);
  MP_DEBUG1("g_psEXIFEnc->IFD0Type=0x%4x",g_psEXIFEnc->IFD0Type[fi]);
  MP_DEBUG1("g_psEXIFEnc->IFD0Type=0x%s",g_psEXIFEnc->IFD0String[fi]);
}

for(fi=0;fi<g_psEXIFEnc->NoOfSubIFD;fi++)
{
  MP_DEBUG1("-----i=%d-----",fi);
  MP_DEBUG1("g_psEXIFEnc->SubIFDTag=0x%4x",g_psEXIFEnc->SubIFDTag[fi]);
  MP_DEBUG1("g_psEXIFEnc->SubIFDType=0x%4x",g_psEXIFEnc->SubIFDType[fi]);
  MP_DEBUG1("g_psEXIFEnc->SubIFDString=0x%s",g_psEXIFEnc->SubIFDString[fi]);
}

MP_DEBUG("Frank Lin Here g_psEXIFEnc->NoOfZeroIFD=%d",g_psEXIFEnc->NoOfZeroIFD);
#endif

/*----------get EXIF information  here.----------------------------------*/
/*Start to Encode with EXIF*/
 
   BYTE *JpegBuf = (BYTE*)ext_mem_malloc(wWidthEnc * wHeightEnc * 2 + 4096);
   DWORD IMG_size;
   ST_IMGWIN MPX_Win;

   if(JpegBuf==NULL)
   {
      MP_ALERT("%s: JpegBuf is NULL", __FUNCTION__);
	  iErrorCode=FAIL;
   }
   
   MPX_Win.pdwStart=pbSourceEnc;
   MPX_Win.wWidth=wWidthEnc;
   MPX_Win.wHeight=wHeightEnc;
   MPX_Win.dwOffset=MPX_Win.wWidth<<1;
	
   if(JpegBuf!=NULL)
   	{
       IMG_size=ImageFile_Encode_Img2JpegWithEXIF_WithQT(JpegBuf, &MPX_Win, g_psEXIFEnc,bQualityTable);
       if(IMG_size==0)
   	   iErrorCode=FAIL;
   	}

   /* save file*/
   ret = FileWrite(psHandleB, JpegBuf, IMG_size);
   if (!ret) UartOutText("?????write file fail\r\n");

   ext_mem_free(JpegBuf);
 
   return iErrorCode;
}


DWORD ImageFile_Encode_Img2JpegWithEXIF(BYTE *pbTrgBuffer, ST_IMGWIN *pWin, ST_EXIF *psEXIF)
{
	return ImageFile_Encode_Img2JpegWithEXIF_WithQT(pbTrgBuffer, pWin, psEXIF, 7);/*need check Standard_QT_NUM = 7*/
}


/*----Add by Frank Lin 20100916----*/
DWORD ImageFile_Encode_Img2JpegWithEXIF_WithQT(BYTE *pbTrgBuffer, ST_IMGWIN *pWin, ST_EXIF *psEXIF, BYTE bQualityTable)
{
	DWORD size = 0;

    MP_DEBUG("--ImageFile_Encode_Img2JpegWithEXIF--");
	
	size = Img2JpegWithEXIF_WithQTable(pbTrgBuffer, pWin, psEXIF, bQualityTable);

	if(size <= (JPEG_HEADER_LENGTH + 1))
	{
		MP_ALERT("%s: Encode Jpeg Fail", __FUNCTION__);
		return 0;
	}

	return size;
}



/*----Add by Frank Lin 20100916----*/
/*API description: Get EXIF Date Time from the input file(JPEG file)           */
/*Input parameter: handle=input file                                                      */
/*                         exif_date_time=EXIF date time                                  */
/*Return : PASS= success. Get EXIF Date Time from the input file.           */
/*             FAIL = not found  EXIF Tag of Date Time                                 */

int Get_EXIF_DateTime_Info(STREAM *handle, DATE_TIME_INFO_TYPE  *exif_date_time)
{
  /*DateTimeOriginal= SubIFD 0x9003*/
  
   ST_EXIF *psEXIF=NULL;
   psEXIF=(ST_EXIF *)ext_mem_malloc(sizeof(ST_EXIF)+32);
   extern ST_EXIF *g_psEXIFEnc;
   BYTE *pbSource;
   DWORD dwSourceSize;
   int ret;
   int iErrorCode;
   static IMAGEFILE stImageFileEnc;
   IMAGEFILE *psImageFile = &stImageFileEnc;
   BYTE bMode=IMG_DECODE_PHOTO;
   DWORD fi;
   //BYTE EXIFTimeString[20];
   //WORD EXIFTagTemp;
   BYTE *GetChar;
   BYTE FindEXIFTime=0;/*0: do not find it, 1:find the TIME Tag*/
   WORD WortTemp;
   BYTE ByteTemp;
   BYTE GetValue;

   iErrorCode=PASS;
   if(handle==NULL)
   {
     MP_ALERT("%s: File handle is NULL.", __FUNCTION__);
	 iErrorCode=FAIL;
	 return iErrorCode;
   }

   if(exif_date_time==NULL)
   {
     MP_ALERT("%s: exif_date_time is NULL.", __FUNCTION__);
	 iErrorCode=FAIL;
	 return iErrorCode;
   }


   dwSourceSize = 256 * 1024;
   pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
   
   g_psEXIFEnc=psEXIF;/*NEED!! and NEED to set NULL later. Set the EXIF structure pointer*/
   memset(psEXIF, 0, sizeof(ST_EXIF));



   if(pbSource != NULL)
    {
      ret=ImageFile_Open(psImageFile, handle, pbSource, dwSourceSize, bMode);
	  if(ret!=PASS)
	  	iErrorCode=FAIL;
    }

   FindEXIFTime=0;
   if(psImageFile->bFileFormat==IMAGE_TYPE_JPEG)
   	{
       for(fi=0;fi<psEXIF->NoOfSubIFD;fi++)
   	   {
   	     WortTemp=psEXIF->SubIFDTag[fi];
   	     if(WortTemp==0x9003)/*DateTimeOriginal TAG=0x9003*/
   	  	 {
		   GetChar=psEXIF->SubIFDString[fi];
		   FindEXIFTime=1;
		   break;
   	  	 }
   	   }
   	}

       /*EXIT String="YYYY:MM:DD HH:MMSS"+0x00 total 20 bytes"*/
       WortTemp=0;
	   ByteTemp=0;
       if(FindEXIFTime==1)/*Find it*/
	   {
         /*year*/
		 GetValue=*GetChar;
		if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
         WortTemp =(WORD) ((GetValue-48)*1000);
		 GetValue=*(GetChar+1);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
    	 WortTemp+=(WORD) ((GetValue-48)*100);
		 GetValue=*(GetChar+2);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     WortTemp+=(WORD) ((GetValue-48)*10);
		 GetValue=*(GetChar+3);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     WortTemp+=(WORD) ( GetValue-48);
	     exif_date_time->year=WortTemp;

         /*month*/
		 GetValue=*(GetChar+5);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp =(BYTE) ((GetValue-48)*10);
		 GetValue=*(GetChar+6);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp+=(BYTE) ( GetValue-48);
		 if(ByteTemp<=0 || ByteTemp>12)
		 {
		 	MP_DEBUG1("%s: Value of Month, out of range", __FUNCTION__);
	        iErrorCode=FAIL;
		 }
	     exif_date_time->month=ByteTemp;

	     /*day*/
		 GetValue=*(GetChar+8);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp =(BYTE) ((GetValue-48)*10);
		 GetValue=*(GetChar+9);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp+=(BYTE) ( GetValue-48);
		 if(ByteTemp<=0 || ByteTemp>31)
		 {
		 	MP_DEBUG1("%s: Value of Day, out of range", __FUNCTION__);
	        iErrorCode=FAIL;
		 }
   	     exif_date_time->day=ByteTemp;

         /*hour*/
		 GetValue=*(GetChar+11);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp =(BYTE) ((GetValue-48)*10);
		 GetValue=*(GetChar+12);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp+=(BYTE) ( GetValue-48);
   	     exif_date_time->hour=ByteTemp;
		 if(ByteTemp<0 || ByteTemp>=24)
		 {
		 	MP_DEBUG1("%s: Value of hour, out of range", __FUNCTION__);
	        iErrorCode=FAIL;
		 }


	     /*minute*/
		 GetValue=*(GetChar+14);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp =(BYTE) ((GetValue-48)*10);
		 GetValue=*(GetChar+15);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp+=(BYTE) ( GetValue-48);
		 if(ByteTemp<0 || ByteTemp>=60)
		 {
		 	MP_DEBUG1("%s: Value of minute, out of range", __FUNCTION__);
	        iErrorCode=FAIL;
		 }
   	     exif_date_time->minute=ByteTemp;

	     /*second*/
		 GetValue=*(GetChar+17);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp =(BYTE) ((GetValue-48)*10);
		 GetValue=*(GetChar+18);
		 if(!IsDigit08(GetValue)) 
		 	{
				iErrorCode=FAIL;
			    goto Lable_EndEXIF;
			}
	     ByteTemp+=(BYTE) (GetValue-48);
		 if(ByteTemp<0 || ByteTemp>=60)
		 {
		 	MP_DEBUG1("%s: Value of second, out of range", __FUNCTION__);
	        iErrorCode=FAIL;
		 }
	     exif_date_time->second=ByteTemp;

	     exif_date_time->UtcOffset=0;

	     /*for Debug*/
	     #if 0
	     MP_DEBUG1("%s: Find EXIF time.", __FUNCTION__);
	     MP_DEBUG("-----Get EXIF time -------");
         MP_DEBUG3("Year=%d,Month=%d,Day=%d",exif_date_time->year,exif_date_time->month,exif_date_time->day);
         MP_DEBUG3("Hour=%d,Minute=%d,Second=%d",exif_date_time->hour,exif_date_time->minute,exif_date_time->second);  
	     #endif
	     
	}
	else
	{
	  MP_DEBUG1("%s: Do not find EXIF time.", __FUNCTION__);
	  iErrorCode=FAIL;
	}
	
Lable_EndEXIF:
	
    g_psEXIFEnc=NULL;/*NEED!! Set g_psEXIFEnc=NULL before release pxEXIF*/
    ext_mem_free(psEXIF);
    ImageReleaseSourceBuffer();

   return iErrorCode;
}


/*Get EXIF Info from memory*/
int Get_EXIF_Info_for_UI(BYTE *pbSource, DWORD dwSourceSize, FILE_EXIF_INFO_TYPE  *stFile_EXIF_Info)
{
    mpDebugPrint("%s: pbSource = 0x%X, dwSourceSize = %d", __func__, pbSource, dwSourceSize);
    int iErrorCode=PASS;
    BYTE bMode = IMG_DECODE_PHOTO;
    static IMAGEFILE stImageFileEXIF;
    IMAGEFILE *psImageFile = &stImageFileEXIF;
    int ret;
 
    extern ST_EXIF *g_psEXIFEnc;
    extern FILE_EXIF_INFO_TYPE *g_psEXIF_Info_ForUI;
    
    if(stFile_EXIF_Info == NULL)
    {
      MP_ALERT("%s: stFile_EXIF_Info is NULL.", __FUNCTION__);
      iErrorCode=FAIL;
      return iErrorCode;
    }
    
    if(pbSource == NULL)
    {
        MP_ALERT("%s: handle is NULL!!", __FUNCTION__);
        MP_ALERT("%s: pbSource is NULL!!", __FUNCTION__);
        iErrorCode=FAIL;
        return iErrorCode;
    }
 
    memset(stFile_EXIF_Info, 0, sizeof(FILE_EXIF_INFO_TYPE));
 
    g_psEXIFEnc = NULL;
    g_psEXIF_Info_ForUI = stFile_EXIF_Info; //not NULL 
 
    STREAM *handle=NULL;
    ret=ImageFile_Open(psImageFile, handle, pbSource, dwSourceSize, bMode);
    if(ret != PASS)
    {
        MP_ALERT("ImageFile_Open fail");
        iErrorCode = FAIL;
    }
    else
    { 
        MP_DEBUG1("wImageWidth = %d", psImageFile->wImageWidth);
        stFile_EXIF_Info->ImageWidth = psImageFile->wImageWidth;
        MP_DEBUG1("wImageHeight = %d", psImageFile->wImageHeight);
        stFile_EXIF_Info->ImageHeight= psImageFile->wImageHeight;
    }
 
    g_psEXIFEnc = NULL;         //Make sure!! Set NULL again, this API not use.  
    g_psEXIF_Info_ForUI = NULL; //Make sure!! NEED to set NULL before EXIT.
   
    ImageFile_Close(psImageFile);   
 
    return iErrorCode;  
}


// UI get EXIF Info.

int Get_EXIF_Info(STREAM *handle, FILE_EXIF_INFO_TYPE  *stFile_EXIF_Info)
{
    MP_DEBUG1("--------------- %s ------------- ", __FUNCTION__);
	MP_DEBUG1("---- free memory = %d", mem_get_free_space_total());
  
    BYTE *pbSource;
	int iErrorCode;
	DWORD dwSourceSize;
	BYTE bMode = IMG_DECODE_PHOTO;
	static IMAGEFILE stImageFileEXIF;
    IMAGEFILE *psImageFile = &stImageFileEXIF;
	int ret;

	ST_MPX_STREAM *psStream = &psImageFile->sStream;

    extern ST_EXIF *g_psEXIFEnc;
    extern FILE_EXIF_INFO_TYPE *g_psEXIF_Info_ForUI;

	memset(stFile_EXIF_Info, 0, sizeof(FILE_EXIF_INFO_TYPE));
   

    iErrorCode=PASS;
   
    if(handle == NULL)
    {
      MP_ALERT("%s: File handle is NULL.", __FUNCTION__);
	  iErrorCode=FAIL;
	  return iErrorCode;
    }

    if(stFile_EXIF_Info == NULL)
    {
      MP_ALERT("%s: stFile_EXIF_Info is NULL.", __FUNCTION__);
	  iErrorCode=FAIL;
	  return iErrorCode;
    }

    dwSourceSize = 64 * 1024; /*Parse JPEG header , set buffer = a64K bytes */
    pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
   
    MP_ALERT("%s-----2",__FUNCTION__);
    if(pbSource != NULL)
    {

      g_psEXIFEnc = NULL;
      g_psEXIF_Info_ForUI = stFile_EXIF_Info; //not NULL 

      ret=ImageFile_Open(psImageFile, handle, pbSource, dwSourceSize, bMode);
	  if(ret != PASS)
	  {
	    MP_ALERT("ImageFile_Open fail");
	   iErrorCode = FAIL;
	    goto Lable_EndEXIF;
	  }

      MP_DEBUG1("wImageWidth = %d", psImageFile->wImageWidth);
	  stFile_EXIF_Info->ImageWidth = psImageFile->wImageWidth;
      MP_DEBUG1("wImageHeight = %d", psImageFile->wImageHeight);
      stFile_EXIF_Info->ImageHeight= psImageFile->wImageHeight;
    }
	
Lable_EndEXIF:
   g_psEXIFEnc = NULL;         //Make sure!! Set NULL again, this API not use.  
   g_psEXIF_Info_ForUI = NULL; //Make sure!! NEED to set NULL before EXIT.
   
   ImageFile_Close(psImageFile);   
   ImageReleaseSourceBuffer();

   return iErrorCode;  
}


#endif //OPEN_EXIF
/*-------------------------------------------------------------------------------------*/




void *ImageAllocSourceBuffer(DWORD dwSize)
{
    if (pb_SourceBuffer)
        ext_mem_free(pb_SourceBuffer);

    dwSourceSize = dwSize;
    pb_SourceBuffer = (BYTE *) ext_mem_malloc(dwSize);

    if (pb_SourceBuffer == NULL)
    {
        MP_ALERT("%s: alloc source buffer fail", __FUNCTION__);
    }

    return pb_SourceBuffer;
}


void ImageReleaseSourceBuffer()
{
    if (pb_SourceBuffer) ext_mem_free(pb_SourceBuffer);
    pb_SourceBuffer = NULL;
}


void *ImageAllocTargetBuffer(DWORD dwSize)
{
    dwTargetSize = dwSize;
    pb_TargetBuffer = (BYTE *) ext_mem_malloc(dwSize);

    if (pb_TargetBuffer == NULL)
    {
        MP_ALERT("%s: alloc target buffer fail", __FUNCTION__);
    }

    return pb_TargetBuffer;
}


void ImageReleaseTargetBuffer()
{
    if (pb_TargetBuffer) ext_mem_free(pb_TargetBuffer);
    pb_TargetBuffer = NULL;
}


void ImageReleaseAllBuffer()
{
    if (pb_SourceBuffer) ext_mem_free(pb_SourceBuffer);
    if (pb_TargetBuffer) ext_mem_free(pb_TargetBuffer);

    pb_SourceBuffer = NULL;
    pb_TargetBuffer = NULL;

    if ( (ext_mem_check() == FAIL) || (ext_mem_get_free_space() < (2 * 1024 * 1024)) )
        mpDebugPrint("ext_mem_check failed in ImageReleaseAllBuffer");
}


void ImageResetAllBuffer()
{
    pb_SourceBuffer = NULL;
    pb_TargetBuffer = NULL;
}


void *ImageGetTargetBuffer()
{
    return pb_TargetBuffer;
}


void *ImageGetSourceBuffer()
{
    return pb_SourceBuffer;
}


DWORD ImageGetTargetSize()
{
    return dwTargetSize;
}


DWORD ImageGetSourceSize()
{
    return dwSourceSize;
}

//--------------------------------------------------------------------------------------------

// For USB -> Printer send Thumb

//BYTE Check_Thumb_exist(ST_MPX_STREAM *psStream, ThumbInfo *thumb)
BYTE Check_Thumb_exist(STREAM * psHandle, ThumbInfo *thumb)
{
		ST_MPX_STREAM *check ;
		DWORD dwJohn, dwApp1Pos, dwThumbStart, dwThumbEnd;
		DWORD dwSourceSize;
		WORD Height, Width ;
		ST_MPX_STREAM psStream;
		BYTE *pbSource;
		BYTE bMarker = 0;
		BYTE result = 1 ;
		int iErrorCode;

		dwSourceSize = 64 * 1024;
		pbSource = ImageAllocSourceBuffer(dwSourceSize + 32);
		iErrorCode = mpxStreamOpen(&psStream, psHandle, pbSource, dwSourceSize, 0, STREAM_SOURCE_FILE);
		if (iErrorCode != PASS)
			mpDebugPrint("Open file error !!!");

		check = &psStream ;
		dwJohn = mpxStreamReadWord(check);

		if (dwJohn != 0xffd8)
	  {
	  		mpDebugPrint("Not Jpeg");
	  		return result ;
	  }
	  while (bMarker != MARKER_EOI && !check->eof)
		{
				if (mpxStreamSearchMarker(check, MARKER_FIRST) == FAIL)
					break;
				do
				{
						bMarker = mpxStreamGetc(check);	// get the second byte of bMarker
				}while(bMarker == MARKER_FIRST);
				if (bMarker == 0) continue;

				switch (bMarker)
				{
						case MARKER_APP1:
								MP_DEBUG("Find MARKER_APP1");
								dwApp1Pos = mpxStreamTell(check);
								result = 0 ;
								MP_DEBUG("dwApp1Pos = 0x%08x", dwApp1Pos);
						break;
						case MARKER_SOF0:  // 0xc0
								MP_DEBUG("Find MARKER SOF0");
								mpxStreamSkip(check, 3);
								Height = mpxStreamReadWord(check);
								Width = mpxStreamReadWord(check);
								MP_DEBUG("Height=%d, Width=%d", Height, Width);
						break;
						case MARKER_SOI:
								MP_DEBUG("Find MARKER SOI");
								dwThumbStart = mpxStreamTell(check) - 2;
								MP_DEBUG("dwThumbStart = 0x%08x", dwThumbStart);
						break;
						case MARKER_EOI:
								MP_DEBUG("Find MARKER EOI");
								dwThumbEnd = mpxStreamTell(check) - 1;
								MP_DEBUG("dwThumbEnd = 0x%08x", dwThumbEnd);
						break;
						default:
							MP_DEBUG("NOT_SUPPORTED_FRAME_TYPE %x", bMarker);
				}
		}
	  MP_DEBUG("result=%d", result);
	  if(result == 0)
	  {
	  		thumb->Start = dwThumbStart ;
	  		thumb->End = dwThumbEnd ;
	  		thumb->Width = Width ;
	  		thumb->Height = Height ;
	  }
	  ImageReleaseSourceBuffer();
	  return result ;

}

//--------------------------------------------------------------------------------------------



void JpegEventInit(void)
{
	EventCreate(JPEG_LOAD_DATA_EVENT_ID1, (OS_ATTR_FIFO | OS_ATTR_WAIT_MULTIPLE | OS_ATTR_EVENT_CLEAR), 0);	
	SemaphoreCreate(JPEG_CDU_SEMA_ID, OS_ATTR_PRIORITY, 1);	
}



