
#ifndef _IPU_TEST_H
#define _IPU_TEST_H

#include "iplaysysconfig.h"

//***********************************************************************
//                                                          define
//***********************************************************************
#define MCU_BYTE                32
#define MCU_PIXEL               16
#define MCU_TOTAL_Y         16
#define MCU_TOTAL_U         8
#define MCU_TOTAL_V         8
#define IMAGE_SIZE_420 	((pstTCON->wWidth / MCU_PIXEL) * MCU_BYTE * pstTCON->wHeight)
#define MCU_SIZE        0x200


#define TYPE_422            0
#define TYPE_420            1

extern DWORD trgBuffStart, totalSectors, CurrTargetWidth;
extern volatile BYTE scalingCompleted, sectorLines;

typedef struct{
    DWORD *dwPointer;           /* start address of image buffer */
    DWORD dwOffset;             /* buffer width in byte unit */
    WORD  wHeight;              /* working area height in pixel */
    WORD  wWidth;               /* working area width in pixel */
    WORD  wType;                /* type of input data stream (422 0r 420) */
    BYTE  bReserve[2];
} ST_IMAGEINFO;

typedef struct{
    WORD wHUp;
    WORD wHSF;
    WORD wHSubRatio;
    WORD wVUp;
    WORD wVSF;
    WORD wVSubRatio;
} ST_SCA_PARM;

typedef struct
{
    char Hue;
    char Saturation;
} CHAR_COLOR;

typedef struct{
    WORD tx1;
	WORD ty1;
	BYTE non_colorkey_level;
	BYTE colorkey_enable;
	BYTE colorkey_level;
	BYTE colorkey_Y;
	BYTE colorkey_Cb;
	BYTE colorkey_Cr;
} ST_OVERLAY_PARAM;

//***********************************************************************
//                                              Function
//***********************************************************************
BYTE Ipu_ImageScaling(ST_IMGWIN *, ST_IMGWIN *, WORD, WORD, WORD, WORD, WORD, WORD, WORD, WORD, BYTE);
//void Ipu_Zoom(ST_IMGWIN *, ST_IMGWIN *, WORD, WORD, WORD, WORD);
void Ipu_Zoom(ST_IMGWIN *, ST_IMGWIN *, WORD, WORD, WORD, WORD, WORD, WORD, WORD, WORD);

#endif

