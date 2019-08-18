/*
****************************************************************
*                      Magic Pixel Inc.
*
*    Copyright 2004, Magic Pixel Inc., HsinChu, Taiwan
*                    All rights reserved.
*
*
*
* Filename:      mpImagePP.c
*
* Programmer:    Liwu Huang
*                MPX E360 division
*
* Created: 	 04/14/2009
*
* Description:  Stand alone Post-Process test for hantro 8190
*              This file is only for testing.
*
* Change History (most recent first):
*
****************************************************************
*/
/*
// define this module show debug message or not,  0 : disable, 1 : enable
*/
#define LOCAL_DEBUG_ENABLE 1
#define PP_IN_WRITE_FILE_TEST 0
#define PP_IN_WRITE_FILE_RAW 0
#define PP_OUT_WRITE_FILE_TEST 0
#define PP_OUT_WRITE_FILE_RAW 0
#define PP_IN_READ_FILE	0
#define FIELD_WRITE_FILE 0
#define IDU_422YCBYCR 0
#define WRAPPER_TO_YYCBCR_IN 1
#define WRAPPER_TO_YYCBCR_OUT 1
#define RANDOM_PP_TEST_COUNT	32
#define	TEST_DELAY	1000

#include "global612.h"
#include "mpTrace.h"
#include "display.h"
#include "ppapi.h"
#include "ppinternal.h"
#include "ppcfg.h"
#include "dwl.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define FLOOR_MUL(x, mul)	((x)-(x)%(mul))
#define CEIL_MUL(x, mul)	((x)+(mul)-(x)%(mul))
#ifndef min
#define min(a,b)   (((a) < (b)) ? (a) : (b))
#endif

unsigned int pp_in_write_file;
unsigned int pp_in_write_raw;
unsigned int pp_out_write_file;
unsigned int pp_out_write_raw;

typedef struct
{
	BYTE Y0;
	BYTE Cb;
	BYTE Y1;
	BYTE Cr;
} PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED;

typedef struct
{
	BYTE Y0;
	BYTE Cr;
	BYTE Y1;
	BYTE Cb;
} PP_PIXEL_FMT_YCRYCB_4_2_2_INTERLEAVED;

typedef struct
{
	BYTE Cb;
	BYTE Y0;
	BYTE Cr;
	BYTE Y1;
} PP_PIXEL_FMT_CBYCRY_4_2_2_INTERLEAVED;

typedef struct
{
	BYTE Cr;
	BYTE Y0;
	BYTE Cb;
	BYTE Y1;
} PP_PIXEL_FMT_CRYCBY_4_2_2_INTERLEAVED;

/*
For PP input
IDU -> PP input
*/
static void Pixel_422YYCbCr_2_422YCbYCr(BYTE *t, const DWORD *s, u32 PixelDimension)
{
	MP_DEBUG("Pixel_422YYCbCr_2_422YCbYCr");
	PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED * target = (PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *)t;
	const ST_PIXEL * source = (const ST_PIXEL *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Cb = source->Cb;
		target->Y1 = source->Y1;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
For PP input
IDU -> PP input
*/
static void Pixel_422YYCbCr_2_422YCrYCb(BYTE *t, const DWORD *s, u32 PixelDimension)
{
	PP_PIXEL_FMT_YCRYCB_4_2_2_INTERLEAVED *target = (PP_PIXEL_FMT_YCRYCB_4_2_2_INTERLEAVED *)t;
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Cb = source->Cb;
		target->Y1 = source->Y1;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
For PP input
IDU -> PP input
*/
static void Pixel_422YYCbCr_2_422CbYCrY(BYTE *t, DWORD *s, u32 PixelDimension)
{
	PP_PIXEL_FMT_CBYCRY_4_2_2_INTERLEAVED * target = (PP_PIXEL_FMT_CBYCRY_4_2_2_INTERLEAVED *)t;
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Cb = source->Cb;
		target->Y1 = source->Y1;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
For PP input
IDU -> PP input
*/
static void Pixel_422YYCbCr_2_422CrYCbY(BYTE *t, const DWORD *s, u32 PixelDimension)
{
	MP_DEBUG("Pixel_422YYCbCr_2_422CrYCbY");
	PP_PIXEL_FMT_CRYCBY_4_2_2_INTERLEAVED *target = (PP_PIXEL_FMT_CRYCBY_4_2_2_INTERLEAVED *)t;
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Cb = source->Cb;
		target->Y1 = source->Y1;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

static void Pixel_422YYCbCr_2_422Semi_Planer(BYTE *target, const DWORD *s, u32 PixelDimension)
{
	MP_DEBUG("Pixel_422YYCbCr_2_422Semi_Planer");
	BYTE *CbCr = target+PixelDimension;
	BYTE *Y = target;
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	while (PixelDimension)
	{
		*Y++	= source->Y0;
		*CbCr++	= source->Cb;
		*Y++	= source->Y1;
		*CbCr++	= source->Cr;

		source++;
		PixelDimension-=2;
	}
}


/*
For PP input
IDU -> PP input 420Semi_Planer
error, should be tile square, not linear, pending
*/
static void Pixel_422YYCbCr_2_420Semi_Planer(BYTE *target, const DWORD *s, const u32 width, const u32 height)
{
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	BYTE *tCbCr = target+width*height;
	u32 w=width, h=height;

	while (h)
	{
		w = width;
		while (w)
		{
			*target++ = source->Y0;
			*target++ = source->Y1;
			if (h%2==0)
			{
				*tCbCr++ = source->Cb;
				*tCbCr++ = source->Cr;
			}
			source++;
			w-=2;
		}
		h--;
	}
}

static void Pixel_422YYCbCr_2_420Planer(BYTE *target, const DWORD *s, const u32 width, const u32 height)
{
	const ST_PIXEL *source = (const ST_PIXEL *)s;
	BYTE *tCb = target+width*height;
	BYTE *tCr = tCb+width*height/4;
	u32 w=width, h=height;

	while (h)
	{
		w = width;
		while (w)
		{
			*target++ = source->Y0;
			*target++ = source->Y1;
			if (h%2==0)
			{
				*tCb++ = source->Cb;
				*tCr++ = source->Cr;
			}
			source++;
			w-=2;
		}
		h--;
	}
}


/*
	for IDU display
	pp output 420 semi planer -> IDU
*/
static void Pixel_420Semi_Planer_2_422YYCbCr(ST_PIXEL *target, const BYTE *source, u32 PixelDimension)
{
	const BYTE *CbCr = source+PixelDimension;
	u32 i=0;
	while (PixelDimension)
	{
		target->Y0 = *source++;
		target->Y1 = *source++;
		target->Cb = *CbCr;
		target->Cr = *(CbCr+1);
		if (i)
		{
			CbCr+=2;
			i=0;
		}
		else i++;

		target++;
		PixelDimension-=2;
	}
}

static void Pixel_420Semi_Planer_2_420Planer(BYTE *target, const BYTE *source, u32 PixelDimension)
{
	BYTE *tCb = target + PixelDimension;
	BYTE *tCr = tCb + PixelDimension/4;
	const BYTE *sCbCr = source + PixelDimension;
	int i=0;
	while (PixelDimension)
	{
		for (i=0; i<4; i++)	*target++ = *source++;
		*tCb++ = *sCbCr++;
		*tCr++ = *sCbCr++;
		PixelDimension-=4;
	}
}

static void Pixel_420Planer_2_420Semi_Planer(BYTE *target, const BYTE *source, u32 PixelDimension)
{
	BYTE *tCbCr = target + PixelDimension;
	const BYTE *sCb = source + PixelDimension;
	const BYTE *sCr = sCb + PixelDimension/4;
	int i=0;
	while (PixelDimension)
	{
		for (i=0; i<4; i++)	*target++ = *source++;
		*tCbCr++ = *sCb++;
		*tCbCr++ = *sCr++;
		PixelDimension-=4;
	}
}

static void Pixel_420Semi_Planer_2_420Tiled(BYTE *target, const BYTE *source, const u32 width, const u32 height)
{
	const u32 PixelDimension = width * height;
	const u32 blocks = PixelDimension/16/16;
	const u32 wblocks = width/16;
	BYTE *tCbCr = target + PixelDimension;
	const BYTE *sCbCr = source + PixelDimension;
	const BYTE * const targetStart = target;
	int i=0;
	u32 w,h;
	const BYTE * sY = source;
	const BYTE *blockY = sY;
	const BYTE *blockCbCr = sCbCr;
	MP_DEBUG("wblocks=%d", wblocks);
//	MP_DEBUG("initial SMB 0x%x, 0x%x tMB 0x%x, 0x%x", sY-source, sCbCr-source, target-targetStart, tCbCr-targetStart);
	for (i=0; i<blocks; i++)
	{
		for (h=0; h<16; h++)
		{
//			MP_DEBUG("\t[%d] 0x%x", h, sY-source);
			memcpy(target, sY, 16);
			target+=16;
			sY+=16;
			if (h<8)
			{
				memcpy(tCbCr, sCbCr, 16);
				tCbCr+=16;
				sCbCr+=16;
			}
			if (h==0)
			{
				blockY = sY;
				blockCbCr = sCbCr;
			}
			sY += width-16;
			if (h<8) sCbCr += width -16;
		}
		if ((i+1)%wblocks)
		{
			sY = blockY;
			sCbCr = blockCbCr;
		}
		else
		{
			sY -= width-16;
			sCbCr -= width -16;
//			MP_DEBUG("Block Change row");
		}
//		MP_DEBUG("SMB 0x%x, 0x%x\tTMB 0x%x, 0x%x", sY-source, sCbCr-source, target-targetStart, tCbCr-targetStart);
	}
}


/*
	for IDU display
	pp output -> IDU
*/
static void Pixel_422YCbYCr_2_422YYCbCr(ST_PIXEL *target, const BYTE *s, u32 PixelDimension)
{
	const PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *source = (const PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Y1 = source->Y1;
		target->Cb = source->Cb;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
	for IDU display
	pp output -> IDU
*/
static void Pixel_422YCrYCb_2_422YYCbCr(ST_PIXEL *target, const BYTE *s, u32 PixelDimension)
{
	const PP_PIXEL_FMT_YCRYCB_4_2_2_INTERLEAVED *source = (const PP_PIXEL_FMT_YCRYCB_4_2_2_INTERLEAVED *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Y1 = source->Y1;
		target->Cb = source->Cb;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
	for IDU display
	pp output -> IDU
*/
static void Pixel_422CbYCrY_2_422YYCbCr(ST_PIXEL *target, const BYTE *s, u32 PixelDimension)
{
	const PP_PIXEL_FMT_CBYCRY_4_2_2_INTERLEAVED *source = (const PP_PIXEL_FMT_CBYCRY_4_2_2_INTERLEAVED *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Y1 = source->Y1;
		target->Cb = source->Cb;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}

/*
	for IDU display
	pp output -> IDU
*/
static void Pixel_422CrYCbY_2_422YYCbCr(ST_PIXEL *target, const BYTE *s, u32 PixelDimension)
{
	const PP_PIXEL_FMT_CRYCBY_4_2_2_INTERLEAVED *source = (const PP_PIXEL_FMT_CRYCBY_4_2_2_INTERLEAVED *)s;
	while (PixelDimension)
	{
		target->Y0 = source->Y0;
		target->Y1 = source->Y1;
		target->Cb = source->Cb;
		target->Cr = source->Cr;

		target++;
		source++;
		PixelDimension-=2;
	}
}


/*
write file test
PP input/output -> file
*/
static void Pixel_422YCbYCr_2_422Planer(BYTE *target, const BYTE *s, u32 PixelDimension)
{
	BYTE *Y = target;
	BYTE *Cb = target+PixelDimension;
	BYTE *Cr = Cb+PixelDimension/2;
	const PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *source = (const PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *)s;
	while (PixelDimension)
	{
		*Y++ = source->Y0;
		*Y++ = source->Y1;
		*Cb++ = source->Cb;
		*Cr++ = source->Cr;
		source++;
		PixelDimension-=2;
	}
}
/*
write file test
IDU -> file
*/
static void Pixel_422YYCbCr_2_422Planer(BYTE *target, const ST_PIXEL *source, u32 PixelDimension)
{
	BYTE *Cb = target+PixelDimension;
	BYTE *Cr = Cb+PixelDimension/2;
	while (PixelDimension)
	{
		*target++ = source->Y0;
		*target++ = source->Y1;
		*Cb++ = source->Cb;
		*Cr++ = source->Cr;
		source++;
		PixelDimension-=2;
	}
}
/*
static void Crop_422YYCbCr(ST_IMGWIN * const target, const BYTE *source, u32 width, u32 height)
{
	const u32 crop_width = target->wWidth - width;
	u32 i;
	for (i=0;i<target->wHeight;i++){
		while (width--){
			*target++ = *source++;
			*target++ = *source++;
		}
		source += crop_width*2;
	}
}
*/
static void Fit2Display(ST_IMGWIN * const targetwin, const BYTE * source, const u32 width, u32 height, const u32 BytesPerPixel)
{
	MP_DEBUG("Fit2Display %dx%d -> %dx%d", width, height, targetwin->wWidth, targetwin->wHeight);
	const WORD minWidth = min(targetwin->wWidth, width);
	WORD minHeight = min(targetwin->wHeight, height);

	BYTE * target = (BYTE *)targetwin->pdwStart;
	const u32 padwidth = targetwin->wWidth>width?targetwin->wWidth-width:width-targetwin->wWidth;
	const u32 padheight = targetwin->wHeight-height;
	while (minHeight--)
	{
		memcpy(target, source, minWidth*BytesPerPixel);
		target+=minWidth*BytesPerPixel;
		source+=minWidth*BytesPerPixel;
		if (targetwin->wWidth > width)
		{
			memset(target, 0, padwidth*BytesPerPixel);
			target+=padwidth*BytesPerPixel;
		}
		else
		{
			source+=padwidth*BytesPerPixel;
		}
	}
	if (targetwin->wHeight > height)
		memset(target, 0, targetwin->wWidth * padheight * BytesPerPixel);
}

/*
seperate 420 planer frame to top and bottom field.  The fields are packed into every line of input memory with 420 semi-planer
*/
static void SeperateField(BYTE * const top, BYTE * const bottom, const BYTE * const source_420Planer, const u32 width, const u32 height)
{
	u32 w, h;
	const u32 pdim = width*height;
	BYTE * tY = top;
	BYTE * tCbCr = tY + pdim/2;

	BYTE * bY = bottom;
	BYTE * bCbCr = bY + pdim/2;

	const BYTE * sY = source_420Planer;
	const BYTE * sCb = sY + pdim;
	const BYTE * sCr = sCb + pdim/4;
	u32 tlinecount=0, blinecount=0;

#if FIELD_WRITE_FILE
	DRIVE *sDrv = DriveGet(SD_MMC);
	STREAM *shandle;
	char fname[50];
	char extname[]="yuv";
	BYTE *fplaner;
	const u32 flen = width*height*1.5/2;
#endif

	for (h=0; h<height; h++)
	{
		if (h%2)
		{
			for (w=0; w<width; w++)
				*bY++ = *sY++;
			if (blinecount++)
			{
				for (w=0; w<width/2; w++)
				{
					*bCbCr++ = *sCb++;
					*bCbCr++ = *sCr++;
				}
				blinecount=0;
			}
		}
		else
		{
			for (w=0; w<width; w++)
				*tY++ = *sY++;
			if (tlinecount++)
			{
				for (w=0; w<width/2; w++)
				{
					*tCbCr++ = *sCb++;
					*tCbCr++ = *sCr++;
				}
				tlinecount=0;
			}
		}
	}

#if FIELD_WRITE_FILE
	fplaner = mem_malloc(flen);
	sprintf(fname, "top420Planer%dx%d", width, height/2);
	MP_DEBUG("Create File %s.%s", fname, extname);
	if (CreateFile(sDrv, fname, extname)) MP_DEBUG("create file %s.%s fail", fname, extname);
	if (!(shandle=FileOpen(sDrv))) MP_DEBUG("open file %s.%s fail", fname, extname);
	Pixel_420Semi_Planer_2_420Planer(fplaner, top, width*height/2);
	if (!FileWrite(shandle, fplaner, flen)) MP_DEBUG("write file %s.%s fail", fname, extname);
	FileClose(shandle);

	sprintf(fname, "bot420Planer%dx%d", width, height/2);
	MP_DEBUG("Create File %s.%s", fname, extname);
	if (CreateFile(sDrv, fname, extname)) MP_DEBUG("create file %s.%s fail", fname, extname);
	if (!(shandle=FileOpen(sDrv))) MP_DEBUG("open file %s.%s fail", fname, extname);
	Pixel_420Semi_Planer_2_420Planer(fplaner, bottom, width*height/2);
	if (!FileWrite(shandle, fplaner, flen)) MP_DEBUG("write file %s.%s fail", fname, extname);
	FileClose(shandle);

	if (fplaner) mem_free(fplaner);
	fplaner = NULL;
#endif
}

static u32 ImageLength(PPConfig *pPpConf, u32 ifOutImg)
{
	u32 len, format, format_rgb;
	if (ifOutImg)
	{
		if (pPpConf->ppOutFrmBuffer.enable)
			len = pPpConf->ppOutFrmBuffer.frameBufferWidth * pPpConf->ppOutFrmBuffer.frameBufferHeight;
		else
			len = pPpConf->ppOutImg.width*pPpConf->ppOutImg.height;
		format = pPpConf->ppOutImg.pixFormat >> 16;
		format_rgb = pPpConf->ppOutImg.pixFormat >> 12;
	}
	else
	{
		len = pPpConf->ppInImg.width*pPpConf->ppInImg.height;
		format = pPpConf->ppInImg.pixFormat >> 16;
	}
	switch (format)
	{
	case 0x1://422
		len *= 2;
		break;
	case 0x2://420
		len *= 1.5;
		break;
	case 0x4://rgb
		if (format_rgb == 0x41)	len *= 4;	//32 bit rgb
		else					len *= 2;	//16 bit rgb
		break;
	default:
		break;
	}
	return len;
}

typedef struct
{
	unsigned char A;
	unsigned char R;
	unsigned char G;
	unsigned char B;
} PP_PIXEL_FMT_RGB32;

typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} PPM_PIXEL_FMT_RGB32;

/*
 * Convert an integer to the corresponding string.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
static int itoa (int num, char *str, int radix)
{
#define RADIX_MAX 16    /* The radix is usually in the 2 to 16 range */
	int i, neg = 0;
	char *p = str;
	char *q = str;

	if (radix == 0) radix = 10;
	else if (radix < 2 || radix > RADIX_MAX)
	{
		return (radix);
	}
	if (num == 0)
	{
		*p++ = '0';
		*p = 0;
		return (0);
	}
	if (num < 0)
	{
		neg = 1;
		num = -num;
	}
	while (num > 0)
	{
		i = num % radix;
		if (i > 9) i += 7;
		*p++ = '0' + i;
		num /= radix;
	}
	if (neg) *p++ = '-';
	*p-- = 0;
	q = str;
	while (p > q)
	{
		i = *q;
		*q++ = *p;
		*p-- = i;
	}
	return (0);
}

static inline int String2File(STREAM * shandle, char *s)
{
	return FileWrite(shandle, s, strlen(s));
}

static u32 OutDimension(const PPConfig * const pPpConf)
{
	u32 OutWidth = pPpConf->ppInImg.width;
	u32 OutHeight = pPpConf->ppInImg.height;

	if (pPpConf->ppInRotation.rotation && pPpConf->ppInRotation.rotation<3)
	{
		OutWidth = pPpConf->ppOutImg.width;
		OutHeight = pPpConf->ppOutImg.height;
	}
	else if (pPpConf->ppInCrop.enable)
	{
		OutWidth = pPpConf->ppInCrop.width;
		OutHeight = pPpConf->ppInCrop.height;
	}
	return OutWidth*OutHeight;
}

static void ppmheader(STREAM * shandle, const PPConfig * const pPpConf, int comment, int maxvalue)
{
	char header[256];//No line should be longer than 70 characters
	String2File(shandle, "P6\n");
	if (comment)
	{
		sprintf(header, "#videoRange:%d rgbTransform:%d pixFormat:%x contrast:%d brightness:%d saturation:%d alpha:%d transparency:%d dithering:%d\n", pPpConf->ppInImg.videoRange, pPpConf->ppOutRgb.rgbTransform, pPpConf->ppOutImg.pixFormat,
		        pPpConf->ppOutRgb.contrast, pPpConf->ppOutRgb.brightness, pPpConf->ppOutRgb.saturation, pPpConf->ppOutRgb.alpha, pPpConf->ppOutRgb.transparency, pPpConf->ppOutRgb.ditheringEnable);
		MP_DEBUG("ppm comment with length(%d): %s", strlen(header), header);
		String2File(shandle, header);
	}
	itoa(pPpConf->ppOutImg.width, header, 10);
	String2File(shandle, header);
	String2File(shandle, " ");
	itoa(pPpConf->ppOutImg.height, header, 10);
	String2File(shandle, header);
	String2File(shandle, "\n");
	itoa(maxvalue, header, 10);
	String2File(shandle, header);
	String2File(shandle, "\n");
}

static DRIVE * initial_mcard(const int card)
{
	DRIVE * const sDrv = DriveGet(card);
	DirReset(sDrv);
	return sDrv;
}

static u32 trailing0(u32 pattern)
{
	u32 i=0;
	while ( !(pattern & (1<<i)) && i<32 ) i++;
	return i;
}

static u32 bits1(u32 pattern)
{
	u32 i=0;
	while (pattern & (1<<i) && i<32) i++;
	return i;
}

/*
WARNING the following write PPM functions do not take crop and rotation into consideration, need to modify if OutFrmBuffer enabled
*/
static void Pixel_RGB32_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="argb";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_RGB32_2_PPM");

	//rgb data
	BYTE *p = frgb;
	for (i=0; i<PixelDimension; i++)
	{
		memcpy(p, ++source, 3);
		source += 3;
		p += 3;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<8)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_BGR32_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="abgr";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_BGR32_2_PPM");

	//rgb data
	BYTE *p = frgb;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = source[3];	//r
		*p++ = source[2];	//g
		*p++ = source[1];	//b
		source += 4;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<8)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}


static void Pixel_RGB16_565_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="rgb565";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Normalizing RGB565->RGB666");

	//rgb data
	BYTE *p = frgb;
	const unsigned short * rgb565 = (const unsigned short*)source;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = (float)(*rgb565 >> 11)/32*64;//normalize to 6-bits
		*p++ = (*rgb565 >> 5) & 0x3F;
		*p++ = (float)(*rgb565 & 0x1F)/32*64;//normalize to 6-bits
		rgb565++;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<6)-1);//the maxvalue is no suit for ppm
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_BGR16_565_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="bgr565";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Normalizing BGR565->RGB666");

	//rgb data
	BYTE *p = frgb;
	const unsigned short * bgr565 = (const unsigned short*)source;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = (float)(*bgr565 & 0x1F)/32*64;//normalize to 6-bits
		*p++ = (*bgr565 >> 5) & 0x3F;
		*p++ = (float)(*bgr565 >> 11)/32*64;//normalize to 6-bits
		bgr565++;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<6)-1);//the maxvalue is no suit for ppm
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_RGB16_555_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="rgb555";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_RGB16_555_2_PPM");

	//rgb data
	BYTE *p = frgb;

	const unsigned short * rgb555 = (const unsigned short*)source;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = (*rgb555 >> 10) & 0x1F;
		*p++ = (*rgb555 >> 5)  & 0x1F;
		*p++ = (*rgb555 >> 0)  & 0x1F;
		rgb555++;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<5)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_BGR16_555_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="bgr555";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_BGR16_555_2_PPM");

	//rgb data
	BYTE *p = frgb;

	const unsigned short * bgr555 = (const unsigned short*)source;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = (*bgr555 >> 0)  & 0x1F;
		*p++ = (*bgr555 >> 5)  & 0x1F;
		*p++ = (*bgr555 >> 10) & 0x1F;
		bgr555++;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<5)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_RGB16_444_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="rgb444";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_RGB16_444_2_PPM");

	//rgb data
	BYTE *p = frgb;

	const unsigned short * rgb444 = (const unsigned short*)source;
	for (i=0; i<PixelDimension; i++)
	{
		*p++ = (*rgb444 >> 8) & 0xF;
		*p++ = (*rgb444 >> 4) & 0xF;
		*p++ = (*rgb444 >> 0) & 0xF;
		rgb444++;
	}

	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<4)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}

static void Pixel_RGBCustom_2_PPM(const BYTE * source, const PPConfig * const pPpConf)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[80]="rgbcustom";
	char extname[]="ppm";
	const u32 PixelDimension = pPpConf->ppOutImg.width * pPpConf->ppOutImg.height;
	u32 i;
	BYTE * const frgb = (BYTE *)mem_malloc(PixelDimension*3);

	MP_ASSERT(frgb);
	MP_DEBUG("Pixel_RGBCustom_2_PPM");

	//rgb data
	BYTE *p = frgb;
	u32 r,g,b;
	const PPRgbBitmask * const mask = &pPpConf->ppOutRgb.rgbBitmask;
	const u32 bits = pPpConf->ppOutImg.pixFormat == PP_PIX_FMT_RGB32_CUSTOM?32:16;

	const unsigned short * rgbs = (const unsigned short*)source;
	const unsigned int * rgbi = (const unsigned int*)source;
	u32 rgb;
	for (i=0; i<PixelDimension; i++)
	{
		if (bits==32)	rgb = *rgbi;
		else			rgb = *rgbs;
		r = (rgb & mask->maskR) >> trailing0(mask->maskR);
		g = (rgb & mask->maskG) >> trailing0(mask->maskG);
		b = (rgb & mask->maskB) >> trailing0(mask->maskB);

		*p++ = (int)((float)r/((mask->maskR >> trailing0(mask->maskR)) +1)*256);
		*p++ = (int)((float)g/((mask->maskG >> trailing0(mask->maskG)) +1)*256);
		*p++ = (int)((float)b/((mask->maskB >> trailing0(mask->maskB)) +1)*256);

		if (bits==32)	rgbi++;
		else			rgbs++;
	}

	sprintf(fname, "%s_%d", fname, bits);
	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	ppmheader(shandle, pPpConf, TRUE, (1<<8)-1);
	MP_ASSERT(FileWrite(shandle, frgb, PixelDimension*3) == PixelDimension*3);
	FileClose(shandle);

	mem_free(frgb);
	MP_DEBUG("write %s.%s completed", fname, extname);
}


static void Pixel_RGB16_444_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension, const PPRgbBitmask * const mask)
{
	unsigned char r,g,b;

	MP_DEBUG("Normalizing RGB444->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	const unsigned short * rgb444 = (const unsigned short*)source;
	while (PixelDimension--)
	{
		r = (*rgb444 & mask->maskR) >> trailing0(mask->maskR);
		g = (*rgb444 & mask->maskG) >> trailing0(mask->maskG);
		b = (*rgb444 & mask->maskB) >> trailing0(mask->maskB);

		*p =  (int)((float)r/((mask->maskR >> trailing0(mask->maskR)) +1)*32) << 11;
		*p |= (int)((float)g/((mask->maskG >> trailing0(mask->maskG)) +1)*64) << 5;
		*p |= (int)((float)b/((mask->maskB >> trailing0(mask->maskB)) +1)*32) << 0;
		p++;
		rgb444++;
	}
}

static void Pixel_RGB16_555_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("Normalizing RGB555->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	unsigned short rgb565;
	const unsigned short * rgb555 = (const unsigned short*)source;
	while (PixelDimension--)
	{
		r = (*rgb555 >> 10) & 0x1F;
		g = (*rgb555 >> 5)  & 0x1F;
		b = (*rgb555 >> 0)  & 0x1F;

		rgb565 = (int)r<<11;
		rgb565 |= ((int)((float)g/32*64))<<5;
		rgb565 |= b;
		*p++ = rgb565;

		rgb555++;
	}
}

static void Pixel_BGR16_555_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("Normalizing BGR555->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	unsigned short rgb565;
	const unsigned short * rgb555 = (const unsigned short*)source;
	while (PixelDimension--)
	{
		b = (*rgb555 >> 10) & 0x1F;
		g = (*rgb555 >> 5)  & 0x1F;
		r = (*rgb555 >> 0)  & 0x1F;

		rgb565 = (int)r<<11;
		rgb565 |= ((int)((float)g/32*64))<<5;
		rgb565 |= b;
		*p++ = rgb565;
		rgb555++;
	}
}

static void Pixel_BGR16_565_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("BGR565->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	unsigned short rgb565;
	const unsigned short * bgr565 = (const unsigned short*)source;
	while (PixelDimension--)
	{
		b = (*bgr565 >> 11) & 0x1F;
		g = (*bgr565 >> 5)  & 0x3F;
		r = (*bgr565 >> 0)  & 0x1F;

		rgb565 = (int)r<<11;
		rgb565 |= (int)g<<5;
		rgb565 |= b;
		*p++ = rgb565;
		bgr565++;
	}
}

static void Pixel_RGB32_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("Normalizing RGB32->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	while (PixelDimension--)
	{
		r = source[1];
		g = source[2];
		b = source[3];

		*p = ((int)((float)r/256*32))<<11;
		*p |= ((int)((float)g/256*64))<<5;
		*p |= (int)((float)b/256*32);
		p++;
		source+=4;
	}
}

static void Pixel_BGR32_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("Normalizing BGR32->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	while (PixelDimension--)
	{
		b = source[1];
		g = source[2];
		r = source[3];

		*p = ((int)((float)r/256*32))<<11;
		*p |= ((int)((float)g/256*64))<<5;
		*p |= (int)((float)b/256*32);
		p++;
		source+=4;
	}
}



static void Pixel_RGB32_AAA_2_RGB16_565(BYTE * target, const BYTE * source, u32 PixelDimension, const PPRgbBitmask * const mask)
{
	unsigned int r,g,b;

	MP_DEBUG("Normalizing RGB32_AAA->RGB565");
	//rgb data
	unsigned short *p = (unsigned short*)target;
	unsigned int *s= (unsigned int*)source;
	while (PixelDimension--)
	{
		r = (*s & mask->maskR) >> trailing0(mask->maskR);
		g = (*s & mask->maskG) >> trailing0(mask->maskG);
		b = (*s & mask->maskB) >> trailing0(mask->maskB);

		*p =  (int)((float)r/((mask->maskR >> trailing0(mask->maskR)) +1)*32) << 11;
		*p |= (int)((float)g/((mask->maskG >> trailing0(mask->maskG)) +1)*64) << 5;
		*p |= (int)((float)b/((mask->maskB >> trailing0(mask->maskB)) +1)*32) << 0;
		p++;
		s++;
	}
}


static void Pixel_BGR32_2_RGB32(BYTE * target, const BYTE * source, u32 PixelDimension)
{
	unsigned char r,g,b;

	MP_DEBUG("BGR32->RGB32");
	//rgb data
	unsigned int *p = (unsigned int*)target;
	while (PixelDimension--)
	{
		b = source[1];
		g = source[2];
		r = source[3];

		*p = (int)r<<16;
		*p |= (int)g<<8;
		*p |= b;
		p++;
		source+=4;
	}
}

//read a ppm WITHOUT header, just rgb data transform to argb, for IDU
static void PPMRGB_2_ARGB(BYTE * target, const int width, const int height)
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[12];
	int PixelDimension = width*height;
	const int rgblen = PixelDimension*3;
	BYTE * rgb	= (BYTE *)mem_malloc(rgblen);
	MP_ASSERT(rgb);
	PP_PIXEL_FMT_RGB32 * argb	= (PP_PIXEL_FMT_RGB32 *)target;
	sprintf(fname,"anh%d", height);
	MP_DEBUG("reading file %s.ppm", fname);
	MP_ASSERT(!FileSearch(sDrv, fname, "ppm", E_FILE_TYPE));
	MP_ASSERT(shandle = FileOpen(sDrv));
	MP_ASSERT(FileRead(shandle, rgb, rgblen)==rgblen);
	BYTE *p = rgb;

	while (PixelDimension--)
	{
		memcpy(++target, p, 3);
		p+=3;
		target+=3;
	}

	mem_free(rgb);
	MP_ASSERT(FileClose(shandle)==FS_SUCCEED);
}

void Img_Rotate180_PP(ST_IMGWIN * const target, const ST_IMGWIN * const source)
{
	PPInst pp = NULL;
	PPContainer *ppC;
	PPResult ppRet;
	PPConfig pPpConf;
	u32 ppInLen, ppOutLen, ppInPixDim, ppOutPixDim;
	const u32 targetLen = target->wWidth*target->wHeight*4;//argb

#if (PP_IN_WRITE_FILE_TEST || PP_OUT_WRITE_FILE_TEST || PP_IN_READ_FILE)
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[50];
	char extname[]="yuv";
#endif

	*((int *)DMA_MPVR0_BASE) = 0x00000001;
	*((int *)DMA_MPVR1_BASE) = 0x00000001;

	MP_DEBUG("Img_Rotate180_PP");
	BYTE * ppIn		= NULL;
	BYTE * ppOut	= NULL;
	BYTE * fplaner	= NULL;
	BYTE * top		= NULL;
	BYTE * bottom	= NULL;

	ppRet = PPInit(&pp);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPInit fail %d", ppRet);
		goto end;
	}//else MP_DEBUG("PPInit OK");
	ppC = (PPContainer *)pp;

	/* First get the default PP settings */
	ppRet = PPGetConfig(pp, &pPpConf);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPGetConfig fail %d", ppRet);
		goto end;
	}//else MP_DEBUG("PPGetConfig PP_OK");

	/* setup PP */
	pPpConf.ppInImg.width = FLOOR_MUL(source->wWidth, 16);//mul of 16
	pPpConf.ppInImg.height = FLOOR_MUL(source->wHeight, 16);//mul of 16
	MP_DEBUG("ppInImg : %dx%d", pPpConf.ppInImg.width, pPpConf.ppInImg.height);

#if (WRAPPER_TO_YYCBCR_IN || WRAPPER_TO_YYCBCR_OUT)
	//PPREG(0x8) |= 0xFF000000;	//for decoder
	//PPREG(0xC) |= 0xFF;		//for decoder
	ppC->MPXWrapper = 1;
#endif
#if WRAPPER_TO_YYCBCR_IN
	Wrapper_reg0 |= 0x200000;
	MP_DEBUG("PP wrapper for YYCBCR in");
	pPpConf.ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
#else
	pPpConf.ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
#endif
	ppInPixDim = pPpConf.ppInImg.width* pPpConf.ppInImg.height;
	ppInLen = ImageLength(&pPpConf, 0);
	ppIn = (BYTE*)mem_malloc(ppInLen);
	MP_ASSERT(ppIn);

	u32 maxOutWidth		= PP_OUT_MAX_WIDTH_UPSCALED(pPpConf.ppInImg.width, 0);
	u32 maxOutHeight	= PP_OUT_MAX_HEIGHT_UPSCALED(pPpConf.ppInImg.height, 0);
	if (ppC->maxOutWidth < maxOutWidth)		maxOutWidth = ppC->maxOutWidth;
	if (ppC->maxOutHeight < maxOutHeight)	maxOutHeight = ppC->maxOutHeight;

#if PP_IN_READ_FILE
	strcpy(fname, "f");
	MP_DEBUG("read %s.%s", fname, extname);

	if (FileSearch(sDrv, fname, extname, E_FILE_TYPE) == FS_SUCCEED)
	{
		if (!(shandle = FileOpen(sDrv)))
			MP_ALERT("open file %s.%s fail", fname, extname);
		if (FileRead(shandle, ppIn, ppInLen) != ppInLen)
			MP_ALERT("FileRead error");

		FileClose(shandle);
	}
	else
	{
		MP_ALERT("No such file %s.%s", fname, extname);
		goto end;
	}
#else
#if WRAPPER_TO_YYCBCR_IN
	memcpy(ppIn, source->pdwStart, ppInLen);
#else
	switch (pPpConf.ppInImg.pixFormat)
	{
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422YCbYCr(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422CrYCbY(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		Pixel_422YYCbCr_2_420Semi_Planer(ppIn, source->pdwStart, source->wWidth, source->wHeight);
		break;
	default:
		MP_DEBUG("NO proper transform for ppIn");
		break;
	}
#endif
#endif

#if PP_IN_WRITE_FILE_TEST
#if PP_IN_WRITE_FILE_RAW
#if WRAPPER_TO_YYCBCR_IN
	sprintf(fname, "wIn422YYCbCr%dx%d", pPpConf.ppInImg.width, pPpConf.ppInImg.height);
#else
	sprintf(fname, "ppIn422YCbYCr%dx%d", pPpConf.ppInImg.width, pPpConf.ppInImg.height);
#endif
	MP_DEBUG("creat %s.%s", fname, extname);
	MP_ASSERT(!CreateFile(sDrv, fname, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	MP_ASSERT(FileWrite(shandle, ppIn, ppInLen)==ppInLen);
	FileClose(shandle);
	if (fplaner)
	{
		mem_free(fplaner);
		fplaner = NULL;
	}
#endif
	/*
		const u32 sourceLen = source->wWidth*source->wHeight*2;//422YYCbCr
		sprintf(fname, "source422YYCbCr%dx%d", source->wWidth, source->wHeight);
		MP_DEBUG("creat %s.%s", fname, extname);
		MP_ASSERT(!CreateFile(sDrv, fname, extname));
		MP_ASSERT(shandle=FileOpen(sDrv));
		MP_ASSERT(FileWrite(shandle, source->pdwStart, sourceLen)==sourceLen);
		FileClose(shandle);
		if (fplaner) {mem_free(fplaner);	fplaner = NULL;}
	*/
	/*
		fplaner = mem_malloc(ppInLen);
		sprintf(fname, "source422planer%dx%d", source->wWidth, source->wHeight);
		MP_DEBUG("creat %s.%s", fname, extname);
		MP_ASSERT(!CreateFile(sDrv, fname, extname));
		MP_ASSERT(shandle=FileOpen(sDrv));
		Pixel_422YYCbCr_2_422Planer(fplaner, source->pdwStart, source->wWidth* source->wHeight);
		MP_ASSERT(FileWrite(shandle, fplaner, ppInLen)==ppInLen);
		FileClose(shandle);
		if (fplaner) {mem_free(fplaner);	fplaner = NULL;}
	*/
#endif
	/* Set video range to 0 */
	pPpConf.ppInImg.videoRange = 1;
	if (pPpConf.ppInImg.videoRange) MP_DEBUG("ppInImg.videoRange: %d", pPpConf.ppInImg.videoRange);

//	pPpConf.ppInRotation.rotation = PP_ROTATION_180;
	if (pPpConf.ppInRotation.rotation) MP_DEBUG("rotation: %d", pPpConf.ppInRotation.rotation);

	//ppInCrop
	pPpConf.ppInCrop.enable	= 0;
	pPpConf.ppInCrop.originX= 64;	//mul of 16
	pPpConf.ppInCrop.originY= 64;	//mul of 16
	pPpConf.ppInCrop.width	= 400;	//mul of 8
	pPpConf.ppInCrop.height	= 400;	//mul of 8
	if (pPpConf.ppInCrop.enable) MP_DEBUG("ppInCrop: (%d, %d), %dx%d", pPpConf.ppInCrop.originX, pPpConf.ppInCrop.originY, pPpConf.ppInCrop.width, pPpConf.ppInCrop.height);

	//ppOutFrmBuffer
	pPpConf.ppOutFrmBuffer.enable			= 0;
	pPpConf.ppOutFrmBuffer.writeOriginX		= 200;	//mul of 2
	pPpConf.ppOutFrmBuffer.writeOriginY		= 200;	//mul of 1
	pPpConf.ppOutFrmBuffer.frameBufferWidth	= 800;
	pPpConf.ppOutFrmBuffer.frameBufferHeight= 592;
	if (pPpConf.ppOutFrmBuffer.enable) MP_DEBUG("ppOutFrmBuffer: (%d, %d), %dx%d", pPpConf.ppOutFrmBuffer.writeOriginX, pPpConf.ppOutFrmBuffer.writeOriginY, pPpConf.ppOutFrmBuffer.frameBufferWidth, pPpConf.ppOutFrmBuffer.frameBufferHeight);

	//ppOutMask1
	pPpConf.ppOutMask1.enable	= 0;
	pPpConf.ppOutMask1.originX	= 320;
	pPpConf.ppOutMask1.originY	= 60;
	pPpConf.ppOutMask1.width	= 60;
	pPpConf.ppOutMask1.height	= 40;
	if (pPpConf.ppOutMask1.enable) MP_DEBUG("ppOutMask1: (%d,%d),%dx%d", pPpConf.ppOutMask1.originX,pPpConf.ppOutMask1.originY, pPpConf.ppOutMask1.width, pPpConf.ppOutMask1.height);

	pPpConf.ppOutMask2.enable	= 0;
	pPpConf.ppOutMask2.originX	= 320;
	pPpConf.ppOutMask2.originY	= 60;
	pPpConf.ppOutMask2.width	= 60;
	pPpConf.ppOutMask2.height	= 40;
	if (pPpConf.ppOutMask2.enable) MP_DEBUG("ppOutMask2: (%d,%d),%dx%d", pPpConf.ppOutMask2.originX,pPpConf.ppOutMask2.originY, pPpConf.ppOutMask2.width, pPpConf.ppOutMask2.height);

	pPpConf.ppOutDeinterlace.enable = 0;
	if (pPpConf.ppOutDeinterlace.enable) MP_DEBUG("ppOutDeinterlace");

	if (pPpConf.ppOutDeinterlace.enable)
	{
		pPpConf.ppInImg.picStruct = PP_PIC_TOP_AND_BOT_FIELD;
		top = (BYTE*)mem_malloc(ppInLen/2);
		bottom = (BYTE*)mem_malloc(ppInLen/2);
		MP_ASSERT(top && bottom);
		SeperateField(top, bottom, ppIn, pPpConf.ppInImg.width, pPpConf.ppInImg.height);

		pPpConf.ppInImg.bufferBusAddr = (u32)top;
		MP_DEBUG("ppInImg.bufferBusAddr: 0x%x", pPpConf.ppInImg.bufferBusAddr);
		pPpConf.ppInImg.bufferCbBusAddr = pPpConf.ppInImg.bufferBusAddr + ppInPixDim/2;
		pPpConf.ppInImg.bufferBusAddrBot = (u32)bottom;
		pPpConf.ppInImg.bufferBusAddrChBot = pPpConf.ppInImg.bufferBusAddrBot + ppInPixDim/2;
	}
	else
	{
		pPpConf.ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
		/* Set the luminance input picture base address */
		pPpConf.ppInImg.bufferBusAddr = (u32)ppIn;
		MP_DEBUG("ppInImg.bufferBusAddr: 0x%x", pPpConf.ppInImg.bufferBusAddr);
		pPpConf.ppInImg.bufferCbBusAddr = pPpConf.ppInImg.bufferBusAddr + ppInPixDim;
		pPpConf.ppInImg.bufferCrBusAddr = pPpConf.ppInImg.bufferCbBusAddr + ppInPixDim/4;
	}

#if WRAPPER_TO_YYCBCR_OUT
	MP_DEBUG("PP wrapper for YYCBCR out");
	Wrapper_reg0 |= 0x400000;
	pPpConf.ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
#elif IDU_422YCBYCR
	MP_DEBUG("PP IDU_422YCBYCR");
	pPpConf.ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
#else
	pPpConf.ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
#endif
	MP_DEBUG("ppInImg.pixFormat: 0x%X", pPpConf.ppInImg.pixFormat);
	MP_DEBUG("ppInImg.picStruct: %d", pPpConf.ppInImg.picStruct);
	MP_DEBUG("ppOutImg.pixFormat: 0x%X", pPpConf.ppOutImg.pixFormat);

	//RGB 444
	pPpConf.ppOutRgb.rgbBitmask.maskR = 0x0F00;
	pPpConf.ppOutRgb.rgbBitmask.maskG = 0x00F0;
	pPpConf.ppOutRgb.rgbBitmask.maskB = 0x000F;

//	pPpConf.ppOutRgb.contrast		= -64;	//[-64, 64]
//	pPpConf.ppOutRgb.brightness		= 127;	//[-128, 127]
//	pPpConf.ppOutRgb.saturation		= 128;	//[-64, 128]
//	pPpConf.ppOutRgb.alpha			= 255;	//[0, 255] just for 32-bit RGB
//	pPpConf.ppOutRgb.transparency	= 1;	//[0, 1] just for RGB 5-5-5
//	pPpConf.ppOutRgb.ditheringEnable= 1;
	//ppOutRgb
	pPpConf.ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_709;
	pPpConf.ppOutRgb.rgbTransformCoeffs.a = 0;
	pPpConf.ppOutRgb.rgbTransformCoeffs.b = 0;
	pPpConf.ppOutRgb.rgbTransformCoeffs.c = 0;
	pPpConf.ppOutRgb.rgbTransformCoeffs.d = 0;
	pPpConf.ppOutRgb.rgbTransformCoeffs.e = 0;

	if (pPpConf.ppOutImg.pixFormat >> 16 == 0x4) //rgb
	{
		MP_DEBUG("ppOutRgb.rgbTransform: %d", pPpConf.ppOutRgb.rgbTransform);
		if (pPpConf.ppOutRgb.contrast)				MP_DEBUG("ppOutRgb.contrast: %d", pPpConf.ppOutRgb.contrast);
		if (pPpConf.ppOutRgb.brightness)			MP_DEBUG("ppOutRgb.brightness: %d", pPpConf.ppOutRgb.brightness);
		if (pPpConf.ppOutRgb.saturation)			MP_DEBUG("ppOutRgb.saturation: %d", pPpConf.ppOutRgb.saturation);
		if (pPpConf.ppOutRgb.alpha)					MP_DEBUG("ppOutRgb.alpha: %d", pPpConf.ppOutRgb.alpha);
		if (pPpConf.ppOutRgb.transparency)			MP_DEBUG("ppOutRgb.transparency: %d", pPpConf.ppOutRgb.transparency);
		if (!pPpConf.ppOutRgb.rgbTransform)			MP_DEBUG("ppOutRgb.rgbTransformCoeffs: (%d, %d, %d, %d, %d)", pPpConf.ppOutRgb.rgbTransformCoeffs.a, pPpConf.ppOutRgb.rgbTransformCoeffs.b, pPpConf.ppOutRgb.rgbTransformCoeffs.c, pPpConf.ppOutRgb.rgbTransformCoeffs.d, pPpConf.ppOutRgb.rgbTransformCoeffs.e);
		if (pPpConf.ppOutImg.pixFormat << 29 == 0)	MP_DEBUG("ppOutRgb.rgbBitmask: (0x%x, 0x%x, 0x%x, 0x%x)", pPpConf.ppOutRgb.rgbBitmask.maskR, pPpConf.ppOutRgb.rgbBitmask.maskG, pPpConf.ppOutRgb.rgbBitmask.maskB, pPpConf.ppOutRgb.rgbBitmask.maskAlpha);
		if (pPpConf.ppOutRgb.ditheringEnable)		MP_DEBUG("ppOutRgb.ditheringEnable");
	}

	if (pPpConf.ppInRotation.rotation && pPpConf.ppInRotation.rotation<3)
	{
		pPpConf.ppOutImg.width = pPpConf.ppInImg.height;
		pPpConf.ppOutImg.height = pPpConf.ppInImg.width;
	}
	else
	{
		pPpConf.ppOutImg.width = //808;
		    pPpConf.ppInImg.width;
		//pPpConf.ppInCrop.width;
		//target->wWidth;//mul of 8
		//10*PP_OUT_MIN_WIDTH;
		pPpConf.ppOutImg.height =  //48;
		    pPpConf.ppInImg.height;
		//pPpConf.ppInCrop.height;
		//target->wHeight;//mul of 2
		//10*PP_OUT_MIN_HEIGHT;
	}

	u32 OutWidth, OutHeight;
	if (pPpConf.ppOutFrmBuffer.enable)
	{
		OutWidth = pPpConf.ppOutFrmBuffer.frameBufferWidth;
		OutHeight = pPpConf.ppOutFrmBuffer.frameBufferHeight;
	}
	else
	{
		OutWidth = pPpConf.ppOutImg.width;
		OutHeight = pPpConf.ppOutImg.height;
	}
	//output resolution loop test
//	for (pPpConf.ppOutImg.width=PP_OUT_MIN_WIDTH; pPpConf.ppOutImg.width<=pPpConf.ppInImg.width ; pPpConf.ppOutImg.width+=8)
//	for (pPpConf.ppOutImg.width=pPpConf.ppInImg.width; pPpConf.ppOutImg.width<=maxOutWidth ; pPpConf.ppOutImg.width+=8)
	{
//		for (pPpConf.ppOutImg.height=PP_OUT_MIN_HEIGHT; pPpConf.ppOutImg.height<=pPpConf.ppInImg.height; pPpConf.ppOutImg.height+=2)
//		for (pPpConf.ppOutImg.height=pPpConf.ppInImg.height; pPpConf.ppOutImg.height<=maxOutHeight; pPpConf.ppOutImg.height+=2)
		{
			MP_DEBUG("ppOutImg : %dx%d", pPpConf.ppOutImg.width, pPpConf.ppOutImg.height);

			/* Set output picture base address */
			ppOutLen = ImageLength(&pPpConf, 1);
			ppOutPixDim = OutWidth * OutHeight;
			ppOut = (BYTE*)mem_malloc(ppOutLen);
			MP_ASSERT(ppOut);
			pPpConf.ppOutImg.bufferBusAddr = (u32)ppOut;
			//pPpConf.ppOutImg.bufferChromaBusAddr = pPpConf.ppOutImg.bufferBusAddr + OutWidth*OutHeight;
			MP_DEBUG("ppOutImg.bufferBusAddr: 0x%X", pPpConf.ppOutImg.bufferBusAddr);
			//MP_DEBUG("ppOutImg.bufferChromaBusAddr: 0x%X", pPpConf.ppOutImg.bufferChromaBusAddr);

			/* Now use the PP API to write in the new setup */
			ppRet = PPSetConfig(pp, &pPpConf);
			if (ppRet != PP_OK)
			{
				MP_ALERT("PPSetConfig fail %d", ppRet);
				goto end;
			}
			else MP_DEBUG("PPSetConfig ok");

			ppRet = PPGetResult(pp);
			if (ppRet != PP_OK)
			{
				MP_ALERT("PPGetResult fail %d", ppRet);
				goto end;
			}
			else MP_DEBUG("PPGetResult ok");

			/* Copy to display buffer */
			register IDU *idu = (IDU *) IDU_BASE;
#if IDU_422YCBYCR
			idu->TvCtrl0 |= 0x20000000;
			MP_DEBUG("IDU support 422YCbYCr");
			//memcpy(target->pdwStart, ppOut, targetLen<ppOutLen?targetLen:ppOutLen);
			Fit2Display(target, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, OutWidth, OutHeight, 2);
#else
			idu->TvCtrl0 &= ~0x20000000;
			if (pPpConf.ppOutImg.pixFormat >> 16 == 0x4) //rgb
			{
				u32 BytesPerPixel;
				if (pPpConf.ppOutImg.pixFormat >> 12 == 0x41)
				{
					idu->TvCtrl0 |= 0x80000000;    //ARGB888
					BytesPerPixel=4;
				}
				else
				{
					idu->TvCtrl0 |= 0x40000000;    //RGB565
					BytesPerPixel=2;
				}

				//idu->TvCtrl0 |= 0x04000000;//CSC_EN
				//idu->TvCtrl0 |= 0x02000000;//PASS_CSC
				MP_DEBUG("IDU RGB idu->TvCtrl0 = 0x%x", idu->TvCtrl0);
				//memcpy(target->pdwStart, ppOut, targetLen<ppOutLen?targetLen:ppOutLen);
				Fit2Display(target, ppOut, OutWidth, OutHeight, BytesPerPixel);
			}
			else
			{
				idu->TvCtrl0 &= 0xFFFFFF;
#if WRAPPER_TO_YYCBCR_OUT
				//memcpy(target->pdwStart, ppOut, targetLen<ppOutLen?targetLen:ppOutLen);
				Fit2Display(target, ppOut, OutWidth, OutHeight, 2);
#else
			switch (pPpConf.ppOutImg.pixFormat)
			{
			case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
				Pixel_422YCbYCr_2_422YYCbCr(target->pdwStart, ppOut, ppOutPixDim);
				break;
			case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
				Pixel_422YCrYCb_2_422YYCbCr(target->pdwStart, ppOut, ppOutPixDim);
				break;
			case PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED:
				Pixel_422CbYCrY_2_422YYCbCr(target->pdwStart, ppOut, ppOutPixDim);
				break;
			case PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED:
				Pixel_422CrYCbY_2_422YYCbCr(target->pdwStart, ppOut, ppOutPixDim);
				break;
			default:
				MP_DEBUG("NO Properly transform for IDU");
				break;
			}
#endif
			}
#endif

#if PP_OUT_WRITE_FILE_TEST
			if (pPpConf.ppOutImg.pixFormat >> 16 == 0x4)
			{
				//RGB conversion
				switch (pPpConf.ppOutImg.pixFormat)
				{
				case PP_PIX_FMT_RGB32:
					Pixel_RGB32_2_PPM(ppOut, &pPpConf);
					break;
				case PP_PIX_FMT_RGB16_5_6_5:
					Pixel_RGB16_565_2_PPM(ppOut, &pPpConf);
					break;
				case PP_PIX_FMT_RGB16_5_5_5:
					Pixel_RGB16_555_2_PPM(ppOut, &pPpConf);
					break;
				case PP_PIX_FMT_RGB16_CUSTOM:
					Pixel_RGB16_444_2_PPM(ppOut, &pPpConf);
					break;
				default:
					MP_DEBUG("NO Properly transform for ppm file");
					break;
				}
#if PP_OUT_WRITE_FILE_RAW
				//ppOut write out
				fplaner = mem_malloc(ppOutLen);
				MP_ASSERT(fplaner);
				sprintf(fname, "ppOut%dx%d_", pPpConf.ppOutImg.width, pPpConf.ppOutImg.height);
				switch (pPpConf.ppOutImg.pixFormat)
				{
				case PP_PIX_FMT_RGB32:
					strcat(fname, "32");
					break;
				case PP_PIX_FMT_RGB16_5_6_5:
					strcat(fname, "565");
					break;
				case PP_PIX_FMT_RGB16_5_5_5:
					strcat(fname, "555");
					break;
				case PP_PIX_FMT_RGB16_CUSTOM:
					strcat(fname, "16C");
					break;
				case PP_PIX_FMT_RGB32_CUSTOM:
					strcat(fname, "32C");
					break;
				default:
					MP_DEBUG("NO Properly transform for ppm file");
					break;
				}
				MP_DEBUG("Create File %s.%s", fname, "rgb");
				MP_ASSERT(!CreateFile(sDrv, fname, "rgb"));
				MP_ASSERT(shandle=FileOpen(sDrv));
				MP_ASSERT(FileWrite(shandle, ppOut, ppOutLen)==ppOutLen);
				FileClose(shandle);
				if (fplaner)
				{
					mem_free(fplaner);
					fplaner = NULL;
				}
#endif	//if PP_OUT_WRITE_FILE_RAW
			}
			else
			{
				//YCbCr space
				fplaner = mem_malloc(ppOutLen);
				MP_ASSERT(fplaner);

				sprintf(fname, "ppOut422Planer%dx%d", pPpConf.ppOutImg.width, pPpConf.ppOutImg.height);
#if WRAPPER_TO_YYCBCR_IN
				strcat(fname, "_wi");
#endif
#if WRAPPER_TO_YYCBCR_OUT
				strcat(fname, "_wo");
#endif
#if (WRAPPER_TO_YYCBCR_IN || WRAPPER_TO_YYCBCR_OUT)
				char regvalue[20];
				itoa(Wrapper_reg0, regvalue, 16);
				strcat(fname, "_");
				strcat(fname, regvalue);
#else
				strcat(fname, "_wx");
#endif
				MP_DEBUG("Create File %s.%s", fname, extname);
				MP_ASSERT(!CreateFile(sDrv, fname, extname));
				MP_ASSERT(shandle=FileOpen(sDrv));
#if WRAPPER_TO_YYCBCR_OUT
				Pixel_422YYCbCr_2_422Planer(fplaner, (ST_PIXEL *)ppOut, ppOutPixDim);
#else
				switch (pPpConf.ppOutImg.pixFormat)
				{
				case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
					Pixel_422YCbYCr_2_422Planer(fplaner, (PP_PIXEL_FMT_YCBCR_4_2_2_INTERLEAVED *)ppOut, ppOutPixDim);
					break;
				default:
					MP_DEBUG("NO Properly transform for file write");
					break;
				}

#endif
				MP_ASSERT(FileWrite(shandle, fplaner, ppOutLen)==ppOutLen);
				FileClose(shandle);
				if (fplaner)
				{
					mem_free(fplaner);
					fplaner = NULL;
				}
#if PP_OUT_WRITE_FILE_RAW
				//ppOut
#if WRAPPER_TO_YYCBCR_OUT
				sprintf(fname, "wOut422YYCbCr%dx%d", pPpConf.ppOutImg.width, pPpConf.ppOutImg.height);
#else
				sprintf(fname, "ppOut422YCbYCr%dx%d", pPpConf.ppOutImg.width, pPpConf.ppOutImg.height);
#endif
#if WRAPPER_TO_YYCBCR_IN
				strcat(fname, "_wi");
#endif
#if WRAPPER_TO_YYCBCR_OUT
				strcat(fname, "_wo");
#endif
#if (WRAPPER_TO_YYCBCR_IN || WRAPPER_TO_YYCBCR_OUT)
				strcat(fname, "_");
				strcat(fname, regvalue);
#else
				strcat(fname, "_wx");
#endif
				MP_DEBUG("Create File %s.%s", fname, extname);
				MP_ASSERT(!CreateFile(sDrv, fname, extname));
				MP_ASSERT(shandle=FileOpen(sDrv));
				MP_ASSERT(FileWrite(shandle, ppOut, ppOutLen)==ppOutLen);
				FileClose(shandle);
#endif	//if PP_OUT_WRITE_FILE_RAW
			}
#endif	//if PP_OUT_WRITE_FILE_TEST
			if (ppOut) mem_free(ppOut);
			ppOut = NULL;
			Idu_ChgWin (target);//change for iner loop
		}
	}

	//dump register
	MP_DEBUG("PP register after PPGetResult()");
	MP_DEBUG("000: %08x %08x %08x %08x",	PPREG(0x0), PPREG(0x4), PPREG(0x8), PPREG(0xc));
	MP_DEBUG("0F0: %08x %08x %08x %08x",	PPREG(0xf0), PPREG(0xf4), PPREG(0xf8), PPREG(0xfc));
	MP_DEBUG("100: %08x %08x %08x %08x",	PPREG(0x100), PPREG(0x104), PPREG(0x108), PPREG(0x10c));
	MP_DEBUG("110: %08x %08x %08x %08x",	PPREG(0x110), PPREG(0x114), PPREG(0x118), PPREG(0x11c));
	MP_DEBUG("120: %08x %08x %08x %08x",	PPREG(0x120), PPREG(0x124), PPREG(0x128), PPREG(0x12c));
	MP_DEBUG("130: %08x %08x %08x %08x",	PPREG(0x130), PPREG(0x134), PPREG(0x138), PPREG(0x13c));
	MP_DEBUG("140: %08x %08x %08x %08x",	PPREG(0x140), PPREG(0x144), PPREG(0x148), PPREG(0x14c));
	MP_DEBUG("150: %08x %08x %08x %08x",	PPREG(0x150), PPREG(0x154), PPREG(0x158), PPREG(0x15c));
	MP_DEBUG("160: %08x %08x %08x %08x",	PPREG(0x160), PPREG(0x164), PPREG(0x168), PPREG(0x16c));
	MP_DEBUG("170: %08x %08x %08x %08x",	PPREG(0x170), PPREG(0x174), PPREG(0x178), PPREG(0x17c));
	MP_DEBUG("180: %08x %08x %08x %08x",	PPREG(0x180), PPREG(0x184), PPREG(0x188), PPREG(0x18c));
	MP_DEBUG("190: %08x",					PPREG(0x190));
//	#if (WRAPPER_TO_YYCBCR_IN || WRAPPER_TO_YYCBCR_OUT)
	MP_DEBUG("PP Wrapper_reg0 0x08030200: %08x", Wrapper_reg0);
//	#endif
	int sw_pp_in_endian = (PPREG(0xf4) >> 7) & 1;
	int sw_pp_out_endian = (PPREG(0xf4) >> 6) & 1;
	MP_DEBUG("sw_pp_in_endian: %s", sw_pp_in_endian?"little":"big");
	MP_DEBUG("sw_pp_out_endian: %s", sw_pp_out_endian?"little":"big");

end:

	/* Release the post-processor instance */
	PPRelease(pp);

	if (ppIn)
	{
		mem_free(ppIn);
		ppIn = NULL;
	}
	if (pPpConf.ppOutDeinterlace.enable)
	{
		if (top)
		{
			mem_free(top);
			top = NULL;
		}
		if (bottom)
		{
			mem_free(bottom);
			bottom = NULL;
		}
	}


	*((u32 *)DMA_MPVR0_BASE) &= 0xFFFFFFFE;
	*((u32 *)DMA_MPVR1_BASE) &= 0xFFFFFFFE;
	MP_DEBUG("Img_Rotate180_PP return");
}

void FileWriteSeperateTest(const ST_IMGWIN * const source)
{
	DRIVE *sDrv = DriveGet(SD_MMC);
	STREAM *shandle;
	char fname1[]="ftest";
	char fname2[]="ftests";
	char fname3[]="ftesth";
	char extname[]="out";
	const u32 PixelDimension = source->wWidth * source->wHeight;
	BYTE * p=(BYTE *)source->pdwStart;

	MP_DEBUG("FileWriteSeperateTest");
	SystemDeviceInit(SD_MMC);
	DriveAdd(SD_MMC);
	DriveChange(SD_MMC);
	if (DriveCurIdGet()!=SD_MMC) MP_ALERT("DriveChange error");
	sDrv = DriveGet(DriveCurIdGet());
	if (DirReset(sDrv) != FS_SUCCEED)
	{
		MP_ALERT("OpenFileByNameForRead(): DirReset() failed !");
		return;
	}

	MP_DEBUG("creat %s.%s", fname1, extname);
	if (CreateFile(sDrv, fname1, extname)) MP_ALERT("create file %s.%s fail", fname1, extname);
	if (!(shandle=FileOpen(sDrv))) MP_ALERT("open file %s.%s fail", fname1, extname);
	MP_ASSERT(FileWrite(shandle, (BYTE*)source->pdwStart, PixelDimension*2)==PixelDimension*2);
	FileClose(shandle);

	//split to 2 parts
	MP_DEBUG("creat %s.%s", fname2, extname);
	MP_ASSERT(!CreateFile(sDrv, fname2, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));

	MP_ASSERT(FileWrite(shandle, p, PixelDimension/2)==PixelDimension/2);
	p+=PixelDimension/2;
	MP_ASSERT(FileWrite(shandle, p, PixelDimension/2)==PixelDimension/2);
	p+=PixelDimension/2;
	MP_ASSERT(FileWrite(shandle, p, PixelDimension/2)==PixelDimension/2);
	p+=PixelDimension/2;
	MP_ASSERT(FileWrite(shandle, p, PixelDimension/2)==PixelDimension/2);
	FileClose(shandle);

	//write something ahead
	MP_DEBUG("creat %s.%s", fname3, extname);
	MP_ASSERT(!CreateFile(sDrv, fname3, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	String2File(shandle, "\n");
	MP_ASSERT(FileWrite(shandle, (BYTE*)source->pdwStart, PixelDimension*2)==PixelDimension*2);
	FileClose(shandle);

	MP_DEBUG("FileWriteSeperateTest return");
}

void FileWriteLineBreak()
{
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname1[]="ftest";
	char fname3[]="ftesth";
	char extname[]="out";
	const u32 len = 800*600*3;
	BYTE * p= (BYTE *)mem_malloc(len);
	u32 i, count=0;
	BYTE *c=p;
	for (i=0; i<len; i++)
	{
		*c++ = count++;
		if (count==256) count = 0;
	}

	MP_DEBUG("FileWriteLineBreak");

	MP_DEBUG("creat %s.%s", fname1, extname);
	if (CreateFile(sDrv, fname1, extname)) MP_ALERT("create file %s.%s fail", fname1, extname);
	if (!(shandle=FileOpen(sDrv))) MP_ALERT("open file %s.%s fail", fname1, extname);
	MP_ASSERT(FileWrite(shandle, p, len)==len);
	FileClose(shandle);

	//write something ahead
	MP_DEBUG("creat %s.%s", fname3, extname);
	MP_ASSERT(!CreateFile(sDrv, fname3, extname));
	MP_ASSERT(shandle=FileOpen(sDrv));
	FileWrite(shandle, "\n", strlen("\n"));
	MP_ASSERT(FileWrite(shandle, p, len)==len);
	FileClose(shandle);

	mem_free(p);
	MP_DEBUG("FileWriteLineBreak return");
}

void IDU_RGB32Data(ST_IMGWIN * target)
{
	MP_DEBUG("IDU_RGB32Data");
	MP_DEBUG("target(%d x %d)", target->wWidth,target->wHeight);
	PPMRGB_2_ARGB((BYTE*)target->pdwStart, target->wWidth, target->wHeight);

	register IDU *idu = (IDU *) IDU_BASE;
	MP_DEBUG("IDU idu->TvCtrl0 = 0x%x", idu->TvCtrl0);

	idu->TvCtrl0 |= 0x80000000;//RGBA888
//	idu->TvCtrl0 |= 0x40000000;//RGBA565
//	idu->TvCtrl0 |= 0x04000000;//CSC_EN
//	idu->TvCtrl0 |= 0x02000000;//PASS_CSC

	MP_DEBUG("IDU RGB idu->TvCtrl0 = 0x%x", idu->TvCtrl0);
	Idu_WaitBufferEnd();
}

static int RangedRand( int range_min, int range_max, u32 multiplier)
{
	// Generate random numbers in the half-closed interval
	// [range_min, range_max). In other words,
	// range_min <= random number < range_max
	int u = (float)rand() / (RAND_MAX + 0) * (range_max - range_min) + range_min;
	return FLOOR_MUL(u, multiplier);
}
#define RAND_ON	(RangedRand(0, 100000, 1)>50000)

static void PP_once(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                    PPInst const pp, PPConfig * const pPpConf)
{
	PPResult ppRet;
	u32 ppInLen, ppOutLen, ppInPixDim, ppOutPixDim;
	const u32 targetLen = target->wWidth*target->wHeight*3;//argb should *4
	u32 OutWidth, OutHeight;
	ppInPixDim = pPpConf->ppInImg.width* pPpConf->ppInImg.height;
	ppInLen = ImageLength(pPpConf, 0);
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[80];
	char extname[]="yuv";
	BYTE * fplaner	= NULL;

	BYTE * ppOut	= NULL;
	IDU * const idu = (IDU *) IDU_BASE;

	MP_DEBUG("\nppInImg : %dx%d", pPpConf->ppInImg.width, pPpConf->ppInImg.height);
	MP_DEBUG("ppInImg.bufferBusAddr: 0x%x", pPpConf->ppInImg.bufferBusAddr);
	if (IF_WRAPPER_IN) MP_DEBUG("PP wrapper for YYCBCR in");
	MP_DEBUG("ppInImg.pixFormat: 0x%X", pPpConf->ppInImg.pixFormat);
	MP_DEBUG("ppInImg.picStruct: %d", pPpConf->ppInImg.picStruct);

	if (pPpConf->ppInCrop.enable)			MP_DEBUG("ppInCrop: (%d, %d), %dx%d", pPpConf->ppInCrop.originX, pPpConf->ppInCrop.originY, pPpConf->ppInCrop.width, pPpConf->ppInCrop.height);
	if (pPpConf->ppInRotation.rotation)		MP_DEBUG("rotation: %d", pPpConf->ppInRotation.rotation);
	if (pPpConf->ppOutDeinterlace.enable)	MP_DEBUG("ppOutDeinterlace");

	if (pPpConf->ppOutFrmBuffer.enable)		MP_DEBUG("ppOutFrmBuffer: (%d, %d), %dx%d", pPpConf->ppOutFrmBuffer.writeOriginX, pPpConf->ppOutFrmBuffer.writeOriginY, pPpConf->ppOutFrmBuffer.frameBufferWidth, pPpConf->ppOutFrmBuffer.frameBufferHeight);
	if (pPpConf->ppOutMask1.enable)			MP_DEBUG("ppOutMask1: (%d,%d), %dx%d", pPpConf->ppOutMask1.originX,pPpConf->ppOutMask1.originY, pPpConf->ppOutMask1.width, pPpConf->ppOutMask1.height);
	if (pPpConf->ppOutMask2.enable)			MP_DEBUG("ppOutMask2: (%d,%d), %dx%d", pPpConf->ppOutMask2.originX,pPpConf->ppOutMask2.originY, pPpConf->ppOutMask2.width, pPpConf->ppOutMask2.height);

	MP_DEBUG("ppOutImg : %dx%d", pPpConf->ppOutImg.width, pPpConf->ppOutImg.height);
	if (IF_WRAPPER_OUT) MP_DEBUG("PP wrapper for YYCBCR out");
	MP_DEBUG("ppOutImg.pixFormat: 0x%X", pPpConf->ppOutImg.pixFormat);
	if (pPpConf->ppOutImg.pixFormat >> 16 == 0x4)
	{//rgb
		MP_DEBUG("ppOutRgb.rgbTransform: %d", pPpConf->ppOutRgb.rgbTransform);
		if (pPpConf->ppOutRgb.contrast)				MP_DEBUG("ppOutRgb.contrast: %d", pPpConf->ppOutRgb.contrast);
		if (pPpConf->ppOutRgb.brightness)			MP_DEBUG("ppOutRgb.brightness: %d", pPpConf->ppOutRgb.brightness);
		if (pPpConf->ppOutRgb.saturation)			MP_DEBUG("ppOutRgb.saturation: %d", pPpConf->ppOutRgb.saturation);
		if (pPpConf->ppOutRgb.alpha) 				MP_DEBUG("ppOutRgb.alpha: %d", pPpConf->ppOutRgb.alpha);
		if (pPpConf->ppOutRgb.transparency)			MP_DEBUG("ppOutRgb.transparency: %d", pPpConf->ppOutRgb.transparency);
		if (!pPpConf->ppOutRgb.rgbTransform) 		MP_DEBUG("ppOutRgb.rgbTransformCoeffs: (%d, %d, %d, %d, %d)", pPpConf->ppOutRgb.rgbTransformCoeffs.a, pPpConf->ppOutRgb.rgbTransformCoeffs.b, pPpConf->ppOutRgb.rgbTransformCoeffs.c, pPpConf->ppOutRgb.rgbTransformCoeffs.d, pPpConf->ppOutRgb.rgbTransformCoeffs.e);
		if (pPpConf->ppOutImg.pixFormat << 29 == 0)	MP_DEBUG("ppOutRgb.rgbBitmask: (0x%x, 0x%x, 0x%x, 0x%x)", pPpConf->ppOutRgb.rgbBitmask.maskR, pPpConf->ppOutRgb.rgbBitmask.maskG, pPpConf->ppOutRgb.rgbBitmask.maskB, pPpConf->ppOutRgb.rgbBitmask.maskAlpha);
		if (pPpConf->ppOutRgb.ditheringEnable)		MP_DEBUG("ppOutRgb.ditheringEnable");
	}

	if (pPpConf->ppOutFrmBuffer.enable)
	{
		OutWidth = pPpConf->ppOutFrmBuffer.frameBufferWidth;
		OutHeight = pPpConf->ppOutFrmBuffer.frameBufferHeight;
	}
	else
	{
		OutWidth = pPpConf->ppOutImg.width;
		OutHeight = pPpConf->ppOutImg.height;
	}
	ppOutLen = ImageLength(pPpConf, 1);
	MP_DEBUG("ppOutLen=%d", ppOutLen);
	ppOutPixDim = OutWidth * OutHeight;
	ppOut = (BYTE*)mem_malloc(ppOutLen+16);
	MP_ASSERT(ppOut);
	memset(ppOut, 0, ppOutLen);
	pPpConf->ppOutImg.bufferBusAddr = ALIGN_8((u32)ppOut);//+4;
	pPpConf->ppOutImg.bufferChromaBusAddr = pPpConf->ppOutImg.bufferBusAddr + ppOutPixDim;
	MP_DEBUG("ppOutImg.bufferBusAddr: 0x%x", pPpConf->ppOutImg.bufferBusAddr);

	ppRet = PPSetConfig(pp, pPpConf);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPSetConfig fail %d", ppRet);
		return;
	}//else MP_DEBUG("PPSetConfig ok");

	ppRet = PPGetResult(pp);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPGetResult fail %d", ppRet);
		return;
	}
	else MP_DEBUG("PPGetResult ok");

	/* Copy to display buffer */

	if (pPpConf->ppOutImg.pixFormat >> 16 == 0x4)
	{//rgb
		u32 BytesPerPixel;
		/*		if (pPpConf->ppOutImg.pixFormat >> 12 == 0x41)	{idu->TvCtrl0 |= 0x80000000;idu->TvCtrl0 &= ~0x40000000;BytesPerPixel=4;}//ARGB888
				else
		*/
		{
			idu->TvCtrl0 |= 0x40000000;    //RGB565 and others
			idu->TvCtrl0 &= ~0x86000000;

			idu->Iducsc0 = 0x02010000;
			idu->Iducsc1 = 0x01d2584d;
			idu->Iducsc2 = 0x083eabd5;
			idu->Iducsc3 = 0x3ebe4c83;
			idu->ATCON_srgb_ctrl &= ~0x00000020;
			BytesPerPixel=2;
		}

		//need to tranform to idu format if needed
		switch (pPpConf->ppOutImg.pixFormat)
		{
		case PP_PIX_FMT_RGB32:
		{
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_RGB32_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_BGR32:
		{
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			//Pixel_BGR32_2_RGB32(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Pixel_BGR32_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_RGB16_5_5_5:
		{
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_RGB16_555_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_BGR16_5_5_5:
		{
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_BGR16_555_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_BGR16_5_6_5:
		{
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_BGR16_565_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_RGB16_CUSTOM:
		{
			//just for RGB16_444
			//Pixel_RGB16_444_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_RGB16_444_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim, &pPpConf->ppOutRgb.rgbBitmask);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_RGB32_CUSTOM:
		{
			//just for RGB32_AAA
			BYTE *RGBTmp = (BYTE*)mem_malloc(ppOutLen);
			Pixel_RGB32_AAA_2_RGB16_565(RGBTmp, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim, &pPpConf->ppOutRgb.rgbBitmask);
			Fit2Display(target, RGBTmp, OutWidth, OutHeight, BytesPerPixel);
			mem_free(RGBTmp);
			break;
		}
		case PP_PIX_FMT_RGB16_5_6_5:
			//memcpy(target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, targetLen<ppOutLen?targetLen:ppOutLen);
			Fit2Display(target, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, OutWidth, OutHeight, BytesPerPixel);
			break;
		default:
			break;
		}
	}
	else if (IF_WRAPPER_OUT)//WRAPPER_TO_YYCBCR_OUT
	{
		Idu_SetCSC();
		//memcpy(target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, targetLen<ppOutLen?targetLen:ppOutLen);
		Fit2Display(target, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, OutWidth, OutHeight, 2);
	}
	else
	{
		Idu_SetCSC();
		MP_DEBUG("Software transform pPpConf->ppOutImg.bufferBusAddr for IDU");
		switch (pPpConf->ppOutImg.pixFormat)
		{
		case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			Pixel_420Semi_Planer_2_422YYCbCr((ST_PIXEL *)target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			break;
		case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			MP_DEBUG("IDU YCbYCr mode");
			idu->TvCtrl0 |= 0x20000000;
			Fit2Display(target, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, OutWidth, OutHeight, 2);
			//Pixel_422YCbYCr_2_422YYCbCr((ST_PIXEL *)target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			break;
		case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
			Pixel_422YCrYCb_2_422YYCbCr((ST_PIXEL *)target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			break;
		case PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED:
			Pixel_422CbYCrY_2_422YYCbCr((ST_PIXEL *)target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			break;
		case PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED:
			Pixel_422CrYCbY_2_422YYCbCr((ST_PIXEL *)target->pdwStart, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			break;
		default:
			MP_DEBUG("NO Properly transform for IDU");
			break;
		}
	}
	MP_DEBUG("idu->TvCtrl0 = 0x%08x", idu->TvCtrl0);
	MP_DEBUG("idu->Iducsc0 = 0x%08x", idu->Iducsc0);
	MP_DEBUG("idu->Iducsc1 = 0x%08x", idu->Iducsc1);
	MP_DEBUG("idu->Iducsc2 = 0x%08x", idu->Iducsc2);
	MP_DEBUG("idu->Iducsc3 = 0x%08x", idu->Iducsc3);
	MP_DEBUG("idu->ATCON_srgb_ctrl =0x%08x", idu->ATCON_srgb_ctrl);

	if (pp_in_write_file)
	{
		if (pp_in_write_raw)
		{
			if (IF_WRAPPER_IN)	sprintf(fname, "wIn422YYCbCr%dx%d", pPpConf->ppInImg.width, pPpConf->ppInImg.height);
			else				sprintf(fname, "ppIn%x_%dx%d", pPpConf->ppInImg.pixFormat, pPpConf->ppInImg.width, pPpConf->ppInImg.height);

			MP_DEBUG("creat %s.%s", fname, extname);
			MP_ASSERT(!CreateFile(sDrv, fname, extname));
			MP_ASSERT(shandle=FileOpen(sDrv));
			MP_ASSERT(FileWrite(shandle, (BYTE*)pPpConf->ppInImg.bufferBusAddr, ppInLen)==ppInLen);
			FileClose(shandle);
			MP_DEBUG("write %s.%s completed", fname, extname);
		}
		fplaner = (BYTE*)mem_malloc(ppInLen);
		MP_ASSERT(fplaner);
		sprintf(fname, "ppInp%x_%dx%d", pPpConf->ppInImg.pixFormat, pPpConf->ppInImg.width, pPpConf->ppInImg.height);
		MP_DEBUG("creat %s.%s", fname, extname);
		MP_ASSERT(!CreateFile(sDrv, fname, extname));
		MP_ASSERT(shandle=FileOpen(sDrv));
		switch (pPpConf->ppInImg.pixFormat)
		{
		case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
			Pixel_420Semi_Planer_2_420Planer(fplaner, (const BYTE*)pPpConf->ppInImg.bufferBusAddr, pPpConf->ppInImg.width* pPpConf->ppInImg.height);
			break;
		case PP_PIX_FMT_YCBCR_4_2_0_TILED:
			//Pixel_420Tiled_2_420Planer
			break;
		case PP_PIX_FMT_YCBCR_4_2_0_PLANAR:
			memcpy(fplaner, (const BYTE*)pPpConf->ppInImg.bufferBusAddr, ppInLen);
			break;
		case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
			Pixel_422YCbYCr_2_422Planer(fplaner, (BYTE*)pPpConf->ppInImg.bufferBusAddr, ppInPixDim);
			break;
		case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
			//		Pixel_422YCrYCb_2_422Planer(fplaner, pPpConf->ppInImg.bufferBusAddr, ppInPixDim);
			break;
		case PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED:
			//		Pixel_422CbYCrY_2_422Planer(fplaner, pPpConf->ppInImg.bufferBusAddr, ppInPixDim);
			break;
		case PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED:
			//		Pixel_422CrYCbY_2_422Planer(fplaner, pPpConf->ppInImg.bufferBusAddr, ppInPixDim);
			break;
		default:
			MP_DEBUG("NO proper transform to Planer");
			break;
		}
		MP_ASSERT(FileWrite(shandle, fplaner, ppInLen)==ppInLen);
		FileClose(shandle);
		MP_DEBUG("write %s.%s completed", fname, extname);
		if (fplaner)
		{
			mem_free(fplaner);
			fplaner = NULL;
		}
	}
	if (pp_out_write_file)
	{
		if (pPpConf->ppOutImg.pixFormat >> 16 == 0x4)
		{//RGB conversion

			switch (pPpConf->ppOutImg.pixFormat)
			{
			case PP_PIX_FMT_RGB32:
				Pixel_RGB32_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_BGR32:
				Pixel_BGR32_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_RGB16_5_6_5:
				Pixel_RGB16_565_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_BGR16_5_6_5:
				Pixel_BGR16_565_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_RGB16_5_5_5:
				Pixel_RGB16_555_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_BGR16_5_5_5:
				Pixel_BGR16_555_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			case PP_PIX_FMT_RGB32_CUSTOM:
			case PP_PIX_FMT_RGB16_CUSTOM:
				Pixel_RGBCustom_2_PPM((BYTE*)pPpConf->ppOutImg.bufferBusAddr, pPpConf);
				break;
			default:
				MP_DEBUG("NO Properly transform for ppm file");
				break;
			}
			if (pp_out_write_raw)
			{
				//pPpConf->ppOutImg.bufferBusAddr write out
				fplaner = (BYTE*)mem_malloc(ppOutLen);
				MP_ASSERT(fplaner);
				sprintf(fname, "ppOut%x_%dx%d_", pPpConf->ppOutImg.pixFormat, pPpConf->ppOutImg.width, pPpConf->ppOutImg.height);
				switch (pPpConf->ppOutImg.pixFormat)
				{
				case PP_PIX_FMT_RGB32:
					strcat(fname, "32");
					break;
				case PP_PIX_FMT_RGB16_5_6_5:
					strcat(fname, "565");
					break;
				case PP_PIX_FMT_RGB16_5_5_5:
					strcat(fname, "555");
					break;
				case PP_PIX_FMT_RGB16_CUSTOM:
					strcat(fname, "16C");
					break;
				case PP_PIX_FMT_RGB32_CUSTOM:
					strcat(fname, "32C");
					break;
				default:
					MP_DEBUG("NO Properly name for raw file");
					break;
				}
				MP_DEBUG("Create File %s.%s", fname, "rgb");
				MP_ASSERT(!CreateFile(sDrv, fname, "rgb"));
				MP_ASSERT(shandle=FileOpen(sDrv));
				MP_ASSERT(FileWrite(shandle, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutLen)==ppOutLen);
				FileClose(shandle);
				MP_DEBUG("File completed %s.%s", fname, "rgb");
				if (fplaner)
				{
					mem_free(fplaner);
					fplaner = NULL;
				}
			}
		}//if (pPpConf.ppOutImg.pixFormat >> 16 == 0x4)
		else
		{
			//YCbCr space
			fplaner = (BYTE*)mem_malloc(ppOutLen);
			MP_ASSERT(fplaner);

			if (IF_WRAPPER_OUT)	sprintf(fname, "wOP%x_%dx%d", pPpConf->ppOutImg.pixFormat, OutWidth, OutHeight);
			else				sprintf(fname, "OP%x_%dx%d", pPpConf->ppOutImg.pixFormat, OutWidth, OutHeight);


			if (pPpConf->ppOutMask1.enable) 		sprintf(fname,"%sM1(%d,%d)%dx%d", fname, pPpConf->ppOutMask1.originX,pPpConf->ppOutMask1.originY, pPpConf->ppOutMask1.width, pPpConf->ppOutMask1.height);
			if (pPpConf->ppOutMask2.enable) 		sprintf(fname,"%sM2(%d,%d)%dx%d", fname, pPpConf->ppOutMask2.originX,pPpConf->ppOutMask2.originY, pPpConf->ppOutMask2.width, pPpConf->ppOutMask2.height);

			MP_DEBUG("Create File %s.%s", fname, extname);
			MP_ASSERT(!CreateFile(sDrv, fname, extname));
			MP_ASSERT(shandle=FileOpen(sDrv));
			if (IF_WRAPPER_OUT)
				Pixel_422YYCbCr_2_422Planer(fplaner, (ST_PIXEL *)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
			else
			{
				switch (pPpConf->ppOutImg.pixFormat)
				{
				case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
					Pixel_422YCbYCr_2_422Planer(fplaner, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
					break;
				case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
					Pixel_420Semi_Planer_2_420Planer(fplaner, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutPixDim);
					break;
				default:
					MP_DEBUG("NO Properly transform for file write Planer");
					break;
				}
			}
			MP_ASSERT(FileWrite(shandle, fplaner, ppOutLen)==ppOutLen);
			FileClose(shandle);
			MP_DEBUG("write %s.%s completed", fname, extname);
			if (fplaner)
			{
				mem_free(fplaner);
				fplaner = NULL;
			}
			if (pp_out_write_raw)
			{
				//ppOut
				if (IF_WRAPPER_OUT)	sprintf(fname, "wO%x_%dx%d", pPpConf->ppOutImg.pixFormat, pPpConf->ppOutImg.width, pPpConf->ppOutImg.height);
				else				sprintf(fname, "ppO%x_%dx%d", pPpConf->ppOutImg.pixFormat, pPpConf->ppOutImg.width, pPpConf->ppOutImg.height);
				
				MP_DEBUG("Create File %s.%s", fname, extname);
				MP_ASSERT(!CreateFile(sDrv, fname, extname));
				MP_ASSERT(shandle=FileOpen(sDrv));
				MP_ASSERT(FileWrite(shandle, (BYTE*)pPpConf->ppOutImg.bufferBusAddr, ppOutLen)==ppOutLen);
				FileClose(shandle);
				MP_DEBUG("Write File %s.%s completed", fname, extname);
			}
		}
	}

	if (ppOut)
	{
		mem_free(ppOut);
		ppOut = NULL;
	}
	Idu_ChgWin (target);//change for iner loop
	TimerDelay(TEST_DELAY);
}



static void rand_down_scaling(PPInst const pp, PPConfig * const pPpConf)
{
	u32 OutWidth = pPpConf->ppInImg.width;
	u32 OutHeight = pPpConf->ppInImg.height;

	if (pPpConf->ppInRotation.rotation && pPpConf->ppInRotation.rotation<3)
	{
		OutWidth = pPpConf->ppOutImg.width;
		OutHeight = pPpConf->ppOutImg.height;
	}
	else if (pPpConf->ppInCrop.enable)
	{
		OutWidth = pPpConf->ppInCrop.width;
		OutHeight = pPpConf->ppInCrop.height;
	}

	srand(GetSysTime());
	{	//downscale
		pPpConf->ppOutImg.width = RangedRand(PP_OUT_MIN_WIDTH, OutWidth, 8);
		pPpConf->ppOutImg.height = RangedRand(PP_OUT_MIN_HEIGHT, OutHeight, 2);
	}

}

static void rand_up_scaling(PPInst const pp, PPConfig * const pPpConf)
{
	u32 OutWidth = pPpConf->ppInImg.width;
	u32 OutHeight = pPpConf->ppInImg.height;

	if (pPpConf->ppInRotation.rotation && pPpConf->ppInRotation.rotation<3)
	{
		OutWidth = pPpConf->ppOutImg.width;
		OutHeight = pPpConf->ppOutImg.height;
	}
	else if (pPpConf->ppInCrop.enable)
	{
		OutWidth = pPpConf->ppInCrop.width;
		OutHeight = pPpConf->ppInCrop.height;
	}

	PPContainer *ppC = (PPContainer *)pp;
	u32 maxOutWidth 	= PP_OUT_MAX_WIDTH_UPSCALED(OutWidth, 0);
	u32 maxOutHeight	= PP_OUT_MAX_HEIGHT_UPSCALED(OutHeight, 0);
	if (ppC->maxOutWidth < maxOutWidth) 	maxOutWidth = ppC->maxOutWidth;
	if (ppC->maxOutHeight < maxOutHeight)	maxOutHeight = ppC->maxOutHeight;
	srand(GetSysTime());
	{	//upscale
		pPpConf->ppOutImg.width = RangedRand(OutWidth, maxOutWidth, 8);
		pPpConf->ppOutImg.height = RangedRand(OutHeight, maxOutHeight, 2);
	}
}

static void rand_scaling(PPInst const pp, PPConfig * const pPpConf)
{
	srand(GetSysTime());
	if (RAND_ON)	rand_up_scaling(pp, pPpConf);
	else			rand_down_scaling(pp, pPpConf);
}

static void rand_range_scaling(PPInst const pp, PPConfig * const pPpConf, const u32 minWidth, const u32 maxWidth, const u32 minHeight, const u32 maxHeight)
{
	MP_DEBUG("rand_range_scaling");
	srand(GetSysTime());
	do
	{
		rand_scaling(pp, pPpConf);
	}
	while (pPpConf->ppOutImg.width < minWidth || pPpConf->ppOutImg.width > maxWidth ||
	        pPpConf->ppOutImg.height < minHeight || pPpConf->ppOutImg.height > maxHeight);
	MP_DEBUG("rand_range_scaling return");
}

static void reset_scaling(PPConfig * const pPpConf)
{
	pPpConf->ppOutImg.width = pPpConf->ppInImg.width;//mul of 8
	pPpConf->ppOutImg.height =  pPpConf->ppInImg.height;//mul of 2
}

static void rand_rotation(PPConfig * const pPpConf)
{
	srand(GetSysTime());
	u32 OutWidth, OutHeight;
	if (pPpConf->ppInCrop.enable)
	{
		OutWidth = pPpConf->ppInCrop.width;
		OutHeight = pPpConf->ppInCrop.height;
	}
	else
	{
		OutWidth = pPpConf->ppInImg.width;
		OutHeight = pPpConf->ppInImg.height;
	}
	pPpConf->ppInRotation.rotation = RangedRand(PP_ROTATION_NONE, PP_ROTATION_180, 1);
	if (pPpConf->ppInRotation.rotation && pPpConf->ppInRotation.rotation<3)
	{
		pPpConf->ppOutImg.width = OutHeight;
		pPpConf->ppOutImg.height = OutWidth;
	}
	else
	{
		pPpConf->ppOutImg.width = OutWidth;
		pPpConf->ppOutImg.height = OutHeight;
	}
}

static void reset_rotation(PPConfig * const pPpConf)
{
	pPpConf->ppInRotation.rotation = PP_ROTATION_NONE;
	reset_scaling(pPpConf);
}

static void rand_crop(PPInst const pp, PPConfig * const pPpConf)
{
	PPContainer *ppC = (PPContainer *)pp;
	srand(GetSysTime());
	pPpConf->ppInCrop.enable = 1;
	pPpConf->ppInCrop.originX	= RangedRand(0, pPpConf->ppInImg.width, 16);	//mul of 16
	pPpConf->ppInCrop.originY	= RangedRand(0, pPpConf->ppInImg.height, 16);	//mul of 16
	pPpConf->ppInCrop.width		= RangedRand(PP_IN_MIN_WIDTH(ppC->decType), pPpConf->ppInImg.width-pPpConf->ppInCrop.originX, 8);	//mul of 8
	pPpConf->ppInCrop.height	= RangedRand(PP_IN_MIN_HEIGHT(ppC->decType), pPpConf->ppInImg.height-pPpConf->ppInCrop.originY, 8);	//mul of 8

	pPpConf->ppOutImg.width = pPpConf->ppInCrop.width;
	pPpConf->ppOutImg.height =	pPpConf->ppInCrop.height;
}

static void reset_crop(PPConfig * const pPpConf)
{
	pPpConf->ppInCrop.enable = 0;
	reset_scaling(pPpConf);
}

static void rand_OutFrmBuffer_targetwin(ST_IMGWIN * const target, PPInst const pp, PPConfig * const pPpConf)
{
	//this random OutFrmBuffer fit to target win
	pPpConf->ppOutFrmBuffer.enable = 1;
	rand_down_scaling(pp, pPpConf);
	srand(GetSysTime());
	pPpConf->ppOutFrmBuffer.writeOriginX = RangedRand(-pPpConf->ppOutImg.width+2, pPpConf->ppOutImg.width-2, 2);//mul of 2
	pPpConf->ppOutFrmBuffer.writeOriginY = RangedRand(-pPpConf->ppOutImg.height+1, pPpConf->ppOutImg.height-1, 1);	//mul of 1
	pPpConf->ppOutFrmBuffer.frameBufferWidth = FLOOR_MUL(target->wWidth, 2);
	pPpConf->ppOutFrmBuffer.frameBufferHeight = FLOOR_MUL(target->wHeight, 1);
}

static void rand_OutFrmBuffer(ST_IMGWIN * const target, PPInst const pp, PPConfig * const pPpConf)
{
	PPContainer *ppC = (PPContainer *)pp;
	pPpConf->ppOutFrmBuffer.enable = 1;
	srand(GetSysTime());
	pPpConf->ppOutFrmBuffer.frameBufferWidth = RangedRand(16, PP_MAX_FRM_BUFF_WIDTH, 2);
	pPpConf->ppOutFrmBuffer.frameBufferHeight = RangedRand(16, ppC->maxOutHeight, 1);//not restricted
	rand_range_scaling(pp, pPpConf, PP_OUT_MIN_WIDTH, pPpConf->ppOutFrmBuffer.frameBufferWidth, PP_OUT_MIN_HEIGHT, pPpConf->ppOutFrmBuffer.frameBufferHeight);
	pPpConf->ppOutFrmBuffer.writeOriginX = RangedRand(-pPpConf->ppOutImg.width+2, pPpConf->ppOutImg.width-2, 4);//multiply of 2 in spec. actually 4
	pPpConf->ppOutFrmBuffer.writeOriginY = RangedRand(-pPpConf->ppOutImg.height+1, pPpConf->ppOutImg.height-1, 1);	//mul of 1
}


static void reset_OutFrmBuffer(PPConfig * const pPpConf)
{
	pPpConf->ppOutFrmBuffer.enable = 0;
	reset_scaling(pPpConf);
}

static void rand_OutMask1(PPConfig * const pPpConf)
{
	srand(GetSysTime());
	pPpConf->ppOutMask1.enable = 1;
	pPpConf->ppOutMask1.originX	= RangedRand(0, pPpConf->ppOutImg.width, 4);		//multiply of 2 in spec. actually 4
	pPpConf->ppOutMask1.originY	= RangedRand(0, pPpConf->ppOutImg.height, 1);
	pPpConf->ppOutMask1.width	= RangedRand(2, pPpConf->ppOutImg.width-pPpConf->ppOutMask1.originX, 4);//multiply of 2 in spec. actually 4
	pPpConf->ppOutMask1.height	= RangedRand(1, pPpConf->ppOutImg.height-pPpConf->ppOutMask1.originY, 1);

}
static void reset_OutMask1(PPConfig * const pPpConf)
{
	pPpConf->ppOutMask1.enable = 0;
}
static void rand_OutMask2(PPConfig * const pPpConf)
{
	srand(GetSysTime());
	pPpConf->ppOutMask2.enable = 1;
	pPpConf->ppOutMask2.originX	= RangedRand(0, pPpConf->ppOutImg.width, 4);//multiply of 2 in spec. actually 4
	pPpConf->ppOutMask2.originY	= RangedRand(0, pPpConf->ppOutImg.height, 1);
	pPpConf->ppOutMask2.width	= RangedRand(2, pPpConf->ppOutImg.width-pPpConf->ppOutMask2.originX, 4);//multiply of 2 in spec. actually 4
	pPpConf->ppOutMask2.height	= RangedRand(1, pPpConf->ppOutImg.height-pPpConf->ppOutMask2.originY, 1);
}
static void reset_OutMask2(PPConfig * const pPpConf)
{
	pPpConf->ppOutMask2.enable = 0;
}




static void set_rgb(PPConfig * const pPpConf)
{
	pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	WRAPPER_OUT_OFF;
}

static void set_rgb16_444(PPConfig * const pPpConf)
{
	set_rgb(pPpConf);
	srand(GetSysTime());
	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_RGB16_CUSTOM;
	if (RAND_ON)	pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	else			pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_709;
	//RGB 444
	pPpConf->ppOutRgb.rgbBitmask.maskR = 0x0F00;
	pPpConf->ppOutRgb.rgbBitmask.maskG = 0x00F0;
	pPpConf->ppOutRgb.rgbBitmask.maskB = 0x000F;
	pPpConf->ppOutRgb.rgbBitmask.maskAlpha = 0;
}

static void set_rgb32_AAA(PPConfig * const pPpConf)
{
	set_rgb(pPpConf);
	srand(GetSysTime());
	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_RGB32_CUSTOM;
	if (RAND_ON)	pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	else			pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_709;
	//RGB 444
	pPpConf->ppOutRgb.rgbBitmask.maskR = 0x3FF00000;
	pPpConf->ppOutRgb.rgbBitmask.maskG = 0x000FFC00;
	pPpConf->ppOutRgb.rgbBitmask.maskB = 0x000003FF;
	pPpConf->ppOutRgb.rgbBitmask.maskAlpha = 0;
}


static void set_rgb16_565(PPConfig * const pPpConf)
{
	set_rgb(pPpConf);
	srand(GetSysTime());
	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_RGB16_5_6_5;
	if (RAND_ON)	pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	else			pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_709;
}
static void set_rgb32(PPConfig * const pPpConf)
{
	set_rgb(pPpConf);
	srand(GetSysTime());
	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_RGB32;
	if (RAND_ON)	pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601;
	else			pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_709;
}
static void reset_rgb(PPConfig * const pPpConf)
{
	WRAPPER_OUT_ON;
	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	memset(&pPpConf->ppOutRgb, 0, sizeof(PPOutRgb));
}

static void set_deinterlace(PPConfig * const pPpConf)
{
	pPpConf->ppOutDeinterlace.enable = 1;
}
static void reset_deinterlace(PPConfig * const pPpConf)
{
	pPpConf->ppOutDeinterlace.enable = 0;
}
static void set_dithering(PPConfig * const pPpConf)
{
	pPpConf->ppOutRgb.ditheringEnable = 1;
}
static void reset_dithering(PPConfig * const pPpConf)
{
	pPpConf->ppOutRgb.ditheringEnable = 0;
}


void static reset_PPConfig(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                           PPInst const pp, PPConfig * const pPpConf)
{
	PPContainer *ppC = (PPContainer *)pp;
	pPpConf->ppInImg.width = ALIGN_CUT_16(source->wWidth);//mul of 16
	pPpConf->ppInImg.height = ALIGN_CUT_16(source->wHeight);//mul of 16

	WRAPPER_SCALING_RAM;
	ppC->MPXWrapper = 1;
	WRAPPER_IN_ON;
	pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;

	pPpConf->ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
	pPpConf->ppInImg.bufferBusAddr = (u32)source->pdwStart;//check 8 algined
	pPpConf->ppOutDeinterlace.enable = 0;

	pp_in_write_file = pp_in_write_raw = pp_out_write_file = pp_out_write_raw = 0;

	WRAPPER_OUT_ON;
	//WRAPPER_WR_REQ_SEPARATE;
	//WRAPPER_RD_REQ_SEPARATE;

	pPpConf->ppOutImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	reset_crop(pPpConf);
	reset_rotation(pPpConf);
	reset_deinterlace(pPpConf);
	reset_rgb(pPpConf);
	reset_scaling(pPpConf);
	reset_dithering(pPpConf);
	reset_OutFrmBuffer(pPpConf);
	reset_OutMask1(pPpConf);
	reset_OutMask2(pPpConf);
}

const u32 InFormatSet[]=
{
//	PP_PIX_FMT_YCBCR_4_2_0_PLANAR,	//test in deinterlace
//	PP_PIX_FMT_YCBCR_4_2_0_TILED,
//	PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR,//test in deinterlace
	PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED,
	PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED,
	PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED,
	PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED,
	0
};
const u32 OutFormatSet[]=
{
	PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR,	//ok, but fail to display
	PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED,	//ok
	PP_PIX_FMT_RGB32,					//ok, but fail to display as 565
	PP_PIX_FMT_BGR32,					//ok, but fail to display as 565
	PP_PIX_FMT_RGB16_5_5_5,				//ok, but fail to display as 565
	PP_PIX_FMT_BGR16_5_5_5,				//ok, but fail to display as 565
	PP_PIX_FMT_RGB16_5_6_5,				//ok, but fail to display as 565
	PP_PIX_FMT_BGR16_5_6_5,				//ok, but fail to display as 565
	PP_PIX_FMT_RGB16_CUSTOM,			//ok, but fail to display as 565
	PP_PIX_FMT_RGB32_CUSTOM,			//ok, but fail to display as 565
	0
};

static void alloc_InPixelFormat(PPConfig * const pPpConf, const ST_IMGWIN * const source)
{
	//should mem_free ppIn in the loop before next pp_once
	const u32 ppInPixDim = pPpConf->ppInImg.width* pPpConf->ppInImg.height;
	u32 ppInLen = ImageLength(pPpConf, 0);
	BYTE * ppIn = (BYTE*)mem_malloc(ppInLen);
	MP_ASSERT(ppIn);

	switch (pPpConf->ppInImg.pixFormat)
	{
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422YCbYCr(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422YCrYCb(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422CbYCrY(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED:
		Pixel_422YYCbCr_2_422CrYCbY(ppIn, source->pdwStart, ppInPixDim);
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		Pixel_422YYCbCr_2_420Semi_Planer(ppIn, source->pdwStart, source->wWidth, source->wHeight);
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_PLANAR:
		Pixel_422YYCbCr_2_420Planer(ppIn, source->pdwStart, source->wWidth, source->wHeight);
		break;
	case PP_PIX_FMT_YCBCR_4_2_0_TILED:
	{
		BYTE * P_420semi = (BYTE*)mem_malloc(ppInLen);
		MP_ASSERT(P_420semi);
		Pixel_422YYCbCr_2_420Semi_Planer(P_420semi, source->pdwStart, source->wWidth, source->wHeight);
		Pixel_420Semi_Planer_2_420Tiled(ppIn, P_420semi, source->wWidth, source->wHeight);
		mem_free(P_420semi);
		break;
	}
	default:
		MP_DEBUG("NO proper transform for ppIn");
		break;
	}

	pPpConf->ppInImg.bufferBusAddr = (u32)ppIn;
//		MP_DEBUG("ppInImg.bufferBusAddr: 0x%x", pPpConf->ppInImg.bufferBusAddr);
	pPpConf->ppInImg.bufferCbBusAddr = pPpConf->ppInImg.bufferBusAddr + ppInPixDim;
	pPpConf->ppInImg.bufferCrBusAddr = pPpConf->ppInImg.bufferCbBusAddr + ppInPixDim/4;
}

static void rand_InPixelFormat(PPConfig * const pPpConf, const ST_IMGWIN * const source)
{
	srand(GetSysTime());
	WRAPPER_IN_OFF;
	u32 i=RangedRand(0, sizeof(InFormatSet)/sizeof(u32)-1, 1);
	pPpConf->ppInImg.pixFormat = InFormatSet[i];

	alloc_InPixelFormat(pPpConf, source);
}
static void reset_InPixelFormat(PPConfig * const pPpConf, const ST_IMGWIN * const source)
{
	//should mem_free pPpConf->ppInImg.pixFormat in the loop before next pp_once
	if (!IF_WRAPPER_IN)
	{
		WRAPPER_IN_ON;
		pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
		pPpConf->ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
		pPpConf->ppInImg.bufferBusAddr = (u32)source->pdwStart;
	}
}
static void check_OutPixelFormat(PPConfig * const pPpConf)
{
	if (pPpConf->ppOutImg.pixFormat >> 16 == 0x4)
	{
		if (pPpConf->ppOutImg.pixFormat == PP_PIX_FMT_RGB16_CUSTOM)
			set_rgb16_444(pPpConf);
		else if (pPpConf->ppOutImg.pixFormat == PP_PIX_FMT_RGB32_CUSTOM)
			set_rgb32_AAA(pPpConf);
		else
			set_rgb(pPpConf);
	}
}
static void rand_OutPixelFormat(PPConfig * const pPpConf)
{
	srand(GetSysTime());
	WRAPPER_OUT_OFF;
	u32 i=RangedRand(0, sizeof(OutFormatSet)/sizeof(u32)-1, 1);
	pPpConf->ppOutImg.pixFormat = OutFormatSet[i];
	check_OutPixelFormat(pPpConf);
}
static void reset_OutPixelFormat(PPConfig * const pPpConf)
{
	reset_rgb(pPpConf);
}

static void auto_PP_rotation(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                             PPInst const pp, PPConfig * const pPpConf)
{
	MP_DEBUG("auto_PP_rotation");
	pPpConf->ppInRotation.rotation = PP_ROTATION_NONE;
	for (; pPpConf->ppInRotation.rotation<=PP_ROTATION_180; pPpConf->ppInRotation.rotation++)
	{
		if (pPpConf->ppInRotation.rotation) MP_DEBUG("rotation: %d", pPpConf->ppInRotation.rotation);

		if (pPpConf->ppInRotation.rotation && pPpConf->ppInRotation.rotation<3)
		{
			pPpConf->ppOutImg.width = pPpConf->ppInImg.height;
			pPpConf->ppOutImg.height = pPpConf->ppInImg.width;
		}
		else
		{
			pPpConf->ppOutImg.width = pPpConf->ppInImg.width;
			pPpConf->ppOutImg.height = pPpConf->ppInImg.height;
		}
		PP_once(target, source, pp, pPpConf);
	}
	reset_rotation(pPpConf);
}

static void auto_PP_scaling(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                            PPInst const pp, PPConfig * const pPpConf)
{
	WRAPPER_SCALING_RAM;
	const u32 StepMultiplier = 2;
	MP_DEBUG("auto_PP_scaling width step %d height step %d", 8*StepMultiplier, 2*StepMultiplier);
	PPContainer *ppC = (PPContainer *)pp;

	u32 maxOutWidth 	= PP_OUT_MAX_WIDTH_UPSCALED(pPpConf->ppInImg.width, 0);
	u32 maxOutHeight	= PP_OUT_MAX_HEIGHT_UPSCALED(pPpConf->ppInImg.height, 0);
	if (ppC->maxOutWidth < maxOutWidth) 	maxOutWidth = ppC->maxOutWidth;
	if (ppC->maxOutHeight < maxOutHeight)	maxOutHeight = ppC->maxOutHeight;
	const u32 maxDownWidth = min(maxOutWidth, pPpConf->ppInImg.width);
	const u32 maxDownHeight = min(maxOutHeight, pPpConf->ppInImg.height);

	//down scale
	for (pPpConf->ppOutImg.height=PP_OUT_MIN_HEIGHT; pPpConf->ppOutImg.height<=maxDownHeight; pPpConf->ppOutImg.height+=2 * StepMultiplier)
		for (pPpConf->ppOutImg.width=PP_OUT_MIN_WIDTH; pPpConf->ppOutImg.width<=maxDownWidth ; pPpConf->ppOutImg.width+=8 * StepMultiplier)
			PP_once(target, source, pp, pPpConf);

	//up scale
	for (pPpConf->ppOutImg.height=pPpConf->ppInImg.height; pPpConf->ppOutImg.height<=maxOutHeight; pPpConf->ppOutImg.height+=2 * StepMultiplier)
		for (pPpConf->ppOutImg.width=pPpConf->ppInImg.width; pPpConf->ppOutImg.width<=maxOutWidth ; pPpConf->ppOutImg.width+=8 * StepMultiplier)
			PP_once(target, source, pp, pPpConf);

	reset_scaling(pPpConf);
}

static void auto_PP_input_resolution(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                     PPInst const pp, PPConfig * const pPpConf)
{
	//this one must test alone
	MP_DEBUG("auto_PP_input_resolution");
	PPContainer *ppC = (PPContainer *)pp;

	u32 maxOutWidth 	= PP_OUT_MAX_WIDTH_UPSCALED(pPpConf->ppInImg.width, 0);
	u32 maxOutHeight	= PP_OUT_MAX_HEIGHT_UPSCALED(pPpConf->ppInImg.height, 0);
	if (ppC->maxOutWidth < maxOutWidth) 	maxOutWidth = ppC->maxOutWidth;
	if (ppC->maxOutHeight < maxOutHeight)	maxOutHeight = ppC->maxOutHeight;
	const u32 maxDownWidth = min(maxOutWidth, pPpConf->ppInImg.width);
	const u32 maxDownHeight = min(maxOutHeight, pPpConf->ppInImg.height);

	if (pPpConf->ppInImg.width>=target->wWidth && pPpConf->ppInImg.height>=target->wHeight)
	{
		pPpConf->ppOutImg.width = target->wWidth;
		pPpConf->ppOutImg.height = target->wHeight;
	}
	else
	{
		pPpConf->ppOutImg.width = maxOutWidth;
		pPpConf->ppOutImg.height = maxOutHeight;
	}
	PP_once(target, source, pp, pPpConf);
	reset_scaling(pPpConf);
}


static void auto_PP_scaling_rand(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                 PPInst const pp, PPConfig * const pPpConf)
{
	u32 count = RANDOM_PP_TEST_COUNT;
	while (count--)
	{
		MP_DEBUG("auto_PP_Scale_OutFrmBuffer loop %d", count);
		rand_scaling(pp, pPpConf);
		PP_once(target, source, pp, pPpConf);
	}
	reset_scaling(pPpConf);
}

static void auto_PP_crop(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                         PPInst const pp, PPConfig * const pPpConf)
{
	MP_DEBUG("auto_PP_crop");
	u32 count = RANDOM_PP_TEST_COUNT;
	while (count--)
	{
		rand_crop(pp, pPpConf);
		PP_once(target, source, pp, pPpConf);
	}
	reset_crop(pPpConf);
}

static void auto_PP_OutFrmBuffer(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                 PPInst const pp, PPConfig * const pPpConf)
{
	pp_out_write_file = 0;
	u32 count = RANDOM_PP_TEST_COUNT;
	while (count--)
	{
		MP_DEBUG("auto_PP_OutFrmBuffer loop %d", count);
		rand_OutFrmBuffer(target, pp, pPpConf);
		PP_once(target, source, pp, pPpConf);
	}
	reset_OutFrmBuffer(pPpConf);
}

static void auto_PP_OutMask1(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                             PPInst const pp, PPConfig * const pPpConf)
{
	pp_out_write_file = 0;
	u32 count = RANDOM_PP_TEST_COUNT;
	while (count--)
	{
		MP_DEBUG("auto_PP_OutMask1 loop %d", count);
		rand_OutMask1(pPpConf);
		PP_once(target, source, pp, pPpConf);
	}
	reset_OutMask1(pPpConf);
	pp_out_write_file = 0;
}
static void auto_PP_OutMask2(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                             PPInst const pp, PPConfig * const pPpConf)
{
	pp_out_write_file = 0;
	u32 count = RANDOM_PP_TEST_COUNT;
	while (count--)
	{
		MP_DEBUG("auto_PP_OutMask2 loop %d", count);
		rand_OutMask2(pPpConf);
		PP_once(target, source, pp, pPpConf);
	}
	reset_OutMask2(pPpConf);
	pp_out_write_file = 0;
}

static void auto_PP_RGB16_565(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                              PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb16_565(pPpConf);
	pp_out_write_file = 0;
	pp_in_write_raw = 0;
	for (pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601; pPpConf->ppOutRgb.rgbTransform <= PP_YCBCR2RGB_TRANSFORM_BT_709; pPpConf->ppOutRgb.rgbTransform++)
		PP_once(target, source, pp, pPpConf);
	pp_out_write_file = 0;
	reset_rgb(pPpConf);
}

static void auto_PP_RGB32(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                          PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb32(pPpConf);
	pp_out_write_file = 0;
	for (pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601; pPpConf->ppOutRgb.rgbTransform <= PP_YCBCR2RGB_TRANSFORM_BT_709; pPpConf->ppOutRgb.rgbTransform++)
		PP_once(target, source, pp, pPpConf);
	pp_out_write_file = 0;
	reset_rgb(pPpConf);
}

static void auto_PP_contrast(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                             PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb16_565(pPpConf);
	for (pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601; pPpConf->ppOutRgb.rgbTransform <= PP_YCBCR2RGB_TRANSFORM_BT_709; pPpConf->ppOutRgb.rgbTransform++)
	{
		for (pPpConf->ppOutRgb.contrast = 0; pPpConf->ppOutRgb.contrast <= 64; pPpConf->ppOutRgb.contrast +=4)
			PP_once(target, source, pp, pPpConf);
		for (pPpConf->ppOutRgb.contrast = 0; pPpConf->ppOutRgb.contrast >= -64; pPpConf->ppOutRgb.contrast -=4)
			PP_once(target, source, pp, pPpConf);
	}
	reset_rgb(pPpConf);
}

static void auto_PP_brightness(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                               PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb16_565(pPpConf);
	for (pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601; pPpConf->ppOutRgb.rgbTransform <= PP_YCBCR2RGB_TRANSFORM_BT_709; pPpConf->ppOutRgb.rgbTransform++)
	{
		for (pPpConf->ppOutRgb.brightness = 0; pPpConf->ppOutRgb.brightness <= 127; pPpConf->ppOutRgb.brightness +=8)
			PP_once(target, source, pp, pPpConf);
		for (pPpConf->ppOutRgb.brightness = 0; pPpConf->ppOutRgb.brightness >= -128; pPpConf->ppOutRgb.brightness -=8)
			PP_once(target, source, pp, pPpConf);
	}
	reset_rgb(pPpConf);
}
static void auto_PP_saturation(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                               PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb16_565(pPpConf);
	for (pPpConf->ppOutRgb.rgbTransform = PP_YCBCR2RGB_TRANSFORM_BT_601; pPpConf->ppOutRgb.rgbTransform <= PP_YCBCR2RGB_TRANSFORM_BT_709; pPpConf->ppOutRgb.rgbTransform++)
	{
		for (pPpConf->ppOutRgb.saturation = 0; pPpConf->ppOutRgb.saturation <= 128; pPpConf->ppOutRgb.saturation +=8)
			PP_once(target, source, pp, pPpConf);
		for (pPpConf->ppOutRgb.saturation = 0; pPpConf->ppOutRgb.saturation >= -64; pPpConf->ppOutRgb.saturation -=8)
			PP_once(target, source, pp, pPpConf);
	}
	reset_rgb(pPpConf);
}

static void auto_PP_dithering(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                              PPInst const pp, PPConfig * const pPpConf)
{
	set_rgb16_444(pPpConf);
	MP_DEBUG("dithering OFF RGB444");
	pp_out_write_file = 0;
	PP_once(target, source, pp, pPpConf);

	MP_DEBUG("dithering ON RGB444");
	set_dithering(pPpConf);
	PP_once(target, source, pp, pPpConf);
	pp_out_write_file = 0;
	reset_dithering(pPpConf);
	reset_rgb(pPpConf);
}

static void auto_PP_InImg_PixelFormat(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                      PPInst const pp, PPConfig * const pPpConf)
{
	MP_DEBUG("auto_PP_InImg_PixelFormat");
	WRAPPER_IN_OFF;
	u32 i=0;

	pp_in_write_file = 0;
	pp_in_write_raw = 0;

	while ((pPpConf->ppInImg.pixFormat = InFormatSet[i++])!=0)
	{
		MP_DEBUG("Test ppInImg.pixFormat 0x%x", pPpConf->ppInImg.pixFormat);
		alloc_InPixelFormat(pPpConf, source);
		PP_once(target, source, pp, pPpConf);

		mem_free((BYTE*)pPpConf->ppInImg.bufferBusAddr);
		pPpConf->ppInImg.bufferBusAddr = (u32)NULL;
	}

	reset_InPixelFormat(pPpConf, source);
}

static void auto_PP_OutImg_PixelFormat(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                       PPInst const pp, PPConfig * const pPpConf)
{
	MP_DEBUG("auto_PP_OutImg_PixelFormat");
	WRAPPER_OUT_OFF;
	u32 i=0;

//	pp_out_write_file = 1;
	while ((pPpConf->ppOutImg.pixFormat = OutFormatSet[i++])!=0)
	{
		MP_DEBUG("Test ppOutImg.pixFormat 0x%x", pPpConf->ppOutImg.pixFormat);
//		GetUartChar();
		check_OutPixelFormat(pPpConf);
		PP_once(target, source, pp, pPpConf);
	}

	reset_OutPixelFormat(pPpConf);
	pp_out_write_file = 0;
}

static void auto_PP_IDU_422YCbYCr(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                  PPInst const pp, PPConfig * const pPpConf)
{
	MP_DEBUG("auto_PP_IDU_422YCbYCr");
	WRAPPER_OUT_OFF;
	register IDU *idu = (IDU *) IDU_BASE;
	PP_once(target, source, pp, pPpConf);
	WRAPPER_OUT_ON;
}

static void auto_PP_Deinterlace(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                                PPInst const pp, PPConfig * const pPpConf)
{
	/*
		Read an interlaced YCbCr420 planer file, seperate to 2 field in 420 semi-planer format
		as pp input, and perform deinterce.
	*/
	MP_DEBUG("auto_PP_Deinterlace");
	DRIVE *sDrv = initial_mcard(SD_MMC);
	STREAM *shandle;
	char fname[]="inter";
	char extname[]="yuv";

	pPpConf->ppInImg.width = 720;
	pPpConf->ppInImg.height = 480;
	pPpConf->ppOutImg.width = pPpConf->ppInImg.width;
	pPpConf->ppOutImg.height = pPpConf->ppInImg.height;
	pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_PLANAR;
	const u32 ppInPixDim = pPpConf->ppInImg.width* pPpConf->ppInImg.height;
	const u32 ppInLen = ImageLength(pPpConf, 0);
	BYTE *ppIn = (BYTE*)mem_malloc(ppInLen);
	MP_ASSERT(ppIn);
	BYTE * top		= NULL;
	BYTE * bottom	= NULL;
	MP_DEBUG("read %s.%s", fname, extname);

	if (FileSearch(sDrv, fname, extname, E_FILE_TYPE) == FS_SUCCEED)
	{
		if (!(shandle = FileOpen(sDrv)))
			MP_ALERT("open file %s.%s fail", fname, extname);
		if (FileRead(shandle, ppIn, ppInLen) != ppInLen)
			MP_ALERT("FileRead error");

		FileClose(shandle);
	}
	else
	{
		MP_ALERT("No such file %s.%s", fname, extname);
		return;
	}

	//scale to display input
	pPpConf->ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
	pPpConf->ppInImg.bufferBusAddr = (u32)ppIn;
	pPpConf->ppInImg.bufferCbBusAddr = pPpConf->ppInImg.bufferBusAddr + ppInPixDim;
	pPpConf->ppInImg.bufferCrBusAddr = pPpConf->ppInImg.bufferCbBusAddr + ppInPixDim/4;
	WRAPPER_IN_OFF;
	PP_once(target, source, pp, pPpConf);

	//test PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR by the way
	pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
	BYTE *splaner = (BYTE*)mem_malloc(ppInLen);
	Pixel_420Planer_2_420Semi_Planer(splaner, ppIn, ppInPixDim);
	pPpConf->ppInImg.bufferBusAddr = (u32)splaner;
	pPpConf->ppInImg.bufferCbBusAddr = pPpConf->ppInImg.bufferBusAddr + ppInPixDim;
	pPpConf->ppInImg.bufferCrBusAddr = pPpConf->ppInImg.bufferCbBusAddr + ppInPixDim/4;
	/*	pp_in_write_file = 1;
		pp_in_write_raw = 1;
		pp_out_write_file = 1;
		pp_out_write_raw = 1;
	*/
	PP_once(target, source, pp, pPpConf);

	//test PP_PIX_FMT_YCBCR_4_2_0_TILED by the way
	pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_TILED;
	BYTE *tile = (BYTE*)mem_malloc(ppInLen);
	Pixel_420Semi_Planer_2_420Tiled(tile, splaner, pPpConf->ppInImg.width, pPpConf->ppInImg.height);
	pPpConf->ppInImg.bufferBusAddr = (u32)tile;
	pPpConf->ppInImg.bufferCbBusAddr = pPpConf->ppInImg.bufferBusAddr + ppInPixDim;
	pPpConf->ppInImg.bufferCrBusAddr = pPpConf->ppInImg.bufferCbBusAddr + ppInPixDim/4;
	PP_once(target, source, pp, pPpConf);
	pp_in_write_raw = 0;
	pp_in_write_file = 0;

	mem_free(splaner);
	mem_free(tile);

	MP_DEBUG("Press any key to deinterlace");
	GetUartChar();
	pPpConf->ppInImg.pixFormat = PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
	pp_out_write_file = 0;
	set_deinterlace(pPpConf);
	pPpConf->ppInImg.picStruct = PP_PIC_TOP_AND_BOT_FIELD;
	top = (BYTE*)mem_malloc(ppInLen/2);
	bottom = (BYTE*)mem_malloc(ppInLen/2);
	MP_ASSERT(top && bottom);
	SeperateField(top, bottom, ppIn, pPpConf->ppInImg.width, pPpConf->ppInImg.height);

	pPpConf->ppInImg.bufferBusAddr = (u32)top;
	pPpConf->ppInImg.bufferCbBusAddr = pPpConf->ppInImg.bufferBusAddr + ppInPixDim/2;
	pPpConf->ppInImg.bufferBusAddrBot = (u32)bottom;
	pPpConf->ppInImg.bufferBusAddrChBot = pPpConf->ppInImg.bufferBusAddrBot + ppInPixDim/2;
	if (ppIn)
	{
		mem_free(ppIn);
		ppIn = NULL;
	}

	PP_once(target, source, pp, pPpConf);

	if (top)
	{
		mem_free(top);
		top = NULL;
	}
	if (bottom)
	{
		mem_free(bottom);
		bottom = NULL;
	}
	reset_PPConfig(target, source, pp, pPpConf);
}


static void auto_PP_random(ST_IMGWIN * const target, const ST_IMGWIN * const source,
                           PPInst const pp, PPConfig * const pPpConf)
{
	srand(GetSysTime());
	u32 count = RANDOM_PP_TEST_COUNT*2;

	while (count--)
	{
		if (RAND_ON)	rand_crop(pp, pPpConf);
		if (RAND_ON)	rand_rotation(pPpConf);
		if (RAND_ON)
		{
			set_rgb16_565(pPpConf);
			if (RAND_ON)	pPpConf->ppOutRgb.contrast		= RangedRand(-64, 64, 1);
			if (RAND_ON)	pPpConf->ppOutRgb.brightness	= RangedRand(-128, 127, 1);
			if (RAND_ON)	pPpConf->ppOutRgb.saturation	= RangedRand(-64, 128, 1);
			//	if (RAND_ON)	rand_dithering(pp, pPpConf);
		}
		if (RAND_ON)	rand_scaling(pp, pPpConf);
		if (RAND_ON)	rand_OutFrmBuffer(target, pp, pPpConf);
		if (RAND_ON)	rand_OutMask1(pPpConf);
		if (RAND_ON)	rand_OutMask2(pPpConf);

		PP_once(target, source, pp, pPpConf);

		if (!IF_WRAPPER_IN)
		{
			if (pPpConf->ppInImg.bufferBusAddr)
			{
				mem_free((BYTE*)pPpConf->ppInImg.bufferBusAddr);
				pPpConf->ppInImg.bufferBusAddr = (u32)NULL;
			}
			reset_InPixelFormat(pPpConf, source);
		}
		reset_crop(pPpConf);
		reset_rotation(pPpConf);
		reset_deinterlace(pPpConf);
		reset_rgb(pPpConf);
		reset_scaling(pPpConf);
		reset_dithering(pPpConf);
		reset_OutFrmBuffer(pPpConf);
		reset_OutMask1(pPpConf);
		reset_OutMask2(pPpConf);
	}
}

void auto_PP(ST_IMGWIN * const target, const ST_IMGWIN * const source)
{
	MP_DEBUG("auto_pp");
	PPInst pp = NULL;
	PPConfig pPpConf;
	PPResult ppRet;
	*((int *)DMA_MPVR0_BASE) = 0x00000001;
	*((int *)DMA_MPVR1_BASE) = 0x00000001;

	ppRet = PPInit(&pp);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPInit fail %d", ppRet);
		goto autoPP_end;
	}//else MP_DEBUG("PPInit OK");
	PPContainer *ppC = (PPContainer *)pp;

	/* First get the default PP settings */
	ppRet = PPGetConfig(pp, &pPpConf);
	if (ppRet != PP_OK)
	{
		MP_ALERT("PPGetConfig fail %d", ppRet);
		goto autoPP_end;
	}//else MP_DEBUG("PPGetConfig PP_OK");

	reset_PPConfig(target, source, pp, &pPpConf);

//	auto_PP_input_resolution(target, source, pp, &pPpConf);//this one must test alone

	auto_PP_rotation(target, source, pp, &pPpConf);
	auto_PP_scaling(target, source, pp, &pPpConf);
	auto_PP_InImg_PixelFormat(target, source, pp, &pPpConf);
	auto_PP_crop(target, source, pp, &pPpConf);
	auto_PP_Deinterlace(target, source, pp, &pPpConf);
	auto_PP_OutImg_PixelFormat(target, source, pp, &pPpConf);
	auto_PP_IDU_422YCbYCr(target, source, pp, &pPpConf);
	auto_PP_contrast(target, source, pp, &pPpConf);
	auto_PP_brightness(target, source, pp, &pPpConf);
	auto_PP_saturation(target, source, pp, &pPpConf);
	auto_PP_dithering(target, source, pp, &pPpConf);
	auto_PP_OutFrmBuffer(target, source, pp, &pPpConf);	//risk: right boundary has something when set negative number
														//failed: 560x294, failed at upper
	auto_PP_OutMask1(target, source, pp, &pPpConf);
	auto_PP_OutMask2(target, source, pp, &pPpConf);
//	auto_PP_random(target, source, pp, &pPpConf);

	//the following rgb conversion are already coverd by contrast, brightness or saturtation
	//auto_PP_RGB16_565(target, source, pp, &pPpConf);
	//auto_PP_RGB32(target, source, pp, &pPpConf);
autoPP_end:
	/* Release the post-processor instance */
	PPRelease(pp);

	*((u32 *)DMA_MPVR0_BASE) &= 0xFFFFFFFE;
	*((u32 *)DMA_MPVR1_BASE) &= 0xFFFFFFFE;
	MP_DEBUG("auto_PP return");
}

void mmcp_memcpy_peformance_test(ST_IMGWIN * const target, ST_IMGWIN * const source)
{
	unsigned int size = 0x1;
	DWORD tcpu, tmmcp, winonce=0;
	BYTE *sourcecpy;
	BYTE *dstcpy;
	DWORD pDtick, pDtpc, pDtmv;

	mpDebugPrint("start test from size=0x%x", size);

	for (;size<0x600;size+=0x1)
	{
		//test for non-chache memory
		sourcecpy=(BYTE*)((int)target->pdwStart | 0x20000000);
		dstcpy = (BYTE*)((int)source->pdwStart | 0x20000000);
	
		get_cur_timeL(&pDtick, &pDtpc, &pDtmv);
		memcpy(dstcpy, sourcecpy, size);
		tcpu = get_elapsed_timeL(pDtick, pDtpc, pDtmv);

		get_cur_timeL(&pDtick, &pDtpc, &pDtmv);
		mmcp_memcpy_polling(dstcpy, sourcecpy, size);
		tmmcp = get_elapsed_timeL(pDtick, pDtpc, pDtmv);

		if (tmmcp < tcpu)
		{
			mpDebugPrint("mmcp win at 0x%x", size);
			winonce=1;
		}
		else if (winonce)	mpDebugPrint("mmcp lose at 0x%x", size);
//		mpDebugPrint("tcpu=%d, tdma=%d", tcpu, tdma);

	}
	mpDebugPrint("end of %s", __FUNCTION__);
}


