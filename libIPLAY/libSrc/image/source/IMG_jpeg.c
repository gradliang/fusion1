
///
///@defgroup    IMAGE    Image
///

///
///@ingroup     IMAGE
///

///
///@defgroup    JPEG    Jpeg codec
///

/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "mpapi.h"
#include "image.h"
#include "Jpeg.h"
#include "taskid.h"

#if OUTPUT_JPEG
BOOL DUMPMEMORY = FALSE;
#endif

#define sgetc(x)    ((*((x)++)) & 0xff)
#define INTEL 0
#define MOTOROLA 1

extern ST_JPEG g_sJpeg[2];
extern ST_JPEG *g_psCurJpeg;

#if IMAGE_SW_DECODER
//int Jpg_Decode_libJpeg(IMAGEFILE *psImage);
int Libjpeg_DecodeImage(IMAGEFILE *psImage);
void LibjpegCallback_Check(IMAGEFILE *psImage, BOOL boLastScan);
#endif

// to remove libjpeg to reduce code size, just remove #include "iplay_image.h" and iplay_image folder
#if 0 //ndef JPEGLIB_H
int Jpg_Decode_libJpeg(IMAGEFILE *psImage)
{
	return FAIL;
}
#else
#if IMAGE_SW_DECODER
    #define WITH_LIB_JPEG 1
#else
		#define WITH_LIB_JPEG 0
#endif
#endif


//--------------------------------------------------------------------------------------------

static DWORD *dwCellSrcPoint;
static DWORD dwSrcOffset;
static WORD wCellSrcOffset, wCellTrgOffset;
static WORD wCellSize;
static WORD wCellCounter, wCellCount;
static WORD wJpegWidth, wJpegHeight, wDataWidth;


//--------------------------------------------------------------------------------------------

static void CopyCell(DWORD * dwpTarget)
{
	DWORD *dwpSource;
	BYTE bLine;
	WORD wCellTrgOffset1, wCellSrcOffset1; // Alex Test, 060401

	dwpSource = (DWORD *)((DWORD)dwCellSrcPoint | 0xa0000000);
	bLine = wCellSize;

	while (bLine--)
	{
		memcpy(dwpTarget, dwpSource, wCellSize << 1);
		dwpTarget += wCellTrgOffset;
		dwpSource += wCellSrcOffset;
	}

	dwCellSrcPoint += (wCellSize >> 1);
	wCellCounter--;
	if (!wCellCounter)
	{
		dwCellSrcPoint += dwSrcOffset;
		wCellCounter = wCellCount;
	}
}

//--------------------------------------------------------------------------------------------

static void ResetCellCopy(ST_JPEG *psJpeg, DWORD * dwpSource)
{
	WORD wJohn, wMary;

	wJpegWidth = psJpeg->wTargetWidth; //psJpeg->wImageWidth >> psJpeg->bJResizeRatio;
	wJpegHeight = psJpeg->wTargetHeight; //psJpeg->wImageHeight >> psJpeg->bJResizeRatio;
	wDataWidth = psJpeg->wCduWidth;
	wCellSize = psJpeg->wCellSize;

	if (wCellSize == 0) {
		MP_DEBUG("wCellSize == 0");
		wCellSize = 1;
	}

	dwCellSrcPoint = dwpSource;

	wJohn = (wCellSize >> 1) - 1;
	wMary = (wDataWidth >> 1) % wCellSize;
	if (wMary != 0)
		wMary = (wCellSize - wMary) << 1;
	wCellSrcOffset = (ALIGN_16(wDataWidth + wMary) >> 1) - wJohn;	// Alex Test, 060401
	wCellSrcOffset += (wCellSize >> 1) - 1;

	wCellTrgOffset = (ALIGN_16(wJpegWidth) >> 1) - wJohn;	// Alex Test, 060401
	wCellTrgOffset += (wCellSize >> 1) - 1;


	wCellCount = (wDataWidth + wMary) / wCellSize;	// Alex Test, 060401
	wCellCounter = wCellCount;

	dwSrcOffset = (ALIGN_16(wDataWidth) - wDataWidth) >> 1;
	dwSrcOffset += (ALIGN_16(wDataWidth + wMary) >> 1) * (wCellSize - 1);
	MP_DEBUG("src=%08x celloffset=%d offset=%d cellcount=%d ",
		(DWORD)dwCellSrcPoint, wCellSrcOffset, dwSrcOffset, wCellCount);

}

//--------------------------------------------------------------------------------------------
static void SpecialSizeRecover(ST_JPEG *psJpeg, DWORD * dwpTarget, DWORD * dwpSource)
{
		WORD height, width,width_2,height_b,width_b;
		WORD i, j, k, l, m, chroma_index;
		BYTE *sptr, *dptr, *tmpsrc, *tmptrg;
		BYTE *Ydptr, *Cbdptr, *Crdptr;
		WORD offset;
		WORD block_no_of_height, block_no_of_width, line, chroma, ratio,block_no_of_width_2,block_no_of_width_x2;
		MP_DEBUG("SpecialSizeRecover");
		MP_TRACE_TIME(SpecialSizeRecover,0);
		sptr = (BYTE *)dwpSource;
		tmpsrc = (BYTE *)((DWORD)dwpSource - 0x10000);

		dptr = tmptrg = (BYTE *)dwpTarget;

		width = ALIGN_16(psJpeg->wCduWidth) /2;
		height = psJpeg->wRealTargetHeight;
		MP_DEBUG2("width = %d, height = %d", width, height);

		block_no_of_height = 16;
		block_no_of_width = 8;
		line = 8;
		chroma = 4;

		block_no_of_height >>= psJpeg->bJResizeRatio;
		block_no_of_width >>= psJpeg->bJResizeRatio;
		line >>= psJpeg->bJResizeRatio;
		chroma >>= psJpeg->bJResizeRatio;

		block_no_of_width_2=block_no_of_width/2;
		block_no_of_width_x2=block_no_of_width*2;
		width_2=width/2;
		height_b=height/block_no_of_height;
		width_b=width/block_no_of_width;
		offset = (ALIGN_16(width) - width)*2;

		Ydptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height *2);
		if(Ydptr == NULL)	goto	SpecialSizeRecover_End;

		Cbdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
		if(Cbdptr == NULL)
			goto	SpecialSizeRecover_End;

		Crdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
		if(Crdptr == NULL)
			goto	SpecialSizeRecover_End;

		MP_DEBUG1("ratio = %d", psJpeg->bJResizeRatio);

		for(i=0; i<height_b; i++)	//i = 0, 1, 2, ...	//16, 8, 4...
		{
			for(j=0; j<line; j++)
			{
				for(k=0; k<width_b; k++)
				{
					chroma_index=0 ;
					for(l=0; l<chroma; l++)
					{
						*(Ydptr + ((j)*width+k*block_no_of_width+l*2)) = *sptr++;
						*(Ydptr + ((j)*width+k*block_no_of_width+l*2+1)) = *sptr++;
						if(chroma_index%2==0)
						{
							*(Cbdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
							*(Crdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
						}
						else
						{
							*(Cbdptr + ((j*width)+width_2+k*block_no_of_width_2+chroma_index/2)) =
							*(Cbdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
							*(Crdptr + ((j*width)+width_2+k*block_no_of_width_2+chroma_index/2)) =
							*(Crdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
						}
						chroma_index++ ;
					}
					for(l=0; l<chroma; l++)
					{
						*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2)) = *sptr++;
						*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2+1)) = *sptr++;
						if(chroma_index%2==0)
						{
							*(Cbdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
							*(Crdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
						}
						else
						{
							*(Cbdptr + ((j*width)+width_2+k*block_no_of_width_2+chroma_index/2)) =
							*(Cbdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
							*(Crdptr + ((j*width)+width_2+k*block_no_of_width_2+chroma_index/2)) =
							*(Crdptr + ((j)*width+k*block_no_of_width_2+chroma_index/2)) = *sptr++;
						}
						chroma_index++ ;
					}
				}
			}
			if (g_bAniFlag & ANI_AUDIO) TaskYield();

			//arrange 8 line, move data to target
			//## write file
			for(m=0; m<block_no_of_height; m++) //16, 8, 4...
			{
				for(j=0; j<width_2; j++)
				{
					*tmpsrc++ = *(Ydptr +(m*width+j*2));
					*tmpsrc++ = *(Ydptr + (m*width+j*2+1));
					*tmpsrc++ = *(Cbdptr + (m*width_2+j));
					*tmpsrc++ = *(Crdptr + (m*width_2+j));
				}
				tmpsrc += offset ;
			}
		}



	SpecialSizeRecover_End:
		MP_DEBUG("SpecialSizeRecover END");
		MP_TRACE_TIME(SpecialSizeRecover,0);

		if (Ydptr) ext_mem_free(Ydptr);
		if (Cbdptr) ext_mem_free(Cbdptr);
		if (Crdptr) ext_mem_free(Crdptr);
	}

#if 0
{
	WORD height, width;
	WORD i, j, k, l, m, chroma_index;
	BYTE *sptr, *dptr, *tmpsrc, *tmptrg;
	BYTE *Ydptr, *Cbdptr, *Crdptr;
	WORD offset;
	WORD block_no_of_height, block_no_of_width, line, chroma, ratio;
	MP_DEBUG("\r\nSpecialSizeRecover");
	sptr = (BYTE *)dwpSource;
	tmpsrc = (BYTE *)((DWORD)dwpSource - 0x10000);

	dptr = tmptrg = (BYTE *)dwpTarget;

	width = ALIGN_16(psJpeg->wCduWidth) /2;
	height = psJpeg->wRealTargetHeight;
	MP_DEBUG2("width = %d, height = %d", width, height);

	block_no_of_height = 16;
	block_no_of_width = 8;
	line = 8;
	chroma = 4;

	block_no_of_height >>= psJpeg->bJResizeRatio;
	block_no_of_width >>= psJpeg->bJResizeRatio;
	line >>= psJpeg->bJResizeRatio;
	chroma >>= psJpeg->bJResizeRatio;

	Ydptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height *2);
	if(Ydptr == NULL)
		MP_DEBUG("Ydptr is NULL");

	Cbdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
	if(Cbdptr == NULL)
		MP_DEBUG("Cbdptr is NULL");

	Crdptr = (BYTE *)ext_mem_malloc((width/2) * block_no_of_height);
	if(Crdptr == NULL)
		MP_DEBUG("Crdptr is NULL");

	MP_DEBUG1("ratio = %d", psJpeg->bJResizeRatio);

	for(i=0; i<height/block_no_of_height; i++)  //i = 0, 1, 2, ...  //16, 8, 4...
	{
		for(j=0; j<line; j++)
		{
			for(k=0; k<width/block_no_of_width; k++)
			{
				chroma_index=0 ;
				for(l=0; l<chroma; l++)
				{
					*(Ydptr + ((j)*width+k*block_no_of_width+l*2)) = *sptr++;
					*(Ydptr + ((j)*width+k*block_no_of_width+l*2+1)) = *sptr++;
					if(chroma_index%2==0)
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
					}
					else
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;

						*(Cbdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) =
							*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
						*(Crdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) =
							*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
					}
					chroma_index++ ;
				}
				for(l=0; l<chroma; l++)
				{
					*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2)) = *sptr++;
					*(Ydptr + ((j+line)*width+k*block_no_of_width+l*2+1)) = *sptr++;
					if(chroma_index%2==0)
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
					}
					else
					{
						*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;
						*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+chroma_index/2)) = *sptr++;

						*(Cbdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) =
							*(Cbdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
						*(Crdptr + ((j*2+1)*width/2+k*(block_no_of_width/2)+chroma_index/2)) =
							*(Crdptr + ((j*2)*width/2+k*(block_no_of_width/2)+ chroma_index/2));
					}
					chroma_index++ ;
				}
						TaskYield();

			}
					TaskYield();

		}
		TaskYield();

		//arrange 8 line, move data to target
		//## write file
		for(m=0; m<block_no_of_height; m++) //16, 8, 4...
		{
			for(j=0; j<width/2; j++)
			{
				*dptr++ = *(Ydptr +(m*width+j*2));
				*dptr++ = *(Ydptr + (m*width+j*2+1));
				*dptr++ = *(Cbdptr + (m*width/2+j));
				*dptr++ = *(Crdptr + (m*width/2+j));
			}
			TaskYield();
		}
		TaskYield();
		WORD offset = ALIGN_16(width) - width;

		for(j =0; j < block_no_of_height; j++){
			for(k = 0; k < width * 2; k++)
			{
				*tmpsrc++ = *tmptrg++;
			}
			tmpsrc += offset * 2;
			TaskYield();
		}
		dptr = tmptrg = (BYTE *)dwpTarget; //move to org trg point
		TaskYield();

	}

	ext_mem_free(Ydptr);
	ext_mem_free(Cbdptr);
	ext_mem_free(Crdptr);
}
#endif
//--------------------------------------------------------------------------------------------
static void FixSpecialImage(ST_JPEG *psJpeg, BYTE *pbTarget)
{
	BYTE *pbAllocBuffer, *pbTempBuffer;
	WORD wd, ht;
	DWORD dwSize;
	MP_DEBUG("FixSpecialImage");
	wd = ALIGN_16(psJpeg->wRealTargetWidth);
	ht = 16 >> psJpeg->bJResizeRatio;//psJpeg->wRealTargetHeight;
	dwSize = wd * ht * 2;

	ImageReleaseSourceBuffer();
	MP_DEBUG("Jpg_FixSpecialImage %d, %d", dwSize, psJpeg->dwTargetSize);

	pbAllocBuffer = (BYTE *)ext_mem_malloc(dwSize + 4096);

	if (pbAllocBuffer == NULL){
	MP_ALERT("pbAllocBuffer alloc fail");
		return;
	}
	pbTempBuffer = (BYTE *)((DWORD)pbAllocBuffer | 0xa0000000);
	memset(pbTempBuffer, 0x80, dwSize);
	pbTarget = (BYTE *)((DWORD)pbTarget | 0xa0000000);
	TaskYield();
	MP_DEBUG("pbTarget %x, pbTempBuffer %x", pbTarget, pbTempBuffer);

	SpecialSizeRecover(psJpeg, (DWORD *) pbTempBuffer, (DWORD *) pbTarget);

#if OUTPUT_JPEG
		if(DUMPMEMORY){
		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		int len = 0;
		WORD width, height;
		width = ALIGN_16(psJpeg->wCduWidth) / 2;
		height = psJpeg->wTargetHeight;
		len = width * height *2;

		UartOutText("creat jpg_422v.bin");
		UartOutText("\r\nwidth");
		UartOutValue(width, 4);
		UartOutText("\r\nheight");
		UartOutValue(height, 4);
	    sDrv=DriveGet(SD_MMC);
        ret=CreateFile(sDrv, "jpg_422v", "bin");
        if (ret) UartOutText("create file fail\r\n");
        shandle=FileOpen(sDrv);
        if(!shandle) UartOutText("open file fail\r\n");

	psJpeg->pbTarget = (BYTE *)((DWORD)psJpeg->pbTarget - 0x10000);

		ret=FileWrite(shandle, psJpeg->pbTarget, len);
        if(!ret) UartOutText("write file fail\r\n");

        FileClose(shandle);
        UartOutText("\n\rfile close\n\r");
	}
#endif

	ext_mem_free(pbAllocBuffer);
}

/*--------------------------------------------------------------------------------------------
Only support JPEG format thumbnail

If the value of Compression (0x0103) Tag in IFD1 is '6', thumbnail image format is JPEG.
Most of Exif image uses JPEG format for thumbnail.

In that case, you can get offset of thumbnail from JpegIFOffset (0x0201) Tag in IFD1,
size of thumbnail from JpegIFByteCount (0x0202) Tag. Data format is ordinary JPEG format,
starts from 0xFFD8 and ends by 0xFFD9.
--------------------------------------------------------------------------------------------*/
static inline DWORD sGetDword(BYTE *bPtr, BYTE byte_align)
{
	if (byte_align == INTEL)
		return (*(bPtr+3) << 24) | (*(bPtr+2) << 16) | (*(bPtr+1) << 8) | *(bPtr+0);
	else
		return (*bPtr << 24) | (*(bPtr+1) << 16) | (*(bPtr+2) << 8) | *(bPtr+3);
}

static inline DWORD sGetWord(BYTE *bPtr, BYTE byte_align)
{
	if (byte_align == INTEL)
		return (*(bPtr+1) << 8) | *(bPtr+0);
	else
		return (*(bPtr+0) << 8) | *(bPtr+1);
}
//--------------------------------------------------------------------------------------------
static int ExtractThumb(ST_MPX_STREAM *psStream, ST_JPEG *psJpeg, DWORD dwSearchSize)
{
	BYTE c;
	BYTE *fp;
	DWORD i, n, j;

	DWORD v, offset;
	DWORD byte_align;
	DWORD compression = FALSE;
	DWORD dwThumbPos = 0;
	DWORD JpegIFByteCount = 0xffffffff;
	BYTE *pbPtr;

	i = mpxStreamTell(psStream);

	if (dwSearchSize > psStream->buf_len - psStream->buf_pos) {
		n = mpxStreamFillCache(psStream, i, dwSearchSize);
		MP_ASSERT("n != dwSearchSize");
	}

	BYTE *pbSource = psStream->buffer + psStream->buf_pos;

	fp = pbSource;

	MP_DEBUG1("Thumb Data Len %d", dwSearchSize);
	//MP_DEBUG8("pbSource   %02x%02x%02x%02x  %02x%02x%02x%02x", fp[0],fp[1],fp[2],fp[3],fp[4],fp[5],fp[6],fp[7]);

	fp = pbSource + 6;	 // bypass EXIF header
	//MP_DEBUG8("pbSource+6 %02x%02x%02x%02x  %02x%02x%02x%02x", fp[0],fp[1],fp[2],fp[3],fp[4],fp[5],fp[6],fp[7]);

	pbPtr = fp;

	v = (fp[0] << 8) | fp[1]; fp += 2; // decide TIFF header byte alignment method
	//MP_DEBUG1("Thumb byte_align %4x", v);

	if (v == 0x4949)
	{
		byte_align = INTEL;
	}
	else if (v == 0x4D4D)
	{
		byte_align = MOTOROLA;
	}
	else
	{
		MP_DEBUG("Unsupport Byte align");
		return FAIL;
	}

	fp += 2;					// bypass remaining TIFF header
	v = sGetDword(fp, byte_align);						//  get offset to IFD0
	fp += 4;

	fp = pbPtr + v;

	n = sGetWord(fp, byte_align); fp += 2;				// get IFD0 directory entry size
	//MP_DEBUG1("Thumb IFD0 directory entry size %d", n);

	fp += (n * 12);				// bypass IFD0 directory entries

	v = sGetDword(fp, byte_align);	fp += 4;		//  get offset to IFD1
	//MP_DEBUG("IFD1 offset %d", v);

	if (v == 0) {
		MP_DEBUG("IFD1 offset == 0");
		return FAIL;
	}
	fp = pbPtr + v;	// bypass to IFD1

	//MP_DEBUG8("pbSource   %02x%02x%02x%02x  %02x%02x%02x%02x", fp[0],fp[1],fp[2],fp[3],fp[4],fp[5],fp[6],fp[7]);

	n = sGetWord(fp, byte_align);
	if (n == 514) {
		n = sGetWord(fp, byte_align);
	}
	else
	fp += 2;				// get IFD1 directory entry size
	//MP_DEBUG1("Thumb IFD1 directory entry size %d", n);

	for (i = 0; i < n; i++)
	{
		v = sGetWord(fp, byte_align); fp += 2;		// get TIFF tag

		switch (v)
		{
		case 0x0103:			// Compression
			if (byte_align == INTEL)
				compression = (*(fp + 6) == 6);
			else
				compression = (*(fp + 7) == 6);

			if (compression == 0) {
				MP_DEBUG("Thumb no compression");
				return FAIL;
			}
			fp += 10;
			break;

		case 0x0201:			// JpegIFOffset
			fp += 6;
			dwThumbPos = sGetDword(fp, byte_align);		//  get JPEGIFOffset
			fp += 4;
			//MP_DEBUG1("Thumb pos %d", dwThumbPos);

			//If the hight data in jpeg header is error, return and use original size scaling to thumbnail size
  	      	//if((DWORD)pbThumb - (DWORD)pbSource > dwSearchSize)	//32M
  	      	//	return FAIL;

  			break;

		case 0x0202:			// JpegIFByteCount
			fp += 6;
			JpegIFByteCount = sGetDword(fp, byte_align);		//  get JpegIFByteCount
			fp += 4;
			//MP_DEBUG1("Thumb byte count %d", JpegIFByteCount);
			break;

		default:
			fp += 10;
			break;
		}
	}

	if ((dwThumbPos == 0) || (JpegIFByteCount > 256 * 1024 * 1024))
	{
		//Unrecognized EXIF header!
		MP_DEBUG("Img_ExtractThumbnail Error");
		return FAIL;
	}

	//mpxStreamPrintOut(psStream, 8);

	dwThumbPos += psJpeg->dwApp1Pos + 8;
	//dwSearchSize -= dwThumbPos;
	mpxStreamSeek(psStream, dwThumbPos, SEEK_SET);
	//n = mpxStreamFillCache(psStream, dwThumbPos, JpegIFByteCount + 256);
	//mpxStreamPrintOut(psStream, 32);

	BYTE *pbTarget = psJpeg->pbTarget;
	DWORD dwTargetSize = psJpeg->dwTargetSize;

	WORD w, h;
	w = psJpeg->wImageWidth;
	h = psJpeg->wImageHeight;
	memset(psJpeg, 0, sizeof(ST_JPEG));

	psJpeg->pbTarget = pbTarget;
	psJpeg->pbSource = psStream->buffer;
	psJpeg->psStream = psStream;
	psJpeg->dwSourceSize = psStream->buf_max_size;
	psJpeg->dwTargetSize = dwTargetSize;
	psJpeg->bDecodeMode = (IMG_DECODE_THUMB1 | IMG_DECODE_CHASE);
	psJpeg->dwOffset = 0;

#if (((CHIP_VER & 0xffff0000) == CHIP_VER_650) || ((CHIP_VER & 0xffff0000) == CHIP_VER_660))
	//In new function, we can get the real end position for thumb		Griffy++
	psJpeg->iErrorCode = Jpg_Thumb_Parse_Marker(psStream, psJpeg);
#else
	psJpeg->iErrorCode = Jpg_Parse_Marker(psStream, psJpeg);
#endif

/*
	if ((psJpeg->blNintyDegreeFlag))
	{
		psJpeg->blNintyDegreeFlag = psJpeg->pbTarget;
		psJpeg->pbTarget += 0x10000;
	}
*/
	if (psJpeg->iErrorCode == PASS) {
		SemaphoreWait(JPEG_CDU_SEMA_ID);
		psJpeg->iErrorCode = Jpg_Decoder_CduDecode(psJpeg);
	    SemaphoreRelease(JPEG_CDU_SEMA_ID);
	}
	psJpeg->wThumbWidth = psJpeg->wImageWidth;
	psJpeg->wThumbHeight = psJpeg->wImageHeight;
	psJpeg->wImageWidth = w;
	psJpeg->wImageHeight = h;

	MP_DEBUG1("extract thumbnail %d", psJpeg->iErrorCode);
	return psJpeg->iErrorCode;
}

//--------------------------------------------------------------------------------------------

int Jpg_Decoder_DecodeThumb(IMAGEFILE *psImage)
{
	ST_JPEG *psJpeg = (ST_JPEG *)psImage->psJpeg;
	ST_MPX_STREAM *psStream = psImage->psStream;
	int iErrorCode = 0;
	DWORD dwJohn;
	DWORD dwEndPoint;
	BYTE bMarker;
	MP_DEBUG("Jpg_Decoder_DecodeThumb");

	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;

	if (psJpeg->dwApp1Pos == 0) {
		MP_DEBUG("no thumb");

		//psImage->bDecodeMode = IMG_DECODE_PHOTO;
		//return Jpg_Decoder_DecodeImage(psImage);
		return FAIL;
	}
	if (iErrorCode = mpxStreamSeek(psStream, psJpeg->dwApp1Pos, SEEK_SET) != PASS)
		return iErrorCode;


	psJpeg->pbTarget = psImage->pbTarget;
	psJpeg->dwTargetSize = psImage->dwTargetSize;

	dwJohn = mpxStreamReadWord(psStream);
	iErrorCode = ExtractThumb(psStream, psJpeg, dwJohn);
//jessamine 20070705 422v
#if 1
	if (iErrorCode == PASS && (psJpeg->blSpecialSizeFlag || psJpeg->blNintyDegreeFlag)) {
		MP_ALERT("Decode thumb Special size=%d or 90 degree=%d, %d X %d",
			psJpeg->blSpecialSizeFlag, psJpeg->blNintyDegreeFlag,
			psJpeg->wImageWidth, psJpeg->wImageHeight);
		MP_DEBUG2("target W %d, H %d", psJpeg->wTargetWidth, psJpeg->wTargetHeight);
		MP_DEBUG2("real target W %d, H %d", psJpeg->wRealTargetWidth, psJpeg->wRealTargetHeight);
		MP_DEBUG2("cdu W %d, H %d", psJpeg->wCduWidth, psJpeg->wCduHeight);
		FixSpecialImage(psJpeg, psJpeg->pbTarget);
	}
#else
	if(psJpeg->blNintyDegreeFlag)
		return 1;
#endif
	//psJpeg = g_psCurJpeg;
	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wRealTargetWidth = psJpeg->wRealTargetWidth;
	psImage->wRealTargetHeight = psJpeg->wRealTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;//psJpeg->wTargetWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;//psJpeg->wTargetHeight;
	psImage->bScaleDown = psJpeg->bJResizeRatio;
	MP_DEBUG("Target W %d, H %d", psImage->wTargetWidth, psImage->wTargetHeight);
	MP_DEBUG("Real W %d, H %d", psImage->wRealTargetWidth, psImage->wRealTargetHeight);
	MP_DEBUG("Thumb W %d, H %d", psImage->wThumbWidth, psImage->wThumbHeight);
	MP_DEBUG("Scale down = %d", psImage->bScaleDown);
	return iErrorCode;
}

//--------------------------------------------------------------------------------------------
int Jpg_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	ST_JPEG *psJpeg;
	MP_ASSERT(psImage != NULL);
	MP_ASSERT(psImage->psJpeg != NULL);
	MP_DEBUG("Jpg_Decoder_DecodeImage");
	psJpeg = (ST_JPEG *)psImage->psJpeg;
	psJpeg->bDecodeMode = psImage->bDecodeMode;
	psJpeg->pbTarget = psImage->pbTarget;
	psJpeg->dwTargetSize = psImage->dwTargetSize;

	SemaphoreWait(JPEG_CDU_SEMA_ID);
	int iErrorCode = Jpg_Decoder_CduDecode(psJpeg);
    SemaphoreRelease(JPEG_CDU_SEMA_ID);

	// Special size or 90 degree image
        //jessamine 20070629, decode 422v use iplay_image
	if (psJpeg->blSpecialSizeFlag || psJpeg->blNintyDegreeFlag) {
            if ((psImage->wImageWidth > 3328) && (psImage->wImageHeight >4992))
                return NOT_SUPPORTED_FRAME_SIZE;
		FixSpecialImage(psJpeg, psJpeg->pbTarget);
		MP_DEBUG2("target W %d, H %d", psJpeg->wTargetWidth, psJpeg->wTargetHeight);
		MP_DEBUG2("real target W %d, H %d", psJpeg->wRealTargetWidth, psJpeg->wRealTargetHeight);
		MP_DEBUG2("cdu W %d, H %d", psJpeg->wCduWidth, psJpeg->wCduHeight);
	}
//jessamine 20070705 422v
	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wRealTargetWidth = psJpeg->wRealTargetWidth;
	psImage->wRealTargetHeight = psJpeg->wRealTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;//psJpeg->wTargetWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;//psJpeg->wTargetHeight;
	psImage->bScaleDown = psJpeg->bJResizeRatio;
	MP_DEBUG2("target W %d, H %d", psJpeg->wTargetWidth, psJpeg->wTargetHeight);
	MP_DEBUG2("real target W %d, H %d", psJpeg->wRealTargetWidth, psJpeg->wRealTargetHeight);
	MP_DEBUG2("cdu W %d, H %d", psJpeg->wCduWidth, psJpeg->wCduHeight);
	MP_DEBUG2("Image W %d, H %d", psJpeg->wImageWidth, psJpeg->wImageHeight);
	MP_DEBUG2("Thumb W %d, H %d", psJpeg->wThumbWidth, psJpeg->wThumbHeight);
//	MP_DEBUG2("Thumbnail W %d, H %d", psJpeg->wThumbnailWidth, psJpeg->wThumbnailHeight);
	MP_DEBUG2("psJpeg->blNintyDegreeFlag %d, psJpeg->bJResizeRatio %d", psJpeg->blNintyDegreeFlag, psJpeg->bJResizeRatio);

	



	return iErrorCode;
}

//--------------------------------------------------------------------------------------------
// please refer to ISO/IEC 10918-1 Annex B for details
ST_JPEG * Jpg_Decoder_Init(ST_MPX_STREAM *psStream, BYTE * pbTarget, DWORD dwTargetSize, BYTE bMode, DWORD dwOffset)
{
	ST_JPEG *psJpeg;

	if ((bMode & 0x3f) != IMG_DECODE_XPG)// && (bMode & 0x3f) != IMG_DECODE_THUMB)
		psJpeg = (ST_JPEG *)&g_sJpeg[0];
	else
		psJpeg = (ST_JPEG *)&g_sJpeg[1];

    MP_DEBUG("%s:%d:psJpeg 0x%x &g_sJpeg[0] 0x%x &g_sJpeg[1] 0x%x", __func__, __LINE__, psJpeg, &g_sJpeg[0], &g_sJpeg[1]);
	g_psCurJpeg = psJpeg;
	//MP_DEBUG5("Jpg_Decode_Init %08x %08x %08x %d %d", pbTarget, psStream, dwTargetSize, bMode, dwOffset);

	memset(psJpeg, 0, sizeof(ST_JPEG));

	psJpeg->pbTarget = pbTarget;
	psJpeg->pbSource = psStream->buffer;
	psJpeg->psStream = psStream;
	psJpeg->dwSourceSize = psStream->buf_max_size;
	psJpeg->dwTargetSize = dwTargetSize;
	psJpeg->bDecodeMode = bMode;
	psJpeg->dwOffset = dwOffset;
	psJpeg->dwCduControl = 0;

	if ((bMode & 0x3f) != IMG_DECODE_THUMB1)
	{
        MP_DEBUG("%s:%d:psStream 0x%x", __func__, __LINE__, psStream);
        mpxStreamSeek(psStream, 0, SEEK_SET);		
	}	
	
	
	if ((psJpeg->bDecodeMode & 0x3f) != IMG_DECODE_MJPEG) 
	{
	    psJpeg->iErrorCode = Jpg_Parse_Marker(psStream, psJpeg);		
        MP_DEBUG("%s:%d:psJpeg->iErrorCode %d", __func__, __LINE__, psJpeg->iErrorCode);
	}	
	else
	{
	    psJpeg->iErrorCode = 0;	
	}
	return psJpeg;
}

//--------------------------------------------------------------------------------------------

int Jpg_Decoder_ImageFile_Init(IMAGEFILE *psImage)
{
	MP_DEBUG("Jpg_Decoder_ImageFile_Init");
	ST_JPEG *psJpeg = Jpg_Decoder_Init(psImage->psStream, psImage->pbTarget, psImage->dwTargetSize, psImage->bDecodeMode, psImage->dwDecodeOffset);
	
	if(psJpeg->iErrorCode != PASS)			// abel 20100222
		return psJpeg->iErrorCode;

	psImage->ImageDecodeImage = Jpg_Decoder_DecodeImage;
	psImage->ImageDecodeThumb = Jpg_Decoder_DecodeThumb;
	psJpeg->wTargetWidth = psJpeg->wImageWidth;



	psJpeg->wTargetHeight = psJpeg->wImageHeight;

	if (psJpeg->dwApp1Pos == 0) {
		MP_DEBUG("no app1 marker");
		if ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB) {
			psImage->bDecodeMode = IMG_DECODE_THUMB2 | IMG_DECODE_CHASE;
			psJpeg->bDecodeMode = IMG_DECODE_THUMB2 | IMG_DECODE_CHASE;
		}
		psImage->ImageDecodeThumb = Jpg_Decoder_DecodeImage;
	}

#ifdef WITH_LIB_JPEG
	if ((psJpeg->bProgressive != 0) ||(psJpeg->bCMYK==1)) 
	{
  		MP_ALERT("psJpeg->bProgressive = %d !!", psJpeg->bProgressive);
			//psImage->ImageDecodeImage = Jpg_Decode_libJpeg;
			psImage->ImageDecodeImage = Libjpeg_DecodeImage;
			psImage->LibjpegCallback = LibjpegCallback_Check;
			//psImage->bDecodeMode |= IMG_DECODE_ONEPASS ;
      if (psJpeg->bProgressive == 1)
      {
#if IMAGE_SW_DECODER
      		if (psJpeg->wTargetWidth * psJpeg->wTargetHeight > MAX_PROGRESSIVE_RESOLUTION)
          {
 						MP_ALERT("progressive jpeg out of resolution psJpeg->wTargetWidth %d, psJpeg->wTargetHeight %d",psJpeg->wTargetWidth, psJpeg->wTargetHeight);
            psJpeg->iErrorCode = IMAGE_TYPE_JPEG_PROGRESSIVE;
            return psJpeg->iErrorCode;
					}
#else
          psJpeg->iErrorCode = IMAGE_TYPE_JPEG_PROGRESSIVE;
          return psJpeg->iErrorCode;
#endif
	}
        }
	//else  if ((psJpeg->dwCduControl & VFMT_MASK) == VIDEO_444)//Jasmine 071017: 444 format, image height cut 32
  //      psJpeg->wImageHeight -= 32;//moved by AlexWang 28nov2007
#endif


	psImage->psJpeg = (void *)psJpeg;
	psImage->bNityDegree = psJpeg->blNintyDegreeFlag;
	psImage->wImageWidth = psJpeg->wImageWidth;
	psImage->wImageHeight = psJpeg->wImageHeight;
	psImage->wTargetWidth = psJpeg->wTargetWidth;
	psImage->wTargetHeight = psJpeg->wTargetHeight;
	psImage->wThumbWidth = psJpeg->wThumbWidth;
	psImage->wThumbHeight = psJpeg->wThumbHeight;
	MP_DEBUG2("image %d x %d", psImage->wImageWidth, psImage->wImageHeight);
	//MP_DEBUG2("thumb W %d, tH %d", psImage->wThumbWidth, psImage->wThumbHeight);
	//MP_DEBUG2("target W %d, tH %d", psImage->wTargetWidth, psImage->wTargetHeight);

	return PASS; //psJpeg->iErrorCode;
}


void LibjpegCallback_Check(IMAGEFILE *psImage, BOOL boLastScan)
{
	ST_IMGWIN src_win, *win ;
	mpDebugPrint("LibjpegCallback status=%d", boLastScan);
	static int progress = 0;
	BYTE state = 0 ;

	state = psImage->bDecodeMode & 0x04 ;
	mpDebugPrint("state=%x", state);
	if(state == 0)
	{
		if(boLastScan==0)
		{
			win = Idu_GetNextWin();
			src_win.pdwStart = (DWORD *)psImage->pbTarget ;
			src_win.wWidth = psImage->wRealTargetWidth ;
			src_win.wHeight = psImage->wRealTargetHeight ;
			src_win.dwOffset = src_win.wWidth * 2 ;
			ImageDraw_FitToOrigin(&src_win, win);
			Idu_ChgWin(win);
		}
	}
}
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

