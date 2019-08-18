#ifndef __bmp_h
#define __bmp_h

#include "global612.h"


#define	NOT_BMP_FILE				-1
#define	BMP_IMG_HEADER_ERR			-2
#define	NOT_SUPPROTED_BMP			-3
#define NOT_SUPPORTED_BMP_SIZE    -4


#define	BMP_GET_WORD(x,y)	(x)++;(y)=*(x);(x)--;(y)=(y<<8)+*(x);(x)+=2
#define	BMP_GET_DWORD(x, y)	(x)+=3;(y)=*(x);(x)--;(y)=(y<<8)+*(x);(x)--;(y)=(y<<8)+*(x);(x)--;(y)=(y<<8)+*(x);(x)+=4

enum	//dwBMP16Type
{
	RGB_X555,
	RGB_565,
	RGB_A555,
};
enum		//biCompression 
{
	BI_RGB =0,
	BI_BITFIELD = 3,
};

typedef struct{
	WORD	bType;
	DWORD	dwSize;
	WORD	wUnused1;
	WORD	wUnused2;
	DWORD	dwOffset;
}ST_BMP_FILE_HEADER;


typedef struct{
	DWORD	dwHeaderSize;
	DWORD	dwWidth;
	DWORD	dwHeight;
	WORD	wPlanes;
	WORD	wBitCount;
	DWORD	dwCompression;
	DWORD	dwImgSize;
	DWORD	dwXperMeter;
	DWORD	dwYperMeter;
	DWORD	dwColourUsed;
	DWORD	dwSignificantColour;
}ST_BMP_IMAGE_HEADER;


SWORD Img_BMPDecode(BYTE *, BYTE *, BYTE);
SWORD Img_BMPDecodeChasing(BYTE * bpSource, BYTE * bpTarget, BYTE yield, STREAM * aHandle);
DWORD Img_BMPGetSize(BYTE *);

#endif

void DumpYuvImage2Bmp(WORD WIDTH,WORD HEIGHT,BYTE *pbTarget);
