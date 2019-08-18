/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 0

/*
// Include section
*/
#include "global612.h"
#include "mpTrace.h"
#include "jpeg.h"
#include "mpapi.h"
//#include "image.h"

#ifndef BMP
#define BMP     1
#endif

#if BMP
DWORD dwBMP16Type;  //neil add for biCompression  20070809

//-------------------------------------------------------------------------------
DWORD YCbCrYCbCr_2_YYCbCr(DWORD dwColor0, DWORD dwColor1)
{
	DWORD Cr = ( (dwColor0 & 0xff) + (dwColor1 & 0xff) ) >> 1;
	DWORD Cb = ( ((dwColor0 >> 8) & 0xff) + ((dwColor1 >> 8) & 0xff) ) >> 1;

	return (dwColor0 & 0xff000000) | (dwColor1 & 0x00ff0000) | (Cb << 8) | Cr;
}
/*
Y = 0.299*R + 0.587*G + 0.114*B
Cb = - 0.1687*R - 0.3313*G + 0.5*B + 128
Cr = 0.5*R - 0.4187*G - 0.0813*B + 128
*/
DWORD RGB_2_YUV(BYTE r0, BYTE g0, BYTE b0)
{
    DWORD Y, Cb, Cr;
#if 0
	Y = ((r0 << 8) + (g0 << 9) + (b0 << 6) + (b0 << 5)) >> 10;
	Cb = (-((r0<<7) + (r0<<5)) - ((g0<<8) + (g0<<5)) + ((b0<<9)-(b0<<5)) + 131072) >> 10;
	Cr = (((r0<<9)-(r0<<5)) - ((g0<<8) + (g0<<7)) - ((b0<<7)+(b0<<3)) + 131072) >> 10;
#else
    Y = ((306 * r0) + (601 * g0) + (117 * b0)) >> 10;
    Cb = ((-173 * r0) + (-339 * g0) + (512 * b0) + 131072) >> 10;
    Cr = ((512 * r0) + (-429 * g0) + (-83 * b0) + 131072) >> 10;
#endif
	return (Y << 24) | (Y << 16) | (Cb << 8) | Cr;
}
//-------------------------------------------------------------------------------
DWORD RGBRGB_2_YUV(BYTE r0, BYTE g0, BYTE b0, BYTE r1, BYTE g1, BYTE b1)
{
	DWORD Y0, Cb0, Cr0, Y1, Cb1, Cr1;
#if 0
	Y0 = ((r0 << 8) + (g0 << 9) + (b0 << 6) + (b0 << 5)) >> 10;
	Cb0 = (-((r0<<7) + (r0<<5)) - ((g0<<8) + (g0<<5)) + ((b0<<9)-(b0<<5)) + 131072) >> 10;
	Cr0 = (((r0<<9)-(r0<<5)) - ((g0<<8) + (g0<<7)) - ((b0<<7)+(b0<<3)) + 131072) >> 10;

	Y1 = ((r1 << 8) + (g1 << 9) + (b1 << 6) + (b1 << 5)) >> 10;
	Cb1 = (-((r1<<7) + (r1<<5)) - ((g1<<8) + (g1<<5)) + ((b1<<9)-(b1<<5)) + 131072) >> 10;
	Cr1 = (((r1<<9)-(r1<<5)) - ((g1<<8) + (g1<<7)) - ((b1<<7)+(b1<<3)) + 131072) >> 10;

	Cb0 = (Cb0 + Cb1) >> 1;
	Cr0 = (Cr0 + Cr1) >> 1;
#else
    Y0 = ((306 * r0) + (601 * g0) + (117 * b0)) >> 10;
    Cb0 = ((-173 * r0) + (-339 * g0) + (512 * b0) + 131072) >> 10;
    Cr0 = ((512 * r0) + (-429 * g0) + (-83 * b0) + 131072) >> 10;

    Y1 = ((306 * r1) + (601 * g1) + (117 * b1)) >> 10;
    Cb1 = ((-173 * r1) + (-339 * g1) + (512 * b1) + 131072) >> 10;
    Cr1 = ((512 * r1) + (-429 * g1) + (-83 * b1) + 131072) >> 10;

    Cb0 = (Cb0 + Cb1) >> 1;
    Cr0 = (Cr0 + Cr1) >> 1;
#endif
	return (Y0 << 24) | (Y1 << 16) | (Cb0 << 8) | Cr0;
}



/*-------------------------------------*/
int Bmp_PollEvent(void)
{
		if (Polling_Event())
		{
			TurnOffIPUClk();
			return FAIL;
		}
	return PASS;
}

//-------------------------------------------------------------------------------
int Bmp_Decode_Bits(IMAGEFILE *psImage, DWORD *pdwPal, WORD wBitCount, BYTE boYield)
{
	int n, iRowBytes;
	WORD wScale =  1 << psImage->bScaleDown;

	MP_DEBUG2("Bmp_Decode_Bits %d %d", wBitCount, wScale);
	if (wBitCount <= 8)
	{
		n = 8 / wBitCount;    // 1bit : n=8, 4bit : n=2, 8bit : n=1
		iRowBytes = psImage->wImageWidth / n;
		if (psImage->wImageWidth & (n - 1)) iRowBytes++;
	}
	else
	{
		n = wBitCount >> 3;    // 16bit : n=2, 24bit : n=3, 32bit : n=4
		iRowBytes = psImage->wImageWidth * n;
	}
	if (iRowBytes & 0x3) iRowBytes += 4 - (iRowBytes & 3);
	MP_DEBUG1("iRowBytes %d", iRowBytes);
	BYTE *pbLineBuffer = (BYTE *)ext_mem_malloc(iRowBytes + 256);
	if (pbLineBuffer == NULL) return FAIL;

	memset(pbLineBuffer, 0, iRowBytes);

	int i, j, y, iReadSize;
	BYTE bData0, bData1;
	BYTE *pbLinePtr;
	DWORD dwData;
	BYTE bgr[6];

    BYTE r0, g0,  b0,  r1,  g1,  b1;
	BYTE bDataP0, bDataP1;	

	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD *pdwTarget, *pdwNextLine;

	pdwTarget = (DWORD *)psImage->pbTarget + ((psImage->wTargetWidth * (psImage->wTargetHeight - 1)) >> 1);
	n = psImage->wRealTargetWidth;
	for (y = 0; y < psImage->wTargetHeight; y++)
	{

        if(Bmp_PollEvent())
         {
		   MP_ALERT("%s: Break BMP decode", __FUNCTION__);
		   goto BMP_ERROR;
         }
	    TaskYield();
		
		pdwNextLine = pdwTarget - (psImage->wTargetWidth >> 1);
		pbLinePtr = (BYTE *)((DWORD)pbLineBuffer | 0xA0000000);
		iReadSize = mpxStreamRead(pbLinePtr, 1, iRowBytes , psStream);
		if (iReadSize == 0) break;

		mpxStreamSkip(psStream, iRowBytes * (wScale - 1));

		switch (wBitCount)
		{
			case 1 :  // 1 byte = 8 pixels, 2 colors
#if 1 //version 1109
			  n = ALIGN_CUT_8(psImage->wRealTargetWidth);
			  psImage->wRealTargetWidth = n;
			  
			  for (i = 0; i < (n); i+=8/*i++*/)
				{
					//bData0 = pbLineBuffer[i];
					bData0 = *pbLinePtr;
					
					for (j = 6; j >= 0; j -= 2) {
						bData1  =  bData0 >> j;
						bDataP1 = (bData0 >> (j)) & 0x1;
						bDataP0 = (bData0 >> (j+1)) & 0x1;
						
						r0 = (pdwPal[bDataP0]>>8) & 0xff;
						g0 = (pdwPal[bDataP0]>>4) & 0xff;
						b0 = (pdwPal[bDataP0]) & 0xff;
						r1 = (pdwPal[bDataP1]>>8) & 0xff;
						g1 = (pdwPal[bDataP1]>>4) & 0xff;
						b1 = (pdwPal[bDataP1]) & 0xff;
                        
						*pdwTarget++ = RGBRGB_2_YUV(r0,g0,b0, r1,g1,b1);
					}

					pbLinePtr += (wScale);//wScale
				}
#endif
#if 0 //old version 
				for (i = 0; i < n; i++)
				{
					bData0 = pbLineBuffer[i];
					for (j = 6; j >= 0; j -= 2) {
						bData1 = bData0 >> j;
						*pdwTarget++ = YCbCrYCbCr_2_YYCbCr(pdwPal[(bData1 >> 1) & 1], pdwPal[bData1 & 1]);
					}
				}
#endif
				break;

			case 4:  // 1 byte = 2 pixels, 16 colors
				n = ALIGN_CUT_2(psImage->wRealTargetWidth);
			  	psImage->wRealTargetWidth = n;
				for (i = 0; i < n; i+=2)
				{
					bData0 = *pbLinePtr;
					pbLinePtr+=(wScale);
					*pdwTarget++ = YCbCrYCbCr_2_YYCbCr(pdwPal[(bData0 >> 4) & 0xf], pdwPal[bData0 & 0xf]);
				}
				break;

			case 8:  // 1 byte = 1 pixels, 256 colors
				for (i = 0; i < n; i+=2)
				{
					bData0 = *pbLinePtr;

					if(wScale == 1)
						pbLinePtr++;
					else
						pbLinePtr+=(wScale);
					
					if (i < n)
						bData1 = *pbLinePtr;
					else
						bData1 = bData0;

					if(wScale == 1)
						pbLinePtr++;
					else
						pbLinePtr+=(wScale);
					
					*pdwTarget++ = YCbCrYCbCr_2_YYCbCr(pdwPal[bData1], pdwPal[bData0]);
				}
				break;

			case 16 :  // 565
				for (i = 0; i < n; i+=2)
				{

					dwData = *(DWORD *)pbLinePtr;

					if (dwBMP16Type == RGB_X555 || dwBMP16Type == RGB_A555 )
					{
						bgr[5] = ((*(pbLinePtr+3)) & 0x7c)<<1;
						bgr[4] = ((*(pbLinePtr+3)) & 0x3) <<6;
						bgr[4] |= ((*(pbLinePtr+2)) & 0xe0) >> 2;
						bgr[3] = ((*(pbLinePtr+2)) & 0x001f) << 3;

						bgr[2] =( (*(pbLinePtr+1)) & 0x7c)<<1;			//r
						bgr[1] = ((*(pbLinePtr+1)) & 0x3) << 6;	//g
						bgr[1] |= ((*(pbLinePtr+0) )& 0xe0) >> 2;
						bgr[0] = ((*(pbLinePtr+0)) & 0x1f) << 3;	//b
					}
					else 		//565
					{
						bgr[5] = (*(pbLinePtr+3)) & 0xF8;
						bgr[4] = ((*(pbLinePtr+3)) & 0x7) <<5;
						bgr[4] |=((*(pbLinePtr+2)) & 0xe0) >> 3 ;
						bgr[3] = ((*(pbLinePtr+2)) & 0x1f) << 3;

						bgr[2] =(*(pbLinePtr+1)) & 0xF8;
						bgr[1] = ((*(pbLinePtr+1)) & 0x7) << 5;
						bgr[1] |=((*(pbLinePtr+0)) & 0xe0) >> 3;
						bgr[0] = ((*(pbLinePtr+0)) & 0x1f) << 3;

					}

					*pdwTarget++ = RGBRGB_2_YUV(bgr[2],bgr[1],bgr[0], bgr[5],bgr[4],bgr[3]);

					pbLinePtr += 4 * wScale;
				}
				break;

			case 24:  // bgr
				for (i = 0; i < n; i+=2)
				{
					*pdwTarget++ = RGBRGB_2_YUV(
					pbLinePtr[2],pbLinePtr[1],pbLinePtr[0],
					pbLinePtr[5],pbLinePtr[4],pbLinePtr[3]);
					pbLinePtr += 6 * wScale;
				}
				break;

			case 32:  // bgra
				for (i = 0; i < n; i+=2)
				{
					*pdwTarget++ = RGBRGB_2_YUV(
						pbLinePtr[2],pbLinePtr[1],pbLinePtr[0],
						pbLinePtr[6],pbLinePtr[5],pbLinePtr[4]);

					pbLinePtr += 8 * wScale;
				}
				break;
		}

		pdwTarget = pdwNextLine;

		if ((boYield) && ((y & 0x3f) == 0))
			TaskYield();
	}

	if (pbLineBuffer) ext_mem_free(pbLineBuffer);
	return PASS;

BMP_ERROR:
	if (pbLineBuffer) ext_mem_free(pbLineBuffer);
	return FAIL;
}



#if 0   //byAlexWang 18may2007 m2project

//--------------------------------------------------------------------------------------------
int Bmp_CheckImageSize(IMAGEFILE *psImage, DWORD dwTargetSize)
{
	WORD wd, ht;
	wd = psImage->wTargetWidth;
	ht = psImage->wTargetHeight;

	psImage->bScaleDown = 0;
	if ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_THUMB)
	{
		while (ht > 240) {
			ht >>= 1;
			psImage->bScaleDown++;
		}
	}
	else
	{
		while (wd * ht * 2 > dwTargetSize && ht > 160) {
			ht >>= 1;
			psImage->bScaleDown++;
		}
	}
	psImage->wTargetWidth = wd;
	psImage->wTargetHeight = ht;
	return PASS;
}
#endif

//-------------------------------------------------------------------------------
int Bmp_Get_FileHeader(	IMAGEFILE *psImage,	ST_BMP_FILE_HEADER * psFileHeader)
{
	ST_MPX_STREAM *psStream = psImage->psStream;

	mpxStreamSeek(psStream, 0, SEEK_SET);
	psFileHeader->bType = mpxStreamReadWord_le(psStream);
	psFileHeader->dwSize   = mpxStreamReadDWord_le(psStream);
	psFileHeader->wUnused1 = mpxStreamReadWord_le(psStream);
	psFileHeader->wUnused2 = mpxStreamReadWord_le(psStream);
	psFileHeader->dwOffset = mpxStreamReadDWord_le(psStream);
	return PASS;
}

//-------------------------------------------------------------------------------
int Bmp_Get_ImgHeader(IMAGEFILE *psImage, ST_BMP_IMAGE_HEADER *psImgHeader)
{
	ST_MPX_STREAM *psStream = psImage->psStream;
	//image header
	psImgHeader->dwHeaderSize = mpxStreamReadDWord_le(psStream);
	if (psImgHeader->dwHeaderSize >= 40)
	{
		// BITMAPINFOHEADER
		psImgHeader->dwWidth = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwHeight = mpxStreamReadDWord_le(psStream);

		psImgHeader->wPlanes = mpxStreamReadWord_le(psStream);
		if (psImgHeader->wPlanes != 1)
			return BMP_IMG_HEADER_ERR;

		psImgHeader->wBitCount = mpxStreamReadWord_le(psStream);
		psImgHeader->dwCompression = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwImgSize = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwXperMeter = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwYperMeter = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwColourUsed = mpxStreamReadDWord_le(psStream);
		psImgHeader->dwSignificantColour = mpxStreamReadDWord_le(psStream);
	}
	else if (psImgHeader->dwHeaderSize == 12)
	{
		// BITMAPCOREHEADER
		psImgHeader->dwWidth = mpxStreamReadWord_le(psStream);
		psImgHeader->dwHeight = mpxStreamReadWord_le(psStream);

		psImgHeader->wPlanes = mpxStreamReadWord_le(psStream);
		if (psImgHeader->wPlanes != 1)
			return BMP_IMG_HEADER_ERR;

		psImgHeader->wBitCount = mpxStreamReadWord_le(psStream);
	}



//
// neil add for 16bit bmpfile 20070809
	if(psImgHeader->wBitCount == 16)
	{
		if(psImgHeader->dwCompression  == BI_RGB)		//x-5-5-5
		{
			dwBMP16Type = RGB_X555;
		}
		else if(psImgHeader->dwCompression  == BI_BITFIELD)
		{
			WORD dwRGBMask;

			mpxStreamSeek(psStream, 0x36, SEEK_SET);
			dwRGBMask = mpxStreamReadWord_le(psStream);
			MP_DEBUG1("dwRGBMask = %x" ,dwRGBMask);

			if (dwRGBMask == 0xF800)	//RGB_565
				dwBMP16Type = RGB_565;
			else if(dwRGBMask == 0x7C00)
				dwBMP16Type = RGB_A555;
		}
	}



	return PASS;
}

//-------------------------------------------------------------------------------
DWORD *Bmp_Get_Palette(IMAGEFILE *psImage, ST_BMP_IMAGE_HEADER *psImgHeader)
{
	DWORD *pdwPal = NULL;
	int iPalSize = 0;
	ST_MPX_STREAM *psStream = psImage->psStream;
	DWORD dwXRGB;

	if (psImgHeader->wBitCount <= 8)
	{
		if (psImgHeader->dwColourUsed)
			iPalSize = psImgHeader->dwColourUsed;
		else
			iPalSize = 1 << psImgHeader->wBitCount;

		if (iPalSize < 2) iPalSize = 2;
		iPalSize <<= 2;
		pdwPal = (DWORD *)ext_mem_malloc(iPalSize + 256);
		if (pdwPal == NULL) return NULL;

		if (psImgHeader->wBitCount == 1) {
#if 1 //new version
           
            pdwPal[0] = mpxStreamReadDWord_le(psStream);
			MP_DEBUG2("%s: dwXRGB[0]=0x%x", __FUNCTION__ , pdwPal[0]);			

			pdwPal[1] = mpxStreamReadDWord_le(psStream);
			MP_DEBUG2("%s: dwXRGB[1]=0x%x", __FUNCTION__ , pdwPal[1]);
			
#endif
   
#if 0 //old version
			pdwPal[0] = RGB_2_YUV(0, 0, 0);
			pdwPal[1] = RGB_2_YUV(0xff, 0xff, 0xff);
#endif			
			return pdwPal;
		}
		mpxStreamSeek(psStream, -iPalSize, SEEK_CUR);
		mpxStreamRead(pdwPal, 1, iPalSize, psStream);

		int i;
		BYTE *pbBgr = (BYTE *)pdwPal;
		iPalSize >>= 2;
		for (i = 0; i < iPalSize; i++)
		{
			pdwPal[i] = RGB_2_YUV(pbBgr[2], pbBgr[1], pbBgr[0]);
			pbBgr += 4;
		}

	}
	return pdwPal;
}

//-------------------------------------------------------------------------------
int Bmp_Decoder_DecodeHeader(IMAGEFILE *psImage,
					ST_BMP_FILE_HEADER *psFileHeader,
					ST_BMP_IMAGE_HEADER *psImgHeader )
{
	MP_DEBUG("Bmp_Decoder_DecodeHeader");
	ST_MPX_STREAM *psStream = psImage->psStream;
	int iErrorCode;

	iErrorCode = Bmp_Get_FileHeader(psImage, psFileHeader);
	if (iErrorCode != PASS) return iErrorCode;

	iErrorCode = Bmp_Get_ImgHeader(psImage, psImgHeader);
	if (iErrorCode != PASS) return iErrorCode;

	psImage->wImageWidth = psImgHeader->dwWidth;
	psImage->wImageHeight = psImgHeader->dwHeight;

	psImage->wRealTargetWidth = psImgHeader->dwWidth >> psImage->bScaleDown;
	psImage->wRealTargetHeight = psImgHeader->dwHeight >> psImage->bScaleDown;

	psImage->wTargetWidth = ALIGN_16(psImage->wRealTargetWidth);
	psImage->wTargetHeight = (psImage->wRealTargetHeight);

	psImage->wThumbWidth = psImage->wRealTargetWidth;
	psImage->wThumbHeight = psImage->wRealTargetHeight;

	if ((psImgHeader->dwWidth < IMAGE_MIN_WIDTH) || (psImgHeader->dwHeight < IMAGE_MIN_HEIGHT))
		return NOT_SUPPORTED_BMP_SIZE;

//	if (psImgHeader->dwWidth  * psImgHeader->dwHeight > 6000 * 6000)
//		return NOT_SUPPORTED_BMP_SIZE;

	if ((psImage->wTargetWidth * psImage->wRealTargetHeight * 2) > psImage->dwTargetSize)
		return NOT_SUPPORTED_BMP_SIZE;
	return iErrorCode;
}

//-------------------------------------------------------------------------------
int Bmp_Decoder_DecodeImage(IMAGEFILE *psImage)
{
	MP_DEBUG("Bmp_Decoder_DecodeImage");
	ST_BMP_FILE_HEADER *psFileHeader = NULL;
	ST_BMP_IMAGE_HEADER *psImgHeader = NULL;
	DWORD *pdwPal = NULL;
	ST_MPX_STREAM *psStream = psImage->psStream;

	BYTE *bpTarget = psImage->pbTarget;
	BYTE boYield = ((psImage->bDecodeMode & 0x3f) == IMG_DECODE_SLIDE);
	int iErrorCode = PASS;

	psFileHeader = (ST_BMP_FILE_HEADER *)ext_mem_malloc(sizeof(ST_BMP_FILE_HEADER)+32);
	if (psFileHeader == NULL) return ERR_MEM_MALLOC;

	psImgHeader = (ST_BMP_IMAGE_HEADER *)ext_mem_malloc(sizeof(ST_BMP_IMAGE_HEADER)+32);

	if (psFileHeader == NULL || psImgHeader == NULL)
		iErrorCode = ERR_MEM_MALLOC;
	else
	{
		iErrorCode = Bmp_Decoder_DecodeHeader(psImage, psFileHeader, psImgHeader);

		if (iErrorCode == PASS && psImgHeader->wBitCount <= 8)
		{
			pdwPal = Bmp_Get_Palette(psImage, psImgHeader);
			if (pdwPal == NULL) iErrorCode = ERR_MEM_MALLOC;
		}

		if (boYield) TaskYield();

		if (iErrorCode == PASS) {
			mpxStreamSeek(psStream, psFileHeader->dwOffset, SEEK_SET);
			iErrorCode = Bmp_Decode_Bits(psImage, pdwPal, psImgHeader->wBitCount, boYield);
		}
	}

	if (pdwPal) 		ext_mem_free(pdwPal);
	if (psFileHeader)	ext_mem_free(psFileHeader);
	if (psImgHeader)	ext_mem_free(psImgHeader);
	return iErrorCode;
}

//-------------------------------------------------------------------------------
DWORD Bmp_GetImageSize(IMAGEFILE *psImage)
{
	if (psImage == NULL) return 0;
	if (psImage->psStream == NULL) return 0;

	ST_MPX_STREAM *psStream = psImage->psStream;
	psImage->wImageWidth = 0;
	psImage->wImageHeight = 0;

	mpxStreamSeek(psStream, 0, SEEK_SET);
	//type "BM", B=0x42, M=0x4D
	if (mpxStreamReadWord(psStream) != IMAGE_TAG_BMP)
		return 0;

	//image header
	mpxStreamSeek(psStream, 14, SEEK_SET);
	DWORD wHeaderSize = mpxStreamReadDWord_le(psStream);

	MP_DEBUG1("wHeaderSize %d", wHeaderSize);
	if(wHeaderSize >= 40)
	{
		psImage->wImageWidth = mpxStreamReadDWord_le(psStream) & 0xffff;
		psImage->wImageHeight = mpxStreamReadDWord_le(psStream) & 0xffff;
	}
	else
	{
		psImage->wImageWidth = mpxStreamReadWord_le(psStream);
		psImage->wImageHeight = mpxStreamReadWord_le(psStream);
	}
	MP_DEBUG("w %d, h %d", psImage->wImageWidth, psImage->wImageHeight);
	return (psImage->wImageWidth << 16) | psImage->wImageHeight;
}

//-------------------------------------------------------------------------------
int Bmp_Decoder_Init(IMAGEFILE *psImage)
{
	MP_ASSERT(psImage != NULL);

	MP_DEBUG("Bmp_Decoder_Init");
	psImage->ImageDecodeThumb = Bmp_Decoder_DecodeImage;
	psImage->ImageDecodeImage = Bmp_Decoder_DecodeImage;

	if (Bmp_GetImageSize(psImage) != 0)
		return PASS;
	else
		return FAIL;
}



#else   // Compiler issue

int Bmp_Decoder_Init(IMAGEFILE *psImage)
{
    return FAIL;
}


#endif




typedef struct  {
    unsigned long size; // Size of this structure: must be 40
    long width; // Image width
    long height; // Image height; -ve means top to bottom.
    unsigned short planes; // Must be 1
    unsigned short bit_count; // Must be 1, 4, 8 or 24
    unsigned long compression; // Only accept 0 here (uncompressed RGB data)
    unsigned long image_size; // Can be 0
    long xpels_per_metre; // We ignore these
    long ypels_per_metre; // We ignore these
    unsigned long num_colours_used; // Entries in colour table (0 means use default)
    unsigned long num_colours_important; // 0 means all colours are important.
  }bmp_header;


unsigned long InverseData(unsigned long data,int datasize)
{
		BYTE data_emp[4]={0};
		DWORD ret_value=0;
		int i;

		//mpDebugPrint("datasize=%d",datasize);


		for(i=0;i<datasize;i++)
		{
			data_emp[i]=(data>>(i*8))&0xFF;
			//mpDebugPrint("data_emp[i]=0x%x",data_emp[i]);

		}

		for(i=0;i<datasize;i++)
		{
			ret_value=(ret_value<<8)+data_emp[i];
			//mpDebugPrint("i=%d ret_value=0x%x",i,ret_value);
		}


		//__asm("	nop						");
		//__asm("break 100");

		return ret_value;
}


#if 0
void DumpYuvImage2Bmp(WORD WIDTH,WORD HEIGHT,BYTE *pbTarget)
{

		static int ret1;
		BYTE magic[14]={0x42,0x4d,
			0,0,0,0,
			0,0,0,0,
			54,0,0,0};
		bmp_header header;
		int width_temp,height_temp;
		int File_Size_tmp;
		BYTE *img_temp,*img_temp_inv;
		DWORD *p_pixel;
		DWORD y0, y1, cb, cr;
		DWORD r1, g1, b1;
		DWORD r2, g2, b2;
		DWORD rt, gt, bt;
		int i,j;
		DWORD yycbcr;
		int red,green,blue;


		width_temp=WIDTH;
		height_temp=HEIGHT;
		p_pixel = pbTarget;
		File_Size_tmp=width_temp*height_temp*3+54;

mpDebugPrint("++++++++++++++++++++++++++File_Size_tmp=0x%X",File_Size_tmp);

		//ret1=FileWrite(shandle, aa, 4);
  	//if(!ret1) UartOutText("write file fail\r\n");

		magic[2]=File_Size_tmp&0xFF;
		magic[3]=(File_Size_tmp>>8)&0xFF;
		magic[4]=(File_Size_tmp>>16)&0xFF;
		magic[5]=(File_Size_tmp>>24)&0xFF;


		header.size=0x28<<24;
		header.width=InverseData(width_temp,sizeof(header.width));
		header.height=InverseData(height_temp,sizeof(header.height));
		header.planes=1<<8;
		header.bit_count=24<<8;
		header.compression=0;
		header.image_size=InverseData(File_Size_tmp-54,sizeof(header.image_size));
		header.xpels_per_metre=0;
		header.ypels_per_metre=0;
		header.num_colours_used=0;
		header.num_colours_important=0;





		//#pragma alignvar(4)
		//img_temp = (BYTE *)ext_mem_malloc(MEMSIZE);//(width_temp*height_temp*3);
		img_temp = (BYTE *)ext_mem_malloc(width_temp*height_temp*3+54);
		if (img_temp == NULL)
				mpDebugPrint("!!!!!!!!!!!!!!!mem ERROR");
		img_temp_inv = (BYTE *)ext_mem_malloc(width_temp*height_temp*3);
		if (img_temp_inv == NULL)
				mpDebugPrint("!!!!!!!!!!!!!!!mem ERROR");


		mpDebugPrint("==========================width_temp=%d",width_temp);
		mpDebugPrint("==========================height_temp=%d",height_temp);

		int index=0,k=0;
		for (i=0;i<14;i++)
		{
				img_temp[index++]=magic[i];
		}
		BYTE *HeaderTemp;
		HeaderTemp=(BYTE *)&header;
		for (i=0;i<40;i++)
		{
				img_temp[index++]=HeaderTemp[i];
		}


    for (i=0;i<height_temp;i++)
      for (j=0;j<(width_temp>>1);j++)
      {

#if 1

					//get RGB
					y0 = (*p_pixel & 0xff000000) >> 24;
					y1 = (*p_pixel & 0x00ff0000) >> 16;
					cb = (*p_pixel & 0x0000ff00) >> 8;
					cr = (*p_pixel & 0x000000ff);


					rt = -1 * cb + 1435 * cr - 183590;
					gt = -352 * cb - 731 * cr + 138710;
					bt = 1814 * cb + cr - 232340;


					//r1 = (242 * y0 + rt);
					//r1 = (1024 * y0 + rt);
					r1 = ( (y0<<10) + rt);
					if (r1 & 0x80000000)
						r1 = 0;
					r1 >>= 10;
					if (r1 & 0x00000100)
						r1 = 255;
					//r2 = (242 * y1 + rt);
					//r2 = (1024 * y1 +rt);
					r2 = ((y1<<10) +rt);
					if (r2 & 0x80000000)
						r2 = 0;
					r2 >>= 10;
					if (r2 & 0x00000100)
						r2 = 255;

					//g1 = (263 * y0 + gt);
					//g1 = (1024 * y0 + gt);
					g1 = ((y0<<10) + gt);
					if (g1 & 0x80000000)
					{
						//mpDebugPrint("y0 = %x,  cb = %x,  cr = %x",y0, cb, cr);
						//mpDebugPrint("g1 = %x", g1);
						g1 = 0;
					}

					g1 >>= 10;
					if (g1 & 0x00000100)
						g1 = 255;
					//g2 = (263 * y1 +gt);
					//g2 = (1024 * y1 + gt);
					g2 = ((y1<<10) + gt);
					if (g2 & 0x80000000)
						g2 = 0;

					g2 >>= 10;
					if (g2 & 0x00000100)
						g2 = 255;

					//b1 = (1024 * y0 + bt);
					b1 = ((y0<<10) + bt);
					if (b1 & 0x80000000)
						b1 = 0;

					b1 >>= 10;
					if (b1 & 0x00000100)
						b1 = 255;
					//b2 = (1024 * y1 + bt);
					b2 = ((y1<<10) + bt);
					if (b2 & 0x80000000)
						b2 = 0;

					b2 >>= 10;
					if (b2 & 0x00000100)
						b2 = 255;



					img_temp_inv[k++]=b1;
					img_temp_inv[k++]=g1;
					img_temp_inv[k++]=r1;

					img_temp_inv[k++]=b2;
					img_temp_inv[k++]=g2;
					img_temp_inv[k++]=r2;
	/*
					img_temp[index++]=b1;
					img_temp[index++]=g1;
					img_temp[index++]=r1;

					img_temp[index++]=b2;
					img_temp[index++]=g2;
					img_temp[index++]=r2;
					*/
					//FileWrite(shandle, B_tmp, 1);
					//FileWrite(shandle, G_tmp, 1);
					//FileWrite(shandle, R_tmp, 1);

#else

//mpDebugPrint("k=%d",k);
					img_temp[k+0]=0x11;
					img_temp[k+1]=0x22;
					img_temp[k+2]=0x33;

					img_temp[k+3]=0x11;
					img_temp[k+4]=0x22;
					img_temp[k+5]=0x33;

      if((i%3)==0)
				img_temp[index++]=0xFF;
			else if((i%3)==1)
				img_temp[index++]=0;
			else if((i%3)==2)
				img_temp[index++]=0;
				//mpDebugPrint("0x%X",img_temp[i]);



#endif
					p_pixel++;
      }

		k=0;
    for (i=(height_temp-1);i>=0;i--)
      for (j=0;j<width_temp;j++)
      {
					k=(i*width_temp+j)*3;
					img_temp[index++]=img_temp_inv[k+0];
					img_temp[index++]=img_temp_inv[k+1];
					img_temp[index++]=img_temp_inv[k+2];
			}



		//FileWrite(shandle, magic, 14);
		//FileWrite(shandle, &header, 40);

		//FileWrite(shandle, img_temp, width_temp*height_temp*3);
		//FileWrite(shandle, img_temp, MEMSIZE);

		//FileClose(shandle);

		/*
		for (i=0;i<54;i++)
				mpDebugPrint("0x%X",img_temp[i]);*/

		Test_CreateAndWriteFileByName_WriteBufferData(SD_MMC, "qqq", "bmp", img_temp, width_temp * height_temp * 3 + 54);

		ext_mem_free(img_temp);
		ext_mem_free(img_temp_inv);

  	UartOutText("\n\rfile close\n\r");

						//__asm("break 100");

}
//-------------------------------------------------------------------------------
#endif

#if 0

#define bufsize 1024*1024*3
void DumpYuvImage2Bmp(WORD WIDTH,WORD HEIGHT,BYTE *pbTarget)
{

		static int ret1;
		BYTE magic[14]={0x42,0x4d,
			0,0,0,0,
			0,0,0,0,
			54,0,0,0};
		bmp_header header;
		int width_temp,height_temp;
		int File_Size_tmp;
		BYTE *img_temp;
		DWORD *p_pixel;
		DWORD y0, y1, cb, cr;
		DWORD r1, g1, b1;
		DWORD r2, g2, b2;
		DWORD rt, gt, bt;
		int i,j;
		DWORD yycbcr;
		int red,green,blue;

		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		char s[256],t[4];




		width_temp=WIDTH;
		height_temp=HEIGHT;
		p_pixel = pbTarget;
		File_Size_tmp=width_temp*height_temp*3+54;

mpDebugPrint("++++++++++++++++++++++++++File_Size_tmp=0x%X",File_Size_tmp);

		//ret1=FileWrite(shandle, aa, 4);
  	//if(!ret1) UartOutText("write file fail\r\n");

		magic[2]=File_Size_tmp&0xFF;
		magic[3]=(File_Size_tmp>>8)&0xFF;
		magic[4]=(File_Size_tmp>>16)&0xFF;
		magic[5]=(File_Size_tmp>>24)&0xFF;


		header.size=0x28<<24;
		header.width=InverseData(width_temp,sizeof(header.width));
		header.height=InverseData(height_temp,sizeof(header.height));
		header.planes=1<<8;
		header.bit_count=24<<8;
		header.compression=0;
		header.image_size=InverseData(File_Size_tmp-54,sizeof(header.image_size));
		header.xpels_per_metre=0;
		header.ypels_per_metre=0;
		header.num_colours_used=0;
		header.num_colours_important=0;


		img_temp = (BYTE *)ext_mem_malloc(54);
		if (img_temp == NULL)
		{
				mpDebugPrint("!!!!!!!!!!!!!!!mem_malloc ERROR");
				return;
		}

		int index=0,k=0;
		for (i=0;i<14;i++)
		{
				img_temp[index++]=magic[i];
		}
		BYTE *HeaderTemp;
		HeaderTemp=(BYTE *)&header;
		for (i=0;i<40;i++)
		{
				img_temp[index++]=HeaderTemp[i];
		}




		//‘ø} § ×
    strcpy(s,"debug");
		sDrv = DriveGet(SD_MMC);
		ret = CreateFile(sDrv, s, "bmp");
		if (ret)
		{
			mpDebugPrint("?????create file fail");
			return;
		}
		shandle = FileOpen(sDrv);
		if (!shandle)
		{
			mpDebugPrint("?????open file fail");
			return;
		}
		//¢Dyþ³g § × §Y
		ret = FileWrite(shandle, img_temp, 54);
			//mpDebugPrint("/////////////////src->buffer=0x%x",src->buffer);
			//ret = FileWrite(shandle, pbSource, dwSourceSize);
		if (!ret)
		{
			mpDebugPrint("?????write file fail");
			return;
		}
		ext_mem_free(img_temp);

		WORD wbufsize;
		if((width_temp*height_temp*3)<bufsize)
			wbufsize=width_temp*height_temp*3;
		else
			wbufsize=bufsize;

		img_temp = (BYTE *)ext_mem_malloc(wbufsize);

		if (img_temp == NULL)
		{
				mpDebugPrint("!!!!!!!!!!!!!!!mem_malloc ERROR");
				return;
		}
		mpDebugPrint("==========================width_temp=%d",width_temp);
		mpDebugPrint("==========================height_temp=%d",height_temp);
		k=0;
    for (i=0;i<height_temp;i++)
    {

	      for (j=0;j<(width_temp>>1);j++)
	      {
						//get RGB
						y0 = (*p_pixel & 0xff000000) >> 24;
						y1 = (*p_pixel & 0x00ff0000) >> 16;
						cb = (*p_pixel & 0x0000ff00) >> 8;
						cr = (*p_pixel & 0x000000ff);


						rt = -1 * cb + 1435 * cr - 183590;
						gt = -352 * cb - 731 * cr + 138710;
						bt = 1814 * cb + cr - 232340;


						//r1 = (242 * y0 + rt);
						//r1 = (1024 * y0 + rt);
						r1 = ( (y0<<10) + rt);
						if (r1 & 0x80000000)
							r1 = 0;
						r1 >>= 10;
						if (r1 & 0x00000100)
							r1 = 255;
						//r2 = (242 * y1 + rt);
						//r2 = (1024 * y1 +rt);
						r2 = ((y1<<10) +rt);
						if (r2 & 0x80000000)
							r2 = 0;
						r2 >>= 10;
						if (r2 & 0x00000100)
							r2 = 255;

						//g1 = (263 * y0 + gt);
						//g1 = (1024 * y0 + gt);
						g1 = ((y0<<10) + gt);
						if (g1 & 0x80000000)
						{
							//mpDebugPrint("y0 = %x,  cb = %x,  cr = %x",y0, cb, cr);
							//mpDebugPrint("g1 = %x", g1);
							g1 = 0;
						}

						g1 >>= 10;
						if (g1 & 0x00000100)
							g1 = 255;
						//g2 = (263 * y1 +gt);
						//g2 = (1024 * y1 + gt);
						g2 = ((y1<<10) + gt);
						if (g2 & 0x80000000)
							g2 = 0;

						g2 >>= 10;
						if (g2 & 0x00000100)
							g2 = 255;

						//b1 = (1024 * y0 + bt);
						b1 = ((y0<<10) + bt);
						if (b1 & 0x80000000)
							b1 = 0;

						b1 >>= 10;
						if (b1 & 0x00000100)
							b1 = 255;
						//b2 = (1024 * y1 + bt);
						b2 = ((y1<<10) + bt);
						if (b2 & 0x80000000)
							b2 = 0;

						b2 >>= 10;
						if (b2 & 0x00000100)
							b2 = 255;



						img_temp[k++]=b1;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}
						img_temp[k++]=g1;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}
						img_temp[k++]=r1;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}

						img_temp[k++]=b2;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}
						img_temp[k++]=g2;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}
						img_temp[k++]=r2;
						if(k>=wbufsize)
						{
							ret = FileWrite(shandle, img_temp, wbufsize);
							k=0;
						}

						p_pixel++;
	      }
				if (!ret)
				{
					mpDebugPrint("?????write file fail");
					return;
				}
			//mpDebugPrint("-----------line=%d",i);
		}

		if(k>0)
		{
			ret = FileWrite(shandle, img_temp, k);

		}


		FileClose(shandle);
		ext_mem_free(img_temp);


  	UartOutText("\n\rfile close\n\r");

						//__asm("break 100");

}


#else



void DumpYuvImage2Bmp(WORD WIDTH,WORD HEIGHT,BYTE *pbTarget)
{

		static int ret1;
		BYTE magic[14]={0x42,0x4d,
			0,0,0,0,
			0,0,0,0,
			54,0,0,0};
		bmp_header header;
		int width_temp,height_temp;
		int File_Size_tmp;
		BYTE *img_temp;
		DWORD *p_pixel;
		DWORD y0, y1, cb, cr;
		DWORD r1, g1, b1;
		DWORD r2, g2, b2;
		DWORD rt, gt, bt;
		int i,j;
		DWORD yycbcr;
		int red,green,blue;

		static DRIVE *sDrv;
		static STREAM *shandle;
		static int ret;
		char s[256],t[4];




		width_temp=WIDTH;
		height_temp=HEIGHT;
		p_pixel = pbTarget;
		File_Size_tmp=width_temp*height_temp*3+54;

mpDebugPrint("++++++++++++++++++++++++++File_Size_tmp=0x%X",File_Size_tmp);

		//ret1=FileWrite(shandle, aa, 4);
  	//if(!ret1) UartOutText("write file fail\r\n");

		magic[2]=File_Size_tmp&0xFF;
		magic[3]=(File_Size_tmp>>8)&0xFF;
		magic[4]=(File_Size_tmp>>16)&0xFF;
		magic[5]=(File_Size_tmp>>24)&0xFF;


		header.size=0x28<<24;
		header.width=InverseData(width_temp,sizeof(header.width));
		header.height=InverseData(height_temp,sizeof(header.height));
		header.planes=1<<8;
		header.bit_count=24<<8;
		header.compression=0;
		header.image_size=InverseData(File_Size_tmp-54,sizeof(header.image_size));
		header.xpels_per_metre=0;
		header.ypels_per_metre=0;
		header.num_colours_used=0;
		header.num_colours_important=0;


		img_temp = (BYTE *)ext_mem_malloc(54);
		if (img_temp == NULL)
		{
				mpDebugPrint("!!!!!!!!!!!!!!!mem_malloc ERROR");
				return;
		}

		int index=0,k=0;
		for (i=0;i<14;i++)
		{
				img_temp[index++]=magic[i];
		}
		BYTE *HeaderTemp;
		HeaderTemp=(BYTE *)&header;
		for (i=0;i<40;i++)
		{
				img_temp[index++]=HeaderTemp[i];
		}




		//¶}ÀÉ
    strcpy(s,"debug");
		sDrv = DriveGet(SD_MMC);
		ret = CreateFile(sDrv, s, "bmp");
		if (ret)
		{
			mpDebugPrint("?????create file fail");
			return;
		}
		shandle = FileOpen(sDrv);
		if (!shandle)
		{
			mpDebugPrint("?????open file fail");
			return;
		}
		//¥ý¼gÀÉÀY
		ret = FileWrite(shandle, img_temp, 54);
			//mpDebugPrint("/////////////////src->buffer=0x%x",src->buffer);
			//ret = FileWrite(shandle, pbSource, dwSourceSize);
		if (!ret)
		{
			mpDebugPrint("?????write file fail");
			return;
		}
		ext_mem_free(img_temp);
		img_temp = (BYTE *)ext_mem_malloc(width_temp*3);
		if (img_temp == NULL)
		{
				mpDebugPrint("!!!!!!!!!!!!!!!mem_malloc ERROR");
				return;
		}
		mpDebugPrint("==========================width_temp=%d",width_temp);
		mpDebugPrint("==========================height_temp=%d",height_temp);

    for (i=0;i<height_temp;i++)
    {
	    	k=0;
	      for (j=0;j<(width_temp>>1);j++)
	      {
						//get RGB
						y0 = (*p_pixel & 0xff000000) >> 24;
						y1 = (*p_pixel & 0x00ff0000) >> 16;
						cb = (*p_pixel & 0x0000ff00) >> 8;
						cr = (*p_pixel & 0x000000ff);


						rt = -1 * cb + 1435 * cr - 183590;
						gt = -352 * cb - 731 * cr + 138710;
						bt = 1814 * cb + cr - 232340;


						//r1 = (242 * y0 + rt);
						//r1 = (1024 * y0 + rt);
						r1 = ( (y0<<10) + rt);
						if (r1 & 0x80000000)
							r1 = 0;
						r1 >>= 10;
						if (r1 & 0x00000100)
							r1 = 255;
						//r2 = (242 * y1 + rt);
						//r2 = (1024 * y1 +rt);
						r2 = ((y1<<10) +rt);
						if (r2 & 0x80000000)
							r2 = 0;
						r2 >>= 10;
						if (r2 & 0x00000100)
							r2 = 255;

						//g1 = (263 * y0 + gt);
						//g1 = (1024 * y0 + gt);
						g1 = ((y0<<10) + gt);
						if (g1 & 0x80000000)
						{
							//mpDebugPrint("y0 = %x,  cb = %x,  cr = %x",y0, cb, cr);
							//mpDebugPrint("g1 = %x", g1);
							g1 = 0;
						}

						g1 >>= 10;
						if (g1 & 0x00000100)
							g1 = 255;
						//g2 = (263 * y1 +gt);
						//g2 = (1024 * y1 + gt);
						g2 = ((y1<<10) + gt);
						if (g2 & 0x80000000)
							g2 = 0;

						g2 >>= 10;
						if (g2 & 0x00000100)
							g2 = 255;

						//b1 = (1024 * y0 + bt);
						b1 = ((y0<<10) + bt);
						if (b1 & 0x80000000)
							b1 = 0;

						b1 >>= 10;
						if (b1 & 0x00000100)
							b1 = 255;
						//b2 = (1024 * y1 + bt);
						b2 = ((y1<<10) + bt);
						if (b2 & 0x80000000)
							b2 = 0;

						b2 >>= 10;
						if (b2 & 0x00000100)
							b2 = 255;



						img_temp[k++]=b1;
						img_temp[k++]=g1;
						img_temp[k++]=r1;

						img_temp[k++]=b2;
						img_temp[k++]=g2;
						img_temp[k++]=r2;

						p_pixel++;
	      }

				//¨C¦¸¥u¥ýnew¤@±ølineªº¤j¤p
				ret = FileWrite(shandle, img_temp, width_temp*3);
				if (!ret)
				{
					mpDebugPrint("?????write file fail");
					return;
				}
			mpDebugPrint("-----------line=%d",i);
		}

		FileClose(shandle);
		ext_mem_free(img_temp);


  	UartOutText("\n\rfile close\n\r");

						//__asm("break 100");

}


#endif


