//---------------------------------------------------------------------------

#ifndef ImageFileH
#define ImageFileH
//---------------------------------------------------------------------------

#include "global612.h"
#include "mpStreamApi.h"


///
///@ingroup     ImageDecoder
///@brief       Image file data sructure
typedef struct stImageFile {
	ST_MPX_STREAM sStream;
	ST_MPX_STREAM *psStream;
	void *psJpeg;
	
	BYTE *pbNityDegreeTarget;
	BYTE *pbTarget;

	int  iErrorCode;
	DWORD dwTargetSize;
#if PNG   
	DWORD dwPngTargetSize;
#else
	DWORD bReserve_1;
#endif
	DWORD dwDecodeOffset;

  WORD wImageWidth;
  WORD wImageHeight;

  WORD wTargetWidth;
  WORD wTargetHeight;	
	WORD wRealTargetWidth;
	WORD wRealTargetHeight;	
	
  WORD wThumbWidth;
  WORD wThumbHeight;
	
	BYTE bNityDegree;
	BYTE bFileFormat;
	BYTE bDecodeMode;
	BYTE bScaleDown;
	BYTE bSpecialFormat;
	BYTE bReserve[3];

#if OPEN_EXIF
	ST_EXIF_INFO sImageDecEXIFinfo;
#endif	

    int(*ImageDecodeThumb) (struct stImageFile *psImage);     
	int(*ImageDecodeImage) (struct stImageFile *psImage);

#if MPO	
	int(*ImageDecodeImageRight)(struct stImageFile *psImage);
	int(*ImageDecodeImageClose)(struct stImageFile *psImage);
#endif
	
	void(*LibjpegCallback) (struct stImageFile *psImage, BOOL boLastScan);

#if MPO
	BYTE bExtension[5];
    BYTE bReserved2[3];
#endif	
}IMAGEFILE;

#ifndef SEEK_SET
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
#endif

#define IMAGE_TYPE_NOTCHECKED   0
#define IMAGE_TYPE_UNKOWN       0xff
#define IMAGE_TYPE_JPEG         1
#define IMAGE_TYPE_BMP 	        2
#define IMAGE_TYPE_GIF          3
#define IMAGE_TYPE_PNG	        4
#define IMAGE_TYPE_JPEG444 			5
#define IMAGE_TYPE_JPEG_PROGRESSIVE 6
#define IMAGE_TYPE_TIFF		7

#define IMAGE_TAG_JPEG 0xFFD8
#define IMAGE_TAG_BMP  0x424D
#define IMAGE_TAG_GIF  0x4749
#define IMAGE_TAG_PNG  0x8950
#define IMAGE_TAG_TIFF_LITTLE 0x4949
#define IMAGE_TAG_TIFF_BIG 0x4D4D

#define INTEL 0
#define MOTOROLA 1



int Jpg_Decoder_DecodeThumb(IMAGEFILE *psImage);
int Jpg_Decoder_DecodeImage(IMAGEFILE *psImage);
int Jpg_Decoder_ImageFile_Init(IMAGEFILE *psImage);


int Bmp_Decoder_DecodeThumb(IMAGEFILE *psImage);
int Bmp_Decoder_DecodeImage(IMAGEFILE *psImage);
int Bmp_Decoder_Init(IMAGEFILE *psImage);


WORD ImageFile_GetThumbHeight();
WORD ImageFile_GetImageWidth();
WORD ImageFile_GetImageHeight();
WORD ImageFile_GetTargetWidth();
WORD ImageFile_GetTargetHeight();
IMAGEFILE *ImageFile_GetCurImageFile();

DWORD ImageFile_GetImageSize(IMAGEFILE *psImage);
int ImageFile_DecodeFile(IMAGEFILE *psImage, STREAM * psHandle, BYTE  *pbSource, DWORD dwSourceSize, BYTE bMode);

void *ImageGetTargetBuffer();
void *ImageGetSourceBuffer();
void *ImageAllocSourceBuffer(DWORD dwSize);
void *ImageAllocTargetBuffer(DWORD dwSize);


#endif

