#ifndef __BASIC_STRUCT_H__
#define __BASIC_STRUCT_H__


#include "Basic_Enum.h"



typedef struct
{
  int					CacheOn;
  int					Capacity;
  CACHE_SPACE_STYLE		CacheSpaceStyle;
  int					MemorySize;
  unsigned char*		pMemory;
  int					bAddDummy4BytesAlign;
} stCacheSetting, *pstCacheSetting;


typedef struct
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
}stRGB, *pstRGB;

typedef struct
{
	int Y;
	int Cb;
	int Cr;
}stYYCbCr, *pstYYCbCr;

typedef struct
{
	int color_1;
	int color_2;
	int color_3;
}stColorData, *pstColorData;


typedef struct
{
	int x;
	int y;
}stPoint, *pstPoint;


typedef struct
{
	int x;
	int y;
}stSize, *pstSize;

typedef struct
{
	int x;
	int y;
}stVector, *pstVector;

typedef struct
{
	stPoint pt_1;
	stPoint pt_2;
}stRectangle, *pstRectangle;

typedef struct
{
	int          Width;
	int          Height;
	int          Pitch;
	int          AdvancePixel;
	int          HoriBearingPixelX;
	int          HoriBearingPixelY;
	int          ShiftLeft;
	int          ShiftTop;
	stPoint      Origin;
	int          BitPerPixel;
}stGlyphBMPMetrics, *pstGlyphBMPMetrics;





#endif //__BASIC_STRUCT_H__
